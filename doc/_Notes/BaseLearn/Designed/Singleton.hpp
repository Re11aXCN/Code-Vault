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

/*
Use Example
#include "singleton.hpp"
#include <iostream>
#include <chrono>
#include <format>
#include <thread>
#include <vector>
#include <string>
#include <string_view>
#include <mutex>
#include <syncstream>
// 定义使用哪种单例模式 (可选LAZY_SINGLETON, EAGER_SINGLETON, STATIC_SINGLETON)
///< 1. 单例创建线程安全解决的是"对象唯一性"，而方法中的锁解决的是"方法调用的并发安全性"
class Logger {
    SINGLETON_CREATE(Logger);

private:
    std::mutex log_mutex_;

public:
    // 使用C++20的std::format格式化日志
    template<typename... Args>
    void log(std::format_string<Args...> fmt, Args&&... args) {
        auto now = std::chrono::system_clock::now();
        auto time_str = std::format("{:%Y-%m-%d %H:%M:%S}", now);
#if __cplusplus >= 202002L
        std::osyncstream { std::cout } << std::format("[{}] [Thread-{}] ",
            time_str,
            std::this_thread::get_id())
            << std::format(fmt, std::forward<Args>(args)...)
            << std::endl;
#else
        std::lock_guard<std::mutex> lock(log_mutex_);
        std::cout << std::format("[{}] [Thread-{}] ",
            time_str,
            std::this_thread::get_id())
            << std::format(fmt, std::forward<Args>(args)...)
            << std::endl;
#endif
    }
};

void thread_func(int id, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        Logger::getInstance().log("Thread {} iteration {}", id, i);
        // 随机休眠一小段时间，模拟工作负载
        //std::this_thread::sleep_for(std::chrono::milliseconds(50 + (id * 10) % 50));
    }
}

int main() {
    std::cout << "单例模式测试开始..." << std::endl;

    // 创建多个线程同时访问Logger单例
    constexpr int num_threads = 5;
    constexpr int iterations_per_thread = 3;

    std::vector<std::thread> threads;
    threads.reserve(num_threads);

    // 启动多个线程
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(thread_func, i, iterations_per_thread);
    }

    // 等待所有线程完成
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }

    std::cout << "单例模式测试完成" << std::endl;
    return 0;
}
*/