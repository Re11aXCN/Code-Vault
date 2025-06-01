### C++ 模板详解：从基础到高级特性

---

#### **为什么需要模板？**
在早期 C++ 中，实现通用功能需为不同类型重复编写代码（如 `max(int, int)`、`max(double, double)`）。模板通过**泛型编程**解决此问题，允许编写与类型无关的代码，提升复用性并减少冗余。

---

#### **模板的作用**

1. **类型无关的通用算法**（如 STL 容器和算法）
2. **编译期多态**（优于运行时多态的性能）
3. **元编程基础**（编译期计算）
4. **代码精简**（避免手动重复）

---

#### **模板编译**

##### 1. 编译要求

> 当编译器遇到一个模板定义时，它并不生成代码。只有当我们实例化出模板的一个特定版本时，编译器才会生成代码。当我们使用（而不是定义）模板时，编译器才生成代码，这一特性影响了我们如何组织代码以及错误何时被检测到。
>
> 通常，*当我们调用一个函数时，编译器只需要掌握函数的声明*。类似的，当我们使用一个类类型的对象时，类定义必须是可用的，但成员函数的定义不必已经出现。因此，我们将类定义和函数声明放在头文件中，而普通函数和类的成员函数的定义放在源文件中。
>
> 模板则不同：**为了生成一个实例化版本，编译器需要掌握函数模板或类模板成员函数的定义**。因此，与非模板代码不同，模板的头文件通常既包括声明也包括定义。

##### 2. 编译报错三阶段

模板直到实例化时才会生成代码，这一特性影响了我们何时才会获知模板内代码的
编译错误。通常，编译器会在三个阶段报告错误。

* 第一个阶段是编译模板本身时。在这个阶段，编译器通常不会发现很多错误。编译器可以**检查语法错误，例如忘记分号或者变量名拼错等**，但也就这么多了。

* 第二个阶段是编译器遇到模板使用时。在此阶段，编译器仍然没有很多可检查的。对于函数模板调用，编译器通常**会检查实参数目是否正确。它还能检查参数类型是否匹配**。对于类模板，编译器可以检查用户是否提供了正确数目的模板实参，但也仅限于此了。

* 第三个阶段是模板实例化时，**只有这个阶段才能发现类型相关的错误**。依赖于编译器如何管理实例化，这类错误可能在链接时才报告。

---

#### **模板定义**

##### 1. 写法使用

```cpp
template <typename T, class U> // ok
template <typename T, U> // error
    
template<typename T>
class BlobPtr;//前向声明

template<typename T, class U>
using Pair = std::pair<T, U> //类型别名

template<typename T>
using PairNo = std::pair<T, unsigned> //偏特化
PairNo<std::string> books; //使用

//默认情况下，C+语言假定通过作用域运算符访问的名字不是类型。
//因此，如果我们希望使用一个模板类型参数的类型成员，就必须显式告诉编译器该名字是
//我们通过使用关键字typename来实现这一点：
//用typename告知编译器T:value_type是一个类型
template <typename T>
typename T:value_type top(const T &c)
{
	return !c.empty() ? c.back() : typename T:value_type();
}
```



##### 2. 函数模板
```cpp
template <typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}
// 使用：max<int>(3, 5); max<double>(2.8, 3.1);
```

##### 3. 类模板

类模板(class template)是用来生成类的蓝图的。与函数模板的不同之处是，编译器不能为类模板推断模板参数类型。如我们已经多次看到的，为了使用类模板，我们必须在模板名后的尖括号中提供额外信息`vector<int>`用来代替模板参数的模板实参列表。对于每个类型的实例，编译器都会生成一个对应类型的类

```cpp
template <typename T>
class Stack {
private:
    std::vector<T> elems;
public:
    void push(const T& elem);
    T pop();
};
// 使用：Stack<std::string> s;
```

---

### 进阶模板编程
#### **1. SFINAE (Substitution Failure Is Not An Error)**
当模板参数匹配失败时，编译器会跳过该候选而非报错，常用于条件编译。

```cpp
#include <type_traits>

// 仅对整数类型生效
template <typename T>
auto add(T a, T b) -> std::enable_if_t<std::is_integral_v<T>, T> {
    return a + b;
}

add(3, 5);     // 编译通过
add(3.2, 5.1); // 编译错误：无匹配函数
```

