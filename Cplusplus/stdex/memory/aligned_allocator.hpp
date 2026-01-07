#pragma once

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <stdexcept>
#include <type_traits>
#include <utility>
#if defined(_WIN32)
#  include <malloc.h>
#endif
namespace stdex {

// https://stackoverflow.com/questions/12942548/making-stdvector-allocate-aligned-memory
inline void* allocate_aligned_memory(size_t align, size_t size)
{
#if defined(_WIN32)
  return _aligned_malloc(size, align);
#else
  return std::aligned_alloc(align, size);
#endif
}

inline void deallocate_aligned_memory(void* pointer) noexcept
{
#if defined(_WIN32)
  _aligned_free(pointer);
#else
  std::free(pointer);
#endif
}

enum class MemAlignPolicy : size_t
{
  SystemDefault = sizeof(void*),
  _4bytes       = 4,
  _8bytes       = 8,
  _16bytes      = 16,
  _32bytes      = 32,
  _64bytes      = 64,
  _128bytes     = 128,
  _256bytes     = 256,
  _512bytes     = 512,
  _1024bytes    = 1024,
};

template<typename Type, MemAlignPolicy Align = MemAlignPolicy::_64bytes>
class AlignedAllocator;

template<MemAlignPolicy Align>
class AlignedAllocator<void, Align>
{
public:

  typedef void*       pointer;
  typedef const void* const_pointer;
  typedef void        value_type;

  template<class OtherType>
  struct rebind
  {
    typedef AlignedAllocator<OtherType, Align> other;
  };
};

template<typename Type, MemAlignPolicy Align>
class AlignedAllocator
{
public:

  typedef Type        value_type;
  typedef Type*       pointer;
  typedef const Type* const_pointer;
  typedef Type&       reference;
  typedef const Type& const_reference;
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;

  typedef std::true_type propagate_on_container_move_assignment;

  template<class OtherType>
  struct rebind
  {
    typedef AlignedAllocator<OtherType, Align> other;
  };

public:

  AlignedAllocator() noexcept { }

  template<class OtherType>
  AlignedAllocator(const AlignedAllocator<OtherType, Align>&) noexcept
  {
  }

  size_type max_size() const noexcept
  {
    return (size_type(~0) - size_type(Align)) / sizeof(Type);
  }

  pointer address(reference x) const noexcept { return std::addressof(x); }

  const_pointer address(const_reference x) const noexcept
  {
    return std::addressof(x);
  }

  pointer allocate(size_type count,
                   typename AlignedAllocator<void, Align>::const_pointer = 0)
  {
    constexpr size_type alignment = static_cast<size_type>(Align);
    void* pointer = allocate_aligned_memory(alignment, count * sizeof(Type));
    if (pointer == nullptr) { throw std::bad_alloc(); }

    return reinterpret_cast<pointer>(pointer);
  }

  void deallocate(pointer pointer, size_type) noexcept
  {
    return deallocate_aligned_memory(pointer);
  }

  template<class OtherType, class... Arguments>
  void construct(OtherType* pointer, Arguments&&... arguments)
  {
    ::new (reinterpret_cast<void*>(pointer))
        OtherType(std::forward<Arguments>(arguments)...);
  }

  void destroy(pointer pointer) { pointer->~Type(); }
};

template<typename Type, MemAlignPolicy Align>
class AlignedAllocator<const Type, Align>
{
public:

  typedef Type        value_type;
  typedef const Type* pointer;
  typedef const Type* const_pointer;
  typedef const Type& reference;
  typedef const Type& const_reference;
  typedef size_t      size_type;
  typedef ptrdiff_t   difference_type;

  typedef std::true_type propagate_on_container_move_assignment;

  template<class OtherType>
  struct rebind
  {
    typedef AlignedAllocator<OtherType, Align> other;
  };

public:

  AlignedAllocator() noexcept { }

  template<class OtherType>
  AlignedAllocator(const AlignedAllocator<OtherType, Align>&) noexcept
  {
  }

  size_type max_size() const noexcept
  {
    return (size_type(~0) - size_type(Align)) / sizeof(Type);
  }

  const_pointer address(const_reference x) const noexcept
  {
    return std::addressof(x);
  }

