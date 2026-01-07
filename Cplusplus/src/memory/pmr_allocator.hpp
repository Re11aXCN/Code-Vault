#pragma once
#include <cstddef>
#include <cstdlib>
#include <memory>
#include <memory_resource>
#include <array>
#include <vector>
#include <algorithm>
#include <mutex>

namespace stdex {
#ifdef _WIN32
  inline void* aligned_alloc(size_t alignment, size_t size) {
    return _aligned_malloc(size, alignment);
  }
  inline void aligned_free(void* ptr) {
    _aligned_free(ptr);
  }
#else
  inline void aligned_free(void* ptr) {
    std::free(ptr);
  }
#endif
  template <bool IsSynchronized>
  struct lock_strategy;

  template <>
  struct lock_strategy<false> {
    struct empty_mutex {
      void lock() const noexcept {}
      void unlock() const noexcept {}
      bool try_lock() const noexcept { return true; }
    };

    using mutex_type = empty_mutex;
    template <typename Mutex>
    using lock_guard = std::conditional_t<true, std::lock_guard<Mutex>, void>;

    // 空锁的lock_guard封装（编译期消除）
    template <typename Mutex>
    struct empty_lock_guard {
      explicit empty_lock_guard(const Mutex&) noexcept {}
      ~empty_lock_guard() noexcept {}
    };

    template <typename Mutex>
    static empty_lock_guard<Mutex> make_lock(const Mutex& m) noexcept {
      return empty_lock_guard<Mutex>(m);
    }
  };

  template <>
  struct lock_strategy<true> {
    using mutex_type = std::mutex;

    template <typename Mutex>
    static std::lock_guard<Mutex> make_lock(Mutex& m) noexcept {
      return std::lock_guard<Mutex>(m);
    }
  };

  template <bool IsSynchronized, std::size_t ALIGN = 8, std::size_t MAX_BYTES = 128>
  class pool_allocator {
    struct dummy {};
  public:
    pool_allocator()
      : free_lists{}
    {
      static_assert(ALIGN && ((ALIGN & (ALIGN - 1)) == 0), "ALIGN must be power of 2");
      static_assert(MAX_BYTES % ALIGN == 0, "MAX_BYTES must be multiple of ALIGN");
    };

    virtual ~pool_allocator() {
      //@ auto global_lock = lock_strategy<IsSynchronized>::make_lock(global_mutex);
      if constexpr (IsSynchronized)
        std::lock_guard<std::mutex> global_lock(global_mutex);

      std::fill(free_lists.begin(), free_lists.end(), nullptr);
      std::for_each(memory_chunks.begin(), memory_chunks.end(), aligned_free);
    }

    pool_allocator(const pool_allocator&) = delete;
    pool_allocator& operator=(const pool_allocator&) = delete;

    [[nodiscard]] void* allocate(std::size_t bytes, std::size_t alignment) {
      if ((alignment & (alignment - 1)) != 0) [[unlikely]] {
        throw std::invalid_argument("alignment must be power of 2");
      }

      // 大分配/对齐要求超过ALIGN，走系统malloc
      if (bytes > MAX_BYTES || alignment > ALIGN) {
        void* ptr = aligned_alloc(alignment, bytes);
        if (!ptr) throw std::bad_alloc();
        return ptr;
      }

      const std::size_t rounded_bytes = round_up(bytes);
      const std::size_t index = free_list_index(rounded_bytes);

      //@ auto list_lock = lock_strategy<IsSynchronized>::make_lock(list_mutexes[index]);
      {
        if constexpr (IsSynchronized)
          std::lock_guard<std::mutex> list_lock(list_mutexes[index]);

        if (node* current = free_lists[index]) {
          free_lists[index] = current->next; // 从空闲链表中移除
          return current;
        }
      }

      // 空闲链表为空，重新填充
      return refill(rounded_bytes);
    }