#### **2. C++20 约束与概念 (Concepts)**
简化 SFINAE，通过 `concept` 声明类型约束。

```cpp
template <typename T>
concept Integral = std::is_integral_v<T>;

template <Integral T> // 约束 T 必须是整数类型
T multiply(T a, T b) {
    return a * b;
}

multiply(4, 2);     // OK
// multiply(1.5, 2.0); // 错误：不满足约束
```

#### **3. 参数包与完美转发**
- **参数包 (Variadic Templates)**：处理任意数量参数
- **完美转发**：保持参数的值类别（左值/右值）

```cpp
// 递归展开参数包
void print() {} // 终止递归

template <typename First, typename... Rest>
void print(First&& first, Rest&&... rest) {
    std::cout << std::forward<First>(first) << " ";
    print(std::forward<Rest>(rest)...);
}

print("Hello", 42, 3.14); // 输出：Hello 42 3.14
```

#### **4. Lambda 定义匿名模板方法 (C++14+)**
Lambda 支持泛型参数（`auto`），实现匿名模板函数。

```cpp
// 泛型 Lambda（C++14）
auto make_adder = [](auto x) {
    return [x](auto y) { return x + y; };
};

auto add5 = make_adder(5);
std::cout << add5(3);     // 输出 8 (int)
std::cout << add5(2.3);   // 输出 7.3 (double)
```

#### 5. 模板特化与偏特化

##### 为什么需要特化？
模板提供通用实现，但特定类型可能需要：
1. **性能优化**（如指针类型的特殊处理）
2. **特殊行为**（如C字符串的格式化）
3. **边界情况处理**（如空指针处理）
4. **类型适配**（如自定义类型的格式化）

---

##### 模板特化（Template Specialization）

###### 1. 全特化（Full Specialization）
为特定类型提供完全不同的实现

```cpp
#include <format>
#include <iostream>
#include <vector>

// 主模板
template <typename T>
struct Formatter {
    static std::string format(const T& value) {
        return std::format("Value: {}", value);
    }
};

// 全特化：int类型
template <>
struct Formatter<int> {
    static std::string format(int value) {
        return std::format("INT: {0} (hex: {0:#x})", value);
    }
};

// 全特化：vector类型
template <typename T>
struct Formatter<std::vector<T>> {
    static std::string format(const std::vector<T>& vec) {
        std::string result = "Vector[";
        for (const auto& item : vec) {
            result += Formatter<T>::format(item) + ", ";
        }
        if (!vec.empty()) result.erase(result.end()-2, result.end());
        return result + "]";
    }
};
```

使用示例：

```cpp
int main() {
    std::cout << Formatter<double>::format(3.14) << "\n"; 
    // 输出: Value: 3.14
    
    std::cout << Formatter<int>::format(42) << "\n";      
    // 输出: INT: 42 (hex: 0x2a)
    
    std::vector<int> vec = {1, 2, 3};
    std::cout << Formatter<decltype(vec)>::format(vec) << "\n";
    // 输出: Vector[INT: 1 (hex: 0x1), INT: 2 (hex: 0x2), INT: 3 (hex: 0x3)]
}
```

---

###### 2.偏特化（Partial Specialization）
（**类模板特有，函数模板需用重载替代**），对模板参数的部分限制进行特化

```cpp
// 主模板：指针类型
template <typename T>
struct Formatter<T*> {
    static std::string format(T* ptr) {
        if (!ptr) return "Null pointer";
        return std::format("Pointer to: {}", Formatter<T>::format(*ptr));
    }
};

// 偏特化：对指针的指针
template <typename T>
struct Formatter<T**> {
    static std::string format(T** ptr) {
        if (!ptr || !*ptr) return "Null double pointer";
        return std::format("Double pointer to: {}", Formatter<T>::format(**ptr));
    }
};

// 偏特化：所有智能指针类型
template <template <typename> typename SmartPtr, typename T>
struct Formatter<SmartPtr<T>> {
    static std::string format(const SmartPtr<T>& ptr) {
        if (!ptr) return "Empty smart pointer";
        return std::format("SmartPtr: {}", Formatter<T>::format(*ptr));
    }
};
```

