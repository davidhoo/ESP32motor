# ESP32-S3-Zero 电机定时控制系统架构设计

## 1. 架构概述

本系统是一个基于ESP32-S3-Zero的简单电机定时控制系统，通过BLE无线控制电机的运行时长和停止间隔，实现自动循环控制。

### 1.1 设计原则
- **简单性**: 避免过度设计，保持代码结构清晰
- **模块化**: 功能模块独立，便于维护和测试
- **实时性**: 确保电机控制的实时响应
- **可靠性**: 参数持久化存储，系统状态可恢复

## 2. 系统架构

### 2.1 整体架构图

```mermaid
graph TB
    subgraph "应用层"
        A[主控制器 MainController]
    end
    
    subgraph "业务逻辑层"
        B[电机控制器 MotorController]
        C[BLE服务器 BLEServer]
        D[LED控制器 LEDController]
        E[配置管理器 ConfigManager]
    end
    
    subgraph "硬件抽象层"
        F[GPIO驱动 GPIODriver]
        G[定时器驱动 TimerDriver]
        H[NVS存储 NVSStorage]
        I[WS2812驱动 WS2812Driver]
    end
    
    A --> B
    A --> C
    A --> D
    A --> E
    
    B --> F
    B --> G
    D --> I
    E --> H
```

### 2.2 核心模块划分

| 模块名称 | 职责 | 依赖关系 |
|---------|------|----------|
| MainController | 系统主控制器，协调各模块工作 | 依赖所有业务模块 |
| MotorController | 电机控制逻辑，定时循环控制 | GPIODriver, TimerDriver |
| BLEServer | BLE通信服务，处理手机连接和数据交换 | 无 |
| LEDController | LED状态指示控制 | WS2812Driver |
| ConfigManager | 配置参数管理和持久化 | NVSStorage |

## 3. 模块接口设计

### 3.1 MotorController 接口

```cpp
class MotorController {
public:
    // 初始化电机控制器
    bool init(uint8_t motor_pin);
    
    // 设置运行参数
    void setRunDuration(uint32_t duration_100ms);
    void setStopInterval(uint32_t interval_seconds);
    
    // 控制命令
    void start();
    void stop();
    
    // 状态查询
    bool isRunning() const;
    uint32_t getCountdown() const;
    uint32_t getCycleCount() const;
    
    // 定时器回调处理
    void onTimer();
    
private:
    enum State { STOPPED, RUNNING, PAUSED };
    State current_state;
    uint32_t run_duration;
    uint32_t stop_interval;
    uint32_t countdown;
    uint32_t cycle_count;
};
```

### 3.2 BLEServer 接口

```cpp
class BLEServer {
public:
    // 初始化BLE服务
    bool init(const char* device_name);
    
    // 设置特征值回调
    void setRunDurationCallback(std::function<void(uint32_t)> callback);
    void setStopIntervalCallback(std::function<void(uint32_t)> callback);
    void setSystemControlCallback(std::function<void(uint8_t)> callback);
    
    // 状态推送
    void updateStatus(const String& json_status);
    
    // 连接状态
    bool isConnected() const;
    
private:
    BLEService* motor_service;
    BLECharacteristic* run_duration_char;
    BLECharacteristic* stop_interval_char;
    BLECharacteristic* system_control_char;
    BLECharacteristic* status_char;
};
```

### 3.3 LEDController 接口

```cpp
class LEDController {
public:
    enum LEDState {
        SYSTEM_INIT,      // 白色闪烁
        BLE_WAITING,      // 蓝色慢闪
        BLE_CONNECTED,    // 绿色常亮
        MOTOR_RUNNING,    // 青色常亮
        MOTOR_STOPPED,    // 黄色常亮
        SYSTEM_STOPPED    // 红色常亮
    };
    
    // 初始化LED控制器
    bool init(uint8_t led_pin);
    
    // 设置LED状态
    void setState(LEDState state);
    
    // 定时器回调处理闪烁效果
    void onTimer();
    
private:
    LEDState current_state;
    bool blink_state;
};
```

