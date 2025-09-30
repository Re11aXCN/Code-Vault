# C11

## 1. 赋值规则

### **核心赋值规则**

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

#### **变量类型与赋值权限**

| 左侧操作数类型  | 可接受右侧操作数             | 示例（`=` 操作）               |
| --------------- | ---------------------------- | ------------------------------ |
| **非const左值** | 右值、非const左值、const左值 | `int x; x = 10; x = y; x = z;` |
| **const左值**   | 仅允许在初始化时赋值         | `const int c = y; // √`        |
|                 | 后续赋值禁止                 | `// c = 20; ✖ 错误！`          |
| **右值**        | 永远不能作为被赋值对象       | `// 10 = x; ✖ 错误！`          |

---

#### **引用类型的特殊规则**

##### 1. 左值引用 (`T&`)

| 引用类型        | 可绑定的值                       | 示例                             |
| --------------- | -------------------------------- | -------------------------------- |
| **非const引用** | 只能绑定**非const左值**          | `int& r1 = a; // √ (a是非const)` |
|                 | 不能绑定const左值或右值          | `int& r2 = b; // ✖ (b是const)`   |
|                 |                                  | `int& r3 = 10; // ✖ (右值)`      |
| **const引用**   | 可绑定const左值/非const左值/右值 | `const int& cr1 = a; // √`       |
|                 |                                  | `const int& cr2 = b; // √`       |
|                 |                                  | `const int& cr3 = 10; // √`      |

##### 2. 右值引用 (`T&&`)

| 引用类型     | 可绑定的值            | 示例                             |
| ------------ | --------------------- | -------------------------------- |
| **右值引用** | 只能绑定**右值**      | `int&& rr1 = 10; // √`           |
|              | 不能直接绑定左值      | `int&& rr2 = a; // ✖`            |
|              | 可通过`std::move`转换 | `int&& rr3 = std::move(a); // √` |

---

#### **常量性（const）传播规则**

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

#### **类对象的特殊规则**

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

#### **总结表格**

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

### std::move

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

## 2. 类型退化

### 1. 数组退化为指针（最常见）

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

### 2. 函数退化为函数指针

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

### 3. 顶层 const 被忽略

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

### 4. 模板参数推导时的类型退化

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

### 5. 避免类型提升的最佳实践

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

### 关键总结：

1. **数组和函数最容易发生退化** → 使用引用传递
2. **模板推导会剥离引用和顶层 const** → 使用完美转发
3. **值传递会丢弃顶层 const** → 使用 const 引用
4. **需要精确类型时** → 使用 `decltype(auto)` 或模板技巧

这些方法能确保在传递左右值时，类型信息不会意外丢失或被提升。

## 3. std::forward/lambda模板使用问题

### （1）lambda实现同款forward

```cpp
auto forward = []<typename T> (std::remove_reference_t<T>&arg) constexpr -> T&& {
    return static_cast<T&&>(arg);
};
int left = 0;
forward<int>(left); // 错误调用
forward.operator()<int>(left); // 正确调用
/*
问题在于：lambda表达式虽然可以模板化，但是它的调用方式与普通函数模板不同。我们不能像调用函数模板那样直接在lambda对象名后面加上模板参数。因为lambda对象本身是一个闭包类型的对象，它不是一个函数模板。
*/
```

### （2）标准库forward

```cpp
template <class T>
constexpr T&& forward(std::remove_reference_t<T>& arg) noexcept {
    return static_cast<T&&>(arg);
}

int x = 0;
int& y = x;
int&& z = 1;
std::forward(int)(x);
std::forward(int&)(y);
std::forward(int&&)(z);
std::forward(int)(z);
```

下面是对四个 `std::forward` 调用的完整流程分析，使用箭头表示每一步的类型转换过程。我们使用标准库中的 `std::forward` 实现：

```cpp
template <class T>
constexpr T&& forward(std::remove_reference_t<T>& arg) noexcept {
    return static_cast<T&&>(arg);
}
```

### 1. `std::forward<int>(x);`

```
参数: int x (左值)
↓
T = int
↓
std::remove_reference_t<T> = std::remove_reference_t<int> = int
↓
函数参数类型: int& arg (绑定到x)
↓
static_cast<T&&> = static_cast<int&&>
↓
返回类型: int&& (右值引用)
```

**结果**: 将左值 `x` 转换为右值引用

### 2. `std::forward<int&>(y);`

```
参数: int& y (左值引用，绑定到x)
↓
T = int&
↓
std::remove_reference_t<T> = std::remove_reference_t<int&> = int
↓
函数参数类型: int& arg (绑定到y)
↓
static_cast<T&&> = static_cast<int& &&> → 引用折叠为 int&
↓
返回类型: int& (左值引用)
```

**结果**: 保持左值引用性质

### 3. `std::forward<int&&>(z);`

```
参数: int&& z (右值引用，但z本身是左值)
↓
T = int&&
↓
std::remove_reference_t<T> = std::remove_reference_t<int&&> = int
↓
函数参数类型: int& arg (绑定到z)
↓
static_cast<T&&> = static_cast<int&& &&> → 引用折叠为 int&&
↓
返回类型: int&& (右值引用)
```

**结果**: 将具名右值引用 `z` 转换回右值引用

### 4. `std::forward<int>(z);`

```
参数: int&& z (右值引用，但z本身是左值)
↓
T = int
↓
std::remove_reference_t<T> = std::remove_reference_t<int> = int
↓
函数参数类型: int& arg (绑定到z)
↓
static_cast<T&&> = static_cast<int&&>
↓
返回类型: int&& (右值引用)
```

**结果**: 将具名右值引用 `z` 转换为右值引用

### 类型转换流程图总结：

| 调用表达式          | T       | remove_reference_t<T> | 参数类型 | static_cast<T&&> | 返回类型 | 实际效果            |
| ------------------- | ------- | --------------------- | -------- | ---------------- | -------- | ------------------- |
| `forward<int>(x)`   | `int`   | `int`                 | `int&`   | `int&&`          | `int&&`  | 左值→右值引用       |
| `forward<int&>(y)`  | `int&`  | `int`                 | `int&`   | `int&` (折叠)    | `int&`   | 保持左值引用        |
| `forward<int&&>(z)` | `int&&` | `int`                 | `int&`   | `int&&` (折叠)   | `int&&`  | 保持右值引用        |
| `forward<int>(z)`   | `int`   | `int`                 | `int&`   | `int&&`          | int&&    | 右值引用→纯右值引用 |
| `forward<int>(12)`  | `int`   | `int`                 | `int&`   | `int&&`          | `int&&`  | 右值→纯右值引用     |

### 关键规则说明：

1. **`remove_reference_t<T>` 行为**：
   - 移除所有引用修饰符，得到基础类型
   - `int` → `int`
   - `int&` → `int`
   - `int&&` → `int`

2. **函数参数类型**：
   - 总是 `remove_reference_t<T>&`（左值引用）
   - 因此只能接受左值（包括具名右值引用）

3. **`static_cast<T&&>` 行为**：
   - 发生引用折叠：
     - `T&&` + `&` → `T&`（当 `T` 是左值引用时）
     - `T&&` + `&&` → `T&&`（当 `T` 是右值引用时）
   - 保留原始值类别：
     - 当 `T` 是引用类型时，返回对应引用
     - 当 `T` 是非引用类型时，返回右值引用

4. **具名右值引用的特殊性**：
   - `z` 是右值引用类型，但作为表达式是左值
   - `forward` 可以将其恢复为右值引用