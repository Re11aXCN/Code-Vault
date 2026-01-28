# 博客介绍：

https://zhuanlan.zhihu.com/p/704205474

https://www.jianshu.com/p/3d369ce911e7

https://geekdaxue.co/read/manbuyihai@ttxghn/fsossb



官方文档

https://tqfx.org/clang-format/BasedOnStyle/

https://clang.llvm.org/docs/ClangFormatStyleOptions.html

https://clang.llvm.net.cn/docs/ClangFormatStyleOptions.html



LLVM下载

https://github.com/llvm/llvm-project/releases

# 示例：

以下是一个全面详细的 `.clang-format` 配置文件，适用于 C++ 项目，包含详细注释说明每个选项的功能：

```yaml
---
# clang-format 版本：15+
# 基础配置风格：基于 Google 风格，但做了大量自定义
BasedOnStyle: Google

# 语言配置
Language: Cpp

# 访问说明符（public、private、protected）的缩进
AccessModifierOffset: -1
# 类继承列表的冒号对齐方式
AlignAfterOpenBracket: Align
# 连续行的对齐方式
AlignConsecutiveAssignments: true
AlignConsecutiveDeclarations: true
# 宏定义对齐
AlignEscapedNewlines: Right
# 运算符对齐
AlignOperands: true
# 连续尾随注释对齐
AlignTrailingComments: true

# 允许函数声明和定义所有参数放在同一行
AllowAllParametersOfDeclarationOnNextLine: false
# 允许短块/短语句放在同一行
AllowShortBlocksOnASingleLine: Empty
# 允许短 case 标签放在同一行
AllowShortCaseLabelsOnASingleLine: true
# 允许短函数放在一行
AllowShortFunctionsOnASingleLine: All
# 允许短 if 语句放在一行
AllowShortIfStatementsOnASingleLine: WithoutElse
# 允许短 lambdas 放在一行
AllowShortLambdasOnASingleLine: All
# 允许短循环放在一行
AllowShortLoopsOnASingleLine: true

# 总是在返回类型后换行
AlwaysBreakAfterReturnType: None
# 总是在模板声明后换行
AlwaysBreakTemplateDeclarations: Yes
# 多行字符串字面量的对齐方式
BinPackArguments: true
BinPackParameters: true

# 大括号风格
BreakBeforeBinaryOperators: None
BreakBeforeBraces: Attach
BreakBeforeTernaryOperators: true
BreakConstructorInitializers: BeforeColon
BreakInheritanceList: BeforeColon

# 列限制
ColumnLimit: 120

# 缩进设置
IndentCaseLabels: true
IndentCaseBlocks: false
IndentGotoLabels: true
IndentPPDirectives: AfterHash
IndentExternBlock: AfterExternBlock
IndentWidth: 4
IndentWrappedFunctionNames: false

# 保持特定结构
KeepEmptyLinesAtTheStartOfBlocks: false
MaxEmptyLinesToKeep: 1

# 命名空间
NamespaceIndentation: All
# 嵌套命名空间缩进
FixNamespaceComments: true

# 指针和引用对齐
PointerAlignment: Left
ReferenceAlignment: Left

# 空格设置
# 在赋值运算符前后加空格
SpaceAfterCStyleCast: false
SpaceAfterLogicalNot: false
SpaceAfterTemplateKeyword: true
SpaceBeforeAssignmentOperators: true
SpaceBeforeCpp11BracedList: false
SpaceBeforeCtorInitializerColon: true
SpaceBeforeInheritanceColon: true
SpaceBeforeParens: ControlStatements
SpaceBeforeRangeBasedForLoopColon: true
SpaceBeforeSquareBrackets: false
SpaceInEmptyBlock: false
SpaceInEmptyParentheses: false
# 在二元运算符前后加空格
SpacesBeforeTrailingComments: 2
SpacesInAngles: Never
SpacesInConditionalStatement: false
SpacesInContainerLiterals: true
SpacesInCStyleCastParentheses: false
SpacesInParentheses: false
SpacesInSquareBrackets: false

# 换行设置
# 在 << 运算符后换行
BreakBeforeConceptDeclarations: true
Standard: Latest

# 制表符设置
UseTab: Never
TabWidth: 4

# 其他
# 宏定义块中的空行
EmptyLineBeforeAccessModifier: LogicalBlock
# 函数定义后保留空行
EmptyLinesAfterAccessModifier: 1
# 单个函数内最大空行数
MaxEmptyLinesToKeep: 1
# 排序 includes
SortIncludes: CaseSensitive
# JavaScript 包装器引号
JavaScriptQuotes: Leave
# Java 导入分组
JavaImportGroups: ['com.', 'org.', 'net.']
# 解析时使用的源文件类型
# 可以是 None、CSharp、Java、ObjC、Proto
DerivePointerAlignment: false
# 对非赋值运算符的换行惩罚
PenaltyBreakAssignment: 2
PenaltyBreakBeforeFirstCallParameter: 19
PenaltyBreakComment: 300
PenaltyBreakFirstLessLess: 120
PenaltyBreakString: 1000
PenaltyBreakTemplateDeclaration: 10
PenaltyExcessCharacter: 1000000
PenaltyReturnTypeOnItsOwnLine: 60
PenaltyIndentedWhitespace: 0

# 属性设置
# 将 C++11 属性放在与声明/定义相同行
BreakBeforeAttribute: Never
AttributeMacros:
  - __capability

# 包装模板表达式
AlwaysBreakBeforeMultilineStrings: false
# 注释格式化
ReflowComments: true
# 连续行的最小行数
MinEmptyLinesBeforeReturn: 0
# 排序 using 声明
SortUsingDeclarations: true
# 空格敏感宏
WhitespaceSensitiveMacros:
  - STRINGIZE
  - PP_STRINGIZE
  - BOOST_PP_STRINGIZE
  - NS_SWIFT_NAME
  - CF_SWIFT_NAME

# 文件编码
Encoding: UTF-8

# 语句宏
StatementMacros:
  - Q_UNUSED
  - QT_REQUIRE_VERSION

# 插入空行
EmptyLineAfterReturn: false
EmptyLineAtEndOfFile: true

# 模块设置
# 在模块导入后插入空行
EmptyLineAfterNamespace: false

# 预处理器指令缩进
IndentPPDirectives: BeforeHash

# 模板格式化
AlwaysBreakTemplateDeclarations: MultiLine
BreakTemplateDeclarations: Yes

# 概念格式化
SpaceAroundConceptRequiresClause: true

# 字符串字面量引号
# 对于 C++，这通常不适用，但为其他语言保留
# CSharpActionEOLCommentFormatting: None

# 对特定语言的特殊处理
# 对于 Objective-C
ObjCBreakBeforeNestedBlockParam: true
ObjCBreakBeforeNestedBlockParam: true
ObjCSpaceAfterProperty: false
ObjCSpaceBeforeProtocolList: true

# 对于 JavaScript
JavaScriptWrapImports: true

# 对于 Proto
SpacesInLineCommentPrefix:
  Minimum: 1
  Maximum: -1

# 对于 C#
CSharpSpaceAfterCast: false
CSharpSpaceAfterKeywordsInControlFlowStatements: true
CSharpSpaceBetweenParentheses: false
CSharpSpaceBeforeColonInBaseTypeDeclaration: true
CSharpSpaceAfterColonInBaseTypeDeclaration: true
CSharpSpaceAroundBinaryExpressionOperators: true
CSharpSpaceBetweenSquares: false

# 实验性功能（clang-format 15+）
# 需要谨慎使用，可能在不同版本中行为不同
ExperimentalAutoDetectBinPacking: false
FixNamespaceComments: true
QualifierAlignment: Leave
SeparateDefinitionBlocks: Always
ShortNamespaceLines: 1
SpaceBeforeCaseColon: false
SpaceBeforeParensOptions:
  AfterControlStatements: true
  AfterFunctionDefinitionName: false
  AfterFunctionDeclarationName: false
  AfterIfMacro: false
  AfterOverloadedOperator: false
  BeforeNonEmptyParentheses: false

# 项目特定配置
# 根据项目需要调整以下设置

# 1. 对于头文件保护宏
# Define: '#ifndef HEADER_GUARD'
# 可以通过脚本或 IDE 插件自动添加

# 2. 对于模板化代码
# 保持模板声明整洁，多行时合理换行

# 3. 对于 lambda 表达式
# 短 lambdas 保持在一行，长 lambdas 适当换行

# 4. 对于范围 for 循环
# 保持简洁，复杂表达式适当换行

# 5. 对于概念和约束
# 使用 C++20 概念时，保持清晰的可读性

# 使用说明：
# 1. 将此文件放置在项目根目录或源代码目录中
# 2. 大多数 IDE 和编辑器都能自动识别
# 3. 可以通过命令行手动格式化：
#    clang-format -i -style=file <源文件>
# 4. 对于整个项目：
#    find . -name "*.cpp" -o -name "*.hpp" -o -name "*.h" | xargs clang-format -i -style=file

# 注意：
# 1. 某些选项可能需要特定版本的 clang-format
# 2. 不同团队可能有不同的代码风格偏好，可根据需要调整
# 3. 建议在项目开始前确定并统一代码风格
# 4. 可以使用 .clang-format 的继承功能创建基础配置

# 扩展建议：
# 1. 配合 pre-commit 钩子自动格式化
# 2. 在 CI/CD 流程中加入格式检查
# 3. 使用 clang-tidy 进行更严格的代码质量检查
```

## 关键选项说明：

### 1. **缩进与空格**
- `IndentWidth: 4`：使用 4 空格缩进
- `UseTab: Never`：禁止使用制表符
- `PointerAlignment: Left`：指针和引用靠左对齐

### 2. **行长度**
- `ColumnLimit: 120`：最大行宽 120 字符
- 超过限制时会自动换行

### 3. **大括号风格**
- `BreakBeforeBraces: Attach`：大括号不单独占行
```cpp
// 示例
void func() {
    // 代码
}
```

### 4. **空行控制**
- `MaxEmptyLinesToKeep: 1`：最多保留一个空行
- 避免过多空行使代码松散

### 5. **函数与语句**
- `AllowShortFunctionsOnASingleLine: All`：短函数可以放在一行
- `AllowShortIfStatementsOnASingleLine: WithoutElse`：简单 if 语句可以放在一行（无 else）

### 6. **包含文件排序**
- `SortIncludes: CaseSensitive`：对 #include 进行排序
- 通常按字母顺序排列

## 使用方法：

1. **放置位置**：
   - 项目根目录：影响整个项目
   - 子目录：影响该目录及子目录

2. **IDE/编辑器集成**：
   - VS Code：安装 C/C++ 扩展
   - CLion：默认支持
   - Vim/Neovim：使用 ALE 或 coc-clangd
   - VS：安装 ClangPowerTools

3. **命令行使用**：
```bash
# 格式化单个文件
clang-format -i -style=file source.cpp

# 格式化目录下所有文件
find . -name "*.cpp" -o -name "*.hpp" | xargs clang-format -i -style=file

# 检查格式（CI/CD 中使用）
clang-format --dry-run --Werror -style=file source.cpp
```

4. **预提交钩子示例**（.pre-commit-config.yaml）：
```yaml
repos:
  - repo: https://github.com/pre-commit/mirrors-clang-format
    rev: v15.0.7
    hooks:
      - id: clang-format
```

## 自定义建议：

1. **团队协作**：
   - 讨论确定团队统一的代码风格
   - 在 README 中记录风格决策

2. **项目特定调整**：
   - 遗留代码：逐步应用格式
   - 第三方库：排除格式化

3. **性能考虑**：
   - 大型项目：增量格式化
   - 避免格式化生成的代码

这个配置文件提供了良好的默认值，你可以根据团队偏好调整具体选项。建议在项目开始时确定风格，并保持一致性。

# Clang-Format Cpp 配置参数代码示例（开启/不开启对比）

## 1. AccessModifierOffset: -4（访问说明符偏移）

### 配置说明

控制类/结构体中 `public`、`private`、`protected` 的缩进偏移，-4 表示比类的缩进级别少 4 个空格（向左偏移 4 空格）。

#### 不开启（默认值，通常为 0，无偏移）

```cpp
class MyClass {
    // 访问说明符与类内代码缩进一致（无偏移）
    public:
        void func1(); // 成员函数额外缩进 4 空格
    private:
        int m_value; // 成员变量额外缩进 4 空格
};
```

#### 开启（AccessModifierOffset: -4）

```cpp
class MyClass {
// 访问说明符向左偏移 4 空格（与类的左大括号对齐）
public:
    void func1(); // 成员函数仅缩进 4 空格（整体更紧凑）
private:
    int m_value; // 成员变量仅缩进 4 空格
};
```

## 2. AlignAfterOpenBracket: Align（开括号后对齐）

### 配置说明

开圆括号 `(`、尖括号 `<`、方括号 `[` 后，后续内容与括号内第一个元素对齐。

#### 不开启（DontAlign/AlwaysBreak，以 DontAlign 为例）

```cpp
// 函数调用参数不与括号内第一个元素对齐，仅按固定缩进
void func(int a,
    double b,
    std::string c);

// 模板参数同理
std::vector<std::map<int,
    std::string>> my_vec;
```

#### 开启（AlignAfterOpenBracket: Align）

```cpp
// 函数调用参数与括号内第一个元素（int a 的 a）对齐
void func(int a,
          double b,
          std::string c);

// 模板参数与括号内第一个元素（int 的 i）对齐
std::vector<std::map<int,
                     std::string>> my_vec;
```

## 3. AlignConsecutiveAssignments: true（连续赋值对齐等号）

### 配置说明

多个连续赋值语句时，自动对齐所有 `=` 符号，提升可读性。

#### 不开启（false，默认样式，无对齐）

```cpp
int a = 10;
double b = 3.14159;
std::string c = "clang-format";
int d = 1000;
```

#### 开启（true，等号对齐）

```cpp
int a     = 10;
double b  = 3.14159;
std::string c = "clang-format";
int d     = 1000;
```

## 4. AlignConsecutiveDeclarations: true（连续声明对齐变量名）

### 配置说明

多个连续变量声明时，自动对齐所有变量名，而非仅按类型缩进。

