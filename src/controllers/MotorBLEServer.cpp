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
        
        // 创建配置特征值
        pConfigCharacteristic = pService->createCharacteristic(
            CONFIG_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ | 
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pConfigCharacteristic->setCallbacks(new CharacteristicCallbacks(this, CONFIG_CHAR_UUID));
        
        // 创建命令特征值
        pCommandCharacteristic = pService->createCharacteristic(
            COMMAND_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ | 
            BLECharacteristic::PROPERTY_WRITE |
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pCommandCharacteristic->setCallbacks(new CharacteristicCallbacks(this, COMMAND_CHAR_UUID));
        
        // 创建状态特征值
        pStatusCharacteristic = pService->createCharacteristic(
            STATUS_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ | 
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pStatusCharacteristic->setCallbacks(new CharacteristicCallbacks(this, STATUS_CHAR_UUID));
        
        // 创建信息特征值
        pInfoCharacteristic = pService->createCharacteristic(
            INFO_CHAR_UUID,
            BLECharacteristic::PROPERTY_READ | 
            BLECharacteristic::PROPERTY_NOTIFY
        );
        pInfoCharacteristic->setCallbacks(new CharacteristicCallbacks(this, INFO_CHAR_UUID));
        
        // 设置初始值
        pConfigCharacteristic->setValue("{}");
        pCommandCharacteristic->setValue("{\"command\":\"none\"}");
        pStatusCharacteristic->setValue(generateStatusJson().c_str());
        pInfoCharacteristic->setValue(generateInfoJson().c_str());
        
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
    if (pStatusCharacteristic && isConnected()) {
        pStatusCharacteristic->setValue(status.c_str());
        pStatusCharacteristic->notify();
    }
}

// 服务器连接回调
void MotorBLEServer::ServerCallbacks::onConnect(BLEServer* pServer) {
    bleServer->deviceConnected = true;
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
    LOG_INFO("BLE客户端已断开");
    
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
    
    if (strcmp(charUUID, CONFIG_CHAR_UUID) == 0) {
        bleServer->handleConfigWrite(strValue);
    } else if (strcmp(charUUID, COMMAND_CHAR_UUID) == 0) {
        bleServer->handleCommandWrite(strValue);
    }
}

void MotorBLEServer::CharacteristicCallbacks::onRead(BLECharacteristic* pCharacteristic) {
    if (strcmp(charUUID, STATUS_CHAR_UUID) == 0) {
        String statusJson = bleServer->generateStatusJson();
        pCharacteristic->setValue(statusJson.c_str());
    } else if (strcmp(charUUID, INFO_CHAR_UUID) == 0) {
        String infoJson = bleServer->generateInfoJson();
        pCharacteristic->setValue(infoJson.c_str());
    }
}

// 处理配置写入
void MotorBLEServer::handleConfigWrite(const String& value) {
    try {
        DynamicJsonDocument doc(512);
        DeserializationError error = deserializeJson(doc, value);
        
        if (error) {
            LOG_ERROR("配置JSON解析失败: %s", error.c_str());
            return;
        }
        
        ConfigManager& configManager = ConfigManager::getInstance();
        MotorConfig currentConfig = configManager.getConfig();
        
        // 更新配置参数
        if (doc.containsKey("runDuration")) {
            currentConfig.runDuration = doc["runDuration"];
        }
        if (doc.containsKey("stopDuration")) {
            currentConfig.stopDuration = doc["stopDuration"];
        }
        if (doc.containsKey("cycleCount")) {
            currentConfig.cycleCount = doc["cycleCount"];
        }
        if (doc.containsKey("autoStart")) {
            currentConfig.autoStart = doc["autoStart"];
        }
        
        // 应用新配置
        // 应用新配置
        configManager.updateConfig(currentConfig);
        configManager.saveConfig();
        
        // === 5.3.1 参数设置的即时生效逻辑 ===
        // 立即通知电机控制器应用新配置
        MotorController& motorController = MotorController::getInstance();
        motorController.updateConfig(currentConfig);
        
        LOG_INFO("配置已更新并即时生效: 运行=%u, 停止=%u, 循环=%u, 自动启动=%s",
                 currentConfig.runDuration, currentConfig.stopDuration, currentConfig.cycleCount,
                 currentConfig.autoStart ? "是" : "否");
        
        // 立即推送更新后的状态给BLE客户端
        if (isConnected()) {
            String statusJson = generateStatusJson();
            sendStatusNotification(statusJson);
            LOG_INFO("配置更新后状态已推送给BLE客户端");
        }
    } catch (const std::exception& e) {
        LOG_ERROR("处理配置写入异常: %s", e.what());
    }
}

// 处理命令写入
void MotorBLEServer::handleCommandWrite(const String& value) {
    try {
        DynamicJsonDocument doc(256);
        DeserializationError error = deserializeJson(doc, value);
        
        if (error) {
            LOG_ERROR("命令JSON解析失败: %s", error.c_str());
            return;
        }
        
        const char* command = doc["command"];
        if (!command) {
            LOG_ERROR("命令格式错误");
            return;
        }
        
        // === 5.3.2 手动启动/停止命令的优先级处理 ===
        MotorController& motorController = MotorController::getInstance();
        MotorControllerState currentState = motorController.getCurrentState();
        
        if (strcmp(command, "start") == 0) {
            // 启动命令优先级处理
            if (currentState == MotorControllerState::STOPPED ||
                currentState == MotorControllerState::STOPPING) {
                bool success = motorController.startMotor();
                if (success) {
                    LOG_INFO("手动启动命令执行成功");
                    // 立即推送状态更新
                    if (isConnected()) {
                        String statusJson = generateStatusJson();
                        sendStatusNotification(statusJson);
                    }
                } else {
                    LOG_ERROR("手动启动命令执行失败: %s", motorController.getLastError());
                }
            } else if (currentState == MotorControllerState::RUNNING ||
                       currentState == MotorControllerState::STARTING) {
                LOG_WARN("电机已在运行中，忽略重复启动命令");
            } else {
                LOG_WARN("电机处于错误状态，无法启动");
            }
            
        } else if (strcmp(command, "stop") == 0) {
            // 停止命令优先级处理（最高优先级，可以中断任何状态）
            if (currentState != MotorControllerState::STOPPED) {
                bool success = motorController.stopMotor();
                if (success) {
                    LOG_INFO("手动停止命令执行成功（优先级处理）");
                    // 立即推送状态更新
                    if (isConnected()) {
                        String statusJson = generateStatusJson();
                        sendStatusNotification(statusJson);
                    }
                } else {
                    LOG_ERROR("手动停止命令执行失败: %s", motorController.getLastError());
                }
            } else {
                LOG_WARN("电机已停止，忽略重复停止命令");
            }
            
        } else if (strcmp(command, "reset") == 0) {
            // 重置命令优先级处理
            if (currentState == MotorControllerState::STOPPED) {
                motorController.resetCycleCount();
                LOG_INFO("循环计数器重置命令执行成功");
                // 立即推送状态更新
                if (isConnected()) {
                    String statusJson = generateStatusJson();
                    sendStatusNotification(statusJson);
                }
            } else {
                LOG_WARN("电机运行中，无法重置循环计数器，请先停止电机");
            }
            
        } else if (strcmp(command, "force_stop") == 0) {
            // 强制停止命令（紧急停止，最高优先级）
            motorController.stopMotor();
            LOG_INFO("强制停止命令执行（紧急停止）");
            // 立即推送状态更新
            if (isConnected()) {
                String statusJson = generateStatusJson();
                sendStatusNotification(statusJson);
            }
            
        } else {
            LOG_WARN("未知命令: %s", command);
        }
        
    } catch (const std::exception& e) {
        LOG_ERROR("处理命令写入异常: %s", e.what());
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
    ConfigManager& configManager = ConfigManager::getInstance();
    MotorConfig config = configManager.getConfig();
    doc["runDuration"] = config.runDuration;
    doc["stopDuration"] = config.stopDuration;
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