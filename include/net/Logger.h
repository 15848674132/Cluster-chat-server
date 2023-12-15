#ifndef LOGGER_H
#define LOGGER_H

#include <string>
#include "noncopyable.h"
#include "Timestamp.h"
// 定义日志级别 INFO DEBUG ERROR FATAL

enum class LogLevel
{
    INFO,  // 普通信息
    ERROR, // 错误信息
    FATAL, // core信息
    DEBUG  // 调试信息
};

#define LOG_INFO(LogmsgFormat, ...)                       \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::getInstance();           \
        logger.setLogLevel(LogLevel::INFO);               \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_ERROR(LogmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::getInstance();           \
        logger.setLogLevel(LogLevel::ERROR);              \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)

#define LOG_FATAL(LogmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::getInstance();           \
        logger.setLogLevel(LogLevel::FATAL);              \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
        exit(-1);                                         \
    } while (0)

#ifdef MUDEBUG
#define LOG_DEBUG(LogmsgFormat, ...)                      \
    do                                                    \
    {                                                     \
        Logger &logger = Logger::getInstance();           \
        logger.setLogLevel(LogLevel::DEBUG);              \
        char buf[1024] = {0};                             \
        snprintf(buf, 1024, LogmsgFormat, ##__VA_ARGS__); \
        logger.log(buf);                                  \
    } while (0)
#else
#define LOG_DEBUG(LogmsgFormat, ...)
#endif
class Logger : noncopyable
{
public:
    // 获取日志唯一的实例对象
    static Logger &getInstance();
    // 设置日志级别
    void setLogLevel(LogLevel level);
    // 写日志
    void log(std::string msg);

private:
    Logger() {}

private:
    LogLevel logLevel_;
};

#endif