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

// 日志配置选项
struct LoggerConfig {
    bool showTimestamp = true;      // 是否显示时间戳
    bool showLevel = true;          // 是否显示日志级别
    bool showTag = true;            // 是否显示标签
    bool useColors = false;         // 是否使用颜色输出
    bool useMilliseconds = true;    // 时间戳是否包含毫秒
    size_t bufferSize = 512;        // 缓冲区大小
    const char* timeFormat = nullptr; // 自定义时间格式
};

class Logger {
public:
    // 获取单例实例
    static Logger& getInstance();
    
    // 初始化日志系统
    void begin(Stream* stream, LogLevel level = LogLevel::INFO);
    void begin(Stream* stream, LogLevel level, const LoggerConfig& config);
    
    // 设置日志级别
    void setLevel(LogLevel level);
    
    // 获取当前日志级别
    LogLevel getLevel() const;
    
    // 设置配置
    void setConfig(const LoggerConfig& config);
    LoggerConfig getConfig() const;
    
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
    
    // 带标签的日志输出（C字符串版本，避免String对象创建）
    void debug(const char* tag, const char* format, ...);
    void info(const char* tag, const char* format, ...);
    void warn(const char* tag, const char* format, ...);
    void error(const char* tag, const char* format, ...);
    
    // 刷新输出缓冲区
    void flush();
    
    // 检查是否启用了指定级别的日志
    bool isLevelEnabled(LogLevel level) const;

private:
    Logger();
    ~Logger();
    
    // 禁止拷贝
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;
    
    // 内部日志输出函数
    void log(LogLevel level, const char* tag, const char* format, va_list args);
    
    // 格式化时间戳
    size_t formatTimestamp(char* buffer, size_t bufferSize);
    
    // 获取日志级别字符串和颜色代码
    const char* getLevelString(LogLevel level);
    const char* getLevelColor(LogLevel level);
    
    // 安全的字符串拷贝
    size_t safeStrCopy(char* dest, const char* src, size_t destSize, size_t& offset);
    
    Stream* _stream;
    LogLevel _level;
    unsigned long _startTime;
    LoggerConfig _config;
    char* _buffer;  // 动态分配的缓冲区
};

// 宏定义简化日志调用
#define LOG_DEBUG(fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::DEBUG)) Logger::getInstance().debug(fmt, ##__VA_ARGS__); } while(0)
#define LOG_INFO(fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::INFO)) Logger::getInstance().info(fmt, ##__VA_ARGS__); } while(0)
#define LOG_WARN(fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::WARN)) Logger::getInstance().warn(fmt, ##__VA_ARGS__); } while(0)
#define LOG_ERROR(fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::ERROR)) Logger::getInstance().error(fmt, ##__VA_ARGS__); } while(0)

#define LOG_TAG_DEBUG(tag, fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::DEBUG)) Logger::getInstance().debug(tag, fmt, ##__VA_ARGS__); } while(0)
#define LOG_TAG_INFO(tag, fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::INFO)) Logger::getInstance().info(tag, fmt, ##__VA_ARGS__); } while(0)
#define LOG_TAG_WARN(tag, fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::WARN)) Logger::getInstance().warn(tag, fmt, ##__VA_ARGS__); } while(0)
#define LOG_TAG_ERROR(tag, fmt, ...) do { if (Logger::getInstance().isLevelEnabled(LogLevel::ERROR)) Logger::getInstance().error(tag, fmt, ##__VA_ARGS__); } while(0)

// 便捷宏，自动使用当前文件名作为标签
#define LOG_D(fmt, ...) LOG_TAG_DEBUG(__FILE__, fmt, ##__VA_ARGS__)
#define LOG_I(fmt, ...) LOG_TAG_INFO(__FILE__, fmt, ##__VA_ARGS__)
#define LOG_W(fmt, ...) LOG_TAG_WARN(__FILE__, fmt, ##__VA_ARGS__)
#define LOG_E(fmt, ...) LOG_TAG_ERROR(__FILE__, fmt, ##__VA_ARGS__)

#endif // LOGGER_H