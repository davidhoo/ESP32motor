#include <Arduino.h>
#include "controllers/BLEServer.h"
#include "controllers/ConfigManager.h"
#include "controllers/MotorController.h"
#include "controllers/LEDController.h"
#include "common/Logger.h"

// 全局对象
BLEServer bleServer;
LEDController ledController;

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 配置日志系统
    LoggerConfig logConfig;
    logConfig.showTimestamp = true;
    logConfig.showLevel = true;
    logConfig.showTag = true;
    logConfig.useColors = true;
    logConfig.useMilliseconds = true;
    logConfig.bufferSize = 256;
    
    Logger::getInstance().begin(&Serial, LogLevel::INFO, logConfig);
    
    LOG_TAG_INFO("Main", "=== ESP32-S3-Zero BLE服务器示例 ===");
    LOG_TAG_INFO("Main", "固件版本: 1.0.0");
    
    // 初始化LED控制器
    LOG_TAG_INFO("Main", "初始化LED控制器...");
    if (ledController.init()) {
        ledController.setState(LEDState::SYSTEM_INIT);
        LOG_TAG_INFO("Main", "✅ LED控制器初始化成功");
    } else {
        LOG_TAG_ERROR("Main", "❌ LED控制器初始化失败");
    }
    
    // 初始化配置管理器
    ConfigManager& configManager = ConfigManager::getInstance();
    LOG_TAG_INFO("Main", "初始化配置管理器...");
    if (configManager.init()) {
        LOG_TAG_INFO("Main", "✅ 配置管理器初始化成功");
        
        // 设置默认配置
        MotorConfig config = configManager.getConfig();
        config.runDuration = 10;      // 10秒运行
        config.stopDuration = 5;      // 5秒停止
        configManager.updateConfig(config);
        configManager.saveConfig();
        LOG_TAG_INFO("Main", "默认配置已设置: 运行10秒，停止5秒");
    } else {
        LOG_TAG_ERROR("Main", "❌ 配置管理器初始化失败");
    }
    
    // 初始化电机控制器
    MotorController& motorController = MotorController::getInstance();
    LOG_TAG_INFO("Main", "初始化电机控制器...");
    if (motorController.init()) {
        LOG_TAG_INFO("Main", "✅ 电机控制器初始化成功");
    } else {
        LOG_TAG_ERROR("Main", "❌ 电机控制器初始化失败");
    }
    
    // 初始化BLE服务器
    LOG_TAG_INFO("Main", "初始化BLE服务器...");
    if (bleServer.init()) {
        bleServer.start();
        LOG_TAG_INFO("Main", "✅ BLE服务器已启动");
        LOG_TAG_INFO("Main", "设备名称: ESP32-Motor-Control");
        LOG_TAG_INFO("Main", "服务UUID: 4fafc201-1fb5-459e-8fcc-c5c9c331914b");
        LOG_TAG_INFO("Main", "等待BLE客户端连接...");
        
        ledController.setState(LEDState::BLE_CONNECTED);
    } else {
        LOG_TAG_ERROR("Main", "❌ BLE服务器初始化失败: %s", bleServer.getLastError());
        ledController.setState(LEDState::ERROR_STATE);
    }
}

void loop() {
    // 更新BLE服务器
    bleServer.update();
    
    // 更新LED控制器
    ledController.update();
    
    // 更新电机控制器
    MotorController& motorController = MotorController::getInstance();
    motorController.update();
    
    // 根据连接状态更新LED
    static bool lastConnected = false;
    bool currentConnected = bleServer.isConnected();
    
    if (currentConnected != lastConnected) {
        if (currentConnected) {
            ledController.setState(LEDState::BLE_CONNECTED);
            LOG_TAG_INFO("Main", "BLE客户端已连接");
        } else {
            ledController.setState(LEDState::BLE_DISCONNECTED);
            LOG_TAG_INFO("Main", "BLE客户端已断开");
        }
        lastConnected = currentConnected;
    }
    
    // 每5秒显示一次状态
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) {
        lastStatus = millis();
        
        MotorControllerState state = motorController.getCurrentState();
        uint32_t runTime = motorController.getRemainingRunTime();
        uint32_t stopTime = motorController.getRemainingStopTime();
        uint32_t cycles = motorController.getCurrentCycleCount();
        
        LOG_TAG_INFO("Main", "状态: %d, 运行剩余: %us, 停止剩余: %us, 循环: %lu, BLE: %s",
                     static_cast<int>(state), runTime, stopTime, cycles,
                     currentConnected ? "已连接" : "未连接");
    }
    
    delay(100);
}