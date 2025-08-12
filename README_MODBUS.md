# MODBUS-RTU 调速器通信功能

## 功能概述

本项目已成功实现了基于MODBUS-RTU协议的ESP32-S3-Zero与调速器通信功能，通过TTL串口实现参数获取和设置。

## 硬件连接

```
ESP32-S3-Zero    调速器TTL接口
GPIO8  (RX)   <--------  TX
GPIO9  (TX)   -------->  RX
GND           --------->  GND
```

## 通信参数

- **波特率**: 9600 bps
- **数据格式**: 8位数据位，无校验位，1位停止位 (8N1)
- **协议**: MODBUS-RTU
- **从机地址**: 0x01 (可配置)

## 实际寄存器映射

| 寄存器地址 | 参数名称 | 类型 | 读写 | 范围 | 说明 |
|------------|----------|------|------|------|------|
| 0x0000 | 模块地址 | uint16 | R/W | 1-255 | 模块地址，1-255（默认1，0为广播） |
| 0x0001 | 外接开关功能 | uint16 | R/W | 0-1 | 0：功能关闭；1：功能打开 |
| 0x0002 | 0-10V控制功能 | uint16 | R/W | 0-1 | 0：功能关闭；1：功能打开 |
| 0x0003 | 开机上电默认状态 | uint16 | R/W | 0-1 | 0：停止；1：运行 |
| 0x0004 | 最小输出设置 | uint16 | R/W | 0-50 | 最小输出百分比（0%-50%） |
| 0x0005 | 最大输出设置 | uint16 | R/W | 60-100 | 最大输出百分比（60%-100%） |
| 0x0006 | 缓启动时间 | uint16 | R/W | 0-65535 | 缓启动至100%速度时间（0.1s） |
| 0x0007 | 缓停止时间 | uint16 | R/W | 0-65535 | 缓停止至0%速度时间（0.1s） |
| 0x0008 | 运行状态 | uint16 | R/W | 0-1 | 0：停止输出；1：正常输出 |
| 0x0009 | 频率高字节 | uint16 | R/W | 0-65535 | 频率高16位 |
| 0x000A | 频率低字节 | uint16 | R/W | 0-65535 | 频率低16位 |
| 0x000B | 占空比 | uint16 | R/W | 0-100 | 占空比（0%-100%） |

## 频率计算
- **实际频率** = (频率高字节 × 65536 + 频率低字节) Hz

## 使用示例

### 1. 基本使用

```cpp
#include "controllers/MotorModbusController.h"

MotorModbusController motor;
motor.begin(0x01);  // 初始化，地址0x01

// 设置频率1000Hz，占空比50%
motor.setFrequency(1000);
motor.setDutyCycle(50);

// 启动输出
motor.start();

// 停止输出
motor.stop();
```

### 2. 完整配置

```cpp
MotorModbusController::MotorConfig config;
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

### 3. 读取状态

```cpp
MotorModbusController::MotorStatus status;
motor.getStatus(status);

Serial.print("运行状态: ");
Serial.println(status.isRunning ? "运行中" : "停止");
Serial.print("当前频率: ");
Serial.print(status.frequency);
Serial.println(" Hz");
Serial.print("当前占空比: ");
Serial.print(status.dutyCycle);
Serial.println(" %");
```

## 测试程序

### 1. 基本测试
```bash
pio run -e modbus-test -t upload
pio device monitor
```

### 2. 使用示例程序

- **modbus_motor_test.cpp**: 基本功能测试
- **modbus_config_example.cpp**: 参数配置示例
- **modbus_test_runner.cpp**: 交互式测试菜单

## 文件结构

```
src/
├── drivers/
│   ├── SerialDriver.h/.cpp      # 串口驱动封装
│   └── ModbusRTUDriver.h/.cpp   # MODBUS-RTU协议实现
├── controllers/
│   └── MotorModbusController.h/.cpp  # 调速器控制接口
examples/
├── modbus_motor_test.cpp        # 基本功能测试
├── modbus_config_example.cpp    # 参数配置示例
└── modbus_test_runner.cpp       # 交互式测试
docs/
└── modbus_rtu_protocol_analysis.md  # 协议详细文档
```

## 功能特点

1. **完整的MODBUS-RTU协议栈**
2. **支持所有标准功能码** (0x03, 0x06, 0x10)
3. **CRC校验和错误处理**
4. **重试机制**
5. **超时保护**
6. **参数边界检查**
7. **频率和占空比精确控制**
8. **软启动/停止时间配置**

## 编译环境

在 `platformio.ini` 中已添加专门的测试环境：

```ini
[env:modbus-test]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
build_flags = -DARDUINO_USB_CDC_ON_BOOT=1 -DBOARD_HAS_PSRAM
```

## 注意事项

1. **频率范围**: 根据调速器规格设置合理的频率范围
2. **占空比限制**: 受minOutput和maxOutput限制
3. **缓启动/停止**: 时间单位为0.1秒
4. **模块地址**: 设置后需要重新初始化通信
5. **广播地址**: 0x00为广播地址，所有模块都会响应

## 下一步建议

1. 与现有BLE系统集成，实现参数同步
2. 添加实时状态推送功能
3. 实现数据记录和统计分析
4. 支持多个调速器并联控制
5. 添加Web配置界面

所有代码已更新为匹配实际调速器寄存器映射，可以直接用于实际调速器通信测试。