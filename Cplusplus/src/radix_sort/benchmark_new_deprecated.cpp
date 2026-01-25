#include <benchmark/benchmark.h>
#include <omp.h>

#include <algorithm>
#include <execution>
#include <iostream>
#include <random>
#include <vector>

#include "algorithm/radix_sort.hpp"
#include "profiling/generator.hpp"

namespace deprecated {
#include "radix_sort/deprecated_radix_sort.hpp"
}

template<typename T>
struct OldRadixSortWrapper
{
  template<typename Iter, typename Compare, typename Extractor>
  void operator() (Iter first, Iter last, Compare comp, Extractor&& extractor)
  {
    deprecated::radix_sort(std::execution::seq, first, last, comp,
                           std::forward<Extractor>(extractor));
  }
};

template<typename T>
struct NewRadixSortWrapper
{
  template<typename Iter, typename Compare, typename Extractor>
  void operator() (Iter first, Iter last, Compare comp, Extractor&& extractor)
  {
    using Value_t = typename std::iterator_traits<Iter>::value_type;
    if constexpr (std::is_same_v<Compare, std::less<>>) {
      stdex::radix_sort(first, last);
    } else {
      using KeyExtractor_t = std::decay_t<Extractor>;
      constexpr auto params =
          stdex::Radix_template_params<KeyExtractor_t, std::uint32_t> {
            .Order = stdex::SortOrder::Descending
          };
      stdex::radix_sort<Iter, params>(first, last);
    }
  }
};

template<typename T>
struct OldRadixSortParallelWrapper
{
  template<typename Iter, typename Compare, typename Extractor>
  void operator() (Iter first, Iter last, Compare comp, Extractor&& extractor)
  {
    deprecated::radix_sort(std::execution::par, first, last, comp,
                           std::forward<Extractor>(extractor));
  }
};

template<typename T>
struct NewRadixSortParallelWrapper
{
  template<typename Iter, typename Compare, typename Extractor>
  void operator() (Iter first, Iter last, Compare comp, Extractor&& extractor)
  {
    using Value_t = typename std::iterator_traits<Iter>::value_type;
    if constexpr (std::is_same_v<Compare, std::less<>>) {
      stdex::radix_sort(std::execution::par, first, last);
    } else {
      using KeyExtractor_t = std::decay_t<Extractor>;
      constexpr auto params =
          stdex::Radix_template_params<KeyExtractor_t, std::uint32_t> {
            .Order = stdex::SortOrder::Descending
          };
      stdex::radix_sort<Iter, std::execution::parallel_policy, params>(
          std::execution::par, first, last);
    }
  }
};

template<typename T, typename SortWrapper, bool Ascending = true>
void BM_Sort_Comparison(benchmark::State& state, std::size_t seed)
{
  const std::size_t size = state.range(0);

  T min_val = std::numeric_limits<T>::min();
  T max_val = std::numeric_limits<T>::max();

  if constexpr (std::is_same_v<T, float>) {
    min_val = static_cast<float>(std::numeric_limits<int32_t>::min());
    max_val = static_cast<float>(std::numeric_limits<int32_t>::max());
  } else if constexpr (std::is_same_v<T, double>) {
    min_val = static_cast<double>(std::numeric_limits<int64_t>::min());
    max_val = static_cast<double>(std::numeric_limits<int64_t>::max());
  }

  const auto& test_data = stdex::generate_random<T>(size, min_val, max_val, seed);

  for (auto _ : state) {
    state.PauseTiming();
    auto temp = test_data;
    state.ResumeTiming();

    using Iter_t  = decltype(temp.begin());
    using Value_t = T;

    if constexpr (Ascending) {
      SortWrapper {}(temp.begin(), temp.end(), std::less<> {},
                     stdex::identity_key_extractor<Value_t> {});
    } else {
      SortWrapper {}(temp.begin(), temp.end(), std::greater<> {},
                     stdex::identity_key_extractor<Value_t> {});
    }

    benchmark::DoNotOptimize(temp.data());
  }

  state.SetBytesProcessed(state.iterations() * size * sizeof(T));
  state.SetComplexityN(size);
}

