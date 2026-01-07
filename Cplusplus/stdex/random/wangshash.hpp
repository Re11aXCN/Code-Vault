#pragma once

#include <cstdint>
#include <random>

namespace stdex {
// Wang's hash algorithm - a fast, high-quality hash function
// Excellent for parallel scenarios due to its stateless nature
//
// Usage:
//   WangsHash rng(42);  // seed
//   uint32_t value = rng();  // generate random number
//
// With distribution:
//   std::vector<int> vec(100);
//   std::generate(vec.begin(), vec.end(),
//       [rng = WangsHash(0), dist = std::uniform_int_distribution<int>(0,
//       100)]() mutable {
//           return dist(rng);
//       });

struct WangsHash
{
  uint32_t state;

  explicit WangsHash(std::size_t seed = 0) : state(static_cast<uint32_t>(seed))
  {
  }

  using result_type = uint32_t;

  constexpr uint32_t operator() () noexcept
  {
    uint32_t x = state;
    x          = (x ^ 61) ^ (x >> 16);
    x *= 9;
    x = x ^ (x >> 4);
    x *= 0x27'D4'EB'2D;
    x            = x ^ (x >> 15);
    return state = x;
  }

  static constexpr uint32_t min() noexcept { return 0; }

  static constexpr uint32_t max() noexcept { return UINT32_MAX; }
};
}  // namespace stdex

/*
// Usage Examples:

#include <algorithm>
#include <iostream>
#include <numeric>
#include <random>
#include <vector>

int main() {
    // Basic usage
    stdex::WangsHash rng(42);
    std::cout << "Random value: " << rng() << std::endl;

    // Generate random vector
    std::vector<int> vec(100);
    std::generate(vec.begin(), vec.end(),
        [rng = stdex::WangsHash(0), dist = std::uniform_int_distribution<int>(0,
100)]() mutable { return dist(rng);
        });

    // Generate with back_inserter
    std::vector<int> vec2;
    vec2.reserve(100);
    std::generate_n(std::back_inserter(vec2), 100,
        [rng = stdex::WangsHash(0), dist = std::uniform_int_distribution<int>(0,
100)]() mutable { return dist(rng);
        });

    // Shuffle existing vector
    std::vector<int> numbers(100);
    std::iota(numbers.begin(), numbers.end(), 0);
    stdex::WangsHash shuffler(std::random_device{}());
    std::shuffle(numbers.begin(), numbers.end(), shuffler);

    return 0;
}
*/
