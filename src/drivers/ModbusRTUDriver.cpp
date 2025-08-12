#include "ModbusRTUDriver.h"

ModbusRTUDriver::ModbusRTUDriver() 
    : _slaveAddress(0x01), _timeout(100), _maxRetries(3), _lastError(ERROR_NONE) {
}

ModbusRTUDriver::~ModbusRTUDriver() {
}

bool ModbusRTUDriver::begin(uint8_t rxPin, uint8_t txPin, uint32_t baudRate, uint8_t slaveAddress) {
    _slaveAddress = slaveAddress;
    
    if (!_serial.init(rxPin, txPin, baudRate)) {
        return false;
    }
    
    _serial.setTimeout(_timeout);
    return true;
}

bool ModbusRTUDriver::readHoldingRegisters(uint16_t startAddress, uint16_t quantity, uint16_t* values) {
    if (quantity == 0 || quantity > 125) {
        _lastError = ERROR_INVALID_RESPONSE;
        return false;
    }
    
    uint8_t request[8];
    request[0] = _slaveAddress;
    request[1] = 0x03;
    request[2] = (startAddress >> 8) & 0xFF;
    request[3] = startAddress & 0xFF;
    request[4] = (quantity >> 8) & 0xFF;
    request[5] = quantity & 0xFF;
    
    uint16_t crc = calculateCRC(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    for (uint8_t retry = 0; retry <= _maxRetries; retry++) {
        if (sendFrame(request, 8)) {
            uint8_t response[256];
            uint16_t responseLength = 0;
            
            if (receiveFrame(response, sizeof(response), responseLength)) {
                if (responseLength >= 5 && response[0] == _slaveAddress && response[1] == 0x03) {
                    uint8_t byteCount = response[2];
                    if (byteCount == quantity * 2 && responseLength >= 5 + byteCount) {
                        for (uint16_t i = 0; i < quantity; i++) {
                            values[i] = (response[3 + i * 2] << 8) | response[4 + i * 2];
                        }
                        _lastError = ERROR_NONE;
                        return true;
                    }
                }
            }
        }
        
        if (retry < _maxRetries) {
            delay(10);
        }
    }
    
    _lastError = ERROR_TIMEOUT;
    return false;
}

bool ModbusRTUDriver::writeSingleRegister(uint16_t address, uint16_t value) {
    uint8_t request[8];
    request[0] = _slaveAddress;
    request[1] = 0x06;
    request[2] = (address >> 8) & 0xFF;
    request[3] = address & 0xFF;
    request[4] = (value >> 8) & 0xFF;
    request[5] = value & 0xFF;
    
    uint16_t crc = calculateCRC(request, 6);
    request[6] = crc & 0xFF;
    request[7] = (crc >> 8) & 0xFF;
    
    for (uint8_t retry = 0; retry <= _maxRetries; retry++) {
        if (sendFrame(request, 8)) {
            uint8_t response[8];
            uint16_t responseLength = 0;
            
            if (receiveFrame(response, sizeof(response), responseLength)) {
                if (responseLength == 8 && memcmp(request, response, 8) == 0) {
                    _lastError = ERROR_NONE;
                    return true;
                }
            }
        }
        
        if (retry < _maxRetries) {
            delay(10);
        }
    }
    
    _lastError = ERROR_TIMEOUT;
    return false;
}

bool ModbusRTUDriver::writeMultipleRegisters(uint16_t startAddress, uint16_t quantity, const uint16_t* values) {
    if (quantity == 0 || quantity > 123) {
        _lastError = ERROR_INVALID_RESPONSE;
        return false;
    }
    
    uint8_t request[256];
    request[0] = _slaveAddress;
    request[1] = 0x10;
    request[2] = (startAddress >> 8) & 0xFF;
    request[3] = startAddress & 0xFF;
    request[4] = (quantity >> 8) & 0xFF;
    request[5] = quantity & 0xFF;
    request[6] = quantity * 2;
    
    for (uint16_t i = 0; i < quantity; i++) {
        request[7 + i * 2] = (values[i] >> 8) & 0xFF;
        request[8 + i * 2] = values[i] & 0xFF;
    }
    
    uint16_t crc = calculateCRC(request, 7 + quantity * 2);
    request[7 + quantity * 2] = crc & 0xFF;
    request[8 + quantity * 2] = (crc >> 8) & 0xFF;
    
    for (uint8_t retry = 0; retry <= _maxRetries; retry++) {
        if (sendFrame(request, 9 + quantity * 2)) {
            uint8_t response[8];
            uint16_t responseLength = 0;
            
            if (receiveFrame(response, sizeof(response), responseLength)) {
                if (responseLength == 8 && response[0] == _slaveAddress && response[1] == 0x10) {
                    uint16_t respAddress = (response[2] << 8) | response[3];
                    uint16_t respQuantity = (response[4] << 8) | response[5];
                    
                    if (respAddress == startAddress && respQuantity == quantity) {
                        _lastError = ERROR_NONE;
                        return true;
                    }
                }
            }
        }
        
        if (retry < _maxRetries) {
            delay(10);
        }
    }
    
    _lastError = ERROR_TIMEOUT;
    return false;
}

uint16_t ModbusRTUDriver::calculateCRC(const uint8_t* data, uint16_t length) {
    uint16_t crc = 0xFFFF;
    
    for (uint16_t i = 0; i < length; i++) {
        crc ^= data[i];
        
        for (uint8_t j = 0; j < 8; j++) {
            if (crc & 0x0001) {
                crc = (crc >> 1) ^ 0xA001;
            } else {
                crc = crc >> 1;
            }
        }
    }
    
    return crc;
}

bool ModbusRTUDriver::sendFrame(const uint8_t* frame, uint16_t length) {
    return _serial.write(frame, length) == length;
}

bool ModbusRTUDriver::receiveFrame(uint8_t* buffer, uint16_t maxLength, uint16_t& receivedLength) {
    unsigned long startTime = millis();
    
    while (millis() - startTime < _timeout) {
        if (_serial.available() > 0) {
            receivedLength = _serial.readBytes(buffer, maxLength);
            return receivedLength > 0;
        }
        delay(1);
    }
    
    return false;
}

String ModbusRTUDriver::getLastErrorString() const {
    switch (_lastError) {
        case ERROR_NONE: return "No error";
        case ERROR_TIMEOUT: return "Timeout";
        case ERROR_CRC: return "CRC error";
        case ERROR_EXCEPTION: return "Exception response";
        case ERROR_INVALID_RESPONSE: return "Invalid response";
        default: return "Unknown error";
    }
}