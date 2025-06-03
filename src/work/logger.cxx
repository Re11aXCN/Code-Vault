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