#ifndef LOGGER_H
#define LOGGER_H

#include <Arduino.h>
#include <Stream.h>

// 日志级别定义
enum class LogLevel {
    DEBUG = 0,
    INFO = 1,
    WARN = 2,
    ERROR = 3,
    NONE = 4
};

class Logger {
public:
    // 获取单例实例
    static Logger& getInstance();
    
    // 初始化日志系统
    void begin(Stream* stream, LogLevel level = LogLevel::INFO);
    
    // 设置日志级别
    void setLevel(LogLevel level);
    
    // 获取当前日志级别
    LogLevel getLevel() const;
    
    // 日志输出函数
    void debug(const char* format, ...);
    void info(const char* format, ...);
    void warn(const char* format, ...);
    void error(const char* format, ...);
    
    // 带标签的日志输出
    void debug(const String& tag, const char* format, ...);
    void info(const String& tag, const char* format, ...);
    void warn(const String& tag, const char* format, ...);
    void error(const String& tag, const char* format, ...);

private:
    Logger();
    ~Logger();
    
    // 禁止拷贝
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 内部日志输出函数
    void log(LogLevel level, const char* tag, const char* format, va_list args);
    
    Stream* _stream;
    LogLevel _level;
    unsigned long _startTime;
};

// 宏定义简化日志调用
#define LOG_DEBUG(fmt, ...) Logger::getInstance().debug(fmt, ##__VA_ARGS__)
#define LOG_INFO(fmt, ...) Logger::getInstance().info(fmt, ##__VA_ARGS__)
#define LOG_WARN(fmt, ...) Logger::getInstance().warn(fmt, ##__VA_ARGS__)
#define LOG_ERROR(fmt, ...) Logger::getInstance().error(fmt, ##__VA_ARGS__)

#define LOG_TAG_DEBUG(tag, fmt, ...) Logger::getInstance().debug(tag, fmt, ##__VA_ARGS__)
#define LOG_TAG_INFO(tag, fmt, ...) Logger::getInstance().info(tag, fmt, ##__VA_ARGS__)
#define LOG_TAG_WARN(tag, fmt, ...) Logger::getInstance().warn(tag, fmt, ##__VA_ARGS__)
#define LOG_TAG_ERROR(tag, fmt, ...) Logger::getInstance().error(tag, fmt, ##__VA_ARGS__)

#endif // LOGGER_H