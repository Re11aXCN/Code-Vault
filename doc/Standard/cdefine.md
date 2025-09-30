# 一、cassert

## 1. 介绍

**<u>断言设置的判断条件</u>**

### （1）源码

```c
    _ACRTIMP void __cdecl _wassert(
        _In_z_ wchar_t const* _Message,
        _In_z_ wchar_t const* _File,
        _In_   unsigned       _Line
        );

    #define assert(expression) ((void)(                                                       \
            (!!(expression)) ||                                                               \
            (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0)) \
        )

// 如果assert判断的expression是false，程序会中断abort
// 否则正常执行
```

### （2）static_assert（C++11）

* static_assert是C++11引入的一个关键字，编译期间进行检查判断条件，常用于模板元编程

* assert宏是运行时检查判断条件

## 2. 使用示例

```c
#include <iostream>
#include <cmath>
#include <cassert>

// 示例函数：计算平方根
double calculate_square_root(double value) {
    // 确保输入值非负
    assert(value >= 0.0 && "Input value must be non-negative");
    return std::sqrt(value);
}

// 示例函数：数组索引访问
int access_array_element(int arr[], size_t index, size_t size) {
    // 确保索引在有效范围内
    assert(index < size && "Array index out of bounds");
    return arr[index];
}

int main() {
    std::cout << "=== assert 宏使用示例 ===\n\n";
    
    // 示例1：正常的assert检查
    std::cout << "1. 正常assert检查:\n";
    int x = 5;
    assert(x == 5);
    std::cout << "   assert(x == 5) 通过\n\n";
    
    // 示例2：计算平方根
    std::cout << "2. 计算平方根:\n";
    double result = calculate_square_root(16.0);
    std::cout << "   √16 = " << result << " (assert通过)\n\n";
    
    // 示例3：数组访问
    std::cout << "3. 数组访问:\n";
    int arr[] = {10, 20, 30, 40, 50};
    size_t size = sizeof(arr) / sizeof(arr[0]);
    int element = access_array_element(arr, 2, size);
    std::cout << "   arr[2] = " << element << " (assert通过)\n\n";
    
    std::cout << "4. 断言失败示例 (已禁用，不会中断程序):\n";
    std::cout << "   以下示例在实际运行时会触发断言失败并终止程序\n";
    std::cout << "   // assert(x == 10);  // 这会触发断言失败\n";
    std::cout << "   // calculate_square_root(-1.0);  // 这会触发断言失败\n";
    std::cout << "   // access_array_element(arr, 10, size);  // 这会触发断言失败\n\n";
    
    std::cout << "5. 禁用assert:\n";
    std::cout << "   在包含 <cassert> 前定义 NDEBUG 宏可以禁用所有assert:\n";
    std::cout << "   #define NDEBUG\n";
    std::cout << "   #include <cassert>\n\n";
    
    std::cout << "6. assert与静态断言(static_assert)的区别:\n";
    std::cout << "   - assert: 运行时检查，可禁用\n";
    std::cout << "   - static_assert: 编译时检查，不可禁用\n\n";
    
    std::cout << "7. 最佳实践:\n";
    std::cout << "   - 使用assert验证假设和不变条件\n";
    std::cout << "   - 不要用于用户输入验证（应使用异常或错误代码）\n";
    std::cout << "   - 在关键算法中验证前置条件\n";
    std::cout << "   - 发布版本中通常禁用assert\n";
    
    return 0;
}
```

------



# 二、cctype

## 1. 介绍

本文件是用于**<u>关于`ascii码`的处理</u>**，详细如下：

### （1）字符分类函数

- `isalpha(int c)`: 检查字符**是否为字母** (a-z, A-Z)
- `isupper(int c)`: 检查字符**是否为大写字母** (A-Z)
- `islower(int c)`: 检查字符**是否为小写字母** (a-z)
- `isdigit(int c)`: 检查字符**是否为数字** (0-9)
- `isxdigit(int c)`: 检查字符**是否为十六进制数字** (0-9, a-f, A-F)
- `isspace(int c)`: 检查字符**是否为空白字符** (空格、制表符、换行符等)
- `ispunct(int c)`: 检查字符**是否为标点符号**
- `isalnum(int c)`: 检查字符**是否为字母或数字**
- `isprint(int c)`: 检查字符**是否为可打印字符**
- `isgraph(int c)`: 检查字符**是否为图形字符** (可打印字符，不包括空格)
- `iscntrl(int c)`: 检查字符**是否为控制字符**

### （2）字符转换函数

- `toupper(int c)`: 将字符转**换为大写**
- `tolower(int c)`: 将字符转**换为小写**
- `_tolower(int c)`: 直接转**换字符为大写 (<u>不检查</u>)**
- `_toupper(int c)`: 直接转**换字符为小写 (<u>不检查</u>)**

### （3）其他函数

- `_isctype(int c, int type)`: 使用特定类型掩码检查字符
- `__isascii(int c)`: 检查字符**是否为ASCII字符 (0-127)**
- `__toascii(int c)`: 将字符**转换为ASCII (取低7位)**
- `__iscsymf(int c)`: 检查字符是否可作为C标识符的起始字符
- `__iscsym(int c)`: 检查字符是否可作为C标识符的有效字符

### （4）区域设置相关函数

- 带`_l`后缀的函数 (如`_isalpha_l`) 允许指定区域设置进行处理

### （5）wide char

如果是宽字符，加上w即可，比如 `isdigit`   的宽字符 call  `iswdigit`

```c
// 定义header包含
#include <corert_wctype.h>
```

## 2. 使用示例

```c
#include <iostream>
#include <cctype>

int main() {
    printf("=== ctype.h 字符处理函数示例 ===\n\n");

    // 1. isalpha() - 检查字符是否为字母
    printf("1. isalpha() - 检查字符是否为字母:\n");
    printf("   isalpha('A') = %d\n", isalpha('A'));
    printf("   isalpha('5') = %d\n", isalpha('5'));
    printf("   isalpha('@') = %d\n\n", isalpha('@'));

    // 2. isupper() 和 islower() - 检查大小写
    printf("2. isupper() 和 islower() - 检查大小写:\n");
    printf("   isupper('A') = %d\n", isupper('A'));
    printf("   isupper('a') = %d\n", isupper('a'));
    printf("   islower('A') = %d\n", islower('A'));
    printf("   islower('a') = %d\n\n", islower('a'));

    // 3. isdigit() - 检查字符是否为数字
    printf("3. isdigit() - 检查字符是否为数字:\n");
    printf("   isdigit('5') = %d\n", isdigit('5'));
    printf("   isdigit('A') = %d\n\n", isdigit('A'));

    // 4. isxdigit() - 检查字符是否为十六进制数字
    printf("4. isxdigit() - 检查字符是否为十六进制数字:\n");
    printf("   isxdigit('5') = %d\n", isxdigit('5'));
    printf("   isxdigit('A') = %d\n", isxdigit('A'));
    printf("   isxdigit('G') = %d\n\n", isxdigit('G'));

    // 5. isspace() - 检查字符是否为空白字符
    printf("5. isspace() - 检查字符是否为空白字符:\n");
    printf("   isspace(' ') = %d\n", isspace(' '));
    printf("   isspace('\\t') = %d\n", isspace('\t'));
    printf("   isspace('A') = %d\n\n", isspace('A'));

    // 6. ispunct() - 检查字符是否为标点符号
    printf("6. ispunct() - 检查字符是否为标点符号:\n");
    printf("   ispunct('.') = %d\n", ispunct('.'));
    printf("   ispunct('A') = %d\n\n", ispunct('A'));

    // 7. isalnum() - 检查字符是否为字母或数字
    printf("7. isalnum() - 检查字符是否为字母或数字:\n");
    printf("   isalnum('A') = %d\n", isalnum('A'));
    printf("   isalnum('5') = %d\n", isalnum('5'));
    printf("   isalnum('@') = %d\n\n", isalnum('@'));

    // 8. isprint() - 检查字符是否为可打印字符
    printf("8. isprint() - 检查字符是否为可打印字符:\n");
    printf("   isprint('A') = %d\n", isprint('A'));
    printf("   isprint('\\t') = %d\n\n", isprint('\t'));

    // 9. isgraph() - 检查字符是否为图形字符（可打印字符，不包括空格）
    printf("9. isgraph() - 检查字符是否为图形字符:\n");
    printf("   isgraph('A') = %d\n", isgraph('A'));
    printf("   isgraph(' ') = %d\n\n", isgraph(' '));

    // 10. iscntrl() - 检查字符是否为控制字符
    printf("10. iscntrl() - 检查字符是否为控制字符:\n");
    printf("    iscntrl('\\n') = %d\n", iscntrl('\n'));
    printf("    iscntrl('A') = %d\n\n", iscntrl('A'));

    // 11. toupper() 和 tolower() - 字符大小写转换
    printf("11. toupper() 和 tolower() - 字符大小写转换:\n");
    printf("    toupper('a') = %c\n", toupper('a'));
    printf("    tolower('A') = %c\n\n", tolower('A'));

    // 12. _isctype() - 使用特定类型检查字符
    printf("12. _isctype() - 使用特定类型检查字符:\n");
    printf("    _isctype('A', _ALPHA) = %d\n", _isctype('A', _ALPHA));
    printf("    _isctype('5', _DIGIT) = %d\n\n", _isctype('5', _DIGIT));

    // 13. __isascii() 和 __toascii() - ASCII字符检查和转换
    printf("13. __isascii() 和 __toascii() - ASCII字符检查和转换:\n");
    printf("    __isascii('A') = %d\n", __isascii('A'));
    printf("    __isascii(200) = %d\n", __isascii(200));
    printf("    __toascii(200) = %d\n\n", __toascii(200));

    // 14. __iscsymf() 和 __iscsym() - 检查C标识符起始字符和有效字符
    printf("14. __iscsymf() 和 __iscsym() - C标识符检查:\n");
    printf("    __iscsymf('a') = %d\n", __iscsymf('a'));
    printf("    __iscsymf('5') = %d\n", __iscsymf('5'));
    printf("    __iscsym('5') = %d\n", __iscsym('5'));
    printf("    __iscsym('@') = %d\n\n", __iscsym('@'));

    // 15. 使用带"_l"后缀的函数处理特定区域设置的字符
    printf("15. 使用带\"_l\"后缀的函数处理特定区域设置的字符:\n");
    printf("    _isalpha_l('A', NULL) = %d\n", _isalpha_l('A', NULL));

    // 16. 使用字符分类宏
    printf("16. 使用字符分类宏:\n");
    printf("    isalpha('B') = %d (使用宏)\n", isalpha('B'));

    return 0;
}
```

------



# 三、cerrno

## 1. 介绍

C语言操作错误的时候，可读取错误码，详细https://www.runoob.com/cprogramming/c-standard-library-errno-h.html

## 2. 使用示例

```c
#include <stdio.h>
#include <errno.h>
#include <string.h>

int main() {
    FILE *file = fopen("nonexistent_file.txt", "r");
    if (file == NULL) {
        printf("Error opening file: %s\n", strerror(errno));
        return 1;
    }

    // 文件处理代码
    fclose(file);
    return 0;
}
```

## 3. [MySQL C API](https://dev.mysql.com/downloads/mysql/9.3.html) 使用errno

```c
// header
#include <mysql.h>
#include <mysqld_error.h>

// define
unsigned int STDCALL mysql_errno(MYSQL *mysql);
unsigned int STDCALL mysql_stmt_errno(MYSQL_STMT *stmt);


// using
std::println(std::format("Failed to stmt bind result: {}, code: {}",
            mysql_stmt_error(stmt),
            mysql_stmt_errno(stmt)));
```

------



# 四、[cfenv](https://www.runoob.com/cprogramming/c-standard-library-fenv-h.html)

## 1. 介绍

`<fenv.h>` 是 C 标准库中的一个头文件，用于**控制浮点环境（Floating-Point Environment）**。

`<fenv.h>` 在 C99 标准中引入，提供了对浮点异常、舍入模式和其他浮点状态的控制和查询功能。

------

`<fenv.h>` 的主要目的是：

- 检测和处理浮点异常（如除以零、溢出等）。
- 控制浮点运算的舍入模式（如向零舍入、向最近舍入等）。
- 查询和修改浮点状态标志。

------

浮点异常是指在浮点运算中发生的特殊情况，例如：

- **FE_DIVBYZERO**：除以零。
- **FE_INEXACT**：不精确结果。
- **FE_INVALID**：无效操作（如对负数开平方）。
- **FE_OVERFLOW**：上溢。
- **FE_UNDERFLOW**：下溢。

这些异常通过浮点状态标志来表示，可以通过 `<fenv.h>` 中的函数检测和处理。

------

更多详细见 https://www.runoob.com/cprogramming/c-standard-library-fenv-h.html

------



# 五、cinttypes

## 1. 介绍

**<u>操作大整数的除法（商   余数），以及字符串转为大整数</u>**

### （1）源码

#### a. imaxdiv_t

```c
typedef struct
{
    intmax_t quot;
    intmax_t rem;
} _Lldiv_t;

typedef _Lldiv_t imaxdiv_t;
```

- **功能**: 用于存储 `imaxdiv` 函数返回的商和余数
- **结构**: 包含两个成员 `quot` (商) 和 `rem` (余数)
- **用途**: 处理大整数除法时同时获取商和余数

#### b. `imaxabs` 函数

- **原型**: `intmax_t imaxabs(intmax_t j)`
- **功能**: 计算最大宽度有符号整数的绝对值
- **参数**: `j` - 要计算绝对值的整数
- **返回值**: `|j|` 的绝对值

#### c. `imaxdiv` 函数

- **原型**: `imaxdiv_t imaxdiv(intmax_t numer, intmax_t denom)`
- **功能**: 计算最大宽度整数的除法，同时返回商和余数
- **参数**:
  - `numer` - 被除数
  - `denom` - 除数
- **返回值**: `imaxdiv_t` 结构，包含商和余数

#### d. `strtoimax` 函数

- **原型**: `intmax_t strtoimax(const char* nptr, char** endptr, int base)`
- **功能**: 将字符串转换为最大宽度有符号整数
- **参数**:
  - `nptr` - 要转换的字符串
  - `endptr` - 指向第一个无法转换字符的指针
  - `base` - 转换的基数 (0-36, 0表示自动检测)
- **返回值**: 转换后的整数值

#### e. `strtoumax` 函数

- **原型**: `uintmax_t strtoumax(const char* nptr, char** endptr, int base)`
- **功能**: 将字符串转换为最大宽度无符号整数
- **参数**: 同 `strtoimax`
- **返回值**: 转换后的无符号整数值

#### f. `wcstoimax` 函数

- **原型**: `intmax_t wcstoimax(const wchar_t* nptr, wchar_t** endptr, int base)`
- **功能**: 将宽字符串转换为最大宽度有符号整数
- **参数**: 同 `strtoimax`，但处理宽字符
- **返回值**: 转换后的整数值

#### g. `wcstoumax` 函数

- **原型**: `uintmax_t wcstoumax(const wchar_t* nptr, wchar_t** endptr, int base)`
- **功能**: 将宽字符串转换为最大宽度无符号整数
- **参数**: 同 `strtoumax`，但处理宽字符
- **返回值**: 转换后的无符号整数值

### （2）使用注意事项

1. **错误处理**: 这些转换函数在遇到无法转换的字符时会停止，并通过 `endptr` 参数指示停止位置
2. **基数参数**:
   - 0: 自动检测基数 (0x前缀为16进制，0前缀为8进制，否则为10进制)
   - 2-36: 指定转换的基数
3. **溢出处理**: 如果转换的值超出范围，函数会设置 `errno` 为 `ERANGE` 并返回最大值或最小值
4. **宽字符版本**: 处理宽字符字符串时使用 `wcstoimax` 和 `wcstoumax`

