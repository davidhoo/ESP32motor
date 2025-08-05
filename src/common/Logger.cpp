#include "Logger.h"
#include <stdarg.h>
#include <stdio.h>

Logger::Logger() : _serial(nullptr), _level(LogLevel::INFO), _startTime(0) {
    _startTime = millis();
}

Logger::~Logger() {
}

Logger& Logger::getInstance() {
    static Logger instance;
    return instance;
}

void Logger::begin(HardwareSerial* serial, LogLevel level) {
    _serial = serial;
    _level = level;
    if (_serial) {
        _serial->begin(115200);
        while (!_serial) {
            ; // 等待串口就绪
        }
    }
}

void Logger::setLevel(LogLevel level) {
    _level = level;
}

LogLevel Logger::getLevel() const {
    return _level;
}

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

void Logger::log(LogLevel level, const char* tag, const char* format, va_list args) {
    if (_serial == nullptr || level < _level) {
        return;
    }

    // 时间戳
    unsigned long currentTime = millis();
    unsigned long seconds = currentTime / 1000;
    unsigned long milliseconds = currentTime % 1000;
    
    // 日志级别前缀
    const char* levelStr = "";
    switch (level) {
        case LogLevel::DEBUG: levelStr = "DEBUG"; break;
        case LogLevel::INFO: levelStr = "INFO"; break;
        case LogLevel::WARN: levelStr = "WARN"; break;
        case LogLevel::ERROR: levelStr = "ERROR"; break;
        default: levelStr = "UNKNOWN"; break;
    }

    // 格式化输出
    char buffer[256];
    vsnprintf(buffer, sizeof(buffer), format, args);

    // 输出日志
    _serial->printf("[%3lu.%03lu] [%s]", seconds, milliseconds, levelStr);
    
    if (tag != nullptr && strlen(tag) > 0) {
        _serial->printf(" [%s]", tag);
    }
    
    _serial->printf(": %s\n", buffer);
}