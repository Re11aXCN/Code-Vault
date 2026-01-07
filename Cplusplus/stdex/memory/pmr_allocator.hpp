#pragma once

#include <algorithm>
#include <array>
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <memory_resource>
#include <mutex>
#include <new>
#include <type_traits>
#include <utility>

// Use aligned_allocator for aligned memory allocation
#include "aligned_allocator.hpp"

namespace stdex {

// ============================================================================
// Lock Strategy for Synchronization Control
// ============================================================================

// Compile-time selection between synchronized and unsynchronized behavior
template<bool IsSynchronized>
struct LockStrategy;

template<>
struct LockStrategy<false>
{
  // Empty mutex for unsynchronized (compile-time optimized away)
  struct EmptyMutex
  {
    void lock() const noexcept { }

    void unlock() const noexcept { }

    [[nodiscard]] bool try_lock() const noexcept { return true; }
  };

  using mutex_type = EmptyMutex;

  // Empty lock guard (zero runtime overhead)
  template<typename Mutex>
  struct EmptyLockGuard
  {
    explicit EmptyLockGuard(const Mutex&) noexcept { }

    ~EmptyLockGuard() noexcept = default;
  };

  template<typename Mutex>
  static EmptyLockGuard<Mutex> make_lock(const Mutex& mutex) noexcept
  {
    return EmptyLockGuard<Mutex>(mutex);
  }
};

template<>
struct LockStrategy<true>
{
  using mutex_type = std::mutex;

  template<typename Mutex>
  static std::lock_guard<Mutex> make_lock(Mutex& mutex) noexcept
  {
    return std::lock_guard<Mutex>(mutex);
  }
};

// ============================================================================
// Pool Allocator using Free-List Allocation Strategy
// ============================================================================

// Optimized for small, frequently allocated objects
// Combines aligned allocation with free-list management
// Similar to FastAllocator: configurable alignment and POD optimization
template<bool        IsSynchronized = false,
         std::size_t Alignment      = 64,   // Cache-line alignment by default
         bool        OptimizePOD    = true  // Skip construction for POD types
         >
class PoolAllocator
{
  struct dummy
  { };

public:

  static_assert((Alignment & (Alignment - 1)) == 0,
                "Alignment must be power of 2");

  PoolAllocator() : m_free_lists {}
  {
    static_assert(Alignment > 0, "Alignment must be positive");
  }

  virtual ~PoolAllocator()
  {
    if constexpr (IsSynchronized) {
      std::lock_guard<std::mutex> global_lock(m_global_mutex);
    }
    std::fill(m_free_lists.begin(), m_free_lists.end(), nullptr);
    std::for_each(m_memory_chunks.begin(), m_memory_chunks.end(),
                  deallocate_aligned_memory);
  }

  PoolAllocator(const PoolAllocator&)             = delete;
  PoolAllocator& operator= (const PoolAllocator&) = delete;

  // Allocate memory with specified alignment
  [[nodiscard]] void* allocate(std::size_t bytes, std::size_t alignment)
  {
    if ((alignment & (alignment - 1)) != 0) [[unlikely]] {
      throw std::invalid_argument("alignment must be power of 2");
    }

    // Large allocations or high alignment requirements go to system allocator
    if (bytes > MaxBytes || alignment > Alignment) {
      void* pointer =
          allocate_aligned_memory(std::max(alignment, Alignment), bytes);
      if (!pointer) throw std::bad_alloc();
      return pointer;
    }

    const std::size_t rounded_bytes = round_up(bytes);
    const std::size_t index         = free_list_index(rounded_bytes);

    {
      auto lock = make_list_lock(index);

      if (Node* current = m_free_lists [index]) {
        m_free_lists [index] = current->next;
        return current;
      }
    }

    // Free list empty, refill it
    return refill(rounded_bytes);
  }

