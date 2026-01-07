# spdlog

## 使用source_loc乱码

### 场景复现

C++包装spdlog，声明为 C_ABI 给 C#使用P/INVOKE调用的时候，如果pattern设置为
`"%^[%Y-%m-%d %T.%e] [tid: %t] [%l]: %v {File: %s Func: %! Line: %#}%$"`

并且使用了`spdlog::source_loc`

```C++
struct source_loc {
    SPDLOG_CONSTEXPR source_loc() = default;
    SPDLOG_CONSTEXPR source_loc(const char *filename_in, int line_in, const char *funcname_in)
        : filename{filename_in},
          line{line_in},
          funcname{funcname_in} {}

    SPDLOG_CONSTEXPR bool empty() const SPDLOG_NOEXCEPT { return line <= 0; }
    const char *filename{nullptr};
    int line{0};
    const char *funcname{nullptr};
};
```

C# call ABI

```c#
private static extern void logger_trace(
    [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
    [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
    [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
    int line);
```

使用 `LPUTF8Str` UTF-8作为信息传输，那么最终得到的txt文件将会是，ANSI编码，因为`spdlog::source_loc`原因造成，即使C#端传递UTF-8编码给C++，即使你在ABI内使用 STD U8 进行处理，仍旧会是 乱码输出，可能spdlog的内部处理文件、函数统一使用了ASCII，如果不使用source_loc那么控制台以及txt文件得到都是UTF-8编码的信息，没有乱码，为此需要重构设计

### 问题背景

在使用spdlog日志库时，开发者Ducatel遇到了一个异步日志记录下的内存问题。当使用异步日志记录器(async logger)配合rotating_file sink和source_loc功能时，输出的日志中文件名和函数名出现了乱码现象，而同步日志记录则表现正常。

### 问题现象

在异步日志模式下，日志输出中本应显示文件名和函数名的位置出现了乱码字符，如"ùùùùùù"和"µµµµµµµµ"等。而在同步日志模式下，这些信息能够正确显示为实际的文件路径和函数名。

### 问题分析

这个问题本质上是一个内存生命周期管理问题。spdlog的source_loc结构体默认情况下只保存了指向字符串的指针，而没有进行深拷贝。当使用异步日志时，日志消息会被放入队列，在另一个线程中被处理。此时原始字符串可能已经超出了作用域或被释放，导致指针失效，从而读取到无效内存。

### 技术细节

spdlog的source_loc结构体默认实现非常简单，它只保存了三个成员：

    文件名指针
    行号
    函数名指针

当这些指针被传递给异步日志器时，原始字符串可能已经失效，但指针仍然被后续的日志处理线程使用，导致内存访问违规。

### 解决方案

开发者提出了一个修改版的source_loc实现，通过深拷贝来确保字符串数据的生命周期足够长。这个方案的关键点包括：

    在构造函数中进行字符串的深拷贝
    实现拷贝构造函数和赋值运算符
    在析构函数中释放分配的内存

这种实现虽然解决了问题，但增加了内存管理的复杂性。根据spdlog维护者的意见，这种改动会增加代码复杂度，而source_loc的这种高级用法并不是常见需求。

### 实际应用建议

对于大多数开发者，建议采用以下替代方案：

    将文件信息和函数信息直接放入日志消息体中，而不是依赖source_loc
    如果必须使用source_loc，确保相关字符串的生命周期足够长
    考虑使用静态字符串或字符串字面量作为source_loc参数

### 总结

spdlog的异步日志功能在性能上有明显优势，但在使用source_loc这类功能时需要特别注意内存管理问题。理解日志记录过程中数据的生命周期对于正确使用高级日志功能至关重要。对于大多数场景，简单的日志消息格式化已经足够，而无需过度依赖source_loc这样的高级特性。

这个案例也提醒我们，在使用任何日志库时，都应该充分理解其内部工作机制，特别是在涉及多线程和异步操作时，要特别注意数据生命周期的管理。


## 问题解决

### 手动解析需要处理大量的 匹配占位符

遍历pattern，构建占位符，使用std::format运行期间拼接字符串即可

#### 动态拼接类以及性能测试比较