使用示例：

```cpp
#include <memory>

int main() {
    int x = 10;
    int* px = &x;
    int** ppx = &px;
    
    std::cout << Formatter<decltype(px)>::format(px) << "\n";
    // 输出: Pointer to: INT: 10 (hex: 0xa)
    
    std::cout << Formatter<decltype(ppx)>::format(ppx) << "\n";
    // 输出: Double pointer to: INT: 10 (hex: 0xa)
    
    auto sp = std::make_shared<double>(3.14);
    std::cout << Formatter<decltype(sp)>::format(sp) << "\n";
    // 输出: SmartPtr: Value: 3.14
}
```

---

##### 函数模板特化替代方案
###### 使用重载 + 约束（C++20）
```cpp
// 主模板
template <typename T>
std::string format_value(const T& value) {
    return std::format("Value: {}", value);
}

// 重载版本：C风格字符串
std::string format_value(const char* str) {
    return std::format("C-string: \"{}\"", str);
}

// 使用C++20约束的重载
template <typename T>
requires std::is_pointer_v<T>
std::string format_value(T ptr) {
    if (!ptr) return "Null pointer";
    return std::format("Pointer: {}", format_value(*ptr));
}
```

---

##### 特化机制解析
| 类型       | 特点                               | 适用场景                            |
| ---------- | ---------------------------------- | ----------------------------------- |
| **全特化** | 完全指定所有模板参数               | 特定类型的完全重写（如`vector<T>`） |
| **偏特化** | 部分指定模板参数（仅类模板）       | 类型类别处理（如所有指针类型）      |
| **重载**   | 函数模板替代方案（结合约束更强大） | 函数模板的特殊版本                  |

---

### 关键特性对比
| 特性         | 用途              | 示例                           |
| ------------ | ----------------- | ------------------------------ |
| **函数模板** | 通用算法          | `sort(vec.begin(), vec.end())` |
| **SFINAE**   | 条件编译          | `std::enable_if`               |
| **Concepts** | 类型约束（C++20） | `requires std::integral<T>`    |
| **参数包**   | 处理不定参数      | `std::make_shared<T>(args...)` |
| **完美转发** | 保持值类别        | `std::forward<T>(arg)`         |

---

### 参考书籍
1. **《C++ Primer》第5版**
2. **《Effective Modern C++》**
3. **《C++ Templates: The Complete Guide》**



# 赋值规则

## **核心赋值规则**
1. **基本赋值要求**：
   - **左侧必须是可修改的左值**（modifiable lvalue）
   - 右侧可以是左值或右值，但必须能隐式转换为左侧类型
   - **常量对象（const lvalue）不能被赋值**（初始化后不可修改）

```cpp
int a = 10;       // √ 右值赋给非const左值（初始化）
a = 20;           // √ 右值赋给非const左值

const int b = 30;
// b = 40;        // ✖ 错误！const左值不可修改
```

---

### **变量类型与赋值权限**

| 左侧操作数类型  | 可接受右侧操作数             | 示例（`=` 操作）               |
| --------------- | ---------------------------- | ------------------------------ |
| **非const左值** | 右值、非const左值、const左值 | `int x; x = 10; x = y; x = z;` |
| **const左值**   | 仅允许在初始化时赋值         | `const int c = y; // √`        |
|                 | 后续赋值禁止                 | `// c = 20; ✖ 错误！`          |
| **右值**        | 永远不能作为被赋值对象       | `// 10 = x; ✖ 错误！`          |

---

### **引用类型的特殊规则**
#### 1. 左值引用 (`T&`)
| 引用类型        | 可绑定的值                       | 示例                             |
| --------------- | -------------------------------- | -------------------------------- |
| **非const引用** | 只能绑定**非const左值**          | `int& r1 = a; // √ (a是非const)` |
|                 | 不能绑定const左值或右值          | `int& r2 = b; // ✖ (b是const)`   |
|                 |                                  | `int& r3 = 10; // ✖ (右值)`      |
| **const引用**   | 可绑定const左值/非const左值/右值 | `const int& cr1 = a; // √`       |
|                 |                                  | `const int& cr2 = b; // √`       |
|                 |                                  | `const int& cr3 = 10; // √`      |

