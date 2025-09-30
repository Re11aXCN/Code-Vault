#ifndef _OPERATOR_H_
#define _OPERATOR_H_

#include <iostream>
#include <functional>
#include <map>
#include <string>
struct MyClass {
	int value;
	int operator()(int x) const { return x + value; }
};
std::ostream& operator<<(std::ostream& os, const MyClass& obj)
{
	os << "MyClass object: " << obj.value;
	return os;
}
std::istream& operator>>(std::istream& is, MyClass& obj)
{
	is >> obj.value;
	return is;
}
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

//std::function<int(int, int)>f1 = add;//函数指针
std::function<int(int, int)>f2 = divide();//函数对象类的对象
std::function<int(int, int)>f3 = [](int i, int j) {return i % j; };//lambda

MyClass add(const MyClass& obj1, const MyClass& obj2)
{
	MyClass result;
	result.value = obj1.value + obj2.value;
	return result;
}
void test_function() {
	int (*ADD)(int, int) = add;
	std::map<std::string, std::function<int(int, int)>> binops = {
		//{"+", add}, // 二义性不知道是哪个add, 可以使用函数指针解决{"+", ADD}，或者使用lambda解决[](int i, int j) {return add(i, j); }
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
#endif // _OPERATOR_H_