# MODBUS-RTU协议调研与实现方案

## 1. MODBUS-RTU协议规范

### 1.1 通信参数
- **接口类型**: TTL串口
- **波特率**: 9600 bps
- **数据格式**: 8位数据位，无校验位，1位停止位 (8N1)
- **电气标准**: TTL电平 (0-3.3V)

### 1.2 硬件连接
```
ESP32-S3-Zero    调速器TTL接口
GPIO8  (RX)   <--------  TX
GPIO9  (TX)   -------->  RX
GND           --------->  GND
```

### 1.3 协议规范
- **数据格式**: 所有寄存器为16位（双字节）数据
- **字节序**: 大端模式（高字节在前，低字节在后）
- **支持功能码**:
  - 0x03: 读保持寄存器 (Read Holding Registers)
  - 0x06: 写单个寄存器 (Write Single Register)
  - 0x10: 写多个寄存器 (Write Multiple Registers)

## 2. 调速器MODBUS寄存器映射（实际规格）

### 2.1 寄存器详细说明

| 寄存器地址 | 参数名称 | 数据类型 | 读写权限 | 数值范围 | 单位 | 说明 |
|------------|----------|----------|----------|----------|------|------|
| 0x0000 | 模块地址 | uint16 | R/W | 1-255 | - | 模块地址，1-255（默认1，0为广播） |
| 0x0001 | 外接开关功能 | uint16 | R/W | 0-1 | - | 0：功能关闭；1：功能打开 |
| 0x0002 | 0-10V控制功能 | uint16 | R/W | 0-1 | - | 0：功能关闭；1：功能打开 |
| 0x0003 | 开机上电默认状态 | uint16 | R/W | 0-1 | - | 0：停止；1：运行 |
| 0x0004 | 最小输出设置 | uint16 | R/W | 0-50 | % | 最小输出百分比（0%-50%） |
| 0x0005 | 最大输出设置 | uint16 | R/W | 60-100 | % | 最大输出百分比（60%-100%） |
| 0x0006 | 缓启动时间 | uint16 | R/W | 0-65535 | 0.1s | 缓启动至100%速度时间 |
| 0x0007 | 缓停止时间 | uint16 | R/W | 0-65535 | 0.1s | 缓停止至0%速度时间 |
| 0x0008 | 运行状态 | uint16 | R/W | 0-1 | - | 0：停止输出；1：正常输出 |
| 0x0009 | 频率高字节 | uint16 | R/W | 0-65535 | - | 频率 = (Fre H×65536 + Fre L) Hz |
| 0x000A | 频率低字节 | uint16 | R/W | 0-65535 | - | 频率低16位 |
| 0x000B | 占空比 | uint16 | R/W | 0-100 | % | 占空比（0%-100%） |

### 2.2 频率计算
- **实际频率** = (频率高字节 × 65536 + 频率低字节) Hz
- **占空比** = 占空比寄存器值 %

### 2.3 功能码支持
- **0x03**: 读保持寄存器 (Read Holding Registers)
- **0x06**: 写单个寄存器 (Write Single Register)
- **0x10**: 写多个寄存器 (Write Multiple Registers)

## 3. 通信示例

### 3.1 读取运行状态
**主机请求**:
```
地址: 0x01
功能码: 0x03
起始地址: 0x0008
寄存器数量: 0x0001
CRC: 计算值
```
完整帧: `01 03 00 08 00 01 [CRC_L] [CRC_H]`

**从机响应**:
```
地址: 0x01
功能码: 0x03
字节数: 0x02
数据: 0x0001 (运行中)
CRC: 计算值
```

### 3.2 设置占空比
**主机请求** (设置50%占空比):
```
地址: 0x01
功能码: 0x06
寄存器地址: 0x000B
寄存器值: 0x0032 (50)
CRC: 计算值
```
完整帧: `01 06 00 0B 00 32 [CRC_L] [CRC_H]`

### 3.3 设置频率
**主机请求** (设置1000Hz):
```
地址: 0x01
功能码: 0x10
起始地址: 0x0009
寄存器数量: 0x0002
字节数: 0x04
数据: 0x0000 0x03E8 (1000Hz)
CRC: 计算值
```