#### 不开启（false，默认样式，无对齐）

```cpp
int a = 10;
int b = 20;
double c = 3.14;
double d = 6.28;
std::string e = "test";
```

#### 开启（true，变量名对齐）

```cpp
int         a = 10;
int         b = 20;
double      c = 3.14;
double      d = 6.28;
std::string e = "test";
```

## 5. AlignEscapedNewlinesLeft: true（左对齐反斜杠换行）

### 配置说明

使用反斜杠 `\` 进行换行（逃脱换行）时，反斜杠向左对齐（与行首/声明开头对齐）。

#### 不开启（false，默认右对齐反斜杠）

```cpp
// 反斜杠靠近代码，无左对齐
#define LONG_MACRO(x, y) \
    (x + y) * (x - y) \
    + 2 * x * y

std::string str = "this is a long string " \
                  "split by backslash " \
                  "in cpp";
```

#### 开启（true，反斜杠左对齐）

```cpp
// 反斜杠统一左对齐，视觉更整齐
#define LONG_MACRO(x, y) \
    (x + y) * (x - y) \
    + 2 * x * y

std::string str = "this is a long string " \
                  "split by backslash " \
                  "in cpp";
// 注：对于声明语句，反斜杠会与变量名/类型开头对齐
int num = 100 + 200 \
          + 300 + 400 \
          - 50;
```

## 6. AlignOperands: true（对齐二元/三元表达式操作数）

### 配置说明

二元运算符（`+`、`-`、`*`、`==` 等）、三元运算符（`?:`）的操作数自动水平对齐。

#### 不开启（false，默认样式，无对齐）

```cpp
// 二元表达式操作数无对齐
int result = a + b
    - c * d
    + e / f;

// 三元表达式无对齐
int max_val = (a > b) ? a
    : b;
```

#### 开启（true，操作数对齐）

```cpp
// 二元表达式操作数与第一个操作数对齐
int result = a + b
            - c * d
            + e / f;

// 三元表达式操作数对齐
int max_val = (a > b) ? a
                      : b;
```

## 7. AlignTrailingComments: true（对齐连续尾随注释）

### 配置说明

多个连续代码行的尾随注释（行尾 `//` 注释）自动对齐，统一注释起始位置。

#### 不开启（false，默认样式，无对齐）

```cpp
int a = 10; // 整数a
double b = 3.14; // 浮点b
std::string c = "test"; // 字符串c
```

#### 开启（true，尾随注释对齐）

```cpp
int a = 10;        // 整数a
double b = 3.14;   // 浮点b
std::string c = "test"; // 字符串c
```

## 8. AllowAllParametersOfDeclarationOnNextLine: true（允许函数声明参数全放下行）

### 配置说明

函数声明时，允许所有参数不跟在函数名后，统一放在下一行。

#### 不开启（false，默认限制，部分参数可能强制分行）

```cpp
// 无法将所有参数统一放在下行，只能部分拆分
void func(int a, double b, std::string c, bool d);
// 若参数过长，会强制拆分，但非全部统一下行
void func2(int a,
    double b,
    std::string c,
    bool d);
```

#### 开启（true，允许所有参数统一放下行）

```cpp
// 支持所有参数统一放在函数名下行，格式更整洁
void func(
    int a,
    double b,
    std::string c,
    bool d);
```

## 9. AllowShortBlocksOnASingleLine: false（禁止短块单行显示）

### 配置说明

禁止 `if`、`for`、`while` 等语句的短代码块（`{}` 包裹）放在同一行。

#### 不开启（true，允许短块单行显示）

```cpp
// 短if块单行显示
if (a > b) { std::swap(a, b); }

// 短for块单行显示
for (int i = 0; i < 10; i++) { sum += i; }

// 短空块单行显示
while (false) {}
```

#### 开启（false，禁止单行，强制分行）

```cpp
// 短if块强制分行
if (a > b) {
    std::swap(a, b);
}

// 短for块强制分行
for (int i = 0; i < 10; i++) {
    sum += i;
}

// 短空块强制分行
while (false) {
}
```

## 10. AllowShortCaseLabelsOnASingleLine: false（禁止短case标签单行显示）

### 配置说明

禁止 `switch` 语句中的短 `case` 标签（含语句）放在同一行。

#### 不开启（true，允许短case单行显示）

```cpp
switch (cmd) {
    case 0: return "exit";
    case 1: std::cout << "run" << std::endl; break;
    case 2: break;
    default: return "unknown";
}
```

#### 开启（false，禁止单行，强制分行）

```cpp
switch (cmd) {
    case 0:
        return "exit";
    case 1:
        std::cout << "run" << std::endl;
        break;
    case 2:
        break;
    default:
        return "unknown";
}
```

## 11. AllowShortFunctionsOnASingleLine: Empty（仅允许空函数单行显示）

### 配置说明

仅空函数（无函数体内容）可以放在同一行，非空函数（即使很短）也强制分行。

#### 不开启（其他选项，如 All，允许所有短函数单行）

```cpp
// 内联函数单行显示
class MyClass {
public:
    void func1() { std::cout << "test" << std::endl; }
    int func2() { return 10; }
    void func3() {} // 空函数单行
};

// 全局函数单行显示
void empty_func() {}
void short_func() { sum = 0; }
```

#### 开启（Empty，仅空函数单行）

```cpp
class MyClass {
public:
    // 非空函数强制分行
    void func1() {
        std::cout << "test" << std::endl;
    }
    int func2() {
        return 10;
    }
    // 仅空函数允许单行
    void func3() {}
};

// 全局函数同理
void empty_func() {}
void short_func() {
    sum = 0;
}
```

## 12. AllowShortIfStatementsOnASingleLine: false（禁止短if语句单行显示）

### 配置说明

禁止短 `if` 语句（无 `else` 或含 `else`）放在同一行，强制分行显示。

#### 不开启（true，允许短if单行显示）

```cpp
// 无else的短if单行
if (a == 0) return false;
if (b > 10) std::cout << "b is large" << std::endl;

// 含else的短if单行
if (c % 2 == 0) is_even = true; else is_even = false;
```

#### 开启（false，禁止单行，强制分行）

```cpp
// 无else的短if强制分行
if (a == 0) {
    return false;
}
if (b > 10) {
    std::cout << "b is large" << std::endl;
}

// 含else的短if强制分行
if (c % 2 == 0) {
    is_even = true;
} else {
    is_even = false;
}
```

## 13. AllowShortLoopsOnASingleLine: false（禁止短循环单行显示）

### 配置说明

控制 `for`、`while`（不含 `do-while`）等短循环语句是否允许将循环体与循环头放在同一行，`false` 表示禁止单行，强制分行展示；`true` 表示允许单行展示短循环。

#### 不开启（true，允许短循环单行显示，默认部分场景生效）

```cpp
// for循环（短循环体）单行展示
for (int i = 0; i < 5; i++) { sum += i; }
// while循环（短循环体）单行展示
while (ptr != nullptr) { ptr = ptr->next; }
// 空循环单行展示
for (int j = 0; j < 10; j++) {}
```

#### 开启（false，禁止单行，强制分行显示）

```cpp
// for循环强制分行
for (int i = 0; i < 5; i++) {
    sum += i;
}
// while循环强制分行
while (ptr != nullptr) {
    ptr = ptr->next;
}
// 空循环也强制分行
for (int j = 0; j < 10; j++) {
}
```

## 14. AlwaysBreakAfterReturnType: None（返回类型后不强制换行）

### 配置说明

控制函数的返回类型之后是否强制换行，可选值为 `None`/`All`/`TopLevel` 等，`None` 表示不强制换行（返回类型与函数名同行）；其他值（如 `All`）表示强制在返回类型后换行，函数名另起一行。

> 补充：`AlwaysBreakAfterDefinitionReturnType` 是该配置的废弃版本，行为与 `AlwaysBreakAfterReturnType` 一致，`None` 均表示不换行。

#### 不开启（如 All，强制返回类型后换行）

```cpp
// 顶级全局函数：返回类型后换行
int
add(int a, int b) {
    return a + b;
}

// 类内成员函数：返回类型后换行
class MyClass {
public:
    std::string
    get_name() {
        return m_name;
    }
private:
    std::string m_name;
};

// 函数声明：返回类型后换行
double
calculate_area(double r);
```

#### 开启（None，返回类型与函数名同行，默认样式）

```cpp
// 顶级全局函数：返回类型与函数名同行
int add(int a, int b) {
    return a + b;
}

// 类内成员函数：返回类型与函数名同行
class MyClass {
public:
    std::string get_name() {
        return m_name;
    }
private:
    std::string m_name;
};

// 函数声明：返回类型与函数名同行
double calculate_area(double r);
```

## 15. AlwaysBreakBeforeMultilineStrings: false（多行字符串前不强制换行）

### 配置说明

控制在多行字符串字面量（C++中通常是拼接的字符串或原始字符串）之前是否强制换行，`false` 表示不强制换行（字符串与前置代码/变量同行）；`true` 表示强制在多行字符串前换行，字符串另起一行。

#### 不开启（true，强制多行字符串前换行）

```cpp
// 变量赋值：多行字符串前强制换行
std::string long_str =
    "this is a multiline string "
    "split into multiple lines "
    "without backslash";

// 函数参数：多行字符串前强制换行
void print_info() {
    std::cout <<
        "user name: test" << std::endl <<
        "user age: 20" << std::endl;
}

// 原始字符串字面量前强制换行
std::string raw_str =
    R"(
    this is a raw string
    with multiple lines
)";
```

#### 开启（false，多行字符串前不换行，默认样式）

```cpp
// 变量赋值：多行字符串与变量同行
std::string long_str = "this is a multiline string "
                       "split into multiple lines "
                       "without backslash";

// 函数参数：多行字符串与cout同行
void print_info() {
    std::cout << "user name: test" << std::endl
              << "user age: 20" << std::endl;
}

// 原始字符串字面量与变量同行
std::string raw_str = R"(
    this is a raw string
    with multiple lines
)";
```

## 16. AlwaysBreakTemplateDeclarations: false（模板声明后不强制换行）

### 配置说明

控制 `template<>` 模板声明之后是否强制换行，`false` 表示不强制换行（模板声明与模板类/函数同行）；`true` 表示强制在模板声明后换行，模板类/函数名另起一行。

#### 不开启（true，强制模板声明后换行）

```cpp
// 模板类：template声明后换行
template <typename T, typename U>
class MyMap {
public:
    T key;
    U value;
};

// 模板函数：template声明后换行
template <typename T>
T max_val(T a, T b) {
    return a > b ? a : b;
}

// 模板特化：template声明后换行
template <>
int max_val<int>(int a, int b) {
    return a > b ? a : b;
}
```

#### 开启（false，模板声明与类/函数同行，默认样式）

```cpp
// 模板类：template声明与类名同行
template <typename T, typename U> class MyMap {
public:
    T key;
    U value;
};

// 模板函数：template声明与函数名同行
template <typename T> T max_val(T a, T b) {
    return a > b ? a : b;
}

// 模板特化：template声明与函数名同行
template <> int max_val<int>(int a, int b) {
    return a > b ? a : b;
}
```

## 17. BinPackArguments: true（函数实参按需分块打包，非全同行/全分行）

### 配置说明

控制函数调用时的实参排列方式：`true` 表示按需分块（尽可能多的实参放在同一行，超出长度再拆分，不强制全同行或全分行）；`false` 表示仅两种样式（所有实参全在同一行，或每个实参单独占一行）。

#### 不开启（false，仅全同行/全分行两种样式）

```cpp
// 样式1：所有实参全在同一行（长度较短时）
func(10, 3.14, "test", true);

// 样式2：每个实参单独占一行（长度超出阈值时，无中间状态）
func(
    10,
    3.1415926535,
    "this is a long string argument",
    true,
    100);
```

#### 开启（true，按需分块打包，灵活排列，默认样式）

```cpp
// 场景1：长度较短，全同行（与false一致）
func(10, 3.14, "test", true);

// 场景2：长度中等，分2块打包（部分同行，部分分行，false不支持此样式）
func(10, 3.1415926535, "this is a long string argument",
     true, 100);

// 场景3：长度极长，分多块打包（更灵活的拆分，非强制每个实参单行）
func(
    10, 3.1415926535,
    "this is a very very long string argument that exceeds line length",
    true, 100, 200);
```

## 18. BinPackParameters: true（函数形参按需分块打包，非全同行/全分行）

### 配置说明

控制函数声明/定义时的形参排列方式，与 `BinPackArguments` 逻辑一致：`true` 表示按需分块（尽可能多的形参放在同一行，超出长度再拆分，灵活排列）；`false` 表示仅两种样式（所有形参全在同一行，或每个形参单独占一行）。

#### 不开启（false，仅全同行/全分行两种样式）

```cpp
// 函数声明
// 样式1：所有形参全在同一行（长度较短时）
void func(int a, double b, std::string c, bool d);

// 样式2：每个形参单独占一行（长度超出阈值时，无中间状态）
void func2(
    int a,
    double b,
    std::string long_arg_name,
    bool d,
    int e);

// 函数定义（与声明逻辑一致）
void func2(
    int a,
    double b,
    std::string long_arg_name,
    bool d,
    int e) {
    // 函数体
}
```

#### 开启（true，按需分块打包，灵活排列，默认样式）

```cpp
// 函数声明
// 场景1：长度较短，全同行（与false一致）
void func(int a, double b, std::string c, bool d);

// 场景2：长度中等，分2块打包（部分同行，部分分行，false不支持此样式）
void func2(int a, double b, std::string long_arg_name,
           bool d, int e);

// 场景3：长度极长，分多块打包（更灵活的拆分）
void func3(
    int a, double b,
    std::string very_long_arg_name_that_exceeds_line_length,
    bool d, int e, int f);

// 函数定义（与声明逻辑一致，灵活分块）
void func2(int a, double b, std::string long_arg_name,
           bool d, int e) {
    // 函数体
}
```

## 19. AfterClass: false（类定义后不换行，大括号与类名同行）

### 配置说明

控制 `class` 定义后是否换行，`false` 表示类名后直接跟 `{`（不换行）；`true` 表示类名后换行，`{` 单独占一行。

