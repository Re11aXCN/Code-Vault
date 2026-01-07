#pragma once

#include <cstdlib>
#include <string>
#include <type_traits>
#include <typeinfo>

#if (defined(__GNUC__) || defined(__clang__)) && __has_include(<cxxabi.h>)
#  include <cxxabi.h>
#endif
namespace stdex {
// Demangle C++ type names at runtime
// Useful for debugging and logging type information
inline std::string cppdemangle(const char* name)
{
#if (defined(__GNUC__) || defined(__clang__)) && __has_include(<cxxabi.h>)
  int         status = 0;
  char*       p      = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  std::string result = p ? p : name;
  std::free(p);
#else
  std::string result = name;
#endif
  return result;
}

inline std::string cppdemangle(const std::type_info& type)
{
  return cppdemangle(type.name());
}

template<class T>
inline std::string cppdemangle()
{
  std::string result =
      cppdemangle(typeid(std::remove_cv_t<std::remove_reference_t<T>>));

  if (std::is_const_v<std::remove_reference_t<T>>) result += " const";
  if (std::is_volatile_v<std::remove_reference_t<T>>) result += " volatile";
  if (std::is_lvalue_reference_v<T>) result += " &";
  if (std::is_rvalue_reference_v<T>) result += " &&";

  return result;
}
}  // namespace stdex

/*
// Usage Examples:

#include <iostream>
#include <vector>

int main() {
    // Basic type
    std::cout << stdex::cppdemangle<int>() << std::endl;
    // Output: int

    // Reference type
    int i = 42;
    std::cout << stdex::cppdemangle<decltype(i)>() << std::endl;
    // Output: int

    // Const reference
    std::cout << stdex::cppdemangle<decltype(std::as_const(i))>() << std::endl;
    // Output: int const &

    // Complex types
    std::cout << stdex::cppdemangle<std::vector<int>>() << std::endl;
    // Output: std::vector<int, std::allocator<int>>

    // With type_info
    std::cout << stdex::cppdemangle(typeid(double)) << std::endl;
    // Output: double

    return 0;
}
*/
