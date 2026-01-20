# C++ 标准库头文件

C++ 标准库的接口由以下头文件集合定义。

## 多用途头文件

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cstdlib>`](https://cppreference.cn/w/cpp/header/cstdlib)  | 通用工具：[程序控制](https://cppreference.cn/w/cpp/utility/program)、[动态内存分配](https://cppreference.cn/w/cpp/memory/c)、[随机数](https://cppreference.cn/w/cpp/numeric/random#C_random_library)、[排序与搜索](https://cppreference.cn/w/cpp/algorithm#C_library) |
| [`<execution>`](https://cppreference.cn/w/cpp/header/execution) (C++17) | 算法并行版本和执行控制组件的预定义执行策略 (C++26 起)        |

## 语言支持库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cfloat>`](https://cppreference.cn/w/cpp/header/cfloat)    | [浮点类型限制](https://cppreference.cn/w/cpp/types/climits)  |
| [`<climits>`](https://cppreference.cn/w/cpp/header/climits)  | [整数类型限制](https://cppreference.cn/w/cpp/types/climits)  |
| [`<compare>`](https://cppreference.cn/w/cpp/header/compare) (C++20) | [三路比较运算符](https://cppreference.cn/w/cpp/language/operator_comparison#Three-way_comparison)支持 |
| [`<contracts>`](https://cppreference.cn/w/cpp/header/contracts) (C++26) | [契约支持库](https://cppreference.cn/w/cpp/contracts)        |
| [`<coroutine>`](https://cppreference.cn/w/cpp/header/coroutine) (C++20) | [协程支持库](https://cppreference.cn/w/cpp/coroutine)        |
| [`<csetjmp>`](https://cppreference.cn/w/cpp/header/csetjmp)  | [保存（和跳转）到执行上下文的宏（和函数）](https://cppreference.cn/w/cpp/utility/program/setjmp) |
| [`<csignal>`](https://cppreference.cn/w/cpp/header/csignal)  | [信号管理函数和宏常量](https://cppreference.cn/w/cpp/utility/program) |
| [`<cstdarg>`](https://cppreference.cn/w/cpp/header/cstdarg)  | [可变长度参数列表处理](https://cppreference.cn/w/cpp/utility/variadic) |
| [`<cstddef>`](https://cppreference.cn/w/cpp/header/cstddef)  | [标准宏和 typedef](https://cppreference.cn/w/cpp/types)      |
| [`<cstdint>`](https://cppreference.cn/w/cpp/header/cstdint) (C++11) | [固定宽度整数类型](https://cppreference.cn/w/cpp/types/integer)和[其他类型限制](https://cppreference.cn/w/cpp/types/climits) |
| [`<exception>`](https://cppreference.cn/w/cpp/header/exception) | [异常处理工具](https://cppreference.cn/w/cpp/error#Exception_handling) |
| [`<initializer_list>`](https://cppreference.cn/w/cpp/header/initializer_list) (C++11) | [`std::initializer_list`](https://cppreference.cn/w/cpp/utility/initializer_list) 类模板 |
| [`<limits>`](https://cppreference.cn/w/cpp/header/limits)    | [查询算术类型属性](https://cppreference.cn/w/cpp/types/numeric_limits) |
| [`<new>`](https://cppreference.cn/w/cpp/header/new)          | [低级内存管理工具](https://cppreference.cn/w/cpp/memory/new) |
| [`<source_location>`](https://cppreference.cn/w/cpp/header/source_location) (C++20) | 提供获取[源代码位置](https://cppreference.cn/w/cpp/utility/source_location)的方法 |
| [`<stdfloat>`](https://cppreference.cn/w/cpp/header/stdfloat) (C++23) | [固定宽度浮点类型](https://cppreference.cn/w/cpp/types/floating-point) |
| [`<typeindex>`](https://cppreference.cn/w/cpp/header/typeindex) (C++11) | [`std::type_index`](https://cppreference.cn/w/cpp/types/type_index) |
| [`<typeinfo>`](https://cppreference.cn/w/cpp/header/typeinfo) | [运行时类型信息工具](https://cppreference.cn/w/cpp/types)    |
| [`<version>`](https://cppreference.cn/w/cpp/header/version) (C++20) | 提供宏以验证库的实现状态                                     |

## 概念库

| 头文件                                                       | 描述                                                 |
| ------------------------------------------------------------ | ---------------------------------------------------- |
| [`<concepts>`](https://cppreference.cn/w/cpp/header/concepts) (C++20) | [基本库概念](https://cppreference.cn/w/cpp/concepts) |

## 诊断库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cassert>`](https://cppreference.cn/w/cpp/header/cassert)  | [将参数与零比较的条件编译宏](https://cppreference.cn/w/cpp/error/assert) |
| [`<cerrno>`](https://cppreference.cn/w/cpp/header/cerrno)    | [包含上次错误码的宏](https://cppreference.cn/w/cpp/error/errno) |
| [`<debugging>`](https://cppreference.cn/w/cpp/header/debugging) (C++26) | 调试库                                                       |
| [`<stacktrace>`](https://cppreference.cn/w/cpp/header/stacktrace) (C++23) | [栈回溯](https://cppreference.cn/w/cpp/utility/basic_stacktrace)库 |
| [`<stdexcept>`](https://cppreference.cn/w/cpp/header/stdexcept) | [标准异常类型](https://cppreference.cn/w/cpp/error#Exception_categories) |
| [`<system_error>`](https://cppreference.cn/w/cpp/header/system_error) (C++11) | 定义 [`std::error_code`](https://cppreference.cn/w/cpp/error/error_code)，一个依赖于平台的错误码 |

## 内存管理库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<memory>`](https://cppreference.cn/w/cpp/header/memory)    | [高级内存管理工具](https://cppreference.cn/w/cpp/memory)     |
| [`<memory_resource>`](https://cppreference.cn/w/cpp/header/memory_resource) (C++17) | [多态分配器和内存资源](https://cppreference.cn/w/cpp/memory/memory_resource) |
| [`<scoped_allocator>`](https://cppreference.cn/w/cpp/header/scoped_allocator) (C++11) | [嵌套分配器类](https://cppreference.cn/w/cpp/memory/scoped_allocator_adaptor) |

## 元编程库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<ratio>`](https://cppreference.cn/w/cpp/header/ratio) (C++11) | [编译时有理数运算](https://cppreference.cn/w/cpp/numeric/ratio) |
| [`<type_traits>`](https://cppreference.cn/w/cpp/header/type_traits) (C++11) | [编译时类型信息工具](https://cppreference.cn/w/cpp/types)    |

## 通用工具库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<any>`](https://cppreference.cn/w/cpp/header/any) (C++17)  | [`std::any`](https://cppreference.cn/w/cpp/utility/any) 类   |
| [`<bit>`](https://cppreference.cn/w/cpp/header/bit) (C++20)  | [位操作](https://cppreference.cn/w/cpp/numeric#Bit_manipulation_.28since_C.2B.2B20.29)函数 |
| [`<bitset>`](https://cppreference.cn/w/cpp/header/bitset)    | [`std::bitset`](https://cppreference.cn/w/cpp/utility/bitset) 类模板 |
| [`<expected>`](https://cppreference.cn/w/cpp/header/expected) (C++23) | `std::expected` 类模板                                       |
| [`<functional>`](https://cppreference.cn/w/cpp/header/functional) | [函数对象、函数调用、绑定操作和引用包装器](https://cppreference.cn/w/cpp/utility/functional) |
| [`<optional>`](https://cppreference.cn/w/cpp/header/optional) (C++17) | [`std::optional`](https://cppreference.cn/w/cpp/utility/optional) 类模板 |
| [`<tuple>`](https://cppreference.cn/w/cpp/header/tuple) (C++11) | [`std::tuple`](https://cppreference.cn/w/cpp/utility/tuple) 类模板 |
| [`<utility>`](https://cppreference.cn/w/cpp/header/utility)  | 各种[实用组件](https://cppreference.cn/w/cpp/utility)        |
| [`<variant>`](https://cppreference.cn/w/cpp/header/variant) (C++17) | [`std::variant`](https://cppreference.cn/w/cpp/utility/variant) 类模板 |

## 容器库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<array>`](https://cppreference.cn/w/cpp/header/array) (C++11) | [`std::array`](https://cppreference.cn/w/cpp/container/array) 容器 |
| [`<deque>`](https://cppreference.cn/w/cpp/header/deque)      | [`std::deque`](https://cppreference.cn/w/cpp/container/deque) 容器 |
| [`<flat_map>`](https://cppreference.cn/w/cpp/header/flat_map) (C++23) | `std::flat_map` 和 `std::flat_multimap` 容器适配器           |
| [`<flat_set>`](https://cppreference.cn/w/cpp/header/flat_set) (C++23) | `std::flat_set` 和 `std::flat_multiset` 容器适配器           |
| [`<forward_list>`](https://cppreference.cn/w/cpp/header/forward_list) (C++11) | [`std::forward_list`](https://cppreference.cn/w/cpp/container/forward_list) 容器 |
| [`<hive>`](https://cppreference.cn/w/cpp/header/hive) (C++26) | `std::hive` 容器                                             |
| [`<inplace_vector>`](https://cppreference.cn/w/cpp/header/inplace_vector) (C++26) | `std::inplace_vector` 容器                                   |
| [`<list>`](https://cppreference.cn/w/cpp/header/list)        | [`std::list`](https://cppreference.cn/w/cpp/container/list) 容器 |
| [`<map>`](https://cppreference.cn/w/cpp/header/map)          | [`std::map`](https://cppreference.cn/w/cpp/container/map) 和 [`std::multimap`](https://cppreference.cn/w/cpp/container/multimap) 关联容器 |
| [`<mdspan>`](https://cppreference.cn/w/cpp/header/mdspan) (C++23) | `std::mdspan` 视图                                           |
| [`<queue>`](https://cppreference.cn/w/cpp/header/queue)      | [`std::queue`](https://cppreference.cn/w/cpp/container/queue) 和 [`std::priority_queue`](https://cppreference.cn/w/cpp/container/priority_queue) 容器适配器 |
| [`<set>`](https://cppreference.cn/w/cpp/header/set)          | [`std::set`](https://cppreference.cn/w/cpp/container/set) 和 [`std::multiset`](https://cppreference.cn/w/cpp/container/multiset) 关联容器 |
| [`<span>`](https://cppreference.cn/w/cpp/header/span) (C++20) | `std::span` 视图                                             |
| [`<stack>`](https://cppreference.cn/w/cpp/header/stack)      | [`std::stack`](https://cppreference.cn/w/cpp/container/stack) 容器适配器 |
| [`<unordered_map>`](https://cppreference.cn/w/cpp/header/unordered_map) (C++11) | [`std::unordered_map`](https://cppreference.cn/w/cpp/container/unordered_map) 和 [`std::unordered_multimap`](https://cppreference.cn/w/cpp/container/unordered_multimap) 无序关联容器 |
| [`<unordered_set>`](https://cppreference.cn/w/cpp/header/unordered_set) (C++11) | [`std::unordered_set`](https://cppreference.cn/w/cpp/container/unordered_set) 和 [`std::unordered_multiset`](https://cppreference.cn/w/cpp/container/unordered_multiset) 无序关联容器 |
| [`<vector>`](https://cppreference.cn/w/cpp/header/vector)    | [`std::vector`](https://cppreference.cn/w/cpp/container/vector) 容器 |

## 迭代器库

| 头文件                                                       | 描述                                                 |
| ------------------------------------------------------------ | ---------------------------------------------------- |
| [`<iterator>`](https://cppreference.cn/w/cpp/header/iterator) | [范围迭代器](https://cppreference.cn/w/cpp/iterator) |

## 范围库 (Ranges library)

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<generator>`](https://cppreference.cn/w/cpp/header/generator) (C++23) | `std::generator` 类模板                                      |
| [`<ranges>`](https://cppreference.cn/w/cpp/header/ranges) (C++20) | [范围访问、原语、需求、工具和适配器](https://cppreference.cn/w/cpp/ranges) |

## 算法库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<algorithm>`](https://cppreference.cn/w/cpp/header/algorithm) | [在范围上操作的算法](https://cppreference.cn/w/cpp/algorithm) |
| [`<numeric>`](https://cppreference.cn/w/cpp/header/numeric)  | [对范围内的值进行数值操作](https://cppreference.cn/w/cpp/numeric) |

## 字符串库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cstring>`](https://cppreference.cn/w/cpp/header/cstring)  | 各种[窄字符字符串处理函数](https://cppreference.cn/w/cpp/string/byte) |
| [`<string>`](https://cppreference.cn/w/cpp/header/string)    | [`std::basic_string`](https://cppreference.cn/w/cpp/string/basic_string) 类模板 |
| [`<string_view>`](https://cppreference.cn/w/cpp/header/string_view) (C++17) | [`std::basic_string_view`](https://cppreference.cn/w/cpp/string/basic_string_view) 类模板 |

## 文本处理库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cctype>`](https://cppreference.cn/w/cpp/header/cctype)    | [确定窄字符类别的函数](https://cppreference.cn/w/cpp/string/byte) |
| [`<charconv>`](https://cppreference.cn/w/cpp/header/charconv) (C++17) | `std::to_chars` 和 `std::from_chars`                         |
| [`<clocale>`](https://cppreference.cn/w/cpp/header/clocale)  | [C 本地化工具](https://cppreference.cn/w/cpp/locale#C_library_locales) |
| [`<codecvt>`](https://cppreference.cn/w/cpp/header/codecvt) (C++11) (C++17 中已弃用) (C++26 中已移除) | [Unicode 转换工具](https://cppreference.cn/w/cpp/locale)     |
| [`<cuchar>`](https://cppreference.cn/w/cpp/header/cuchar) (C++11) | C 风格的[Unicode 字符转换函数](https://cppreference.cn/w/cpp/string/multibyte) |
| [`<cwchar>`](https://cppreference.cn/w/cpp/header/cwchar)    | 各种[宽](https://cppreference.cn/w/cpp/string/wide)和[多字节](https://cppreference.cn/w/cpp/string/multibyte)字符串处理函数 |
| [`<cwctype>`](https://cppreference.cn/w/cpp/header/cwctype)  | [确定宽字符类别的函数](https://cppreference.cn/w/cpp/string/wide) |
| [`<format>`](https://cppreference.cn/w/cpp/header/format) (C++20) | [格式化库](https://cppreference.cn/w/cpp/utility/format)，包括 [`std::format`](https://cppreference.cn/w/cpp/utility/format/format) |
| [`<locale>`](https://cppreference.cn/w/cpp/header/locale)    | [本地化工具](https://cppreference.cn/w/cpp/locale)           |
| [`<regex>`](https://cppreference.cn/w/cpp/header/regex) (C++11) | [支持正则表达式处理的类、算法和迭代器](https://cppreference.cn/w/cpp/regex) |
| [`<text_encoding>`](https://cppreference.cn/w/cpp/header/text_encoding) (C++26) | 文本编码识别                                                 |

## 数值库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cfenv>`](https://cppreference.cn/w/cpp/header/cfenv) (C++11) | [浮点环境](https://cppreference.cn/w/cpp/numeric/fenv)访问函数 |
| [`<cmath>`](https://cppreference.cn/w/cpp/header/cmath)      | [常用数学函数](https://cppreference.cn/w/cpp/numeric/math)   |
| [`<complex>`](https://cppreference.cn/w/cpp/header/complex)  | [复数类型](https://cppreference.cn/w/cpp/numeric/complex)    |
| [`<linalg>`](https://cppreference.cn/w/cpp/header/linalg) (C++26) | [基本线性代数算法 (BLAS)](https://cppreference.cn/w/cpp/numeric/linalg) |
| [`<numbers>`](https://cppreference.cn/w/cpp/header/numbers) (C++20) | [数学常数](https://cppreference.cn/w/cpp/numeric/constants)  |
| [`<random>`](https://cppreference.cn/w/cpp/header/random) (C++11) | [随机数生成器和分布](https://cppreference.cn/w/cpp/numeric/random) |
| [`<simd>`](https://cppreference.cn/w/cpp/header/simd) (C++26) | [数据并行类型以及对这些类型的操作](https://cppreference.cn/w/cpp/numeric/simd) |
| [`<valarray>`](https://cppreference.cn/w/cpp/header/valarray) | [表示和操作值数组的类](https://cppreference.cn/w/cpp/numeric/valarray) |

## 时间库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<chrono>`](https://cppreference.cn/w/cpp/header/chrono) (C++11) | [C++ 时间工具](https://cppreference.cn/w/cpp/chrono)         |
| [`<ctime>`](https://cppreference.cn/w/cpp/header/ctime)      | [C 风格时间/日期工具](https://cppreference.cn/w/cpp/chrono/c) |

## 输入/输出库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<cinttypes>`](https://cppreference.cn/w/cpp/header/cinttypes) (C++11) | [格式化宏](https://cppreference.cn/w/cpp/types/integer#Format_macro_constants)，`intmax_t` 和 `uintmax_t` 数学和转换 |
| [`<cstdio>`](https://cppreference.cn/w/cpp/header/cstdio)    | [C 风格输入/输出函数](https://cppreference.cn/w/cpp/io/c)    |
| [`<filesystem>`](https://cppreference.cn/w/cpp/header/filesystem) (C++17) | [`std::filesystem::path`](https://cppreference.cn/w/cpp/filesystem/path) 类和[支持函数](https://cppreference.cn/w/cpp/filesystem) |
| [`<fstream>`](https://cppreference.cn/w/cpp/header/fstream)  | [`std::basic_fstream`](https://cppreference.cn/w/cpp/io/basic_fstream)、[`std::basic_ifstream`](https://cppreference.cn/w/cpp/io/basic_ifstream)、[`std::basic_ofstream`](https://cppreference.cn/w/cpp/io/basic_ofstream) 类模板和 typedef |
| [`<iomanip>`](https://cppreference.cn/w/cpp/header/iomanip)  | [帮助函数以控制输入和输出的格式](https://cppreference.cn/w/cpp/io/manip) |
| [`<ios>`](https://cppreference.cn/w/cpp/header/ios)          | [`std::ios_base`](https://cppreference.cn/w/cpp/io/ios_base) 类、[`std::basic_ios`](https://cppreference.cn/w/cpp/io/basic_ios) 类模板和 typedef |
| [`<iosfwd>`](https://cppreference.cn/w/cpp/header/iosfwd)    | 输入/输出库中所有类的前向声明                                |
| [`<iostream>`](https://cppreference.cn/w/cpp/header/iostream) | 几个标准流对象                                               |
| [`<istream>`](https://cppreference.cn/w/cpp/header/istream)  | [`std::basic_istream`](https://cppreference.cn/w/cpp/io/basic_istream) 类模板和 typedef |
| [`<ostream>`](https://cppreference.cn/w/cpp/header/ostream)  | [`std::basic_ostream`](https://cppreference.cn/w/cpp/io/basic_ostream)、[`std::basic_iostream`](https://cppreference.cn/w/cpp/io/basic_iostream) 类模板和 typedef |
| [`<print>`](https://cppreference.cn/w/cpp/header/print) (C++23) | 格式化输出库，包括 `std::print`                              |
| [`<spanstream>`](https://cppreference.cn/w/cpp/header/spanstream) (C++23) | `std::basic_spanstream`、`std::basic_ispanstream`、`std::basic_ospanstream` 类模板和 typedef |
| [`<sstream>`](https://cppreference.cn/w/cpp/header/sstream)  | [`std::basic_stringstream`](https://cppreference.cn/w/cpp/io/basic_stringstream)、[`std::basic_istringstream`](https://cppreference.cn/w/cpp/io/basic_istringstream)、[`std::basic_ostringstream`](https://cppreference.cn/w/cpp/io/basic_ostringstream) 类模板和 typedef |
| [`<streambuf>`](https://cppreference.cn/w/cpp/header/streambuf) | [`std::basic_streambuf`](https://cppreference.cn/w/cpp/io/basic_streambuf) 类模板 |
| [`<strstream>`](https://cppreference.cn/w/cpp/header/strstream) (在 C++98 中已废弃) (在 C++26 中已移除) | [`std::strstream`](https://cppreference.cn/w/cpp/io/strstream)、[`std::istrstream`](https://cppreference.cn/w/cpp/io/istrstream)、[`std::ostrstream`](https://cppreference.cn/w/cpp/io/ostrstream) |
| [`<syncstream>`](https://cppreference.cn/w/cpp/header/syncstream) (C++20) | `std::basic_osyncstream`、`std::basic_syncbuf` 和 typedef    |

## 并发支持库

| 头文件                                                       | 描述                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| [`<atomic>`](https://cppreference.cn/w/cpp/header/atomic) (C++11) | [原子操作库](https://cppreference.cn/w/cpp/thread#Atomic_operations) |
| [`<barrier>`](https://cppreference.cn/w/cpp/header/barrier) (C++20) | [屏障](https://cppreference.cn/w/cpp/thread/barrier)         |
| [`<condition_variable>`](https://cppreference.cn/w/cpp/header/condition_variable) (C++11) | [线程等待条件](https://cppreference.cn/w/cpp/thread)         |
| [`<future>`](https://cppreference.cn/w/cpp/header/future) (C++11) | [异步计算原语](https://cppreference.cn/w/cpp/thread)         |
| [`<hazard_pointer>`](https://cppreference.cn/w/cpp/header/hazard_pointer) (C++26) | 危险指针                                                     |
| [`<latch>`](https://cppreference.cn/w/cpp/header/latch) (C++20) | [闩](https://cppreference.cn/w/cpp/thread/latch)             |
| [`<mutex>`](https://cppreference.cn/w/cpp/header/mutex) (C++11) | [互斥原语](https://cppreference.cn/w/cpp/thread)             |
| [`<rcu>`](https://cppreference.cn/w/cpp/header/rcu) (C++26)  | 读-复制-更新机制                                             |
| [`<semaphore>`](https://cppreference.cn/w/cpp/header/semaphore) (C++20) | [信号量](https://cppreference.cn/w/cpp/thread/counting_semaphore) |
| [`<shared_mutex>`](https://cppreference.cn/w/cpp/header/shared_mutex) (C++14) | [共享互斥原语](https://cppreference.cn/w/cpp/thread)         |
| [`<stop_token>`](https://cppreference.cn/w/cpp/header/stop_token) (C++20) | [`std::jthread`](https://cppreference.cn/w/cpp/thread/jthread) 的停止令牌 |
| [`<thread>`](https://cppreference.cn/w/cpp/header/thread) (C++11) | [`std::thread`](https://cppreference.cn/w/cpp/thread/thread) 类和[支持函数](https://cppreference.cn/w/cpp/thread) |