  void deallocate(void* pointer, std::size_t bytes, std::size_t alignment)
  {
    if (pointer == nullptr) [[unlikely]]
      return;

    if (bytes > MaxBytes || alignment > Alignment) {
      deallocate_aligned_memory(pointer);
      return;
    }

    const std::size_t index = free_list_index(round_up(bytes));

    auto lock = make_list_lock(index);

    Node* released       = reinterpret_cast<Node*>(pointer);
    released->next       = m_free_lists [index];
    m_free_lists [index] = released;
  }

  // Construct with POD optimization (similar to FastAllocator)
  template<typename Type, typename... Args>
  void construct(Type* pointer, Args&&... arguments) noexcept(
      std::is_nothrow_constructible_v<Type, Args...>)
  {
    if constexpr (!(OptimizePOD &&
                    std::is_trivially_default_constructible_v<Type> &&
                    sizeof...(arguments) == 0)) {
      ::new (static_cast<void*>(pointer))
          Type(std::forward<Args>(arguments)...);
    }
    // For POD types with no arguments, skip construction (FastAllocator
    // optimization)
  }

  // Destroy with POD optimization
  template<typename Type>
  void destroy(Type* pointer) noexcept
  {
    if constexpr (!(OptimizePOD && std::is_trivially_destructible_v<Type>)) {
      pointer->~Type();
    }
    // For POD types, skip destruction (FastAllocator optimization)
  }

protected:

  union Node
  {
    union Node* next;
    char        data [1] alignas(sizeof(void*));
  };

  static constexpr std::size_t MaxBytes        = 128;
  static constexpr std::size_t FREE_LIST_COUNT = MaxBytes / Alignment;

  std::array<Node*, FREE_LIST_COUNT> m_free_lists;
  std::vector<char*>                 m_memory_chunks;

  using node_lock_arr_t =
      std::conditional_t<IsSynchronized,
                         std::array<std::mutex, FREE_LIST_COUNT>, Dummy>;
  using global_lock_t = std::conditional_t<IsSynchronized, std::mutex, Dummy>;

  [[no_unique_address]] node_lock_arr_t m_list_mutexes {};
  [[no_unique_address]] global_lock_t   m_global_mutex;

  // Compile-time round up to alignment boundary
  static constexpr std::size_t round_up(std::size_t bytes) noexcept
  {
    return ((bytes + Alignment - 1) & ~(Alignment - 1));
  }

  // Compile-time compute free list index
  static constexpr std::size_t free_list_index(std::size_t bytes) noexcept
  {
    return (bytes / Alignment) - 1;
  }

  // Helper to make lock on specific free list
  auto make_list_lock(std::size_t index)
  {
    if constexpr (IsSynchronized) {
      return std::lock_guard<std::mutex>(m_list_mutexes [index]);
    } else {
      return typename LockStrategy<false>::template EmptyLockGuard<
          decltype(m_list_mutexes)> {};
    }
  }

  // Allocate memory chunk for free list using aligned allocation
  char* chunk_alloc(std::size_t size, std::uint32_t& chunk_count)
  {
    const std::uint32_t min_chunk_count = 1;
    while (chunk_count >= min_chunk_count) {
      // Use aligned allocation (from AlignedAllocator)
      char* chunk = reinterpret_cast<char*>(
          allocate_aligned_memory(Alignment, size * chunk_count));
      if (chunk != nullptr) {
        if constexpr (IsSynchronized) {
          std::lock_guard<std::mutex> global_lock(m_global_mutex);
        }
        m_memory_chunks.push_back(chunk);
        return chunk;
      }
      // Allocation failed: halve and retry (minimum 1)
      chunk_count = (chunk_count > min_chunk_count) ? (chunk_count >> 1)
                                                    : min_chunk_count;
    }
    throw std::bad_alloc();
  }

