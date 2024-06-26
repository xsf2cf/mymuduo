#pragma once

#include "noncopyable.h"

#include<string>

/**
 * 提供格式化的日志输出方式
 * 使用do while 确保宏属于一个完整的单一语句，不会在特定场景下破坏代码
*/
#define LOG_INFO(logMsg, ...) \
    do \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(INFO); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logMsg, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_ERROR(logMsg, ...) \
    do \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(ERROR); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logMsg, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_FATAL(logMsg, ...) \
    do \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(FATAL); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logMsg, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)

#define LOG_DEBUG(logMsg, ...) \
    do \
    { \
        Logger& logger = Logger::instance(); \
        logger.setLogLevel(DEBUG); \
        char buf[1024] = {0}; \
        snprintf(buf, 1024, logMsg, ##__VA_ARGS__); \
        logger.log(buf); \
    } while(0)


enum LogLevel
{
    INFO,
    ERROR,
    FATAL,
    DEBUG,
};

class Logger : noncopyable
{
public:
    //单例模式，获取唯一的实例对象
    static Logger& instance();

    void setLogLevel(int level);

    void log(std::string msg);

private:
    //Logger();
    int logLevel_;
};