#include <Arduino.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "controllers/MotorController.h"
#include "controllers/MotorBLEServer.h"
#include "controllers/LEDController.h"
#include "controllers/ConfigManager.h"

// 全局对象
MotorController& motorController = MotorController::getInstance();
MotorBLEServer& bleServer = MotorBLEServer::getInstance();
ConfigManager& configManager = ConfigManager::getInstance();
LEDController ledController;

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 配置日志系统
    LoggerConfig logConfig;
    logConfig.showTimestamp = LOG_SHOW_TIMESTAMP;
    logConfig.showLevel = LOG_SHOW_LEVEL;
    logConfig.showTag = LOG_SHOW_TAG;
    logConfig.useColors = LOG_ENABLE_COLORS;
    logConfig.useMilliseconds = LOG_SHOW_MILLISECONDS;
    logConfig.bufferSize = LOG_BUFFER_SIZE;
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LOG_DEFAULT_LEVEL, logConfig);
    
    LOG_TAG_INFO("System", "=== ESP32 电机控制系统启动 ===");
    LOG_TAG_INFO("System", "固件版本: 1.0.0");
    LOG_TAG_INFO("System", "编译时间: " __DATE__ " " __TIME__);
    LOG_TAG_INFO("System", "生产环境模式");
    
    // 初始化配置管理器
    LOG_TAG_INFO("System", "初始化配置管理器...");
    if (!configManager.init()) {
        LOG_TAG_ERROR("System", "❌ 配置管理器初始化失败: %s", configManager.getLastError());
        return;
    }
    LOG_TAG_INFO("System", "✅ 配置管理器初始化成功");
    
    // 初始化LED控制器
    LOG_TAG_INFO("System", "初始化LED控制器...");
    if (!ledController.init()) {
        LOG_TAG_ERROR("System", "❌ LED控制器初始化失败");
        return;
    }
    LOG_TAG_INFO("System", "✅ LED控制器初始化成功");
    
    // 设置系统初始化状态
    ledController.setState(LEDState::SYSTEM_INIT);
    
    // 初始化电机控制器
    LOG_TAG_INFO("System", "初始化电机控制器...");
    if (!motorController.init()) {
        LOG_TAG_ERROR("System", "❌ 电机控制器初始化失败: %s", motorController.getLastError());
        ledController.setState(LEDState::ERROR_STATE);
        return;
    }
    LOG_TAG_INFO("System", "✅ 电机控制器初始化成功");
    
    // 初始化BLE服务器
    LOG_TAG_INFO("System", "初始化BLE服务器...");
    if (!bleServer.init()) {
        LOG_TAG_ERROR("System", "❌ BLE服务器初始化失败: %s", bleServer.getLastError());
        ledController.setState(LEDState::ERROR_STATE);
        return;
    }
    LOG_TAG_INFO("System", "✅ BLE服务器初始化成功");
    
    // 启动BLE服务
    bleServer.start();
    LOG_TAG_INFO("System", "✅ BLE服务已启动");
    LOG_TAG_INFO("System", "设备名称: ESP32-Motor-Control");
    
    // 设置BLE断开状态（等待连接）
    ledController.setState(LEDState::BLE_DISCONNECTED);
    
    LOG_TAG_INFO("System", "🚀 系统初始化完成，等待BLE客户端连接...");
}

void loop() {
    // 更新BLE服务器状态
    bleServer.update();
    
    // 更新电机控制器
    motorController.update();
    
    // 根据BLE连接状态更新LED
    static bool lastBleConnected = false;
    bool currentBleConnected = bleServer.isConnected();
    
    if (currentBleConnected != lastBleConnected) {
        if (currentBleConnected) {
            LOG_TAG_INFO("System", "📱 BLE客户端已连接");
            ledController.setState(LEDState::BLE_CONNECTED);
        } else {
            LOG_TAG_INFO("System", "📱 BLE客户端已断开");
            ledController.setState(LEDState::BLE_DISCONNECTED);
        }
        lastBleConnected = currentBleConnected;
    }
    
    // 根据电机状态更新LED
    static MotorControllerState lastMotorState = MotorControllerState::STOPPED;
    MotorControllerState currentMotorState = motorController.getCurrentState();
    
    if (currentMotorState != lastMotorState) {
        switch (currentMotorState) {
            case MotorControllerState::RUNNING:
                if (currentBleConnected) {
                    ledController.setState(LEDState::MOTOR_RUNNING);
                }
                LOG_TAG_INFO("System", "⚡ 电机开始运行");
                break;
                
            case MotorControllerState::STOPPED:
                if (currentBleConnected) {
                    ledController.setState(LEDState::MOTOR_STOPPED);
                }
                LOG_TAG_INFO("System", "⏹️ 电机已停止");
                break;
                
            case MotorControllerState::STOPPING:
                LOG_TAG_INFO("System", "⏸️ 电机正在停止");
                break;
                
            case MotorControllerState::STARTING:
                LOG_TAG_INFO("System", "🚀 电机正在启动");
                break;
                
            case MotorControllerState::ERROR_STATE:
                ledController.setState(LEDState::ERROR_STATE);
                LOG_TAG_ERROR("System", "❌ 电机出现错误");
                break;
                
            default:
                break;
        }
        lastMotorState = currentMotorState;
    }
    
    // 更新LED控制器
    ledController.update();
    
    // 定期输出系统状态（每30秒）
    static unsigned long lastStatusUpdate = 0;
    if (millis() - lastStatusUpdate > 30000) {
        lastStatusUpdate = millis();
        
        LOG_TAG_INFO("System", "📊 系统状态报告:");
        LOG_TAG_INFO("System", "  - BLE连接: %s", currentBleConnected ? "已连接" : "未连接");
        LOG_TAG_INFO("System", "  - 电机状态: %d", static_cast<int>(currentMotorState));
        LOG_TAG_INFO("System", "  - 运行时间: %lu ms", millis());
        LOG_TAG_INFO("System", "  - 空闲内存: %d bytes", ESP.getFreeHeap());
    }
    
    // 短暂延时，避免CPU占用过高
    delay(10);
}