#### 不开启（true，类定义后换行，大括号单独成行）

```cpp
// class定义后换行，{ 单独占一行
class MyClass
{
public:
    void func();
private:
    int m_value;
};

// 带继承的类同理
class ChildClass : public MyClass
{
public:
    void childFunc();
};
```

#### 开启（false，类定义后不换行，大括号与类名同行，当前配置）

```cpp
// class定义后不换行，{ 与类名同行
class MyClass {
public:
    void func();
private:
    int m_value;
};

// 带继承的类同理
class ChildClass : public MyClass {
public:
    void childFunc();
};
```

## 20. AfterControlStatement: false（控制语句后不换行，大括号与控制语句同行）

### 配置说明

控制 `if`、`for`、`while`、`switch` 等控制语句后是否换行，`false` 表示控制语句后直接跟 `{`（不换行）；`true` 表示控制语句后换行，`{` 单独占一行。

#### 不开启（true，控制语句后换行，大括号单独成行）

```cpp
// if语句后换行
if (a > b)
{
    std::swap(a, b);
}

// for循环后换行
for (int i = 0; i < 10; i++)
{
    sum += i;
}

// switch语句后换行
switch (cmd)
{
    case 1:
        break;
    default:
        break;
}
```

#### 开启（false，控制语句后不换行，大括号与控制语句同行，当前配置）

```cpp
// if语句后不换行，{ 与条件判断同行
if (a > b) {
    std::swap(a, b);
}

// for循环后不换行，{ 与循环头同行
for (int i = 0; i < 10; i++) {
    sum += i;
}

// switch语句后不换行，{ 与switch同行
switch (cmd) {
    case 1:
        break;
    default:
        break;
}
```

## 21. AfterEnum: false（枚举定义后不换行，大括号与枚举名同行）

### 配置说明

控制 `enum`（普通枚举/强类型枚举）定义后是否换行，`false` 表示枚举名后直接跟 `{`（不换行）；`true` 表示枚举名后换行，`{` 单独占一行。

#### 不开启（true，枚举定义后换行，大括号单独成行）

```cpp
// 普通枚举
enum Color
{
    Red,
    Green,
    Blue
};

// 强类型枚举
enum class Direction
{
    Left,
    Right,
    Up,
    Down
};
```

#### 开启（false，枚举定义后不换行，大括号与枚举名同行，当前配置）

```cpp
// 普通枚举
enum Color {
    Red,
    Green,
    Blue
};

// 强类型枚举
enum class Direction {
    Left,
    Right,
    Up,
    Down
};
```

## 22. AfterFunction: false（函数定义后不换行，大括号与函数名同行）

### 配置说明

控制函数（全局函数/类成员函数/Lambda 表达式）定义后是否换行，`false` 表示函数头后直接跟 `{`（不换行）；`true` 表示函数头后换行，`{` 单独占一行。

#### 不开启（true，函数定义后换行，大括号单独成行）

```cpp
// 全局函数
int add(int a, int b)
{
    return a + b;
}

// 类成员函数
class MyClass {
public:
    void func()
    {
        std::cout << "test" << std::endl;
    }
};

// Lambda表达式
auto lambda = [](int x)
{
    return x * 2;
};
```

#### 开启（false，函数定义后不换行，大括号与函数头同行，当前配置）

```cpp
// 全局函数
int add(int a, int b) {
    return a + b;
}

// 类成员函数
class MyClass {
public:
    void func() {
        std::cout << "test" << std::endl;
    }
};

// Lambda表达式
auto lambda = [](int x) {
    return x * 2;
};
```

## 23. AfterNamespace: false（命名空间定义后不换行，大括号与命名空间名同行）

### 配置说明

控制 `namespace` 定义后是否换行，`false` 表示命名空间名后直接跟 `{`（不换行）；`true` 表示命名空间名后换行，`{` 单独占一行。

#### 不开启（true，命名空间定义后换行，大括号单独成行）

```cpp
// 普通命名空间
namespace MyNamespace
{
    int value = 10;
    void func();
}

// 嵌套命名空间
namespace OuterNamespace
{
    namespace InnerNamespace
    {
        double pi = 3.14;
    }
}
```

#### 开启（false，命名空间定义后不换行，大括号与命名空间名同行，当前配置）

```cpp
// 普通命名空间
namespace MyNamespace {
    int value = 10;
    void func();
}

// 嵌套命名空间
namespace OuterNamespace {
    namespace InnerNamespace {
        double pi = 3.14;
    }
}
```

## 24. AfterObjCDeclaration: false（ObjC声明后不换行，大括号与声明同行）

### 配置说明

控制 Objective-C 相关声明（如 `@interface`、`@implementation`）后是否换行，`false` 表示声明后直接跟 `{`（不换行）；`true` 表示声明后换行，`{` 单独占一行（仅对 ObjC 代码有效）。

#### 不开启（true，ObjC声明后换行，大括号单独成行）

```objc
// ObjC接口声明
@interface MyObjCClass
{
    NSInteger _count;
}
- (void)objcFunc;
@end

// ObjC实现声明
@implementation MyObjCClass
- (void)objcFunc
{
    NSLog(@"ObjC Test");
}
@end
```

#### 开启（false，ObjC声明后不换行，大括号与声明同行，当前配置）

```objc
// ObjC接口声明
@interface MyObjCClass {
    NSInteger _count;
}
- (void)objcFunc;
@end

// ObjC实现声明
@implementation MyObjCClass
- (void)objcFunc {
    NSLog(@"ObjC Test");
}
@end
```

## 25. AfterStruct: false（结构体定义后不换行，大括号与结构体名同行）

### 配置说明

控制 `struct` 定义后是否换行，`false` 表示结构体名后直接跟 `{`（不换行）；`true` 表示结构体名后换行，`{` 单独占一行。

#### 不开启（true，结构体定义后换行，大括号单独成行）

```cpp
// 普通结构体
struct Person
{
    std::string name;
    int age;
};

// 带成员函数的结构体
struct Data
{
    int value;
    void setValue(int v)
    {
        value = v;
    }
};
```

#### 开启（false，结构体定义后不换行，大括号与结构体名同行，当前配置）

```cpp
// 普通结构体
struct Person {
    std::string name;
    int age;
};

// 带成员函数的结构体
struct Data {
    int value;
    void setValue(int v) {
        value = v;
    }
};
```

## 26. AfterUnion: false（联合体定义后不换行，大括号与联合体名同行）

### 配置说明

控制 `union` 定义后是否换行，`false` 表示联合体名后直接跟 `{`（不换行）；`true` 表示联合体名后换行，`{` 单独占一行。

#### 不开启（true，联合体定义后换行，大括号单独成行）

```cpp
// 普通联合体
union MyUnion
{
    int int_val;
    double double_val;
    char char_val;
};

// 带命名的联合体
union DataUnion
{
    struct {
        int a;
        int b;
    } pair;
    long long val;
};
```

#### 开启（false，联合体定义后不换行，大括号与联合体名同行，当前配置）

```cpp
// 普通联合体
union MyUnion {
    int int_val;
    double double_val;
    char char_val;
};

// 带命名的联合体
union DataUnion {
    struct {
        int a;
        int b;
    } pair;
    long long val;
};
```

## 27. BeforeCatch: true（catch 之前换行，catch 单独占一行）

### 配置说明

控制 `try-catch` 语句中 `catch` 关键字前是否换行，`true` 表示 `catch` 前换行（与 `try` 块的 `}` 分行）；`false` 表示 `catch` 与 `try` 块的 `}` 同行，不换行。

#### 不开启（false，catch 与 try 块的 } 同行，不换行）

```cpp
// catch 紧跟在 try 块的 } 之后，无换行
try {
    int a = 10 / 0;
} catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}

// 多catch场景
try {
    // 业务逻辑
} catch (const std::runtime_error& e) {
    std::cout << "runtime error: " << e.what() << std::endl;
} catch (const std::exception& e) {
    std::cout << "exception: " << e.what() << std::endl;
}
```

#### 开启（true，catch 之前换行，catch 单独成行，当前配置）

```cpp
// catch 与 try 块的 } 分行，单独占一行
try {
    int a = 10 / 0;
}
catch (const std::exception& e) {
    std::cout << e.what() << std::endl;
}

// 多catch场景，每个catch前都换行
try {
    // 业务逻辑
}
catch (const std::runtime_error& e) {
    std::cout << "runtime error: " << e.what() << std::endl;
}
catch (const std::exception& e) {
    std::cout << "exception: " << e.what() << std::endl;
}
```

## 29. BeforeElse: true（else 之前换行，else 单独占一行）

### 配置说明

控制 `if-else` 语句中 `else` 关键字前是否换行，`true` 表示 `else` 前换行（与 `if` 块的 `}` 分行）；`false` 表示 `else` 与 `if` 块的 `}` 同行，不换行（含 `else if` 场景）。

#### 不开启（false，else 与 if 块的 } 同行，不换行）

```cpp
// if-else 场景：else 紧跟 } 之后
if (a % 2 == 0) {
    std::cout << "a is even" << std::endl;
} else {
    std::cout << "a is odd" << std::endl;
}

// if-else if-else 场景：所有 else/else if 均紧跟 } 之后
if (a > 100) {
    std::cout << "a is large" << std::endl;
} else if (a > 50) {
    std::cout << "a is medium" << std::endl;
} else {
    std::cout << "a is small" << std::endl;
}
```

#### 开启（true，else 之前换行，else 单独成行，当前配置）

```cpp
// if-else 场景：else 与 if 块的 } 分行
if (a % 2 == 0) {
    std::cout << "a is even" << std::endl;
}
else {
    std::cout << "a is odd" << std::endl;
}

// if-else if-else 场景：每个 else if/else 前都换行
if (a > 100) {
    std::cout << "a is large" << std::endl;
}
else if (a > 50) {
    std::cout << "a is medium" << std::endl;
}
else {
    std::cout << "a is small" << std::endl;
}
```

## 30. IndentBraces: false（不缩进大括号，大括号与外层代码对齐）

### 配置说明

控制大括号 `{}` 自身是否缩进，`false` 表示大括号不缩进（与外层代码/声明对齐）；`true` 表示大括号跟随内层代码缩进（自身占一个缩进级别）。

#### 不开启（true，缩进大括号，大括号自身占一个缩进级别）

```cpp
// 类：大括号缩进（比 class 多一级）
class MyClass {
    public:
        void func() {
            // 函数体大括号再缩进一级
            std::cout << "test" << std::endl;
        }
    private:
        int m_value;
    };

// if 语句：大括号缩进（比 if 多一级）
if (a > b) {
    std::swap(a, b);
    }

// 命名空间：大括号缩进（比 namespace 多一级）
namespace MyNamespace {
    int value = 10;
    }
```

#### 开启（false，不缩进大括号，大括号与外层对齐，当前配置）

```cpp
// 类：大括号与 class 对齐，无额外缩进
class MyClass {
public:
    void func() {
        // 函数体代码缩进，大括号与 func 对齐
        std::cout << "test" << std::endl;
    }
private:
    int m_value;
};

// if 语句：大括号与 if 对齐，无额外缩进
if (a > b) {
    std::swap(a, b);
}

// 命名空间：大括号与 namespace 对齐，无额外缩进
namespace MyNamespace {
    int value = 10;
}
```

## 31. BreakBeforeBinaryOperators: NonAssignment（非赋值二元运算符前换行，赋值运算符后换行）

### 配置说明

控制二元运算符（`+`、`-`、`*`、`=`、`+=` 等）的换行位置，可选值：

- `NonAssignment`（当前配置）：**非赋值运算符**（`+`、`-`、`*`、`==` 等）前换行；**赋值运算符**（`=`、`+=`、`-=` 等）后换行（保持赋值语句紧凑）
- `None`（默认）：所有二元运算符**后**换行
- `All`：所有二元运算符**前**换行

#### 不开启1（None，所有二元运算符后换行，默认样式）

```cpp
// 非赋值运算符（+、-）后换行
int result = a + b + c
             - d * e
             + f / g;

// 赋值运算符（=、+=）后换行（与NonAssignment一致）
int x = 10 + 20
        + 30;
x += 50
    + 40;

// 比较运算符（==）后换行
bool is_valid = (a == b)
                && (c > d)
                || (e < f);
```

#### 不开启2（All，所有二元运算符前换行）

```cpp
// 非赋值运算符（+、-）前换行（与NonAssignment一致）
int result = a
             + b
             + c
             - d * e
             + f / g;

// 赋值运算符（=、+=）也前换行（与NonAssignment不一致）
int x
    = 10
      + 20
      + 30;
x
    += 50
       + 40;

// 比较运算符（==）前换行（与NonAssignment一致）
bool is_valid
    = (a == b)
      && (c > d)
      || (e < f);
```

#### 开启（NonAssignment，当前配置，仅非赋值运算符前换行）

```cpp
// 非赋值运算符（+、-、==、&&）前换行（拆分清晰，符合阅读习惯）
int result = a
             + b
             + c
             - d * e
             + f / g;

// 比较运算符前换行
bool is_valid = (a == b)
                && (c > d)
                || (e < f);

// 赋值运算符（=、+=）后换行（保持紧凑，避免赋值语句过于分散）
int x = 10 + 20
        + 30;
x += 50
    + 40;
```

## 31. BreakBeforeBraces: Custom（自定义大括号换行规则，依赖BraceWrapping配置）

### 配置说明

控制大括号 `{` 前是否换行及换行规则，`Custom` 表示启用自定义规则（完全依赖 `BraceWrapping` 子配置，无默认行为）；其他取值为预设规则（无需手动配置 `BraceWrapping`），核心预设值对比：

- `Attach`（默认）：大括号始终附加到上下文后（无换行，如 `if (a>b) {`）
- `Allman`：所有大括号前都换行（大括号单独占一行，如 `if (a>b)\n{`）
- `Custom`（当前配置）：仅遵循 `BraceWrapping` 配置，无自身默认行为

#### 不开启1（Attach，默认样式，大括号始终附加到上下文）

