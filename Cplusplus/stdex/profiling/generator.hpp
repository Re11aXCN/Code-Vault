#pragma once

#include <array>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <random>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace stdex {
// ============================================================================
// Generator Concepts and Traits
// ============================================================================

namespace detail {
template<typename T>
struct is_character : std::false_type
{ };

template<>
struct is_character<char> : std::true_type
{ };

template<>
struct is_character<wchar_t> : std::true_type
{ };

template<>
struct is_character<char8_t> : std::true_type
{ };

template<>
struct is_character<char16_t> : std::true_type
{ };

template<>
struct is_character<char32_t> : std::true_type
{ };

template<typename T>
constexpr bool is_character_v = is_character<T>::value;

}  // namespace detail

// ============================================================================
// Random Data Generation
// ============================================================================

namespace detail {
// Helper to generate random values
template<typename T, typename Gen>
T generate_random_value(Gen& gen, T min_val, T max_val)
{
  if constexpr (std::is_integral_v<T>) {
    if constexpr (sizeof(T) == 1) {
      std::uniform_int_distribution<int> dist(static_cast<int>(min_val),
                                              static_cast<int>(max_val));
      return static_cast<T>(dist(gen));
    } else {
      std::uniform_int_distribution<T> dist(min_val, max_val);
      return dist(gen);
    }
  } else if constexpr (std::is_floating_point_v<T>) {
    std::uniform_real_distribution<T> dist(min_val, max_val);
    return dist(gen);
  } else {
    static_assert(std::is_arithmetic_v<T>,
                  "Type must be arithmetic for random generation");
  }
}

// Generate random string
template<typename Gen>
std::string generate_random_string(
    Gen& gen, std::size_t length,
    const char* charset =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789")
{
  std::uniform_int_distribution<std::size_t> dist(
      0, std::char_traits<char>::length(charset) - 1);
  std::string result;
  result.reserve(length);
  for (std::size_t i = 0; i < length; ++i) {
    result += charset [dist(gen)];
  }
  return result;
}
}  // namespace detail

// Generate random data into a container
template<template<typename...> class Container, typename T, typename... Args>
Container<T> generate_random_data(std::size_t count, T min_val, T max_val,
                                  std::size_t seed = std::random_device {}())
{
  Container<T> result;
  result.reserve(count);

  std::mt19937 gen(seed);

  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(detail::generate_random_value<T>(gen, min_val, max_val));
  }

  return result;
}

// Generate random data into std::vector (convenience overload)
template<typename T>
std::vector<T> generate_random(std::size_t count, T min_val, T max_val,
                               std::size_t seed = std::random_device {}())
{
  return generate_random_data<std::vector, T>(count, min_val, max_val, seed);
}

// Generate random sequence as std::array
template<std::size_t N, typename T>
std::array<T, N>
generate_random_array(T min_val, T max_val,
                      std::size_t seed = std::random_device {}())
{
  std::array<T, N> result {};
  std::mt19937     gen(seed);

  for (std::size_t i = 0; i < N; ++i) {
    result [i] = detail::generate_random_value<T>(gen, min_val, max_val);
  }

  return result;
}

// Generate random strings
inline std::vector<std::string> generate_random_strings(
    std::size_t count, std::size_t length,
    const char* charset =
        "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789",
    std::size_t seed = std::random_device {}())
{
  std::vector<std::string> result;
  result.reserve(count);

  std::mt19937 gen(seed);
  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(detail::generate_random_string(gen, length, charset));
  }

  return result;
}

// ============================================================================
// Sequential Data Generation
// ============================================================================

// Generate sequential data: start, start+1, start+2, ...
template<typename T = std::size_t>
std::vector<T> generate_sequence(std::size_t count, T start = T { 0 })
{
  std::vector<T> result;
  result.reserve(count);
  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(static_cast<T>(start + i));
  }
  return result;
}

// Generate sequence with custom step
template<typename T>
std::vector<T> generate_sequence(std::size_t count, T start, T step)
{
  std::vector<T> result;
  result.reserve(count);
  T current = start;
  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(current);
    current += step;
  }
  return result;
}

// ============================================================================
// Pattern-Based Generation
// ============================================================================

// Generate repeated pattern
template<typename T>
std::vector<T> generate_pattern(std::size_t           count,
                                const std::vector<T>& pattern)
{
  std::vector<T> result;
  result.reserve(count);

  if (pattern.empty()) { return result; }

  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(pattern [i % pattern.size()]);
  }

  return result;
}

// Generate repeated pattern (initializer_list version)
template<typename T>
std::vector<T> generate_pattern(std::size_t              count,
                                std::initializer_list<T> pattern)
{
  return generate_pattern(count, std::vector<T>(pattern));
}

// ============================================================================
// Custom Callable-Based Generation
// ============================================================================

// Generate data using a custom callable
template<typename T, typename Func>
std::vector<T> generate_custom(std::size_t count, Func&& func)
{
  std::vector<T> result;
  result.reserve(count);

  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(func(i));
  }

  return result;
}

// Generate data with index-aware callable
template<typename T, typename Func>
std::vector<T> generate_with_index(std::size_t count, Func&& func)
{
  std::vector<T> result;
  result.reserve(count);

  for (std::size_t i = 0; i < count; ++i) {
    result.push_back(func(i));
  }

  return result;
}

// ============================================================================
// Complex Type Generation
// ============================================================================

// Generate pairs
template<typename T1, typename T2>
std::vector<std::pair<T1, T2>> generate_pairs(std::size_t count)
{
  return generate_custom<std::pair<T1, T2>>(count, [](std::size_t i) {
    return std::make_pair(static_cast<T1>(i), static_cast<T2>(i));
  });
}