  // Refill free list with new memory chunk
  void* refill(std::size_t size)
  {
    std::uint32_t chunk_count = 16;  // Allocate 16 nodes at a time
    char*         chunk       = chunk_alloc(size, chunk_count);

    // Only allocated 1 node: return directly
    if (chunk_count == 1) return chunk;

    // Allocated multiple nodes: build free list
    const std::size_t index = free_list_index(size);

    Node* return_node = reinterpret_cast<Node*>(chunk);
    Node* head        = reinterpret_cast<Node*>(chunk + size);
    Node* current     = head;

    // Build linked list from remaining nodes
    for (std::uint32_t i = 2; i < chunk_count; ++i) {
      Node* next =
          reinterpret_cast<Node*>(reinterpret_cast<char*>(current) + size);
      current->next = next;
      current       = next;
    }
    current->next = nullptr;

    {
      auto lock            = make_list_lock(index);
      m_free_lists [index] = head;
    }

    return return_node;
  }
};

// ============================================================================
// Pool-Based Memory Resource (PMR compatible)
// ============================================================================

template<bool IsSynchronized = false>
class PoolResource : public std::pmr::memory_resource
{
public:

  PoolResource()           = default;
  ~PoolResource() override = default;

  PoolResource(const PoolResource&)             = delete;
  PoolResource& operator= (const PoolResource&) = delete;

protected:

  [[nodiscard]] void* do_allocate(std::size_t bytes,
                                  std::size_t alignment) override
  {
    return m_allocator.allocate(bytes, alignment);
  }

  void do_deallocate(void* pointer, std::size_t bytes,
                     std::size_t alignment) override
  {
    m_allocator.deallocate(pointer, bytes, alignment);
  }

  [[nodiscard]] bool
  do_is_equal(const std::pmr::memory_resource& other) const noexcept override
  {
    // PMR specification: only equal if same object
    return this == &other;
  }

private:

  PoolAllocator<IsSynchronized, 64, true> m_allocator;
};

// Convenience aliases
using SynchronizedPoolResource   = PoolResource<true>;
using UnsynchronizedPoolResource = PoolResource<false>;

// ============================================================================
// Polymorphic Allocator (PMR compatible)
// ============================================================================

template<typename Type>
class PolymorphicAllocator
{
private:

  std::pmr::memory_resource* m_resource;

public:

  using value_type = Type;

  PolymorphicAllocator() noexcept : m_resource(std::pmr::get_default_resource())
  {
  }

  explicit PolymorphicAllocator(std::pmr::memory_resource* resource) noexcept
      : m_resource(resource ? resource : std::pmr::get_default_resource())
  {
  }

  template<typename OtherType>
  PolymorphicAllocator(const PolymorphicAllocator<OtherType>& other) noexcept
      : m_resource(other.resource())
  {
  }

  // Core allocation function
  Type* allocate(std::size_t count)
  {
    if (count > std::size_t(-1) / sizeof(Type)) {
      throw std::bad_array_new_length();
    }
    return static_cast<Type*>(
        m_resource->allocate(count * sizeof(Type), alignof(Type)));
  }

  // Core deallocation function
  void deallocate(Type* pointer, std::size_t count) noexcept
  {
    if (pointer) {
      m_resource->deallocate(pointer, count * sizeof(Type), alignof(Type));
    }
  }

  // Construct with POD optimization
  template<typename OtherType, typename... Args>
  void construct(OtherType* pointer, Args&&... arguments)
  {
    if constexpr (!(std::is_trivially_default_constructible_v<OtherType> &&
                    sizeof...(arguments) == 0)) {
      ::new (static_cast<void*>(pointer))
          OtherType(std::forward<Args>(arguments)...);
    }
  }

  // Destroy with POD optimization
  template<typename OtherType>
  void destroy(OtherType* pointer) noexcept
  {
    if constexpr (!std::is_trivially_destructible_v<OtherType>) {
      pointer->~OtherType();
    }
  }

  // Allocator rebinding
  template<typename OtherType>
  struct rebind
  {
    using other = PolymorphicAllocator<OtherType>;
  };

  // Get underlying resource
  std::pmr::memory_resource* resource() const noexcept { return m_resource; }

  // Comparison operators
  template<typename OtherType>
  bool operator== (const PolymorphicAllocator<OtherType>& other) const noexcept
  {
    return m_resource == other.resource();
  }

  template<typename OtherType>
  bool operator!= (const PolymorphicAllocator<OtherType>& other) const noexcept
  {
    return !(*this == other);
  }
};
}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <vector>
#include <memory_resource>

