#pragma once

#include <chrono>
#include <iomanip>
#include <iostream>
#include <map>
#include <set>
#include <string_view>
#include <vector>

namespace stdex {
// Scope-based profiler for measuring execution time
// Records timing statistics for different code sections
class ScopeProfiler
{
public:

  using ClockType = std::chrono::high_resolution_clock;

  struct Record
  {
    const char* tag;
    int         microseconds;
  };

private:

  inline thread_local static std::vector<Record> m_Records;

  ClockType::time_point m_Begin;
  const char*           m_Tag;

  inline ScopeProfiler(const char* tag, ClockType::time_point start);
  inline void on_destroy(ClockType::time_point end);

public:

  explicit ScopeProfiler(const char* tag) : ScopeProfiler(tag, ClockType::now())
  {
  }

  ~ScopeProfiler() { on_destroy(ClockType::now()); }

  static const std::vector<Record>& get_records() { return m_Records; }

  static void print_log(std::ostream& out = std::cout);
};

ScopeProfiler::ScopeProfiler(const char* tag, ClockType::time_point start)
    : m_Begin(start), m_Tag(tag)
{
}

void ScopeProfiler::on_destroy(ClockType::time_point end)
{
  auto diff = end - m_Begin;
  int  microseconds =
      std::chrono::duration_cast<std::chrono::microseconds>(diff).count();
  m_Records.push_back({ m_Tag, microseconds });
}

void ScopeProfiler::print_log(std::ostream& out)
{
  if (m_Records.empty()) { return; }

  struct Statistic
  {
    int         max_microseconds   = 0;
    int         min_microseconds   = 0;
    int         total_microseconds = 0;
    int         count              = 0;
    const char* tag                = nullptr;
  };

  std::map<std::string_view, Statistic> statistics;

  for (const auto& [tag, microseconds] : m_Records) {
    auto& stat = statistics [tag];
    stat.total_microseconds += microseconds;
    stat.max_microseconds = std::max(stat.max_microseconds, microseconds);
    stat.min_microseconds = !stat.count
        ? microseconds
        : std::min(stat.min_microseconds, microseconds);
    stat.count++;
    stat.tag = tag;
  }

  struct StatisticCompare
  {
    using value_type = std::pair<std::string_view, Statistic>;

    bool operator() (const value_type& lhs, const value_type& rhs) const
    {
      return lhs.second.total_microseconds > rhs.second.total_microseconds;
    }
  };

  std::multiset<std::pair<std::string_view, Statistic>, StatisticCompare>
      sorted_statistics(statistics.begin(), statistics.end());

  auto format_duration = [&out](int value, int width) {
    auto threshold = 1;
    for (int i = 0; i < width - 1; ++i) {
      threshold *= 10;
    }

    if (value > threshold) {
      if (value / 1000 > threshold / 10) {
        out << std::setw(width - 1) << value / 1000000 << 'M';
      } else {
        out << std::setw(width - 1) << value / 1000 << 'k';
      }
    } else {
      out << std::setw(width) << value;
    }
  };

  out << "   avg   |   min   |   max   |  total  | cnt | tag\n";
  for (const auto& [tag, stat] : sorted_statistics) {
    format_duration(stat.total_microseconds / stat.count, 9);
    out << '|';
    format_duration(stat.min_microseconds, 9);
    out << '|';
    format_duration(stat.max_microseconds, 9);
    out << '|';
    format_duration(stat.total_microseconds, 9);
    out << '|';
    format_duration(stat.count, 5);
    out << '|';
    out << ' ' << tag << '\n';
  }
}
}  // namespace stdex

// Convenience macro for automatic scope profiling
// Uses compiler-specific function name macros
#if defined(__GNUC__) || defined(__clang__)
#  define STDEX_SCOPE_PROFILER                                 \
    stdex::ScopeProfiler _scope_profiler(__PRETTY_FUNCTION__);
#elif defined(_MSC_VER)
#  define STDEX_SCOPE_PROFILER                         \
    stdex::ScopeProfiler _scope_profiler(__FUNCSIG__);
#else
#  define STDEX_SCOPE_PROFILER stdex::ScopeProfiler _scope_profiler(__func__);
#endif
// Helper function to print profiler results
inline void print_scope_profiler(std::ostream& out = std::cout)
{
  stdex::ScopeProfiler::print_log(out);
}
// Prevent compiler from optimizing away a value
template<class T>
#if defined(_MSC_VER)
__declspec(noinline)
#elif defined(__GNUC__) || defined(__clang__)
__attribute__((noinline))
#endif
void do_not_optimize(T volatile const& value)
{
#if defined(_MSC_VER)
  (void)value;
#elif defined(__GNUC__) || defined(__clang__)
  asm volatile("" : "r"(std::addressof(value)) : "cc", "memory");
#endif
}
/*
// Usage Example:

#include "scope_profiler.hpp"
#include <thread>

void quick_function() {
    STDEX_SCOPE_PROFILER
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
}

void slow_function() {
    STDEX_SCOPE_PROFILER
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}

int main() {
    for (int i = 0; i < 5; ++i) {
        quick_function();
    }

    for (int i = 0; i < 3; ++i) {
        slow_function();
    }

    // Print profiling results
    print_scope_profiler();

    return 0;
}
*/