### （3）实际应用场景

- 处理大整数运算和转换
- 解析配置文件中的数值参数
- 处理用户输入的大数值
- 跨平台代码中确保使用最大宽度的整数类型
- 处理不同基数（如十六进制、八进制）的数值字符串

## 2. 使用示例

```c
#include <iostream>
#include <cinttypes>  // 包含 imaxabs, imaxdiv, strtoimax, strtoumax
#include <cstdint>    // 包含 intmax_t, uintmax_t
#include <cwchar>     // 包含 wcstoimax, wcstoumax
#include <string>

int main() {
    std::cout << "=== C++ 最大宽度整数类型和转换函数示例 ===\n\n";

    // 1. imaxdiv_t 类型 - 用于存储除法的商和余数
    std::cout << "1. imaxdiv_t 类型 - 用于存储除法的商和余数:\n";
    intmax_t dividend = 100;
    intmax_t divisor = 7;
    
    imaxdiv_t result = imaxdiv(dividend, divisor);
    std::cout << "   " << dividend << " ÷ " << divisor << " = " << result.quot 
              << " 余 " << result.rem << "\n\n";

    // 2. imaxabs - 计算最大宽度整数的绝对值
    std::cout << "2. imaxabs - 计算最大宽度整数的绝对值:\n";
    intmax_t negative = -123456789012345;
    intmax_t positive = imaxabs(negative);
    std::cout << "   imaxabs(" << negative << ") = " << positive << "\n\n";

    // 3. strtoimax - 将字符串转换为最大宽度有符号整数
    std::cout << "3. strtoimax - 将字符串转换为最大宽度有符号整数:\n";
    const char* str_num = "123456789012345";
    char* end_ptr;
    intmax_t imax_value = strtoimax(str_num, &end_ptr, 10);
    
    std::cout << "   字符串 \"" << str_num << "\" 转换为整数: " << imax_value;
    std::cout << "\n   解析结束位置: " << (end_ptr - str_num) << "\n\n";

    // 4. strtoumax - 将字符串转换为最大宽度无符号整数
    std::cout << "4. strtoumax - 将字符串转换为最大宽度无符号整数:\n";
    const char* str_hex = "0xabcdef123";
    uintmax_t umax_value = strtoumax(str_hex, &end_ptr, 0); // 0表示自动检测基数
    
    std::cout << "   字符串 \"" << str_hex << "\" 转换为无符号整数: " << umax_value;
    std::cout << "\n   解析结束位置: " << (end_ptr - str_hex) << "\n\n";

    // 5. wcstoimax - 将宽字符串转换为最大宽度有符号整数
    std::cout << "5. wcstoimax - 将宽字符串转换为最大宽度有符号整数:\n";
    const wchar_t* wstr_num = L"-98765432109876";
    wchar_t* wend_ptr;
    intmax_t wimax_value = wcstoimax(wstr_num, &wend_ptr, 10);
    
    std::wcout << L"   宽字符串 \"" << wstr_num << L"\" 转换为整数: " << wimax_value;
    std::wcout << L"\n   解析结束位置: " << (wend_ptr - wstr_num) << L"\n\n";

    // 6. wcstoumax - 将宽字符串转换为最大宽度无符号整数
    std::cout << "6. wcstoumax - 将宽字符串转换为最大宽度无符号整数:\n";
    const wchar_t* wstr_hex = L"0x123456789ABCDEF";
    uintmax_t wumax_value = wcstoumax(wstr_hex, &wend_ptr, 0);
    
    std::wcout << L"   宽字符串 \"" << wstr_hex << L"\" 转换为无符号整数: " << wumax_value;
    std::wcout << L"\n   解析结束位置: " << (wend_ptr - wstr_hex) << L"\n\n";

    // 7. 错误处理示例
    std::cout << "7. 错误处理示例:\n";
    const char* invalid_str = "123abc";
    intmax_t parsed_value = strtoimax(invalid_str, &end_ptr, 10);
    
    std::cout << "   字符串 \"" << invalid_str << "\" 部分转换为整数: " << parsed_value;
    std::cout << "\n   解析结束位置: " << (end_ptr - invalid_str);
    std::cout << "\n   剩余字符串: \"" << end_ptr << "\"\n\n";

    // 8. 边界值测试
    std::cout << "8. 边界值测试:\n";
    // macro LLONG_MAX
    const char* max_int_str = "9223372036854775807"; // 64位有符号整数的最大值
    intmax_t max_value = strtoimax(max_int_str, &end_ptr, 10);
    std::cout << "   最大有符号整数: " << max_value << "\n";

    // macro ULLONG_MAX
    const char* max_uint_str = "18446744073709551615"; // 64位无符号整数的最大值
    uintmax_t max_uvalue = strtoumax(max_uint_str, &end_ptr, 10);
    std::cout << "   最大无符号整数: " << max_uvalue << "\n\n";

    return 0;
}
```

------



# 六、climits

## 1、介绍

<u>**各个数值的范围宏定义**</u>

### （1）源码

```c
#define CHAR_BIT      8
#define SCHAR_MIN   (-128)
#define SCHAR_MAX     127
#define UCHAR_MAX     0xff

#ifndef _CHAR_UNSIGNED
    #define CHAR_MIN    SCHAR_MIN
    #define CHAR_MAX    SCHAR_MAX
#else
    #define CHAR_MIN    0
    #define CHAR_MAX    UCHAR_MAX
#endif

#define MB_LEN_MAX    5
#define SHRT_MIN    (-32768)
#define SHRT_MAX      32767
#define USHRT_MAX     0xffff
#define INT_MIN     (-2147483647 - 1)
#define INT_MAX       2147483647
#define UINT_MAX      0xffffffff
#define LONG_MIN    (-2147483647L - 1)
#define LONG_MAX      2147483647L
#define ULONG_MAX     0xffffffffUL
#define LLONG_MAX     9223372036854775807i64
#define LLONG_MIN   (-9223372036854775807i64 - 1)
#define ULLONG_MAX    0xffffffffffffffffui64

#define _I8_MIN     (-127i8 - 1)
#define _I8_MAX       127i8
#define _UI8_MAX      0xffui8

#define _I16_MIN    (-32767i16 - 1)
#define _I16_MAX      32767i16
#define _UI16_MAX     0xffffui16

#define _I32_MIN    (-2147483647i32 - 1)
#define _I32_MAX      2147483647i32
#define _UI32_MAX     0xffffffffui32

#define _I64_MIN    (-9223372036854775807i64 - 1)
#define _I64_MAX      9223372036854775807i64
#define _UI64_MAX     0xffffffffffffffffui64

#ifndef SIZE_MAX
    // SIZE_MAX definition must match exactly with stdint.h for modules support.
    #ifdef _WIN64
        #define SIZE_MAX 0xffffffffffffffffui64
    #else
        #define SIZE_MAX 0xffffffffui32
    #endif
#endif

#ifndef RSIZE_MAX
    #define RSIZE_MAX (SIZE_MAX >> 1)
#endif
```

------



# 七、[clocale](https://www.runoob.com/cprogramming/c-standard-library-locale-h.html)

## 1. 介绍

用于**支持程序的国际化和本地化**。它提供了一组函数和宏来设置或查询程序的本地化信息，例如**<u>日期、时间、货币、数字格式</u>**等。

详细见 https://www.runoob.com/cprogramming/c-standard-library-locale-h.html

------



# 八、cmath

## 1、介绍

**<u>数学运行操作</u>**

以下是 C++ `<cmath>` 头文件中定义的数学函数的详细说明，按功能分类列出：

### （1）基本数学运算

| 方法签名            | 功能作用                                      |
| ------------------- | --------------------------------------------- |
| `abs(x)`            | 计算整数的绝对值                              |
| `fabs(x)`           | 计算浮点数的绝对值                            |
| `fmod(x, y)`        | 计算 x 除以 y 的浮点余数                      |
| `remainder(x, y)`   | 计算 IEEE 浮点余数                            |
| `remquo(x, y, quo)` | 计算余数并将商的符号和至少低三位存储在 quo 中 |
| `fma(x, y, z)`      | 计算 (x * y) + z 的融合乘加运算               |
| `fmax(x, y)`        | 返回两个浮点参数中的较大值                    |
| `fmin(x, y)`        | 返回两个浮点参数中的较小值                    |
| `fdim(x, y)`        | 返回 x 和 y 之间的正差                        |
| `nan(str)`          | 返回安静的 NaN 值                             |

### （2）指数和对数函数

| 方法签名        | 功能作用                                          |
| --------------- | ------------------------------------------------- |
| `exp(x)`        | 计算 e 的 x 次幂                                  |
| `exp2(x)`       | 计算 2 的 x 次幂                                  |
| `expm1(x)`      | 计算 e 的 x 次幂减 1                              |
| `log(x)`        | 计算自然对数 (以 e 为底)                          |
| `log10(x)`      | 计算常用对数 (以 10 为底)                         |
| `log2(x)`       | 计算以 2 为底的对数                               |
| `log1p(x)`      | 计算 1+x 的自然对数                               |
| `logb(x)`       | 提取浮点数的指数                                  |
| `ilogb(x)`      | 提取浮点数的指数作为整数                          |
| `scalbn(x, n)`  | 将浮点数乘以 FLT_RADIX 的 n 次幂                  |
| `scalbln(x, n)` | 将浮点数乘以 FLT_RADIX 的 n 次幂 (n 为 long 类型) |

### （3）幂函数

| 方法签名         | 功能作用                            |
| ---------------- | ----------------------------------- |
| `pow(x, y)`      | 计算 x 的 y 次幂                    |
| `sqrt(x)`        | 计算平方根                          |
| `cbrt(x)`        | 计算立方根                          |
| `hypot(x, y)`    | 计算直角三角形的斜边长度 (√(x²+y²)) |
| `hypot(x, y, z)` | 计算三维空间中的距离 (√(x²+y²+z²))  |

### （4）三角函数

| 方法签名      | 功能作用                                 |
| ------------- | ---------------------------------------- |
| `sin(x)`      | 计算正弦值 (参数为弧度)                  |
| `cos(x)`      | 计算余弦值 (参数为弧度)                  |
| `tan(x)`      | 计算正切值 (参数为弧度)                  |
| `asin(x)`     | 计算反正弦值 (结果范围为 [-π/2, π/2])    |
| `acos(x)`     | 计算反余弦值 (结果范围为 [0, π])         |
| `atan(x)`     | 计算反正切值 (结果范围为 [-π/2, π/2])    |
| `atan2(y, x)` | 计算 y/x 的反正切值 (结果范围为 [-π, π]) |

### （5）双曲函数

| 方法签名   | 功能作用         |
| ---------- | ---------------- |
| `sinh(x)`  | 计算双曲正弦值   |
| `cosh(x)`  | 计算双曲余弦值   |
| `tanh(x)`  | 计算双曲正切值   |
| `asinh(x)` | 计算反双曲正弦值 |
| `acosh(x)` | 计算反双曲余弦值 |
| `atanh(x)` | 计算反双曲正切值 |

### （6）误差和伽玛函数

| 方法签名    | 功能作用                      |
| ----------- | ----------------------------- |
| `erf(x)`    | 计算误差函数                  |
| `erfc(x)`   | 计算互补误差函数 (1 - erf(x)) |
| `tgamma(x)` | 计算伽玛函数                  |
| `lgamma(x)` | 计算伽玛函数绝对值的自然对数  |

### （7）取整和余数函数

| 方法签名       | 功能作用                              |
| -------------- | ------------------------------------- |
| `ceil(x)`      | 向上取整 (返回不小于 x 的最小整数值)  |
| `floor(x)`     | 向下取整 (返回不大于 x 的最大整数值)  |
| `trunc(x)`     | 向零取整 (舍弃小数部分)               |
| `round(x)`     | 四舍五入到最接近的整数                |
| `lround(x)`    | 四舍五入到最接近的 long 整数          |
| `llround(x)`   | 四舍五入到最接近的 long long 整数     |
| `rint(x)`      | 使用当前舍入模式取整                  |
| `lrint(x)`     | 使用当前舍入模式取整为 long 整数      |
| `llrint(x)`    | 使用当前舍入模式取整为 long long 整数 |
| `nearbyint(x)` | 使用当前舍入模式取整 (不引发异常)     |

### （8）浮点操作函数

| 方法签名           | 功能作用                                          |
| ------------------ | ------------------------------------------------- |
| `frexp(x, exp)`    | 将浮点数分解为尾数和指数                          |
| `ldexp(x, exp)`    | 将尾数和指数组合成浮点数                          |
| `modf(x, intpart)` | 将浮点数分解为整数和小数部分                      |
| `nextafter(x, y)`  | 返回 x 朝向 y 的下一个可表示值                    |
| `nexttoward(x, y)` | 返回 x 朝向 y 的下一个可表示值 (y 为 long double) |
| `copysign(x, y)`   | 组合 x 的绝对值和 y 的符号                        |

### （9）分类和比较函数

| 方法签名               | 功能作用                                  |
| ---------------------- | ----------------------------------------- |
| `fpclassify(x)`        | 对浮点值进行分类                          |
| `isfinite(x)`          | 检查是否为有限值                          |
| `isinf(x)`             | 检查是否为无穷大                          |
| `isnan(x)`             | 检查是否为非数字 (NaN)                    |
| `isnormal(x)`          | 检查是否为正规数                          |
| `signbit(x)`           | 检查符号位是否为负                        |
| `isgreater(x, y)`      | 检查 x 是否大于 y (不引发 NaN 异常)       |
| `isgreaterequal(x, y)` | 检查 x 是否大于等于 y (不引发 NaN 异常)   |
| `isless(x, y)`         | 检查 x 是否小于 y (不引发 NaN 异常)       |
| `islessequal(x, y)`    | 检查 x 是否小于等于 y (不引发 NaN 异常)   |
| `islessgreater(x, y)`  | 检查 x 是否小于或大于 y (不引发 NaN 异常) |
| `isunordered(x, y)`    | 检查两个浮点数是否无序 (至少一个是 NaN)   |

### （10）C++17 特殊数学函数

| 方法签名                    | 功能作用                   |
| --------------------------- | -------------------------- |
| `assoc_laguerre(n, m, x)`   | 计算连带拉盖尔多项式       |
| `assoc_legendre(l, m, x)`   | 计算连带勒让德多项式       |
| `beta(x, y)`                | 计算贝塔函数               |
| `comp_ellint_1(k)`          | 计算第一类完全椭圆积分     |
| `comp_ellint_2(k)`          | 计算第二类完全椭圆积分     |
| `comp_ellint_3(k, nu)`      | 计算第三类完全椭圆积分     |
| `cyl_bessel_i(nu, x)`       | 计算第一类修正柱贝塞尔函数 |
| `cyl_bessel_j(nu, x)`       | 计算第一类柱贝塞尔函数     |
| `cyl_bessel_k(nu, x)`       | 计算第二类修正柱贝塞尔函数 |
| `cyl_neumann(nu, x)`        | 计算柱诺依曼函数           |
| `ellint_1(k, phi)`          | 计算第一类不完全椭圆积分   |
| `ellint_2(k, phi)`          | 计算第二类不完全椭圆积分   |
| `ellint_3(k, nu, phi)`      | 计算第三类不完全椭圆积分   |
| `expint(x)`                 | 计算指数积分               |
| `hermite(n, x)`             | 计算埃尔米特多项式         |
| `laguerre(n, x)`            | 计算拉盖尔多项式           |
| `legendre(l, x)`            | 计算勒让德多项式           |
| `riemann_zeta(x)`           | 计算黎曼ζ函数              |
| `sph_bessel(n, x)`          | 计算第一类球贝塞尔函数     |
| `sph_legendre(l, m, theta)` | 计算球连带勒让德函数       |
| `sph_neumann(n, x)`         | 计算球诺依曼函数           |