    void deallocate(void* ptr, std::size_t bytes, std::size_t alignment) {
      if (ptr == nullptr) [[unlikely]] return;

      if (bytes > MAX_BYTES || alignment > ALIGN) {
        aligned_free(ptr);
        return;
      }

      const std::size_t index = free_list_index(round_up(bytes));

      //@ auto list_lock = lock_strategy<IsSynchronized>::make_lock(list_mutexes[index]);
      {
        if constexpr (IsSynchronized)
          std::lock_guard<std::mutex> list_lock(list_mutexes[index]);

        node* released = reinterpret_cast<node*>(ptr);
        released->next = free_lists[index];
        free_lists[index] = released;
      }
    }

  protected:
    union node {
      union node* next;
      char data[1] alignas(sizeof(void*));
    };
    static constexpr std::size_t FREE_LIST_COUNT = MAX_BYTES / ALIGN;

    std::array<node*, FREE_LIST_COUNT> free_lists;// 空闲链表数组
    std::vector<char*> memory_chunks;             // 已分配的内存块列表（用于析构释放）

    // 每个链表一个锁，细粒度（非线程安全时为empty_mutex，编译期优化）
    //@ std::array<typename lock_strategy<IsSynchronized>::mutex_type, FREE_LIST_COUNT> list_mutexes;
    //@ [[no_unique_address]] typename lock_strategy<IsSynchronized>::mutex_type global_mutex;

    using node_lock_arr_t = std::conditional_t<IsSynchronized, std::array<std::mutex, FREE_LIST_COUNT>, dummy>;
    using global_lock_t = std::conditional_t<IsSynchronized, std::mutex, dummy>;
    [[no_unique_address]] node_lock_arr_t list_mutexes{};
    [[no_unique_address]] global_lock_t global_mutex;

    // 编译期向上取整到ALIGN的倍数
    static constexpr std::size_t round_up(std::size_t bytes) noexcept {
      return ((bytes + ALIGN - 1) & ~(ALIGN - 1));
    }

    // 编译期计算空闲链表索引
    static constexpr std::size_t free_list_index(std::size_t bytes) noexcept {
      return (bytes / ALIGN) - 1;
    }

    // 为空闲链表分配内存块
    char* chunk_alloc(std::size_t size, uint32_t& chunk_count) {
      const uint32_t min_chunk_count = 1;
      while (chunk_count >= min_chunk_count) {
        char* chunk = reinterpret_cast<char*>(aligned_alloc(ALIGN, size * chunk_count));
        if (chunk != nullptr) {
          //@ auto global_lock = lock_strategy<IsSynchronized>::make_lock(global_mutex);
          if constexpr (IsSynchronized)
            std::lock_guard<std::mutex> global_lock(global_mutex);

          memory_chunks.push_back(chunk);
          return chunk;
        }
        // 分配失败：减半重试（最小1）
        chunk_count = (chunk_count > min_chunk_count) ? (chunk_count >> 1) : min_chunk_count;
      }
      throw std::bad_alloc();
    }

    // 重新填充空闲链表
    void* refill(std::size_t size) {
      uint32_t chunk_count = 16U; // 默认每次分配16个节点
      char* chunk = chunk_alloc(size, chunk_count);

      // 仅分配了1个节点：直接返回
      if (chunk_count == 1U) return chunk;

      // 分配多个节点：构建空闲链表
      const std::size_t index = free_list_index(size);
      //@ auto list_lock = lock_strategy<IsSynchronized>::make_lock(list_mutexes[index]);

      // 第一个节点返回给调用者，剩余节点加入空闲链表
      node* return_node = reinterpret_cast<node*>(chunk);
      node* head = reinterpret_cast<node*>(chunk + size);
      node* current = head;

      // 遍历剩余节点构建链表
      for (uint32_t i = 2U; i < chunk_count; ++i) {
        node* next = reinterpret_cast<node*>(reinterpret_cast<char*>(current) + size);
        current->next = next;
        current = next;
      }
      current->next = nullptr;

      {
        if constexpr (IsSynchronized)
          std::lock_guard<std::mutex> list_lock(list_mutexes[index]);
        free_lists[index] = head;
      }

      return return_node;
    }
  };

