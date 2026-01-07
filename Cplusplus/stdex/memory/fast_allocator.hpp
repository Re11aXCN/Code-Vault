#pragma once

#include <cstddef>
#include <memory>
#include <new>
#include <type_traits>
#include <utility>

namespace stdex {

template<class Type = std::byte, std::size_t Align = 64, bool Pod = true>
struct FastAllocator
{
  /* cacheline-aligned and non-zero-initialized FastAllocator for std::vector
   */
  using value_type                             = Type;
  using size_type                              = std::size_t;
  using propagate_on_container_move_assignment = std::true_type;

  template<class OtherType>
  struct rebind
  {
    using other = FastAllocator<OtherType, Align, Pod>;
  };

  FastAllocator() = default;

  template<class OtherType = Type>
  static OtherType* allocate(size_type count)
  {
    count *= sizeof(OtherType);
    return reinterpret_cast<OtherType*>(
        ::operator new (count, std::align_val_t(Align)));
  }

  template<class OtherType = Type>
  static void deallocate(OtherType* pointer, size_type = 0)
  {
    ::operator delete (reinterpret_cast<void*>(pointer),
                       std::align_val_t(Align));
  }

  template<class OtherType, class... Arguments>
  constexpr static void
  construct(OtherType* pointer, Arguments&&... arguments) noexcept(
      std::is_nothrow_constructible_v<Type, Arguments...>)
  {
    if constexpr (!(Pod && std::is_pod_v<Type> && sizeof...(Arguments) == 0))
      ::new ((void*)pointer) Type(std::forward<Arguments>(arguments)...);
  }

  template<class OtherType, std::size_t UAlign, bool UPod>
  constexpr bool
  operator== (FastAllocator<OtherType, UAlign, UPod> const&) noexcept
  {
    return Align == UAlign && Pod == UPod;
  }

  template<class OtherType, std::size_t UAlign, bool UPod>
  constexpr bool
  operator!= (FastAllocator<OtherType, UAlign, UPod> const&) noexcept
  {
    return Align != UAlign || Pod != UPod;
  }
};

}  // namespace stdex

/*
// ========== Usage Examples ==========

#include "fast_allocator.hpp"  // Assuming allocator code is saved in this
header #include <vector> #include <iostream> #include <iomanip> #include
<string> #include <type_traits>

// Helper function: verify pointer alignment
template <typename T>
void check_alignment(const T* ptr, std::size_t expected_alignment, const
std::string& desc) { uintptr_t addr = reinterpret_cast<uintptr_t>(ptr); bool
is_aligned = (addr % expected_alignment) == 0;

    std::cout << "[" << desc << "]" << std::endl;
    std::cout << "  Pointer address: 0x" << std::hex << std::setw(16) <<
std::fill('0')
<< addr << std::dec << std::endl; std::cout << "  Expected alignment: " <<
expected_alignment << " bytes" << std::endl; std::cout << "  Actual alignment: "
<< (is_aligned ? "OK" : "FAILED") << "\n" << std::endl;
}

// Test POD type (Plain Old Data)
struct PODData {
    int id;
    double value;
    char name[16];
    // No custom constructor/destructor/copy functions, meets POD
characteristics
};

// Test non-POD type
struct NonPODData {
    int id;
    double value;

    // Custom constructor makes it non-POD
    NonPODData(int id_ = 0, double value_ = 0.0) : id(id_), value(value_) {
        std::cout << "  NonPODData constructed: id=" << id << ", value=" <<
value << std::endl;
    }

    // Custom destructor
    ~NonPODData() {
        std::cout << "  NonPODData destructed: id=" << id << std::endl;
    }

    // Disable copy (optional, just for demonstration)
    NonPODData(const NonPODData&) = default;
    NonPODData& operator=(const NonPODData&) = default;
};

int main() {
    try {
        // ===================== Example 1: Basic type + 64-byte alignment
(Pod=true by default)
        // =====================
        std::cout << "=== Example 1: int type, 64-byte alignment, Pod=true ==="
<< std::endl;
        // Use default template parameters: Type=int, Align=64, Pod=true
        std::vector<int, FastAllocator<int, 64, true>> vec_int;

        // Fill data
        for (int i = 0; i < 5; ++i) {
            vec_int.push_back(i * 100);
        }

        // Verify alignment
        if (!vec_int.empty()) {
            check_alignment(vec_int.data(), 64, "int vector alignment check");
        }

        // Output data
        std::cout << "  vec_int contents: ";
        for (int val : vec_int) {
            std::cout << val << " ";
        }
        std::cout << "\n" << std::endl;

        // ===================== Example 2: POD type + 128-byte alignment
        // =====================
        std::cout << "=== Example 2: PODData type, 128-byte alignment, Pod=true
===" << std::endl; static_assert(std::is_pod_v<PODData>, "PODData should be POD
type"); std::vector<PODData, FastAllocator<PODData, 128, true>> vec_pod;

        // Resize (no-arg constructor, won't call constructor when Pod=true)
        vec_pod.resize(2);
        // Manual assignment
        vec_pod[0] = {1, 3.14159, "data1"};
        vec_pod[1] = {2, 2.71828, "data2"};

        // Verify alignment
        if (!vec_pod.empty()) {
            check_alignment(vec_pod.data(), 128, "PODData vector alignment
check");
        }
        std::cout << "  PODData[0]: id=" << vec_pod[0].id << ", value=" <<
vec_pod[0].value << "\n" << std::endl;

        // ===================== Example 3: Non-POD type + 32-byte alignment
(Pod=false)
        // =====================
        std::cout << "=== Example 3: NonPODData type, 32-byte alignment,
Pod=false ===" << std::endl; static_assert(!std::is_pod_v<NonPODData>,
"NonPODData should not be POD type"); std::vector<NonPODData,
FastAllocator<NonPODData, 32, false>> vec_nonpod;

        // emplace_back triggers constructor (when Pod=false)
        vec_nonpod.emplace_back(10, 99.9);
        vec_nonpod.emplace_back(20, 88.8);

        // Verify alignment
        if (!vec_nonpod.empty()) {
            check_alignment(vec_nonpod.data(), 32,
"NonPODData vector alignment check");
        }

        // ===================== Example 4: Compare Pod=true/false behavior
        // =====================
        std::cout << "\n=== Example 4: Compare Pod=true/false construction
behavior ===" << std::endl;
        // Pod=true: won't call constructor (even for non-POD types)
        std::cout << "  --- Pod=true scenario (no construction) ---" <<
std::endl; std::vector<NonPODData, FastAllocator<NonPODData, 64, true>>
vec_pod_true; vec_pod_true.resize(1);  // No construction log output

        // Pod=false: will call constructor
        std::cout << "  --- Pod=false scenario (with construction) ---" <<
std::endl; std::vector<NonPODData, FastAllocator<NonPODData, 64, false>>
vec_pod_false; vec_pod_false.resize(1);  // Will output construction log

    } catch (const std::bad_alloc& e) {
        std::cerr << "Memory allocation failed: " << e.what() << std::endl;
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
        return 1;
    }

    std::cout << "\n=== Program end (NonPODData destructor triggered here) ==="
<< std::endl; return 0;
}

*/