```C++
#include <iostream>
#include <string>
#include <string_view>
#include <chrono>
#include <vector>
#include <iomanip>
#include <format>
#include <array>

class DynamicFormatter {
private:
    static constexpr std::string_view PLACEHOLDERS[] = { "%v", "%s", "%!", "%#" };
    static constexpr size_t MAX_PLACEHOLDERS = 4;

    struct PlaceholderInfo {
        size_t position;
        int index;
    };

public:
    static auto build_format_string(std::string_view pattern,
        std::string_view msg,
        std::string_view file,
        std::string_view func,
        int line) -> std::string {
        std::string result;
        result.reserve(pattern.size() + 32);

        std::array<PlaceholderInfo, MAX_PLACEHOLDERS> found_placeholders;
        size_t placeholder_count = 0;

        for (size_t i = 0; i < pattern.size(); ) {
            if (pattern[i] == '%' && i + 1 < pattern.size()) {
                char next_char = pattern[i + 1];
                int placeholder_index = get_placeholder_index(next_char);

                if (placeholder_index != -1) {
                    found_placeholders[placeholder_count++] = { i, placeholder_index };
                    i += 2;
                    continue;
                }
            }
            i++;
        }

        if (placeholder_count == 0) {
            return std::string(pattern);
        }

        size_t last_pos = 0;
        for (size_t i = 0; i < placeholder_count; ++i) {
            const auto& info = found_placeholders[i];
            result.append(pattern.substr(last_pos, info.position - last_pos));
            result.append(std::format("{{{}}}", info.index));
            last_pos = info.position + 2;
        }

        if (last_pos < pattern.size()) {
            result.append(pattern.substr(last_pos));
        }

        return result;
    }

    static auto format_message(std::string_view pattern,
        std::string_view msg,
        std::string_view file,
        std::string_view func,
        int line) -> std::string {
        std::string format_str = build_format_string(pattern, msg, file, func, line);
        return std::vformat(format_str, std::make_format_args(msg, file, func, line));
    }

private:
    static constexpr int get_placeholder_index(char c) {
        switch (c) {
        case 'v': return 0;
        case 's': return 1;
        case '!': return 2;
        case '#': return 3;
        default: return -1;
        }
    }
};

class FastDynamicFormatter {
private:
    static constexpr size_t BUFFER_SIZE = 256;

public:
    static std::string build_and_format(std::string_view pattern,
        std::string_view msg,
        std::string_view file,
        std::string_view func,
        int line) {
        char buffer[BUFFER_SIZE];
        char* ptr = buffer;
        const char* end = buffer + BUFFER_SIZE - 1;

        const char* pattern_ptr = pattern.data();
        const char* pattern_end = pattern.data() + pattern.size();

        while (pattern_ptr < pattern_end && ptr < end) {
            if (pattern_ptr + 1 < pattern_end && pattern_ptr[0] == '%') {
                switch (pattern_ptr[1]) {
                case 'v':
                    ptr = append_string(ptr, end, msg);
                    pattern_ptr += 2;
                    continue;
                case 's':
                    ptr = append_string(ptr, end, file);
                    pattern_ptr += 2;
                    continue;
                case '!':
                    ptr = append_string(ptr, end, func);
                    pattern_ptr += 2;
                    continue;
                case '#':
                    ptr = append_number(ptr, end, line);
                    pattern_ptr += 2;
                    continue;
                }
            }
            *ptr++ = *pattern_ptr++;
        }

        *ptr = '\0';
        return std::string(buffer, ptr - buffer);
    }

private:
    static char* append_string(char* ptr, const char* end, std::string_view str) {
        size_t available = end - ptr;
        size_t copy_size = std::min(str.size(), available);
        std::char_traits<char>::copy(ptr, str.data(), copy_size);
        return ptr + copy_size;
    }

    static char* append_number(char* ptr, const char* end, int value) {
        if (value == 0) {
            if (ptr < end) *ptr++ = '0';
            return ptr;
        }

        char temp[16];
        char* temp_ptr = temp + 15;
        *temp_ptr = '\0';

        int num = value;
        bool negative = num < 0;
        if (negative) num = -num;

        do {
            *--temp_ptr = '0' + (num % 10);
            num /= 10;
        } while (num > 0);

        if (negative && temp_ptr > temp) {
            *--temp_ptr = '-';
        }

        return append_string(ptr, end, std::string_view(temp_ptr, temp + 15 - temp_ptr));
    }
};

// 性能测试工具
class PerformanceTester {
public:
    template<typename Func>
    static long long measure_time(Func&& func, int iterations = 100000) {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            func();
        }

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    static void print_result(const std::string& test_name, long long time1, long long time2,
        const std::string& formatter1, const std::string& formatter2) {
        std::cout << std::left << std::setw(40) << test_name
            << std::setw(15) << formatter1 << ": " << time1 << " μs\n"
            << std::setw(40) << ""
            << std::setw(15) << formatter2 << ": " << time2 << " μs\n"
            << std::setw(40) << ""
            << std::setw(15) << "Speedup" << ": "
            << std::fixed << std::setprecision(2)
            << (static_cast<double>(time1) / time2) << "x\n"
            << std::string(60, '-') << "\n";
    }
};

// 测试用例
class TestCases {
public:
    struct TestCase {
        std::string name;
        std::string pattern;
        std::string msg;
        std::string file;
        std::string func;
        int line;
    };

    static std::vector<TestCase> get_test_cases() {
        return {
            // 测试用例1: 简单消息
            {"Simple message",
             "Error: %v",
             "File not found", "main.cpp", "main", 42},

             // 测试用例2: 完整信息
             {"Full information",
              "[%s] %v in %! at line %#",
              "Division by zero", "math.cpp", "calculate", 156},

              // 测试用例3: 复杂格式
              {"Complex format",
               "Function: %!, File: %s, Line: %#, Message: %v",
               "Invalid argument", "utils.hpp", "parse_input", 89},

               // 测试用例4: 重复占位符
               {"Repeated placeholders",
                "%v - %v - %v",
                "Same message", "test.cpp", "test_func", 10},

                // 测试用例5: 长字符串
                {"Long strings",
                 "File %s reported error %v in function %! at line %# with details",
                 "This is a very long error message that describes the problem in detail",
                 "very_long_file_name_that_might_cause_issues.cpp",
                 "very_long_function_name_that_should_be_handled_properly",
                 999},

                 // 测试用例6: 无占位符
                 {"No placeholders",
                  "Static error message",
                  "msg", "file.cpp", "func", 1},

                  // 测试用例7: 混合内容
                  {"Mixed content",
                   "DEBUG: %s:%# [%!] %v",
                   "User authentication failed", "auth.cpp", "login", 203},

                   // 测试用例8: 边缘情况 - 空字符串
                   {"Empty strings",
                    "%s: %v",
                    "", "", "empty_func", 0},

                    // 测试用例9: 只有占位符
                    {"Only placeholders",
                     "%v%s%!%#",
                     "msg", "file", "func", 123},

                     // 测试用例10: 大量文本
                     {"High volume text",
                      "Starting process... File: %s, Function: %!, Line: %#, Status: %v",
                      "Initialization completed successfully with all modules loaded and verified",
                      "application/core/initialization/module_manager.cpp",
                      "initialize_application_modules",
                      42}
        };
    }
};

// 验证两个格式化器输出是否相同
bool validate_outputs() {
    auto test_cases = TestCases::get_test_cases();

    for (const auto& test_case : test_cases) {
        std::string result1 = DynamicFormatter::format_message(
            test_case.pattern, test_case.msg, test_case.file, test_case.func, test_case.line);

        std::string result2 = FastDynamicFormatter::build_and_format(
            test_case.pattern, test_case.msg, test_case.file, test_case.func, test_case.line);

        // 由于FastDynamicFormatter可能会截断，我们只检查前缀是否匹配
        if (result1.substr(0, result2.size()) != result2 && result2.substr(0, result1.size()) != result1) {
            std::cout << "Output mismatch for test: " << test_case.name << "\n";
            std::cout << "DynamicFormatter: " << result1 << "\n";
            std::cout << "FastDynamicFormatter: " << result2 << "\n";
            return false;
        }
    }
    return true;
}

int main() {
    std::cout << "Performance Comparison: DynamicFormatter vs FastDynamicFormatter\n";
    std::cout << "================================================================\n\n";

    // 首先验证输出一致性
    std::cout << "Validating output consistency... ";
    if (validate_outputs()) {
        std::cout << "PASSED\n\n";
    }
    else {
        std::cout << "FAILED - outputs differ between formatters\n\n";
    }

    auto test_cases = TestCases::get_test_cases();
    const int iterations = 100000;

    for (const auto& test_case : test_cases) {
        // 测试 DynamicFormatter
        long long time_dynamic = PerformanceTester::measure_time([&]() {
            auto result = DynamicFormatter::format_message(
                test_case.pattern, test_case.msg, test_case.file, test_case.func, test_case.line);
            volatile int i = result.size();
            // 防止优化
            //__asm volatile("" : : "r,m"(result) : "memory");
            }, iterations);

        // 测试 FastDynamicFormatter
        long long time_fast = PerformanceTester::measure_time([&]() {
            auto result = FastDynamicFormatter::build_and_format(
                test_case.pattern, test_case.msg, test_case.file, test_case.func, test_case.line);
            volatile int i = result.size();
            // 防止优化
            //__asm volatile("" : : "r,m"(result) : "memory");
            }, iterations);

        PerformanceTester::print_result(test_case.name, time_dynamic, time_fast,
            "DynamicFormatter", "FastDynamicFormatter");
    }

    // 总体统计
    std::cout << "\nSUMMARY:\n";
    std::cout << "FastDynamicFormatter 在大多数情况下应该比 DynamicFormatter 快，因为:\n";
    std::cout << "1. 单次扫描 vs 两次扫描\n";
    std::cout << "2. 栈上分配 vs 堆分配\n";
    std::cout << "3. 直接字符串操作 vs std::format 开销\n";
    std::cout << "4. 避免构建中间格式字符串\n";

    return 0;
}
```



