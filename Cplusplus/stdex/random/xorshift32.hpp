#pragma once

#include <cstdint>
#include <random>

namespace stdex {
// XorShift32 PRNG - fast pseudorandom number generator
// Uses XOR and shift operations for efficient random number generation
//
// Usage:
//   XorShift32 rng(std::random_device{}());
//   std::uniform_int_distribution<int> dist(0, 100);
//   int random_value = dist(rng);
//
//   std::vector<std::string> choices = {"apple", "peach", "cherry"};
//   std::uniform_int_distribution<std::size_t> index_dist(0, choices.size() -
//   1); std::string choice = choices[index_dist(rng)];

struct XorShift32
{
  uint32_t state;

  explicit XorShift32(std::size_t seed = 0)
      : state(static_cast<uint32_t>(seed + 1))
  {
  }

  using result_type = uint32_t;

  constexpr uint32_t operator() () noexcept
  {
    uint32_t x = state;
    x ^= x << 13;
    x ^= x >> 17;
    x ^= x << 5;
    return state = x;
  }

  static constexpr uint32_t min() noexcept { return 1; }

  static constexpr uint32_t max() noexcept { return UINT32_MAX; }
};
}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <random>
#include <vector>
#include <string>

int main() {
    // Basic usage with uniform distribution
    stdex::XorShift32 rng(std::random_device{}());
    std::uniform_int_distribution<int> dist(0, 100);
    int random_value = dist(rng);
    std::cout << "Random value: " << random_value << std::endl;

    // Random selection from a container
    std::vector<std::string> choices = {"apple", "peach", "cherry"};
    std::uniform_int_distribution<std::size_t> index_dist(0, choices.size() -
1); std::string choice = choices[index_dist(rng)]; std::cout << "Random choice:
" << choice << std::endl;

    return 0;
}
*/
