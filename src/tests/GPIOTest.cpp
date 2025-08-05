#include "GPIOTest.h"
#include <Arduino.h>

GPIOTest::GPIOTest(GPIODriver* driver) 
    : gpioDriver(driver), lastToggle(0), motorState(false), cycleCount(0) {
}

bool GPIOTest::initializeTest() {
    Logger::getInstance().info("Test", "开始GPIO驱动测试...");
    
    // 测试GPIO驱动初始化
    bool success = true;
    
    // 测试引脚初始化
    if (!testPinInitialization()) {
        success = false;
    }
    
    // 测试引脚状态查询
    if (!testPinStatusQuery()) {
        success = false;
    }
    
    // 测试无效引脚处理
    if (!testInvalidPin()) {
        success = false;
    }
    
    if (success) {
        Logger::getInstance().info("Test", "GPIO驱动初始化测试完成");
        Logger::getInstance().info("Test", "开始电机控制循环测试...");
    } else {
        Logger::getInstance().error("Test", "GPIO驱动初始化测试失败");
    }
    
    return success;
}

bool GPIOTest::testPinInitialization() {
    // 初始化电机控制引脚 GPIO 7
    bool motorPinInit = gpioDriver->init(MOTOR_PIN, OUTPUT, MOTOR_OFF);
    if (motorPinInit) {
        Logger::getInstance().info("Test", "电机控制引脚 GPIO7 初始化成功");
        return true;
    } else {
        Logger::getInstance().error("Test", "电机控制引脚 GPIO7 初始化失败");
        return false;
    }
}

bool GPIOTest::testPinStatusQuery() {
    // 测试引脚状态查询
    if (gpioDriver->isPinInitialized(MOTOR_PIN)) {
        Logger::getInstance().info("Test", "GPIO7 已正确初始化");
        int mode = gpioDriver->getPinMode(MOTOR_PIN);
        Logger::getInstance().info("Test", ("GPIO7 模式: " + String(mode)).c_str());
        return true;
    } else {
        Logger::getInstance().error("Test", "GPIO7 状态查询失败");
        return false;
    }
}

bool GPIOTest::testInvalidPin() {
    // 测试无效引脚
    bool invalidPinTest = gpioDriver->init(99, OUTPUT, LOW);
    if (!invalidPinTest) {
        Logger::getInstance().info("Test", "无效引脚测试通过 - 正确拒绝了无效引脚");
        return true;
    } else {
        Logger::getInstance().error("Test", "无效引脚测试失败 - 应该拒绝无效引脚");
        return false;
    }
}

void GPIOTest::runLoopTest() {
    unsigned long currentTime = millis();
    
    // 每3秒切换一次电机状态
    if (currentTime - lastToggle >= 3000) {
        lastToggle = currentTime;
        cycleCount++;
        
        // 执行电机控制测试
        testMotorControl();
        
        // 每10个循环后测试引脚切换功能
        if (cycleCount % 10 == 0) {
            testPinToggle();
        }
        
        // 每20个循环后显示系统状态
        if (cycleCount % 20 == 0) {
            showSystemStatus();
        }
    }
}

bool GPIOTest::testMotorControl() {
    // 切换电机状态
    motorState = !motorState;
    uint8_t outputLevel = motorState ? MOTOR_ON : MOTOR_OFF;
    
    // 使用GPIO驱动控制电机
    bool result = gpioDriver->digitalWrite(MOTOR_PIN, outputLevel);
    
    if (result) {
        String stateStr = motorState ? "启动" : "停止";
        String levelStr = (outputLevel == HIGH) ? "HIGH" : "LOW";
        Logger::getInstance().info("Motor", ("第" + String(cycleCount) + "次循环 - 电机" + stateStr + " (GPIO7=" + levelStr + ")").c_str());
    } else {
        Logger::getInstance().error("Motor", "电机控制失败");
        return false;
    }
    
    // 测试引脚状态读取
    int readState = gpioDriver->digitalRead(MOTOR_PIN);
    if (readState >= 0) {
        String readStr = (readState == HIGH) ? "HIGH" : "LOW";
        Logger::getInstance().debug("Motor", ("GPIO7 读取状态: " + readStr).c_str());
    } else {
        Logger::getInstance().error("Motor", "GPIO7 状态读取失败");
        return false;
    }
    
    return true;
}

void GPIOTest::testPinToggle() {
    Logger::getInstance().info("Test", "测试引脚切换功能...");
    delay(500);
    gpioDriver->togglePin(MOTOR_PIN);
    delay(500);
    gpioDriver->togglePin(MOTOR_PIN);
    Logger::getInstance().info("Test", "引脚切换测试完成");
}

void GPIOTest::showSystemStatus() {
    Logger::getInstance().info("System", ("系统运行时间: " + String(millis()/1000) + "秒").c_str());
    Logger::getInstance().info("System", ("已完成测试循环: " + String(cycleCount) + "次").c_str());
}