#include <Arduino.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "drivers/GPIODriver.h"
#include "tests/GPIOTest.h"

// 全局对象
GPIODriver gpioDriver;
GPIOTest gpioTest(&gpioDriver);

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG);
    Logger::getInstance().info("System", "=== ESP32-S3-Zero GPIO驱动测试程序 ===");
    Logger::getInstance().info("System", "固件版本: 1.0.0");
    Logger::getInstance().info("System", "编译时间: " __DATE__ " " __TIME__);
    
    // 执行GPIO驱动初始化测试
    bool testResult = gpioTest.initializeTest();
    if (!testResult) {
        Logger::getInstance().error("System", "初始化测试失败，程序终止");
        return;
    }
}

void loop() {
    // 运行循环测试
    gpioTest.runLoopTest();
    
    // 短暂延时，避免过度占用CPU
    delay(10);
}