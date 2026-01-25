#include <algorithm>
#include <chrono>
#include <iostream>
#include <random>
#include <vector>

#include "algorithm/radix_sort.hpp"

template<typename T>
void test_with_policies(const std::string& name, size_t size)
{
  std::cout << "\n=== Testing " << name << " ===" << std::endl;

  std::vector<T> data(size);

  std::mt19937_64 rng(42);
  if constexpr (std::is_integral_v<T>) {
    std::uniform_int_distribution<T> dist(std::numeric_limits<T>::min(),
                                          std::numeric_limits<T>::max());
    for (auto& val : data) {
      val = dist(rng);
    }
  } else {
    std::uniform_real_distribution<T> dist(-1000.0, 1000.0);
    for (auto& val : data) {
      val = dist(rng);
    }
  }

  std::vector<std::pair<std::string, bool>> results;

  auto test_policy = [&](const std::string& policy_name, auto policy) {
    std::vector<T> test_data = data;
    auto           start     = std::chrono::high_resolution_clock::now();
    stdex::radix_sort(policy, test_data.begin(), test_data.end());
    auto end     = std::chrono::high_resolution_clock::now();
    bool correct = std::is_sorted(test_data.begin(), test_data.end());
    auto duration =
        std::chrono::duration_cast<std::chrono::microseconds>(end - start);

    std::cout << "  " << policy_name << ": " << (correct ? "PASS" : "FAIL")
              << " (" << duration.count() << " us)" << std::endl;

    results.push_back({ policy_name, correct });
  };

  test_policy("seq", std::execution::seq);
  test_policy("par", std::execution::par);
  test_policy("unseq", std::execution::unseq);
  test_policy("par_unseq", std::execution::par_unseq);

  bool all_passed = std::all_of(results.begin(), results.end(),
                                [](const auto& r) { return r.second; });

  if (!all_passed) {
    std::cout << "  WARNING: Some policies failed!" << std::endl;
  }
}

int main()
{
  std::cout << "=== SIMD Detection ===" << std::endl;
  std::cout << "Has AVX2: " << stdex::simd::has_avx2() << std::endl;
  std::cout << "Architecture: "
            << stdex::simd::to_string(stdex::simd::get_architecture())
            << std::endl;
  std::cout << "Instruction Set: "
            << stdex::simd::to_string(stdex::simd::get_instruction_set())
            << std::endl;

  std::cout << "\n=== Policy Tests ===" << std::endl;
  test_with_policies<int32_t>("int32_t", 100000);
  test_with_policies<uint32_t>("uint32_t", 100000);
  test_with_policies<int64_t>("int64_t", 100000);
  test_with_policies<uint64_t>("uint64_t", 100000);
  test_with_policies<float>("float", 100000);
  test_with_policies<double>("double", 100000);

  return 0;
}