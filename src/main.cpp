#include <Arduino.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "drivers/GPIODriver.h"
#include "drivers/TimerDriver.h"
#include "tests/GPIOTest.h"
#include "tests/TimerTest.h"

// 全局对象
GPIODriver gpioDriver;
GPIOTest gpioTest(&gpioDriver);
TimerTest timerTest;

// 测试模式选择
enum TestMode {
    GPIO_TEST_MODE = 0,
    TIMER_TEST_MODE = 1,
    COMBINED_TEST_MODE = 2
};

// 当前测试模式
TestMode currentTestMode = TIMER_TEST_MODE; // 默认运行定时器测试

// 函数声明
void runGPIOTests();
void runTimerTests();
void runCombinedTests();

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG);
    Logger::getInstance().info("System", "=== ESP32-S3-Zero 驱动测试程序 ===");
    Logger::getInstance().info("System", "固件版本: 1.0.0");
    Logger::getInstance().info("System", "编译时间: " __DATE__ " " __TIME__);
    
    // 根据测试模式执行相应的测试
    switch (currentTestMode) {
        case GPIO_TEST_MODE:
            Logger::getInstance().info("System", "运行模式: GPIO驱动测试");
            runGPIOTests();
            break;
            
        case TIMER_TEST_MODE:
            Logger::getInstance().info("System", "运行模式: 定时器驱动测试");
            runTimerTests();
            break;
            
        case COMBINED_TEST_MODE:
            Logger::getInstance().info("System", "运行模式: 综合驱动测试");
            runCombinedTests();
            break;
            
        default:
            Logger::getInstance().error("System", "未知的测试模式");
            break;
    }
}

void loop() {
    // 根据测试模式执行循环测试
    switch (currentTestMode) {
        case GPIO_TEST_MODE:
            // 运行GPIO循环测试
            gpioTest.runLoopTest();
            delay(10);
            break;
            
        case TIMER_TEST_MODE:
            // 定时器测试通常在setup中完成，这里只做简单的状态监控
            delay(1000);
            Logger::getInstance().debug("System", "定时器测试运行中...");
            break;
            
        case COMBINED_TEST_MODE:
            // 运行综合测试
            gpioTest.runLoopTest();
            delay(100);
            break;
            
        default:
            delay(1000);
            break;
    }
}

/**
 * 运行GPIO测试
 */
void runGPIOTests() {
    Logger::getInstance().info("System", "开始GPIO驱动测试");
    
    bool testResult = gpioTest.initializeTest();
    if (!testResult) {
        Logger::getInstance().error("System", "GPIO初始化测试失败");
        return;
    }
    
    Logger::getInstance().info("System", "GPIO驱动测试初始化完成");
}

/**
 * 运行定时器测试
 */
void runTimerTests() {
    Logger::getInstance().info("System", "开始定时器驱动测试");
    
    // 运行所有定时器测试
    bool testResult = timerTest.runAllTests();
    
    if (testResult) {
        Logger::getInstance().info("System", "定时器驱动测试全部通过！");
    } else {
        Logger::getInstance().error("System", "定时器驱动测试存在失败项");
    }
    
    Logger::getInstance().info("System", "定时器驱动测试完成");
}

/**
 * 运行综合测试
 */
void runCombinedTests() {
    Logger::getInstance().info("System", "开始综合驱动测试");
    
    // 先运行GPIO测试
    bool gpioResult = gpioTest.initializeTest();
    if (!gpioResult) {
        Logger::getInstance().error("System", "GPIO测试失败");
        return;
    }
    
    // 再运行定时器测试
    bool timerResult = timerTest.runAllTests();
    
    // 输出综合测试结果
    if (gpioResult && timerResult) {
        Logger::getInstance().info("System", "综合驱动测试全部通过！");
    } else {
        Logger::getInstance().error("System", "综合驱动测试存在失败项");
        Logger::getInstance().info("System", ("GPIO测试: " + String(gpioResult ? "通过" : "失败")).c_str());
        Logger::getInstance().info("System", ("定时器测试: " + String(timerResult ? "通过" : "失败")).c_str());
    }
    
    Logger::getInstance().info("System", "综合驱动测试完成");
}