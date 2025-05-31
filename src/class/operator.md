# 1. 重载输出运算符<<

> 通常情况下，输出运算符的**第一个形参是一个非常量ostream对像的引用**。之所以ostream是非常量是因为向流写入内容会改变其状态；而该形参是引用是因为我们<u>无法直接复制一个ostream对象</u>。
>
> 第**二个形参一般来说是一个常量的引用**，该常量是我们想要打印的类类型。<u>第二个</u><u>形参是引用的原因是我们希望避免复制实参</u>；而之所以该形参可以是常量是因为(通常情况下)打印对像不会改变对象的内容。
>
> 为了与其他输出运算符保持一致，operator<<一般要返回它的ostream形参。
>
> 

```cpp
std::ostream& operator<<(std::ostream& os, const MyClass& obj)
{
	os << "MyClass object: " << obj.value;
	return os;
}
```

# 2. 重载输入运算符>>

```cpp
std::istream& operator>>(std::istream& is, MyClass& obj)
{
	is >> obj.value;
	return is;
}
```

# 3. 重载+、==、!=

```cpp
MyClass operator+(const MyClass& obj1, const MyClass& obj2)
{
	MyClass result;
	result.value = obj1.value + obj2.value;
	return result;
}
bool operator==(const MyClass& obj1, const MyClass& obj2)
{
	return obj1.value == obj2.value;
}
bool operator!=(const MyClass& obj1, const MyClass& obj2)
{
	return obj1.value!= obj2.value;
}
```

# 4. 重载前/后置++、--

要想同时定义前置和后置运算符，必须首先解决一个问题，即普通的重载形式无法区分这两种情况。前置和后置版本使用的是同一个符号，意味着其重载版本所用的名字将是相同的，并且运算对像的数量和类型也相同。
为了解决这个问题，**后置版本接受一个额外的（不被使用）it类型的形参**。当我们使用后置运算符时，编译器为这个形参提供一个值为0的实参。尽管从语法上来说后置函数可以使用这个额外的形参，但是在实际过程中通常不会这么做。**这个形参的唯一作用就是区分前置版本和后置版本的函数，而不是真的要在实现后置版本时参与运算**。

```cpp
///< 如果声明再类内，就不需要MyClass& obj，因为会隐藏一个默认的this指针
///< 前置++
MyClass& operator++(MyClass& obj)
{
	obj.value++;
	return obj;
}
///< 后置++
MyClass operator++(MyClass& obj, int)
{
	MyClass result = obj;
	obj.value++;
	return result;
}
///< 前置--
MyClass& operator--(MyClass& obj)
{
	obj.value--;
	return obj;
}
///< 后置--
MyClass operator--(MyClass& obj, int)
{
	MyClass result = obj;
	obj.value--;
	return result;
}
```

# 5. 重载() 仿函数

```cpp
struct MyClass {
	int value;
	int operator()(int x) const { return x + value; }
};
```

## lambda表达式就是生成了一个结构体仿函数

# function

```cpp
//普通函数
int add(int i, int j) { return i + j; }
//lambda,其产生一个未命名的函数对象类
auto mod = [](int i, int j) {return i % j; };
//函数对象类
struct divide {
	int operator()(int denominator, int divisor) const {
		return denominator / divisor;
	}
};
```

function是一个模板，和我们使用过的其他模板一样，当创健一个具体的function类型时我们必须提供额外的信息。在此例中，所谓额外的信息是指该function类型能够表示的对象的调用形式。参考其他模板，我们在一对尖括号内指定类型：

```cpp
function<int (int,int)>f1 = add;//函数指针
function<int(int,int)>f2 = divide();//函数对象类的对象
function<int (int,int)>f3 = [](int i, int j) {return i % j; };//lambda
```

使用这个function类型我们可以重新定义map:

```cpp
//列举了可调用对象与二元运算符对应关系的表格
//所有可调用对象都必须接受两个1nt、返回一个int
//其中的元素可以是函数指针、函数对象或者lambda
MyClass add(const MyClass& obj1, const MyClass& obj2)
{
	MyClass result;
	result.value = obj1.value + obj2.value;
	return result;
}
int test_function() {
	int (*ADD)(int, int) = add;
	std::map<std::string, std::function<int(int, int)>> binops = {
		{"+", add}, // 二义性不知道是哪个add, 可以使用函数指针解决{"+", ADD}，或者使用lambda解决[](int i, int j) {return add(i, j); }
		{"-", std::minus<int>()},
		{"/", divide()},
		{"*", [](int i, int j) {return i * j; }},
		{"%", mod },
	};
	binops["+"](10, 5);
	binops["-"](10, 5);
	binops["/"](10, 5);
	binops["*"](10, 5);
	binops["%"](10, 5);
}
```

