/*
* 模块不允许循环依赖
* 
* 导入是不可传递的，
* 例如：A导出了func1，B导入了A，B导出func2，但是B没有导出A，
*   如果你导入B使用其导出的功能，那么A的func1将无法被使用
*   因为你并没有在B导出A
* 
* 
  export
  1. 可以导出 声明 或 定义
  2. 可以导出 包含再花括号中的 一组声明
  3. 可以导出 命名空间
  4. 可以导出 命名空间的一个子集，如某一个特定的命名空间成员

  Common Module Filename Extensions
  • .ixx——Microsoft Visual C++ filename extension for
    primary module interface unit
  • .ifc——Microsoft Visual C++ filename extension for
    compiled primary module interface unit
  • .cpp——Filename extension for Ct+ source code,
    including module units
  • .cppm——A recommended clang++ filename extension
    for module units (recognized by Visual C++)
  • .pcm——clang++ compiled primary module interface unit
*/

// 如果无法 import <string> 使用， 则需要在 全局模块片段中放置对应的
// #include预处理器指令
module;  // 全局模块片段
#include <concepts>
#include <string>
#include <chrono>
export module hello;  // 声明一个主模块接口单元
// 整个模块只能有一个export module 语句，那就是你的主模块接口单元。
// 以下的所有导出语句必须在模块声明之后
//
// 被导出的声明不允许具有所谓的内部链接
//    例如：在全局命名空间作用域中声明的静态变量。
//         同样也不允许导出全局常量或常量表达式变量从翻译单元中导出。
// 任何在无名命名空间中声明的标识符，也就是我们没有给它们显式命名的命名空间，这两者都被视为具有内部链接，只能在本翻译单元使用
//
// 如果你在这里使用#define
// 预处理器命令定义了任何预处理器宏，这些宏仅在模块内部可知。即 不能导出#define
// 宏

// import <string>;
// import <concepts>;

// export static const int MY_CONST = 42; // Error
#define MY_CONST 42;
// export MY_CONST; // Error

export void hello();  // 导出声明

export std::string hello_world() { return "Hello, world!"; }  // 导出声明定义

// 如果 一个方法是 模板 或者 该方法被 声明为 inline 或 constexpr，那么你应该
// 导出完整的声明定义，因为无论你在何处使用该模块，编译器都需要访问该完整定义。
export inline constexpr int my_constexpr_func() { return MY_CONST; }

export template<
    std::bidirectional_iterator BiIter,
    std::strict_weak_order<std::iter_value_t<BiIter>, std::iter_value_t<BiIter>>
        Compare>
inline void insert_sort(BiIter start, BiIter end, Compare comp)
{
  if (start == end) [[unlikely]]
    return;
  for (auto curr = std::next(start); curr != end; ++curr) {
    auto value = std::move(*curr);
    auto hole  = curr;

    auto prev = hole;
    while (prev > start && comp(value, *std::prev(prev))) {
      --prev;
      *hole = std::move(*prev);
      hole  = prev;
    }
    *hole = std::move(value);
  }
}

// 类似 extern "C" {}，使用花括号包裹导出的内容
export {
  std::string hello_world_from_braces_block()
  {
    return "Hello, world from block!";
  }
}

export namespace my_namespace {
std::string hello_world_from_namespace()
{
  return "Hello, world from namespace!";
}
}  // namespace my_namespace

// ！！！C++20 模块 可见性 与 可达性，测试
namespace TimeUtils {
  class Time // 可达的但是不可见
  {
  public:

    explicit Time() { }

    std::string_view get_utc_time() const noexcept {
      auto now = std::chrono::system_clock::now();
      auto time_t = std::chrono::system_clock::to_time_t(now);
      auto time_str = std::ctime(&time_t);
      return std::string_view(time_str);
    }
  private:
    std::chrono::time_point<std::chrono::system_clock> time_point_;
  };

  // 封装对象，只能用 auto 接收
  export Time get_time_obj() { return Time(); };
}

module :private;  // implementation details below this

// 私有模块，只能在当前翻译单元中使用，不导出
// 优势：所有实现都在一个文件中，利于编译器进行内联优化
// 缺点：如果是一个大型团队开发，较分模块分离实现开发来说，不利于维护，
// 分模块还有好处就是如果修改-impl.cpp文件不会导致模块ixx文件重新编译
// 而非私有模块修改则会导致所有模块链重新编译

// BUG：如果修改当前的私有模块内容，会导致所有的模块链都重新编译，理论上应该不会的，模块这一特性还需要厂商继续完善