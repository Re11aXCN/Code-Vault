#ifndef _BLOB_HPP_
#define _BLOB_HPP_
#include <vector>
#include <memory>
#include <stdexcept> // 添加异常头文件

/*
## shared_ptr和weak_ptr的设计理念
### 为什么使用shared_ptr？
在Blob类中，使用 std::shared_ptr<std::vector<T>> 管理底层数据有以下优点：

1. 资源共享 ：多个Blob对象可以共享同一个vector，实现写时复制(COW)语义
2. 自动内存管理 ：当最后一个引用vector的Blob对象被销毁时，vector会自动释放
3. 线程安全的引用计数 ：shared_ptr的引用计数是原子操作，多线程环境下安全
4. 异常安全 ：即使发生异常，shared_ptr也能确保资源正确释放
### 为什么使用weak_ptr？
BlobPtr类中使用 std::weak_ptr<std::vector<T>> 而不是shared_ptr的原因：

1. 避免循环引用 ：如果BlobPtr使用shared_ptr，可能导致循环引用，造成内存泄漏
2. 观察者模式 ：BlobPtr只是观察Blob对象，不应影响其生命周期
3. 悬空指针检测 ：通过weak_ptr可以检测Blob对象是否已被销毁
4. 按需获取控制权 ：只有在实际需要访问数据时，才通过lock()临时获取shared_ptr

## 应用场景
这种设计模式适用于以下场景：

1. 需要共享大型数据结构的应用，如文本编辑器中多个视图共享同一文档
2. 需要观察者模式但不希望观察者影响被观察对象生命周期的场景
3. 缓存系统 ，其中缓存项可能被多个客户端引用，但缓存管理器需要能够在必要时释放资源
4. 编辑器/查看器分离的应用，如文档编辑器和只读查看器


*/

// 前向声明BlobPtr模板类
template<typename T>
class BlobPtr;

/**
 * @brief Blob类模板 - 一个类似于std::vector的容器，但支持共享数据
 * @tparam T 元素类型
 */
template<typename T>
class Blob {
private:
    // 使用shared_ptr管理动态分配的vector，允许多个Blob对象共享同一个vector
    std::shared_ptr<std::vector<T>> _data;
    // 声明BlobPtr为友元类，使其可以访问Blob的私有成员
    friend class BlobPtr<T>;
public:
    // 类型别名定义
    using value_type = T;
    using size_type = typename std::vector<T>::size_type; // 修复：添加typename关键字

    // 构造函数
    Blob() : _data(std::make_shared<std::vector<T>>()) {}
    Blob(std::initializer_list<T> il) : _data(std::make_shared<std::vector<T>>(il)) {}

    // 容器操作
    size_type size() const { return _data->size(); }
    bool empty() const { return _data->empty(); }
    void push_back(const T& value) { _data->push_back(value); }
    void pop_back() { _data->pop_back(); }

    // 元素访问
    T& front() { return _data->front(); }
    const T& front() const { return _data->front(); }
    T& back() { return _data->back(); }
    const T& back() const { return _data->back(); }
    T& operator[](size_type index) { return (*_data)[index]; }
    const T& operator[](size_type index) const { return (*_data)[index]; }

    // 比较操作符
    friend bool operator==(const Blob<T>& lhs, const Blob<T>& rhs) { // 修复：添加参数名
        if (lhs._data->size() != rhs._data->size()) return false;
        for (size_type i = 0; i < lhs._data->size(); ++i) {
            if ((*lhs._data)[i] != (*rhs._data)[i]) return false;
        }
        return true;
    }

    friend bool operator!=(const Blob<T>& lhs, const Blob<T>& rhs) { // 修复：添加参数名
        return !(lhs == rhs);
    }
};

/**
 * @brief BlobPtr类模板 - Blob的迭代器类，使用weak_ptr避免循环引用
 * @tparam T 元素类型
 */
template<typename T>
class BlobPtr {
private:
    // 使用weak_ptr引用Blob中的vector，不增加引用计数
    std::weak_ptr<std::vector<T>> _wptr;
    std::size_t _index; // 当前位置

    // 检查weak_ptr是否有效，并返回对应的shared_ptr
    std::shared_ptr<std::vector<T>> _check_ptr() const {
        if (_wptr.expired()) throw std::runtime_error("BlobPtr: dangling pointer"); // 修复：使用expired()检查
        return _wptr.lock(); // 修复：返回lock()结果
    }

public:
    // 构造函数
    BlobPtr() : _index(0) {}
    BlobPtr(Blob<T>& blob, std::size_t index = 0) : _wptr(blob._data), _index(index) {}
    BlobPtr(const Blob<T>& blob) : _wptr(blob._data), _index(0) {}
    BlobPtr(const BlobPtr& other) : _wptr(other._wptr), _index(other._index) {}

    // 赋值操作符
    BlobPtr& operator=(const BlobPtr& other) {
        _wptr = other._wptr;
        _index = other._index;
        return *this;
    }

    // 比较操作符
    bool operator==(const BlobPtr& other) const {
        return _wptr.lock() == other._wptr.lock() && _index == other._index;
    }

    bool operator!=(const BlobPtr& other) const {
        return !(*this == other);
    }

    // 解引用操作符
    T& operator*() const {
        auto p = _check_ptr();
        return (*p)[_index];
    }

    T* operator->() const {
        return &**this;
    }

    // 自增操作符
    BlobPtr& operator++() { // 前置++
        ++_index;
        return *this;
    }

    BlobPtr operator++(int) { // 后置++
        BlobPtr ret = *this;
        ++*this;
        return ret;
    }

    // 自减操作符
    BlobPtr& operator--() { // 前置--
        --_index;
        return *this;
    }

    BlobPtr operator--(int) { // 后置--
        BlobPtr ret = *this;
        --*this;
        return ret;
    }
};

#endif // !_BLOB_HPP_