# MyString

```cpp
class MyString {
public:
    // 默认构造函数
    MyString() : data_(new char[1]), size_(0), capacity_(1) {
        data_[0] = '\0';
    }

    // 从C字符串构造
    MyString(const char* str) {
        size_ = strlen(str);
        capacity_ = size_ + 1;
        data_ = new char[capacity_];
        strncpy(data_, str, size_);
        data_[size_] = '\0';
    }

    // 拷贝构造函数
    MyString(const MyString& other) :
        size_(other.size_),
        capacity_(other.capacity_)
    {
        data_ = new char[capacity_];
        strncpy(data_, other.data_, size_);
        data_[size_] = '\0';
    }

    // 移动构造函数
    MyString(MyString&& other) noexcept
        : data_(other.data_),
        size_(other.size_),
        capacity_(other.capacity_)
    {
        other.data_ = nullptr;
        other.size_ = 0;
        other.capacity_ = 0;
    }

    // 析构函数
    ~MyString() {
        delete[] data_;
    }

    // 拷贝赋值运算符（3/5法则）
    MyString& operator=(const MyString& other) {
        if (this != &other) {
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            data_ = new char[capacity_];
            strncpy(data_, other.data_, size_);
            data_[size_] = '\0';
        }
        return *this;
    }

    // 移动赋值运算符
    MyString& operator=(MyString&& other) noexcept {
        if (this != &other) {
            delete[] data_;
            data_ = other.data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            other.data_ = nullptr;
            other.size_ = 0;
            other.capacity_ = 0;
        }
        return *this;
    }

    // 基本操作
    size_t size() const { return size_; }
    size_t capacity() const { return capacity_; }
    bool empty() const { return size_ == 0; }

    void push_back(char c) {
        if (size_ + 1 >= capacity_) {
            reserve(capacity_ == 0 ? 1 : capacity_ * 2);
        }
        data_[size_] = c;
        size_++;
        data_[size_] = '\0';
    }

    void pop_back() {
        if (size_ > 0) {
            size_--;
            data_[size_] = '\0';
        }
    }

    void erase(size_t pos, size_t len = npos) {
        if (pos >= size_) return;

        size_t erase_len = (len == npos || pos + len > size_) ?
            size_ - pos : len;

        char* dest = data_ + pos;
        char* src = data_ + pos + erase_len;
        size_t bytes = size_ - pos - erase_len + 1;

        memmove(dest, src, bytes);
        size_ -= erase_len;
    }

    void insert(size_t pos, const char* str, size_t len = npos) {
        if (pos > size_) return;
        if (len == npos) len = strlen(str);

        size_t new_size = size_ + len;
        if (new_size + 1 > capacity_) {
            reserve(std::max(capacity_ * 2, new_size + 1));
        }

        memmove(data_ + pos + len, data_ + pos, size_ - pos + 1);
        strncpy(data_ + pos, str, len);
        size_ = new_size;
    }

    void reserve(size_t new_cap) {
        if (new_cap <= capacity_) return;

        char* new_data = new char[new_cap];
        strncpy(new_data, data_, size_);
        new_data[size_] = '\0';

        delete[] data_;
        data_ = new_data;
        capacity_ = new_cap;
    }

    void clear() {
        size_ = 0;
        data_[0] = '\0';
    }

    // 运算符重载
    char& operator[](size_t idx) { return data_[idx]; }
    const char& operator[](size_t idx) const { return data_[idx]; }

    MyString& operator+=(const MyString& rhs) {
        size_t new_size = size_ + rhs.size_;
        if (new_size + 1 > capacity_) {
            reserve(new_size + 1);
        }
        strncpy(data_ + size_, rhs.data_, rhs.size_);
        size_ = new_size;
        data_[size_] = '\0';
        return *this;
    }

    const char* c_str() const { return data_; }

private:
    char* data_ = nullptr;
    size_t size_ = 0;
    size_t capacity_ = 0;
    static const size_t npos = static_cast<size_t>(-1);
};

// 非成员运算符重载
bool operator==(const MyString& lhs, const MyString& rhs) {
    if (lhs.size() != rhs.size()) return false;
    return strcmp(lhs.c_str(), rhs.c_str()) == 0;
}

bool operator!=(const MyString& lhs, const MyString& rhs) {
    return !(lhs == rhs);
}

MyString operator+(const MyString& lhs, const MyString& rhs) {
    MyString result = lhs;
    result += rhs;
    return result;
}

std::ostream& operator<<(std::ostream& os, const MyString& str) {
    os << str.c_str();
    return os;
}

std::istream& operator>>(std::istream& is, MyString& str) {
    char buffer[1024];
    if (is >> buffer) {
        str = MyString(buffer);
    }
    return is;
}
```

