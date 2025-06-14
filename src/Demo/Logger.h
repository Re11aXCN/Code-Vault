#ifndef _LOGQUEUE_H_
#define _LOGQUEUE_H_

#include <queue>
#include <mutex>
#include <string>
#include <format>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
class LogQueue
{
public:
    void push(const std::string& log) {
        std::lock_guard<std::mutex> lock(_mutex);
        _queue.push(log);
        _cv.notify_one(); // 唤醒消费者消费
    }
    bool pop(std::string& log) {
        std::unique_lock<std::mutex> lock(_mutex);
        // 操作系统可能虚假唤醒，注意不是由主线程notify_one唤醒的
        // 造成问题是如果消费者消费，那么队列为空的时候会一直阻塞
        // 所以通过队列是否为空，来是否挂起条件变量
        // 如果lambda返回false，那么线程先挂起后lock将被解锁（将_mutex给生产者使用），wait内部集成的
        // 如果lambda返回true，那么锁始终是消费者持有，直到消费结束
        _cv.wait(lock, [this] { return!_queue.empty() || _closed; });
        if (_closed && _queue.empty())  return false; // 队列已关闭且为空，则返回false
        
        log = _queue.front();
        _queue.pop();
        return true;
    }
    void close() {
        std::lock_guard<std::mutex> lock(_mutex);
        _closed = true;
        _cv.notify_all(); // 唤醒所有消费者消费
    }
private:
    std::queue<std::string> _queue;
    std::mutex _mutex;
    std::condition_variable _cv;
    std::atomic<bool> _closed{ false };
};

class Logger {
public:
    enum Level {
        Debug,
        Info,
        Warn,
        Error,
        Fatal
    };
    explicit Logger(const std::string& filename)
        : _file(filename.data(), std::ios::app | std::ios::out)
    {
        if (!_file.is_open()) {
            throw std::runtime_error("Failed to open log file: " + filename);
        }
        _worker = std::thread([this]() -> void {
            std::string log;
            // 持续处理日志直到队列关闭且为空
            while (true) {
                if (!_queue.pop(log)) // 当pop返回false时退出
                    break;

                // 获取当前时间
                auto now = std::chrono::system_clock::now();
                auto in_time_t = std::chrono::system_clock::to_time_t(now);

                // 获取毫秒部分
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
                    now.time_since_epoch()) % 1000;

                // 格式化时间字符串,使用std::put_time替代std::ctime（线程安全）
                std::stringstream ss;
                ss << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X")
                    << '.' << std::setfill('0') << std::setw(3) << ms.count();

                // 写入日志文件
                _file << ss.str() << " " << log << std::endl;
                _file.flush(); // 确保日志立即写入
            }
            });
    }
    ~Logger() {
        // 关闭队列并等待消费者线程完成
        _queue.close();
        if (_worker.joinable()) {
            _worker.join();
        }
        if (_file.is_open()) {
            _file.close();
        }
    };

    template<typename... Args>
    void log(Level level, std::format_string<Args...> fmt, Args&&... args) {
        std::string prefix;
        switch (level) {
        case Debug: prefix = "[DEBUG]"; break;
        case Info:  prefix = "[INFO]";  break;
        case Warn:  prefix = "[WARN]";  break;
        case Error: prefix = "[ERROR]"; break;
        case Fatal: prefix = "[FATAL]"; break;
        }

        // 格式化日志消息并推入队列
        _queue.push(prefix + " " + std::format(fmt, std::forward<Args>(args)...));
    }

private:
    LogQueue _queue;
    std::thread _worker;
    std::ofstream _file;
};
/*
use Example
#include "logger.h"

int main()
{
    Logger logger("cclog.txt");
    logger.log(Logger::Debug, "Hello, {}! I'm a {} logger.", "world", "C++");
    logger.log(Logger::Info, "The answer is {}", 42);
    logger.log(Logger::Warn, "This is a warning");
    logger.log(Logger::Error, "Division by {} occurred", "zero");
    logger.log(Logger::Fatal, "Critical system failure");
    
    // 添加短暂延时确保日志写入完成
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return 0;
}
*/
#endif // !