```cpp
// 控制语句：大括号紧跟上下文，无换行
if (a > b) {
    std::swap(a, b);
}
for (int i = 0; i < 10; i++) {
    sum += i;
}

// 类/函数/命名空间：大括号紧跟上下文，无换行
class MyClass {
public:
    void func() {
        // 函数体大括号也附加到上下文
    }
};

namespace MyNamespace {
    int value = 10;
}
```

#### 不开启2（Allman，所有大括号前换行，大括号单独成行）

```cpp
// 控制语句：大括号前换行，单独占一行
if (a > b)
{
    std::swap(a, b);
}
for (int i = 0; i < 10; i++)
{
    sum += i;
}

// 类/函数/命名空间：大括号前换行，单独占一行
class MyClass
{
public:
    void func()
    {
        // 函数体大括号也单独成行
    }
};

namespace MyNamespace
{
    int value = 10;
}
```

#### 开启（Custom，当前配置，依赖BraceWrapping，示例沿用之前的BraceWrapping配置）

```cpp
// 遵循BraceWrapping配置：AfterXXX=false（大括号附加上下文），BeforeCatch/BeforeElse=true（catch/else前换行）
class MyClass { // 无换行，附加类名
public:
    void func() { // 无换行，附加函数头
        std::cout << "test" << std::endl;
    }
};

if (a > b) { // 无换行，附加if语句
    sum += a;
}
else { // else前换行（遵循BraceWrapping::BeforeElse=true）
    sum += b;
}

try {
    int a = 10 / 0;
}
catch (const std::exception& e) { // catch前换行（遵循BraceWrapping::BeforeCatch=true）
    std::cout << e.what() << std::endl;
}

namespace MyNamespace { // 无换行，附加命名空间名
    int value = 10;
}
```

## 32. BreakBeforeTernaryOperators: true（三元运算符 `?:` 前换行）

### 配置说明

控制三元运算符（`condition ? val1 : val2`）的换行位置，`true` 表示在 `?` 和 `:` 前换行；`false` 表示不强制换行（或在运算符后换行），保持三元表达式紧凑。

#### 不开启（false，三元运算符不换行，保持紧凑样式）

```cpp
// 短三元表达式：单行显示
int max_val = (a > b) ? a : b;

// 长三元表达式：仅在运算符后换行（无强制前换行）
int result = (a > 100 && b < 200 && c != 300)
             ? (a + b + c)
             : (a - b - c);

// 嵌套三元表达式：紧凑排列，无强制前换行
int nested_val = (a % 2 == 0) ? (a / 2) : (a % 3 == 0) ? (a / 3) : a;
```

#### 开启（true，当前配置，三元运算符前换行，拆分清晰）

```cpp
// 短三元表达式：强制在?和:前换行
int max_val = (a > b)
              ? a
              : b;

// 长三元表达式：在?和:前换行，层级更清晰
int result = (a > 100 && b < 200 && c != 300)
              ? (a + b + c)
              : (a - b - c);

// 嵌套三元表达式：每层?和:前都换行，可读性更强
int nested_val = (a % 2 == 0)
                  ? (a / 2)
                  : (a % 3 == 0)
                    ? (a / 3)
                    : a;
```

## 33. BreakConstructorInitializersBeforeComma: false（构造函数初始化列表，逗号后换行，不前置换行）

### 配置说明

控制类构造函数初始化列表的逗号换行位置，`false` 表示在**逗号后**换行（初始化项跟随逗号换行）；`true` 表示在**逗号前**换行（逗号单独占一行，初始化项另起一行）。

#### 不开启（true，逗号前换行，逗号单独占一行）

```cpp
class MyClass {
public:
    // 初始化列表：逗号前换行，逗号单独成行
    MyClass(int a, double b, std::string c)
        : m_a(a)
        , m_b(b)
        , m_c(c)
        , m_d(100) {
        // 构造函数体
    }

private:
    int m_a;
    double m_b;
    std::string m_c;
    int m_d;
};

// 带注释的初始化列表
class ChildClass : public MyClass {
public:
    ChildClass(int a)
        : MyClass(a, 3.14, "test")
        , m_e(0) { // 逗号前换行
    }

private:
    int m_e;
};
```

#### 开启（false，当前配置，逗号后换行，初始化项换行）

```cpp
class MyClass {
public:
    // 初始化列表：逗号后换行，紧凑且清晰
    MyClass(int a, double b, std::string c)
        : m_a(a),
          m_b(b),
          m_c(c),
          m_d(100) {
        // 构造函数体
    }

private:
    int m_a;
    double m_b;
    std::string m_c;
    int m_d;
};

// 带注释的初始化列表
class ChildClass : public MyClass {
public:
    ChildClass(int a)
        : MyClass(a, 3.14, "test"),
          m_e(0) { // 逗号后换行
    }

private:
    int m_e;
};
```

## 34. ColumnLimit: 200（每行字符限制为200，超出则自动换行）

### 配置说明

控制代码每行的最大字符数，`0` 表示无限制（任意长行都不换行）；`200`（当前配置）表示当行字符数超过200时，Clang-Format自动拆分换行（对齐规则遵循其他配置）。

#### 不开启（0，无字符数限制，不自动换行）

```cpp
// 超长函数声明：单行显示，无自动换行（即使字符数远超200）
void very_long_function_name(int a, double b, std::string c, bool d, int e, double f, std::string g, bool h, int i, double j) {
    // 超长表达式：单行显示，无拆分
    int very_long_result = a + b * 100 + c.length() - (d ? e : f) + g.find("test") * h + i / j;
}

// 超长字符串拼接：单行显示
std::string very_long_str = "this is a very long string that exceeds 200 characters, but with ColumnLimit 0, it will not be split into multiple lines, even if it is too long to read at once.";
```

#### 开启（200，当前配置，超出200字符自动换行）

```cpp
// 超长函数声明：超出200字符后，自动按BinPackArguments规则拆分换行
void very_long_function_name(int a, double b, std::string c, bool d, int e, double f,
                             std::string g, bool h, int i, double j) {
    // 超长表达式：超出200字符后，按BreakBeforeBinaryOperators规则拆分换行
    int very_long_result = a + b * 100 + c.length()
                           - (d ? e : f)
                           + g.find("test") * h
                           + i / j;
}

// 超长字符串拼接：超出200字符后，自动拆分换行（对齐字符串）
std::string very_long_str = "this is a very long string that exceeds 200 characters, but with "
                           "ColumnLimit 200, it will be split into multiple lines automatically "
                           "to ensure that each line does not exceed 200 characters.";
```

## 36. CommentPragmas: '^ IWYU pragma:'（保护指定正则表达式的注释不被修改）

### 配置说明

指定特殊注释的正则表达式，匹配该正则的注释不会被Clang-Format拆分、缩进修改或格式调整，保持原始样式；若不配置该参数，所有注释都会被格式化（如拆分多行、调整缩进）。

#### 不开启（未配置该参数，所有注释都会被格式化）

```cpp
// 原始IWYU注释：单行超长注释会被拆分多行
// IWYU pragma: begin_exports
// #include <vector>
// #include <string>
// #include <iostream>
// IWYU pragma: end_exports

void func() {
    // 缩进被自动调整（与代码对齐）
    //  IWYU pragma: no_include <unnecessary.h>
    int a = 10;
}

// 多行IWYU注释会被调整缩进
// IWYU pragma: private, include "my_header.h"
//  IWYU pragma: friend "*.cpp"
```

#### 开启（当前配置，匹配 `^ IWYU pragma:` 的注释不被修改）

```cpp
// 原始IWYU注释：超长单行不被拆分，保持原始样式
// IWYU pragma: begin_exports #include <vector> #include <string> #include <iostream> IWYU pragma: end_exports

void func() {
    // 缩进不被调整，保持原始空格和格式
    //  IWYU pragma: no_include <unnecessary.h>
    int a = 10;
}

// 多行IWYU注释：保持原始缩进和排版，不被格式化
// IWYU pragma: private, include "my_header.h"
//  IWYU pragma: friend "*.cpp"
```

## 36. ConstructorInitializerAllOnOneLineOrOnePerLine: false（构造函数初始化列表灵活排列，非仅全同行/全分行）

### 配置说明

控制类构造函数初始化列表的排列规则：

- `false`（当前配置）：灵活排列（可部分参数同行、部分分行，按需拆分，非强制两种极端样式）
- `true`（不开启/默认）：仅两种样式（所有初始化项全在同一行，或每个初始化项单独占一行，无中间状态）

#### 不开启（true，仅全同行/全分行两种样式）

```cpp
class MyClass {
public:
    // 样式1：所有初始化项全在同一行（长度较短时）
    MyClass(int a, double b) : m_a(a), m_b(b), m_c(100) {
    }

    // 样式2：每个初始化项单独占一行（长度超出阈值时，无中间状态）
    MyClass(int a, double b, std::string c, bool d)
        : m_a(a)
        , m_b(b)
        , m_c_str(c)
        , m_d(d) {
    }

private:
    int m_a;
    double m_b;
    int m_c;
    std::string m_c_str;
    bool m_d;
};
```

#### 开启（false，当前配置，灵活排列，支持部分同行+部分分行）

```cpp
class MyClass {
public:
    // 场景1：长度较短，全同行（与true一致）
    MyClass(int a, double b) : m_a(a), m_b(b), m_c(100) {
    }

    // 场景2：长度中等，部分同行+部分分行（灵活拆分，true不支持此样式）
    MyClass(int a, double b, std::string c, bool d)
        : m_a(a), m_b(b),
          m_c_str(c), m_d(d) {
    }

    // 场景3：长度较长，按需分块分行（更紧凑，非强制每个项单行）
    MyClass(int a, double b, std::string c, bool d, long e)
        : m_a(a), m_b(b), m_c_str(c),
          m_d(d), m_e(e) {
    }

private:
    int m_a;
    double m_b;
    int m_c;
    std::string m_c_str;
    bool m_d;
    long m_e;
};
```

## 37. ConstructorInitializerIndentWidth: 4（构造函数初始化列表缩进宽度为4个空格）

### 配置说明

控制构造函数初始化列表（`:` 后的初始化项）相对于构造函数头的缩进宽度，核心对比缩进差异：

- `4`（当前配置）：初始化项缩进4个空格
- 非4（如2，不开启/自定义其他值）：初始化项缩进2个空格（默认常为2，可自定义其他数值）

#### 不开启（缩进宽度2，非4）

```cpp
class MyClass {
public:
    // 初始化项仅缩进2个空格（视觉上更紧凑，层级略模糊）
    MyClass(int a, double b)
      : m_a(a),  // 缩进2个空格
        m_b(b) { // 缩进2个空格
    }

    // 多行初始化列表同理（缩进2个空格）
    MyClass(int a, double b, std::string c)
      : m_a(a),
        m_b(b),
        m_c(c) {
    }

private:
    int m_a;
    double m_b;
    std::string m_c;
};
```

#### 开启（4，当前配置，初始化项缩进4个空格）

```cpp
class MyClass {
public:
    // 初始化项缩进4个空格（视觉层级清晰，与函数体内代码缩进一致）
    MyClass(int a, double b)
        : m_a(a),  // 缩进4个空格
          m_b(b) { // 缩进4个空格
    }

    // 多行初始化列表同理（缩进4个空格，对齐更整齐）
    MyClass(int a, double b, std::string c)
        : m_a(a),
          m_b(b),
          m_c(c) {
    }

private:
    int m_a;
    double m_b;
    std::string m_c;
};
```

## 38. ContinuationIndentWidth: 4（延续行缩进宽度为4个空格）

### 配置说明

控制所有「延续行」（因长度超限或语法要求换行的后续行，如函数参数、表达式拆分、字符串拼接等）的缩进宽度：

- `4`（当前配置）：延续行缩进4个空格
- 非4（如2，不开启/默认）：延续行缩进2个空格

#### 不开启（缩进宽度2，非4）

```cpp
// 1. 函数参数延续行（缩进2个空格）
void long_function_name(int a, double b,
  std::string c, bool d) { // 延续行缩进2个空格
}

// 2. 表达式拆分延续行（缩进2个空格）
int result = a + b + c
  - d * e
  + f / g;

// 3. 字符串拼接延续行（缩进2个空格）
std::string long_str = "this is a long string "
  "split into multiple lines "
  "with continuation indent 2";
```

#### 开启（4，当前配置，延续行缩进4个空格）

```cpp
// 1. 函数参数延续行（缩进4个空格，层级更清晰）
void long_function_name(int a, double b,
    std::string c, bool d) { // 延续行缩进4个空格
}

// 2. 表达式拆分延续行（缩进4个空格，对齐更整齐）
int result = a + b + c
            - d * e
            + f / g;

// 3. 字符串拼接延续行（缩进4个空格，可读性更强）
std::string long_str = "this is a long string "
                       "split into multiple lines "
                       "with continuation indent 4";
```

## 40. Cpp11BracedListStyle: false（保留C++11列表初始化大括号前后的空格，不启用紧凑样式）

### 配置说明

控制C++11列表初始化（`{}`）的空格样式：

- `false`（当前配置）：保留 `{` 后、`}` 前的空格，不启用Cpp11紧凑样式
- `true`（不开启/默认）：去除 `{` 后、`}` 前的空格，启用Cpp11专属紧凑列表样式

#### 不开启（true，去除大括号前后空格，紧凑样式）

```cpp
// 1. 变量列表初始化（无空格）
std::vector<int> vec = {1, 2, 3, 4}; // {后、}前无额外空格
std::map<int, std::string> mp = {{1, "a"}, {2, "b"}}; // 嵌套初始化也无空格

// 2. 函数参数列表初始化（无空格）
void func(std::vector<int> v) {}
func({10, 20, 30}); // 大括号前后无空格

// 3. 返回值列表初始化（无空格）
std::pair<int, std::string> get_pair() {
    return {1, "test"}; // 无空格
}

// 4. 类成员列表初始化（无空格）
class MyClass {
private:
    std::vector<int> m_vec = {1, 2, 3}; // 无空格
};
```

#### 开启（false，当前配置，保留大括号前后空格，传统样式）