### （11）C++20 线性插值函数

| 方法签名        | 功能作用                                  |
| --------------- | ----------------------------------------- |
| `lerp(a, b, t)` | 计算 a 和 b 之间的线性插值 (a + t(b - a)) |

------



# 九、[csetjmp](https://www.runoob.com/cprogramming/c-standard-library-setjmp-h.html)

## 1.介绍

**setjmp.h** 头文件定义了宏 **setjmp()**、函数 **longjmp()** 和变量类型 **jmp_buf**，该变量类型会绕过正常的函数调用和返回规则。

提供了一组函数和宏，用于非本地跳转（即**<u>从一个函数跳转到另一个之前调用过的函数，而不需要正常的函数返回机制</u>**）。这种机制通常用于错误处理、异常处理或者跳出深层嵌套的循环或递归。

类似**回调、goto**

详细见 https://www.runoob.com/cprogramming/c-standard-library-setjmp-h.html

------



# 十、[csignal](https://www.runoob.com/cprogramming/c-standard-library-signal-h.html)

## 1. 介绍

* 定义了一个变量类型 **sig_atomic_t**、两个函数调用和一些宏来处理程序执行期间报告的不同信号。信号是一种异步通知机制，允许进程在特定事件发生时执行预定义的处理函数。



* **常用于服务器优雅的关闭**



详细见 https://www.runoob.com/cprogramming/c-standard-library-signal-h.html

## 2. 使用示例

```c
#include <signal.h>
#include <boost/asio.hpp>
#include <boost/beast.hpp>

int main(int argc, char* argv[]) {
    if (argc != 1) {
        return EXIT_FAILURE;
    }
    std::shared_ptr<MyServer> server;
    boost::asio::io_context io_context;
    try {
		// ctrl + c 程序会退出
        boost::asio::signal_set signals(io_context, SIGINT, SIGTERM);
        server = std::make_shared<MyServer>(io_context, Host, Port, ServerID);

        signals.async_wait([&](const boost::system::error_code& error, int signal_number) {
            if (error) {
                std::println("信号处理错误: {}", error.message());
                return;
            }
            std::println("接收到停止信号 ({}), 服务器正在关闭...", signal_number);

            server->stop();
            io_context.stop();
            });

        server->start();
        // 在单独线程中运行信号处理
        std::thread signal_thread([&io_context] {
            try {
                io_context.run();
            }
            catch (const std::exception& e) {
                std::cerr << "信号处理线程异常: " << e.what() << std::endl;
            }
            });

        // 确保信号线程正常结束
        if (signal_thread.joinable()) {
            signal_thread.join();
        }
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "聊天服务器启动失败: " << e.what() << std::endl;
        return 1;
    }
}

```

------



# 十一、[cstdarg](https://www.runoob.com/cprogramming/c-standard-library-stdarg-h.html)

## 1. 介绍

<u>**类似C++11的可变参数包 ...**</u>



`<stdarg.h>`提供了一组宏，用于访问可变数量的参数。

* **stdarg.h** 头文件定义了一个变量类型 **va_list** 和三个宏，这三个宏可用于在参数个数未知（即参数个数可变）时获取函数中的参数。

* 可变参数的函数通在参数列表的末尾是使用省略号 ... 定义的。

### （1）签名说明

下面是头文件 stdarg.h 中定义的变量类型：

| 序号 | 变量 & 描述                                                  |
| ---- | ------------------------------------------------------------ |
| 1    | **va_list**  这是一个适用于 **va_start()、va_arg()** 和 **va_end()** 这三个宏存储信息的类型。用于存储可变参数信息的类型。 |

下面是头文件 stdarg.h 中定义的宏：

