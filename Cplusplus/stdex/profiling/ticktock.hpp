#pragma once

#include <chrono>
#include <iostream>

// Simple timing macros for measuring code execution time
// Usage:
//   TICK(my_timer);
//   // ... code to measure ...
//   TOCK(my_timer);
// Output: my_timer: 0.123456s

#define TICK(x) auto bench_##x = std::chrono::steady_clock::now();

#define TOCK(x)                                                           \
  std::cerr << #x ": "                                                    \
            << std::chrono::duration_cast<std::chrono::duration<double>>( \
                   std::chrono::steady_clock::now() - bench_##x)          \
                   .count();                                              \
  std::cerr << "s\n";
