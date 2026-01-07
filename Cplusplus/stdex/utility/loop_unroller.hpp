#pragma once

#include <cstddef>
#include <type_traits>
#include <utility>

namespace stdex {
// Compile-time loop unroller for performance optimization
// Unrolls loops by a fixed factor to reduce branch overhead
template<std::size_t UnrollFactor>
class LoopUnroller
{
public:

  // Basic loop unrolling - execute function N times
  template<typename Func, typename... Arguments>
  static void execute(Func&& function, Arguments&&... arguments)
  {
    if constexpr (UnrollFactor > 0) {
      [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
        ((void)Indices, ...,
         std::forward<Func>(function)(std::forward<Arguments>(arguments)...));
      }(std::make_index_sequence<UnrollFactor> {});
    }
  }

  // Loop unrolling with index - execute function N times with compile-time
  // index
  template<typename Func>
  static void execute_with_index(Func&& function)
  {
    if constexpr (UnrollFactor > 0) {
      [&]<std::size_t... Indices>(std::index_sequence<Indices...>) {
        (std::forward<Func>(function)(Indices), ...);
      }(std::make_index_sequence<UnrollFactor> {});
    }
  }

  // Process array with unrolled loop
  // Handles arrays that may not be evenly divisible by unroll factor
  template<typename Type, typename Func>
  static void process_array(Type* data, std::size_t size, Func&& function)
  {
    constexpr std::size_t step_size = UnrollFactor;
    std::size_t           index     = 0;

    // Process elements in chunks of UnrollFactor
    for (; index + step_size <= size; index += step_size) {
      execute_with_index(
          [&](std::size_t offset) { function(data [index + offset]); });
    }

    // Process remaining elements
    for (; index < size; ++index) {
      function(data [index]);
    }
  }
};

// Convenience function for unrolling with index
template<std::size_t N, typename Func>
void unroll_with_index(Func&& function)
{
  LoopUnroller<N>::execute_with_index(std::forward<Func>(function));
}

// Convenience function for basic unrolling
template<std::size_t N, typename Func, typename... Arguments>
void unroll(Func&& function, Arguments&&... arguments)
{
  LoopUnroller<N>::execute(std::forward<Func>(function),
                           std::forward<Arguments>(arguments)...);
}
}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <vector>

int main() {
    using namespace stdex;

    // Example 1: Basic loop unrolling
    std::cout << "Basic loop unrolling:\n";
    LoopUnroller<4>::execute([](auto) {
        std::cout << "Hello ";
    });
    std::cout << "\n\n";

    // Example 2: Loop unrolling with index
    std::cout << "Loop unrolling with index:\n";
    LoopUnroller<4>::execute_with_index([](std::size_t index) {
        std::cout << "Index: " << index << " ";
    });
    std::cout << "\n\n";

    // Example 3: Array processing
    std::vector<int> vector = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::cout << "Array processing:\n";
    LoopUnroller<4>::process_array(vector.data(), vector.size(), [](int& value)
    {
      std::cout << value * 2 << " ";
    });
    std::cout << "\n\n";

    // Example 4: Using unroll_with_index convenience function
    std::cout << "Using unroll_with_index:\n";
    unroll_with_index<5>([](std::size_t index) {
        std::cout << "Square[" << index << "] = " << index * index << "\n";
    });

    return 0;
}
*/
