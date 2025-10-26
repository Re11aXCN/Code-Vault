# 并发



## atomic

### 1. mtpool.hpp

> **多线程测试**

- 创建多个线程同时执行测试代码
- 使用栅栏(barrier)确保所有线程同步开始和结束
- 绑定线程到不同CPU核心实现真正并行

### 2. spin_barrier.hpp

> **SpinBarrier** 让多个线程在某个执行点等待，直到所有线程都到达该点后才能继续执行。

1. **与 `std::barrier` 的区别**

| 特性         | SpinBarrier      | std::barrier       |
| ------------ | ---------------- | ------------------ |
| **等待策略** | 自旋等待(忙等待) | 阻塞等待(线程休眠) |
| **CPU使用**  | 高(持续检查条件) | 低(线程让出CPU)    |
| **延迟**     | 低(立即响应)     | 较高(线程唤醒开销) |
| **适用场景** | 短时间等待       | 长时间等待         |

2. **与 `std::latch` 的区别**

| 特性           | SpinBarrier     | std::latch    |
| -------------- | --------------- | ------------- |
| **可重用性**   | ✅ 可重复使用    | ❌ 一次性使用  |
| **线程数可变** | ❌ 固定线程数    | ✅ 可动态调整  |
| **同步方向**   | 双向(等待+继续) | 单向(计数到0) |



适合使用 SpinBarrier：

- **高性能计算**：等待时间极短(纳秒/微秒级)
- **实时系统**：需要确定性延迟
- **内核开发**：不能进行线程调度的环境

适合使用 std::barrier/std::latch：

- **通用应用程序**：等待时间不确定
- **I/O密集型任务**：线程可能长时间等待
- **节能考虑**：减少CPU占用

### 3. spin_mutex.hpp

> 自旋锁，详细见[Standard/并发支持库.md - 第六章 锁的性能测试]()



------



## thread

### 1. mtqueue.hpp

> 小彭老师编写的多线程队列，没有性能瓶颈mt_queue的泛用性和易用性和维护性都是你选择它的理由



## oldatomic

### 1. no_aba_concurrent_stack.cpp

> 使用**标签指针（Tagged Pointer）** 策略来解决ABA问题的无锁并发链表实现



## co_async、co_http

> 协程、协程服务器，需要仔细研究
>
> 工具
>
> co_async-master\co_async\utils



# 设计模式

## bind

### 1. Signal.hpp

> **轻量级信号槽(signals-slots)系统**的实现，类似于Qt的信号槽机制，但更加轻量和现代化
>
> - 都是观察者模式的实现，支持一对多的通信
> - 支持槽函数在信号发射时被自动调用
> - 支持槽函数的自动管理（如连接断开）

主要组成部分

1. **CallbackResult 枚举**：表示槽函数执行后的结果，有两种可能：
   - `Keep`：保留槽函数，继续使用
   - `Erase`：移除槽函数，通常用于一次性槽函数或当对象被销毁时
2. **oneshot_t 和 nshot_t**：用于指定槽函数的调用次数。
   - `oneshot`：一次性槽函数，调用一次后自动移除
   - `nshot_t`：指定次数的槽函数，调用指定次数后自动移除
3. **details_ 命名空间**：包含一些实现细节，如：
   - `lock_if_weak`：处理弱指针，如果弱指针无效则返回空，用于对象生命周期管理
   - 多个 `bind` 函数：将成员函数绑定为槽函数，支持一次性、多次和永久绑定



## design

> 设计模式文档



# 反射

## reflect-hpp-master

### 1.reflect.hpp

> C++的静态反射，它允许在编译时获取结构体的成员信息，并提供了遍历成员的能力。这个库通过宏和模板技术实现，主要功能包括：
>
> 1. 遍历结构体的所有成员，并获取每个成员的名称和指针。
> 2. 根据成员名称获取成员的值或类型。
> 3. 判断成员的类型（成员变量、成员函数、静态变量、静态函数等）。

这是一个功能完整的**C++静态反射库**，提供了在编译时获取和操作类成员信息的能力。让我详细分析它的功能：

1. **成员遍历和访问**

```cpp
struct Person {
    std::string name;
    int age;
    
    REFLECT(name, age)  // 注册反射信息
};

Person p{"John", 25};

// 遍历所有成员
reflect::foreach_member(p, [](const char* name, auto& value) {
    std::cout << name << ": " << value << std::endl;
});
```

2. **成员指针操作**

```cpp
// 获取成员指针信息
reflect::foreach_member_ptr<Person>([](const char* name, auto member_ptr) {
    std::cout << "Member: " << name << std::endl;
});
```

3. **运行时成员查询**

