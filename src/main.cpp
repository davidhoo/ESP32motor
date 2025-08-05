#include <Arduino.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "drivers/GPIODriver.h"
#include "drivers/TimerDriver.h"
#include "tests/GPIOTest.h"
#include "tests/TimerTest.h"
#include "drivers/WS2812Driver.h"
#include "tests/WS2812Test.h"

// 全局对象
GPIODriver gpioDriver;
GPIOTest gpioTest(&gpioDriver);
TimerTest timerTest;
WS2812Driver ws2812Driver(21, 1);  // GPIO 21, 1个LED
WS2812Test ws2812Test(&ws2812Driver);

// 测试模式选择
enum TestMode {
    GPIO_TEST_MODE = 0,
    TIMER_TEST_MODE = 1,
    COMBINED_TEST_MODE = 2,
    WS2812_TEST_MODE = 3
};

// 当前测试模式
TestMode currentTestMode = WS2812_TEST_MODE; // 默认运行WS2812测试

// 函数声明
void runGPIOTests();
void runTimerTests();
void runCombinedTests();
void runWS2812Tests();

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG);
    Logger::getInstance().info(String("System"), "=== ESP32-S3-Zero 驱动测试程序 ===");
    Logger::getInstance().info(String("System"), "固件版本: 1.0.0");
    Logger::getInstance().info(String("System"), "编译时间: " __DATE__ " " __TIME__);
    
    // 根据测试模式执行相应的测试
    switch (currentTestMode) {
        case GPIO_TEST_MODE:
            Logger::getInstance().info(String("System"), "运行模式: GPIO驱动测试");
            runGPIOTests();
            break;
            
        case TIMER_TEST_MODE:
            Logger::getInstance().info(String("System"), "运行模式: 定时器驱动测试");
            runTimerTests();
            break;
            
        case COMBINED_TEST_MODE:
            Logger::getInstance().info(String("System"), "运行模式: 综合驱动测试");
            runCombinedTests();
            break;
            
        case WS2812_TEST_MODE:
            Logger::getInstance().info(String("System"), "运行模式: WS2812驱动测试");
            runWS2812Tests();
            break;
            
        default:
            Logger::getInstance().error(String("System"), "未知的测试模式");
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
            Logger::getInstance().debug(String("System"), "定时器测试运行中...");
            break;
            
        case COMBINED_TEST_MODE:
            // 运行综合测试
            gpioTest.runLoopTest();
            delay(100);
            break;
            
        default:
            delay(1000);
            break;
            
        case WS2812_TEST_MODE:
            // 运行WS2812循环测试
            ws2812Test.runLoopTest();
            delay(10);
            break;
    }
}

/**
 * 运行GPIO测试
 */
void runGPIOTests() {
    Logger::getInstance().info(String("System"), "开始GPIO驱动测试");
    
    bool testResult = gpioTest.initializeTest();
    if (!testResult) {
        Logger::getInstance().error(String("System"), "GPIO初始化测试失败");
        return;
    }
    
    Logger::getInstance().info(String("System"), "GPIO驱动测试初始化完成");
}

/**
 * 运行定时器测试
 */
void runTimerTests() {
    Logger::getInstance().info(String("System"), "开始定时器驱动测试");
    
    // 运行所有定时器测试
    bool testResult = timerTest.runAllTests();
    
    if (testResult) {
        Logger::getInstance().info(String("System"), "定时器驱动测试全部通过！");
    } else {
        Logger::getInstance().error(String("System"), "定时器驱动测试存在失败项");
    }
    
    Logger::getInstance().info(String("System"), "定时器驱动测试完成");
}

/**
 * 运行综合测试
 */
void runCombinedTests() {
    Logger::getInstance().info(String("System"), "开始综合驱动测试");
    
    // 先运行GPIO测试
    bool gpioResult = gpioTest.initializeTest();
    if (!gpioResult) {
        Logger::getInstance().error(String("System"), "GPIO测试失败");
        return;
    }
    
    // 再运行定时器测试
    bool timerResult = timerTest.runAllTests();
    
    // 最后运行WS2812测试
    bool ws2812Result = ws2812Test.initializeTest();
    if (!ws2812Result) {
        Logger::getInstance().error(String("System"), "WS2812测试失败");
        return;
    }
    
    // 输出综合测试结果
    if (gpioResult && timerResult && ws2812Result) {
        Logger::getInstance().info(String("System"), "综合驱动测试全部通过！");
    } else {
        Logger::getInstance().error(String("System"), "综合驱动测试存在失败项");
        Logger::getInstance().info(String("System"), ("GPIO测试: " + String(gpioResult ? "通过" : "失败")).c_str());
        Logger::getInstance().info(String("System"), ("定时器测试: " + String(timerResult ? "通过" : "失败")).c_str());
        Logger::getInstance().info(String("System"), ("WS2812测试: " + String(ws2812Result ? "通过" : "失败")).c_str());
    }
    
    Logger::getInstance().info(String("System"), "综合驱动测试完成");
}

/**
 * 运行WS2812测试
 */
void runWS2812Tests() {
    Logger::getInstance().info(String("System"), "开始WS2812驱动测试");
    
    bool testResult = ws2812Test.initializeTest();
    if (!testResult) {
        Logger::getInstance().error(String("System"), "WS2812初始化测试失败");
        return;
    }
    
    Logger::getInstance().info(String("System"), "WS2812驱动测试初始化完成");
}