  pointer allocate(size_type count,
                   typename AlignedAllocator<void, Align>::const_pointer = 0)
  {
    constexpr size_type alignment = static_cast<size_type>(Align);
    void* pointer = allocate_aligned_memory(alignment, count * sizeof(Type));
    if (pointer == nullptr) { throw std::bad_alloc(); }

    return reinterpret_cast<pointer>(pointer);
  }

  void deallocate(pointer pointer, size_type) noexcept
  {
    return deallocate_aligned_memory(pointer);
  }

  template<class OtherType, class... Arguments>
  void construct(OtherType* pointer, Arguments&&... arguments)
  {
    ::new (reinterpret_cast<void*>(pointer))
        OtherType(std::forward<Arguments>(arguments)...);
  }

  void destroy(pointer pointer) { pointer->~Type(); }
};

template<typename Type, MemAlignPolicy TAlign, typename OtherType,
         MemAlignPolicy UAlign>
inline bool operator== (const AlignedAllocator<Type, TAlign>&,
                        const AlignedAllocator<OtherType, UAlign>&) noexcept
{
  return TAlign == UAlign;
}

template<typename Type, MemAlignPolicy TAlign, typename OtherType,
         MemAlignPolicy UAlign>
inline bool operator!= (const AlignedAllocator<Type, TAlign>&,
                        const AlignedAllocator<OtherType, UAlign>&) noexcept
{
  return TAlign != UAlign;
}

}  // namespace stdex

/*
// ========== Usage Examples ==========

#include "aligned_allocator.hpp"
#include <vector>
#include <iostream>
#include <iomanip>
#include <string>

// Helper function: verify pointer alignment
template <typename T>
void check_alignment(const T* ptr, size_t expected_alignment, const std::string&
desc) { uintptr_t addr = reinterpret_cast<uintptr_t>(ptr); bool is_aligned =
(addr % expected_alignment) == 0;

    std::cout << desc << std::endl;
    std::cout << "  Pointer address: 0x" << std::hex << addr << std::dec <<
std::endl; std::cout << "  Expected alignment: " << expected_alignment << "
bytes" << std::endl; std::cout << "  Actual alignment: " << (is_aligned ? "OK" :
"FAILED") <<
"\n" << std::endl;
}

// Test custom type (verify construct/destroy work properly)
struct TestData {
    int id;
    double value;

    TestData(int id_, double value_) : id(id_), value(value_) {
        std::cout << "TestData constructed: id=" << id << ", value=" << value <<
std::endl;
    }

    ~TestData() {
        std::cout << "TestData destructed: id=" << id << std::endl;
    }
};

int main() {
    try {
        // ===================== Example 1: Basic type + 64-byte alignment
        // =====================
        std::cout << "=== Example 1: int type, 64-byte alignment ===" <<
std::endl;
        // Use AlignedAllocator as vector's allocator
        std::vector<int, AlignedAllocator<int, MemAlignPolicy::_64bytes>>
vec_64;

        // Fill data
        for (int i = 0; i < 10; ++i) {
            vec_64.push_back(i * 10);
        }

        // Verify memory alignment
        if (!vec_64.empty()) {
            check_alignment(vec_64.data(), 64, "vec_64 alignment check");
        }

        // Output data
        std::cout << "vec_64 contents: ";
        for (int val : vec_64) {
            std::cout << val << " ";
        }
        std::cout << "\n" << std::endl;

        // ===================== Example 2: Custom type + 128-byte alignment
        // =====================
        std::cout << "=== Example 2: Custom type, 128-byte alignment ===" <<
std::endl; std::vector<TestData, AlignedAllocator<TestData,
MemAlignPolicy::_128bytes>> vec_128;

        // emplace_back calls allocator's construct method
        vec_128.emplace_back(1, 3.14159);
        vec_128.emplace_back(2, 2.71828);

        // Verify memory alignment
        if (!vec_128.empty()) {
            check_alignment(vec_128.data(), 128, "vec_128 alignment check");
        }

        // ===================== Example 3: System default alignment
        // =====================
        std::cout << "=== Example 3: double type, system default alignment ==="
<< std::endl; std::vector<double, AlignedAllocator<double,
MemAlignPolicy::SystemDefault>> vec_default; vec_default.resize(5, 9.9);

        if (!vec_default.empty()) {
            size_t sys_align =
static_cast<size_t>(MemAlignPolicy::SystemDefault);
            check_alignment(vec_default.data(), sys_align, "vec_default
alignment check");
        }

    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}

*/