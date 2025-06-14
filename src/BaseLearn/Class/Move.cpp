#ifndef _STRVEC_H_
#define _STRVEC_H_
#include <string>
#include <memory>
#include <stdexcept>

class StrVec
{
private:
    static std::allocator<std::string> alloc_; // C++17以上 construct、destroy被移除
    std::string* start_;
    std::string* finish_;
    std::string* end_of_storage_;
    using AllocTraits = std::allocator_traits<decltype(alloc_)>;
public:
    // 类型定义
    using iterator = std::string*;
    using const_iterator = const std::string*;
    using size_type = std::size_t;
    using reference = std::string&;
    using const_reference = const std::string&;

    ///< 无参构造
    ///< 拷贝构造
    ///< 拷贝赋值
    ///< 移动构造
    ///< 移动赋值
    ///< 析构函数
    StrVec() : start_(nullptr), finish_(nullptr), end_of_storage_(nullptr) {}
    StrVec(const StrVec& rhs) : start_(nullptr), finish_(nullptr), end_of_storage_(nullptr)
    {
        *this = rhs;
    }
    StrVec& operator=(const StrVec& rhs)
    {
        if (this == &rhs)
            return *this;
        clear();
        start_ = alloc_.allocate(rhs.size());
        finish_ = start_;
        end_of_storage_ = start_ + rhs.size();
        for (const auto& s : rhs)
#if __cplusplus >= 201703L
            AllocTraits::construct(alloc_, finish_++, s);
#else
            alloc_.construct(finish_++, s);
#endif
        return *this;
    }
    StrVec(StrVec&& rhs) noexcept;
    StrVec& operator=(StrVec&& rhs) noexcept;
    ~StrVec()
    {
        clear();
    }

    // 元素访问
    reference at(size_type n)
    {
        if (n >= size())
            throw std::out_of_range("StrVec::at out of range");
        return *(start_ + n);
    }
    const_reference at(size_type n) const
    {
        if (n >= size())
            throw std::out_of_range("StrVec::at out of range");
        return *(start_ + n);
    }
    reference front() { return *start_; }
    const_reference front() const { return *start_; }
    reference back() { return *(finish_ - 1); }
    const_reference back() const { return *(finish_ - 1); }

    // 迭代器
    iterator begin() { return start_; }
    const_iterator begin() const { return start_; }
    const_iterator cbegin() const { return start_; }
    iterator end() { return finish_; }
    const_iterator end() const { return finish_; }
    const_iterator cend() const { return finish_; }

    // 容量
    void reserve(size_type n)
    {
        if (n <= capacity()) return;
        
        std::string* new_start = alloc_.allocate(n);
        std::string* new_finish = new_start;
        
        for (auto p = start_; p != finish_; ++p)
#if __cplusplus >= 201703L
            AllocTraits::construct(alloc_, new_finish++, std::move(*p));
#else
            alloc_.construct(new_finish++, std::move(*p));
#endif
        size_type old_size = size();
        clear();
        
        start_ = new_start;
        finish_ = new_start + old_size;
        end_of_storage_ = new_start + n;
    }

    void resize(size_type n, const std::string& val = std::string())
    {
        if (n < size())
        {
            while (finish_ != start_ + n)
#if __cplusplus >= 201703L
                AllocTraits::destroy(alloc_, --finish_);
#else
                alloc_.destroy(--finish_);
#endif
        }
        else if (n > size())
        {
            if (n > capacity())
                reserve(n * 2);
            while (finish_ != start_ + n)
#if __cplusplus >= 201703L
                AllocTraits::construct(alloc_, finish_++, val);
#else
                alloc_.construct(finish_++, val);
#endif
        }
    }

    void push_back(const std::string& s)
    {
        if (finish_ == end_of_storage_)
            reallocate();
#if __cplusplus >= 201703L
        AllocTraits::construct(alloc_, finish_++, s);
#else
        alloc_.construct(finish_++, s);
#endif
    }

    // 添加移动版本的push_back
    void push_back(std::string&& s)
    {
        if (finish_ == end_of_storage_)
            reallocate();
#if __cplusplus >= 201703L
        AllocTraits::construct(alloc_, finish_++, std::move(s));
#else
        alloc_.construct(finish_++, std::move(s));
#endif
    }

    void pop_back()
    {
        if (empty())
            return;
#if __cplusplus >= 201703L
        AllocTraits::destroy(alloc_, --finish_);
#else
        alloc_.destroy(--finish_);
#endif
    }

    void clear()
    {
        if (!start_) return; // 空指针检查
        for (auto p = finish_; p != start_;)
#if __cplusplus >= 201703L
            AllocTraits::destroy(alloc_, start_++);
#else
            alloc_.destroy(start_++);
#endif
        alloc_.deallocate(start_, capacity());
        start_ = finish_ = end_of_storage_ = nullptr;
    }