```c++
#include <string>
#include <string_view>
#include <array>
#include <format>
#include <iostream>
#include <chrono>
#include <iomanip>
#include <vector>
#include <mutex>
#include <shared_mutex>
#include <unordered_map>

class DynamicFormatter {
private:
    static constexpr std::string_view PLACEHOLDERS[] = { "%v", "%s", "%!", "%#" };
    static constexpr size_t MAX_PLACEHOLDERS = 4;

    struct PlaceholderInfo {
        size_t position;
        int index;
    };

    // 缓存不同pattern的格式字符串
    inline static std::unordered_map<std::string, std::string> format_cache;
    inline static std::shared_mutex cache_mutex;

public:
    static auto build_format_string(std::string_view pattern) -> std::string {
        // 检查缓存
        {
            std::shared_lock lock(cache_mutex);
            auto it = format_cache.find(std::string(pattern));
            if (it != format_cache.end()) {
                return it->second;
            }
        }

        std::string result;
        result.reserve(pattern.size() + 32);

        std::array<PlaceholderInfo, MAX_PLACEHOLDERS> found_placeholders;
        size_t placeholder_count = 0;

        for (size_t i = 0; i < pattern.size(); ) {
            if (pattern[i] == '%' && i + 1 < pattern.size()) {
                char next_char = pattern[i + 1];
                int placeholder_index = get_placeholder_index(next_char);

                if (placeholder_index != -1) {
                    found_placeholders[placeholder_count++] = { i, placeholder_index };
                    i += 2;
                    continue;
                }
            }
            i++;
        }

        if (placeholder_count == 0) {
            result = std::string(pattern);
        }
        else {
            size_t last_pos = 0;
            for (size_t i = 0; i < placeholder_count; ++i) {
                const auto& info = found_placeholders[i];
                result.append(pattern.substr(last_pos, info.position - last_pos));
                result.append(std::format("{{{}}}", info.index));
                last_pos = info.position + 2;
            }

            if (last_pos < pattern.size()) {
                result.append(pattern.substr(last_pos));
            }
        }

        // 存入缓存
        {
            std::unique_lock lock(cache_mutex);
            format_cache[std::string(pattern)] = result;
        }

        return result;
    }

    static auto format_message(std::string_view pattern,
        std::string_view msg,
        std::string_view file,
        std::string_view func,
        int line) -> std::string {
        std::string format_str = build_format_string(pattern);
        return std::vformat(format_str, std::make_format_args(msg, file, func, line));
    }

private:
    static constexpr int get_placeholder_index(char c) {
        switch (c) {
        case 'v': return 0;
        case 's': return 1;
        case '!': return 2;
        case '#': return 3;
        default: return -1;
        }
    }
};

class FastDynamicFormatter {
private:
    static constexpr size_t BUFFER_SIZE = 256;

public:
    static std::string build_and_format(std::string_view pattern,
        std::string_view msg,
        std::string_view file,
        std::string_view func,
        int line) {
        char buffer[BUFFER_SIZE];
        char* ptr = buffer;
        const char* end = buffer + BUFFER_SIZE - 1;

        const char* pattern_ptr = pattern.data();
        const char* pattern_end = pattern.data() + pattern.size();

        while (pattern_ptr < pattern_end && ptr < end) {
            if (pattern_ptr + 1 < pattern_end && pattern_ptr[0] == '%') {
                switch (pattern_ptr[1]) {
                case 'v':
                    ptr = append_string(ptr, end, msg);
                    pattern_ptr += 2;
                    continue;
                case 's':
                    ptr = append_string(ptr, end, file);
                    pattern_ptr += 2;
                    continue;
                case '!':
                    ptr = append_string(ptr, end, func);
                    pattern_ptr += 2;
                    continue;
                case '#':
                    ptr = append_number(ptr, end, line);
                    pattern_ptr += 2;
                    continue;
                }
            }
            *ptr++ = *pattern_ptr++;
        }

        *ptr = '\0';
        return std::string(buffer, ptr - buffer);
    }

private:
    static char* append_string(char* ptr, const char* end, std::string_view str) {
        size_t available = end - ptr;
        size_t copy_size = std::min(str.size(), available);
        std::char_traits<char>::copy(ptr, str.data(), copy_size);
        return ptr + copy_size;
    }

    static char* append_number(char* ptr, const char* end, int value) {
        if (value == 0) {
            if (ptr < end) *ptr++ = '0';
            return ptr;
        }

        char temp[16];
        char* temp_ptr = temp + 15;
        *temp_ptr = '\0';

        int num = value;
        bool negative = num < 0;
        if (negative) num = -num;

        do {
            *--temp_ptr = '0' + (num % 10);
            num /= 10;
        } while (num > 0);

        if (negative && temp_ptr > temp) {
            *--temp_ptr = '-';
        }

        return append_string(ptr, end, std::string_view(temp_ptr, temp + 15 - temp_ptr));
    }
};

// 新增：优化的FastDynamicFormatter，支持pattern预编译
class OptimizedFastFormatter {
private:
    struct PatternSegment {
        enum Type { TEXT, PLACEHOLDER_MSG, PLACEHOLDER_FILE, PLACEHOLDER_FUNC, PLACEHOLDER_LINE };
        Type type;
        std::string text;
    };

    std::vector<PatternSegment> segments;
    bool compiled = false;

public:
    void compile_pattern(std::string_view pattern) {
        segments.clear();

        for (size_t i = 0; i < pattern.size(); ) {
            if (pattern[i] == '%' && i + 1 < pattern.size()) {
                PatternSegment segment;
                switch (pattern[i + 1]) {
                case 'v': segment.type = PatternSegment::PLACEHOLDER_MSG; break;
                case 's': segment.type = PatternSegment::PLACEHOLDER_FILE; break;
                case '!': segment.type = PatternSegment::PLACEHOLDER_FUNC; break;
                case '#': segment.type = PatternSegment::PLACEHOLDER_LINE; break;
                default:
                    // 不是有效占位符，当作普通文本
                    segment.type = PatternSegment::TEXT;
                    segment.text.push_back(pattern[i]);
                    i++;
                    continue;
                }
                segments.push_back(segment);
                i += 2;
            }
            else {
                // 普通文本段
                PatternSegment segment;
                segment.type = PatternSegment::TEXT;
                size_t start = i;
                while (i < pattern.size() && !(pattern[i] == '%' && i + 1 < pattern.size() &&
                    (pattern[i + 1] == 'v' || pattern[i + 1] == 's' || pattern[i + 1] == '!' || pattern[i + 1] == '#'))) {
                    i++;
                }
                segment.text = std::string(pattern.substr(start, i - start));
                segments.push_back(segment);
            }
        }

        compiled = true;
    }

    std::string format(std::string_view msg, std::string_view file, std::string_view func, int line) {
        if (!compiled) {
            return "";
        }

        // 估算所需缓冲区大小
        size_t estimated_size = 0;
        for (const auto& segment : segments) {
            if (segment.type == PatternSegment::TEXT) {
                estimated_size += segment.text.size();
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_MSG) {
                estimated_size += msg.size();
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_FILE) {
                estimated_size += file.size();
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_FUNC) {
                estimated_size += func.size();
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_LINE) {
                estimated_size += 10; // 保守估计行号长度
            }
        }

        std::string result;
        result.reserve(estimated_size);

        for (const auto& segment : segments) {
            if (segment.type == PatternSegment::TEXT) {
                result.append(segment.text);
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_MSG) {
                result.append(msg);
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_FILE) {
                result.append(file);
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_FUNC) {
                result.append(func);
            }
            else if (segment.type == PatternSegment::PLACEHOLDER_LINE) {
                result.append(std::to_string(line));
            }
        }

        return result;
    }
};

// 性能测试工具
class PerformanceTester {
public:
    template<typename Func>
    static long long measure_time(Func&& func, int iterations = 100000) {
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < iterations; ++i) {
            func();
        }

        auto end = std::chrono::high_resolution_clock::now();
        return std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
    }

    static void print_result(const std::string& test_name, long long time1, long long time2, long long time3,
        const std::string& formatter1, const std::string& formatter2, const std::string& formatter3) {
        std::cout << std::left << std::setw(40) << test_name
            << std::setw(20) << formatter1 << ": " << time1 << " μs\n"
            << std::setw(40) << ""
            << std::setw(20) << formatter2 << ": " << time2 << " μs\n"
            << std::setw(40) << ""
            << std::setw(20) << formatter3 << ": " << time3 << " μs\n"
            << std::setw(40) << ""
            << std::setw(20) << "Fast/Dynamic Ratio" << ": "
            << std::fixed << std::setprecision(2)
            << (static_cast<double>(time1) / time2) << "x\n"
            << std::setw(40) << ""
            << std::setw(20) << "Optimized/Dynamic Ratio" << ": "
            << (static_cast<double>(time1) / time3) << "x\n"
            << std::string(80, '-') << "\n";
    }
};

// 测试用例 - 固定pattern，变化内容
class RealWorldTestCases {
public:
    struct TestCase {
        std::string name;
        std::string pattern;
        std::vector<std::tuple<std::string, std::string, std::string, int>> messages;
    };

    static std::vector<TestCase> get_test_cases() {
        return {
            {
                "Simple error pattern",
                "ERROR: %v",
                {
                    {"File not found", "main.cpp", "main", 42},
                    {"Permission denied", "file_io.cpp", "open_file", 156},
                    {"Out of memory", "allocator.cpp", "allocate", 89},
                    {"Invalid argument", "parser.cpp", "parse", 203},
                    {"Network timeout", "network.cpp", "connect", 315}
                }
            },
            {
                "Detailed log pattern",
                "[%s:%#] %! - %v",
                {
                    {"User login successful", "auth.cpp", "authenticate", 45},
                    {"Database query executed", "db.cpp", "execute_query", 128},
                    {"Cache miss occurred", "cache.cpp", "get_value", 76},
                    {"Request processed", "server.cpp", "handle_request", 234},
                    {"Configuration loaded", "config.cpp", "load_config", 51}
                }
            },
            {
                "Full debug pattern",
                "DEBUG %s %! %#: %v",
                {
                    {"Starting initialization", "app.cpp", "init", 25},
                    {"Loading modules", "module.cpp", "load_modules", 67},
                    {"Connecting to database", "db.cpp", "connect", 142},
                    {"Processing user input", "ui.cpp", "handle_input", 88},
                    {"Shutting down", "app.cpp", "shutdown", 205}
                }
            },
            {
                "Complex pattern with mixed content",
                "Function %! in file %s at line %# reported: %v",
                {
                    {"Division by zero", "math.cpp", "calculate", 156},
                    {"Null pointer dereference", "utils.cpp", "process_data", 89},
                    {"Index out of bounds", "array.cpp", "access_element", 42},
                    {"Type conversion error", "converter.cpp", "convert", 117},
                    {"Resource busy", "lock.cpp", "acquire_lock", 203}
                }
            }
        };
    }
};

// 验证输出一致性
bool validate_outputs() {
    auto test_cases = RealWorldTestCases::get_test_cases();

    for (const auto& test_case : test_cases) {
        // 预编译优化格式化器
        OptimizedFastFormatter optimized_formatter;
        optimized_formatter.compile_pattern(test_case.pattern);

        for (const auto& [msg, file, func, line] : test_case.messages) {
            std::string result1 = DynamicFormatter::format_message(test_case.pattern, msg, file, func, line);
            std::string result2 = FastDynamicFormatter::build_and_format(test_case.pattern, msg, file, func, line);
            std::string result3 = optimized_formatter.format(msg, file, func, line);

            if (result1 != result3) {
                std::cout << "Output mismatch for test: " << test_case.name << "\n";
                std::cout << "DynamicFormatter: " << result1 << "\n";
                std::cout << "OptimizedFastFormatter: " << result3 << "\n";
                return false;
            }
        }
    }
    return true;
}

int main() {
    std::cout << "Real-world Performance Comparison: Fixed Pattern, Variable Content\n";
    std::cout << "==================================================================\n\n";

    // 验证输出一致性
    std::cout << "Validating output consistency... ";
    if (validate_outputs()) {
        std::cout << "PASSED\n\n";
    }
    else {
        std::cout << "FAILED\n\n";
    }

    auto test_cases = RealWorldTestCases::get_test_cases();
    const int iterations = 100000;

    for (const auto& test_case : test_cases) {
        std::cout << "Testing pattern: \"" << test_case.pattern << "\"\n";

        // 预编译优化格式化器
        OptimizedFastFormatter optimized_formatter;
        optimized_formatter.compile_pattern(test_case.pattern);

        // 预热缓存
        if (!test_case.messages.empty()) {
            auto [msg, file, func, line] = test_case.messages[0];
            DynamicFormatter::format_message(test_case.pattern, msg, file, func, line);
        }

        // 测试 DynamicFormatter (带缓存)
        long long time_dynamic = PerformanceTester::measure_time([&]() {
            for (const auto& [msg, file, func, line] : test_case.messages) {
                auto result = DynamicFormatter::format_message(test_case.pattern, msg, file, func, line);
                volatile int i = result.size();
                //asm volatile("" : : "r,m"(result) : "memory");
            }
            }, iterations / test_case.messages.size());

        // 测试 FastDynamicFormatter (无缓存，每次解析)
        long long time_fast = PerformanceTester::measure_time([&]() {
            for (const auto& [msg, file, func, line] : test_case.messages) {
                auto result = FastDynamicFormatter::build_and_format(test_case.pattern, msg, file, func, line);
                volatile int i = result.size();
                //asm volatile("" : : "r,m"(result) : "memory");
            }
            }, iterations / test_case.messages.size());

        // 测试 OptimizedFastFormatter (预编译pattern)
        long long time_optimized = PerformanceTester::measure_time([&]() {
            for (const auto& [msg, file, func, line] : test_case.messages) {
                auto result = optimized_formatter.format(msg, file, func, line);
                volatile int i = result.size();
                //asm volatile("" : : "r,m"(result) : "memory");
            }
            }, iterations / test_case.messages.size());

        PerformanceTester::print_result(test_case.name, time_dynamic, time_fast, time_optimized,
            "DynamicFormatter", "FastDynamicFormatter", "OptimizedFastFormatter");
    }

    // 总结分析
    std::cout << "\nSUMMARY ANALYSIS:\n";
    std::cout << "在真实日志记录场景中（pattern固定，内容变化）：\n";
    std::cout << "1. DynamicFormatter: 使用std::format，有缓存，但仍有运行时开销\n";
    std::cout << "2. FastDynamicFormatter: 每次解析pattern，适合pattern频繁变化的场景\n";
    std::cout << "3. OptimizedFastFormatter: 预编译pattern，在固定pattern场景中性能最佳\n";
    std::cout << "\n推荐使用场景：\n";
    std::cout << "- 如果pattern在运行时频繁变化：使用FastDynamicFormatter\n";
    std::cout << "- 如果pattern在初始化后固定不变：使用OptimizedFastFormatter\n";
    std::cout << "- 如果需要标准库兼容性：使用DynamicFormatter\n";

    return 0;
}
```