```cpp
// 检查成员是否存在
bool has_age = reflect::has_member<Person>("age");

// 获取成员引用
int& age_ref = reflect::get_member<int>(p, "age");

// 安全获取成员指针
int* age_ptr = reflect::try_get_member<int>(p, "age");
```

4. **成员类型识别**

```cpp
// 检查成员种类（变量/函数/静态变量/静态函数）
bool is_var = reflect::is_member_kind<Person>("age", 
               reflect::member_kind::member_variable);

// 检查具体类型
bool is_int = reflect::is_member_type<Person, int>("age");
```



**REFLECT_TYPE** - 注册外部类型

```cpp
REFLECT_TYPE(Person, name, age)
```

**REFLECT** - 类内部注册

```cpp
struct Person {
    std::string name;
    int age;
    REFLECT(name, age)  // 在类内部使用
};
```

**REFLECT_TYPE_TEMPLATED** - 模板类注册

```cpp
template<typename T>
struct Container {
    T value;
};
REFLECT_TYPE_TEMPLATED((Container, T), value)
```

* 应用场景

1. **序列化/反序列化**

```cpp
// 自动JSON序列化
template<typename T>
std::string to_json(const T& obj) {
    std::stringstream ss;
    ss << "{";
    reflect::foreach_member(obj, [&](const char* name, auto& value) {
        ss << "\"" << name << "\": \"" << value << "\",";
    });
    ss << "}";
    return ss.str();
}
```

2. **对象关系映射(ORM)**

```cpp
// 自动生成SQL语句
template<typename T>
std::string create_table() {
    std::string sql = "CREATE TABLE " + get_table_name<T>() + " (";
    reflect::foreach_member_ptr<T>([&](const char* name, auto member) {
        sql += std::string(name) + " " + get_sql_type<decltype(member)>() + ", ";
    });
    sql += ");";
    return sql;
}
```

3. **测试框架**

```cpp
// 自动测试所有字段
template<typename T>
void test_all_fields(const T& obj) {
    reflect::foreach_member(obj, [](const char* name, auto& value) {
        std::cout << "Testing " << name << ": " << value << std::endl;
        // 执行各种测试...
    });
}
```

4. **配置系统**

```cpp
// 从配置文件加载到对象
template<typename T>
void load_from_config(T& obj, const Config& config) {
    reflect::foreach_member(obj, [&](const char* name, auto& value) {
        if (config.has(name)) {
            value = config.get<decltype(value)>(name);
        }
    });
}
```



### 2.reflect_json.hpp



## neorefelect

### 1. reflect.hpp

> **编译时反射**，主要用于在编译时获取聚合类型（如结构体）的成员信息，包括成员数量、成员的值、类型和名称



## enumreflecttest

### 1. scienum.h

> 反射得到枚举名字，可以学习思路，正常使用建议使用`magic_enum`





# 类型萃取

## cpplifetimetest

### 1. cppdemangle.h

> **跨平台 萃取正确完整类型 cvref**，关于`类型萃取`详细见[Standard/类型支持库.md - <type_info>]()



# 算法

## cpprandomtest

### 1.main.cpp（xorshift32、wangshash随机数生成算法）



# parallel

## cpp17pmrtest

> 所有文件可以阅读学习一下，C++17的std::pmr

### 1. filter.cpp（**AVX2指令集**、查找表）

> **使用AVX2指令集和预计算的查找表实现的高性能float向量化过滤器**，用于根据比较条件过滤数组中的元素

1. 核心功能

**从浮点数数组 `x` 中筛选出满足比较条件 `cmp` 的元素，存入数组 `z`**

```cpp
// 示例：筛选出所有大于 y 的元素
size_t count = filterp<_CMP_GT_OQ>(x, n, y, z);
```

2. 关键技术解析
   1. **查找表(LUT)预计算**

```cpp
__m256i masklut[512];  // 512个AVX向量
```
- **前半部分(256个)**：排列索引，将符合条件的元素聚集到向量前部
- **后半部分(256个)**：存储掩码，控制哪些元素写入内存
  2. **向量化处理流程**

```cpp
while (x + 16 <= xend) {  // 每次处理16个元素
    // 加载16个元素，分为两个AVX向量
    __m256 xi = _mm256_loadu_ps(x);
    __m256 xi2 = _mm256_loadu_ps(x + 8);
    
    // 比较并生成位掩码
    __m256 mask = _mm256_cmp_ps(xi, pred, cmp);
    size_t m = (size_t)_mm256_movemask_ps(mask) << 6;
    
    // 使用LUT获取排列和存储掩码
    __m256i wa = _mm256_load_si256(masklut + (m >> 5));      // 排列索引
    __m256i wb = _mm256_load_si256(masklut + (m >> 5) + 1);  // 存储掩码
    
    // 重排并条件存储
    xi = _mm256_permutevar8x32_ps(xi, wa);
    _mm256_maskstore_ps(z, wb, xi);
}
```