```cpp
// 1. 变量列表初始化（保留空格）
std::vector<int> vec = { 1, 2, 3, 4 }; // {后、}前各有1个空格
std::map<int, std::string> mp = { { 1, "a" }, { 2, "b" } }; // 嵌套初始化也保留空格

// 2. 函数参数列表初始化（保留空格）
void func(std::vector<int> v) {}
func({ 10, 20, 30 }); // 大括号前后保留空格

// 3. 返回值列表初始化（保留空格）
std::pair<int, std::string> get_pair() {
    return { 1, "test" }; // 保留空格
}

// 4. 类成员列表初始化（保留空格）
class MyClass {
private:
    std::vector<int> m_vec = { 1, 2, 3 }; // 保留空格
};
```

## 41. DerivePointerAlignment: false（不继承指针/引用对齐方式，使用显式配置）

### 配置说明

控制指针（`*`）、引用（`&`）的对齐规则：

- `false`（当前配置）：不自动继承代码中最常用的对齐方式，严格遵循显式配置（如指针与类型对齐/与变量名对齐，需手动指定）
- `true`（不开启/默认）：自动检测代码中已有指针/引用的对齐方式，继承最常用的样式进行格式化

#### 前置说明

假设显式配置 `PointerAlignment: Left`（指针与类型对齐，如 `int* a;`），对比两种配置的差异：

#### 不开启（true，自动继承最常用对齐方式）

```cpp
// 原始代码中存在两种对齐方式：
int* a = nullptr;    // 指针与类型对齐（Left）
double b* = nullptr; // 指针与变量名对齐（Right，少数样式）
std::string& c = str; // 引用与类型对齐（Left）

// 格式化后：自动继承最常用的Left样式，统一格式化
int* a = nullptr;
double* b = nullptr; // 自动修正为Left样式（继承最常用）
std::string& c = str;
int* d = nullptr;   // 新声明也沿用继承的Left样式
```

#### 开启（false，当前配置，不继承，严格遵循显式配置）

```cpp
// 显式配置 PointerAlignment: Left
// 格式化后：严格按Left样式格式化，不检测/继承原始代码的常用样式
int* a = nullptr;    // 符合Left样式，不修改
double* b = nullptr; // 强制修正为Left样式（不继承原始少数Right样式）
std::string& c = str; // 符合Left样式，不修改
int* d = nullptr;   // 严格按Left样式声明

// 若显式配置 PointerAlignment: Right，同样严格遵循：
int a* = nullptr;
double b* = nullptr;
std::string c& = str;
```

## 41. DisableFormat: false（启用代码格式化，所有代码均被Clang-Format处理）

### 配置说明

控制是否全局关闭Clang-Format格式化功能：

- `false`（当前配置）：启用格式化，代码将严格遵循所有Clang-Format配置规则
- `true`（不开启）：关闭格式化，代码保持原始样式，不做任何格式调整

#### 不开启（true，关闭格式化，保留原始混乱样式）

```cpp
// 原始代码：缩进混乱、换行随意、等号不对齐
int a=10;
double  b =3.14;
std::string c="test";
void func(int a,
    double b, std::string c) {
if (a>b) {
sum +=a;
} else {
sum +=b;
}
}
```

#### 开启（false，当前配置，启用格式化，按规则整理样式）

```cpp
// 格式化后：缩进整齐、换行规范、等号对齐（遵循之前的配置）
int a     = 10;
double b  = 3.14;
std::string c = "test";
void func(int a,
          double b, std::string c) {
    if (a > b) {
        sum += a;
    }
    else {
        sum += b;
    }
}
```

## 43. ExperimentalAutoDetectBinPacking: false（不自动检测参数打包方式，严格遵循BinPack配置）

### 配置说明

控制是否自动检测函数参数（形参/实参）的打包方式（实验性功能）：

- `false`（当前配置）：不自动检测，严格遵循 `BinPackArguments`/`BinPackParameters` 的配置规则
- `true`（不开启/实验性开启）：自动检测代码原有参数排列风格，自适应调整打包方式

#### 不开启（true，自动检测并继承原有参数打包风格）

```cpp
// 原始代码：函数实参采用分块打包风格
func(10, 3.14, "test",
     true, 100);

// 格式化后：自动检测并继承该分块风格，保持不变
func(10, 3.14, "test",
     true, 100);

// 原始代码：函数形参采用全分行风格
void func2(
    int a,
    double b,
    std::string c);

// 格式化后：自动检测并继承全分行风格，不强制改为分块打包
void func2(
    int a,
    double b,
    std::string c);
```

#### 开启（false，当前配置，不自动检测，严格遵循BinPack配置）

```cpp
// 假设 BinPackArguments: true
// 格式化后：严格按分块打包规则处理，忽略原始代码风格
func(10, 3.14, "test",
     true, 100); // 符合BinPackArguments: true，保持分块

// 若原始代码是全分行风格，会强制按BinPack配置调整为分块打包
func2(
    10,
    3.14,
    "test");
// 格式化后（按BinPackArguments: true）：
func2(10, 3.14,
      "test");

// 函数形参同理，严格遵循BinPackParameters配置
```

## 43. ForEachMacros: [ foreach, Q_FOREACH, BOOST_FOREACH ]（将指定宏解读为foreach循环，而非函数调用）

### 配置说明

控制Clang-Format对自定义宏的解析方式：

- 配置该参数（当前配置）：将 `foreach`、`Q_FOREACH`、`BOOST_FOREACH` 解读为foreach循环（与 `for` 循环格式一致）
- 不配置该参数：将这些宏解读为普通函数调用，按函数调用格式格式化

#### 不开启（未配置该参数，解读为函数调用）

```cpp
// 按普通函数调用格式化：括号内参数紧凑排列，无循环体对齐
foreach (auto& elem, vec) { // 解读为函数调用，格式与func(a,b)一致
    std::cout << elem << std::endl;
}

Q_FOREACH (int val, int_list) { // 空格混乱，按函数调用处理
    sum += val;
}

BOOST_FOREACH (std::string& str, str_list) {
    str += "_suffix";
}
```

#### 开启（当前配置，解读为foreach循环，按for循环格式处理）

```cpp
// 按for循环格式化：对齐整齐，循环体缩进规范
foreach (auto& elem, vec) { // 与for (auto& elem : vec)格式一致
    std::cout << elem << std::endl;
}

Q_FOREACH (int val, int_list) { // 格式统一，与普通for循环对齐
    sum += val;
}

BOOST_FOREACH (std::string& str, str_list) {
    str += "_suffix";
}

// 对比普通for循环，格式完全一致
for (auto& elem : vec) {
    std::cout << elem << std::endl;
}
```

## 45. #include 排序（按正则表达式优先级排序，优先级越小越靠前）

### 配置说明

控制 `#include` 语句的排序规则：通过正则表达式匹配头文件，为匹配的头文件指定优先级（优先级越小排序越靠前），未匹配的头文件默认优先级为 `INT_MAX`（排序最靠后）。

- 配置该参数：按自定义优先级排序 `#include`
- 不配置该参数：使用Clang-Format默认排序（通常按头文件类型/字母序排序，无自定义优先级）

#### 不开启（未配置自定义优先级，默认排序）

```cpp
// 默认排序：按字母序+头文件类型（本地头文件""在前，系统头文件<>在后）
#include "b_header.h"
#include "a_header.h"
#include <vector>
#include <string>
#include <iostream>
#include "c_header.h"
```

#### 开启（配置自定义优先级，按优先级排序）

假设配置示例（对应需求）：

```
# 配置示例（伪代码）
IncludeCategories:
  - Regex:           '^".*/my_project/.*\.h"'
    Priority:        1 # 项目内部头文件，优先级最高
  - Regex:           '^"[^/].*\.h"'
    Priority:        2 # 本地当前目录头文件，优先级次之
  - Regex:           '^<boost/.*\.h>'
    Priority:        3 # Boost库头文件，优先级第三
  - Regex:           '^<.*>'
    Priority:        4 # 其他系统头文件，优先级第四
```

格式化后（按优先级排序，同优先级内按字母序）：

```cpp
// 优先级1（项目内部头文件，按字母序）
#include "my_project/a_inner.h"
#include "my_project/b_inner.h"

// 优先级2（本地当前目录头文件，按字母序）
#include "a_header.h"
#include "b_header.h"
#include "c_header.h"

// 优先级3（Boost库头文件，按字母序）
#include <boost/algorithm.h>
#include <boost/utility.h>

// 优先级4（其他系统头文件，按字母序）
#include <iostream>
#include <string>
#include <vector>
```

## 46. IncludeCategories（按正则优先级排序#include，优先级越小越靠前）

### 配置说明

通过正则表达式匹配头文件路径/名称，为不同类型头文件分配优先级（优先级越小排序越靠前，支持负数优先级），未匹配的头文件默认优先级为 `INT_MAX`（最靠后）。当前配置的优先级规则（需注意：正则匹配按声明顺序，先匹配先生效，当前配置优先级实际生效为 `1` > `2` > `3`）：

1.  匹配 `^"(llvm|llvm-c|clang|clang-c)/`：优先级 2
2.  匹配 `^(<|"(gtest|isl|json)/`：优先级 3
3.  匹配所有（`.*`）：优先级 1（兜底规则，未匹配前两个正则的头文件均适用）

#### 不开启（未配置IncludeCategories，默认排序）

默认排序规则：本地头文件（用 `"` 包裹）在前，系统头文件（用 `<>` 包裹）在后，**同类型内按字母序排序**，无自定义优先级区分。

```cpp
// 原始混乱样式，格式化后按默认规则排序
#include "json/reader.h" // 本地头文件（""）
#include <gtest/gtest.h> // 系统头文件（<>）
#include "clang/AST.h"   // 本地头文件（""）
#include "my_header.h"   // 本地头文件（""）
#include <vector>        // 系统头文件（<>）
#include "llvm/IR.h"     // 本地头文件（""）
#include <iostream>      // 系统头文件（<>）
```

格式化后（默认样式）：

```cpp
// 本地头文件（""）：按字母序排列
#include "clang/AST.h"
#include "json/reader.h"
#include "llvm/IR.h"
#include "my_header.h"

// 系统头文件（<>）：按字母序排列
#include <gtest/gtest.h>
#include <iostream>
#include <vector>
```

#### 开启（当前配置，按自定义正则优先级排序）

按配置的优先级规则（1 > 2 > 3）排序，**同优先级内按字母序排列**，先匹配的正则优先生效：

1.  优先级 1：未匹配前两个正则的头文件（如 `my_header.h`、`<vector>`、`<iostream>`）
2.  优先级 2：匹配 `llvm/`、`clang/` 的头文件（如 `clang/AST.h`、`llvm/IR.h`）
3.  优先级 3：匹配 `gtest/`、`json/` 的头文件（如 `json/reader.h`、`<gtest/gtest.h>`）

格式化后（按自定义优先级排序）：

```cpp
// 优先级 1（最靠前，同优先级按字母序：本地头文件""在前，系统头文件<>在后，内部再按字母序）
#include "my_header.h"
#include <iostream>
#include <vector>

// 优先级 2（中间，同优先级按字母序）
#include "clang/AST.h"
#include "llvm/IR.h"

// 优先级 3（最靠后，同优先级按字母序：本地头文件""在前，系统头文件<>在后）
#include "json/reader.h"
#include <gtest/gtest.h>
```

> 补充：若要让某类头文件永远在最前，可设置负数优先级（如 `Priority: -1`），例如配置 `Regex: "^\"my_project/\"" Priority: -1`，则 `#include "my_project/xxx.h"` 会排在所有头文件之前。

## 47. IndentCaseLabels: false（不缩进switch的case标签，与switch对齐）

### 配置说明

控制 `switch` 语句中 `case`、`default` 标签的缩进方式：

- `false`（当前配置）：`case`/`default` 标签不缩进，与 `switch` 关键字对齐
- `true`（不开启/反向值）：`case`/`default` 标签缩进一个层级（与 `switch` 体内代码缩进一致）

#### 不开启（true，缩进case标签，与switch体内代码对齐）

```cpp
switch (cmd) {
    case 0: // case标签缩进4个空格（与体内代码对齐）
        std::cout << "Exit" << std::endl;
        break;
    case 1: // 缩进4个空格
        std::cout << "Run" << std::endl;
        break;
    case 2: // 缩进4个空格
        for (int i = 0; i < 5; i++) {
            sum += i;
        }
        break;
    default: // 缩进4个空格
        std::cout << "Unknown command" << std::endl;
}
```

#### 开启（false，当前配置，不缩进case标签，与switch对齐）

```cpp
switch (cmd) {
case 0: // case标签不缩进，与switch关键字对齐
    std::cout << "Exit" << std::endl;
    break;
case 1: // 不缩进，与switch对齐
    std::cout << "Run" << std::endl;
    break;
case 2: // 不缩进，与switch对齐
    for (int i = 0; i < 5; i++) {
        sum += i;
    }
    break;
default: // 不缩进，与switch对齐
    std::cout << "Unknown command" << std::endl;
}
```

## 48. IndentWidth: 4（缩进宽度为4个空格）

### 配置说明

控制代码所有缩进层级的基础宽度（如函数体内、循环体内、if体内等），核心对比不同缩进宽度的差异：

- `4`（当前配置）：每个缩进层级为4个空格
- `2`（不开启/默认常见值，非4）：每个缩进层级为2个空格（更紧凑，视觉层级略模糊）

#### 不开启（缩进宽度2，非4）

```cpp
void func() {
  // 缩进2个空格
  int a = 10;
  if (a > 5) {
    // 再缩进2个空格（累计4个，视觉上较紧凑）
    std::cout << "a is large" << std::endl;
    for (int i = 0; i < 3; i++) {
      // 再缩进2个空格（累计6个）
      sum += i;
    }
  }
}

class MyClass {
public:
  // 缩进2个空格
  void classFunc() {
    // 再缩进2个空格
    double b = 3.14;
  }
};
```

#### 开启（4，当前配置，缩进宽度4个空格）

```cpp
void func() {
    // 缩进4个空格
    int a = 10;
    if (a > 5) {
        // 再缩进4个空格（累计8个，视觉层级清晰）
        std::cout << "a is large" << std::endl;
        for (int i = 0; i < 3; i++) {
            // 再缩进4个空格（累计12个）
            sum += i;
        }
    }
}

class MyClass {
public:
    // 缩进4个空格
    void classFunc() {
        // 再缩进4个空格
        double b = 3.14;
    }
};
```

