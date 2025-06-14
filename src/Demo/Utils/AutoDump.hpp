#ifndef _AUTO_DUMP_HPP_
#define _AUTO_DUMP_HPP_
#include <iostream>
#include <format>
#include <syncstream>
#include <string>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <chrono>
#include <tuple>
#include <optional>
#include <variant>

// ========================= 核心模板类 =========================
template <typename T, typename OutputStrategy>
class AutoDump {
public:
    AutoDump(T&& obj, OutputStrategy strategy)
        : object(std::forward<T>(obj)),
        output_strategy(std::move(strategy))
    {
    }

    ~AutoDump() {
        output_strategy(object);
    }

private:
    T object;
    OutputStrategy output_strategy;
};

// ========================= 输出策略 =========================
// 策略1: cout输出
struct CoutStrategy {
    template <typename U>
    void operator()(const U& obj) const {
        std::cout << "cout: " << obj << std::endl;
    }
};

// 策略2: osyncstream线程安全输出
struct SyncStreamStrategy {
    explicit SyncStreamStrategy(std::ostream& os = std::cout) : stream(os) {}

    template <typename U>
    void operator()(const U& obj) const {
        std::osyncstream sync(stream);
        sync << "sync: " << obj << std::endl;
    }

    std::ostream& stream;
};

// 策略3: format格式化输出
struct FormatStrategy {
    explicit FormatStrategy() {}

    template <typename U>
    void operator()(const U& obj) const {
        std::cout << std::format("fmt: {}", obj) << std::endl;
    }
};

// 策略4: 时间戳输出
struct TimestampStrategy {
    explicit TimestampStrategy(std::string fmt = "{}") : format_str(std::move(fmt)) {}

    template <typename U>
    void operator()(const U& obj) const {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::tm tm_buf;
        localtime_s(&tm_buf, &time);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;
        
        std::cout << std::format("{:04d}-{:02d}-{:02d} {:02d}:{:02d}:{:02d}.{:03d} | ", 
            tm_buf.tm_year + 1900, tm_buf.tm_mon + 1, tm_buf.tm_mday,
            tm_buf.tm_hour, tm_buf.tm_min, tm_buf.tm_sec, ms.count());
        std::cout << std::format(format_str, obj) << std::endl;
    }

    std::string format_str;
};

// 策略5: 调试信息输出
///< __func__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前函数的未修饰名称（const char[]）。
///     作用域：仅在函数体内有效。
///< __FUNCTION__
///     标准：MSVC扩展（非标准）。
///     返回值：当前函数的未修饰名称（const char[]）。
///     作用域：仅在函数体内有效。
///< __FUNCTIONW__
///     标准：MSVC扩展。
///     返回值：当前函数的未修饰名称的宽字符版本（const wchar_t[]）。
///     作用域：仅在函数体内有效。
///< __FUNCSIG__
///     标准：MSVC扩展。
///     返回值：当前函数的完整签名（包括返回类型、参数列表等），格式为const char[]。
///     示例："void __cdecl myFunction(int)"。
///     作用域：仅在函数体内有效。
///< __FUNCDNAME__
///     说明：MSVC扩展（非标准）。
///     返回函数的修饰名称（mangled name），格式为const char[]。
///     用于链接器层面的标识（如："?myFunction@@YAXH@Z"）。
///< __FILE__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前源文件名（const char[]）。
///     作用域：仅在函数体内有效。
///< __LINE__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前源文件行号（int）。
///     作用域：仅在函数体内有效。
///< __COUNTER__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：一个唯一的整型值，每次调用递增1。
///     作用域：仅在函数体内有效。
///< __TIME__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前系统时间（const char[]）。
///     作用域：仅在函数体内有效。
///< __DATE__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前系统日期（const char[]）。
///     作用域：仅在函数体内有效。
///< __TIMESTAMP__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前系统时间戳（const char[]）。
///     作用域：仅在函数体内有效。
///< __VA_ARGS__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：可变参数的个数。
///     作用域：仅在函数体内有效。
///< __PRETTY_FUNCTION__
///     标准：C++11引入的预定义标识符（非宏）。
///     返回值：当前函数的可读形式（const char[]）。
///     作用域：仅在函数体内有效。
struct DebugStrategy {
    explicit DebugStrategy(const char* file, int line, const char* func)
        : file_name(file), line_number(line), function_name(func) {}