### spdlog abi代码

```c++
#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string_view>
#include <memory>
#include <atomic>

namespace spdlog::abi {

typedef struct LoggerConfig {
    int level;
    int flush_level;
    int flush_interval;
    int truncate;
    int enable_source_loc;
    int thread_count;
    const char* log_path;
    const char* logger_name;
    const char* pattern;
} LoggerConfig;

namespace details {
#ifdef SPDLOG_WCHAR_FILENAMES
inline std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

inline std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}
#endif

class source_loc_dynamic_formatter {
private:
    static constexpr size_t BUFFER_SIZE = 256;

public:
    static std::string build_and_format(std::string_view pattern,
                                        std::string_view msg,
                                        std::string_view file,
                                        std::string_view func,
                                        int line) {
        char buffer[BUFFER_SIZE];
        char* ptr = buffer;
        const char* end = buffer + BUFFER_SIZE - 1;

        const char* pattern_ptr = pattern.data();
        const char* pattern_end = pattern.data() + pattern.size();

        while (pattern_ptr < pattern_end && ptr < end) {
            if (pattern_ptr + 1 < pattern_end && pattern_ptr[0] == '%') {
                switch (pattern_ptr[1]) {
                    case 'v':
                        ptr = append_string(ptr, end, msg);
                        pattern_ptr += 2;
                        continue;
                    case 's':
                        ptr = append_string(ptr, end, file);
                        pattern_ptr += 2;
                        continue;
                    case '!':
                        ptr = append_string(ptr, end, func);
                        pattern_ptr += 2;
                        continue;
                    case '#':
                        ptr = append_number(ptr, end, line);
                        pattern_ptr += 2;
                        continue;
                }
            }
            *ptr++ = *pattern_ptr++;
        }

        *ptr = '\0';
        return std::string(buffer, ptr - buffer);
    }

private:
    static char* append_string(char* ptr, const char* end, std::string_view str) {
        size_t available = end - ptr;
        size_t copy_size = std::min(str.size(), available);
        std::char_traits<char>::copy(ptr, str.data(), copy_size);
        return ptr + copy_size;
    }

    static char* append_number(char* ptr, const char* end, int value) {
        if (value == 0) {
            if (ptr < end) *ptr++ = '0';
            return ptr;
        }

        char temp[16];
        char* temp_ptr = temp + 15;
        *temp_ptr = '\0';

        int num = value;
        bool negative = num < 0;
        if (negative) num = -num;

        do {
            *--temp_ptr = '0' + (num % 10);
            num /= 10;
        } while (num > 0);

        if (negative && temp_ptr > temp) {
            *--temp_ptr = '-';
        }

        return append_string(ptr, end, std::string_view(temp_ptr, temp + 15 - temp_ptr));
    }
};

struct spin_mutex {
    std::atomic_bool flag{false};
    void lock() {
        bool old;
#if __cpp_lib_atomic_wait
        int retries = 1000;
        do {
            old = false;
            // 操作成功期望其他线程都读取得到，操作失败不同步
            if (flag.compare_exchange_weak(old, true, std::memory_order_acquire,
                                           std::memory_order_relaxed))
                // load barrier
                return;
        } while (--retries);
#endif
        do {
#if __cpp_lib_atomic_wait
            // fast-user-space mutex = futex (linux) SYS_futex
            flag.wait(true, std::memory_order_relaxed);  // wait until flag not true
#endif
            old = false;
        } while (!flag.compare_exchange_weak(old, true, std::memory_order_acquire,
                                             std::memory_order_relaxed));
        // load barrier
    }

    void unlock() {
        // store barrier
        flag.store(false, std::memory_order_release);
#if __cpp_lib_atomic_wait
        flag.notify_one();
#endif
    }
};

}  // namespace details

extern "C" {
// 导出函数声明
SPDLOG_API void init_logger(const LoggerConfig* config);
SPDLOG_API void flush_logger();
SPDLOG_API void shutdown_logger();
SPDLOG_API void logger_trace(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_debug(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_info(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_warn(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_error(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_critical(const char* msg, const char* file, const char* func, int line);
}

class logger_wrapper {
private:
    struct LoggerState {
        std::shared_ptr<spdlog::logger> logger;
        std::string pattern;
        bool enable_source_loc;
    };

public:
    static void init(const LoggerConfig* config) {
        if (!config) return;

        // 如果线程池已经初始化，跳过
        if (!_thread_pool_initialized) {
            spdlog::init_thread_pool(8192, config->thread_count);
            _thread_pool_initialized = true;
        }

        spdlog::flush_every(std::chrono::seconds(config->flush_interval));

#ifdef SPDLOG_WCHAR_FILENAMES
        spdlog::filename_t log_path = details::utf8_to_wstring(config->log_path);
#else
        spdlog::filename_t log_path = config->log_path;
#endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, config->truncate != 0);
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

        auto logger = std::make_shared<spdlog::async_logger>(config->logger_name, sinks.begin(),
                                                             sinks.end(), spdlog::thread_pool(),
                                                             spdlog::async_overflow_policy::block);

        // 存储pattern和enable_source_loc状态
        std::string pattern = config->pattern ? config->pattern : "";
        bool enable_source_loc = config->enable_source_loc != 0;

        if (!enable_source_loc) {
            pattern = replace_placeholders(pattern);
        }

        logger->set_pattern(pattern);
        logger->set_level(static_cast<spdlog::level::level_enum>(config->level));
        logger->flush_on(static_cast<spdlog::level::level_enum>(config->flush_level));

        // 存储logger状态
        auto state = std::make_shared<LoggerState>();
        state->logger = logger;
        state->pattern = pattern;
        state->enable_source_loc = enable_source_loc;

        // 注册logger
        spdlog::register_logger(logger);

        // 更新当前logger状态
        {
            std::lock_guard<details::spin_mutex> lock(_mutex);
            _logger_states[config->logger_name] = state;
            _current_logger_name = config->logger_name;
        }
    }

    static void flush() {
        auto logger = get_current_logger();
        if (logger) logger->flush();
    }

    static std::shared_ptr<spdlog::logger> get_current_logger() {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(_current_logger_name);
        if (it != _logger_states.end()) {
            return it->second->logger;
        }
        return nullptr;
    }

    static std::shared_ptr<LoggerState> get_current_logger_state() {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(_current_logger_name);
        if (it != _logger_states.end()) {
            return it->second;
        }
        return nullptr;
    }

    static void log_with_source_loc(spdlog::level::level_enum level,
                                    const char* msg,
                                    const char* file,
                                    const char* func,
                                    int line) {
        auto state = get_current_logger_state();
        if (!state || !state->logger) return;

        if (state->enable_source_loc) {
            // 使用动态格式化器
            auto formatted_msg = details::source_loc_dynamic_formatter::build_and_format(
                state->pattern, msg, file, func, line);
            state->logger->log(level, formatted_msg);
        } else {
            // 直接记录消息
            state->logger->log(level, msg);
        }
    }

private:
    static std::string replace_placeholders(const std::string& pattern) {
        std::string result;
        result.reserve(pattern.length());

        for (size_t i = 0; i < pattern.length(); ++i) {
            if (pattern[i] == '%' && i + 1 < pattern.length()) {
                char next_char = pattern[i + 1];
                if (next_char == 's' || next_char == '!' || next_char == '#') {
                    // 跳过源位置占位符
                    ++i;
                    continue;
                }
            }
            result += pattern[i];
        }
        return result;
    }

    inline static details::spin_mutex _mutex;
    inline static std::unordered_map<std::string, std::shared_ptr<LoggerState>> _logger_states;
    inline static std::string _current_logger_name;
    inline static bool _thread_pool_initialized = false;
};

// C接口实现
extern "C" {
SPDLOG_API void init_logger(const LoggerConfig* config) { logger_wrapper::init(config); }

SPDLOG_API void flush_logger() { logger_wrapper::flush(); }

SPDLOG_API void shutdown_logger() { spdlog::shutdown(); }

SPDLOG_API void logger_trace(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::trace, msg, file, func, line);
}

SPDLOG_API void logger_debug(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::debug, msg, file, func, line);
}

SPDLOG_API void logger_info(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::info, msg, file, func, line);
}

SPDLOG_API void logger_warn(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::warn, msg, file, func, line);
}

SPDLOG_API void logger_error(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::err, msg, file, func, line);
}

SPDLOG_API void logger_critical(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::critical, msg, file, func, line);
}
}

}  // namespace spdlog::abi
```