template<typename T, typename SortWrapper, bool Ascending = true>
void BM_Sort_Comparison_Parallel(benchmark::State& state, std::size_t seed)
{
  const std::size_t size = state.range(0);

  T min_val = std::numeric_limits<T>::min();
  T max_val = std::numeric_limits<T>::max();

  if constexpr (std::is_same_v<T, float>) {
    min_val = static_cast<float>(std::numeric_limits<int32_t>::min());
    max_val = static_cast<float>(std::numeric_limits<int32_t>::max());
  } else if constexpr (std::is_same_v<T, double>) {
    min_val = static_cast<double>(std::numeric_limits<int64_t>::min());
    max_val = static_cast<double>(std::numeric_limits<int64_t>::max());
  }

  const auto& test_data = stdex::generate_random<T>(size, min_val, max_val, seed);

  for (auto _ : state) {
    state.PauseTiming();
    auto temp = test_data;
    state.ResumeTiming();

    using Iter_t  = decltype(temp.begin());
    using Value_t = T;

    if constexpr (Ascending) {
      SortWrapper {}(temp.begin(), temp.end(), std::less<> {},
                     stdex::identity_key_extractor<Value_t> {});
    } else {
      SortWrapper {}(temp.begin(), temp.end(), std::greater<> {},
                     stdex::identity_key_extractor<Value_t> {});
    }

    benchmark::DoNotOptimize(temp.data());
  }

  state.SetBytesProcessed(state.iterations() * size * sizeof(T));
  state.SetComplexityN(size);
}

std::size_t global_seed = 42;

BENCHMARK_CAPTURE((BM_Sort_Comparison<int32_t, OldRadixSortWrapper<int32_t>, true>),
                  "Old_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<int32_t, NewRadixSortWrapper<int32_t>, true>),
                  "New_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison<int32_t, OldRadixSortWrapper<int32_t>, false>),
    "Old_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison<int32_t, NewRadixSortWrapper<int32_t>, false>),
    "New_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<int64_t, OldRadixSortWrapper<int64_t>, true>),
                  "Old_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<int64_t, NewRadixSortWrapper<int64_t>, true>),
                  "New_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison<int64_t, OldRadixSortWrapper<int64_t>, false>),
    "Old_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison<int64_t, NewRadixSortWrapper<int64_t>, false>),
    "New_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<float, OldRadixSortWrapper<float>, true>),
                  "Old_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<float, NewRadixSortWrapper<float>, true>),
                  "New_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<float, OldRadixSortWrapper<float>, false>),
                  "Old_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<float, NewRadixSortWrapper<float>, false>),
                  "New_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<double, OldRadixSortWrapper<double>, true>),
                  "Old_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<double, NewRadixSortWrapper<double>, true>),
                  "New_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<double, OldRadixSortWrapper<double>, false>),
                  "Old_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison<double, NewRadixSortWrapper<double>, false>),
                  "New_Descending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      int32_t, OldRadixSortParallelWrapper<int32_t>, true>),
                  "Old_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      int32_t, NewRadixSortParallelWrapper<int32_t>, true>),
                  "New_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      int64_t, OldRadixSortParallelWrapper<int64_t>, true>),
                  "Old_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      int64_t, NewRadixSortParallelWrapper<int64_t>, true>),
                  "New_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison_Parallel<float, OldRadixSortParallelWrapper<float>, true>),
    "Old_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE(
    (BM_Sort_Comparison_Parallel<float, NewRadixSortParallelWrapper<float>, true>),
    "New_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      double, OldRadixSortParallelWrapper<double>, true>),
                  "Old_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_CAPTURE((BM_Sort_Comparison_Parallel<
                      double, NewRadixSortParallelWrapper<double>, true>),
                  "New_Par_Ascending", global_seed)
    ->RangeMultiplier(2)
    ->Range(1 << 10, 1 << 20)
    ->Unit(benchmark::kMillisecond)
    ->Complexity();

BENCHMARK_MAIN();