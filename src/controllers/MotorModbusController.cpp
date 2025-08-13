#include "MotorModbusController.h"

MotorModbusController::MotorModbusController() : _motorAddress(0x01) {
}

MotorModbusController::~MotorModbusController() {
}

bool MotorModbusController::begin(uint8_t motorAddress) {
    _motorAddress = motorAddress;
    return _modbus.begin(8, 9, 9600, motorAddress);
}

bool MotorModbusController::getConfig(MotorConfig& config) {
    uint16_t values[8];
    
    // 读取所有配置寄存器
    if (!_modbus.readHoldingRegisters(REG_MODULE_ADDRESS, 8, values)) {
        return false;
    }
    
    config.moduleAddress = values[0];
    config.externalSwitch = (values[1] == 1);
    config.analogControl = (values[2] == 1);
    config.powerOnState = (values[3] == 1);
    config.minOutput = values[4];
    config.maxOutput = values[5];
    config.softStartTime = values[6];
    config.softStopTime = values[7];
    
    return true;
}

bool MotorModbusController::getModuleAddress(uint8_t& address) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_MODULE_ADDRESS, 1, &value)) {
        address = (uint8_t)value;
        return true;
    }
    return false;
}

bool MotorModbusController::getExternalSwitch(bool& enabled) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_EXTERNAL_SWITCH, 1, &value)) {
        enabled = (value == 1);
        return true;
    }
    return false;
}

bool MotorModbusController::getAnalogControl(bool& enabled) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_0_10V_CONTROL, 1, &value)) {
        enabled = (value == 1);
        return true;
    }
    return false;
}

bool MotorModbusController::getPowerOnState(bool& state) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_POWER_ON_STATE, 1, &value)) {
        state = (value == 1);
        return true;
    }
    return false;
}

bool MotorModbusController::getOutputLimits(uint8_t& minOutput, uint8_t& maxOutput) {
    uint16_t values[2];
    if (_modbus.readHoldingRegisters(REG_MIN_OUTPUT, 2, values)) {
        minOutput = (uint8_t)values[0];
        maxOutput = (uint8_t)values[1];
        return true;
    }
    return false;
}

bool MotorModbusController::getSoftTimes(uint16_t& startTime, uint16_t& stopTime) {
    uint16_t values[2];
    if (_modbus.readHoldingRegisters(REG_SOFT_START_TIME, 2, values)) {
        startTime = values[0];
        stopTime = values[1];
        return true;
    }
    return false;
}

bool MotorModbusController::setConfig(const MotorConfig& config) {
    uint16_t values[8] = {
        config.moduleAddress,
        config.externalSwitch ? 1 : 0,
        config.analogControl ? 1 : 0,
        config.powerOnState ? 1 : 0,
        config.minOutput,
        config.maxOutput,
        config.softStartTime,
        config.softStopTime
    };
    
    return _modbus.writeMultipleRegisters(REG_MODULE_ADDRESS, 8, values);
}

bool MotorModbusController::setModuleAddress(uint8_t address) {
    if (address > 255) address = 255;
    return _modbus.writeSingleRegister(REG_MODULE_ADDRESS, address);
}

bool MotorModbusController::setExternalSwitch(bool enabled) {
    return _modbus.writeSingleRegister(REG_EXTERNAL_SWITCH, enabled ? 1 : 0);
}

bool MotorModbusController::setAnalogControl(bool enabled) {
    return _modbus.writeSingleRegister(REG_0_10V_CONTROL, enabled ? 1 : 0);
}

bool MotorModbusController::setPowerOnState(bool state) {
    return _modbus.writeSingleRegister(REG_POWER_ON_STATE, state ? 1 : 0);
}

bool MotorModbusController::setOutputLimits(uint8_t minOutput, uint8_t maxOutput) {
    if (minOutput > 50) minOutput = 50;
    if (maxOutput < 60) maxOutput = 60;
    if (maxOutput > 100) maxOutput = 100;
    
    uint16_t values[2] = {minOutput, maxOutput};
    return _modbus.writeMultipleRegisters(REG_MIN_OUTPUT, 2, values);
}

bool MotorModbusController::setSoftTimes(uint16_t startTime, uint16_t stopTime) {
    uint16_t values[2] = {startTime, stopTime};
    return _modbus.writeMultipleRegisters(REG_SOFT_START_TIME, 2, values);
}

bool MotorModbusController::start() {
    return _modbus.writeSingleRegister(REG_RUN_STATUS, 1);
}

bool MotorModbusController::stop() {
    return _modbus.writeSingleRegister(REG_RUN_STATUS, 0);
}

bool MotorModbusController::getRunStatus(bool& running) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_RUN_STATUS, 1, &value)) {
        running = (value == 1);
        return true;
    }
    return false;
}

bool MotorModbusController::setFrequency(uint32_t frequency) {
    uint16_t freqHigh = (frequency >> 16) & 0xFFFF;
    uint16_t freqLow = frequency & 0xFFFF;
    uint16_t values[2] = {freqHigh, freqLow};
    return _modbus.writeMultipleRegisters(REG_FREQ_HIGH, 2, values);
}

bool MotorModbusController::getFrequency(uint32_t& frequency) {
    uint16_t values[2];
    if (_modbus.readHoldingRegisters(REG_FREQ_HIGH, 2, values)) {
        frequency = ((uint32_t)values[0] << 16) | values[1];
        return true;
    }
    return false;
}

bool MotorModbusController::setDutyCycle(uint8_t duty) {
    if (duty > 100) duty = 100;
    return _modbus.writeSingleRegister(REG_DUTY_CYCLE, duty);
}

bool MotorModbusController::getDutyCycle(uint8_t& duty) {
    uint16_t value;
    if (_modbus.readHoldingRegisters(REG_DUTY_CYCLE, 1, &value)) {
        duty = (uint8_t)value;
        return true;
    }
    return false;
}

bool MotorModbusController::setOutput(uint32_t frequency, uint8_t duty) {
    uint16_t freqHigh = (frequency >> 16) & 0xFFFF;
    uint16_t freqLow = frequency & 0xFFFF;
    uint16_t values[3] = {freqHigh, freqLow, duty};
    return _modbus.writeMultipleRegisters(REG_FREQ_HIGH, 3, values);
}

bool MotorModbusController::getOutput(uint32_t& frequency, uint8_t& duty) {
    uint16_t values[3];
    if (_modbus.readHoldingRegisters(REG_RUN_STATUS, 3, values)) {
        frequency = ((uint32_t)values[1] << 16) | values[2];
        duty = (uint8_t)values[3];
        return true;
    }
    return false;
}

String MotorModbusController::getLastError() const {
    return _modbus.getLastErrorString();
}