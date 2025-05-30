#ifndef _SINGLETON_HPP_
#define _SINGLETON_HPP_
#include <mutex>
#include <memory>
#include <atomic>

template<typename T>
class Singleton {
private:
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
    Singleton(Singleton&&) = delete;
    Singleton& operator=(Singleton&&) = delete;

public:
    static T& getInstance() {
        static T instance;
        return instance;
    }

protected:
    // 构造函数和析构函数都设置为protected，禁止外部直接创建实例
    Singleton() = default;
    ~Singleton() = default;
};

#define SINGLETON_CREATE(Class)                 \
private:                                        \
    friend class Singleton<Class>;              \
                                                \
public:                                         \
    static Class& getInstance()                 \
    {                                           \
        return Singleton<Class>::getInstance(); \
    }

#define SINGLETON_CREATE_H(Class)                   \
private:                                            \
    static std::unique_ptr<Class> _instance;        \
    static std::once_flag _flag;                    \
    friend std::default_delete<Class>;              \
                                                    \
public:                                             \
    static Class& getInstance();

#define SINGLETON_CREATE_CPP(Class)                              \
    std::unique_ptr<Class> Class::_instance = nullptr;           \
    std::once_flag Class::_flag;                                 \
    Class& Class::getInstance() {                                \
        std::call_once(_flag, [&] { _instance.reset(new Class); }); \
        return *_instance;                                       \
    }

#endif // !_SINGLETON_HPP_
