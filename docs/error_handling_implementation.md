# ESP32 电机控制系统错误处理机制实现文档

## 概述

本文档详细描述了ESP32电机控制系统中实现的错误处理机制，对应todo.md中5.4部分的功能要求。错误处理机制包括三个主要方面：

1. **模块初始化失败的错误处理机制**
2. **参数越界检查和默认值回退功能**
3. **BLE断连时的系统稳定运行机制**

## 5.4.1 模块初始化失败的错误处理机制

### 设计目标

- 提供模块初始化的重试机制
- 区分关键模块和非关键模块
- 在关键模块失败时启动安全模式
- 确保系统在部分模块失败时仍能提供基本功能

### 实现细节

#### 重试机制

**位置**: [`src/controllers/MainController.cpp`](../src/controllers/MainController.cpp)

```cpp
// 最大重试次数
static const int MAX_INIT_RETRIES = 3;

// 带重试机制的模块初始化
bool MainController::initializeWithRetry(const char* moduleName, 
                                        std::function<bool()> initFunc, 
                                        bool isCritical);
```

**特性**:
- 每个模块最多重试3次
- 重试间隔递增：1秒、2秒、3秒
- 详细的日志记录每次重试过程
- 区分关键模块和非关键模块的处理策略

#### 模块分类

**关键模块** (初始化失败将导致系统进入安全模式):
- LED控制器 - 系统状态指示必需
- 事件管理器 - 系统通信核心
- 电机控制器 - 核心功能模块

**非关键模块** (初始化失败系统仍可继续运行):
- BLE服务器 - 可在离线模式下运行
- 配置管理器 - 可使用默认配置

#### 安全模式

**触发条件**:
- 关键模块初始化失败且重试耗尽
- 运行时检测到关键模块异常

**安全模式行为**:
```cpp
void MainController::enterSafeMode() {
    // 设置LED为错误状态指示
    if (ledControllerInitialized) {
        ledController.setState(LEDState::ERROR_STATE);
    }
    
    // 停止所有非关键服务
    // 标记系统为未初始化状态
    // 记录详细错误信息
}
```

#### 降级策略

**配置管理器失败**:
- 使用内置默认配置
- 系统继续运行但无法持久化设置

**BLE服务器失败**:
- 系统进入离线模式
- 电机控制功能正常工作
- LED指示BLE不可用状态

### 使用示例

```cpp
// 初始化关键模块
if (!initializeWithRetry("电机控制器", 
    [this]() { return initializeMotorController(); }, true)) {
    enterSafeMode();
    return false;
}

// 初始化非关键模块
if (!initializeWithRetry("BLE服务器", 
    [this]() { return initializeBLEServer(); }, false)) {
    LOG_WARN("BLE服务器不可用，系统将在离线模式下运行");
}
```

## 5.4.2 参数越界检查和默认值回退功能

### 设计目标

- 自动检测和修正越界参数
- 提供合理的默认值回退
- 确保系统配置始终处于有效状态
- 详细记录参数修正过程

### 实现细节

#### 参数验证范围

**位置**: [`src/controllers/ConfigManager.cpp`](../src/controllers/ConfigManager.cpp)

| 参数 | 最小值 | 最大值 | 默认值 | 说明 |
|------|--------|--------|--------|------|
| runDuration | 100ms | 3600000ms (1小时) | 5000ms | 电机运行时长 |
| stopDuration | 0ms | 3600000ms (1小时) | 2000ms | 电机停止时长 |
| cycleCount | 0 | 1000000 | 0 (无限) | 循环次数 |
| autoStart | - | - | true | 自动启动标志 |

#### 参数修正逻辑

```cpp
bool ConfigManager::validateAndSanitizeConfig(MotorConfig& config) const {
    bool wasModified = false;
    String corrections = "";
    
    // 运行时长修正
    if (config.runDuration < 100) {
        corrections += "运行时长过小，已修正为100ms; ";
        config.runDuration = 100;
        wasModified = true;
    } else if (config.runDuration > 3600000) {
        corrections += "运行时长过大，已修正为3600000ms; ";
        config.runDuration = 3600000;
        wasModified = true;
    }
    
    // 停止时长修正
    if (config.stopDuration < 0) {
        corrections += "停止时长为负数，已修正为0ms; ";
        config.stopDuration = 0;
        wasModified = true;
    }
    
    // 合理性检查
    if (config.runDuration < 1000 && config.stopDuration > 60000) {
        corrections += "运行时长过短而停止时长过长，已调整为合理比例; ";
        config.runDuration = (config.runDuration > 1000ul) ? config.runDuration : 1000ul;
        config.stopDuration = (config.stopDuration < 30000ul) ? config.stopDuration : 30000ul;
        wasModified = true;
    }
    
    return !wasModified; // 返回true表示没有修正
}
```