## 49. IndentWrappedFunctionNames: false（函数返回类型换行时，不缩进函数名）

### 配置说明

控制当函数返回类型过长需要换行时，函数名的缩进方式：

- `false`（当前配置）：函数名不缩进，与返回类型所在行的行首对齐
- `true`（不开启/反向值）：函数名缩进一个层级（按 `IndentWidth` 配置的宽度缩进）

#### 不开启（true，函数返回类型换行后，缩进函数名）

```cpp
// 超长返回类型，换行后函数名缩进4个空格
very_long_return_type_name_that_needs_line_break
    func1(int a, double b) { // 函数名缩进4个空格
    return very_long_return_type_name_that_needs_line_break();
}

// 类成员函数同理
class MyClass {
public:
    // 返回类型换行后，函数名缩进4个空格
    std::map<int, std::string> long_member_function_return_type
        classFunc(int a) { // 函数名缩进4个空格
        return std::map<int, std::string>();
    }
};
```

#### 开启（false，当前配置，函数返回类型换行后，不缩进函数名）

```cpp
// 超长返回类型，换行后函数名不缩进，与返回类型行首对齐
very_long_return_type_name_that_needs_line_break
func1(int a, double b) { // 函数名不缩进，与上一行返回类型对齐
    return very_long_return_type_name_that_needs_line_break();
}

// 类成员函数同理
class MyClass {
public:
    // 返回类型换行后，函数名不缩进，与返回类型行首对齐
    std::map<int, std::string> long_member_function_return_type
    classFunc(int a) { // 函数名不缩进
        return std::map<int, std::string>();
    }
};
```

## 50. KeepEmptyLinesAtTheStartOfBlocks: true（保留块开始处的空行）

### 配置说明

控制代码块（如函数体、if块、循环块、类体等）开始处（`{` 后第一行）的空行是否保留：

- `true`（当前配置）：保留块开始处的空行，保持代码视觉分隔
- `false`（不开启/反向值）：自动删除块开始处的空行，强制紧凑排列

#### 不开启（false，删除块开始处的空行）

```cpp
void func() {
    // 块开始处的空行被删除，直接紧跟代码
    int a = 10;
    sum += a;
}

if (a > 5) {
    // 块开始处的空行被删除
    std::cout << "a is large" << std::endl;
}

class MyClass {
public:
    // 块开始处的空行被删除
    void classFunc() {
        // 函数体开始处的空行也被删除
        double b = 3.14;
    }
private:
    // 块开始处的空行被删除
    int m_value;
};
```

#### 开启（true，当前配置，保留块开始处的空行）

```cpp
void func() {

    // 块开始处的空行被保留，视觉分隔更清晰
    int a = 10;
    sum += a;
}

if (a > 5) {

    // 块开始处的空行被保留
    std::cout << "a is large" << std::endl;
}

class MyClass {

public:
    void classFunc() {

        // 函数体开始处的空行被保留
        double b = 3.14;
    }

private:
    // 块开始处的空行被保留
    int m_value;
};
```

## 51. MacroBlockBegin / MacroBlockEnd（块宏的开始/结束正则，当前配置为空）

### 配置说明

用于识别自定义块宏（如自定义 `BEGIN_MACRO`/`END_MACRO` 包裹的代码块），指定宏的正则表达式后，Clang-Format会按代码块规则格式化宏内代码：

- 当前配置：`''`（空字符串，不开启），不识别任何自定义块宏，按普通宏处理
- 开启配置：指定正则表达式（如 `MacroBlockBegin: '^BEGIN_BLOCK$'`，`MacroBlockEnd: '^END_BLOCK$'`），识别自定义块宏并格式化内部代码

#### 不开启（当前配置，宏值为空，不识别自定义块宏）

自定义块宏内的代码会保持原始混乱样式，Clang-Format不进行格式化：

```cpp
// 自定义块宏
#define BEGIN_BLOCK
#define END_BLOCK

BEGIN_BLOCK
int a=10;
double b =3.14;
if (a>b) {
sum +=a;
}
END_BLOCK

// 格式化后：宏内代码仍混乱，无缩进/换行调整
BEGIN_BLOCK
int a=10;
double b =3.14;
if (a>b) {
sum +=a;
}
END_BLOCK
```

#### 开启（配置宏正则，识别自定义块宏）

假设配置：

```
MacroBlockBegin:  '^BEGIN_BLOCK$'
MacroBlockEnd:    '^END_BLOCK$'
```

格式化后，Clang-Format会将 `BEGIN_BLOCK` 和 `END_BLOCK` 之间的代码按普通代码块规则格式化：

```cpp
// 自定义块宏
#define BEGIN_BLOCK
#define END_BLOCK

// 格式化前（混乱样式）
BEGIN_BLOCK
int a=10;
double b =3.14;
if (a>b) {
sum +=a;
}
END_BLOCK

// 格式化后（按配置规则整理，缩进/换行规范）
BEGIN_BLOCK
    int a = 10;
    double b = 3.14;
    if (a > b) {
        sum += a;
    }
END_BLOCK
```

## 52. MaxEmptyLinesToKeep: 1（最多保留1个连续空行，多余的自动删除）

### 配置说明

控制代码中连续空行的最大数量，超出部分自动删除，保持代码整洁：

- `1`（当前配置）：最多保留1个连续空行，多余的空行自动合并为1个
- `0`（不开启1，无空行）：删除所有连续空行，代码完全紧凑
- `3`（不开启2，多于1）：保留最多3个连续空行，代码分隔更松散

#### 不开启1（MaxEmptyLinesToKeep: 0，删除所有连续空行）

```cpp
void func1() {
    int a = 10;
    sum += a;
}
// 无空行分隔，直接紧跟下一个函数
void func2() {
    double b = 3.14;
    std::cout << b << std::endl;
}
// 无空行分隔，直接紧跟类定义
class MyClass {
public:
    void classFunc() {}
};
```

#### 不开启2（MaxEmptyLinesToKeep: 3，保留最多3个连续空行）

```cpp
void func1() {
    int a = 10;
    sum += a;
}



// 保留3个连续空行，分隔松散
void func2() {
    double b = 3.14;
    std::cout << b << std::endl;
}


// 保留2个连续空行（未超出3个，不删除）
class MyClass {
public:
    void classFunc() {}
};
```

#### 开启（1，当前配置，最多保留1个连续空行）

```cpp
void func1() {
    int a = 10;
    sum += a;
}

// 多余空行被合并为1个，整洁分隔
void func2() {
    double b = 3.14;
    std::cout << b << std::endl;
}

// 保留1个连续空行，分隔清晰且不松散
class MyClass {
public:
    void classFunc() {}
};
```

## 52. NamespaceIndentation: Inner（仅缩进嵌套命名空间的内容，顶层命名空间不缩进）

### 配置说明

控制命名空间内代码的缩进规则，可选值：`None`（无缩进）、`Inner`（仅嵌套命名空间内容缩进）、`All`（所有命名空间内容缩进）。当前配置 `Inner` 是主流平衡样式。

#### 不开启1（None，所有命名空间内容均不缩进）

```cpp
// 顶层命名空间：内容不缩进
namespace TopNamespace {
int top_value = 10;
void top_func() {
    // 函数体内正常缩进，命名空间内无额外缩进
    double pi = 3.14;
}

// 嵌套命名空间：内容也不缩进
namespace InnerNamespace {
int inner_value = 20;
void inner_func() {
    std::cout << "inner func" << std::endl;
}
} // 嵌套命名空间结束
} // 顶层命名空间结束
```

#### 不开启2（All，所有命名空间内容均缩进）

```cpp
// 顶层命名空间：内容缩进4个空格
namespace TopNamespace {
    int top_value = 10;
    void top_func() {
        double pi = 3.14;
    }

    // 嵌套命名空间：内容再缩进4个空格（累计8个）
    namespace InnerNamespace {
        int inner_value = 20;
        void inner_func() {
            std::cout << "inner func" << std::endl;
        }
    }
}
```

#### 开启（Inner，当前配置，仅嵌套命名空间内容缩进）

```cpp
// 顶层命名空间：内容不缩进（保持紧凑）
namespace TopNamespace {
int top_value = 10;
void top_func() {
    double pi = 3.14;
}

// 嵌套命名空间：内容缩进4个空格（仅嵌套层级缩进，视觉层级清晰）
namespace InnerNamespace {
    int inner_value = 20;
    void inner_func() {
        std::cout << "inner func" << std::endl;
    }
} // 嵌套命名空间结束
} // 顶层命名空间结束
```

## 53. ObjCBlockIndentWidth: 4（ObjC 块缩进宽度为4个空格）

### 配置说明

专门控制 Objective-C 中块（Block）的缩进宽度，仅对 ObjC 代码有效：

- `4`（当前配置）：ObjC 块内容缩进4个空格
- `2`（不开启/默认常见值，非4）：ObjC 块内容缩进2个空格

#### 不开启（缩进宽度2，非4）

```objc
// ObjC 方法中的 Block 缩进2个空格
- (void)objcFuncWithBlock {
    [self performBlock:^{
      // Block 内容缩进2个空格
      NSInteger count = 0;
      NSLog(@"count: %ld", count);
    } completion:^(BOOL success) {
      // 另一个 Block 缩进2个空格
      if (success) {
        // Block 内嵌套代码再缩进2个空格
        NSLog(@"success");
      }
    }];
}
```

#### 开启（4，当前配置，ObjC 块缩进宽度4个空格）

```objc
// ObjC 方法中的 Block 缩进4个空格
- (void)objcFuncWithBlock {
    [self performBlock:^{
        // Block 内容缩进4个空格（视觉层级清晰）
        NSInteger count = 0;
        NSLog(@"count: %ld", count);
    } completion:^(BOOL success) {
        // 另一个 Block 缩进4个空格
        if (success) {
            // Block 内嵌套代码再缩进4个空格
            NSLog(@"success");
        }
    }];
}
```

## 55. ObjCSpaceAfterProperty: false（ObjC @property 后不添加空格）

### 配置说明

控制 Objective-C 中 `@property` 关键字后是否添加空格，仅对 ObjC 代码有效：

- `false`（当前配置）：`@property` 后直接跟属性类型，无额外空格
- `true`（不开启/反向值）：`@property` 后添加一个空格，再跟属性类型

#### 不开启（true，@property 后添加空格）

```objc
@interface MyObjCClass : NSObject
// @property 后添加一个空格
@property (nonatomic, assign) NSInteger age;
@property (nonatomic, copy) NSString *name;
@property (nonatomic, strong) UIView *contentView;
@end

@implementation MyObjCClass
// 实现中同理
@end
```

#### 开启（false，当前配置，@property 后不添加空格）

```objc
@interface MyObjCClass : NSObject
// @property 后直接跟属性修饰符，无额外空格（紧凑样式）
@property(nonatomic, assign) NSInteger age;
@property(nonatomic, copy) NSString *name;
@property(nonatomic, strong) UIView *contentView;
@end

@implementation MyObjCClass
// 实现中同理
@end
```

## 55. ObjCSpaceBeforeProtocolList: true（ObjC 协议列表前添加空格）

### 配置说明

控制 Objective-C 中类/接口遵循协议列表（`<Protocol1, Protocol2>`）前是否添加空格，仅对 ObjC 代码有效：

- `true`（当前配置）：类名后添加一个空格，再跟协议列表 `<>`
- `false`（不开启/反向值）：类名后直接跟协议列表 `<>`，无额外空格

#### 不开启（false，协议列表前不添加空格）

```objc
// 类名后直接跟协议列表，无空格
@interface MyObjCClass<NSCopying, NSCoding> : NSObject
@end

// 协议继承时也无空格
@protocol MyProtocol<NSObject>
- (void)protocolFunc;
@end

// 类别定义时也无空格
@interface MyObjCClass (MyCategory)<UIScrollViewDelegate>
@end
```

#### 开启（true，当前配置，协议列表前添加空格）

```objc
// 类名后添加空格，再跟协议列表（视觉更清晰）
@interface MyObjCClass <NSCopying, NSCoding> : NSObject
@end

// 协议继承时也添加空格
@protocol MyProtocol <NSObject>
- (void)protocolFunc;
@end

// 类别定义时也添加空格
@interface MyObjCClass (MyCategory) <UIScrollViewDelegate>
@end
```

## 56. 格式化惩罚值系列配置（PenaltyXXX）

### 配置说明

惩罚值（Penalty）是 Clang-Format 的「权重配置」，值越大表示「越不倾向于执行该操作」（即该操作的格式化成本越高，Clang-Format 会优先避免）。下面针对每个惩罚项，对比「高惩罚值」（当前配置）和「低惩罚值/0」（不开启）的差异。

### 5.1 PenaltyBreakBeforeFirstCallParameter: 19（函数调用首个参数前换行的惩罚值19）

#### 说明

控制「函数名后换行，首个参数单独成行」的惩罚程度：值越大，越不倾向于在首个参数前换行；值越小（如0），越容易出现该换行样式。

#### 不开启（惩罚值0，容易在首个参数前换行）

```cpp
// 函数名后直接换行，首个参数单独成行（Clang-Format 不排斥该样式）
void long_function_name(
    int a, double b, std::string c, bool d) {
    // 函数调用时，也容易在首个参数前换行
    long_function_name(
        10, 3.14, "test", true);
}
```

#### 开启（惩罚值19，当前配置，不倾向于在首个参数前换行）

```cpp
// 优先将首个参数与函数名同行，避免首个参数单独成行（更紧凑）
void long_function_name(int a, double b, std::string c,
                        bool d) {
    // 函数调用时，优先首个参数与函数名同行
    long_function_name(10, 3.14, "test",
                       true);
}
```

### 5.2 PenaltyBreakComment: 300（注释内换行的惩罚值300）

#### 说明

控制「将单行注释拆分为多行注释」的惩罚程度：值越大（300），越不倾向于拆分注释；值越小（如0），会优先按 ColumnLimit 拆分注释。

