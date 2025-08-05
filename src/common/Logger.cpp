#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>

// ANSI颜色代码
#define ANSI_COLOR_RESET   "\033[0m"
#define ANSI_COLOR_BLACK   "\033[30m"
#define ANSI_COLOR_RED     "\033[31m"
#define ANSI_COLOR_GREEN   "\033[32m"
#define ANSI_COLOR_YELLOW  "\033[33m"
#define ANSI_COLOR_BLUE    "\033[34m"
#define ANSI_COLOR_MAGENTA "\033[35m"
#define ANSI_COLOR_CYAN    "\033[36m"
#define ANSI_COLOR_WHITE   "\033[37m"

Logger::Logger() : _stream(nullptr), _level(LogLevel::INFO), _startTime(0), _buffer(nullptr) {
    _startTime = millis();
    // 使用默认配置
    _buffer = new char[_config.bufferSize];
}

Logger::~Logger() {
    if (_buffer) {
        delete[] _buffer;
        _buffer = nullptr;
    }
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::begin(Stream* stream, LogLevel level) {
    _stream = stream;
    _level = level;
    // Stream 接口不需要 begin() 调用，由调用者负责初始化
}

void Logger::begin(Stream* stream, LogLevel level, const LoggerConfig& config) {
    _stream = stream;
    _level = level;
    setConfig(config);
}

void Logger::setLevel(LogLevel level) {
    _level = level;
}

LogLevel Logger::getLevel() const {
    return _level;
}

void Logger::setConfig(const LoggerConfig& config) {
    // 如果缓冲区大小改变，重新分配内存
    if (config.bufferSize != _config.bufferSize) {
        if (_buffer) {
            delete[] _buffer;
        }
        _buffer = new char[config.bufferSize];
    }
    _config = config;
}

LoggerConfig Logger::getConfig() const {
    return _config;
}

bool Logger::isLevelEnabled(LogLevel level) const {
    return _stream != nullptr && level >= _level;
}

void Logger::flush() {
    if (_stream) {
        _stream->flush();
    }
}

// 无标签版本的日志函数
void Logger::debug(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::DEBUG, nullptr, format, args);
    va_end(args);
}

void Logger::info(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::INFO, nullptr, format, args);
    va_end(args);
}

void Logger::warn(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::WARN, nullptr, format, args);
    va_end(args);
}

void Logger::error(const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::ERROR, nullptr, format, args);
    va_end(args);
}

// String标签版本的日志函数
void Logger::debug(const String& tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::DEBUG, tag.c_str(), format, args);
    va_end(args);
}

void Logger::info(const String& tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::INFO, tag.c_str(), format, args);
    va_end(args);
}

void Logger::warn(const String& tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::WARN, tag.c_str(), format, args);
    va_end(args);
}

void Logger::error(const String& tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::ERROR, tag.c_str(), format, args);
    va_end(args);
}

// C字符串标签版本的日志函数
void Logger::debug(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::DEBUG, tag, format, args);
    va_end(args);
}

void Logger::info(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::INFO, tag, format, args);
    va_end(args);
}

void Logger::warn(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::WARN, tag, format, args);
    va_end(args);
}

void Logger::error(const char* tag, const char* format, ...) {
    va_list args;
    va_start(args, format);
    log(LogLevel::ERROR, tag, format, args);
    va_end(args);
}

void Logger::log(LogLevel level, const char* tag, const char* format, va_list args) {
    if (!isLevelEnabled(level) || !_buffer) {
        return;
    }

    size_t offset = 0;
    
    // 添加颜色代码（如果启用）
    if (_config.useColors) {
        const char* color = getLevelColor(level);
        safeStrCopy(_buffer, color, _config.bufferSize, offset);
    }
    
    // 添加时间戳
    if (_config.showTimestamp) {
        offset += formatTimestamp(_buffer + offset, _config.bufferSize - offset);
        safeStrCopy(_buffer, " ", _config.bufferSize, offset);
    }
    
    // 添加日志级别
    if (_config.showLevel) {
        safeStrCopy(_buffer, "[", _config.bufferSize, offset);
        safeStrCopy(_buffer, getLevelString(level), _config.bufferSize, offset);
        safeStrCopy(_buffer, "]", _config.bufferSize, offset);
        if (_config.showTag || tag) {
            safeStrCopy(_buffer, " ", _config.bufferSize, offset);
        }
    }
    
    // 添加标签
    if (_config.showTag && tag && strlen(tag) > 0) {
        safeStrCopy(_buffer, "[", _config.bufferSize, offset);
        safeStrCopy(_buffer, tag, _config.bufferSize, offset);
        safeStrCopy(_buffer, "]", _config.bufferSize, offset);
    }
    
    // 添加分隔符
    if (offset > 0) {
        safeStrCopy(_buffer, ": ", _config.bufferSize, offset);
    }
    
    // 格式化用户消息
    if (offset < _config.bufferSize - 1) {
        int written = vsnprintf(_buffer + offset, _config.bufferSize - offset, format, args);
        if (written > 0) {
            offset += min((size_t)written, _config.bufferSize - offset - 1);
        }
    }
    
    // 添加换行符
    if (offset < _config.bufferSize - 1) {
        _buffer[offset++] = '\n';
        _buffer[offset] = '\0';
    }
    
    // 添加颜色重置代码（如果启用）
    if (_config.useColors && offset < _config.bufferSize - strlen(ANSI_COLOR_RESET)) {
        strcat(_buffer + offset - 1, ANSI_COLOR_RESET);  // 覆盖换行符
        strcat(_buffer, "\n");  // 重新添加换行符
    }
    
    // 输出到流
    _stream->print(_buffer);
}

size_t Logger::formatTimestamp(char* buffer, size_t bufferSize) {
    if (bufferSize < 16) return 0;  // 最小缓冲区大小检查
    
    unsigned long currentTime = millis();
    
    if (_config.useMilliseconds) {
        unsigned long seconds = currentTime / 1000;
        unsigned long milliseconds = currentTime % 1000;
        return snprintf(buffer, bufferSize, "[%3lu.%03lu]", seconds, milliseconds);
    } else {
        unsigned long seconds = currentTime / 1000;
        return snprintf(buffer, bufferSize, "[%lu]", seconds);
    }
}

const char* Logger::getLevelString(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return "DEBUG";
        case LogLevel::INFO:  return "INFO";
        case LogLevel::WARN:  return "WARN";
        case LogLevel::ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

const char* Logger::getLevelColor(LogLevel level) {
    switch (level) {
        case LogLevel::DEBUG: return ANSI_COLOR_CYAN;
        case LogLevel::INFO:  return ANSI_COLOR_GREEN;
        case LogLevel::WARN:  return ANSI_COLOR_YELLOW;
        case LogLevel::ERROR: return ANSI_COLOR_RED;
        default: return ANSI_COLOR_WHITE;
    }
}

size_t Logger::safeStrCopy(char* dest, const char* src, size_t destSize, size_t& offset) {
    if (!src || offset >= destSize - 1) return 0;
    
    size_t srcLen = strlen(src);
    size_t availableSpace = destSize - offset - 1;  // 保留空间给null终止符
    size_t copyLen = min(srcLen, availableSpace);
    
    if (copyLen > 0) {
        memcpy(dest + offset, src, copyLen);
        offset += copyLen;
        dest[offset] = '\0';
    }
    
    return copyLen;
}