#### 2. 右值引用 (`T&&`)
| 引用类型     | 可绑定的值            | 示例                             |
| ------------ | --------------------- | -------------------------------- |
| **右值引用** | 只能绑定**右值**      | `int&& rr1 = 10; // √`           |
|              | 不能直接绑定左值      | `int&& rr2 = a; // ✖`            |
|              | 可通过`std::move`转换 | `int&& rr3 = std::move(a); // √` |

---

### **常量性（const）传播规则**
| 场景                         | 是否允许              | 原因                       |
| ---------------------------- | --------------------- | -------------------------- |
| **非const → 非const**        | √ 允许                | 无常量丢失                 |
| **const → 非const**          | √ 允许（复制值）      | 右侧的常量性不影响左侧     |
| **非const → const**          | √ 允许（初始化/构造） | 增加常量性是安全的         |
| **const → const**            | √ 允许（初始化/构造） | 常量性一致                 |
| **去除常量性（const_cast）** | ✖ 禁止（未定义行为）  | 修改常量对象导致未定义行为 |

```cpp
// 常量性传播示例
int x = 10;
const int y = x;    // √ 非const → const (安全)

const int z = 20;
int w = z;          // √ const → 非const (复制值)

// 危险操作（未定义行为）
const int c_val = 100;
int& cheat = const_cast<int&>(c_val);
cheat = 200;         // ✖ 编译通过，但运行时未定义行为！
```

---

### **类对象的特殊规则**
类对象遵循相同基础规则，但受赋值运算符重载影响：
```cpp
class MyClass {
public:
    // 拷贝赋值运算符 (接受 const 左值引用)
    MyClass& operator=(const MyClass&) { return *this; }

    // 移动赋值运算符 (接受 非const 右值引用)
    MyClass& operator=(MyClass&&) { return *this; }
};

MyClass obj1;
const MyClass obj2;

obj1 = obj2;       // √ 调用拷贝赋值 (const 左值 → 非const 左值)
obj1 = MyClass();  // √ 调用移动赋值 (右值 → 非const 左值)

// obj2 = obj1;    // ✖ 错误！const对象不可赋值
```

---

### **总结表格**
| 操作                          | 是否允许 | 条件/说明                   |
| ----------------------------- | -------- | --------------------------- |
| **非const左值 = 右值**        | ✓        | 基础赋值                    |
| **非const左值 = 非const左值** | ✓        | 值拷贝                      |
| **非const左值 = const左值**   | ✓        | 值拷贝（不修改原const对象） |
| **const左值 = 任意值**        | ✗        | const对象不可修改           |
| **右值 = 任意值**             | ✗        | 右值不可作为被赋值对象      |
| **右值引用 = 右值**（字面量） | ✓        |                             |
| **非const引用绑定右值**       | ✗        | 必须用const引用或右值引用   |
| **const引用绑定右值**         | ✓        | 延长右值生命周期            |
| **右值引用绑定左值**          | ✗        | 需用`std::move`转换         |

掌握这些规则能有效避免常见错误，如：
1. 尝试修改常量对象
2. 将右值绑定到非const引用
3. 在赋值时意外丢弃常量性

## std::move

理解万能引用折叠，**与运算**

```cpp
typename <class T>
typename std::remove_reference<T>::type &&move(T &&t)
{
    return static_cast<typename std::remove_reference<T>::type &&>(t);
}

// int & i
// move(i) -> t 为int &
// remove_reference t int & -> t int
// static_cast && t int -> t int &&
//符合预期，我们将一个对象转变为了右值引用赋值给左值，即移动

// 字面量(右值引用)
// move(88) -> t 为int
// remove_reference t int -> t int
// static_cast && t int -> t int &&
```

# 类型退化

## 1. 数组退化为指针（最常见）
当数组作为函数参数按值传递时，会自动退化为指向其首元素的指针：

```cpp
#include <iostream>
#include <type_traits>

void print_size(int arr[5]) {
    // 这里 arr 实际是指针，不是数组
    std::cout << "In function: " 
              << std::is_same_v<decltype(arr), int*> << "\n"; // true
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};
    std::cout << "In main: " 
              << std::is_same_v<decltype(arr), int[5]> << "\n"; // true
    
    print_size(arr); // 传入时退化为 int*
}
```

