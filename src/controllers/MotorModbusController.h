#pragma once

#include <Arduino.h>
#include "../drivers/ModbusRTUDriver.h"

class MotorModbusController {
public:
    struct MotorConfig {
        uint8_t moduleAddress;      // 模块地址 (1-255)
        bool externalSwitch;        // 外接开关功能 (0=关闭, 1=打开)
        bool analogControl;         // 0-10V控制功能 (0=关闭, 1=打开)
        bool powerOnState;          // 开机上电默认状态 (0=停止, 1=运行)
        uint8_t minOutput;          // 最小输出设置 (0-50%)
        uint8_t maxOutput;          // 最大输出设置 (60-100%)
        uint16_t softStartTime;     // 缓启动时间 (0-65535, 0.1s单位)
        uint16_t softStopTime;      // 缓停止时间 (0-65535, 0.1s单位)
    };
    
    struct AllConfig {
        bool externalSwitch;        // 外接开关功能 (0=关闭, 1=打开)
        bool analogControl;         // 0-10V控制功能 (0=关闭, 1=打开)
        bool powerOnState;          // 开机上电默认状态 (0=停止, 1=运行)
        uint8_t minOutput;          // 最小输出设置 (0-50%)
        uint8_t maxOutput;          // 最大输出设置 (60-100%)
        uint16_t softStartTime;     // 缓启动时间 (0-65535, 0.1s单位)
        uint16_t softStopTime;      // 缓停止时间 (0-65535, 0.1s单位)
        bool isRunning;             // 运行状态 (0=停止, 1=运行)
        uint32_t frequency;         // 频率 (Hz)
        uint8_t dutyCycle;          // 占空比 (0-100%)
    };
    
    struct MotorStatus {
        bool isRunning;             // 运行状态 (0=停止, 1=运行)
        uint32_t frequency;         // 当前频率 (Hz)
        uint8_t dutyCycle;          // 当前占空比 (0-100%)
    };
    
    MotorModbusController();
    ~MotorModbusController();
    
    // 初始化
    bool begin(uint8_t motorAddress = 0x01);
    
    // 配置获取
    bool getConfig(MotorConfig& config);
    bool getAllConfig(AllConfig& config);
    bool getModuleAddress(uint8_t& address);
    bool getExternalSwitch(bool& enabled);
    bool getAnalogControl(bool& enabled);
    bool getPowerOnState(bool& state);
    bool getOutputLimits(uint8_t& minOutput, uint8_t& maxOutput);
    bool getSoftTimes(uint16_t& startTime, uint16_t& stopTime);
    
    // 配置设置
    bool setConfig(const MotorConfig& config);
    bool setModuleAddress(uint8_t address);
    bool setExternalSwitch(bool enabled);
    bool setAnalogControl(bool enabled);
    bool setPowerOnState(bool state);
    bool setOutputLimits(uint8_t minOutput, uint8_t maxOutput);
    bool setSoftTimes(uint16_t startTime, uint16_t stopTime);
    
    // 运行控制
    bool start();
    bool stop();
    bool getRunStatus(bool& running);
    
    // 频率控制
    bool setFrequency(uint32_t frequency);
    bool getFrequency(uint32_t& frequency);
    
    // 占空比控制
    bool setDutyCycle(uint8_t duty);
    bool getDutyCycle(uint8_t& duty);
    
    // 组合控制
    bool setOutput(uint32_t frequency, uint8_t duty);
    bool getOutput(uint32_t& frequency, uint8_t& duty);
    
    // 配置
    void setMotorAddress(uint8_t address) { _motorAddress = address; _modbus.setSlaveAddress(address); }
    uint8_t getMotorAddress() const { return _motorAddress; }
    
    // 错误信息
    String getLastError() const;
    
private:
    // 寄存器地址定义（实际规格）
    static const uint16_t REG_MODULE_ADDRESS = 0x0000;
    static const uint16_t REG_EXTERNAL_SWITCH = 0x0001;
    static const uint16_t REG_0_10V_CONTROL = 0x0002;
    static const uint16_t REG_POWER_ON_STATE = 0x0003;
    static const uint16_t REG_MIN_OUTPUT = 0x0004;
    static const uint16_t REG_MAX_OUTPUT = 0x0005;
    static const uint16_t REG_SOFT_START_TIME = 0x0006;
    static const uint16_t REG_SOFT_STOP_TIME = 0x0007;
    static const uint16_t REG_RUN_STATUS = 0x0008;
    static const uint16_t REG_FREQ_HIGH = 0x0009;
    static const uint16_t REG_FREQ_LOW = 0x000A;
    static const uint16_t REG_DUTY_CYCLE = 0x000B;
    
    ModbusRTUDriver _modbus;
    uint8_t _motorAddress;
};