  template <bool IsSynchronized>
  class pool_resource : public std::pmr::memory_resource {
  public:
    pool_resource() = default;
    ~pool_resource() override = default;

    pool_resource(const pool_resource&) = delete;
    pool_resource& operator=(const pool_resource&) = delete;

  protected:
    [[nodiscard]] void* do_allocate(std::size_t bytes, std::size_t alignment) override {
      return m_allocator.allocate(bytes, alignment);
    }

    void do_deallocate(void* ptr, std::size_t bytes, std::size_t alignment) override {
      m_allocator.deallocate(ptr, bytes, alignment);
    }

    [[nodiscard]] bool do_is_equal(const std::pmr::memory_resource& other) const noexcept override {
      // PMR规范：只有同一对象才相等
      return this == &other;
    }

  private:
    pool_allocator<IsSynchronized> m_allocator;
  };

  using synchronized_pool_resource = pool_resource<true>;
  using unsynchronized_pool_resource = pool_resource<false>;

  template <typename T>
  class polymorphic_allocator {
  private:
    std::pmr::memory_resource* m_mr;

  public:
    using value_type = T;

    polymorphic_allocator() noexcept
      : m_mr(std::pmr::get_default_resource()) {
    }

    explicit polymorphic_allocator(std::pmr::memory_resource* mr) noexcept
      : m_mr(mr ? mr : std::pmr::get_default_resource()) {
    }

    template <typename U>
    polymorphic_allocator(const polymorphic_allocator<U>& other) noexcept
      : m_mr(other.resource()) {
    }

    // 核心分配函数
    T* allocate(std::size_t n) {
      if (n > std::size_t(-1) / sizeof(T)) {
        throw std::bad_array_new_length();
      }
      return static_cast<T*>(m_mr->allocate(n * sizeof(T), alignof(T)));
    }

    // 核心释放函数
    void deallocate(T* p, std::size_t n) noexcept {
      if (p) {
        m_mr->deallocate(p, n * sizeof(T), alignof(T));
      }
    }

    // 使用placement new而非allocator_traits
    template <typename U, typename... Args>
    void construct(U* p, Args&&... args) {
      // 直接使用placement new构造对象（标准allocator的默认行为）
      ::new (static_cast<void*>(p)) U(std::forward<Args>(args)...);
    }

    // 直接调用析构函数
    template <typename U>
    void destroy(U* p) {
      p->~U();
    }

    // 分配器重绑定
    template <typename U>
    struct rebind {
      using other = polymorphic_allocator<U>;
    };

    // 获取底层资源
    std::pmr::memory_resource* resource() const noexcept {
      return m_mr;
    }

    // 比较运算符
    template <typename U>
    bool operator==(const polymorphic_allocator<U>& other) const noexcept {
      return m_mr == other.resource();
    }

    template <typename U>
    bool operator!=(const polymorphic_allocator<U>& other) const noexcept {
      return !(*this == other);
    }
  };

}
//int main() {
//  // 1. 非线程安全版本（无锁，性能最优）
//  stdex::unsynchronized_pool_resource unsync_pool;
//  stdex::polymorphic_allocator<int> unsync_alloc(&unsync_pool);
//  std::vector<int, stdex::polymorphic_allocator<int>> vec1(unsync_alloc);
//  for (int i = 0; i < 1000; ++i) {
//    vec1.push_back(i);
//  }
//
//  // 2. 线程安全版本（细粒度锁，多线程安全）
//  stdex::synchronized_pool_resource sync_pool;
//  stdex::polymorphic_allocator<std::string> sync_alloc(&sync_pool);
//  std::vector<std::string, stdex::polymorphic_allocator<std::string>> vec2(sync_alloc);
//  vec2.emplace_back("hello");
//
//  return 0;
//}