// Generate tuples (homogeneous)
template<typename T, std::size_t N>
std::vector<std::array<T, N>> generate_tuple_arrays(std::size_t count,
                                                    T           start = T { 0 })
{
  return generate_custom<std::array<T, N>>(count, [start](std::size_t i) {
    std::array<T, N> result {};
    for (std::size_t j = 0; j < N; ++j) {
      result [j] = static_cast<T>(start + i * N + j);
    }
    return result;
  });
}

// ============================================================================
// File I/O for Generated Data
// ============================================================================

// Serialize container data to file
template<typename T>
void serialize_data(const std::vector<std::vector<T>>& data,
                    const std::string&                 filename)
{
  std::ofstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  for (const auto& vec : data) {
    file << "[";
    for (std::size_t i = 0; i < vec.size(); ++i) {
      file << vec [i];
      if (i != vec.size() - 1) { file << ","; }
    }
    file << "]\n";
  }

  file.close();
}

// Deserialize container data from file
template<typename T>
std::vector<std::vector<T>> deserialize_data(const std::string& filename)
{
  std::vector<std::vector<T>> result;

  std::ifstream file(filename);
  if (!file.is_open()) {
    throw std::runtime_error("Failed to open file: " + filename);
  }

  std::string line;
  while (std::getline(file, line)) {
    if (line.empty()) continue;

    // Remove brackets
    line = line.substr(1, line.length() - 2);

    std::vector<T>    vec;
    std::stringstream ss(line);
    std::string       token;

    while (std::getline(ss, token, ',')) {
      std::stringstream converter(token);
      T                 value;
      converter >> value;
      vec.push_back(value);
    }

    result.push_back(std::move(vec));
  }

  file.close();
  return result;
}

// Convenience overload for filesystem::path
template<typename T>
std::vector<std::vector<T>>
deserialize_data(const std::filesystem::path& filepath)
{
  return deserialize_data<T>(filepath.string());
}

// ============================================================================
// Utility Functions
// ============================================================================

// Shuffle a container using Fisher-Yates algorithm
template<typename T>
void shuffle(std::vector<T>& vec, std::size_t seed = std::random_device {}())
{
  std::mt19937 gen(seed);
  std::shuffle(vec.begin(), vec.end(), gen);
}

// Sample N elements from a container without replacement
template<typename T>
std::vector<T> sample(const std::vector<T>& source, std::size_t count,
                      std::size_t seed = std::random_device {}())
{
  if (count > source.size()) {
    throw std::invalid_argument("Sample count cannot exceed source size");
  }

  std::mt19937   gen(seed);
  std::vector<T> result = source;

  // Partial Fisher-Yates shuffle
  for (std::size_t i = 0; i < count; ++i) {
    std::uniform_int_distribution<std::size_t> dist(i, result.size() - 1);
    std::swap(result [i], result [dist(gen)]);
  }

  result.resize(count);
  return result;
}
}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <vector>
#include <string>

int main() {
    using namespace stdex;

    // Example 1: Generate random integers
    std::cout << "=== Random Integers ===\n";
    auto random_ints = generate_random<int>(10, 0, 100);
    for (auto val : random_ints) std::cout << val << " ";
    std::cout << "\n\n";

    // Example 2: Generate random floating point numbers
    std::cout << "=== Random Doubles ===\n";
    auto random_doubles = generate_random<double>(5, -1.0, 1.0);
    for (auto val : random_doubles) std::cout << val << " ";
    std::cout << "\n\n";

    // Example 3: Generate random strings
    std::cout << "=== Random Strings ===\n";
    auto random_strings = generate_random_strings(3, 8);
    for (const auto& str : random_strings) std::cout << str << "\n";
    std::cout << "\n";

    // Example 4: Generate sequential data
    std::cout << "=== Sequential ===\n";
    auto seq = generate_sequence<int>(10, 1);
    for (auto val : seq) std::cout << val << " ";
    std::cout << "\n\n";

    // Example 5: Generate sequence with step
    std::cout << "=== Sequential with Step ===\n";
    auto seq_step = generate_sequence(10, 0, 5);
    for (auto val : seq_step) std::cout << val << " ";
    std::cout << "\n\n";

    // Example 6: Generate repeated pattern
    std::cout << "=== Pattern ===\n";
    auto pattern = generate_pattern(10, {1, 2, 3});
    for (auto val : pattern) std::cout << val << " ";
    std::cout << "\n\n";

    // Example 7: Generate with custom callable
    std::cout << "=== Custom (squares) ===\n";
    auto squares = generate_custom<int>(10, [](std::size_t i) { return i * i;
}); for (auto val : squares) std::cout << val << " "; std::cout << "\n\n";

    // Example 8: Generate pairs
    std::cout << "=== Pairs ===\n";
    auto pairs = generate_pairs<int, long>(5);
    for (const auto& [a, b] : pairs) {
        std::cout << "(" << a << "," << b << ") ";
    }
    std::cout << "\n\n";

    // Example 9: Generate tuple arrays
    std::cout << "=== Tuple Arrays ===\n";
    auto tuples = generate_tuple_arrays<int, 3>(3);
    for (const auto& arr : tuples) {
        std::cout << "[";
        for (auto val : arr) std::cout << val << " ";
        std::cout << "] ";
    }
    std::cout << "\n\n";

    // Example 10: Shuffle and sample
    std::cout << "=== Shuffle & Sample ===\n";
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    shuffle(data, 42);  // Fixed seed for reproducibility
    std::cout << "Shuffled: ";
    for (auto val : data) std::cout << val << " ";
    std::cout << "\n";

    auto sampled = sample(data, 5, 42);
    std::cout << "Sampled: ";
    for (auto val : sampled) std::cout << val << " ";
    std::cout << "\n";

    return 0;
}
*/