    std::size_t size() const
    {
        return finish_ - start_;
    }

    std::size_t capacity() const
    {
        return end_of_storage_ - start_;
    }

    bool empty() const
    {
        return finish_ == start_;
    }

    std::string& operator[](std::size_t n)
    {
        return *(start_ + n);
    }

    const std::string& operator[](std::size_t n) const
    {
        return *(start_ + n);
    }

private:
    void reallocate()
    {
        std::size_t old_size = size();
        std::size_t new_size = old_size? 2 * old_size : 1;
        std::string* new_start = alloc_.allocate(new_size);
        
        // 使用移动构造函数提高效率
        for (size_type i = 0; i < old_size; ++i)
#if __cplusplus >= 201703L
            AllocTraits::construct(alloc_, new_start + i, std::move(*(start_ + i)));
#else
            alloc_.construct(new_start + i, std::move(*(start_ + i)));
#endif
        
        // 销毁原有元素
        for (size_type i = 0; i < old_size; ++i)
#if __cplusplus >= 201703L
            AllocTraits::destroy(alloc_, start_ + i);
#else
            alloc_.destroy(start_ + i);
#endif
            
        if (start_)
            alloc_.deallocate(start_, capacity());
            
        start_ = new_start;
        finish_ = new_start + old_size;
        end_of_storage_ = new_start + new_size;
    }
};

std::allocator<std::string> StrVec::alloc_;

///< 类内成员方法声明了noexcept, 那么要求类外实现时也需要声明noexcept
/// 
///< 虽然移动通常不抛出异常，但抛出异常是被允许的
/*
 * 以可以移动的vector对象为例
 * 但不声明noexcept,因移动一个对象通常会改变它的值，如果重新分配过程使用移动构造函数
 * 且在移动了部分而不是全部元素后抛出了一个异常，就会产生问题旧空间中的移动源元素已经被改变了，
 * 而新空间中未构造的元素可能尚不存在。在此情况下，vector将不能满足自身保持不变的要求。

 * 如果希望在vectori重新分配内存这类情况下对我们自定义类型的对像进行移动而不
 * 是拷贝，就必须显式地告诉标准库我们的移动构造函数可以安全使用。我们通过将
 * 移动构造函数（及移动赋值运算符）标记为noexcept来做到这一点。
*/
/// 
///< 移动构造不分配任何内存，它只是转移对象所有资源的所有权，源对象将被销毁，因此不能够依赖于源对象的数据，比如通过 源对象.size() 进行判断
StrVec::StrVec(StrVec&& rhs) noexcept : start_(rhs.start_), finish_(rhs.finish_), end_of_storage_(rhs.end_of_storage_)
{
    rhs.start_ = nullptr;
    rhs.finish_ = nullptr;
    rhs.end_of_storage_ = nullptr;
}

StrVec& StrVec::operator=(StrVec&& rhs) noexcept
{
    if (this == &rhs)
        return *this;
    clear();
    start_ = rhs.start_;
    finish_ = rhs.finish_;
    end_of_storage_ = rhs.end_of_storage_;
    rhs.start_ = nullptr;
    rhs.finish_ = nullptr;
    rhs.end_of_storage_ = nullptr;
    return *this;
}


///< md tips:
///< 1. 如果一个类定义了拷贝构造、拷贝赋值或析构函数，那么编译器将不会生成默认的移动构造函数、移动赋值运算符或移动构造函数。
///< 2. 只有当一个类没有定义任何自己版本的拷贝控制成员，且类的每个非static数据成
///     员都可以移动时，编译器才会为它合成移动构造函数或移动赋值运算符。编译器可
///     以移动内置类型的成员。如果一个成员是类类型，且该类有对应的移动操作，编译
///     器也能移动这个成员：
struct X {
    int value;  // 内置类型成员
    std::string str; // string定义了移动操作
};
struct HasX {
    X x; // 类类型成员
};
X x1, x2 = std::move(x1); // 合成的移动构造函数将调用string的移动构造函数
HasX h1, h2 = std::move(h1); // 合成的移动构造函数将调用X的移动构造函数

///< 3. 如果移动设置为 = default，且类内的成员可以移动，编译器将自动生成移动，否则等价于 移动 = delete
///< 4. 析构函数定义为 = delete或private，或不定义析构时，编译器将不会自动生成移动，等价于 移动 = delete。
///< 5. 一个类如果只定义拷贝构造，不定义移动，即使使用std::move也不会调用移动，而是拷贝。
struct Point {
    Point() = default;
    Point(const Point&) = default;
};
Point point1;
Point point2(point1); // 调用拷贝构造函数，point1是左值
Point point3(std::move(point1)); // 调用拷贝构造函数，因为没有移动构造函数

///< 6. 3/5法则，如果定义了任何一个拷贝或移动构造函数，则应该定义所有。（5法则是引入了移动出现的）
#endif // !_STRVEC_H_