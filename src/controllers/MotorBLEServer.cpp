#include "MotorBLEServer.h"
#include "../controllers/MotorController.h"
#include "../controllers/ConfigManager.h"
#include "../common/Logger.h"
#include "../common/EventManager.h"
#include <ArduinoJson.h>

// 单例实例
MotorBLEServer& MotorBLEServer::getInstance() {
    static MotorBLEServer instance;
    return instance;
}

// 构造函数
MotorBLEServer::MotorBLEServer() : stateManager(StateManager::getInstance()) {
    disconnectionHandled = false;
    lastConnectionTime = 0;
    disconnectionCount = 0;
}

// 初始化BLE服务器
bool MotorBLEServer::init() {
    LOG_INFO("初始化BLE服务器...");
    
    try {
        // 初始化BLE设备
        BLEDevice::init(DEVICE_NAME);
        
        // 创建BLE服务器
        pServer = BLEDevice::createServer();
        if (!pServer) {
            setError("创建BLE服务器失败");
            return false;
        }
        
        // 设置服务器回调
        pServer->setCallbacks(new ServerCallbacks(this));
        
        // 创建BLE服务
        pService = pServer->createService(SERVICE_UUID);
        if (!pService) {
            setError("创建BLE服务失败");
            return false;
        }
        
        // 创建运行时长特征值
        pRunDurationCharacteristic = pService->createCharacteristic(
            RUN_DURATION_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pRunDurationCharacteristic->setCallbacks(new CharacteristicCallbacks(this, RUN_DURATION_CHAR_UUID));
        
        // 创建停止间隔特征值
        pStopIntervalCharacteristic = pService->createCharacteristic(
            STOP_INTERVAL_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pStopIntervalCharacteristic->setCallbacks(new CharacteristicCallbacks(this, STOP_INTERVAL_CHAR_UUID));
        
        // 创建系统控制特征值
        pSystemControlCharacteristic = pService->createCharacteristic(
            SYSTEM_CONTROL_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pSystemControlCharacteristic->setCallbacks(new CharacteristicCallbacks(this, SYSTEM_CONTROL_CHAR_UUID));
        
        // 创建状态查询特征值
        pStatusQueryCharacteristic = pService->createCharacteristic(
            STATUS_QUERY_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pStatusQueryCharacteristic->setCallbacks(new CharacteristicCallbacks(this, STATUS_QUERY_CHAR_UUID));
        
        // 设置初始值
        pRunDurationCharacteristic->setValue("0");
        pStopIntervalCharacteristic->setValue("0");
        pSystemControlCharacteristic->setValue("0");
        pStatusQueryCharacteristic->setValue(generateStatusJson().c_str());
        
        // 注册系统状态变更监听器
        stateManager.registerStateListener([this](const StateChangeEvent& event) {
            this->onSystemStateChanged(event);
        });
        
        LOG_INFO("BLE服务器初始化成功");
        return true;
        
    } catch (const std::exception& e) {
        snprintf(lastError, sizeof(lastError), "初始化异常: %s", e.what());
        LOG_ERROR("BLE服务器初始化失败: %s", lastError);
        return false;
    }
}

// 启动BLE服务
void MotorBLEServer::start() {
    if (!pService) {
        setError("服务未初始化");
        return;
    }
    
    pService->start();
    
    // 启动广播
    BLEAdvertising* pAdvertising = BLEDevice::getAdvertising();
    if (pAdvertising) {
        pAdvertising->addServiceUUID(SERVICE_UUID);
        pAdvertising->setScanResponse(false);
        pAdvertising->setMinPreferred(0x0);
        BLEDevice::startAdvertising();
        LOG_INFO("BLE广播已启动");
    }
}

// 停止BLE服务
void MotorBLEServer::stop() {
    BLEDevice::stopAdvertising();
    LOG_INFO("BLE服务已停止");
}

// 更新BLE状态
void MotorBLEServer::update() {
    if (!isConnected()) {
        return;
    }
    
    // === 5.3.3 实时状态推送机制 ===
    static uint32_t lastStatusUpdate = 0;
    static uint32_t statusUpdateInterval = 1000; // 1秒定时推送间隔
    
    uint32_t currentTime = millis();
    
    // 定时状态推送（每秒推送一次，确保客户端获得最新状态）
    if (currentTime - lastStatusUpdate >= statusUpdateInterval) {
        String statusJson = generateStatusJson();
        sendStatusNotification(statusJson);
        lastStatusUpdate = currentTime;
        
        // 动态调整推送频率：电机运行时更频繁推送
        MotorController& motorController = MotorController::getInstance();
        if (motorController.isRunning()) {
            statusUpdateInterval = 500; // 运行时每0.5秒推送
        } else {
            statusUpdateInterval = 2000; // 停止时每2秒推送
        }
    }
}

// 获取连接状态
bool MotorBLEServer::isConnected() const {
    return deviceConnected;
}

// 发送状态通知
void MotorBLEServer::sendStatusNotification(const String& status) {
    if (pStatusQueryCharacteristic && isConnected()) {
        pStatusQueryCharacteristic->setValue(status.c_str());
        pStatusQueryCharacteristic->notify();
    }
}

// 服务器连接回调
void MotorBLEServer::ServerCallbacks::onConnect(BLEServer* pServer) {
    bleServer->deviceConnected = true;
    bleServer->lastConnectionTime = millis();
    bleServer->disconnectionHandled = false;
    LOG_INFO("BLE客户端已连接");
    
    // === 5.3.3 实时状态推送机制 - 发布BLE连接事件 ===
    EventManager::getInstance().publish(EventData(
        EventType::BLE_CONNECTED,
        "MotorBLEServer",
        "BLE客户端连接成功"
    ));
}

void MotorBLEServer::ServerCallbacks::onDisconnect(BLEServer* pServer) {
    bleServer->deviceConnected = false;
    bleServer->disconnectionCount++;
    LOG_INFO("BLE客户端已断开 (第%lu次断连)", bleServer->disconnectionCount);
    
    // === 5.4.3 BLE断连时的系统稳定运行机制 ===
    bleServer->handleDisconnection();
    
    // === 5.3.3 实时状态推送机制 - 发布BLE断开事件 ===
    EventManager::getInstance().publish(EventData(
        EventType::BLE_DISCONNECTED,
        "MotorBLEServer",
        "BLE客户端连接断开"
    ));
    
    // 重新启动广播
    BLEDevice::startAdvertising();
}

// 特征值读写回调
void MotorBLEServer::CharacteristicCallbacks::onWrite(BLECharacteristic* pCharacteristic) {
    std::string value = pCharacteristic->getValue();
    if (value.length() == 0) {
        return;
    }
    
    String strValue = String(value.c_str());
    LOG_INFO("收到BLE写入: %s = %s", charUUID, strValue.c_str());
    
    if (strcmp(charUUID, RUN_DURATION_CHAR_UUID) == 0) {
        bleServer->handleRunDurationWrite(strValue);
    } else if (strcmp(charUUID, STOP_INTERVAL_CHAR_UUID) == 0) {
        bleServer->handleStopIntervalWrite(strValue);
    } else if (strcmp(charUUID, SYSTEM_CONTROL_CHAR_UUID) == 0) {
        bleServer->handleSystemControlWrite(strValue);
    }
}

void MotorBLEServer::CharacteristicCallbacks::onRead(BLECharacteristic* pCharacteristic) {
    if (strcmp(charUUID, STATUS_QUERY_CHAR_UUID) == 0) {
        String statusJson = bleServer->generateStatusJson();
        pCharacteristic->setValue(statusJson.c_str());
    }
}

// 处理运行时长写入
// 处理运行时长写入
void MotorBLEServer::handleRunDurationWrite(const String& value) {
    try {
        uint32_t runDuration = atoi(value.c_str());
        if (runDuration < 1 || runDuration > 999) {
            LOG_ERROR("运行时长超出范围: %u (有效范围: 1-999秒)", runDuration);
            return;
        }
        
        ConfigManager& configManager = ConfigManager::getInstance();
        MotorConfig currentConfig = configManager.getConfig();
        currentConfig.runDuration = runDuration;  // 直接使用秒为单位
        
        configManager.updateConfig(currentConfig);
        configManager.saveConfig();
        
        // 立即通知电机控制器应用新配置
        MotorController& motorController = MotorController::getInstance();
        motorController.updateConfig(currentConfig);
        
        LOG_INFO("运行时长已更新: %u 秒", runDuration);
        
        // 立即推送更新后的状态
        if (this->isConnected()) {
            String statusJson = this->generateStatusJson();
            this->sendStatusNotification(statusJson);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("处理运行时长写入异常: %s", e.what());
    }
}
// 处理停止间隔写入
// 处理停止间隔写入
void MotorBLEServer::handleStopIntervalWrite(const String& value) {
    try {
        uint32_t stopInterval = atoi(value.c_str());
        if (stopInterval < 1 || stopInterval > 999) {
            LOG_ERROR("停止间隔超出范围: %u (有效范围: 1-999秒)", stopInterval);
            return;
        }
        
        ConfigManager& configManager = ConfigManager::getInstance();
        MotorConfig currentConfig = configManager.getConfig();
        currentConfig.stopDuration = stopInterval;  // 直接使用秒为单位
        
        configManager.updateConfig(currentConfig);
        configManager.saveConfig();
        
        // 立即通知电机控制器应用新配置
        MotorController& motorController = MotorController::getInstance();
        motorController.updateConfig(currentConfig);
        
        LOG_INFO("停止间隔已更新: %u 秒", stopInterval);
        
        // 立即推送更新后的状态
        if (this->isConnected()) {
            String statusJson = this->generateStatusJson();
            this->sendStatusNotification(statusJson);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("处理停止间隔写入异常: %s", e.what());
    }
}
// 处理系统控制写入
void MotorBLEServer::handleSystemControlWrite(const String& value) {
    try {
        uint8_t control = atoi(value.c_str());
        if (control > 1) {
            LOG_ERROR("系统控制值无效: %u (有效值: 0=停止, 1=启动)", control);
            return;
        }
        
        MotorController& motorController = MotorController::getInstance();
        
        if (control == 1) {
            // 启动命令
            bool success = motorController.startMotor();
            if (success) {
                LOG_INFO("系统控制: 启动命令执行成功");
            } else {
                LOG_ERROR("系统控制: 启动命令执行失败: %s", motorController.getLastError());
            }
        } else {
            // 停止命令
            bool success = motorController.stopMotor();
            if (success) {
                LOG_INFO("系统控制: 停止命令执行成功");
            } else {
                LOG_ERROR("系统控制: 停止命令执行失败: %s", motorController.getLastError());
            }
        }
        
        // 立即推送更新后的状态
        if (this->isConnected()) {
            String statusJson = this->generateStatusJson();
            this->sendStatusNotification(statusJson);
        }
    } catch (const std::exception& e) {
        LOG_ERROR("处理系统控制写入异常: %s", e.what());
    }
}

// 生成状态JSON
String MotorBLEServer::generateStatusJson() {
    MotorController& motorController = MotorController::getInstance();
    
    DynamicJsonDocument doc(512);
    
    // 电机状态
    MotorControllerState state = motorController.getCurrentState();
    doc["state"] = static_cast<int>(state);
    
    // 状态名称映射
    const char* stateNames[] = {"STOPPED", "RUNNING", "STOPPING", "STARTING", "ERROR"};
    int stateIndex = static_cast<int>(state);
    if (stateIndex >= 0 && stateIndex < 5) {
        doc["stateName"] = stateNames[stateIndex];
    } else {
        doc["stateName"] = "UNKNOWN";
    }
    
    // 时间信息
    doc["remainingRunTime"] = motorController.getRemainingRunTime();
    doc["remainingStopTime"] = motorController.getRemainingStopTime();
    doc["currentCycleCount"] = motorController.getCurrentCycleCount();
    
    // 配置信息
    // 配置信息
    ConfigManager& configManager = ConfigManager::getInstance();
    MotorConfig config = configManager.getConfig();
    doc["runDuration"] = config.runDuration;  // 直接使用秒
    doc["stopDuration"] = config.stopDuration;  // 直接使用秒
    doc["cycleCount"] = config.cycleCount;
    doc["autoStart"] = config.autoStart;
    // 系统信息
    doc["uptime"] = millis();
    doc["freeHeap"] = ESP.getFreeHeap();
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

// 生成信息JSON
String MotorBLEServer::generateInfoJson() {
    DynamicJsonDocument doc(256);
    
    doc["deviceName"] = DEVICE_NAME;
    doc["serviceUUID"] = SERVICE_UUID;
    doc["firmwareVersion"] = "1.0.0";
    doc["hardware"] = "ESP32-S3-Zero";
    doc["features"] = "Motor Control, LED Status, BLE Communication";
    
    String jsonStr;
    serializeJson(doc, jsonStr);
    return jsonStr;
}

// 设置错误信息
void MotorBLEServer::setError(const char* error) {
    strncpy(lastError, error, sizeof(lastError) - 1);
    lastError[sizeof(lastError) - 1] = '\0';
    LOG_ERROR("BLE错误: %s", error);
}

// 系统状态变更回调
void MotorBLEServer::onSystemStateChanged(const StateChangeEvent& event) {
    LOG_INFO("BLE服务器收到系统状态变更: %s -> %s",
             StateManager::getStateName(event.oldState).c_str(),
             StateManager::getStateName(event.newState).c_str());
    
    // === 5.3.3 实时状态推送机制 - 事件驱动推送 ===
    // 如果有客户端连接，立即发送状态更新
    if (isConnected()) {
        // 生成包含系统状态变更信息的完整状态JSON
        DynamicJsonDocument doc(1024);
        
        // 首先获取基础状态信息
        String baseStatusJson = generateStatusJson();
        deserializeJson(doc, baseStatusJson);
        
        // 添加系统状态变更信息
        doc["systemState"] = StateManager::getStateName(event.newState);
        doc["systemStateReason"] = event.reason;
        doc["systemStateTimestamp"] = event.timestamp;
        doc["eventType"] = "system_state_change";
        doc["eventTime"] = millis();
        
        // 添加状态变更详情
        JsonObject stateChange = doc.createNestedObject("stateChange");
        stateChange["from"] = StateManager::getStateName(event.oldState);
        stateChange["to"] = StateManager::getStateName(event.newState);
        stateChange["reason"] = event.reason;
        
        String updatedJson;
        serializeJson(doc, updatedJson);
        sendStatusNotification(updatedJson);
        
        LOG_INFO("系统状态变更已实时推送给BLE客户端");
    }
}

// === 5.4.3 BLE断连时的系统稳定运行机制 ===

/**
 * 处理BLE断连事件
 */
void MotorBLEServer::handleDisconnection() {
    if (disconnectionHandled) {
        return; // 避免重复处理
    }
    
    disconnectionHandled = true;
    LOG_INFO("处理BLE断连事件，确保系统稳定运行");
    
    // 确保系统核心功能继续运行
    ensureSystemStability();
    
    // 记录断连时间和次数
    uint32_t currentTime = millis();
    uint32_t connectionDuration = currentTime - lastConnectionTime;
    
    LOG_INFO("BLE连接持续时间: %lu ms, 累计断连次数: %lu",
             connectionDuration, disconnectionCount);
    
    // 如果断连次数过多，可能需要重置BLE服务
    if (disconnectionCount > 10) {
        LOG_WARN("BLE断连次数过多，考虑重置BLE服务");
        // 这里可以添加BLE服务重置逻辑
        resetConnectionState();
    }
}

/**
 * 确保系统稳定性
 */
void MotorBLEServer::ensureSystemStability() {
    LOG_INFO("确保BLE断连后系统稳定运行");
    
    // 1. 确保电机控制器继续正常工作
    try {
        MotorController& motorController = MotorController::getInstance();
        MotorControllerState currentState = motorController.getCurrentState();
        
        if (currentState == MotorControllerState::ERROR_STATE) {
            LOG_WARN("检测到电机控制器处于错误状态，尝试恢复");
            // 这里可以添加电机控制器恢复逻辑
        } else {
            LOG_INFO("电机控制器状态正常: %d", static_cast<int>(currentState));
        }
    } catch (...) {
        LOG_ERROR("检查电机控制器状态时发生异常");
    }
    
    // 2. 确保配置管理器继续工作
    try {
        ConfigManager& configManager = ConfigManager::getInstance();
        if (configManager.isConfigModified()) {
            LOG_INFO("BLE断连时保存未保存的配置更改");
            configManager.saveConfig();
        }
    } catch (...) {
        LOG_ERROR("保存配置时发生异常");
    }
    
    // 3. 确保系统状态管理器正常
    try {
        StateManager& stateManager = StateManager::getInstance();
        SystemState currentState = stateManager.getCurrentState();
        LOG_INFO("系统状态正常: %s", StateManager::getStateName(currentState).c_str());
        
        // 如果系统处于错误状态，尝试恢复
        if (currentState == SystemState::ERROR) {
            LOG_WARN("系统处于错误状态，BLE断连可能加剧问题");
        }
    } catch (...) {
        LOG_ERROR("检查系统状态时发生异常");
    }
    
    LOG_INFO("系统稳定性检查完成，核心功能继续运行");
}

/**
 * 检查是否应该尝试重连
 */
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

/**
 * 重置连接状态
 */
void MotorBLEServer::resetConnectionState() {
    LOG_INFO("重置BLE连接状态");
    
    disconnectionCount = 0;
    disconnectionHandled = false;
    lastConnectionTime = 0;
    
    // 清除错误状态
    memset(lastError, 0, sizeof(lastError));
    
    LOG_INFO("BLE连接状态已重置");
}