3. **LUT工作原理**

对于8位掩码 `i`（每个位代表一个比较结果）：
- **排列向量**：将满足条件的元素索引放在前面
- **存储掩码**：只有前 `c` 个通道为 `-1`（c=符合条件的元素个数）

性能优化特点

🚀 **向量化优势**

- **16元素并行处理**：使用两个AVX向量
- **零分支预测**：避免条件分支导致的流水线停顿
- **SIMD指令**：单指令多数据

🎯 **内存访问优化**

- **聚集-散射模式**：将分散的满足条件元素聚集到连续内存
- **对齐存储**：`_mm256_store_si256` 使用对齐存储提高性能

⚡ **避免标量处理**

只有在最后剩余元素不足16个时才使用标量处理：
```cpp
for (; x < xend; x++) {  // 标量处理尾部
    if (*x > y) *z++ = *x;
}
```

* 使用示例

```cpp
#include <immintrin.h>

int main() {
    const size_t n = 1000;
    float x[n], z[n];
    
    // 初始化数据
    for (size_t i = 0; i < n; i++) x[i] = (float)i;
    
    // 筛选出所有大于500的元素
    size_t count = filterp<_CMP_GT_OQ>(x, n, 500.0f, z);
    
    // z[0..count-1] 现在包含所有大于500的元素
    return 0;
}
```

4. 比较操作符

`cmp` 参数使用AVX比较谓词：
- `_CMP_GT_OQ`：大于 (ordered, quiet)
- `_CMP_LT_OQ`：小于  
- `_CMP_EQ_OQ`：等于
- `_CMP_GE_OQ`：大于等于
- `_CMP_LE_OQ`：小于等于

5. 性能对比

与标量版本对比：
```cpp
// 标量实现
size_t scalar_filter(float const* x, size_t n, float y, float* z) {
    auto zbeg = z;
    for (size_t i = 0; i < n; i++) {
        if (x[i] > y) *z++ = x[i];
    }
    return z - zbeg;
}
```

**性能提升**：预计向量化版本比标量版本快 **8-15倍**

6. 适用场景

- **数据库查询**：WHERE条件过滤
- **科学计算**：数据预处理和筛选
- **游戏引擎**：可见性裁剪、物理碰撞检测
- **图像处理**：阈值分割
- **金融分析**：条件数据筛选



------



> 







# function

## 1.Function.hpp

> std::function的实现细节，简单实现Function



# hexdump

## 1. cxxopts.hpp

这是一个**C++命令行参数解析库**，名为 `cxxopts`。让我为您详细解释它的功能和用途：

主要功能

1. **命令行参数解析**

- 解析类似 `--input file.txt -o output --verbose` 这样的命令行参数
- 支持短选项 (`-v`) 和长选项 (`--verbose`)
- 支持带值的参数 (`--file=data.txt`)

2. **特性亮点**

- **类型安全**：自动将参数转换为相应的C++类型
- **自动生成帮助信息**：根据定义的选项自动生成 `--help` 输出
- **灵活的选项定义**：
  - 必需/可选参数
  - 默认值
  - 隐式值
  - 容器类型（如vector）
  - 布尔标志

基本用法示例

```cpp
#include "cxxopts.hpp"

int main(int argc, char* argv[]) {
    cxxopts::Options options("MyProgram", "A description of my program");
    
    // 定义命令行选项
    options.add_options()
        ("h,help", "Print help")
        ("i,input", "Input file", cxxopts::value<std::string>())
        ("o,output", "Output file", cxxopts::value<std::string>()->default_value("out.txt"))
        ("v,verbose", "Verbose output", cxxopts::value<bool>()->default_value("false"))
        ("n,number", "Number of iterations", cxxopts::value<int>())
        ("files", "Input files", cxxopts::value<std::vector<std::string>>());
    
    // 解析参数
    auto result = options.parse(argc, argv);
    
    // 处理结果
    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        return 0;
    }
    
    if (result.count("input")) {
        std::string input_file = result["input"].as<std::string>();
        // 使用输入文件...
    }
    
    bool verbose = result["verbose"].as<bool>();
    int number = result["number"].as<int>();
    
    return 0;
}
```

代码结构概览

1. 核心类

- **`Options`**：主类，用于定义和配置所有选项
- **`ParseResult`**：存储解析结果
- **`OptionAdder`**：流畅接口用于添加选项
- **`Value`**：类型系统的基础类

2. 关键特性

1. **异常处理**：提供详细的错误信息
2. **Unicode支持**：可选的ICU库集成
3. **无异常模式**：可通过宏定义禁用异常
4. **跨平台**：支持GCC、Clang、MSVC等编译器