#### 不开启（惩罚值0，容易拆分注释）

```cpp
// 超长单行注释：按 ColumnLimit 强制拆分为多行
// this is a very long comment that will be split into multiple lines because PenaltyBreakComment is 0, even if it is not necessary
// 拆分后
// this is a very long comment that will be split into multiple lines because
// PenaltyBreakComment is 0, even if it is not necessary
```

#### 开启（惩罚值300，当前配置，不倾向于拆分注释）

```cpp
// 超长单行注释：优先保持单行，即使接近 ColumnLimit（避免注释拆分混乱）
// this is a very long comment that will not be split into multiple lines because PenaltyBreakComment is 300, which makes Clang-Format avoid splitting comments
```

### 5.3 PenaltyBreakFirstLessLess: 120（首次 << 前换行的惩罚值120）

#### 说明

控制 C++ 流操作（`std::cout << ...`）中「首个 `<<` 前换行」的惩罚程度：值越大，越不倾向于在首个 `<<` 前换行；值越小，越容易换行。

#### 不开启（惩罚值0，容易在首个 << 前换行）

```cpp
// 首个 << 前换行，流操作符单独成行
std::cout
    << "this is a message" << std::endl
    << "user name: " << name << std::endl
    << "user age: " << age << std::endl;
```

#### 开启（惩罚值120，当前配置，不倾向于在首个 << 前换行）

```cpp
// 优先将首个 << 与 cout 同行，保持流操作紧凑
std::cout << "this is a message" << std::endl
          << "user name: " << name << std::endl
          << "user age: " << age << std::endl;
```

### 5.4 PenaltyBreakString: 1000（字符串字面量内换行的惩罚值1000）

#### 说明

控制「将单行字符串拆分为多行字符串」的惩罚程度：1000 是高惩罚值，几乎不倾向于拆分字符串；值为0时，会优先按 ColumnLimit 拆分字符串。

#### 不开启（惩罚值0，容易拆分字符串）

```cpp
// 超长字符串：按 ColumnLimit 强制拆分为多行
std::string long_str = "this is a very long string that will be split into multiple lines "
                       "because PenaltyBreakString is 0, which allows splitting strings easily";
```

#### 开启（惩罚值1000，当前配置，几乎不拆分字符串）

```cpp
// 超长字符串：优先保持单行，即使超出 ColumnLimit（避免字符串拆分，保证语义完整）
std::string long_str = "this is a very long string that will not be split into multiple lines because PenaltyBreakString is 1000, which makes Clang-Format strongly avoid splitting strings";
```

### 5.5 PenaltyExcessCharacter: 1000000（超出列限制字符的惩罚值1000000）

#### 说明

控制「代码行字符数超出 ColumnLimit」的惩罚程度：1000000 是极高惩罚值，**强制避免**行字符数超出限制（优先换行拆分，绝不允许超长行）；值较小时（如100），允许少量字符超出列限制。

#### 不开启（惩罚值100，允许少量字符超出列限制）

```cpp
// 假设 ColumnLimit: 80，该行字符数略超80，但惩罚值低，允许保留单行
void short_func(int a, double b, std::string c, bool d, int e, double f) { sum += a + b; }
```

#### 开启（惩罚值1000000，当前配置，强制避免超出列限制）

```cpp
// 假设 ColumnLimit: 80，该行字符数超限制，强制换行拆分
void short_func(int a, double b, std::string c,
                bool d, int e, double f) {
    sum += a + b;
}
```

### 5.6 PenaltyReturnTypeOnItsOwnLine: 60（函数返回类型单独成行的惩罚值60）

#### 说明

控制「将函数返回类型单独放在一行，函数名另起一行」的惩罚程度：值越大，越不倾向于让返回类型单独成行；值越小，越容易出现该样式。

#### 不开启（惩罚值0，容易让返回类型单独成行）

```cpp
// 返回类型单独成行，函数名另起一行
very_long_return_type_name
func_with_long_return_type(int a, double b) {
    return very_long_return_type_name();
}
```

#### 开启（惩罚值60，当前配置，不倾向于让返回类型单独成行）

```cpp
// 优先将返回类型与函数名同行，避免单独成行（更紧凑）
very_long_return_type_name func_with_long_return_type(int a,
                                                      double b) {
    return very_long_return_type_name();
}
```

## 57. PointerAlignment: Left（指针/引用与类型左对齐）

### 配置说明

控制指针（`*`）、引用（`&`）的对齐规则，可选值：`Left`（与类型对齐）、`Right`（与变量名对齐）、`Middle`（居中对齐，极少使用），当前配置 `Left` 是 C++ 主流样式之一。

#### 不开启1（Right，指针/引用与变量名右对齐）

```cpp
// 指针与变量名对齐
int a* = nullptr;
double b* = nullptr;
std::string c* = new std::string("test");

// 引用与变量名对齐
int d& = a;
std::string e& = c;
```

#### 不开启2（Middle，指针/引用居中对齐，非主流样式）

```cpp
// 指针居中对齐（位于类型和变量名中间）
int * a = nullptr;
double * b = nullptr;
std::string * c = new std::string("test");

// 引用居中对齐
int & d = a;
std::string & e = c;
```

#### 开启（Left，当前配置，指针/引用与类型左对齐）

```cpp
// 指针与类型对齐（紧贴类型，后跟变量名）
int* a = nullptr;
double* b = nullptr;
std::string* c = new std::string("test");

// 引用与类型对齐（紧贴类型，后跟变量名）
int& d = a;
std::string& e = *c;
```

## 58. ReflowComments: true（允许重新排版注释，自动调整注释格式）

### 配置说明

控制是否自动重新排版单行注释（`//`）和多行注释（`/* */`）：

- `true`（当前配置）：自动换行超长注释、调整注释缩进，保持注释格式整洁
- `false`（不开启/反向值）：保留注释原始样式，不做任何排版调整

#### 不开启（false，保留注释原始混乱样式）

```cpp
void func() {
    int a = 10;
    // this is a very long comment that exceeds the column limit but will not be reflowed because ReflowComments is false, it will keep the original line break and indent
    if (a > 5) {
        /* this is a multi-line comment
            with messy indentation and long lines,
                it will not be adjusted */
        sum += a;
    }
}
```

#### 开启（true，当前配置，自动重新排版注释）

```cpp
void func() {
    int a = 10;
    // this is a very long comment that exceeds the column limit and will be reflowed
    // automatically to fit the column limit, with consistent indentation
    if (a > 5) {
        /* this is a multi-line comment with neat indentation and adjusted lines,
           it will be reflowed to keep the format tidy */
        sum += a;
    }
}
```

## 59. SortIncludes: true（允许自动排序#include 语句）

### 配置说明

控制是否按规则自动排序 `#include` 语句：

- `true`（当前配置）：按 `IncludeCategories` 优先级（无自定义配置则按默认规则）排序 `#include`
- `false`（不开启/反向值）：保留 `#include` 原始书写顺序，不做任何排序

#### 不开启（false，保留#include 原始混乱顺序）

```cpp
// 原始书写顺序混乱，格式化后仍保持不变
#include <iostream>
#include "my_header.h"
#include <vector>
#include "clang/AST.h"
#include <gtest/gtest.h>
#include "llvm/IR.h"
```

#### 开启（true，当前配置，自动排序#include）

```cpp
// 若沿用之前的 IncludeCategories 配置，格式化后按优先级排序（同优先级按字母序）
// 优先级1（兜底规则：未匹配前两个正则的头文件）
#include "my_header.h"
#include <iostream>
#include <vector>

// 优先级2（llvm/clang 系列头文件）
#include "clang/AST.h"
#include "llvm/IR.h"

// 优先级3（gtest/isl/json 系列头文件）
#include <gtest/gtest.h>

// 无自定义 IncludeCategories 时，按默认规则排序（本地头文件""在前，系统头文件<>在后，内部按字母序）
#include "clang/AST.h"
#include "llvm/IR.h"
#include "my_header.h"
#include <gtest/gtest.h>
#include <iostream>
#include <vector>
```

## 60. SpaceAfterCStyleCast: false（C风格类型转换后不添加空格）

### 配置说明

控制 C 风格类型转换（`(Type)value`）后是否添加空格：

- `false`（当前配置）：转换后直接跟变量/值，无额外空格（紧凑样式）
- `true`（不开启/反向值）：转换后添加一个空格，再跟变量/值

#### 不开启（true，C风格类型转换后添加空格）

```cpp
// 基本类型转换后添加空格
int a = (int) 3.14159;
double b = (double) 100;
char c = (char) 65;

// 指针类型转换后添加空格
void* ptr = malloc(10);
int* int_ptr = (int*) ptr;
```

#### 开启（false，当前配置，C风格类型转换后不添加空格）

```cpp
// 基本类型转换后无空格（紧凑整洁）
int a = (int)3.14159;
double b = (double)100;
char c = (char)65;

// 指针类型转换后无空格
void* ptr = malloc(10);
int* int_ptr = (int*)ptr;
```

## 61. SpaceBeforeAssignmentOperators: true（赋值运算符前添加空格）

### 配置说明

控制所有赋值运算符（`=`、`+=`、`-=`、`*=`、`/=` 等）前是否添加空格：

- `true`（当前配置）：赋值运算符前后均有空格（规范样式，可读性强）
- `false`（不开启/反向值）：赋值运算符前无空格，仅后有空格（紧凑但可读性弱）

#### 不开启（false，赋值运算符前不添加空格）

```cpp
// 基本赋值运算符
int a= 10;
double b= 3.14;
std::string c= "test";

// 复合赋值运算符
a+= 5;
b-= 1.0;
c+= "_suffix";
```

#### 开启（true，当前配置，赋值运算符前添加空格）

```cpp
// 基本赋值运算符（前后均有空格，视觉清晰）
int a = 10;
double b = 3.14;
std::string c = "test";

// 复合赋值运算符（前后均有空格）
a += 5;
b -= 1.0;
c += "_suffix";
```

## 62. SpaceBeforeParens: ControlStatements（仅控制语句前的圆括号添加空格）

### 配置说明

控制开圆括号 `(` 前是否添加空格，可选值：`Never`（从不添加）、`ControlStatements`（仅控制语句添加）、`Always`（始终添加），当前配置 `ControlStatements` 是主流平衡样式。

#### 不开启1（Never，任何场景都不添加空格）

```cpp
// 控制语句（if/for/while）前无空格
if(a > 5) {
    sum += a;
}
for(int i = 0; i < 10; i++) {
    sum += i;
}
while(ptr != nullptr) {
    ptr = ptr->next;
}

// 函数调用/声明前无空格（与 ControlStatements 一致）
void func(int a, double b) {
    func(10, 3.14);
}
```

#### 不开启2（Always，所有场景都添加空格）

```cpp
// 控制语句前添加空格
if (a > 5) {
    sum += a;
}
for (int i = 0; i < 10; i++) {
    sum += i;
}
while (ptr != nullptr) {
    ptr = ptr->next;
}

// 函数调用/声明前也添加空格（与 ControlStatements 不一致）
void func (int a, double b) {
    func (10, 3.14);
}
```

#### 开启（ControlStatements，当前配置，仅控制语句前添加空格）

```cpp
// 控制语句（if/for/while/switch）前添加空格（视觉区分控制语句和函数）
if (a > 5) {
    sum += a;
}
for (int i = 0; i < 10; i++) {
    sum += i;
}
while (ptr != nullptr) {
    ptr = ptr->next;
}
switch (cmd) {
    case 0: break;
}

// 函数调用/声明前不添加空格（保持紧凑）
void func(int a, double b) {
    func(10, 3.14);
}
```

## 63. SpaceInEmptyParentheses: false（空圆括号内不添加空格）

### 配置说明

控制空圆括号 `()` 内是否添加空格：

- `false`（当前配置）：空圆括号内无空格，保持 `()` 样式
- `true`（不开启/反向值）：空圆括号内添加一个空格，形成 `( )` 样式

#### 不开启（true，空圆括号内添加空格）

```cpp
// 空函数声明/定义
void empty_func ( ); // 结合 SpaceBeforeParens: Always 的样式
void empty_func_impl() { // 结合 SpaceBeforeParens: Never 的样式
}

// 空函数调用
empty_func( );

// 控制语句的空圆括号
for ( ; i < 10; i++) { // for 循环初始化/增量为空
}
```

#### 开启（false，当前配置，空圆括号内不添加空格）

```cpp
// 空函数声明/定义（紧凑样式）
void empty_func();
void empty_func_impl() {
}

// 空函数调用
empty_func();

// 控制语句的空圆括号
for (; i < 10; i++) {
}
```

## 64. SpacesBeforeTrailingComments: 2（尾随注释前添加2个空格）

### 配置说明

控制单行尾随注释（`//`）前的空格数量，仅对行尾 `//` 注释有效：

- `2`（当前配置）：注释前保留2个空格（视觉分隔清晰）
- `1`（不开启1，少于2）：注释前仅保留1个空格
- `0`（不开启2，无空格）：注释前无空格，紧贴代码

#### 不开启1（1个空格，尾随注释前仅加1个空格）

```cpp
int a = 10; // 整数a（仅1个空格，视觉较紧凑）
double b = 3.14; // 圆周率
std::string c = "test"; // 测试字符串
```

#### 不开启2（0个空格，尾随注释前无空格）

```cpp
int a = 10;// 整数a（无空格，紧贴代码，可读性差）
double b = 3.14;// 圆周率
std::string c = "test";// 测试字符串
```

#### 开启（2，当前配置，尾随注释前添加2个空格）

```cpp
int a = 10;  // 整数a（2个空格，分隔清晰）
double b = 3.14;  // 圆周率
std::string c = "test";  // 测试字符串
```

## 65. SpacesInAngles: true（尖括号 `<>` 内添加空格）

### 配置说明

控制尖括号 `<` 后、`>` 前是否添加空格（适用于模板、命名空间嵌套等场景）：

- `true`（当前配置）：尖括号内添加空格，视觉清晰
- `false`（不开启/反向值）：尖括号内无空格，紧凑样式

#### 不开启（false，尖括号内不添加空格）