### 采用bool根据用户是否开启直接在msg后面拼接

## 最终构建代码

### C++

```c++
#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <string_view>
#include <memory>
#include <atomic>
#include <optional>

namespace spdlog::abi {

typedef struct LoggerConfig {
    int level;                // 日志级别
    int flush_level;          // 立即刷新级别
    int flush_interval;       // 刷新间隔(秒)
    int truncate;             // 是否截断文件 (bool)
    int enable_source_loc;    // 是否启用源位置信息 (bool)
    int thread_count;         // 线程池线程数
    int log_async;            // 是否启用异步记录
    const char* log_path;     // 日志文件路径
    const char* logger_name;  // 日志器名称
    const char* pattern;      // 日志格式模式
} LoggerConfig;

namespace details {
#ifdef SPDLOG_WCHAR_FILENAMES
inline std::wstring utf8_to_wstring(const std::string& str) {
    if (str.empty()) return L"";

    int size_needed = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), NULL, 0);
    std::wstring wstr(size_needed, 0);
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int)str.size(), &wstr[0], size_needed);
    return wstr;
}

inline std::string wstring_to_utf8(const std::wstring& wstr) {
    if (wstr.empty()) return "";
    int size_needed =
        WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), NULL, 0, NULL, NULL);
    std::string str(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &wstr[0], (int)wstr.size(), &str[0], size_needed, NULL, NULL);
    return str;
}
#endif

struct spin_mutex {
    std::atomic_bool flag{false};
    void lock() {
        bool old;
#if __cpp_lib_atomic_wait
        int retries = 1000;
        do {
            old = false;
            // 操作成功期望其他线程都读取得到，操作失败不同步
            if (flag.compare_exchange_weak(old, true, std::memory_order_acquire,
                                           std::memory_order_relaxed))
                // load barrier
                return;
        } while (--retries);
#endif
        do {
#if __cpp_lib_atomic_wait
            // fast-user-space mutex = futex (linux) SYS_futex
            flag.wait(true, std::memory_order_relaxed);  // wait until flag not true
#endif
            old = false;
        } while (!flag.compare_exchange_weak(old, true, std::memory_order_acquire,
                                             std::memory_order_relaxed));
        // load barrier
    }

    void unlock() {
        // store barrier
        flag.store(false, std::memory_order_release);
#if __cpp_lib_atomic_wait
        flag.notify_one();
#endif
    }
};

}  // namespace details

extern "C" {
// 导出函数声明
SPDLOG_API void init_logger(const LoggerConfig* config);
SPDLOG_API void flush_logger();
SPDLOG_API void flush_logger_by_name(const char* logger_name);
SPDLOG_API void shutdown_logger();
SPDLOG_API void set_current_logger(const char* logger_name);
SPDLOG_API int logger_exists(const char* logger_name);

// 默认使用当前logger的日志函数
SPDLOG_API void logger_trace(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_debug(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_info(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_warn(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_error(const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_critical(const char* msg, const char* file, const char* func, int line);

// 指定logger名称的日志函数
SPDLOG_API void logger_trace_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_debug_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_info_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_warn_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_error_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
SPDLOG_API void logger_critical_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line);
}

class logger_wrapper {
private:
    struct LoggerState {
        std::shared_ptr<spdlog::logger> logger;
        bool enable_source_loc = false;

        bool valid() const { return logger != nullptr; }
    };

public:
    static void init(const LoggerConfig* config) {
        if (!config) return;

        // 如果线程池已经初始化，跳过
        if (!_thread_pool_initialized) {
            spdlog::init_thread_pool(8192, config->thread_count);
            _thread_pool_initialized = true;
        }

        spdlog::flush_every(std::chrono::seconds(config->flush_interval));

#ifdef SPDLOG_WCHAR_FILENAMES
        spdlog::filename_t log_path = details::utf8_to_wstring(config->log_path);
#else
        spdlog::filename_t log_path = config->log_path;
#endif

        auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        auto file_sink =
            std::make_shared<spdlog::sinks::basic_file_sink_mt>(log_path, config->truncate != 0);
        std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

        std::shared_ptr<spdlog::logger> logger;
        if (config->log_async) {
            logger = std::make_shared<spdlog::async_logger>(config->logger_name, sinks.begin(),
                                                            sinks.end(), spdlog::thread_pool(),
                                                            spdlog::async_overflow_policy::block);
        } else {
            logger =
                std::make_shared<spdlog::logger>(config->logger_name, sinks.begin(), sinks.end());
        }

        // 设置日志格式
        std::string pattern = config->pattern ? config->pattern : "";
        logger->set_pattern(pattern);
        logger->set_level(static_cast<spdlog::level::level_enum>(config->level));
        logger->flush_on(static_cast<spdlog::level::level_enum>(config->flush_level));

        // 创建并存储logger状态
        LoggerState state;
        state.logger = logger;
        state.enable_source_loc = (config->enable_source_loc != 0);

        // 注册logger
        spdlog::register_logger(logger);

        // 更新当前logger状态
        {
            std::lock_guard<details::spin_mutex> lock(_mutex);
            _logger_states[config->logger_name] = state;
            _current_logger_name = config->logger_name;
        }
    }

    static void flush() {
        auto logger = get_current_logger();
        if (logger) {
            logger->flush();
        }
    }

    static void flush_by_name(const char* logger_name) {
        auto logger = get_logger_by_name(logger_name);
        if (logger) {
            logger->flush();
        }
    }

    static void set_current_logger(const char* logger_name) {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        if (_logger_states.find(logger_name) != _logger_states.end()) {
            _current_logger_name = logger_name;
        }
    }

    static int logger_exists(const char* logger_name) {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        return _logger_states.find(logger_name) != _logger_states.end() ? 1 : 0;
    }

    static std::shared_ptr<spdlog::logger> get_current_logger() {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(_current_logger_name);
        if (it != _logger_states.end() && it->second.valid()) {
            return it->second.logger;
        }
        return nullptr;
    }

    static std::shared_ptr<spdlog::logger> get_logger_by_name(const char* logger_name) {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(logger_name);
        if (it != _logger_states.end() && it->second.valid()) {
            return it->second.logger;
        }
        return nullptr;
    }

    static std::optional<LoggerState> get_current_logger_state() {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(_current_logger_name);
        if (it != _logger_states.end() && it->second.valid()) {
            return it->second;
        }
        return std::nullopt;
    }

    static std::optional<LoggerState> get_logger_state_by_name(const char* logger_name) {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        auto it = _logger_states.find(logger_name);
        if (it != _logger_states.end() && it->second.valid()) {
            return it->second;
        }
        return std::nullopt;
    }

    static void log_with_source_loc(spdlog::level::level_enum level,
                                    const char* msg,
                                    const char* file,
                                    const char* func,
                                    int line) {
        auto state_opt = get_current_logger_state();
        if (!state_opt.has_value()) return;

        const auto& state = state_opt.value();

        if (state.enable_source_loc) {
            // 如果启用了源位置信息，在消息后面拼接源位置信息
            std::string formatted_msg =
                std::format("{} {{File: {} Func: {} Line: {}}}", msg, file, func, line);
            state.logger->log(level, formatted_msg);
        } else {
            // 如果没有启用源位置信息，直接记录原始消息
            state.logger->log(level, msg);
        }
    }

    static void log_with_source_loc_by_name(const char* logger_name,
                                            spdlog::level::level_enum level,
                                            const char* msg,
                                            const char* file,
                                            const char* func,
                                            int line) {
        auto state_opt = get_logger_state_by_name(logger_name);
        if (!state_opt.has_value()) return;

        const auto& state = state_opt.value();

        if (state.enable_source_loc) {
            // 如果启用了源位置信息，在消息后面拼接源位置信息
            std::string formatted_msg =
                std::format("{} {{File: {} Func: {} Line: {}}}", msg, file, func, line);
            state.logger->log(level, formatted_msg);
        } else {
            // 如果没有启用源位置信息，直接记录原始消息
            state.logger->log(level, msg);
        }
    }

    static void shutdown() {
        std::lock_guard<details::spin_mutex> lock(_mutex);
        _logger_states.clear();
        _current_logger_name.clear();
        spdlog::shutdown();
        _thread_pool_initialized = false;
    }

private:
    inline static details::spin_mutex _mutex;
    inline static std::unordered_map<std::string, LoggerState> _logger_states;
    inline static std::string _current_logger_name;
    inline static bool _thread_pool_initialized = false;
};

// C接口实现
extern "C" {
SPDLOG_API void init_logger(const LoggerConfig* config) { logger_wrapper::init(config); }

SPDLOG_API void flush_logger() { logger_wrapper::flush(); }

SPDLOG_API void flush_logger_by_name(const char* logger_name) {
    logger_wrapper::flush_by_name(logger_name);
}

SPDLOG_API void shutdown_logger() { logger_wrapper::shutdown(); }

SPDLOG_API void set_current_logger(const char* logger_name) {
    logger_wrapper::set_current_logger(logger_name);
}

SPDLOG_API int logger_exists(const char* logger_name) {
    return logger_wrapper::logger_exists(logger_name);
}

// 使用当前logger的日志函数
SPDLOG_API void logger_trace(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::trace, msg, file, func, line);
}

SPDLOG_API void logger_debug(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::debug, msg, file, func, line);
}

SPDLOG_API void logger_info(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::info, msg, file, func, line);
}

SPDLOG_API void logger_warn(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::warn, msg, file, func, line);
}

SPDLOG_API void logger_error(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::err, msg, file, func, line);
}

SPDLOG_API void logger_critical(const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc(spdlog::level::critical, msg, file, func, line);
}

// 指定logger名称的日志函数
SPDLOG_API void logger_trace_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::trace, msg, file, func,
                                                line);
}

SPDLOG_API void logger_debug_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::debug, msg, file, func,
                                                line);
}

SPDLOG_API void logger_info_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::info, msg, file, func,
                                                line);
}

SPDLOG_API void logger_warn_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::warn, msg, file, func,
                                                line);
}

SPDLOG_API void logger_error_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::err, msg, file, func,
                                                line);
}

SPDLOG_API void logger_critical_by_name(
    const char* logger_name, const char* msg, const char* file, const char* func, int line) {
    logger_wrapper::log_with_source_loc_by_name(logger_name, spdlog::level::critical, msg, file,
                                                func, line);
}
}

}  // namespace spdlog::abi
```



