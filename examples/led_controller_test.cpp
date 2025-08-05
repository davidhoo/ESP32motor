#include <Arduino.h>
#include "controllers/LEDController.h"
#include "common/Logger.h"
#include "drivers/TimerDriver.h"

// 创建LED控制器实例
LEDController ledController;

void setup() {
    Serial.begin(115200);
    
    // 初始化日志系统
    LoggerConfig logConfig;
    logConfig.showTimestamp = true;
    logConfig.showLevel = true;
    logConfig.showTag = true;
    logConfig.useColors = true;
    logConfig.useMilliseconds = true;
    
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG, logConfig);
    
    LOG_TAG_INFO("Main", "=== ESP32 LED控制器测试程序 ===");
    LOG_TAG_INFO("Main", "固件版本: 1.0.0");
    
    // 初始化定时器驱动
    TimerDriver::getInstance().init();
    
    // 初始化LED控制器
    if (!ledController.init()) {
        LOG_TAG_ERROR("Main", "LED控制器初始化失败");
        while(1);
    }
    
    LOG_TAG_INFO("Main", "系统初始化完成");
    
    // 运行LED测试
    LOG_TAG_INFO("Main", "开始LED测试...");
    ledController.testLED();
}

void loop() {
    static unsigned long lastStateChange = 0;
    static int currentTestState = 0;
    
    // 每5秒切换一次状态
    if (millis() - lastStateChange > 5000) {
        lastStateChange = millis();
        
        switch (currentTestState) {
            case 0:
                LOG_TAG_INFO("Main", "切换到系统初始化状态");
                ledController.setState(LEDState::SYSTEM_INIT);
                break;
            case 1:
                LOG_TAG_INFO("Main", "切换到电机运行状态");
                ledController.setState(LEDState::MOTOR_RUNNING);
                break;
            case 2:
                LOG_TAG_INFO("Main", "切换到电机停止状态");
                ledController.setState(LEDState::MOTOR_STOPPED);
                break;
            case 3:
                LOG_TAG_INFO("Main", "切换到BLE连接状态");
                ledController.setState(LEDState::BLE_CONNECTED);
                break;
            case 4:
                LOG_TAG_INFO("Main", "切换到BLE断开状态");
                ledController.setState(LEDState::BLE_DISCONNECTED);
                break;
            case 5:
                LOG_TAG_INFO("Main", "切换到错误状态");
                ledController.setState(LEDState::ERROR_STATE);
                break;
        }
        
        currentTestState = (currentTestState + 1) % 6;
    }
    
    // 更新LED控制器（如果需要）
    ledController.update();
    
    delay(100);
}