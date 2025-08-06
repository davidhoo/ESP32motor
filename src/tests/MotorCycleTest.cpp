#include "MotorCycleTest.h"
#include "../controllers/MotorController.h"
#include "../controllers/ConfigManager.h"
#include "../common/Logger.h"
#include <Arduino.h>

// 测试用例1: 基本循环控制 (运行3秒，停止2秒，循环3次)
bool MotorCycleTest::testBasicCycle() {
    LOG_TAG_INFO("MotorCycleTest", "=== 测试基本循环控制 ===");
    
    // 设置测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 3000;    // 3秒运行
    testConfig.stopDuration = 2000;   // 2秒停止
    testConfig.cycleCount = 3;        // 循环3次
    testConfig.autoStart = false;     // 手动启动
    
    // 应用配置
    ConfigManager::getInstance().updateConfig(testConfig);
    MotorController& motor = MotorController::getInstance();
    
    // 初始化电机控制器
    if (!motor.init()) {
        LOG_TAG_ERROR("MotorCycleTest", "电机控制器初始化失败: %s", motor.getLastError());
        return false;
    }
    
    motor.updateConfig(testConfig);
    motor.resetCycleCount();
    
    LOG_TAG_INFO("MotorCycleTest", "配置: 运行%ums, 停止%ums, 循环%u次", 
                 testConfig.runDuration, testConfig.stopDuration, testConfig.cycleCount);
    
    // 启动电机
    if (!motor.startMotor()) {
        LOG_TAG_ERROR("MotorCycleTest", "启动电机失败");
        return false;
    }
    
    // 模拟运行过程
    uint32_t startTime = millis();
    uint32_t lastLogTime = 0;
    bool testPassed = true;
    
    while (millis() - startTime < 20000) { // 最多测试20秒
        motor.update();
        
        // 每秒输出一次状态
        if (millis() - lastLogTime >= 1000) {
            MotorControllerState state = motor.getCurrentState();
            uint32_t cycleCount = motor.getCurrentCycleCount();
            uint32_t remainingRun = motor.getRemainingRunTime();
            uint32_t remainingStop = motor.getRemainingStopTime();
            
            LOG_TAG_INFO("MotorCycleTest", "状态: %d, 循环: %u/3, 剩余运行: %us, 剩余停止: %us",
                         static_cast<int>(state), cycleCount, remainingRun, remainingStop);
            
            lastLogTime = millis();
            
            // 检查是否完成所有循环
            if (cycleCount >= 3 && state == MotorControllerState::STOPPED) {
                LOG_TAG_INFO("MotorCycleTest", "所有循环已完成，测试通过");
                break;
            }
        }
        
        delay(100);
    }
    
    // 验证最终状态
    if (motor.getCurrentCycleCount() != 3) {
        LOG_TAG_ERROR("MotorCycleTest", "循环次数不正确: 期望3, 实际%u", motor.getCurrentCycleCount());
        testPassed = false;
    }
    
    if (motor.getCurrentState() != MotorControllerState::STOPPED) {
        LOG_TAG_ERROR("MotorCycleTest", "最终状态不正确: 期望STOPPED, 实际%d", 
                      static_cast<int>(motor.getCurrentState()));
        testPassed = false;
    }
    
    return testPassed;
}