### C#

```c#
using Aspose.CAD.FileFormats.Cgm;
using Aspose.Slides.Export.Web;
using System;
using System.Buffers;
using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;
using System.Text;

namespace WizConverter
{
    /// <summary>
    /// 日志级别枚举
    /// </summary>
    public enum LogLevel
    {
        Trace = 0,
        Debug = 1,
        Info = 2,
        Warn = 3,
        Error = 4,
        Critical = 5,
        Off = 6
    }

    /// <summary>
    /// 日志配置结构体
    /// </summary>
    [StructLayout(LayoutKind.Sequential, CharSet = CharSet.Ansi)]
    public struct LoggerConfig
    {
        public int level;              // 日志级别
        public int flush_level;        // 立即刷新级别
        public int flush_interval;     // 刷新间隔(秒)
        public int truncate;           // 是否截断文件 (bool)
        public int enable_source_loc;  // 是否启用源位置信息 (bool)
        public int thread_count;       // 线程池线程数
        public int log_async;          // 是否启用异步记录

        [MarshalAs(UnmanagedType.LPUTF8Str)]
        public string log_path;        // 日志文件路径

        [MarshalAs(UnmanagedType.LPUTF8Str)]
        public string logger_name;     // 日志器名称

        [MarshalAs(UnmanagedType.LPUTF8Str)]
        public string pattern;         // 日志格式模式
    }

    /// <summary>
    /// SpdLog封装类 - 提供高性能的日志记录功能
    /// </summary>
    public static class SpdLog
    {
        private const string DllPath = @"C:\Users\Re11a\Downloads\spdlog-1.16.0\out\build\x64-Release\spdlog.dll";
        private static bool _isInitialized = false;

        #region P/Invoke 声明

        [DllImport(DllPath, EntryPoint = "init_logger", CallingConvention = CallingConvention.Cdecl)]
        private static extern void init_logger(ref LoggerConfig config);

        [DllImport(DllPath, EntryPoint = "flush_logger", CallingConvention = CallingConvention.Cdecl)]
        private static extern void flush_logger();

        [DllImport(DllPath, EntryPoint = "flush_logger_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void flush_logger_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name);

        [DllImport(DllPath, EntryPoint = "shutdown_logger", CallingConvention = CallingConvention.Cdecl)]
        private static extern void shutdown_logger();

        [DllImport(DllPath, EntryPoint = "set_current_logger", CallingConvention = CallingConvention.Cdecl)]
        private static extern void set_current_logger(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name);

        [DllImport(DllPath, EntryPoint = "logger_exists", CallingConvention = CallingConvention.Cdecl)]
        private static extern int logger_exists(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name);

        // 默认使用当前logger的日志函数
        [DllImport(DllPath, EntryPoint = "logger_trace", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_trace(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_debug", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_debug(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_info", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_info(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_warn", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_warn(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_error", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_error(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_critical", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_critical(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        // 指定logger名称的日志函数
        [DllImport(DllPath, EntryPoint = "logger_trace_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_trace_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_debug_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_debug_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_info_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_info_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_warn_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_warn_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_error_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_error_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);

        [DllImport(DllPath, EntryPoint = "logger_critical_by_name", CallingConvention = CallingConvention.Cdecl)]
        private static extern void logger_critical_by_name(
            [MarshalAs(UnmanagedType.LPUTF8Str)] string logger_name,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string msg,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string file,
            [MarshalAs(UnmanagedType.LPUTF8Str)] string func,
            int line);
        #endregion

        #region 默认配置

        /// <summary>
        /// 默认日志配置
        /// </summary>
        public static readonly LoggerConfig DefaultConfig = new LoggerConfig
        {
            level = (int)LogLevel.Trace,
            flush_level = (int)LogLevel.Warn,
            flush_interval = 3,
            truncate = 1,
            enable_source_loc = 1, // 默认不启用源位置信息
            thread_count = 1,
            log_async = 1,
            log_path = "./Log.txt",
            logger_name = "default_logger",
            pattern = "%^[%Y-%m-%d %T.%e] [tid: %t] [%l]: %v%$"
        };

        #endregion

        #region 公共方法

        /// <summary>
        /// 初始化日志系统
        /// </summary>
        /// <param name="config">日志配置</param>
        public static void Initialize(LoggerConfig? config = null)
        {
            var actualConfig = config ?? DefaultConfig;
            init_logger(ref actualConfig);
            _isInitialized = true;
        }

        /// <summary>
        /// 刷新日志缓冲区
        /// </summary>
        public static void Flush()
        {
            if (_isInitialized)
                flush_logger();
        }

        /// <summary>
        /// 刷新指定logger的缓冲区
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        public static void Flush(string loggerName)
        {
            if (_isInitialized)
                flush_logger_by_name(loggerName);
        }

        /// <summary>
        /// 设置当前logger
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        public static void SetCurrentLogger(string loggerName)
        {
            if (_isInitialized)
                set_current_logger(loggerName);
        }

        /// <summary>
        /// 检查logger是否存在
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <returns>是否存在</returns>
        public static bool LoggerExists(string loggerName)
        {
            if (!_isInitialized) return false;
            return logger_exists(loggerName) != 0;
        }


        /// <summary>
        /// 关闭日志系统
        /// </summary>
        public static void Shutdown()
        {
            if (_isInitialized)
            {
                shutdown_logger();
                _isInitialized = false;
            }
        }

        #endregion

        #region 使用当前logger的日志记录方法

        /// <summary>
        /// 记录Trace级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Trace(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            //Encoding.UTF8.GetString(Encoding.UTF8.GetBytes(message));
            if (_isInitialized)
                logger_trace(message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 记录Debug级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Debug(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_debug(message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 记录Info级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Info(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_info(message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 记录Warn级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Warn(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_warn(message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 记录Error级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Error(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_error(message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 记录Critical级别日志
        /// </summary>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void Critical(
            string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_critical(message, filePath, memberName, lineNumber);
        }
        #endregion

        #region 指定logger名称的日志记录方法

        /// <summary>
        /// 使用指定logger记录Trace级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void TraceBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_trace_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 使用指定logger记录Debug级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void DebugBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_debug_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 使用指定logger记录Info级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void InfoBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_info_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 使用指定logger记录Warn级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void WarnBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_warn_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 使用指定logger记录Error级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void ErrorBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_error_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        /// <summary>
        /// 使用指定logger记录Critical级别日志
        /// </summary>
        /// <param name="loggerName">logger名称</param>
        /// <param name="message">日志消息</param>
        /// <param name="filePath">调用文件路径（自动填充）</param>
        /// <param name="memberName">调用成员名称（自动填充）</param>
        /// <param name="lineNumber">调用行号（自动填充）</param>
        public static void CriticalBy(string loggerName, string message,
            [CallerFilePath] string filePath = "",
            [CallerMemberName] string memberName = "",
            [CallerLineNumber] int lineNumber = 0)
        {
            if (_isInitialized)
                logger_critical_by_name(loggerName, message, filePath, memberName, lineNumber);
        }

        #endregion
    }

    /// <summary>
    /// 日志使用示例
    /// </summary>
    public static class LoggerExample
    {
        public static void Main()
        {
            try
            {
                // 初始化默认日志系统
                SpdLog.Initialize();

                // 记录不同级别的日志到默认logger
                SpdLog.Trace("这是一个Trace级别的日志");
                SpdLog.Info("这是一个Info级别的日志");

                // 自定义配置 - 创建第二个logger
                var customConfig = new LoggerConfig
                {
                    level = (int)LogLevel.Info,
                    flush_level = (int)LogLevel.Error,
                    flush_interval = 5,
                    truncate = 1,
                    enable_source_loc = 0,
                    thread_count = 2,
                    log_async = 0,
                    log_path = @".\CustomLog.txt",
                    logger_name = "custom_logger",
                    pattern = "%^[%Y-%m-%d %T] [%l] %v%$"
                };

                SpdLog.Initialize(customConfig);

                // 现在有两个logger：default_logger 和 custom_logger

                // 方法1：设置当前logger为custom_logger，然后使用默认方法记录
                SpdLog.SetCurrentLogger("custom_logger");
                SpdLog.Info("这条消息会记录到custom_logger");

                // 方法2：直接指定logger名称记录日志
                SpdLog.InfoBy("default_logger", "这条消息会记录到default_logger");
                SpdLog.InfoBy("custom_logger", "这条消息会记录到custom_logger");

                // 检查logger是否存在
                if (SpdLog.LoggerExists("default_logger"))
                {
                    SpdLog.InfoBy("default_logger", "default_logger存在");
                }

                if (SpdLog.LoggerExists("custom_logger"))
                {
                    SpdLog.InfoBy("custom_logger", "custom_logger存在");
                }

                // 分别刷新不同的logger
                SpdLog.Flush("default_logger");
                SpdLog.Flush("custom_logger");

                // 更多格式化示例
                SpdLog.ErrorBy("custom_logger", string.Format("文件 {0} 处理失败，错误代码: {1}", "example.pdf", 404));
                SpdLog.WarnBy("default_logger", string.Format("内存使用率: {0:P2}", 0.756));

                // 刷新所有logger的缓冲区
                SpdLog.Flush();
            }
            finally
            {
                // 关闭日志系统
                SpdLog.Shutdown();
            }
        }
    }
}
```

