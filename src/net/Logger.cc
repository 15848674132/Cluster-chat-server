#include "Logger.h"
#include <iostream>
// 获取日志唯一的实例对象
Logger &Logger::getInstance()
{
    static Logger logger;
    return logger;
}
// 设置日志级别
void Logger::setLogLevel(LogLevel level)
{
    logLevel_ = level;
}
// 写日志
void Logger::log(std::string msg)
{
    switch (logLevel_)
    {
    case LogLevel::INFO:
        std::cout << "[INFO]";
        break;
    case LogLevel::ERROR:
        std::cout << "[ERROR]";
        break;
    case LogLevel::FATAL:
        std::cout << "[FATAL]";
        break;
    case LogLevel::DEBUG:
        std::cout << "[DEBUG]";
        break;
    default:
        break;
    }

    // 打印时间和msg
    std::cout << Timestamp::now().toString() << " : " << msg << std::endl;

}