// 测试用例2: 持续运行模式 (停止间隔为0)
bool MotorCycleTest::testContinuousMode() {
    LOG_TAG_INFO("MotorCycleTest", "=== 测试持续运行模式 ===");
    
    // 设置测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 2000;    // 2秒运行
    testConfig.stopDuration = 0;      // 0秒停止 (持续运行)
    testConfig.cycleCount = 5;        // 循环5次
    testConfig.autoStart = false;     // 手动启动
    
    // 应用配置
    ConfigManager::getInstance().updateConfig(testConfig);
    MotorController& motor = MotorController::getInstance();
    
    // 初始化电机控制器
    if (!motor.init()) {
        LOG_TAG_ERROR("MotorCycleTest", "电机控制器初始化失败: %s", motor.getLastError());
        return false;
    }
    
    motor.updateConfig(testConfig);
    motor.resetCycleCount();
    
    LOG_TAG_INFO("MotorCycleTest", "配置: 运行%ums, 停止%ums (持续模式), 循环%u次", 
                 testConfig.runDuration, testConfig.stopDuration, testConfig.cycleCount);
    
    // 启动电机
    if (!motor.startMotor()) {
        LOG_TAG_ERROR("MotorCycleTest", "启动电机失败");
        return false;
    }
    
    // 模拟运行过程
    uint32_t startTime = millis();
    uint32_t lastLogTime = 0;
    bool testPassed = true;
    
    while (millis() - startTime < 15000) { // 最多测试15秒
        motor.update();
        
        // 每秒输出一次状态
        if (millis() - lastLogTime >= 1000) {
            MotorControllerState state = motor.getCurrentState();
            uint32_t cycleCount = motor.getCurrentCycleCount();
            uint32_t remainingRun = motor.getRemainingRunTime();
            
            LOG_TAG_INFO("MotorCycleTest", "状态: %d, 循环: %u/5, 剩余运行: %us",
                         static_cast<int>(state), cycleCount, remainingRun);
            
            lastLogTime = millis();
            
            // 检查是否完成所有循环
            if (cycleCount >= 5 && state == MotorControllerState::STOPPED) {
                LOG_TAG_INFO("MotorCycleTest", "所有循环已完成，测试通过");
                break;
            }
        }
        
        delay(100);
    }
    
    // 验证最终状态
    if (motor.getCurrentCycleCount() != 5) {
        LOG_TAG_ERROR("MotorCycleTest", "循环次数不正确: 期望5, 实际%u", motor.getCurrentCycleCount());
        testPassed = false;
    }
    
    return testPassed;
}

// 测试用例3: 无限循环模式
bool MotorCycleTest::testInfiniteMode() {
    LOG_TAG_INFO("MotorCycleTest", "=== 测试无限循环模式 ===");
    
    // 设置测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 1000;    // 1秒运行
    testConfig.stopDuration = 500;    // 0.5秒停止
    testConfig.cycleCount = 0;        // 0表示无限循环
    testConfig.autoStart = false;     // 手动启动
    
    // 应用配置
    ConfigManager::getInstance().updateConfig(testConfig);
    MotorController& motor = MotorController::getInstance();
    
    // 初始化电机控制器
    if (!motor.init()) {
        LOG_TAG_ERROR("MotorCycleTest", "电机控制器初始化失败: %s", motor.getLastError());
        return false;
    }
    
    motor.updateConfig(testConfig);
    motor.resetCycleCount();
    
    LOG_TAG_INFO("MotorCycleTest", "配置: 运行%ums, 停止%ums, 无限循环", 
                 testConfig.runDuration, testConfig.stopDuration);
    
    // 启动电机
    if (!motor.startMotor()) {
        LOG_TAG_ERROR("MotorCycleTest", "启动电机失败");
        return false;
    }
    
    // 模拟运行过程 (测试10秒，应该能完成多个循环)
    uint32_t startTime = millis();
    uint32_t lastLogTime = 0;
    uint32_t expectedMinCycles = 5; // 10秒内至少应该完成5个循环
    
    while (millis() - startTime < 10000) { // 测试10秒
        motor.update();
        
        // 每秒输出一次状态
        if (millis() - lastLogTime >= 1000) {
            MotorControllerState state = motor.getCurrentState();
            uint32_t cycleCount = motor.getCurrentCycleCount();
            uint32_t remainingRun = motor.getRemainingRunTime();
            uint32_t remainingStop = motor.getRemainingStopTime();
            
            LOG_TAG_INFO("MotorCycleTest", "状态: %d, 循环: %u, 剩余运行: %us, 剩余停止: %us",
                         static_cast<int>(state), cycleCount, remainingRun, remainingStop);
            
            lastLogTime = millis();
        }
        
        delay(100);
    }
    
    // 手动停止电机
    motor.stopMotor();
    delay(100);
    motor.update();
    
    // 验证循环次数
    uint32_t finalCycles = motor.getCurrentCycleCount();
    if (finalCycles < expectedMinCycles) {
        LOG_TAG_ERROR("MotorCycleTest", "循环次数不足: 期望至少%u, 实际%u", 
                      expectedMinCycles, finalCycles);
        return false;
    }
    
    LOG_TAG_INFO("MotorCycleTest", "无限循环测试完成，共完成%u个循环", finalCycles);
    return true;
}

