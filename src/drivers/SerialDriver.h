#pragma once

#include <Arduino.h>
#include <HardwareSerial.h>

class SerialDriver {
public:
    SerialDriver();
    ~SerialDriver();
    
    // 初始化串口
    bool init(uint8_t rxPin, uint8_t txPin, uint32_t baudRate = 9600);
    
    // 发送数据
    size_t write(const uint8_t* data, size_t length);
    size_t write(uint8_t byte);
    
    // 接收数据
    int available();
    int read();
    size_t readBytes(uint8_t* buffer, size_t length);
    
    // 清空缓冲区
    void flush();
    
    // 设置超时时间
    void setTimeout(unsigned long timeout);
    
    // 获取串口状态
    bool isInitialized() const { return _initialized; }
    
private:
    HardwareSerial* _serial;
    uint8_t _rxPin;
    uint8_t _txPin;
    bool _initialized;
};