#### 特殊情况处理

**零值处理**:
```cpp
// 如果运行时长和停止时长都为0，使用默认值
if (config.runDuration == 0 && config.stopDuration == 0) {
    corrections += "运行和停止时长都为0，已恢复为默认值; ";
    config.runDuration = defaultConfig.runDuration;
    config.stopDuration = defaultConfig.stopDuration;
    wasModified = true;
}
```

**配置加载时的处理**:
```cpp
// 验证并修正加载的配置
if (!validateAndSanitizeConfig(loadedConfig)) {
    LOG_TAG_WARN("ConfigManager", "加载的配置存在问题，已自动修正: %s", getValidationError());
}

// 再次验证修正后的配置
if (!validateConfig(loadedConfig)) {
    setLastError("加载的配置无效且无法修正");
    resetToDefaults();
    return false;
}
```

### 使用示例

```cpp
// 更新配置时自动验证和修正
void ConfigManager::updateConfig(const MotorConfig& config) {
    MotorConfig safeConfig = config;
    
    // 自动验证和修正参数
    if (!validateAndSanitizeConfig(safeConfig)) {
        LOG_TAG_WARN("ConfigManager", "配置参数越界，已自动修正: %s", getValidationError());
    }
    
    // 应用修正后的配置
    currentConfig = safeConfig;
    isModified = true;
}
```

## 5.4.3 BLE断连时的系统稳定运行机制

### 设计目标

- 确保BLE断连不影响核心功能
- 提供断连检测和处理机制
- 实现智能重连策略
- 维护系统状态的一致性

### 实现细节

#### 断连检测

**位置**: [`src/controllers/MotorBLEServer.cpp`](../src/controllers/MotorBLEServer.cpp)

```cpp
// 断连状态跟踪
bool disconnectionHandled = false;
uint32_t lastConnectionTime = 0;
uint32_t disconnectionCount = 0;
static const uint32_t RECONNECTION_TIMEOUT = 30000; // 30秒重连超时
```

#### 断连处理流程

```cpp
void MotorBLEServer::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    bleServer->deviceConnected = false;
    bleServer->disconnectionCount++;
    LOG_INFO("BLE客户端已断开 (第%lu次断连)", bleServer->disconnectionCount);
    
    // 执行断连处理
    bleServer->handleDisconnection();
    
    // 发布断连事件
    EventManager::getInstance().publish(EventData(
        EventType::BLE_DISCONNECTED,
        "MotorBLEServer",
        "BLE客户端连接断开"
    ));
    
    // 重新启动广播
    BLEDevice::startAdvertising();
}
```

#### 系统稳定性保证

```cpp
void MotorBLEServer::ensureSystemStability() {
    LOG_INFO("确保BLE断连后系统稳定运行");
    
    // 1. 检查电机控制器状态
    try {
        MotorController& motorController = MotorController::getInstance();
        MotorControllerState currentState = motorController.getCurrentState();
        
        if (currentState == MotorControllerState::ERROR_STATE) {
            LOG_WARN("检测到电机控制器处于错误状态，尝试恢复");
        }
    } catch (...) {
        LOG_ERROR("检查电机控制器状态时发生异常");
    }
    
    // 2. 保存未保存的配置
    try {
        ConfigManager& configManager = ConfigManager::getInstance();
        if (configManager.isConfigModified()) {
            LOG_INFO("BLE断连时保存未保存的配置更改");
            configManager.saveConfig();
        }
    } catch (...) {
        LOG_ERROR("保存配置时发生异常");
    }
    
    // 3. 检查系统状态
    try {
        StateManager& stateManager = StateManager::getInstance();
        SystemState currentState = stateManager.getCurrentState();
        LOG_INFO("系统状态正常: %s", StateManager::getStateName(currentState).c_str());
    } catch (...) {
        LOG_ERROR("检查系统状态时发生异常");
    }
}
```