// 测试用例4: 配置动态更新
bool MotorCycleTest::testConfigUpdate() {
    LOG_TAG_INFO("MotorCycleTest", "=== 测试配置动态更新 ===");
    
    // 初始配置
    MotorConfig testConfig;
    testConfig.runDuration = 2000;    // 2秒运行
    testConfig.stopDuration = 1000;   // 1秒停止
    testConfig.cycleCount = 0;        // 无限循环
    testConfig.autoStart = false;     // 手动启动
    
    // 应用配置
    ConfigManager::getInstance().updateConfig(testConfig);
    MotorController& motor = MotorController::getInstance();
    
    // 初始化电机控制器
    if (!motor.init()) {
        LOG_TAG_ERROR("MotorCycleTest", "电机控制器初始化失败: %s", motor.getLastError());
        return false;
    }
    
    motor.updateConfig(testConfig);
    motor.resetCycleCount();
    
    // 启动电机
    if (!motor.startMotor()) {
        LOG_TAG_ERROR("MotorCycleTest", "启动电机失败");
        return false;
    }
    
    // 运行一段时间
    uint32_t startTime = millis();
    while (millis() - startTime < 3000) {
        motor.update();
        delay(100);
    }
    
    uint32_t cyclesBeforeUpdate = motor.getCurrentCycleCount();
    LOG_TAG_INFO("MotorCycleTest", "更新前循环次数: %u", cyclesBeforeUpdate);
    
    // 动态更新配置
    testConfig.runDuration = 1000;    // 改为1秒运行
    testConfig.stopDuration = 500;    // 改为0.5秒停止
    testConfig.cycleCount = cyclesBeforeUpdate + 3; // 设置循环次数限制
    
    motor.updateConfig(testConfig);
    LOG_TAG_INFO("MotorCycleTest", "配置已更新: 运行%ums, 停止%ums, 循环%u次", 
                 testConfig.runDuration, testConfig.stopDuration, testConfig.cycleCount);
    
    // 继续运行直到完成
    while (millis() - startTime < 10000) {
        motor.update();
        
        MotorControllerState state = motor.getCurrentState();
        uint32_t cycleCount = motor.getCurrentCycleCount();
        
        if (cycleCount >= testConfig.cycleCount && state == MotorControllerState::STOPPED) {
            LOG_TAG_INFO("MotorCycleTest", "配置更新后循环完成，最终循环次数: %u", cycleCount);
            return true;
        }
        
        delay(100);
    }
    
    LOG_TAG_ERROR("MotorCycleTest", "配置更新测试超时");
    return false;
}

// 运行所有测试
bool MotorCycleTest::runAllTests() {
    LOG_TAG_INFO("MotorCycleTest", "开始电机循环控制测试...");
    
    bool allPassed = true;
    
    // 测试1: 基本循环控制
    if (!testBasicCycle()) {
        LOG_TAG_ERROR("MotorCycleTest", "基本循环控制测试失败");
        allPassed = false;
    }
    delay(1000);
    
    // 测试2: 持续运行模式
    if (!testContinuousMode()) {
        LOG_TAG_ERROR("MotorCycleTest", "持续运行模式测试失败");
        allPassed = false;
    }
    delay(1000);
    
    // 测试3: 无限循环模式
    if (!testInfiniteMode()) {
        LOG_TAG_ERROR("MotorCycleTest", "无限循环模式测试失败");
        allPassed = false;
    }
    delay(1000);
    
    // 测试4: 配置动态更新
    if (!testConfigUpdate()) {
        LOG_TAG_ERROR("MotorCycleTest", "配置动态更新测试失败");
        allPassed = false;
    }
    
    if (allPassed) {
        LOG_TAG_INFO("MotorCycleTest", "=== 所有测试通过! ===");
    } else {
        LOG_TAG_ERROR("MotorCycleTest", "=== 部分测试失败! ===");
    }
    
    return allPassed;
}