# 日志说明

以下是针对 C++20 环境下 `spdlog` 日志库的 **Pattern 标记符**最新说明，包含其含义、用法及注意事项，综合自官方文档和最新实践总结：

---

### 📌 一、核心 Pattern 标记符（最新完整列表）
Pattern 以 `%` 开头，支持控制日志输出的格式元素，以下是全部标记符及其含义：

| **标记** | **含义**                  | **示例输出**                                 | **使用场景**     |
| -------- | ------------------------- | -------------------------------------------- | ---------------- |
| `%v`     | 实际日志文本              | `"User logged in"`                           | 必需项，消息主体 |
| `%t`     | 线程 ID                   | `"1232"`                                     | 多线程调试       |
| `%P`     | 进程 ID                   | `"3456"`                                     | 多进程追踪       |
| `%n`     | Logger 名称               | `"network_logger"`                           | 区分不同日志源   |
| `%l`     | 日志级别（完整）          | `"info"`, `"error"`                          | 显示级别全称     |
| `%L`     | 日志级别（单字符）        | `"I"`, `"E"`                                 | 紧凑格式         |
| `%a`     | 星期缩写                  | `"Thu"`                                      | 时间信息         |
| `%A`     | 星期全称                  | `"Thursday"`                                 |                  |
| `%b`     | 月份缩写                  | `"Aug"`                                      |                  |
| `%B`     | 月份全称                  | `"August"`                                   |                  |
| `%Y`     | 四位年份                  | `"2025"`                                     |                  |
| `%m`     | 月份（数字）              | `"07"`                                       |                  |
| `%d`     | 日（数字）                | `"14"`                                       |                  |
| `%H`     | 24 小时制小时             | `"15"`                                       |                  |
| `%I`     | 12 小时制小时             | `"03"`                                       |                  |
| `%M`     | 分钟                      | `"45"`                                       |                  |
| `%S`     | 秒                        | `"30"`                                       |                  |
| `%e`     | 毫秒                      | `"678"`                                      | 高精度时间       |
| `%f`     | 微秒                      | `"056789"`                                   |                  |
| `%F`     | 纳秒                      | `"256789123"`                                |                  |
| `%p`     | AM/PM                     | `"PM"`                                       |                  |
| `%z`     | 时区偏移（ISO 8601）      | `"+08:00"`                                   | 跨时区日志       |
| `%r`     | 12 小时制时间（含 AM/PM） | `"03:45:30 PM"`                              |                  |
| `%R`     | 小时:分钟（24 制）        | `"15:45"`                                    |                  |
| `%T`     | 小时:分钟:秒（24 制）     | `"15:45:30"`                                 | 默认时间格式     |
| `%@`     | 源文件:行号               | `"/src/main.cpp:123"`                        | **调试定位**     |
| `%s`     | 源文件名（不含路径）      | `"main.cpp"`                                 |                  |
| `%g`     | 源文件全路径              | `"/project/src/main.cpp"`                    |                  |
| `%#`     | 行号                      | `"123"`                                      |                  |
| `%!`     | 函数名                    | `"my_function()"`                            |                  |
| `%^`     | **颜色范围开始**          | 后续内容着色（如错误红色）                   | 控制台颜色控制   |
| `%$`     | **颜色范围结束**          | 结束着色                                     | 需配对使用       |
| `%+`     | 默认格式                  | `"[2025-07-14 15:45:30.678] [info] Message"` | 快速恢复默认     |