#### 智能重连策略

```cpp
bool MotorBLEServer::shouldAttemptReconnection() {
    uint32_t currentTime = millis();
    
    // 如果断连次数过多且时间间隔太短，暂时不重连
    if (disconnectionCount > 5 && 
        (currentTime - lastConnectionTime) < RECONNECTION_TIMEOUT) {
        LOG_WARN("断连频繁，暂缓重连尝试");
        return false;
    }
    
    return true;
}
```

#### 连接状态重置

```cpp
void MotorBLEServer::resetConnectionState() {
    LOG_INFO("重置BLE连接状态");
    
    disconnectionCount = 0;
    disconnectionHandled = false;
    lastConnectionTime = 0;
    
    // 清除错误状态
    memset(lastError, 0, sizeof(lastError));
}
```

### 断连场景处理

**正常断连**:
- 客户端主动断开连接
- 信号范围外断连
- 设备休眠导致的断连

**异常断连**:
- 网络故障
- 设备故障
- 频繁断连重连

**处理策略**:
1. 立即保存当前状态和配置
2. 确保核心功能继续运行
3. 更新LED状态指示
4. 启动重连机制
5. 记录断连统计信息

## 错误处理测试

### 测试覆盖

**位置**: [`src/tests/ErrorHandlingTest.cpp`](../src/tests/ErrorHandlingTest.cpp)

测试包括：

1. **模块初始化失败测试**
   - 重试机制验证
   - 安全模式激活测试
   - 非关键模块失败处理

2. **参数验证测试**
   - 运行时长越界检查
   - 停止时长越界检查
   - 循环次数越界检查
   - 参数自动修正功能

3. **BLE断连测试**
   - 断连处理机制
   - 系统稳定性保证
   - 重连机制验证

### 测试执行

```cpp
// 创建测试实例
ErrorHandlingTest errorTest;

// 运行所有测试
errorTest.runAllTests();

// 检查测试结果
if (errorTest.allTestsPassed()) {
    LOG_INFO("所有错误处理功能测试通过！");
} else {
    LOG_WARN("有%d个测试失败", errorTest.getFailedCount());
}
```

## 日志和监控

### 错误日志格式

```
[时间戳] [级别] [模块] 错误描述
[2025-08-06 15:30:15] [ERROR] [MainController] LED控制器初始化失败，进入安全模式
[2025-08-06 15:30:16] [WARN] [ConfigManager] 配置参数越界，已自动修正: 运行时长过小，已修正为100ms
[2025-08-06 15:30:17] [INFO] [MotorBLEServer] BLE客户端已断开 (第3次断连)
```

### 监控指标

- 模块初始化成功率
- 参数修正频率
- BLE断连次数和频率
- 系统安全模式激活次数
- 错误恢复成功率

## 最佳实践

### 开发建议

1. **错误处理优先级**
   - 关键模块失败 > 非关键模块失败
   - 数据完整性 > 功能可用性
   - 系统稳定性 > 性能优化

2. **日志记录**
   - 详细记录错误发生的上下文
   - 包含错误恢复的具体步骤
   - 提供足够信息用于问题诊断

3. **测试策略**
   - 模拟各种失败场景
   - 验证错误恢复机制
   - 确保系统在错误状态下的稳定性

### 维护指南

1. **定期检查**
   - 监控错误日志
   - 分析错误模式
   - 优化错误处理策略

2. **版本更新**
   - 保持错误处理机制的向后兼容性
   - 更新测试用例覆盖新功能
   - 文档同步更新

## 总结

ESP32电机控制系统的错误处理机制提供了全面的故障检测、处理和恢复能力。通过模块化的设计和分层的错误处理策略，系统能够在各种异常情况下保持稳定运行，确保核心功能的可用性。

错误处理机制的实现遵循了以下原则：
- **渐进式降级**: 优先保证核心功能
- **自动恢复**: 减少人工干预需求
- **详细日志**: 便于问题诊断和系统监控
- **测试覆盖**: 确保错误处理逻辑的正确性

这些机制共同构成了一个健壮、可靠的嵌入式控制系统。