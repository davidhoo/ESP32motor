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

bool MotorModbusController::getAllConfig(AllConfig& config) {
    // 需要读取的寄存器数量: 从 0x0001 到 0x000B (共 11 个寄存器)
    uint16_t values[11];
    
    // 从寄存器 0x0001 开始读取 11 个寄存器
    if (!_modbus.readHoldingRegisters(REG_EXTERNAL_SWITCH, 11, values)) {
        return false;
    }
    
    // 填充 AllConfig 结构体
    config.externalSwitch = (values[0] == 1);    // 0x0001
    config.analogControl = (values[1] == 1);     // 0x0002
    config.powerOnState = (values[2] == 1);      // 0x0003
    config.minOutput = (uint8_t)values[3];       // 0x0004
    config.maxOutput = (uint8_t)values[4];       // 0x0005
    config.softStartTime = values[5];            // 0x0006
    config.softStopTime = values[6];             // 0x0007
    config.isRunning = (values[7] == 1);         // 0x0008
    config.frequency = ((uint32_t)values[8] << 16) | values[9]; // 0x0009, 0x000A
    config.dutyCycle = (uint8_t)values[10];      // 0x000B
    
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
        (uint16_t)(config.externalSwitch ? 1 : 0),
        (uint16_t)(config.analogControl ? 1 : 0),
        (uint16_t)(config.powerOnState ? 1 : 0),
        config.minOutput,
        config.maxOutput,
        config.softStartTime,
        config.softStopTime
    };
    
    return _modbus.writeMultipleRegisters(REG_MODULE_ADDRESS, 8, values);
}

bool MotorModbusController::setAllConfig(const AllConfig& config, bool setRunning) {
    // 准备寄存器值数组
    // 从 REG_EXTERNAL_SWITCH (0x0001) 开始，共 11 个寄存器
    uint16_t values[11];
    
    // 填充配置值
    values[0] = config.externalSwitch ? 1 : 0;    // REG_EXTERNAL_SWITCH (0x0001)
    values[1] = config.analogControl ? 1 : 0;     // REG_0_10V_CONTROL (0x0002)
    values[2] = config.powerOnState ? 1 : 0;      // REG_POWER_ON_STATE (0x0003)
    values[3] = config.minOutput;                // REG_MIN_OUTPUT (0x0004)
    values[4] = config.maxOutput;                // REG_MAX_OUTPUT (0x0005)
    values[5] = config.softStartTime;            // REG_SOFT_START_TIME (0x0006)
    values[6] = config.softStopTime;             // REG_SOFT_STOP_TIME (0x0007)
    values[7] = config.isRunning ? 1 : 0;        // REG_RUN_STATUS (0x0008) - 只有在 setRunning 为 true 时才设置
    values[8] = (config.frequency >> 16) & 0xFFFF; // REG_FREQ_HIGH (0x0009)
    values[9] = config.frequency & 0xFFFF;       // REG_FREQ_LOW (0x000A)
    values[10] = config.dutyCycle;               // REG_DUTY_CYCLE (0x000B)
    
    // 如果不设置运行状态，则不写入运行状态寄存器
    if (!setRunning) {
        // 只写入前7个寄存器（0x0001-0x0007），不包括运行状态
        if (!_modbus.writeMultipleRegisters(REG_EXTERNAL_SWITCH, 7, values)) {
            return false;
        }
        
        // 然后写入频率和占空比寄存器（0x0009-0x000B）
        uint16_t freqDutyValues[3] = {values[8], values[9], values[10]};
        return _modbus.writeMultipleRegisters(REG_FREQ_HIGH, 3, freqDutyValues);
    } else {
        // 设置所有寄存器，包括运行状态
        return _modbus.writeMultipleRegisters(REG_EXTERNAL_SWITCH, 11, values);
    }
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