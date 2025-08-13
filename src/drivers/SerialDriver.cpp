#include "SerialDriver.h"

SerialDriver::SerialDriver() : _serial(nullptr), _rxPin(0), _txPin(0), _initialized(false) {
}

SerialDriver::~SerialDriver() {
    if (_serial) {
        _serial->end();
    }
}

bool SerialDriver::init(uint8_t rxPin, uint8_t txPin, uint32_t baudRate) {
    _rxPin = rxPin;
    _txPin = txPin;
    
    // 使用UART2 (GPIO8=RX, GPIO9=TX)
    _serial = &Serial2;
    _serial->begin(baudRate, SERIAL_8N1, rxPin, txPin);
    
    _initialized = true;
    return true;
}

size_t SerialDriver::write(const uint8_t* data, size_t length) {
    if (!_initialized) return 0;
    return _serial->write(data, length);
}

size_t SerialDriver::write(uint8_t byte) {
    if (!_initialized) return 0;
    return _serial->write(byte);
}

int SerialDriver::available() {
    if (!_initialized) return 0;
    return _serial->available();
}

int SerialDriver::read() {
    if (!_initialized) return -1;
    return _serial->read();
}

size_t SerialDriver::readBytes(uint8_t* buffer, size_t length) {
    if (!_initialized) return 0;
    return _serial->readBytes(buffer, length);
}

void SerialDriver::flush() {
    if (!_initialized) return;
    _serial->flush();
}

void SerialDriver::setTimeout(unsigned long timeout) {
    if (!_initialized) return;
    _serial->setTimeout(timeout);
}