### 3.4 批量读取配置
**主机请求** (读取所有配置参数):
```
地址: 0x01
功能码: 0x03
起始地址: 0x0000
寄存器数量: 0x000C (12个寄存器)
CRC: 计算值
```

## 4. 更新后的实现架构

### 4.1 新的寄存器定义

```cpp
// 调速器实际寄存器地址
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
```

### 4.2 频率操作函数

```cpp
// 设置频率（Hz）
bool setFrequency(uint32_t frequency) {
    uint16_t freqHigh = (frequency >> 16) & 0xFFFF;
    uint16_t freqLow = frequency & 0xFFFF;
    uint16_t values[2] = {freqHigh, freqLow};
    return writeMultipleRegisters(REG_FREQ_HIGH, 2, values);
}

// 获取频率（Hz）
uint32_t getFrequency() {
    uint16_t values[2];
    if (readHoldingRegisters(REG_FREQ_HIGH, 2, values)) {
        return (values[0] << 16) | values[1];
    }
    return 0;
}
```

### 4.3 占空比操作

```cpp
// 设置占空比（0-100%）
bool setDutyCycle(uint8_t duty) {
    if (duty > 100) duty = 100;
    return writeSingleRegister(REG_DUTY_CYCLE, duty);
}

// 获取占空比
uint8_t getDutyCycle() {
    uint16_t duty;
    if (readHoldingRegisters(REG_DUTY_CYCLE, 1, &duty)) {
        return (uint8_t)duty;
    }
    return 0;
}
```

### 4.4 运行控制

```cpp
// 启动输出
bool startOutput() {
    return writeSingleRegister(REG_RUN_STATUS, 1);
}

// 停止输出
bool stopOutput() {
    return writeSingleRegister(REG_RUN_STATUS, 0);
}

// 获取运行状态
bool isRunning() {
    uint16_t status;
    if (readHoldingRegisters(REG_RUN_STATUS, 1, &status)) {
        return status == 1;
    }
    return false;
}
```

## 5. 更新后的API接口

### 5.1 新的MotorController类

```cpp
class MotorController {
public:
    struct MotorConfig {
        uint8_t moduleAddress;      // 模块地址
        bool externalSwitch;        // 外接开关功能
        bool analogControl;         // 0-10V控制功能
        bool powerOnState;          // 开机默认状态
        uint8_t minOutput;          // 最小输出百分比
        uint8_t maxOutput;          // 最大输出百分比
        uint16_t softStartTime;     // 缓启动时间(0.1s)
        uint16_t softStopTime;      // 缓停止时间(0.1s)
    };
    
    struct MotorStatus {
        bool isRunning;             // 运行状态
        uint32_t frequency;         // 当前频率(Hz)
        uint8_t dutyCycle;          // 当前占空比(%)
    };
    
    bool begin(uint8_t address = 1);
    bool getConfig(MotorConfig& config);
    bool setConfig(const MotorConfig& config);
    bool getStatus(MotorStatus& status);
    bool setFrequency(uint32_t freq);
    bool setDutyCycle(uint8_t duty);
    bool start();
    bool stop();
};
```

## 6. 使用示例更新

### 6.1 基本控制
```cpp
#include "controllers/MotorController.h"

MotorController motor;
motor.begin(1);  // 地址1

// 设置频率1000Hz，占空比50%
motor.setFrequency(1000);
motor.setDutyCycle(50);

// 启动输出
motor.start();

// 停止输出
motor.stop();
```

### 6.2 完整配置
```cpp
MotorController::MotorConfig config;
config.moduleAddress = 1;
config.externalSwitch = false;
config.analogControl = false;
config.powerOnState = false;
config.minOutput = 10;   // 10%
config.maxOutput = 90;   // 90%
config.softStartTime = 50;  // 5秒
config.softStopTime = 50;   // 5秒

motor.setConfig(config);
```

## 7. 测试程序更新

所有测试程序已更新为使用新的寄存器映射：
- 支持频率设置和读取
- 支持占空比控制
- 支持软启动/停止时间配置
- 支持运行状态监控

## 8. 注意事项

1. **频率范围**: 根据调速器规格设置合理的频率范围
2. **占空比限制**: 受minOutput和maxOutput限制
3. **缓启动/停止**: 时间单位为0.1秒
4. **模块地址**: 设置后需要重新初始化通信
5. **广播地址**: 0x00为广播地址，所有模块都会响应