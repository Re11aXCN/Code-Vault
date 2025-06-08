#ifndef THREADPOOL_H
#define THREADPOOL_H

#include "../../designed/singleton.hpp"
#include <boost/asio.hpp>
#include <thread>
#include <vector>
#include <memory>
#include <functional>
#include <atomic>
#include <iostream> // for std::cerr

/**
 * @brief 线程池类，管理多个工作线程，每个线程运行一个io_context
 * 使用单例模式确保全局唯一实例
 */
class ThreadPool {
    SINGLETON_CREATE(ThreadPool)

public:
    // 使用别名简化工作守卫类型
    using work_guard_type = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;

    /**
     * @brief 初始化线程池
     * @param threadCount 线程数量，默认为系统核心数
     */
    void Initialize(size_t threadCount = std::thread::hardware_concurrency()) {
        if (_isRunning) return;

        _isRunning = true;
        _ioContexts.resize(threadCount);
        _ioWorks.clear(); // 清空工作守卫
        _ioWorks.reserve(threadCount); // 预留空间
        _threads.resize(threadCount);

        // 为每个io_context创建工作守卫和线程
        for (size_t i = 0; i < threadCount; ++i) {
            // 创建executor_work_guard工作守卫
            _ioWorks.emplace_back(boost::asio::make_work_guard(_ioContexts[i]));

            // 创建线程运行io_context
            _threads[i] = std::make_unique<std::thread>([this, i]() {
                try {
                    std::string threadName = "IoThread-" + std::to_string(i);
                    // 设置线程名称（仅在支持的平台上有效）
#if defined(__linux__)
                    pthread_setname_np(pthread_self(), threadName.c_str());
#elif defined(_WIN32)
// Windows线程命名需要特殊处理
                    const DWORD MS_VC_EXCEPTION = 0x406D1388;
#pragma pack(push,8)
                    struct THREADNAME_INFO {
                        DWORD dwType;     // Must be 0x1000.
                        LPCSTR szName;     // Pointer to name (in user addr space).
                        DWORD dwThreadID;  // Thread ID (-1=caller thread).
                        DWORD dwFlags;     // Reserved for future use, must be zero.
                    };
#pragma pack(pop)
                    THREADNAME_INFO info;
                    info.dwType = 0x1000;
                    info.szName = threadName.c_str();
                    info.dwThreadID = GetThreadId(GetCurrentThread());
                    info.dwFlags = 0;
                    __try {
                        RaiseException(MS_VC_EXCEPTION, 0, sizeof(info) / sizeof(ULONG_PTR), (ULONG_PTR*)&info);
                    }
                    __except (EXCEPTION_EXECUTE_HANDLER) {
                    }
#endif

                    _ioContexts[i].run();
                }
                catch (const std::exception& e) {
                    std::cerr << "Thread pool exception: " << e.what() << std::endl;
                }
                });
        }
    }

    /**
     * @brief 停止线程池
     */
    void Stop() {
        if (!_isRunning) return;

        // 移除工作守卫，允许io_context退出
        for (auto& guard : _ioWorks) {
            guard.reset(); // 释放工作守卫
        }

        // 停止所有io_context
        for (auto& io : _ioContexts) {
            io.stop();
        }

        // 等待所有线程结束
        for (auto& thread : _threads) {
            if (thread && thread->joinable()) {
                thread->join();
            }
        }

        // 清理资源
        _threads.clear();
        _ioWorks.clear();
        _ioContexts.clear();
        _isRunning = false;
    }

    /**
     * @brief 获取下一个可用的io_context，使用轮询方式实现负载均衡
     * @return io_context引用
     */
    boost::asio::io_context& GetNextContext() {
        // 使用原子操作实现线程安全的轮询
        size_t current = _nextIndex.fetch_add(1, std::memory_order_relaxed);
        return _ioContexts[current % _ioContexts.size()];
    }

    /**
     * @brief 获取指定索引的io_context
     * @param index io_context的索引
     * @return io_context引用
     */
    boost::asio::io_context& GetContext(size_t index) {
        return _ioContexts[index % _ioContexts.size()];
    }

    /**
     * @brief 获取线程池大小
     * @return 线程数量
     */
    size_t Size() const {
        return _threads.size();
    }

    /**
     * @brief 检查线程池是否正在运行
     * @return 运行状态
     */
    bool IsRunning() const {
        return _isRunning;
    }

    /**
     * @brief 析构函数，确保资源释放
     */
    ~ThreadPool() {
        Stop();
    }

private:
    // 私有构造函数，防止外部创建实例
    ThreadPool() : _isRunning(false), _nextIndex(0) {}

    std::vector<boost::asio::io_context> _ioContexts;                  // IO上下文数组
    std::vector<work_guard_type> _ioWorks;                            // 工作守卫数组
    std::vector<std::unique_ptr<std::thread>> _threads;                // 工作线程数组
    std::atomic<bool> _isRunning;                                     // 运行状态标志
    std::atomic<size_t> _nextIndex;                                   // 下一个要使用的io_context索引
};

#endif //!THREADPOOL_H