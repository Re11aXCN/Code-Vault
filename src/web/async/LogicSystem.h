#ifndef LOGICSYSTEM_H
#define LOGICSYSTEM_H

#include "../../designed/singleton.hpp"
#include "MessagePackage.h"
#include <functional>
#include <unordered_map>
#include <mutex>
#include <queue>
#include <condition_variable>
#include <thread>
#include <atomic>

// 前向声明
class AsyncSession;

/**
 * @brief 逻辑系统类，处理业务逻辑
 * 使用单例模式确保全局唯一实例
 */
class LogicSystem {
    SINGLETON_CREATE(LogicSystem)

public:
    // 消息处理函数类型定义
    using MessageHandler = std::function<void(std::shared_ptr<AsyncSession>, const MessagePackage&)>;
    
    /**
     * @brief 初始化逻辑系统
     * @param threadCount 工作线程数量
     */
    void Initialize(size_t threadCount = 1) {
        if (_isRunning) return;
        
        _isRunning = true;
        
        // 创建工作线程
        for (size_t i = 0; i < threadCount; ++i) {
            _workers.emplace_back([this]() {
                _workerThread();
            });
        }
    }
    
    /**
     * @brief 停止逻辑系统
     */
    void Stop() {
        if (!_isRunning) return;
        
        // 设置停止标志
        _isRunning = false;
        
        // 唤醒所有等待的工作线程
        _condition.notify_all();
        
        // 等待所有工作线程结束
        for (auto& worker : _workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
        
        // 清空任务队列
        std::lock_guard<std::mutex> lock(_queueMutex);
        while (!_taskQueue.empty()) {
            _taskQueue.pop();
        }
    }
    
    /**
     * @brief 注册消息处理函数
     * @param messageType 消息类型
     * @param handler 处理函数
     */
    void RegisterHandler(uint16_t messageType, MessageHandler handler) {
        std::lock_guard<std::mutex> lock(_handlersMutex);
        _handlers[messageType] = std::move(handler);
    }
    
    /**
     * @brief 提交消息处理任务
     * @param session 会话指针
     * @param message 消息包
     */
    void SubmitMessage(std::shared_ptr<AsyncSession> session, const MessagePackage& message) {
        if (!_isRunning) return;
        
        // 创建任务并加入队列
        std::lock_guard<std::mutex> lock(_queueMutex);
        _taskQueue.emplace([this, session, message]() {
            _processMessage(session, message);
        });
        
        // 通知一个工作线程处理任务
        _condition.notify_one();
    }
    /**
     * @brief 检查逻辑系统是否正在运行
     * @return 运行状态
     */
    bool IsRunning() const {
        return _isRunning;
    }
    /**
     * @brief 析构函数，确保资源释放
     */
    ~LogicSystem() {
        Stop();
    }
    
private:
    // 私有构造函数，防止外部创建实例
    LogicSystem() : _isRunning(false) {}
    
    /**
     * @brief 工作线程函数
     */
    void _workerThread() {
        while (_isRunning) {
            std::function<void()> task;
            
            // 等待并获取任务
            {
                std::unique_lock<std::mutex> lock(_queueMutex);
                _condition.wait(lock, [this]() {
                    return !_isRunning || !_taskQueue.empty();
                });
                
                if (!_isRunning && _taskQueue.empty()) {
                    return; // 系统停止且队列为空，退出线程
                }
                
                task = std::move(_taskQueue.front());
                _taskQueue.pop();
            }
            
            // 执行任务
            try {
                task();
            } catch (const std::exception& e) {
                std::cerr << "Logic system exception: " << e.what() << std::endl;
            }
        }
    }
    
    /**
     * @brief 处理消息
     * @param session 会话指针
     * @param message 消息包
     */
    void _processMessage(std::shared_ptr<AsyncSession> session, const MessagePackage& message) {
        // 查找对应的消息处理函数
        MessageHandler handler;
        {
            std::lock_guard<std::mutex> lock(_handlersMutex);
            auto it = _handlers.find(message.GetMessageType());
            if (it == _handlers.end()) {
                std::cerr << "No handler for message type: " << message.GetMessageType() << std::endl;
                return;
            }
            handler = it->second;
        }
        
        // 调用处理函数
        try {
            handler(session, message);
        } catch (const std::exception& e) {
            std::cerr << "Message handler exception: " << e.what() << std::endl;
        }
    }
    
    std::atomic<bool> _isRunning;                                 // 运行状态标志
    std::vector<std::thread> _workers;                           // 工作线程数组
    std::queue<std::function<void()>> _taskQueue;                // 任务队列
    std::mutex _queueMutex;                                      // 队列互斥锁
    std::condition_variable _condition;                          // 条件变量，用于线程同步
    std::unordered_map<uint16_t, MessageHandler> _handlers;      // 消息处理函数映射表
    std::mutex _handlersMutex;                                   // 处理函数映射表互斥锁
};

#endif //!LOGICSYSTEM_H