| 序号 | 宏 & 描述                                                    |
| ---- | ------------------------------------------------------------ |
| 1    | [void va_start(va_list ap, last_arg)](https://www.runoob.com/cprogramming/c-macro-va_start.html) 这个宏初始化 **ap** 变量，它与 **va_arg** 和 **va_end** 宏是一起使用的。**last_arg** 是最后一个传递给函数的已知的固定参数，即省略号之前的参数。 |
| 2    | [type va_arg(va_list ap, type)](https://www.runoob.com/cprogramming/c-macro-va_arg.html) 这个宏检索函数参数列表中类型为 **type** 的下一个参数。 |
| 3    | [void va_end(va_list ap)](https://www.runoob.com/cprogramming/c-macro-va_end.html) 这个宏允许使用了 **va_start** 宏的带有可变参数的函数返回。如果在从函数返回之前没有调用 **va_end**，则结果为未定义。 |



## 2. 使用示例

```c
#include <stdio.h>
#include <stdarg.h>

// 计算可变参数的和
int sum(int count, ...) {
    int total = 0;
    va_list args;
    
    // 初始化 args 以访问可变参数
    va_start(args, count);
    
    // 逐个访问可变参数
    for (int i = 0; i < count; i++) {
        total += va_arg(args, int);
    }
    
    // 清理 args
    va_end(args);
    
    return total;
}

int main() {
    printf("Sum of 1, 2, 3: %d\n", sum(3, 1, 2, 3)); // 输出 6
    printf("Sum of 4, 5, 6, 7: %d\n", sum(4, 4, 5, 6, 7)); // 输出 22
    return 0;
}
```

## 3. C++11、C++17、C++20的参数包示例

```C
#include <iostream>
#include <string>
#include <type_traits>

// C++11 递归展开参数包的方法
namespace cpp11 {
    // 递归终止函数
    int sum() {
        return 0;
    }
    
    // 递归函数模板
    template<typename T, typename... Args>
    auto sum(T first, Args... args) -> typename std::common_type<T, Args...>::type {
        return first + sum(args...);
    }
    
    void demo() {
        std::cout << "C++11 - Recursive variadic template:\n";
        std::cout << "Sum of 1, 2, 3: " << sum(1, 2, 3) << std::endl;
        std::cout << "Sum of 4, 5, 6, 7: " << sum(4, 5, 6, 7) << std::endl;
        std::cout << "Sum of 1.5, 2.3, 3.7: " << sum(1.5, 2.3, 3.7) << std::endl;
        std::cout << std::endl;
    }
}

// C++17 折叠表达式方法
namespace cpp17 {
    template<typename... Args>
    auto sum(Args... args) {
        return (args + ...); // 一元右折叠
    }
    
    void demo() {
        std::cout << "C++17 - Fold expressions:\n";
        std::cout << "Sum of 1, 2, 3: " << sum(1, 2, 3) << std::endl;
        std::cout << "Sum of 4, 5, 6, 7: " << sum(4, 5, 6, 7) << std::endl;
        std::cout << "Sum of 1.5, 2.3, 3.7: " << sum(1.5, 2.3, 3.7) << std::endl;
        std::cout << std::endl;
    }
}

// C++20 带有约束的模板
namespace cpp20 {
    // 使用概念约束只允许数值类型
    template<typename T>
    concept Numeric = std::is_arithmetic_v<T>;
    
    template<Numeric... Args>
    auto sum(Args... args) {
        return (args + ...);
    }
    
    void demo() {
        std::cout << "C++20 - Fold expressions with concepts:\n";
        std::cout << "Sum of 1, 2, 3: " << sum(1, 2, 3) << std::endl;
        std::cout << "Sum of 4, 5, 6, 7: " << sum(4, 5, 6, 7) << std::endl;
        std::cout << "Sum of 1.5, 2.3, 3.7: " << sum(1.5, 2.3, 3.7) << std::endl;
        
        // 下面这行如果取消注释会编译错误，因为字符串不是数值类型
        // std::cout << "Sum of strings: " << sum(std::string("Hello"), std::string(" World")) << std::endl;
        
        std::cout << std::endl;
    }
}

// 更复杂的例子 - 打印所有参数
template<typename... Args>
void printAll(Args... args) {
    std::cout << "Printing all arguments: ";
    // 使用折叠表达式和逗号运算符
    ((std::cout << args << " "), ...);
    std::cout << std::endl;
}

// 计算参数的平方和
template<typename... Args>
auto sumOfSquares(Args... args) {
    return ((args * args) + ...);
}

int main() {
    std::cout << "C++ Parameter Pack Examples\n";
    std::cout << "===========================\n\n";
    
    cpp11::demo();
    cpp17::demo();
    cpp20::demo();
    
    // 额外示例
    std::cout << "Additional examples:\n";
    printAll(1, 2.5, "Hello", 'A');
    
    std::cout << "Sum of squares of 1, 2, 3, 4: " 
              << sumOfSquares(1, 2, 3, 4) << std::endl;
    
    return 0;
}
```

------



# 十二、cstddef

## 1. 介绍

<u>**类型定义**</u>

### （1）源码

```c
_EXPORT_STD using _CSTD ptrdiff_t;	// 迭代器差值
_EXPORT_STD using _CSTD size_t;		// 计算机的位数size类型，64位就是 u long long
_EXPORT_STD using max_align_t = double; // most aligned type
_EXPORT_STD using nullptr_t   = decltype(nullptr);

#ifdef __cpp_lib_byte
_EXPORT_STD enum class byte : unsigned char {}; // since C++17

// byte 由于是枚举类，它不会隐式转换为整数或其他类型，必须显式转换（例如通过 std::to_integer），避免了误用。
// byte 重载了位运算符（如 <<, >>, |, &, ^, ~），可以直接进行位操作。
```



## 2. 使用示例

```c
#include <cstddef>
#include <iostream>

int main() {
    // 定义两个 std::byte 变量
    std::byte b1{0x3F}; // 16进制值 0x3F
    std::byte b2{0xA5}; // 16进制值 0xA5

    // 位操作示例
    std::byte result = b1 | b2; // 按位或
    std::cout << std::to_integer<int>(result) << std::endl; // 输出: 191 (0xBF)

    // 左移操作
    std::byte shifted = b1 << 2;
    std::cout << std::to_integer<int>(shifted) << std::endl; // 输出: 252 (0xFC)

    // 直接赋值会报错（类型安全）
    // int num = b1; // 错误：不能隐式转换
    int num = std::to_integer<int>(b1); // 必须显式转换

    return 0;
}
```

## 3. 实际应用

- **处理二进制数据**：例如读取文件或网络数据时，用 `std::byte` 表示原始字节流。
- **底层内存操作**：如自定义内存分配器或序列化/反序列化时，避免类型歧义。

```c
#include <fstream>
#include <vector>
#include <cstddef>

// 读取文件的原始字节
std::vector<std::byte> readFile(const std::string& filename) {
    std::ifstream file(filename, std::ios::binary);
    return std::vector<std::byte>(
        std::istreambuf_iterator<char>(file),
        std::istreambuf_iterator<char>()
    );
}
```

------



# 十三、cstdint

## 1. 介绍

<u>**int的有/无符号类型标准化，以及最大/最小的数值范围宏定义**</u>

### （1）源码

```c
typedef signed char        int8_t;
typedef short              int16_t;
typedef int                int32_t;
typedef long long          int64_t;
typedef unsigned char      uint8_t;
typedef unsigned short     uint16_t;
typedef unsigned int       uint32_t;
typedef unsigned long long uint64_t;

typedef signed char        int_least8_t;
typedef short              int_least16_t;
typedef int                int_least32_t;
typedef long long          int_least64_t;
typedef unsigned char      uint_least8_t;
typedef unsigned short     uint_least16_t;
typedef unsigned int       uint_least32_t;
typedef unsigned long long uint_least64_t;

typedef signed char        int_fast8_t;
typedef int                int_fast16_t;
typedef int                int_fast32_t;
typedef long long          int_fast64_t;
typedef unsigned char      uint_fast8_t;
typedef unsigned int       uint_fast16_t;
typedef unsigned int       uint_fast32_t;
typedef unsigned long long uint_fast64_t;

typedef long long          intmax_t;
typedef unsigned long long uintmax_t;

// These macros must exactly match those in the Windows SDK's intsafe.h.
#define INT8_MIN         (-127i8 - 1)
#define INT16_MIN        (-32767i16 - 1)
#define INT32_MIN        (-2147483647i32 - 1)
#define INT64_MIN        (-9223372036854775807i64 - 1)
#define INT8_MAX         127i8
#define INT16_MAX        32767i16
#define INT32_MAX        2147483647i32
#define INT64_MAX        9223372036854775807i64
#define UINT8_MAX        0xffui8
#define UINT16_MAX       0xffffui16
#define UINT32_MAX       0xffffffffui32
#define UINT64_MAX       0xffffffffffffffffui64

#define INT_LEAST8_MIN   INT8_MIN
#define INT_LEAST16_MIN  INT16_MIN
#define INT_LEAST32_MIN  INT32_MIN
#define INT_LEAST64_MIN  INT64_MIN
#define INT_LEAST8_MAX   INT8_MAX
#define INT_LEAST16_MAX  INT16_MAX
#define INT_LEAST32_MAX  INT32_MAX
#define INT_LEAST64_MAX  INT64_MAX
#define UINT_LEAST8_MAX  UINT8_MAX
#define UINT_LEAST16_MAX UINT16_MAX
#define UINT_LEAST32_MAX UINT32_MAX
#define UINT_LEAST64_MAX UINT64_MAX

#define INT_FAST8_MIN    INT8_MIN
#define INT_FAST16_MIN   INT32_MIN
#define INT_FAST32_MIN   INT32_MIN
#define INT_FAST64_MIN   INT64_MIN
#define INT_FAST8_MAX    INT8_MAX
#define INT_FAST16_MAX   INT32_MAX
#define INT_FAST32_MAX   INT32_MAX
#define INT_FAST64_MAX   INT64_MAX
#define UINT_FAST8_MAX   UINT8_MAX
#define UINT_FAST16_MAX  UINT32_MAX
#define UINT_FAST32_MAX  UINT32_MAX
#define UINT_FAST64_MAX  UINT64_MAX

#ifdef _WIN64
    #define INTPTR_MIN   INT64_MIN
    #define INTPTR_MAX   INT64_MAX
    #define UINTPTR_MAX  UINT64_MAX
#else
    #define INTPTR_MIN   INT32_MIN
    #define INTPTR_MAX   INT32_MAX
    #define UINTPTR_MAX  UINT32_MAX
#endif

#define INTMAX_MIN       INT64_MIN
#define INTMAX_MAX       INT64_MAX
#define UINTMAX_MAX      UINT64_MAX

#define PTRDIFF_MIN      INTPTR_MIN
#define PTRDIFF_MAX      INTPTR_MAX

#ifndef SIZE_MAX
    // SIZE_MAX definition must match exactly with limits.h for modules support.
    #ifdef _WIN64
        #define SIZE_MAX 0xffffffffffffffffui64
    #else
        #define SIZE_MAX 0xffffffffui32
    #endif
#endif

#define SIG_ATOMIC_MIN   INT32_MIN
#define SIG_ATOMIC_MAX   INT32_MAX

#define WCHAR_MIN        0x0000
#define WCHAR_MAX        0xffff

#define WINT_MIN         0x0000
#define WINT_MAX         0xffff

#define INT8_C(x)    (x)
#define INT16_C(x)   (x)
#define INT32_C(x)   (x)
#define INT64_C(x)   (x ## LL)

#define UINT8_C(x)   (x)
#define UINT16_C(x)  (x)
#define UINT32_C(x)  (x ## U)
#define UINT64_C(x)  (x ## ULL)

#define INTMAX_C(x)  INT64_C(x)
#define UINTMAX_C(x) UINT64_C(x)
```

------



# 十四、[cstdio](https://www.runoob.com/cprogramming/c-standard-library-stdlib-h.html)

## 1. 介绍

**<u>提供文件/IO/控制台打印/格式化等方法操作</u>**

对于C++，见

```c
#include <iostream>
#include <print>

#include <istream>
#include <ostream>

#include <fstream>
#include <sstream>
#include <syncstream>
```

以下是 C++ `<cstdio>` 头文件中定义的标准输入输出函数的详细说明，包括 Windows 平台的安全注意事项和使用示例。

### （1）文件操作函数

| 方法签名                                                     | 功能作用       | Windows 安全注意事项                                         |
| ------------------------------------------------------------ | -------------- | ------------------------------------------------------------ |
| `FILE* fopen(const char* filename, const char* mode)`        | 打开文件       | Windows 推荐使用 `fopen_s`，需定义 `_CRT_SECURE_NO_WARNINGS` 禁用警告 |
| `int fclose(FILE* stream)`                                   | 关闭文件       | 相对安全                                                     |
| `FILE* freopen(const char* filename, const char* mode, FILE* stream)` | 重新打开文件流 | Windows 推荐使用 `freopen_s`                                 |
| `int remove(const char* filename)`                           | 删除文件       | 相对安全                                                     |
| `int rename(const char* oldname, const char* newname)`       | 重命名文件     | 相对安全                                                     |
| `FILE* tmpfile(void)`                                        | 创建临时文件   | Windows 推荐使用 `tmpfile_s`                                 |
| `char* tmpnam(char* str)`                                    | 生成临时文件名 | Windows 推荐使用 `tmpnam_s`，不安全，可能产生重复名称        |

### （2）文件访问函数

| 方法签名                                                     | 功能作用                             | Windows 安全注意事项       |
| ------------------------------------------------------------ | ------------------------------------ | -------------------------- |
| `int fgetc(FILE* stream)`                                    | 从文件流读取一个字符                 | 相对安全                   |
| `int getc(FILE* stream)`                                     | 从文件流读取一个字符（通常实现为宏） | 相对安全                   |
| `int getchar(void)`                                          | 从标准输入读取一个字符               | 相对安全                   |
| `int ungetc(int character, FILE* stream)`                    | 将字符推回输入流                     | 相对安全                   |
| `char* fgets(char* str, int num, FILE* stream)`              | 从文件流读取字符串                   | Windows 推荐使用 `fgets_s` |
| `int fputc(int character, FILE* stream)`                     | 向文件流写入一个字符                 | 相对安全                   |
| `int putc(int character, FILE* stream)`                      | 向文件流写入一个字符（通常实现为宏） | 相对安全                   |
| `int putchar(int character)`                                 | 向标准输出写入一个字符               | 相对安全                   |
| `int fputs(const char* str, FILE* stream)`                   | 向文件流写入字符串                   | 相对安全                   |
| `int puts(const char* str)`                                  | 向标准输出写入字符串                 | 相对安全                   |
| `size_t fread(void* ptr, size_t size, size_t count, FILE* stream)` | 从文件流读取数据块                   | 需要确保缓冲区足够大       |
| `size_t fwrite(const void* ptr, size_t size, size_t count, FILE* stream)` | 向文件流写入数据块                   | 相对安全                   |

### （3）格式化输入输出函数

| 方法签名                                                     | 功能作用                           | Windows 安全注意事项                              |
| ------------------------------------------------------------ | ---------------------------------- | ------------------------------------------------- |
| `int printf(const char* format, ...)`                        | 格式化输出到标准输出               | Windows 推荐使用 `printf_s`                       |
| `int fprintf(FILE* stream, const char* format, ...)`         | 格式化输出到文件流                 | Windows 推荐使用 `fprintf_s`                      |
| `int sprintf(char* str, const char* format, ...)`            | 格式化输出到字符串                 | **非常不安全**，Windows 强烈推荐使用 `sprintf_s`  |
| `int snprintf(char* str, size_t n, const char* format, ...)` | 安全格式化输出到字符串             | 相对安全，但 Windows 仍推荐使用 `_snprintf_s`     |
| `int scanf(const char* format, ...)`                         | 从标准输入格式化读取               | Windows 推荐使用 `scanf_s`                        |
| `int fscanf(FILE* stream, const char* format, ...)`          | 从文件流格式化读取                 | Windows 推荐使用 `fscanf_s`                       |
| `int sscanf(const char* str, const char* format, ...)`       | 从字符串格式化读取                 | Windows 推荐使用 `sscanf_s`                       |
| `int vprintf(const char* format, va_list arg)`               | 使用可变参数格式化输出到标准输出   | Windows 推荐使用 `vprintf_s`                      |
| `int vfprintf(FILE* stream, const char* format, va_list arg)` | 使用可变参数格式化输出到文件流     | Windows 推荐使用 `vfprintf_s`                     |
| `int vsprintf(char* str, const char* format, va_list arg)`   | 使用可变参数格式化输出到字符串     | **非常不安全**，Windows 强烈推荐使用 `vsprintf_s` |
| `int vsnprintf(char* str, size_t n, const char* format, va_list arg)` | 安全使用可变参数格式化输出到字符串 | 相对安全，但 Windows 仍推荐使用 `_vsnprintf_s`    |
| `int vfscanf(FILE* stream, const char* format, va_list arg)` | 使用可变参数从文件流格式化读取     | Windows 推荐使用 `vfscanf_s`                      |
| `int vscanf(const char* format, va_list arg)`                | 使用可变参数从标准输入格式化读取   | Windows 推荐使用 `vscanf_s`                       |
| `int vsscanf(const char* str, const char* format, va_list arg)` | 使用可变参数从字符串格式化读取     | Windows 推荐使用 `vsscanf_s`                      |

### （4）文件定位函数

| 方法签名                                               | 功能作用                       | Windows 安全注意事项 |
| ------------------------------------------------------ | ------------------------------ | -------------------- |
| `int fseek(FILE* stream, long int offset, int origin)` | 设置文件位置指示器             | 相对安全             |
| `long int ftell(FILE* stream)`                         | 获取当前文件位置               | 相对安全             |
| `void rewind(FILE* stream)`                            | 将文件位置指示器设置到文件开头 | 相对安全             |
| `int fgetpos(FILE* stream, fpos_t* position)`          | 获取当前文件位置               | 相对安全             |
| `int fsetpos(FILE* stream, const fpos_t* position)`    | 设置文件位置                   | 相对安全             |

### （5）错误处理函数

| 方法签名                       | 功能作用         | Windows 安全注意事项 |
| ------------------------------ | ---------------- | -------------------- |
| `void clearerr(FILE* stream)`  | 清除错误标志     | 相对安全             |
| `int feof(FILE* stream)`       | 检查文件结束标志 | 相对安全             |
| `int ferror(FILE* stream)`     | 检查错误标志     | 相对安全             |
| `void perror(const char* str)` | 打印错误消息     | 相对安全             |

### （6）缓冲控制函数

| 方法签名                                                     | 功能作用           | Windows 安全注意事项 |
| ------------------------------------------------------------ | ------------------ | -------------------- |
| `int fflush(FILE* stream)`                                   | 刷新输出缓冲区     | 相对安全             |
| `void setbuf(FILE* stream, char* buffer)`                    | 设置文件缓冲       | 相对安全             |
| `int setvbuf(FILE* stream, char* buffer, int mode, size_t size)` | 设置文件缓冲和模式 | 相对安全             |

### （7）类型定义

| 类型定义 | 功能作用                     |
| -------- | ---------------------------- |
| `FILE`   | 文件流类型                   |
| `fpos_t` | 文件位置类型                 |
| `size_t` | 无符号整数类型，用于表示大小 |

### Windows 安全函数使用说明

在 Windows 平台上，Microsoft 提供了一系列带有 `_s` 后缀的安全版本函数。要使用传统的非安全函数，需要在代码开头定义以下宏：

```cpp
#define _CRT_SECURE_NO_WARNINGS
```

或者可以在项目属性中设置：
- 项目属性 → C/C++ → 预处理器 → 预处理器定义 → 添加 `_CRT_SECURE_NO_WARNINGS`

## 2. 使用示例

```cpp
#include <cstdio>
#include <cstdarg>

// 定义宏以禁用安全警告（仅用于演示，生产代码应使用安全版本）
#define _CRT_SECURE_NO_WARNINGS

int main() {
    // 1. 文件操作示例
    FILE* file = fopen("example.txt", "w");
    if (file) {
        fputs("Hello, World!\n", file);
        fclose(file);
    }
    
    // 2. 格式化输出示例
    printf("Integer: %d, Float: %.2f, String: %s\n", 42, 3.14159, "example");
    
    char buffer[100];
    sprintf(buffer, "Formatted string: %d", 123);
    printf("%s\n", buffer);
    
    // 3. 格式化输入示例
    int num;
    float f;
    char str[50];
    
    printf("Enter an integer, a float and a string: ");
    scanf("%d %f %49s", &num, &f, str);
    printf("You entered: %d, %.2f, %s\n", num, f, str);
    
    // 4. 文件读取示例
    file = fopen("example.txt", "r");
    if (file) {
        char line[100];
        while (fgets(line, sizeof(line), file)) {
            printf("Read from file: %s", line);
        }
        fclose(file);
    }
    
    // 5. 错误处理示例
    file = fopen("nonexistent.txt", "r");
    if (!file) {
        perror("Error opening file");
    }
    
    // 6. 使用可变参数函数示例
    void printFormatted(const char* format, ...) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
    
    printFormatted("Variable arguments: %d, %s, %.2f\n", 10, "test", 3.14);
    
    // 7. 安全版本函数示例（Windows）
    #ifdef _WIN32
    FILE* safeFile;
    errno_t err = fopen_s(&safeFile, "safe_example.txt", "w");
    if (err == 0 && safeFile) {
        fputs("This is a safe write operation\n", safeFile);
        fclose(safeFile);
    }
    
    char safeBuffer[100];
    sprintf_s(safeBuffer, sizeof(safeBuffer), "Safe formatted: %d", 456);
    printf("%s\n", safeBuffer);
    #endif
    
    return 0;
}
```

## 3. 安全编程建议

### （1）编程建议

1. **避免使用不安全的字符串函数**：如 `sprintf`, `vsprintf`, `gets` 等
2. **使用带长度限制的版本**：如 `snprintf`, `fgets` 等
3. **在 Windows 上使用安全版本**：如 `fopen_s`, `printf_s`, `scanf_s` 等
4. **始终检查返回值**：确保函数执行成功
5. **验证输入数据**：特别是用户输入和外部数据
6. **使用缓冲区大小参数**：确保不会发生缓冲区溢出



### （2）跨平台建议及示例

如果需要跨平台使用，建议使用宏定义。

示例：snprintf方法， 格式化输出到字符串——[Bcrypt库|bcrypt.cpp](https://github.com/hilch/Bcrypt.cpp)

```c
#ifdef _WIN32
#include <windows.h>
#define snprintf(buffer, count, format, ...) _snprintf_s(buffer, count, _TRUNCATE, format, __VA_ARGS__)
#else
#include <unistd.h>
#include <fcntl.h>
#endif

void
encode_salt(char *salt, u_int8_t *csalt, char minor, u_int16_t clen, u_int8_t logr)
{
	salt[0] = '$';
	salt[1] = BCRYPT_VERSION;
	salt[2] = minor;
	salt[3] = '$';

    // Max rounds are 31
	snprintf(salt + 4, 4, "%2.2u$", logr & 0x001F);// Windows平台，编译时候宏会被替换为_snprintf_s，其他平台仍旧是snprintf，达到跨平台效果

	encode_base64((u_int8_t *) salt + 7, csalt, clen);
}
```

------



# 十五、[cstdlib](https://www.runoob.com/cprogramming/c-standard-library-stdlib-h.html)

## 1. 介绍

`<cstdlib>` 头文件提供了各种通用工具函数，包括**<u>内存管理、随机数生成、字符串转换、程序控制</u>**等功能。

### （1）内存管理函数（C++ <'memory>）

- `void* malloc(size_t size)` : **分配**指定字节数的**未初始化内存块**
- `void* calloc(size_t num, size_t size)` : 分配指定数量和大小的内存块，并将所有位**初始化为零**
- `void* realloc(void* ptr, size_t new_size)` : **重新分配内存块，改变其大小**（可能移动内存位置）
- `void free(void* ptr)` : **释放**之前分配的内存块

### （2）字符串转换函数

> 更多关于字符串转换的方法  见： 	**<u>第二章 cctype</u>**（ASCII码转换）
>
> C++ 可以使用 `std::to_string(x)` 转换   **可计算类型**  为  字符串 

- `int atoi(const char* str)` : 将**字符串转换为整数**
- `long atol(const char* str)` : 将**字符串转换为长整数**
- `long long atoll(const char* str)` : 将**字符串转换为长长整数**
- `double atof(const char* str)` : 将**字符串转换为双精度浮点数**
- `double strtod(const char* str, char** endptr)` : 将**字符串转换为双精度浮点数**，并提供结束指针
- `float strtof(const char* str, char** endptr)` : 将**字符串转换为单精度浮点数**，并提供结束指针
- `long double strtold(const char* str, char** endptr)` : 将**字符串转换为长双精度浮点数**，并提供结束指针
- `long strtol(const char* str, char** endptr, int base)` : 将**字符串转换为长整数**，指定基数
- `long long strtoll(const char* str, char** endptr, int base)` : 将**字符串转换为长长整数**，指定基数
- `unsigned long strtoul(const char* str, char** endptr, int base)` : 将**字符串转换为无符号长整数**，指定基数
- `unsigned long long strtoull(const char* str, char** endptr, int base)` : 将**字符串转换为无符号长长整数**，指定基数

### （3）数学函数

> 更多数学方法 见： 	**<u>第八章 cmath</u>**， **第五章 cinttypes**（大整数除法）

- `int abs(int n)` : 计算整数的绝对值
- `long labs(long n)` : 计算长整数的绝对值
- `long long llabs(long long n)` : 计算长长整数的绝对值
- `div_t div(int numer, int denom)` : 计算整数除法的商和余数
- `ldiv_t ldiv(long numer, long denom)` : 计算长整数除法的商和余数
- `lldiv_t lldiv(long long numer, long long denom)` : 计算长长整数除法的商和余数

### （4）随机数生成函数（更多随机算法见 C++<‘random>）

- `int rand(void)` : 生成伪随机数
- `void srand(unsigned int seed)` : 初始化伪随机数生成器

### （5）环境函数

- `char* getenv(const char* name)` : 获取环境变量的值
- `int system(const char* command)` : 执行系统命令

### （6）多字节/宽字符转换函数

- `int mblen(const char* pmb, size_t max)` : 确定多字节字符的字节数
- `int mbtowc(wchar_t* pwc, const char* pmb, size_t max)` : 将多字节字符转换为宽字符
- `int wctomb(char* pmb, wchar_t wc)` : 将宽字符转换为多字节字符
- `size_t mbstowcs(wchar_t* dest, const char* src, size_t max)` : 将多字节字符串转换为宽字符串
- `size_t wcstombs(char* dest, const wchar_t* src, size_t max)` : 将宽字符串转换为多字节字符串

### （7）搜索和排序函数（更多搜索和排序算法见 C++<'algorithm>）

- `void* bsearch(const void* key, const void* base, size_t num, size_t size, int (*compar)(const void*, const void*))` : 在数组中执行二分查找
- `void qsort(void* base, size_t num, size_t size, int (*compar)(const void*, const void*))` : 对数组进行快速排序

### （8）程序控制函数

> 程序优雅退出，见： 	**<u>第十章 csignal（通过异步读取键盘信号退出）</u>**

- `void abort(void)` : 异常终止程序
- `void exit(int status)` : 正常终止程序
- `void _Exit(int status)` : 立即终止程序，不执行清理操作
- `int atexit(void (*func)(void))` : 注册在程序正常终止时调用的函数
- `int at_quick_exit(void (*func)(void))` : 注册在程序快速终止时调用的函数
- `void quick_exit(int status)` : 快速终止程序，执行已注册的快速终止函数

## 2. 使用示例

```c
#include <cstdlib>
#include <iostream>
#include <ctime>
#include <cstring>

// 比较函数，用于qsort和bsearch
int compareInts(const void* a, const void* b) {
    return (*(int*)a - *(int*)b);
}

// atexit注册的函数
void cleanup() {
    std::cout << "清理函数被调用\n";
}

// at_quick_exit注册的函数
void quickCleanup() {
    std::cout << "快速清理函数被调用\n";
}

int main() {
    std::cout << "=== cstdlib 函数使用示例 ===\n\n";
    
    // 1. 内存管理示例
    std::cout << "1. 内存管理示例:\n";
    int* arr = (int*)malloc(5 * sizeof(int));
    if (arr) {
        for (int i = 0; i < 5; i++) {
            arr[i] = i + 1;
        }
        std::cout << "   malloc分配的内存: ";
        for (int i = 0; i < 5; i++) {
            std::cout << arr[i] << " ";
        }
        std::cout << "\n";
        
        // 重新分配内存
        arr = (int*)realloc(arr, 10 * sizeof(int));
        for (int i = 5; i < 10; i++) {
            arr[i] = i + 1;
        }
        std::cout << "   realloc后的内存: ";
        for (int i = 0; i < 10; i++) {
            std::cout << arr[i] << " ";
        }
        std::cout << "\n";
        
        free(arr);
    }
    
    // 使用calloc分配并初始化内存
    int* zeros = (int*)calloc(5, sizeof(int));
    std::cout << "   calloc初始化的内存: ";
    for (int i = 0; i < 5; i++) {
        std::cout << zeros[i] << " ";
    }
    std::cout << "\n";
    free(zeros);
    
    // 2. 字符串转换示例
    std::cout << "\n2. 字符串转换示例:\n";
    std::cout << "   atoi(\"123\") = " << atoi("123") << "\n";
    std::cout << "   atof(\"3.14\") = " << atof("3.14") << "\n";
    std::cout << "   atol(\"1000000\") = " << atol("1000000") << "\n";
    
    char* end;
    double d = strtod("123.45abc", &end);
    std::cout << "   strtod(\"123.45abc\") = " << d << ", 结束位置: " << (end - "123.45abc") << "\n";
    
    long l = strtol("1010", &end, 2); // 二进制转换
    std::cout << "   strtol(\"1010\", NULL, 2) = " << l << " (二进制1010 = 十进制10)\n";
    
    // 3. 数学函数示例
    std::cout << "\n3. 数学函数示例:\n";
    std::cout << "   abs(-10) = " << abs(-10) << "\n";
    std::cout << "   labs(-100000L) = " << labs(-100000L) << "\n";
    
    div_t result = div(10, 3);
    std::cout << "   div(10, 3) = 商: " << result.quot << ", 余数: " << result.rem << "\n";
    
    // 4. 随机数生成示例
    std::cout << "\n4. 随机数生成示例:\n";
    srand(static_cast<unsigned int>(time(nullptr))); // 使用当前时间作为种子
    std::cout << "   随机数: ";
    for (int i = 0; i < 5; i++) {
        std::cout << rand() % 100 << " "; // 生成0-99的随机数
    }
    std::cout << "\n";
    
    // 5. 环境函数示例
    std::cout << "\n5. 环境函数示例:\n";
    char* path = getenv("PATH");
    if (path) {
        std::cout << "   PATH环境变量: " << path << "\n";
    }
    
    // 6. 多字节/宽字符转换示例
    std::cout << "\n6. 多字节/宽字符转换示例:\n";
    const char* mbstr = "Hello";
    wchar_t wstr[10];
    size_t converted = mbstowcs(wstr, mbstr, 10);
    std::wcout << L"   多字节字符串 \"" << mbstr << L"\" 转换为宽字符: " << wstr << L" (长度: " << converted << L")\n";
    
    // 7. 搜索和排序示例
    std::cout << "\n7. 搜索和排序示例:\n";
    int numbers[] = {5, 2, 8, 1, 9};
    int size = sizeof(numbers) / sizeof(numbers[0]);
    
    qsort(numbers, size, sizeof(int), compareInts);
    std::cout << "   排序后的数组: ";
    for (int i = 0; i < size; i++) {
        std::cout << numbers[i] << " ";
    }
    std::cout << "\n";
    
    int key = 8;
    int* found = (int*)bsearch(&key, numbers, size, sizeof(int), compareInts);
    if (found) {
        std::cout << "   找到元素 " << key << " 在数组中的位置\n";
    }
    
    // 8. 程序控制示例
    std::cout << "\n8. 程序控制示例:\n";
    // 注册终止时调用的函数
    atexit(cleanup);
    at_quick_exit(quickCleanup);
    
    std::cout << "   程序即将退出...\n";
    
    // 正常退出程序
    // exit(0);
    
    // 或者使用快速退出
    // quick_exit(0);
    
    return 0;
}
```

------

## 3. 特别说明Windows平台获取环境变量示例

* 使用   模板   和C++23的   std::expected<>   进行编写示例

```c
#include <iostream>
#include <expected>
#include <string>
#include <filesystem>

template<size_t N>
struct fixed_wstring {
    wchar_t value[N];

    constexpr fixed_wstring(const wchar_t(&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }

    constexpr operator const wchar_t* () const { return value; }
    constexpr const wchar_t* data() const { return value; }
    constexpr size_t size() const { return N - 1; } // 减去空字符
};
template<size_t N>
struct fixed_string {
    char value[N];

    constexpr fixed_string(const char(&str)[N]) {
        for (size_t i = 0; i < N; ++i) {
            value[i] = str[i];
        }
    }

    constexpr operator const char* () const { return value; }
    constexpr const char* data() const { return value; }
    constexpr size_t size() const { return N - 1; } // 减去空字符
};

enum class PathErrorCode {
    BufferTooSmall,             // 缓冲区不足
    ModuleHandleFailed,         // 模块句柄获取失败
    UnknownSystemError,         // 未知系统错误
    PathResolutionFailed,       // 路径解析失败
    NoParentDirectory,          // 路径无父目录
    EnvironmentVariableNotFound,// 环境变量不存在
    EnvironmentVariableFailed,  // 环境变量获取失败
    CharacterConversionFailed   // 字符转换失败
};

// 结构化错误信息
struct PathError {
    PathErrorCode code;
    std::error_code system_error;
    std::string message;

    friend std::ostream& operator<<(std::ostream& os, const PathError& error) {
        os << "PathError: ";
        switch (error.code) {
        case PathErrorCode::BufferTooSmall:          os << "Buffer too small"; break;
        case PathErrorCode::ModuleHandleFailed:      os << "Failed to get module handle"; break;
        case PathErrorCode::PathResolutionFailed:    os << "Path resolution failed"; break;
        case PathErrorCode::UnknownSystemError:       os << "Unknown system error"; break;
        case PathErrorCode::NoParentDirectory:        os << "No parent directory"; break;
        case PathErrorCode::EnvironmentVariableNotFound: os << "Environment variable not found"; break;
        case PathErrorCode::EnvironmentVariableFailed: os << "Environment variable retrieval failed"; break;
        case PathErrorCode::CharacterConversionFailed: os << "Character conversion failed"; break;
        }
        if (error.system_error) {
            os << " (system: " << error.system_error.message() << ")";
        }
        if (!error.message.empty()) {
            os << " - " << error.message;
        }
        return os;
    }
};

template<fixed_string Name>
inline std::expected<std::filesystem::path, PathError> get_env_a() {
#if defined(_WIN32)
    // Windows实现
    DWORD valueLen = GetEnvironmentVariableA(Name.value, nullptr, 0);
    if (valueLen == 0) {
        DWORD err = GetLastError();
        if (err == ERROR_ENVVAR_NOT_FOUND) {
            return std::unexpected(PathError{
                PathErrorCode::EnvironmentVariableNotFound,
                std::error_code(static_cast<int>(err), std::system_category()),
                "Environment variable not found: " + Name.value
                });
        }
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableFailed,
            std::error_code(static_cast<int>(err), std::system_category()),
            "Failed to retrieve environment variable: " + Name.value
            });
    }

    std::string value(valueLen, '\0');
    valueLen = GetEnvironmentVariableA(Name.value, value.data(), valueLen);
    if (valueLen == 0 || valueLen >= value.size()) {
        DWORD err = GetLastError();
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableFailed,
            std::error_code(static_cast<int>(err), std::system_category()),
            "Failed to retrieve environment variable: " + Name.value
            });
    }
    value.resize(valueLen); // 移除多余的null字符
    return std::filesystem::path(value);
#else
    // Unix-like系统实现
    const char* value = std::getenv(Name.value);
    if (value == nullptr) {
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableNotFound,
            {},
            "Environment variable not found: "+Name.value
            });
    }
    return std::filesystem::path(value);
#endif
}

template<fixed_wstring Name>
inline std::expected<std::filesystem::path, PathError> get_env_w() {
#if defined(_WIN32)
    // Windows实现
    DWORD valueLen = GetEnvironmentVariableW(Name.value, nullptr, 0);
    if (valueLen == 0) {
        DWORD err = GetLastError();
        if (err == ERROR_ENVVAR_NOT_FOUND) {
            return std::unexpected(PathError{
                PathErrorCode::EnvironmentVariableNotFound,
                std::error_code(static_cast<int>(err), std::system_category()),
                "Environment variable not found"
                });
        }
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableFailed,
            std::error_code(static_cast<int>(err), std::system_category()),
            "Failed to retrieve environment variable"
            });
    }

    std::wstring wideValue(valueLen, L'\0');
    valueLen = GetEnvironmentVariableW(Name.value, wideValue.data(), valueLen);
    if (valueLen == 0 || valueLen >= wideValue.size()) {
        DWORD err = GetLastError();
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableFailed,
            std::error_code(static_cast<int>(err), std::system_category()),
            "Failed to retrieve environment variable"
            });
    }
    wideValue.resize(valueLen); // 移除多余的null字符
    return std::filesystem::path(wideValue);
#else
    // Unix-like系统实现：将宽字符名称转换为窄字符
    std::string narrowName;
    size_t len = wcslen(Name.value);
    narrowName.resize(len * 4 + 1); // 预留UTF-8最大字节空间

    size_t converted = wcstombs(narrowName.data(), Name.value, narrowName.size());
    if (converted == static_cast<size_t>(-1)) {
        return std::unexpected(PathError{
            PathErrorCode::CharacterConversionFailed,
            std::error_code(errno, std::generic_category()),
            "Failed to convert environment variable name"
            });
    }
    narrowName.resize(converted);

    const char* value = std::getenv(narrowName.c_str());
    if (value == nullptr) {
        return std::unexpected(PathError{
            PathErrorCode::EnvironmentVariableNotFound,
            {},
            "Environment variable not found"
            });
    }
    return std::filesystem::path(value);
#endif
}
```

------



# 十六、[cstring](https://www.runoob.com/cprogramming/c-standard-library-string-h.html)（C++字符串见 <'string>）

> ASCII字符串的转换、数值计算   见：	**<u>第十五章 cstdlib 介绍（2）</u>** 	**<u>第二章 cctype</u>**（ASCII码转换）
>
> strerror   见：	**<u>第三章 cerrno</u>**

## 1. 介绍

**<u>字符串拷贝、移动、拼接、比较、搜索、长度计算等方法定义</u>**

### （1）字符串函数表格

| 函数签名                                                     | 功能作用                                                   | Windows 安全注意事项                                  |
| ------------------------------------------------------------ | ---------------------------------------------------------- | ----------------------------------------------------- |
| `void* memcpy(void* dest, const void* src, size_t count)`    | 从源内存区域复制指定数量的字节到目标内存区域               | Windows 推荐使用 `memcpy_s`，需要确保目标缓冲区足够大 |
| `void* memmove(void* dest, const void* src, size_t count)`   | 从源内存区域复制指定数量的字节到目标内存区域，处理内存重叠 | 相对安全，但 Windows 仍推荐使用 `memmove_s`           |
| `void* memset(void* dest, int ch, size_t count)`             | 将目标内存区域的前 count 个字节设置为指定值                | 相对安全                                              |
| `int memcmp(const void* buf1, const void* buf2, size_t count)` | 比较两个内存区域的前 count 个字节                          | 相对安全                                              |
| `void* memchr(const void* ptr, int ch, size_t count)`        | 在内存区域的前 count 个字节中搜索指定字符                  | 相对安全                                              |
| `char* strcpy(char* dest, const char* src)`                  | 复制源字符串到目标字符串                                   | **非常不安全**，Windows 强烈推荐使用 `strcpy_s`       |
| `char* strncpy(char* dest, const char* src, size_t count)`   | 从源字符串复制最多 count 个字符到目标字符串                | Windows 推荐使用 `strncpy_s`                          |
| `char* strcat(char* dest, const char* src)`                  | 将源字符串追加到目标字符串末尾                             | **非常不安全**，Windows 强烈推荐使用 `strcat_s`       |
| `char* strncat(char* dest, const char* src, size_t count)`   | 从源字符串追加最多 count 个字符到目标字符串末尾            | Windows 推荐使用 `strncat_s`                          |
| `int strcmp(const char* lhs, const char* rhs)`               | 比较两个字符串                                             | 相对安全                                              |
| `int strncmp(const char* lhs, const char* rhs, size_t count)` | 比较两个字符串的前 count 个字符                            | 相对安全                                              |
| `char* strchr(const char* str, int ch)`                      | 在字符串中搜索指定字符的第一次出现                         | 相对安全                                              |
| `char* strrchr(const char* str, int ch)`                     | 在字符串中搜索指定字符的最后一次出现                       | 相对安全                                              |
| `size_t strcspn(const char* dest, const char* src)`          | 返回目标字符串中不包含源字符串中任何字符的初始段长度       | 相对安全                                              |
| `size_t strspn(const char* dest, const char* src)`           | 返回目标字符串中包含源字符串中字符的初始段长度             | 相对安全                                              |
| `char* strpbrk(const char* dest, const char* breakset)`      | 在目标字符串中搜索源字符串中任何字符的第一次出现           | 相对安全                                              |
| `char* strstr(const char* str, const char* substr)`          | 在字符串中搜索子字符串的第一次出现                         | 相对安全                                              |
| `char* strtok(char* str, const char* delim)`                 | 使用分隔符分割字符串                                       | Windows 推荐使用 `strtok_s`，原函数不是线程安全的     |
| `size_t strlen(const char* str)`                             | 计算字符串长度（不包括空终止符）                           | 相对安全                                              |
| `char* strerror(int errnum)`                                 | 获取错误码对应的错误消息字符串                             | 相对安全                                              |
| `int strcoll(const char* lhs, const char* rhs)`              | 根据当前区域设置比较两个字符串                             | 相对安全                                              |
| `size_t strxfrm(char* dest, const char* src, size_t count)`  | 根据当前区域设置转换字符串，使其能够通过 strcmp 进行比较   | Windows 推荐使用 `strxfrm_s`                          |

### Windows 安全函数使用说明

在 Windows 平台上，Microsoft 提供了一系列带有 `_s` 后缀的安全版本函数。要使用传统的非安全函数，需要在代码开头定义以下宏：

```cpp
#define _CRT_SECURE_NO_WARNINGS
```

或者可以在项目属性中设置：
- 项目属性 → C/C++ → 预处理器 → 预处理器定义 → 添加 `_CRT_SECURE_NO_WARNINGS`

## 2. 使用示例

```cpp
#include <cstring>
#include <iostream>
#include <cstdio>
#include <cerrno>

// 定义宏以禁用安全警告（仅用于演示，生产代码应使用安全版本）
#define _CRT_SECURE_NO_WARNINGS

int main() {
    std::cout << "=== cstring 函数使用示例 ===\n\n";
    
    // 1. 内存操作函数示例
    std::cout << "1. 内存操作函数示例:\n";
    char source[] = "Hello, World!";
    char dest[20];
    
    // memcpy 示例
    memcpy(dest, source, strlen(source) + 1); // +1 是为了复制空终止符
    std::cout << "   memcpy结果: " << dest << "\n";
    
    // memmove 示例（处理内存重叠）
    char buffer[] = "abcdefghijk";
    memmove(buffer + 2, buffer, 5);
    std::cout << "   memmove结果: " << buffer << "\n";
    
    // memset 示例
    char arr[10];
    memset(arr, 'A', 9);
    arr[9] = '\0';
    std::cout << "   memset结果: " << arr << "\n";
    
    // memcmp 示例
    char str1[] = "abc";
    char str2[] = "abd";
    int cmp_result = memcmp(str1, str2, 3);
    std::cout << "   memcmp比较 \"abc\" 和 \"abd\": " << cmp_result << "\n";
    
    // memchr 示例
    char* found = (char*)memchr(source, 'W', strlen(source));
    if (found) {
        std::cout << "   memchr找到 'W' 在位置: " << (found - source) << "\n";
    }
    
    // 2. 字符串复制函数示例
    std::cout << "\n2. 字符串复制函数示例:\n";
    char dest1[20];
    
    // strcpy 示例
    strcpy(dest1, "Hello");
    std::cout << "   strcpy结果: " << dest1 << "\n";
    
    // strncpy 示例
    char dest2[10];
    strncpy(dest2, "Hello, World!", 9);
    dest2[9] = '\0'; // 确保字符串正确终止
    std::cout << "   strncpy结果: " << dest2 << "\n";
    
    // 3. 字符串连接函数示例
    std::cout << "\n3. 字符串连接函数示例:\n";
    char dest3[20] = "Hello";
    
    // strcat 示例
    strcat(dest3, ", World!");
    std::cout << "   strcat结果: " << dest3 << "\n";
    
    // strncat 示例
    char dest4[20] = "Hello";
    strncat(dest4, ", World!", 5);
    std::cout << "   strncat结果: " << dest4 << "\n";
    
    // 4. 字符串比较函数示例
    std::cout << "\n4. 字符串比较函数示例:\n";
    char str3[] = "apple";
    char str4[] = "banana";
    
    // strcmp 示例
    int result = strcmp(str3, str4);
    std::cout << "   strcmp比较 \"apple\" 和 \"banana\": " << result << "\n";
    
    // strncmp 示例
    result = strncmp("apple", "application", 3);
    std::cout << "   strncmp比较 \"apple\" 和 \"application\" 的前3个字符: " << result << "\n";
    
    // 5. 字符串搜索函数示例
    std::cout << "\n5. 字符串搜索函数示例:\n";
    char text[] = "The quick brown fox jumps over the lazy dog";
    
    // strchr 示例
    char* first_o = strchr(text, 'o');
    if (first_o) {
        std::cout << "   strchr找到第一个 'o' 在位置: " << (first_o - text) << "\n";
    }
    
    // strrchr 示例
    char* last_o = strrchr(text, 'o');
    if (last_o) {
        std::cout << "   strrchr找到最后一个 'o' 在位置: " << (last_o - text) << "\n";
    }
    
    // strstr 示例
    char* fox = strstr(text, "fox");
    if (fox) {
        std::cout << "   strstr找到 \"fox\" 在位置: " << (fox - text) << "\n";
    }
    
    // strpbrk 示例
    char* vowel = strpbrk(text, "aeiou");
    if (vowel) {
        std::cout << "   strpbrk找到第一个元音 '" << *vowel << "' 在位置: " << (vowel - text) << "\n";
    }
    
    // 6. 字符串分割函数示例
    std::cout << "\n6. 字符串分割函数示例:\n";
    char csv[] = "apple,banana,cherry,date";
    char* token = strtok(csv, ",");
    
    std::cout << "   strtok分割结果: ";
    while (token) {
        std::cout << token << " ";
        token = strtok(nullptr, ",");
    }
    std::cout << "\n";
    
    // 7. 其他字符串函数示例
    std::cout << "\n7. 其他字符串函数示例:\n";
    
    // strlen 示例
    char long_str[] = "This is a long string";
    std::cout << "   strlen(\"" << long_str << "\") = " << strlen(long_str) << "\n";
    
    // strerror 示例
    errno = ENOENT; // 设置错误号
    std::cout << "   strerror(ENOENT): " << strerror(errno) << "\n";
    
    // strcspn 示例
    char filename[] = "file.txt";
    size_t len = strcspn(filename, ".");
    std::cout << "   strcspn(\"file.txt\", \".\") = " << len << " (文件名部分长度)\n";
    
    // strspn 示例
    char digits[] = "123abc456";
    len = strspn(digits, "0123456789");
    std::cout << "   strspn(\"123abc456\", \"0123456789\") = " << len << " (数字前缀长度)\n";
    
    // 8. Windows 安全版本函数示例
    std::cout << "\n8. Windows 安全版本函数示例:\n";
    #ifdef _WIN32
    char safe_dest[20];
    errno_t err = strcpy_s(safe_dest, sizeof(safe_dest), "Safe copy");
    if (err == 0) {
        std::cout << "   strcpy_s结果: " << safe_dest << "\n";
    }
    
    char safe_cat[20] = "Hello";
    err = strcat_s(safe_cat, sizeof(safe_cat), ", Safe World!");
    if (err == 0) {
        std::cout << "   strcat_s结果: " << safe_cat << "\n";
    }
    
    char tokens[] = "one;two;three";
    char* context = nullptr;
    char* safe_token = strtok_s(tokens, ";", &context);
    
    std::cout << "   strtok_s分割结果: ";
    while (safe_token) {
        std::cout << safe_token << " ";
        safe_token = strtok_s(nullptr, ";", &context);
    }
    std::cout << "\n";
    #endif
    
    return 0;
}
```

## 3. 安全编程建议

1. **避免使用不安全的字符串函数**：如 `strcpy`, `strcat`, `sprintf` 等
2. **使用带长度限制的版本**：如 `strncpy`, `strncat`, `snprintf` 等
3. **在 Windows 上使用安全版本**：如 `strcpy_s`, `strcat_s`, `strtok_s` 等
4. **始终检查缓冲区大小**：确保目标缓冲区足够大，避免缓冲区溢出
5. **正确终止字符串**：确保所有字符串操作后都有空终止符 `\0`
6. **验证输入数据**：特别是用户输入和外部数据
7. **使用更安全的 C++ 字符串类**：如 `std::string` 和 `std::wstring`

## 4. 注意事项

1. **strtok 函数的线程安全性**：`strtok` 函数使用静态缓冲区，不是线程安全的。在多线程环境中应使用 `strtok_s` 或其他替代方案。

2. **缓冲区溢出风险**：许多 C 字符串函数不检查目标缓冲区的大小，容易导致缓冲区溢出漏洞。

3. **字符串终止符**：所有 C 字符串函数都假设字符串以空字符 `\0` 终止，如果字符串没有正确终止，可能会导致未定义行为。

4. **区域设置影响**：`strcoll` 和 `strxfrm` 函数的行为受当前区域设置影响。

5. **错误处理**：某些函数（如 `strerror`）依赖于全局变量 `errno`，在多线程环境中需要注意同步问题。

通过遵循这些安全编程实践，可以大大减少与字符串操作相关的安全漏洞。在现代 C++ 开发中，推荐使用 `std::string` 和 `std::wstring` 类，它们提供了更安全、更方便的字符串操作功能。

## 5. 特别说明关于C++的拼接方法编写示例

基于C++17 的 **参数包折叠**	和   模板编程

* join，使用分隔符拼接字符串
* concat，拼接字符串
* join_range，拼接容器字符串，预留空间，适用于大字符串拼接
* plain_join_range，拼接容器字符串，适用于字符串不长 （长度5以内），字符串个数不多（个数5个以内）

```c
#include <string>
#include <string_view>
#include <type_traits>
#include <numeric>
#include <concepts>

namespace string {
    template<size_t N>
    struct fixed_wstring {
        wchar_t value[N];

        constexpr fixed_wstring(const wchar_t(&str)[N]) {
            for (size_t i = 0; i < N; ++i) {
                value[i] = str[i];
            }
        }

        constexpr operator const wchar_t* () const { return value; }
        constexpr const wchar_t* data() const { return value; }
        constexpr size_t size() const { return N - 1; } // 减去空字符
    };
    template<size_t N>
    struct fixed_string {
        char value[N];

        constexpr fixed_string(const char(&str)[N]) {
            for (size_t i = 0; i < N; ++i) {
                value[i] = str[i];
            }
        }

        constexpr operator const char* () const { return value; }
        constexpr const char* data() const { return value; }
        constexpr size_t size() const { return N - 1; } // 减去空字符
    };

    // 字符串长度计算辅助函数
    inline size_t string_length(const char* str) { return str ? std::char_traits<char>::length(str) : 0/*std::strlen(str)*/; }
    inline size_t string_length(const std::string& str) { return str.length(); }
    inline size_t string_length(std::string_view str) { return str.length(); }
    template<std::floating_point T>
    size_t string_length(T value) {
        // 浮点数最大长度：符号 + 最大位数 + 小数点 + 'e' + 指数符号 + 指数位数
        return 8 + std::numeric_limits<T>::max_digits10;
    }
    template<std::integral T>
    size_t string_length(T value) {
        // 快速估算最大位数：log10(2^bits) + 符号
        if constexpr (std::is_signed_v<T>) {
            return std::to_string(value).size(); // 实际计算
        }
        else {
            return std::to_string(value).size();
        }
    }

    template<size_t N>
    void append_to_string(std::string& result, const fixed_string<N>& fs) {
        result.append(fs.data(), fs.size());
    }

    // 字符串追加辅助函数
    inline void append_to_string(std::string& result, const char* str) { result.append(str); }
    inline void append_to_string(std::string& result, std::string str) { result.append(std::move(str)); }
    inline void append_to_string(std::string& result, std::string_view str) { result.append(str); }

    template<std::integral T>
    void append_to_string(std::string& result, T value) {
        result.append(std::to_string(value));
    }

    template<std::floating_point T>
    void append_to_string(std::string& result, T value) {
        result.append(std::to_string(value));
    }

    template<typename T>
    concept Appendable = requires(std::string & s, T && t) {
        { append_to_string(s, std::forward<T>(t)) } -> std::same_as<void>;
    };

    template<Appendable T>
    void append_to_string(std::string& result, T&& value) {
        append_to_string(result, std::forward<T>(value));
    }

    template<typename Sep, typename Range>
    inline std::string join_range(Sep&& separator, const Range& range)
    {
        if (std::empty(range)) return std::string();

        auto it = std::begin(range);
        auto end = std::end(range);

        // 计算总长度
        size_t totalSize = std::accumulate(
            it, end, size_t(0),
            [](size_t acc, const auto& item) {
                return acc + string_length(item);
            }
        ) + /*std::distance(it, end)*/(std::size(range) - 1) * string_length(separator);
        /*size_t count = 0;
        for (const auto& item : range) {
            totalSize += string_length(item);
            count++;
        }
        totalSize += (count - 1) * string_length(separator);*/

        std::string result;
        result.reserve(totalSize);

        // 添加第一个元素
        append_to_string(result, *it);
        ++it;

        // 添加后续元素
        for (; it != end; ++it) {
            append_to_string(result, separator);
            append_to_string(result, *it);
        }

        return result;
    }

    template<typename Sep, typename Range>
    inline std::string plain_join_range(Sep&& separator, const Range& range)
    {
        if (std::empty(range)) return std::string();

        auto it = std::begin(range);
        auto end = std::end(range);

        std::string result;
        append_to_string(result, *it);
        ++it;

        for (; it != end; ++it) {
            append_to_string(result, separator);
            append_to_string(result, *it);
        }

        return result;
    }

    /**
        * @brief 使用分隔符连接多个字符串
        * @tparam Args 可变参数类型
        * @param separator 分隔符
        * @param args 要连接的字符串参数
        * @return 连接后的字符串
        *
        * 使用示例:
        * std::string result = join(", ", "apple", "banana", "orange", 42);
        * // 结果: "apple, banana, orange, 42"
        */
    template<typename Sep, typename... Args>
    inline std::string join(Sep&& separator, Args&&... args) {
        if constexpr (sizeof...(args) == 0) return std::string();

        // 计算分隔符数量和总长度
        const size_t separatorCount = sizeof...(args) - 1;
        const size_t separatorLength = string_length(separator);
        const size_t totalSize = (0 + ... + string_length(args)) +
            separatorCount * separatorLength;

        std::string result;
        result.reserve(totalSize);

        // 使用折叠表达式实现连接
        size_t index = 0;
        ((append_to_string(result, (index++ > 0 ? separator : "")),
            append_to_string(result, std::forward<Args>(args))), ...);

        return result;
    }

    /**
        * @brief 高效拼接多个字符串
        * @tparam Args 可变参数类型
        * @param args 要拼接的字符串参数
        * @return 拼接后的字符串
        *
        * 使用示例:
        * std::string result = concat("Hello", " ", "World", "!", 123);
        * // 结果: "Hello World!123"
        */
    template<typename... Args>
    inline std::string concat(Args&&... args) {
        // 计算总长度以预分配内存
        size_t totalSize = (0 + ... + string_length(args));
        std::string result;
        result.reserve(totalSize);

        // 使用折叠表达式依次追加每个参数
        (append_to_string(result, std::forward<Args>(args)), ...);
        return result;
    }
} // namespace string
```

------



# 十七、[ctime](https://www.runoob.com/cprogramming/c-standard-library-time-h.html)（C++时间见 <'chrono>）

## 1. 介绍

**<u>时间计算，转换为字符串，转换为时间戳操作定义</u>**



### （1）函数签名功能说明

下表列出了主要的时间相关函数及其跨平台特性：

| 函数签名                                                     | 功能作用                       | Windows 安全注意事项          | Linux 方法    |
| ------------------------------------------------------------ | ------------------------------ | ----------------------------- | ------------- |
| `char* asctime(const struct tm*)`                            | 将tm结构转换为字符串           | 不安全，建议使用`asctime_s`   | `asctime_r`   |
| `clock_t clock(void)`                                        | 返回程序处理器时间             | 安全                          | 相同          |
| `char* ctime(const time_t*)`                                 | 将time_t转换为本地时间字符串   | 不安全，建议使用`ctime_s`     | `ctime_r`     |
| `double difftime(time_t, time_t)`                            | 计算两个时间之间的差值         | 安全                          | 相同          |
| `struct tm* gmtime(const time_t*)`                           | 将time_t转换为UTC时间的tm结构  | 不安全，建议使用`gmtime_s`    | `gmtime_r`    |
| `struct tm* localtime(const time_t*)`                        | 将time_t转换为本地时间的tm结构 | 不安全，建议使用`localtime_s` | `localtime_r` |
| `time_t mktime(struct tm*)`                                  | 将tm结构转换为time_t           | 安全                          | 相同          |
| `time_t time(time_t*)`                                       | 获取当前时间                   | 安全                          | 相同          |
| `size_t strftime(char*, size_t, const char*, const struct tm*)` | 格式化时间输出                 | 安全                          | 相同          |
| `int timespec_get(timespec*, int)`                           | 获取高精度时间                 | 安全                          | 相同          |

### （2）Windows 安全版本函数

Windows 提供了以下安全版本函数：

- `errno_t asctime_s(char*, size_t, const struct tm*)`
- `errno_t ctime_s(char*, size_t, const time_t*)`
- `errno_t gmtime_s(struct tm*, const time_t*)`
- `errno_t localtime_s(struct tm*, const time_t*)`

### （3）Linux 可重入版本函数

Linux 提供了以下可重入版本函数：

- `char* asctime_r(const struct tm*, char*)`
- `char* ctime_r(const time_t*, char*)`
- `struct tm* gmtime_r(const time_t*, struct tm*)`
- `struct tm* localtime_r(const time_t*, struct tm*)`

### Windows 安全函数使用说明

在 Windows 平台上，Microsoft 提供了一系列带有 `_s` 后缀的安全版本函数。要使用传统的非安全函数，需要在代码开头定义以下宏：

```cpp
#define _CRT_SECURE_NO_WARNINGS
```

或者可以在项目属性中设置：

- 项目属性 → C/C++ → 预处理器 → 预处理器定义 → 添加 `_CRT_SECURE_NO_WARNINGS`

## 2. 使用示例

```c
#include <iostream>
#include <ctime>
#include <cstring>

#ifdef _WIN32
// Windows 安全函数定义
#define _CRT_SECURE_NO_WARNINGS
#else
// Linux 可重入函数定义
char* asctime_r(const tm*, char*);
char* ctime_r(const time_t*, char*);
tm* gmtime_r(const time_t*, tm*);
tm* localtime_r(const time_t*, tm*);
#endif

int main() {
    // 获取当前时间
    time_t currentTime;
    time(&currentTime);
    
    std::cout << "当前时间戳: " << currentTime << std::endl;
    
    // 转换为字符串格式
    std::cout << "\n--- 时间字符串转换 ---" << std::endl;
    
    #ifdef _WIN32
    // Windows 安全版本
    char timeStr[26];
    ctime_s(timeStr, sizeof(timeStr), &currentTime);
    std::cout << "ctime_s: " << timeStr;
    #else
    // Linux 可重入版本
    char timeStr[26];
    ctime_r(&currentTime, timeStr);
    std::cout << "ctime_r: " << timeStr;
    #endif
    
    // 转换为tm结构
    std::cout << "\n--- 时间结构转换 ---" << std::endl;
    
    #ifdef _WIN32
    // Windows 安全版本
    tm localTime;
    localtime_s(&localTime, &currentTime);
    std::cout << "localtime_s: " << asctime(&localTime);
    #else
    // Linux 可重入版本
    tm localTime;
    localtime_r(&currentTime, &localTime);
    std::cout << "localtime_r: " << asctime(&localTime);
    #endif
    
    // 使用strftime格式化输出
    std::cout << "\n--- 格式化时间输出 ---" << std::endl;
    char formattedTime[80];
    
    #ifdef _WIN32
    localtime_s(&localTime, &currentTime);
    #else
    localtime_r(&currentTime, &localTime);
    #endif
    
    strftime(formattedTime, sizeof(formattedTime), "%Y-%m-%d %H:%M:%S", &localTime);
    std::cout << "格式化时间: " << formattedTime << std::endl;
    
    // 计算时间差
    std::cout << "\n--- 时间计算 ---" << std::endl;
    time_t futureTime = currentTime + 3600; // 1小时后
    double diff = difftime(futureTime, currentTime);
    std::cout << "时间差(秒): " << diff << std::endl;
    
    // 获取处理器时间
    std::cout << "\n--- 处理器时间 ---" << std::endl;
    clock_t start = clock();
    // 模拟一些工作
    for (volatile int i = 0; i < 1000000; i++) {}
    clock_t end = clock();
    std::cout << "处理器时间差: " << (double)(end - start) / CLOCKS_PER_SEC << "秒" << std::endl;
    
    // 高精度时间
    std::cout << "\n--- 高精度时间 ---" << std::endl;
    #ifdef _WIN32
    _timespec64 ts;
    _timespec64_get(&ts, TIME_UTC);
    #else
    timespec ts;
    timespec_get(&ts, TIME_UTC);
    #endif
    std::cout << "高精度时间: " << ts.tv_sec << "秒 " << ts.tv_nsec << "纳秒" << std::endl;
    
    return 0;
}
```



## 3. 和chrono的转换编写代码示例

使用  模板   和   std::chrono   进行实现

* time_point_to_str，time_point转换为string返回
* str_to_time_point，string转换为time_point返回
* get_gmtime，获取HTTP格式时间，例如: "Sun, 06 Nov 1994 08:49:37 GMT"
* get_timezone_offset，获取时区字符串，例如: "UTC/GMT+8:00"，即东八区

```c
namespace time {
    enum class TimeZone { Local, UTC };
    enum class TimePrecision { WithMilliseconds, WithoutMilliseconds };

    // 将时间点转换为格式化字符串
    template <TimeZone Zone = TimeZone::Local, TimePrecision Precision = TimePrecision::WithMilliseconds>
    inline std::string time_point_to_str(
        const std::chrono::system_clock::time_point& timePoint)
    {
        auto time = std::chrono::system_clock::to_time_t(timePoint);
        std::tm tmBuf{};

        // 选择时区转换函数
        if constexpr (Zone == TimeZone::Local) {
#ifdef _WIN32
            localtime_s(&tmBuf, &time);
#else
            localtime_r(&time, &tmBuf);
#endif
        }
        else {
#ifdef _WIN32
            gmtime_s(&tmBuf, &time);
#else
            gmtime_r(&time, &tmBuf);
#endif
        }

        std::ostringstream ss;
        ss << std::put_time(&tmBuf, "%Y-%m-%d %H:%M:%S");

        // 添加毫秒精度（如果启用）
        if constexpr (Precision == TimePrecision::WithMilliseconds) {
            auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(
                timePoint.time_since_epoch()
            ) % 1000;
            ss << '.' << std::setfill('0') << std::setw(3) << milliseconds.count();
        }

        return ss.str();
    }

    template <TimeZone Zone = TimeZone::Local,
        TimePrecision Precision = TimePrecision::WithMilliseconds>
    inline std::chrono::system_clock::time_point str_to_time_point(
        const std::string& str)
    {
        std::tm tm = {};
        std::istringstream ss(str);
        ss >> std::get_time(&tm, "%Y-%m-%d %H:%M:%S");

        if (ss.fail()) {
            throw std::runtime_error("Failed to parse time string");
        }

        // 解析毫秒部分（如果启用）
        int milliseconds = 0;
        if constexpr (Precision == TimePrecision::WithMilliseconds) {
            if (ss.peek() == '.') {
                ss.ignore();
                ss >> milliseconds;
                if (milliseconds < 0 || milliseconds > 999) {
                    throw std::runtime_error("Invalid milliseconds value");
                }
            }
            else {
                throw std::runtime_error("Expected milliseconds component");
            }
        }

        // 转换为 time_t（根据时区选择函数）
        std::time_t time;
        if constexpr (Zone == TimeZone::Local) {
            time = std::mktime(&tm);
        }
        else { // UTC 处理
#ifdef _WIN32
            time = _mkgmtime(&tm);
#else
            time = timegm(&tm);
#endif
        }

        if (time == -1) {
            throw std::runtime_error("Invalid time structure");
        }

        // 转换为 time_point 并添加毫秒
        auto tp = std::chrono::system_clock::from_time_t(time);
        return tp + std::chrono::milliseconds(milliseconds);
    }

    // 获取HTTP日期格式的当前时间
    inline std::string get_gmtime() {
        // 获取当前时间点
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);

        // 转换为GMT时间
        std::tm gmt_tm;
#ifdef _WIN32
        gmtime_s(&gmt_tm, &time_t_now);
#else
        gmtime_r(&time_t_now, &gmt_tm);
#endif

        // 格式化为HTTP日期格式 (RFC 7231)
        // 例如: "Sun, 06 Nov 1994 08:49:37 GMT"
        std::stringstream ss;
        ss << std::put_time(&gmt_tm, "%a, %d %b %Y %H:%M:%S GMT");
        return ss.str();
    }

    inline std::string get_timezone_offset() {
        std::time_t t = std::time(nullptr);
        std::tm local_tm, utc_tm;

        // 获取本地时间和UTC时间
#if defined(_WIN32)
        localtime_s(&local_tm, &t);
        gmtime_s(&utc_tm, &t);
#else
        localtime_r(&t, &local_tm);
        gmtime_r(&t, &utc_tm);
#endif

        // 计算时间差（分钟）
        int local_min = local_tm.tm_hour * 60 + local_tm.tm_min;
        int utc_min = utc_tm.tm_hour * 60 + utc_tm.tm_min;
        int diff_min = local_min - utc_min;

        // 处理跨日情况
        if (local_tm.tm_mday != utc_tm.tm_mday) {
            diff_min += (local_tm.tm_mday > utc_tm.tm_mday) ? 1440 : -1440;
        }

        // 转换为小时和分钟
        int hours = diff_min / 60;
        int minutes = std::abs(diff_min % 60);

        // 格式化为字符串
        std::ostringstream oss;
        oss << "UTC/GMT"
            << (diff_min >= 0 ? "+" : "-")
            << std::setfill('0') << std::setw(2) << std::abs(hours)
            << ":"
            << std::setfill('0') << std::setw(2) << minutes;

        return oss.str();
    }
}
```

------



# 十八、cuchar（C++ UTF字面量见 <'string_literal>）

> char8_t 于 C++20引入，*<u>多字节/宽字符转换函数</u>* 见： 	**<u>第十五章 cstdlib 介绍（6）</u>**

## 1. 介绍

**<u>Unicode的字符转换定义，utf-16、utf-32</u>**



### （1）函数列表

* `size_t mbrtoc16(char16_t* pc16, const char* s, size_t n, mbstate_t* ps)`: 将多字节字符转换为 UTF-16 编码
* `size_t c16rtomb(char* s, char16_t c16, mbstate_t* ps)`: 将 UTF-16 字符转换为多字节编码
* `size_t mbrtoc32(char32_t* pc32, const char* s, size_t n, mbstate_t* ps)`: 将多字节字符转换为 UTF-32 编码
* `size_t c32rtomb(char* s, char32_t c32, mbstate_t* ps)`: 将 UTF-32 字符转换为多字节编码

### （2）函数详细说明

#### a. mbrtoc16

- **功能**: 将多字节字符转换为UTF-16编码
- **参数**:
  - `pc16`: 指向存储转换结果的char16_t指针
  - `s`: 指向多字节字符的指针
  - `n`: 要检查的最大字节数
  - `ps`: 指向转换状态的指针
- **返回值**:
  - 0: 如果转换的是空字符
  - 1..n: 转换使用的字节数
  - static_cast<size_t>(-2): 输入不完整
  - static_cast<size_t>(-1): 编码错误

#### b. c16rtomb

- **功能**: 将UTF-16字符转换为多字节编码
- **参数**:
  - `s`: 指向存储转换结果的字符指针
  - `c16`: 要转换的UTF-16字符
  - `ps`: 指向转换状态的指针
- **返回值**:
  - 写入的字节数
  - static_cast<size_t>(-1): 编码错误

#### c. mbrtoc32

- **功能**: 将多字节字符转换为UTF-32编码
- **参数**: 与mbrtoc16类似，但输出是char32_t
- **返回值**: 与mbrtoc16相同

#### d. c32rtomb

- **功能**: 将UTF-32字符转换为多字节编码
- **参数**: 与c16rtomb类似，但输入是char32_t
- **返回值**: 与c16rtomb相同

## 2. 示例代码

```cpp
#include <cuchar>
#include <iostream>
#include <clocale>
#include <cstring>

int main() {
    // 设置本地化环境
    std::setlocale(LC_ALL, "en_US.utf8");
    
    // 初始化转换状态
    std::mbstate_t state{};
    
    // 示例1: 多字节字符到UTF-16转换
    std::cout << "=== 多字节字符到UTF-16转换 ===" << std::endl;
    
    const char* mbstr = "Hello, 世界!"; // 包含中文字符的字符串
    std::cout << "原始字符串: " << mbstr << std::endl;
    
    char16_t c16;
    const char* ptr = mbstr;
    size_t bytes_consumed;
    int count = 0;
    
    std::cout << "UTF-16 编码: ";
    while ((bytes_consumed = std::mbrtoc16(&c16, ptr, std::strlen(ptr), &state)) > 0) {
        std::cout << "U+" << std::hex << static_cast<uint16_t>(c16) << " ";
        ptr += bytes_consumed;
        count++;
    }
    std::cout << std::dec << "\n共转换了 " << count << " 个字符" << std::endl;
    
    // 示例2: UTF-16到多字节字符转换
    std::cout << "\n=== UTF-16到多字节字符转换 ===" << std::endl;
    
    // 重置状态
    state = std::mbstate_t{};
    
    char16_t utf16_chars[] = u"Hello, 世界!";
    char mb_output[100]{};
    char* output_ptr = mb_output;
    
    for (size_t i = 0; i < sizeof(utf16_chars)/sizeof(utf16_chars[0]) - 1; ++i) {
        size_t result = std::c16rtomb(output_ptr, utf16_chars[i], &state);
        if (result != static_cast<size_t>(-1)) {
            output_ptr += result;
        }
    }
    
    std::cout << "转换回多字节: " << mb_output << std::endl;
    
    // 示例3: 多字节字符到UTF-32转换
    std::cout << "\n=== 多字节字符到UTF-32转换 ===" << std::endl;
    
    // 重置状态
    state = std::mbstate_t{};
    ptr = mbstr;
    count = 0;
    
    char32_t c32;
    std::cout << "UTF-32 编码: ";
    while ((bytes_consumed = std::mbrtoc32(&c32, ptr, std::strlen(ptr), &state)) > 0) {
        std::cout << "U+" << std::hex << static_cast<uint32_t>(c32) << " ";
        ptr += bytes_consumed;
        count++;
    }
    std::cout << std::dec << "\n共转换了 " << count << " 个字符" << std::endl;
    
    // 示例4: UTF-32到多字节字符转换
    std::cout << "\n=== UTF-32到多字节字符转换 ===" << std::endl;
    
    // 重置状态
    state = std::mbstate_t{};
    
    char32_t utf32_chars[] = U"Hello, 世界!";
    char mb_output2[100]{};
    output_ptr = mb_output2;
    
    for (size_t i = 0; i < sizeof(utf32_chars)/sizeof(utf32_chars[0]) - 1; ++i) {
        size_t result = std::c32rtomb(output_ptr, utf32_chars[i], &state);
        if (result != static_cast<size_t>(-1)) {
            output_ptr += result;
        }
    }
    
    std::cout << "转换回多字节: " << mb_output2 << std::endl;
    
    return 0;
}
```

------



# 十九、cwchar

> 详细见	**<u>第十四章 cstdio	第十五章 cstdlib	第十六章 cstring	第十七章 ctime</u>**	的窄字符方法定义



## 1. 介绍

***宽字符*** 的 **<u>文件操作、控制台打印输入IO、环境变量获取、字符串拼接/长度/查找/比较/拷贝/移动、转换为数值类型的定义、时间格式化</u>**

### （1）源码定义

```c
using _CSTD _Mbstatet;

_EXPORT_STD using _CSTD mbstate_t;
_EXPORT_STD using _CSTD size_t;
_EXPORT_STD using _CSTD tm;
_EXPORT_STD using _CSTD wint_t;

_EXPORT_STD using _CSTD btowc;
_EXPORT_STD using _CSTD fgetwc;
_EXPORT_STD using _CSTD fgetws;
_EXPORT_STD using _CSTD fputwc;
_EXPORT_STD using _CSTD fputws;
_EXPORT_STD using _CSTD fwide;
_EXPORT_STD using _CSTD fwprintf;
_EXPORT_STD using _CSTD fwscanf;
_EXPORT_STD using _CSTD getwc;
_EXPORT_STD using _CSTD getwchar;
_EXPORT_STD using _CSTD mbrlen;
_EXPORT_STD using _CSTD mbrtowc;
_EXPORT_STD using _CSTD mbsrtowcs;
_EXPORT_STD using _CSTD mbsinit;
_EXPORT_STD using _CSTD putwc;
_EXPORT_STD using _CSTD putwchar;
_EXPORT_STD using _CSTD swprintf;
_EXPORT_STD using _CSTD swscanf;
_EXPORT_STD using _CSTD ungetwc;
_EXPORT_STD using _CSTD vfwprintf;
_EXPORT_STD using _CSTD vswprintf;
_EXPORT_STD using _CSTD vwprintf;
_EXPORT_STD using _CSTD wcrtomb;
_EXPORT_STD using _CSTD wprintf;
_EXPORT_STD using _CSTD wscanf;
_EXPORT_STD using _CSTD wcsrtombs;
_EXPORT_STD using _CSTD wcstol;
_EXPORT_STD using _CSTD wcscat;
_EXPORT_STD using _CSTD wcschr;
_EXPORT_STD using _CSTD wcscmp;
_EXPORT_STD using _CSTD wcscoll;
_EXPORT_STD using _CSTD wcscpy;
_EXPORT_STD using _CSTD wcscspn;
_EXPORT_STD using _CSTD wcslen;
_EXPORT_STD using _CSTD wcsncat;
_EXPORT_STD using _CSTD wcsncmp;
_EXPORT_STD using _CSTD wcsncpy;
_EXPORT_STD using _CSTD wcspbrk;
_EXPORT_STD using _CSTD wcsrchr;
_EXPORT_STD using _CSTD wcsspn;
_EXPORT_STD using _CSTD wcstod;
_EXPORT_STD using _CSTD wcstoul;
_EXPORT_STD using _CSTD wcsstr;
_EXPORT_STD using _CSTD wcstok;
_EXPORT_STD using _CSTD wcsxfrm;
_EXPORT_STD using _CSTD wctob;
_EXPORT_STD using _CSTD wmemchr;
_EXPORT_STD using _CSTD wmemcmp;
_EXPORT_STD using _CSTD wmemcpy;
_EXPORT_STD using _CSTD wmemmove;
_EXPORT_STD using _CSTD wmemset;
_EXPORT_STD using _CSTD wcsftime;

_EXPORT_STD using _CSTD vfwscanf;
_EXPORT_STD using _CSTD vswscanf;
_EXPORT_STD using _CSTD vwscanf;
_EXPORT_STD using _CSTD wcstof;
_EXPORT_STD using _CSTD wcstold;
_EXPORT_STD using _CSTD wcstoll;
_EXPORT_STD using _CSTD wcstoull;
```



### （2）宽字符函数分类说明

#### a. 宽字符输入输出函数

| 函数签名                                           | 功能作用                     | Windows 安全注意事项          |
| -------------------------------------------------- | ---------------------------- | ----------------------------- |
| `wint_t fgetwc(FILE*)`                             | 从文件流读取宽字符           | 安全                          |
| `wchar_t* fgetws(wchar_t*, int, FILE*)`            | 从文件流读取宽字符串         | 安全                          |
| `wint_t fputwc(wchar_t, FILE*)`                    | 向文件流写入宽字符           | 安全                          |
| `int fputws(const wchar_t*, FILE*)`                | 向文件流写入宽字符串         | 安全                          |
| `int fwide(FILE*, int)`                            | 设置/获取文件流方向          | 安全                          |
| `int fwprintf(FILE*, const wchar_t*, ...)`         | 格式化输出到文件流           | 不安全，建议使用`fwprintf_s`  |
| `int fwscanf(FILE*, const wchar_t*, ...)`          | 从文件流格式化输入           | 不安全，建议使用`fwscanf_s`   |
| `wint_t getwc(FILE*)`                              | 从文件流读取宽字符           | 安全                          |
| `wint_t getwchar(void)`                            | 从标准输入读取宽字符         | 安全                          |
| `wint_t putwc(wchar_t, FILE*)`                     | 向文件流写入宽字符           | 安全                          |
| `wint_t putwchar(wchar_t)`                         | 向标准输出写入宽字符         | 安全                          |
| `int swprintf(wchar_t*, const wchar_t*, ...)`      | 格式化输出到宽字符串         | 不安全，建议使用`swprintf_s`  |
| `int swscanf(const wchar_t*, const wchar_t*, ...)` | 从宽字符串格式化输入         | 不安全，建议使用`swscanf_s`   |
| `int vfwprintf(FILE*, const wchar_t*, va_list)`    | 可变参数格式化输出到文件流   | 不安全，建议使用`vfwprintf_s` |
| `int vswprintf(wchar_t*, const wchar_t*, va_list)` | 可变参数格式化输出到宽字符串 | 不安全，建议使用`vswprintf_s` |
| `int vwprintf(const wchar_t*, va_list)`            | 可变参数格式化输出到标准输出 | 不安全，建议使用`vwprintf_s`  |
| `int wprintf(const wchar_t*, ...)`                 | 格式化输出到标准输出         | 不安全，建议使用`wprintf_s`   |
| `int wscanf(const wchar_t*, ...)`                  | 从标准输入格式化输入         | 不安全，建议使用`wscanf_s`    |
| `wint_t ungetwc(wint_t, FILE*)`                    | 将宽字符推回输入流           | 安全                          |

#### b. 宽字符转换函数

| 函数签名                                                     | 功能作用                     | Windows 安全注意事项          |
| ------------------------------------------------------------ | ---------------------------- | ----------------------------- |
| `wint_t btowc(int)`                                          | 将单字节字符转换为宽字符     | 安全                          |
| `size_t mbrlen(const char*, size_t, mbstate_t*)`             | 计算多字节字符长度           | 安全                          |
| `size_t mbrtowc(wchar_t*, const char*, size_t, mbstate_t*)`  | 将多字节字符转换为宽字符     | 安全                          |
| `size_t mbsrtowcs(wchar_t*, const char**, size_t, mbstate_t*)` | 将多字节字符串转换为宽字符串 | 不安全，建议使用`mbsrtowcs_s` |
| `int mbsinit(const mbstate_t*)`                              | 检查转换状态是否初始         | 安全                          |
| `size_t wcrtomb(char*, wchar_t, mbstate_t*)`                 | 将宽字符转换为多字节字符     | 安全                          |
| `size_t wcsrtombs(char*, const wchar_t**, size_t, mbstate_t*)` | 将宽字符串转换为多字节字符串 | 不安全，建议使用`wcsrtombs_s` |
| `int wctob(wint_t)`                                          | 将宽字符转换为单字节字符     | 安全                          |

#### c. 宽字符串操作函数

| 函数签名                                               | 功能作用                           | Windows 安全注意事项        |
| ------------------------------------------------------ | ---------------------------------- | --------------------------- |
| `wchar_t* wcscat(wchar_t*, const wchar_t*)`            | 连接宽字符串                       | 不安全，建议使用`wcscat_s`  |
| `wchar_t* wcschr(const wchar_t*, wchar_t)`             | 查找宽字符在字符串中的位置         | 安全                        |
| `int wcscmp(const wchar_t*, const wchar_t*)`           | 比较两个宽字符串                   | 安全                        |
| `int wcscoll(const wchar_t*, const wchar_t*)`          | 根据区域设置比较宽字符串           | 安全                        |
| `wchar_t* wcscpy(wchar_t*, const wchar_t*)`            | 复制宽字符串                       | 不安全，建议使用`wcscpy_s`  |
| `size_t wcscspn(const wchar_t*, const wchar_t*)`       | 计算不在指定字符集中的前缀长度     | 安全                        |
| `size_t wcslen(const wchar_t*)`                        | 计算宽字符串长度                   | 安全                        |
| `wchar_t* wcsncat(wchar_t*, const wchar_t*, size_t)`   | 连接指定长度的宽字符串             | 不安全，建议使用`wcsncat_s` |
| `int wcsncmp(const wchar_t*, const wchar_t*, size_t)`  | 比较指定长度的宽字符串             | 安全                        |
| `wchar_t* wcsncpy(wchar_t*, const wchar_t*, size_t)`   | 复制指定长度的宽字符串             | 不安全，建议使用`wcsncpy_s` |
| `wchar_t* wcspbrk(const wchar_t*, const wchar_t*)`     | 查找宽字符串中任何指定字符的位置   | 安全                        |
| `wchar_t* wcsrchr(const wchar_t*, wchar_t)`            | 查找宽字符在字符串中最后出现的位置 | 安全                        |
| `size_t wcsspn(const wchar_t*, const wchar_t*)`        | 计算在指定字符集中的前缀长度       | 安全                        |
| `wchar_t* wcsstr(const wchar_t*, const wchar_t*)`      | 查找子宽字符串                     | 安全                        |
| `wchar_t* wcstok(wchar_t*, const wchar_t*, wchar_t**)` | 分割宽字符串为标记                 | 安全                        |
| `size_t wcsxfrm(wchar_t*, const wchar_t*, size_t)`     | 根据区域设置转换宽字符串           | 安全                        |
| `wchar_t* wmemchr(const wchar_t*, wchar_t, size_t)`    | 在宽字符数组中查找字符             | 安全                        |
| `int wmemcmp(const wchar_t*, const wchar_t*, size_t)`  | 比较宽字符数组                     | 安全                        |
| `wchar_t* wmemcpy(wchar_t*, const wchar_t*, size_t)`   | 复制宽字符数组                     | 不安全，建议使用`wmemcpy_s` |
| `wchar_t* wmemmove(wchar_t*, const wchar_t*, size_t)`  | 移动宽字符数组                     | 安全                        |
| `wchar_t* wmemset(wchar_t*, wchar_t, size_t)`          | 设置宽字符数组                     | 安全                        |

#### d. 宽字符串数值转换函数

| 函数签名                                                     | 功能作用                       | Windows 安全注意事项 |
| ------------------------------------------------------------ | ------------------------------ | -------------------- |
| `long wcstol(const wchar_t*, wchar_t**, int)`                | 将宽字符串转换为长整数         | 安全                 |
| `double wcstod(const wchar_t*, wchar_t**)`                   | 将宽字符串转换为双精度浮点数   | 安全                 |
| `unsigned long wcstoul(const wchar_t*, wchar_t**, int)`      | 将宽字符串转换为无符号长整数   | 安全                 |
| `float wcstof(const wchar_t*, wchar_t**)`                    | 将宽字符串转换为单精度浮点数   | 安全                 |
| `long double wcstold(const wchar_t*, wchar_t**)`             | 将宽字符串转换为长双精度浮点数 | 安全                 |
| `long long wcstoll(const wchar_t*, wchar_t**, int)`          | 将宽字符串转换为长长整数       | 安全                 |
| `unsigned long long wcstoull(const wchar_t*, wchar_t**, int)` | 将宽字符串转换为无符号长长整数 | 安全                 |

#### e. 宽字符时间格式化函数

| 函数签名                                                     | 功能作用               | Windows 安全注意事项 |
| ------------------------------------------------------------ | ---------------------- | -------------------- |
| `size_t wcsftime(wchar_t*, size_t, const wchar_t*, const struct tm*)` | 格式化宽字符时间字符串 | 安全                 |

### Windows 安全版本函数

Windows 提供了以下安全版本函数：
- `errno_t wcscat_s(wchar_t*, size_t, const wchar_t*)`
- `errno_t wcscpy_s(wchar_t*, size_t, const wchar_t*)`
- `errno_t wcsncat_s(wchar_t*, size_t, const wchar_t*, size_t)`
- `errno_t wcsncpy_s(wchar_t*, size_t, const wchar_t*, size_t)`
- `errno_t wmemcpy_s(wchar_t*, size_t, const wchar_t*, size_t)`
- `errno_t mbsrtowcs_s(size_t*, wchar_t*, size_t, const char**, size_t, mbstate_t*)`
- `errno_t wcsrtombs_s(size_t*, char*, size_t, const wchar_t**, size_t, mbstate_t*)`
- `int fwprintf_s(FILE*, const wchar_t*, ...)`
- `int fwscanf_s(FILE*, const wchar_t*, ...)`
- `int swprintf_s(wchar_t*, size_t, const wchar_t*, ...)`
- `int swscanf_s(const wchar_t*, const wchar_t*, ...)`
- `int vfwprintf_s(FILE*, const wchar_t*, va_list)`
- `int vswprintf_s(wchar_t*, size_t, const wchar_t*, va_list)`
- `int vwprintf_s(const wchar_t*, va_list)`
- `int wprintf_s(const wchar_t*, ...)`
- `int wscanf_s(const wchar_t*, ...)`

## 2. 示例代码

```cpp
#include <cwchar>
#include <iostream>
#include <clocale>
#include <ctime>

int main() {
    // 设置本地化环境以支持宽字符
    std::setlocale(LC_ALL, "");
    
    // 示例1: 宽字符串基本操作
    std::wcout << L"=== 宽字符串基本操作 ===" << std::endl;
    
    wchar_t str1[20] = L"Hello, ";
    wchar_t str2[] = L"世界!";
    
    // 连接字符串 (使用安全版本)
    #ifdef _WIN32
    wcscat_s(str1, 20, str2);
    #else
    wcscat(str1, str2);
    #endif
    
    std::wcout << L"连接后的字符串: " << str1 << std::endl;
    std::wcout << L"字符串长度: " << wcslen(str1) << std::endl;
    
    // 示例2: 宽字符输入输出
    std::wcout << L"\n=== 宽字符输入输出 ===" << std::endl;
    
    wchar_t input[100];
    std::wcout << L"请输入一些文本: ";
    std::wcin.getline(input, 100);
    std::wcout << L"您输入的是: " << input << std::endl;
    
    // 示例3: 宽字符串比较和查找
    std::wcout << L"\n=== 宽字符串比较和查找 ===" << std::endl;
    
    const wchar_t* text = L"这是一个示例文本，包含一些字符";
    wchar_t search = L'包';
    
    wchar_t* found = wcschr(text, search);
    if (found) {
        std::wcout << L"找到字符 '" << search << L"' 在位置: " << (found - text) << std::endl;
    }
    
    // 示例4: 宽字符串数值转换
    std::wcout << L"\n=== 宽字符串数值转换 ===" << std::endl;
    
    const wchar_t* numStr = L"123.45";
    wchar_t* endPtr;
    double num = wcstod(numStr, &endPtr);
    
    std::wcout << L"转换后的数值: " << num << std::endl;
    std::wcout << L"未转换部分: " << endPtr << std::endl;
    
    // 示例5: 宽字符时间格式化
    std::wcout << L"\n=== 宽字符时间格式化 ===" << std::endl;
    
    std::time_t now = std::time(nullptr);
    std::tm* timeInfo = std::localtime(&now);
    
    wchar_t timeStr[100];
    std::wcsftime(timeStr, 100, L"%Y年%m月%d日 %H:%M:%S", timeInfo);
    std::wcout << L"当前时间: " << timeStr << std::endl;
    
    // 示例6: 多字节与宽字符转换
    std::wcout << L"\n=== 多字节与宽字符转换 ===" << std::endl;
    
    const char* mbStr = "这是一个多字节字符串";
    wchar_t wcStr[100];
    std::mbstate_t state{};
    
    #ifdef _WIN32
    size_t converted;
    mbsrtowcs_s(&converted, wcStr, 100, &mbStr, strlen(mbStr), &state);
    #else
    mbsrtowcs(wcStr, &mbStr, 100, &state);
    #endif
    
    std::wcout << L"转换后的宽字符串: " << wcStr << std::endl;
    
    return 0;
}
```

------



# 二十、cwctype（详细见第二章cctype）

------



# 、ciso646（deprecated in C++20）

------

# 、ccomplex（deprecated in C++20）

------

# 、cstdbool（deprecated in C++20）

------