// Example 1: Basic pool allocator usage
void example_pool_allocator() {
    stdex::PoolAllocator<false, 64, true> allocator;

    // Allocate small objects from pool
    void* pointer1 = allocator.allocate(16, 8);
    void* pointer2 = allocator.allocate(32, 8);

    // Use memory...
    std::cout << "Allocated from pool: " << pointer1 << " and " << pointer2 <<
std::endl;

    // Construct objects (POD optimization: skipped for trivial types)
    int* int_pointer = static_cast<int*>(pointer1);
    allocator.construct(int_pointer, 42);
    std::cout << "Constructed value: " << *int_pointer << std::endl;
    allocator.destroy(int_pointer);

    // Deallocate
    allocator.deallocate(pointer1, 16, 8);
    allocator.deallocate(pointer2, 32, 8);
}

// Example 2: Using pool_resource with PMR containers
void example_pmr_containers() {
    // Create unsynchronized pool resource (64-byte aligned, POD optimized)
    stdex::UnsynchronizedPoolResource pool;

    // Create PMR containers using pool resource
    std::pmr::vector<int> vec(&pool);
    vec.reserve(100);

    for (int i = 0; i < 100; ++i) {
        vec.push_back(i);
    }

    std::cout << "Vector size: " << vec.size() << std::endl;
}

// Example 3: Thread-safe synchronized pool
void example_synchronized_pool() {
    stdex::SynchronizedPoolResource pool;

    std::pmr::vector<double> vec(&pool);
    for (int i = 0; i < 1000; ++i) {
        vec.push_back(i * 1.5);
    }

    std::cout << "Synchronized pool vector size: " << vec.size() << std::endl;
}

// Example 4: Custom polymorphic allocator with POD optimization
template<typename Type>
using MyVector = std::vector<Type, stdex::PolymorphicAllocator<Type>>;

void example_custom_allocator() {
    stdex::UnsynchronizedPoolResource pool;
    stdex::PolymorphicAllocator<int> alloc(&pool);

    MyVector<int> vec(alloc);
    vec.reserve(50);

    for (int i = 0; i < 50; ++i) {
        vec.push_back(i * 2);
    }

    std::cout << "Custom allocator vector size: " << vec.size() << std::endl;
}

// Example 5: POD vs non-POD construction
struct PODData {
    int x;
    double y;
};

struct NonPODData {
    int x;
    NonPODData(int val) : x(val) {
        std::cout << "NonPODData constructed: " << x << std::endl;
    }
    ~NonPODData() {
        std::cout << "NonPODData destroyed: " << x << std::endl;
    }
};

void example_pod_optimization() {
    stdex::PoolAllocator<false, 64, true> allocator;

    // POD: construction is skipped for efficiency
    void* pod_memory = allocator.allocate(sizeof(PODData), alignof(PODData));
    PODData* pod_pointer = static_cast<PODData*>(pod_memory);
    allocator.construct(pod_pointer);  // No-op for POD with OptimizePOD=true
    pod_pointer->x = 42;
    pod_pointer->y = 3.14;
    std::cout << "POD: x=" << pod_pointer->x << ", y=" << pod_pointer->y <<
std::endl; allocator.destroy(pod_pointer);  // No-op for POD
    allocator.deallocate(pod_memory, sizeof(PODData), alignof(PODData));

    // Non-POD: construction is NOT skipped
    void* non_pod_memory = allocator.allocate(sizeof(NonPODData),
alignof(NonPODData)); NonPODData* non_pod_pointer =
static_cast<NonPODData*>(non_pod_memory); allocator.construct(non_pod_pointer,
100);  // Calls constructor allocator.destroy(non_pod_pointer);  // Calls
destructor allocator.deallocate(non_pod_memory, sizeof(NonPODData),
alignof(NonPODData));
}

int main() {
    example_pool_allocator();
    example_pmr_containers();
    example_synchronized_pool();
    example_custom_allocator();
    example_pod_optimization();
    return 0;
}
*/