---

### ⚠️ 二、关键注意事项与高级用法
#### 1. **源代码位置标记（`%@`, `%s`, `%#`, `%!`）** 
   - **必须** 在编译前定义宏以启用：  
     ```cpp
     #define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_TRACE  // 或 DEBUG/INFO 等
     ```
   - **必须** 使用宏输出日志（非普通函数）：  
     ```cpp
     SPDLOG_LOGGER_INFO(logger, "Login failed");  // 自动注入 __FILE__, __LINE__
     ```
   - 普通函数调用（如 `logger->info()`）**不会** 填充这些字段。

#### 2. **对齐与截断控制** 
   - **语法**：`%[对齐][宽度][!]标志`
     - 对齐：`-`（左对齐）、`=`（居中）、缺省（右对齐）
     - 宽度：数字（1-64），如 `%8l`
     - `!`：超宽时截断（否则保留）
   - **示例**：
     ```cpp
     set_pattern("%-10l %v");       // 左对齐级别，宽度10 → "[info     ] Message"
     set_pattern("%=5!l");          // 居中对齐，超宽截断 → "inf" (原为"info")
     ```

#### 3. **自定义格式化扩展** 
   可通过继承 `spdlog::custom_flag_formatter` 添加自定义标记：
   ```cpp
   class MyFormatter : public spdlog::custom_flag_formatter {
   public:
       void format(...) override { /* 实现逻辑 */ }
       std::unique_ptr<custom_flag_formatter> clone() const override { ... }
   };
   auto formatter = std::make_unique<spdlog::pattern_formatter>();
   formatter->add_flag<MyFormatter>('*');  // 注册新标记 %*
   logger->set_formatter(std::move(formatter));
   ```

---

### ⚙️ 三、C++20 下的特别注意事项
1. **`source_location` 替代宏**   
   C++20 可用 `std::source_location` 替代 `__FILE__` 等宏，需手动封装：
   ```cpp
   #include <source_location>
   void log_with_loc(std::string msg, const std::source_location& loc = std::source_location::current()) {
       logger->log(spdlog::source_loc{loc.file_name(), loc.line(), loc.function_name()}, ...);
   }
   ```

2. **异步模式与队列策略**   
   - 启用异步防止阻塞主线程：
     ```cpp
     spdlog::init_thread_pool(8192, 1);  // 队列大小=8192，线程数=1
     auto logger = spdlog::create_async<...>("logger");
     ```
   - 队列满时策略：
     - `block_retry`（默认）：阻塞直到空间可用。
     - `discard_log_msg`：丢弃新日志（高性能场景）。

3. **C++20 模块兼容性问题**   
   使用 MSVC 时若遇静态函数未定义错误（如 `to_12h`）：
   - **解决方案**：将 spdlog 编译为静态库（非 Header-Only），或避免使用 12 小时制格式。

---

### 💎 四、推荐实践格式示例
```cpp
// 完整调试格式（含线程、文件、函数、行号）
set_pattern("%^[%Y-%m-%d %T.%e] [%t] [%s:%# %!] [%l] %v%$");

// 生产环境精简格式
set_pattern("[%Y-%m-%d %H:%M:%S] [%l] %v");
```

> **提示**：完整标记符列表见 [spdlog Wiki](https://github.com/gabime/spdlog/wiki/3.-Custom-formatting)。启用源码位置需 **严格** 结合 `SPDLOG_ACTIVE_LEVEL` 与宏调用。