3. 使用场景

这个库非常适合需要复杂命令行接口的C++应用程序，比如：
- 命令行工具
- 编译器前端
- 数据处理工具
- 系统管理工具

相比手动解析 `argc/argv`，这个库提供了更安全、更易维护的解决方案，并且自动处理了很多边界情况和错误处理。



# IO

## iostream

### 1. cstdio.cpp

> 输出流缓冲介绍



### 2. ostream.cpp

> 实现了一个**Unix系统下的I/O流抽象层**，提供了类似于C++标准库流的功能
>
> - `InStream`：抽象输入流基类
> - `OutStream`：抽象输出流基类
> - 提供统一的读写接口
>
> 学习I/O用

## minilog-master

### 1. minilog.h

> 日志、可以学习思路，正常使用建议使用`spdlog`



# STL

## stl_map/experiment

> 所有代码可以学习，map、hash、flatmap等



# memory

## mallochook、mallocvis-main

[mallocvis-main](https://github.com/archibate/mallocvis)研发的一款用于可视化C/C++程序中动态内存分配的工具

b站视频：https://www.bilibili.com/video/BV1if421B7Jo/

> 一个**内存分配跟踪和可视化工具**
>
> - alloc_action.hpp: 定义数据结构、枚举和常量，用于描述内存分配操作。
> - check.cpp: 实现一个内存检查工具，用于检测内存泄漏、重复释放等问题。
> - hook.cpp: 实现内存分配函数的钩子，记录所有内存操作，并收集数据。
> - main.cpp: 主程序，使用多种STL和第三方库容器进行内存分配，用于测试。
> - plot.cpp: 将记录的内存分配数据可视化为SVG图形。

1. alloc_action.hpp
   - 定义枚举AllocOp，表示内存分配操作类型（如New、Delete、Malloc、Free等）。
   - 定义结构体AllocAction，记录一次内存分配操作的具体信息，包括操作类型、线程ID、指针、大小、对齐、调用地址和时间。
   - 定义一些常量数组，如kAllocOpNames（操作类型的字符串名称）、kAllocOpIsAllocation（标识操作是否为分配）、kAllocOpPair（配对的操作，如New对应Delete）。
   - 定义常量kNone，表示未指定的值。
2. check.cpp
   - 实现一个全局对象GlobalData，用于跟踪当前已分配的内存块。
   - 在GlobalData的构造函数中启用跟踪，析构函数中输出内存泄漏信息。
   - 重载了全局的operator new和operator delete（包括数组版本、对齐版本等），在内存分配和释放时调用GlobalData的on方法进行检查。
   - 使用EnableGuard防止在检查过程中递归调用。
   - 在on方法中，根据操作类型检查内存分配和释放的合法性，如重复分配、释放不存在的内存、释放函数不匹配、大小或对齐不匹配等。
3. hook.cpp
   - 同样实现一个全局对象GlobalData，但这里用于记录所有的内存分配操作（包括C风格的malloc/free等）。
   - 重载了C风格的内存分配函数（malloc、free、calloc、realloc等）和C++的operator new/delete，记录每次操作到actions队列中。
   - 在程序结束时（GlobalData析构时）调用plot_alloc_actions函数将记录的操作可视化。
   - 使用EnableGuard防止递归调用。
   - 对于不同的平台（Unix和Windows）使用不同的方式获取线程ID，并记录时间戳。
4. main.cpp
   - 主程序，使用多种容器（vector、deque、list、set、unordered_set以及第三方库的robin_set、flat_hash_set、node_hash_set）进行插入操作。
   - 每次插入后睡眠1微秒，以产生不同的时间戳。
   - 用于测试内存分配和释放，并生成内存操作记录。
5. plot.cpp
   - 定义LifeBlock结构，表示一个内存块的生命周期（从分配到释放）。
   - 将记录的内存分配操作转换为LifeBlock，并收集所有已释放和未释放（在程序结束时仍存在）的内存块。
   - 使用SVG格式绘制内存生命周期图，横轴为时间，纵轴为内存块（按大小堆叠）。
   - 每个内存块的颜色根据其分配和释放的调用地址生成渐变，以区分不同的调用来源。
   - 支持生成HTML文件，内嵌SVG，并包含缩放和拖拽功能。







# time

## muduotest

### 1. muduo_jd.cpp

> **muduo数学公式方法**和**chrono库方法toJulianDay()**实现了两种将日期转换为儒略日(Julian Day)的方法，并提供了性能测试框架



## ScopeProfiler

### 1. ScopeProfiler.h

> 使用RAII技术仿照C++benchmark 性能测试类





# moderncuda

> cuda学习，待看B站的视频