### 3.4 ConfigManager 接口

```cpp
struct MotorConfig {
    uint32_t run_duration;    // 运行时长(100毫秒单位)
    uint32_t stop_interval;   // 停止间隔(秒)
    bool auto_start;          // 开机自动启动
};

class ConfigManager {
public:
    // 初始化配置管理器
    bool init();
    
    // 加载配置
    MotorConfig loadConfig();
    
    // 保存配置
    bool saveConfig(const MotorConfig& config);
    
    // 获取默认配置
    static MotorConfig getDefaultConfig();
    
private:
    static const char* NVS_NAMESPACE;
    static const char* CONFIG_KEY;
};
```

## 4. 数据流设计

### 4.1 系统启动流程

```mermaid
sequenceDiagram
    participant Main as MainController
    participant Config as ConfigManager
    participant Motor as MotorController
    participant BLE as BLEServer
    participant LED as LEDController
    
    Main->>LED: setState(SYSTEM_INIT)
    Main->>Config: loadConfig()
    Config-->>Main: MotorConfig
    Main->>Motor: init() & setParams()
    Main->>BLE: init()
    Main->>LED: setState(BLE_WAITING)
    Main->>Motor: start()
    Main->>LED: setState(MOTOR_RUNNING)
```

### 4.2 BLE控制流程

```mermaid
sequenceDiagram
    participant Phone as 手机App
    participant BLE as BLEServer
    participant Main as MainController
    participant Motor as MotorController
    participant Config as ConfigManager
    
    Phone->>BLE: 连接
    BLE-->>Main: onConnected()
    Main->>LED: setState(BLE_CONNECTED)
    
    Phone->>BLE: 写入运行时长
    BLE-->>Main: onRunDurationChanged()
    Main->>Motor: setRunDuration()
    Main->>Config: saveConfig()
    
    Phone->>BLE: 写入系统控制
    BLE-->>Main: onSystemControl()
    Main->>Motor: start()/stop()
```

## 5. 关键技术决策

### 5.1 定时器策略
- 使用ESP32硬件定时器，精度1ms
- 主定时器负责电机状态切换和倒计时
- LED闪烁使用独立的软件计数器

### 5.2 状态管理
- 电机控制器维护独立的状态机
- LED状态与系统状态解耦，便于扩展
- BLE连接状态独立管理

### 5.3 数据存储
- 使用NVS存储用户配置参数
- 配置变更立即持久化
- 系统重启后自动恢复上次配置

### 5.4 错误处理
- 各模块初始化失败时返回错误码
- BLE断连时系统继续正常工作
- 参数越界时使用默认值并记录日志

## 6. 文件组织结构

```
src/
├── main.cpp                 // 主程序入口
├── controllers/
│   ├── MainController.h/.cpp    // 主控制器
│   ├── MotorController.h/.cpp   // 电机控制器
│   ├── BLEServer.h/.cpp         // BLE服务器
│   ├── LEDController.h/.cpp     // LED控制器
│   └── ConfigManager.h/.cpp     // 配置管理器
├── drivers/
│   ├── GPIODriver.h/.cpp        // GPIO驱动
│   ├── TimerDriver.h/.cpp       // 定时器驱动
│   ├── NVSStorage.h/.cpp        // NVS存储驱动
│   └── WS2812Driver.h/.cpp      // WS2812驱动
└── common/
    ├── Config.h                 // 全局配置定义
    └── Logger.h/.cpp            // 日志工具
```

## 7. 开发和测试建议

### 7.1 开发顺序
1. 实现硬件驱动层（GPIO、定时器、NVS、WS2812）
2. 实现配置管理器和LED控制器
3. 实现电机控制器核心逻辑
4. 实现BLE服务器
5. 集成主控制器
6. 系统测试和优化

### 7.2 测试策略
- 单元测试：各模块独立功能测试
- 集成测试：模块间接口测试
- 硬件测试：实际硬件环境验证
- 压力测试：长时间运行稳定性测试

---

**文档版本**: v1.0  
**创建日期**: 2025-01-28  
**负责人**: 架构师