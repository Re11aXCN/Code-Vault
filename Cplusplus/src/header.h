#pragma once


// 非标准头文件（MSVC编译器扩展）
//#include <hash_map> // deprecated, please use unordered_map instead
//#include <hash_set> // deprecated, please use unordered_set instead
#include <xfacet>
#include <xhash>
#include <xiosbase>
#include <xlocale>
#include <xlocbuf>
#include <xlocinfo>
#include <xlocmes>
#include <xlocmon>
#include <xlocnum>
#include <xloctime>
#include <xmemory>
#include <xstring>
#include <xtr1common>
#include <xtree>
#include <xutility>

// C++98、C++03和C++11标准化的头文件
#include <cassert>
//#include <ccomplex> // deprecated in C++20
#include <cctype>
#include <cerrno>
#include <cfenv>
#include <cfloat>
#include <cinttypes>
//#include <ciso646> // deprecated in C++20
#include <climits> 
#include <clocale>
#include <cmath>
#include <csetjmp>
#include <csignal>
//#include <cstdalign> // deprecated in C++17
#include <cstdarg>
//#include <cstdbool> // deprecated in C++17
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cstring>
//#include <ctgmath> // deprecated in C++17
#include <ctime>
#include <cuchar>
#include <cwchar>
#include <cwctype>

// #include <codecvt> // deprecated in C++17
#include <complex>
#include <numeric>
#include <random>
#include <limits>
#include <ratio> 

#include <algorithm>

#include <exception> 
#include <stdexcept>
#include <system_error>

#include <iomanip>
#include <ios>
#include <iosfwd>
#include <fstream>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <streambuf>
//#include <strstream> // deprecated in C++98

#include <iterator>
#include <initializer_list>
#include <valarray>
#include <bitset>
#include <array>
#include <map>
#include <queue> // contais std::priority_queue
#include <set>
#include <tuple>
#include <vector>
#include <deque>
#include <list>
#include <forward_list>
#include <unordered_map> // contains std::unordered_multimap
#include <unordered_set> // contains std::unordered_multiset
#include <string>
#include <stack>

#include <chrono> 
#include <functional>

#include <locale>   // see clocale above
#include <regex>    

#include <typeindex>
#include <typeinfo>
#include <type_traits>
#include <utility> 


#include <new>
#include <memory>
#include <scoped_allocator>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <future>
#include <thread>

// C++14
#include <shared_mutex>

// C++17
#include <any>
#include <optional>
#include <variant>
#include <string_view>
#include <memory_resource>
#include <execution> 
#include <filesystem>   
#include <charconv>
#include <version>  

// C++20
#include <barrier>
#include <latch>
#include <stop_token>
#include <syncstream>
#include <format>
#include <span>
#include <ranges>
#include <compare>
#include <concepts>
#include <coroutine>
#include <semaphore>
#include <source_location>
#include <numbers> 
#include <bit>

// C++23
#include <expected>
#include <print>
#include <stacktrace>
#include <stdfloat>
#include <mdspan>
#include <spanstream>
//#include <flat_set>
//#include <flat_map>
#include <generator>

// C++26
//#include <inplace_vector>
//#include <hive>