```cpp
// 模板类/函数
std::vector<int> vec;
std::map<int, std::string> mp;
template <typename T> T max_val(T a, T b) {
    return a > b ? a : b;
}

// 嵌套尖括号
std::vector<std::map<int, std::string>> nested_vec;
```

#### 开启（true，当前配置，尖括号内添加空格）

```cpp
// 模板类/函数（尖括号内有空格，视觉层级清晰）
std::vector< int > vec;
std::map< int, std::string > mp;
template < typename T > T max_val(T a, T b) {
    return a > b ? a : b;
}

// 嵌套尖括号（每层都添加空格）
std::vector< std::map< int, std::string > > nested_vec;
```

## 66. SpacesInContainerLiterals: true（容器字面量内添加空格）

### 配置说明

控制容器字面量（C++11 列表初始化、ObjC 数组/字典等）内是否添加空格：

- `true`（当前配置）：容器内元素前后添加空格，视觉清晰
- `false`（不开启/反向值）：容器内无额外空格，紧凑样式

#### 不开启（false，容器字面量内不添加空格）

```cpp
// C++ 容器列表初始化
std::vector<int> vec = {1,2,3,4};
std::map<int, std::string> mp = {{1,"a"},{2,"b"}};
std::pair<int, std::string> p = {1,"test"};

// ObjC 数组/字典字面量
NSArray *arr = @[@1, @2, @3];
NSDictionary *dict = @{@"name": @"test", @"age": @20};
```

#### 开启（true，当前配置，容器字面量内添加空格）

```cpp
// C++ 容器列表初始化（元素前后有空格，分隔清晰）
std::vector<int> vec = { 1, 2, 3, 4 };
std::map<int, std::string> mp = { { 1, "a" }, { 2, "b" } };
std::pair<int, std::string> p = { 1, "test" };

// ObjC 数组/字典字面量（元素前后有空格）
NSArray *arr = @[ @1, @2, @3 ];
NSDictionary *dict = @{ @"name": @"test", @"age": @20 };
```

### 总结

1.  对齐/排版类配置（`PointerAlignment`/`ReflowComments`/`SortIncludes`）：
    - `PointerAlignment: Left` 是 C++ 主流指针对齐样式，代码整洁规范；
    - `ReflowComments: true` 自动优化注释格式，`SortIncludes: true` 保证头文件顺序清晰，提升代码可读性；
2.  空格控制类配置（核心主流样式）：
    - `SpaceBeforeAssignmentOperators: true`、`SpacesBeforeTrailingComments: 2` 保证赋值语句和尾随注释的视觉分隔；
    - `SpaceBeforeParens: ControlStatements` 区分控制语句和函数，平衡清晰性和紧凑性；
    - `SpaceAfterCStyleCast: false`、`SpaceInEmptyParentheses: false` 保持紧凑样式，避免冗余空格；
3.  容器/尖括号空格配置（`SpacesInAngles`/`SpacesInContainerLiterals`）：
    - `true` 配置虽增加少量空格，但提升了模板和容器字面量的视觉层级，更易阅读复杂嵌套结构。

## 67. SpacesInCStyleCastParentheses: true（C风格类型转换的括号内添加空格）

### 配置说明

控制C风格类型转换 `(Type)value` 中，括号 `()` 内部（`(` 后、`)` 前）是否添加空格：

- `true`（当前配置）：括号内添加空格，形成 `( Type )value` 的样式，视觉分隔更清晰
- `false`（不开启/反向值）：括号内无空格，保持 `(Type)value` 的紧凑样式

#### 不开启（false，C风格转换括号内无空格）

```cpp
// 基本数据类型转换（括号内紧凑，无空格）
int a = (int)3.1415926;
double b = (double)100;
char c = (char)65;
long d = (long)INT_MAX;

// 指针类型转换（括号内无空格）
void* raw_ptr = malloc(20);
int* int_ptr = (int*)raw_ptr;
char* char_ptr = (char*)raw_ptr;
const void* const_ptr = (const void*)int_ptr;
```

#### 开启（true，当前配置，C风格转换括号内添加空格）

```cpp
// 基本数据类型转换（括号内添加空格，视觉更清晰）
int a = ( int )3.1415926;
double b = ( double )100;
char c = ( char )65;
long d = ( long )INT_MAX;

// 指针类型转换（括号内添加空格，嵌套转换也生效）
void* raw_ptr = malloc(20);
int* int_ptr = ( int* )raw_ptr;
char* char_ptr = ( char* )raw_ptr;
const void* const_ptr = ( const void* )int_ptr;
```

## 68. SpacesInParentheses: true（圆括号内添加空格，`(` 后、`)` 前均加空格）

### 配置说明

控制所有普通圆括号 `()`（控制语句、函数参数、表达式分组等）内部是否添加空格：

- `true`（当前配置）：圆括号内添加空格，形成 `( 表达式/参数 )` 的样式
- `false`（不开启/反向值）：圆括号内无空格，保持 `(表达式/参数)` 的紧凑样式

#### 不开启（false，圆括号内无空格）

```cpp
// 1. 控制语句括号（紧凑无空格）
if (a > 5 && b < 10) {
    sum += a;
}
for (int i = 0; i < 10; i++) {
    sum += i;
}
while (ptr != nullptr) {
    ptr = ptr->next;
}

// 2. 函数声明/调用括号（无空格）
void func(int a, double b) {
    int result = func(10, 3.14);
}

// 3. 表达式分组括号（无空格）
int calc = (a + b) * (c - d);
bool is_valid = (a == 0) || (b == 0);
```

#### 开启（true，当前配置，圆括号内添加空格）

```cpp
// 1. 控制语句括号（内有空格，视觉分隔清晰）
if ( a > 5 && b < 10 ) {
    sum += a;
}
for ( int i = 0; i < 10; i++ ) {
    sum += i;
}
while ( ptr != nullptr ) {
    ptr = ptr->next;
}

// 2. 函数声明/调用括号（内有空格）
void func( int a, double b ) {
    int result = func( 10, 3.14 );
}

// 3. 表达式分组括号（内有空格，易区分分组边界）
int calc = ( a + b ) * ( c - d );
bool is_valid = ( a == 0 ) || ( b == 0 );
```

## 69. SpacesInSquareBrackets: true（方括号内添加空格，`[` 后、`]` 前均加空格）

### 配置说明

控制普通方括号 `[]`（数组访问、指定大小数组声明等）内部是否添加空格，**Lambda表达式参数列表、未指明大小的数组声明不受影响**：

- `true`（当前配置）：方括号内添加空格，形成 `[ 索引/大小 ]` 的样式
- `false`（不开启/反向值）：方括号内无空格，保持 `[索引/大小]` 的紧凑样式

#### 不开启（false，方括号内无空格）

```cpp
// 1. 数组访问（紧凑无空格）
int arr[5] = {1,2,3,4,5}; // 指定大小的数组
int val = arr[0];
arr[1] = 10;
arr[2] = arr[0] + arr[1];

// 2. 多维数组访问（无空格）
int mat[3][3] = {{1,2,3},{4,5,6},{7,8,9}};
int mat_val = mat[0][1];

// 3. 不受影响的场景（Lambda、未指明大小数组）
int unsized_arr[] = {1,2,3}; // 未指明大小，不受配置影响
auto lambda = [&](int x) { return x * 2; }; // Lambda表达式，不受配置影响
```

#### 开启（true，当前配置，方括号内添加空格）

```cpp
// 1. 数组访问（内有空格，视觉分隔清晰）
int arr[ 5 ] = { 1, 2, 3, 4, 5 }; // 指定大小的数组，括号内加空格
int val = arr[ 0 ];
arr[ 1 ] = 10;
arr[ 2 ] = arr[ 0 ] + arr[ 1 ];

// 2. 多维数组访问（每层括号内都加空格）
int mat[ 3 ][ 3 ] = { { 1, 2, 3 }, { 4, 5, 6 }, { 7, 8, 9 } };
int mat_val = mat[ 0 ][ 1 ];

// 3. 不受影响的场景（样式不变，与配置无关）
int unsized_arr[] = { 1, 2, 3 }; // 未指明大小，括号内无额外空格（不受配置影响）
auto lambda = [&](int x) { return x * 2; }; // Lambda表达式，方括号内无空格（不受配置影响）
```

## 70. Standard: Cpp11（按C++11标准格式化代码，支持C++11特性语法）

### 配置说明

指定格式化时遵循的C++标准，可选值 `Cpp03`、`Cpp11`、`Auto`：

- `Cpp11`（当前配置）：支持C++11及以上特性（列表初始化、Lambda表达式、auto关键字等），按C++11语法格式化
- `Cpp03`（不开启/旧标准）：仅支持C++03语法，不识别C++11特性，会按C++03样式强制修改C++11代码
- `Auto`：自动检测代码使用的C++标准，自适应格式化

#### 不开启（Cpp03，按C++03标准格式化，不兼容C++11特性）

```cpp
// 原始C++11代码（列表初始化、auto、Lambda）
auto val = 10;
std::vector<int> vec = {1,2,3,4};
auto lambda = [&](int x) { return x * 2; };
int arr[] = {1,2,3};

// 格式化后（强制转为C++03兼容样式，破坏C++11特性）
int val = 10; // auto 被强制改为具体类型
std::vector<int> vec;
vec.push_back(1);
vec.push_back(2);
vec.push_back(3);
vec.push_back(4); // 列表初始化被强制转为push_back
// Lambda表达式无法兼容，保持原始样式或格式化异常
int arr[] = {1,2,3}; // 未指明大小数组不受影响
```

#### 开启（Cpp11，当前配置，按C++11标准格式化，兼容C++11特性）

```cpp
// 原始C++11代码（列表初始化、auto、Lambda）
auto val = 10;
std::vector<int> vec = {1,2,3,4};
auto lambda = [&](int x) { return x * 2; };
int arr[] = {1,2,3};

// 格式化后（保留C++11特性，按C++11样式优化排版）
auto val = 10;
std::vector<int> vec = { 1, 2, 3, 4 };
auto lambda = [&](int x) { return x * 2; };
int arr[] = { 1, 2, 3 };
```

## 71. TabWidth: 4（Tab字符的宽度为4个空格）

### 配置说明

指定Tab字符对应的视觉宽度（即使不使用Tab，该配置也会影响依赖Tab宽度的格式化逻辑）：

- `4`（当前配置）：1个Tab字符等价于4个空格的宽度
- `2`（不开启/默认常见值，非4）：1个Tab字符等价于2个空格的宽度

> 补充：该配置需结合 `UseTab` 配置生效，若 `UseTab: Never`，则仅作为格式化时的参考宽度，不实际生成Tab字符。

#### 不开启（TabWidth: 2，1个Tab等价于2个空格宽度）

```cpp
// 若 UseTab: ForIndentation（使用Tab缩进）
// 缩进1级 = 1个Tab（等价2个空格），视觉上更紧凑
void func() {
	// 1个Tab（2个空格宽度）
	int a = 10;
	if (a > 5) {
		// 2个Tab（累计4个空格宽度）
		sum += a;
	}
}

// 若 UseTab: Never（不使用Tab，仅参考宽度）
// 缩进层级按2个空格排列（与IndentWidth: 2效果一致）
void func() {
  int a = 10;
  if (a > 5) {
    sum += a;
  }
}
```

#### 开启（TabWidth: 4，当前配置，1个Tab等价于4个空格宽度）

```cpp
// 若 UseTab: ForIndentation（使用Tab缩进）
// 缩进1级 = 1个Tab（等价4个空格宽度），视觉层级清晰
void func() {
	// 1个Tab（4个空格宽度）
	int a = 10;
	if (a > 5) {
		// 2个Tab（累计8个空格宽度）
		sum += a;
	}
}

// 若 UseTab: Never（不使用Tab，仅参考宽度）
// 缩进层级按4个空格排列（与IndentWidth: 4效果一致，当前配置即为此场景）
void func() {
    int a = 10;
    if (a > 5) {
        sum += a;
    }
}
```

## 72. UseTab: Never（从不使用Tab字符，全部用空格实现缩进/对齐）

### 配置说明

控制是否使用Tab字符，可选值 `Never`、`ForIndentation`、`Always` 等：

- `Never`（当前配置）：完全不生成Tab字符，所有缩进、对齐均使用空格
- `ForIndentation`（不开启1）：仅缩进部分使用Tab字符，对齐部分使用空格
- `Always`（不开启2）：所有需要空白的地方（缩进、对齐）均优先使用Tab字符

#### 不开启1（UseTab: ForIndentation，缩进用Tab，对齐用空格）

```cpp
// 缩进部分（1级缩进=1个Tab，显示为\t），对齐部分用空格
void func() {
	// \t（Tab缩进）
	int a = 10;
	int b = 3.14;  // \t + 空格对齐（等号对齐）
	if (a > 5) {
		// \t\t（2个Tab缩进）
		sum += a;
		sum += b;  // \t\t + 空格对齐
	}
}

// 注：实际文件中，缩进位置是Tab字符（\t），对齐位置是空格（  ）
```

#### 不开启2（UseTab: Always，所有空白优先用Tab）

```cpp
// 缩进和对齐均使用Tab字符（尽可能用Tab填充空白）
void func() {
	// \t（Tab缩进）
	int a = 10;
	int b = 3.14;  // \t + \t（Tab对齐，可能导致视觉对齐混乱）
	if (a > 5) {
		// \t\t（2个Tab缩进）
		sum += a;
		sum += b;  // \t\t + \t（Tab对齐）
	}
}

// 注：实际文件中，所有空白位置均为Tab字符，不同编辑器Tab宽度不一致时，会出现对齐错乱
```

#### 开启（Never，当前配置，从不使用Tab，全部用空格）

```cpp
// 所有缩进、对齐均使用空格（无Tab字符，跨编辑器样式一致）
void func() {
    // 4个空格（缩进1级）
    int a = 10;
    int b = 3.14;  // 4个空格 + 2个空格（对齐等号）
    if (a > 5) {
        // 8个空格（缩进2级）
        sum += a;
        sum += b;  // 8个空格 + 2个空格（对齐等号）
    }
}

// 注：实际文件中无\t字符，全部为空格，在任何编辑器中显示样式一致
```