**输出：**
```
In main: 1
In function: 1
```

**避免方法：**
1. **使用引用传递**：
   ```cpp
   void print_size(int (&arr)[5]) {
       std::cout << "Size: " << sizeof(arr)/sizeof(arr[0]) << "\n"; // 5
   }
   ```

2. **使用 `std::array`**：
   ```cpp
   #include <array>
   void print_size(std::array<int, 5> arr) {
       std::cout << "Size: " << arr.size() << "\n"; // 5
   }
   ```

3. **模板推导保留类型**：
   ```cpp
   template <size_t N>
   void print_size(int (&arr)[N]) {
       std::cout << "Size: " << N << "\n";
   }
   ```

---

## 2. 函数退化为函数指针
函数作为参数传递时，会退化为函数指针：

```cpp
#include <iostream>
#include <type_traits>

void func() {}

void call(void f()) {
    // 这里 f 实际是函数指针
    std::cout << "In call: "
              << std::is_same_v<decltype(f), void(*)()> << "\n"; // true
}

int main() {
    std::cout << "In main: "
              << std::is_same_v<decltype(func), void()> << "\n"; // true
              
    call(func); // 传入时退化为 void(*)()
}
```

**避免方法：**
1. **使用函数引用**：
   ```cpp
   void call(void (&f)()) {
       std::cout << "Type preserved\n";
   }
   ```

2. **模板推导保留类型**：
   ```cpp
   template <typename F>
   void call(F&& f) {
       static_assert(std::is_same_v<F, void()>);
   }
   ```

---

## 3. 顶层 const 被忽略
当按值传递带有顶层 `const` 的对象时，`const` 会被忽略：

```cpp
void process(int x) {
    x = 10; // 允许修改
}

int main() {
    const int y = 5;
    process(y); // const 被丢弃
}
```

**避免方法：**
使用引用传递保留 `const`：
```cpp
void process(const int& x) {
    // x = 10; // 错误：不能修改 const 引用
}
```

---

## 4. 模板参数推导时的类型退化
模板类型推导会移除引用和顶层 `const`：

```cpp
template <typename T>
void deduce(T param) {}

int main() {
    int x = 10;
    const int& rx = x;
    
    deduce(rx); // T 被推导为 int（移除了 const 和引用）
}
```

**避免方法：**
1. **完美转发保留类型**：
   ```cpp
   template <typename T>
   void forward(T&& param) {
       // param 保留原始类型
   }
   ```

2. **显式指定类型**：
   ```cpp
   deduce<const int&>(rx); // 显式指定类型
   ```

---

## 5. 避免类型提升的最佳实践

| 场景         | 问题                | 解决方案                               |
| ------------ | ------------------- | -------------------------------------- |
| 数组传递     | 退化为指针          | 使用引用传递或 `std::array`            |
| 函数传递     | 退化为函数指针      | 使用函数引用或模板                     |
| 保留 `const` | 顶层 `const` 被忽略 | 使用 `const` 引用                      |
| 模板推导     | 移除引用和 `const`  | 使用完美转发（`T&&` + `std::forward`） |
| 保留原始类型 | 类型信息丢失        | 使用 `decltype(auto)`（C++14+）        |

```cpp
// 保留所有类型信息的终极方案（C++14+）
decltype(auto) preserve_type() {
    int arr[5];
    return (arr); // 返回 int(&)[5]（注意括号使 decltype 产生引用）
}
```

---

## 关键总结：
1. **数组和函数最容易发生退化** → 使用引用传递
2. **模板推导会剥离引用和顶层 const** → 使用完美转发
3. **值传递会丢弃顶层 const** → 使用 const 引用
4. **需要精确类型时** → 使用 `decltype(auto)` 或模板技巧

这些方法能确保在传递左右值时，类型信息不会意外丢失或被提升。

## std::forward

```cpp
template <class T>
constexpr T&& forward(std::remove_reference_t<T>& arg) noexcept {
    return static_cast<T&&>(arg);
}

auto forward = []<typename T> (std::remove_reference_t<T>&arg) constexpr -> T&& {
    return static_cast<T&&>(arg);
};
```

