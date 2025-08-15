#pragma once

#include <Arduino.h>
#include "SerialDriver.h"
#include "../common/Config.h"

class ModbusRTUDriver {
public:
    ModbusRTUDriver();
    ~ModbusRTUDriver();
    
    
    // 初始化
    bool begin(uint8_t rxPin = MODBUS_RX_PIN, uint8_t txPin = MODBUS_TX_PIN, uint32_t baudRate = MODBUS_BAUD_RATE, uint8_t slaveAddress = MODBUS_SLAVE_ADDRESS);
    // MODBUS功能码实现
    bool readHoldingRegisters(uint16_t startAddress, uint16_t quantity, uint16_t* values);
    bool writeSingleRegister(uint16_t address, uint16_t value);
    bool writeMultipleRegisters(uint16_t startAddress, uint16_t quantity, const uint16_t* values);
    
    // 配置
    void setSlaveAddress(uint8_t address) { _slaveAddress = address; }
    void setTimeout(uint16_t timeout) { _timeout = timeout; }
    void setRetries(uint8_t retries) { _maxRetries = retries; }
    
    // 错误信息
    uint8_t getLastError() const { return _lastError; }
    String getLastErrorString() const;
    
private:
    // 内部方法
    uint16_t calculateCRC(const uint8_t* data, uint16_t length);
    bool sendFrame(const uint8_t* frame, uint16_t length);
    bool receiveFrame(uint8_t* buffer, uint16_t maxLength, uint16_t& receivedLength);
    bool validateResponse(const uint8_t* response, uint16_t length);
    
    // 成员变量
    SerialDriver _serial;
    uint8_t _slaveAddress;
    uint16_t _timeout;
    uint8_t _maxRetries;
    uint8_t _lastError;
    
    // 错误码定义
    static const uint8_t ERROR_NONE = 0;
    static const uint8_t ERROR_TIMEOUT = 1;
    static const uint8_t ERROR_CRC = 2;
    static const uint8_t ERROR_EXCEPTION = 3;
    static const uint8_t ERROR_INVALID_RESPONSE = 4;
};