    template <typename U>
    void operator()(const U& obj) const {
        std::cout << std::format("[DEBUG] {}:{} in {} | ", file_name, line_number, function_name);
        std::cout << obj << std::endl;
    }

    const char* file_name;
    int line_number;
    const char* function_name;
};

// ========================= 容器特化输出 =========================
// 容器输出辅助函数
template <typename Container>
struct ContainerPrinter {
    static std::string print(const Container& container) {
        std::string result = "{";
        bool first = true;
        for (const auto& item : container) {
            if (!first) result += ", ";
            result += std::to_string(item);
            first = false;
        }
        result += "}";
        return result;
    }
};

// 字符串容器特化
template <typename Container>
struct StringContainerPrinter {
    static std::string print(const Container& container) {
        std::string result = "{";
        bool first = true;
        for (const auto& item : container) {
            if (!first) result += ", ";
            result += "\"" + std::string(item) + "\"";
            first = false;
        }
        result += "}";
        return result;
    }
};

// 自定义类型容器特化 - 需要实现 to_string 方法
template <typename T>
concept Stringable = requires(T t) {
    { std::to_string(t) } -> std::convertible_to<std::string>;
};

// 容器输出策略
struct ContainerStrategy {
    template <typename Container>
    void operator()(const Container& container) const {
        std::cout << "container: " << ContainerPrinter<Container>::print(container) << std::endl;
    }
};

// ========================= 宏定义简化调用 =========================
// 使用 __COUNTER__ 和 __LINE__ 组合生成唯一变量名
#define AUTO_DUMP_CONCAT2(a, b, c) a##b##c
#define AUTO_DUMP_CONCAT(a, b, c) AUTO_DUMP_CONCAT2(a, b, c)

// 宏定义：使用cout输出
#define AUTO_DUMP_COUT(obj) \
    auto AUTO_DUMP_CONCAT(AD_COUT_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), CoutStrategy{} }

// 宏定义：使用osyncstream输出
#define AUTO_DUMP_SYNC(obj, stream) \
    auto AUTO_DUMP_CONCAT(AD_SYNC_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), SyncStreamStrategy{stream} }

// 宏定义：使用format输出
#define AUTO_DUMP_FORMAT(obj, fmt) \
    auto AUTO_DUMP_CONCAT(AD_FMT_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), FormatStrategy{} }

// 宏定义：使用时间戳输出
#define AUTO_DUMP_TIME(obj, fmt) \
    auto AUTO_DUMP_CONCAT(AD_TIME_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), TimestampStrategy{fmt} }

// 宏定义：使用调试信息输出
#define AUTO_DUMP_DEBUG(obj) \
    auto AUTO_DUMP_CONCAT(AD_DBG_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), DebugStrategy{__FILE__, __LINE__, __func__} }

// 宏定义：容器输出
#define AUTO_DUMP_CONTAINER(obj) \
    auto AUTO_DUMP_CONCAT(AD_CONT_, __LINE__, __COUNTER__) = AutoDump { std::move(obj), ContainerStrategy{} }

#endif // !_AUTO_DUMP_HPP_

/*
Use Example
struct Point {
    int x, y;
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "Point(" << p.x << "," << p.y << ")";
    }
};
void test_autodump() {
    // 基本类型
    AUTO_DUMP_COUT(42);
    AUTO_DUMP_FORMAT(3.14, "PI = {}");

    
    // 时间戳输出
    //AUTO_DUMP_TIME("程序启动", "{}");
    // 调试信息
    //AUTO_DUMP_DEBUG("这是一个调试消息");
    //
    // 容器输出
    std::vector<int> vec{ 1, 2, 3, 4, 5 };
    AUTO_DUMP_CONTAINER(vec);

    std::map<std::string, int> scores{ {"Alice", 95}, {"Bob", 87} };
    AUTO_DUMP_FORMAT(scores.size(), "学生数量: {}");
}
*/