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