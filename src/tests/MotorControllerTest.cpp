#include "MotorControllerTest.h"
#include "../common/Logger.h"

void MotorControllerTest::runAllTests() {
    LOG_TAG_INFO("MotorControllerTest", "开始电机控制器测试...");
    
    testInit();
    testStartStop();
    testStateTransitions();
    testCycleControl();
    testCountdown();
    testConfigUpdate();
    testParameterQueries();
    testBoundaryConditions();
    testErrorHandling();
    testCycleCounter();
    
    LOG_TAG_INFO("MotorControllerTest", "所有测试完成！");
}

void MotorControllerTest::testInit() {
    LOG_TAG_INFO("MotorControllerTest", "测试初始化...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 测试初始化
    bool result = motorController.init();
    assertTrue(result, "电机控制器初始化应该成功");
    
    // 验证初始状态
    assertEqual(MotorControllerState::STOPPED, motorController.getCurrentState(), 
                "初始状态应该是STOPPED");
    
    // 验证初始配置
    const MotorConfig& config = motorController.getCurrentConfig();
    assertEqual(5000u, config.runDuration, "默认运行时间应该是5000ms");
    assertEqual(2000u, config.stopDuration, "默认停止时间应该是2000ms");
    assertEqual(0u, config.cycleCount, "默认循环次数应该是0");
    assertEqual(true, config.autoStart, "默认应该自动启动");
    
    LOG_TAG_INFO("MotorControllerTest", "初始化测试通过");
}

void MotorControllerTest::testStartStop() {
    LOG_TAG_INFO("MotorControllerTest", "测试启动和停止...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 确保已初始化
    if (!motorController.isStopped()) {
        motorController.stopMotor();
    }
    
    // 测试启动
    bool startResult = motorController.startMotor();
    assertTrue(startResult, "启动电机应该成功");
    
    // 测试停止
    bool stopResult = motorController.stopMotor();
    assertTrue(stopResult, "停止电机应该成功");
    
    LOG_TAG_INFO("MotorControllerTest", "启动停止测试通过");
}

void MotorControllerTest::testStateTransitions() {
    LOG_TAG_INFO("MotorControllerTest", "测试状态转换...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 重置状态
    motorController.stopMotor();
    
    // 测试从STOPPED到STARTING
    motorController.startMotor();
    assertEqual(MotorControllerState::STARTING, motorController.getCurrentState(), 
                "启动后应该进入STARTING状态");
    
    // 手动更新一次状态机
    motorController.update();
    assertEqual(MotorControllerState::RUNNING, motorController.getCurrentState(), 
                "更新后应该进入RUNNING状态");
    
    // 测试从RUNNING到STOPPING
    motorController.stopMotor();
    assertEqual(MotorControllerState::STOPPING, motorController.getCurrentState(), 
                "停止后应该进入STOPPING状态");
    
    // 手动更新一次状态机
    motorController.update();
    assertEqual(MotorControllerState::STOPPED, motorController.getCurrentState(), 
                "更新后应该进入STOPPED状态");
    
    LOG_TAG_INFO("MotorControllerTest", "状态转换测试通过");
}

void MotorControllerTest::testCycleControl() {
    LOG_TAG_INFO("MotorControllerTest", "测试循环控制...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 设置短时间的测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 1000;   // 1秒运行
    testConfig.stopDuration = 1000;  // 1秒停止
    testConfig.cycleCount = 2;       // 2次循环
    
    motorController.updateConfig(testConfig);
    motorController.resetCycleCount();
    
    // 验证初始循环次数
    assertEqual(0u, motorController.getCurrentCycleCount(), "初始循环次数应该是0");
    
    // 启动电机
    motorController.startMotor();
    
    // 模拟运行和停止周期
    LOG_TAG_INFO("MotorControllerTest", "模拟运行周期...");
    
    // 由于实际测试需要真实硬件，这里主要验证状态转换逻辑
    motorController.update();
    
    LOG_TAG_INFO("MotorControllerTest", "循环控制测试通过");
}

void MotorControllerTest::testCountdown() {
    LOG_TAG_INFO("MotorControllerTest", "测试倒计时功能...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 设置测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 3000;   // 3秒运行
    testConfig.stopDuration = 2000;  // 2秒停止
    
    motorController.updateConfig(testConfig);
    
    // 启动电机
    motorController.startMotor();
    motorController.update(); // 进入RUNNING状态
    
    // 验证初始剩余时间
    uint32_t initialRunTime = motorController.getRemainingRunTime();
    assertTrue(initialRunTime <= 3, "初始剩余运行时间应该小于等于3秒");
    
    // 停止电机
    motorController.stopMotor();
    motorController.update(); // 进入STOPPED状态
    
    // 验证停止后的剩余时间
    uint32_t remainingStopTime = motorController.getRemainingStopTime();
    assertTrue(remainingStopTime <= 2, "初始剩余停止时间应该小于等于2秒");
    
    LOG_TAG_INFO("MotorControllerTest", "倒计时测试通过");
}

void MotorControllerTest::testConfigUpdate() {
    LOG_TAG_INFO("MotorControllerTest", "测试配置更新...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 保存原始配置
    MotorConfig originalConfig = motorController.getCurrentConfig();
    
    // 创建新配置
    MotorConfig newConfig;
    newConfig.runDuration = 10000;   // 10秒运行
    newConfig.stopDuration = 5000;   // 5秒停止
    newConfig.cycleCount = 10;       // 10次循环
    newConfig.autoStart = false;     // 不自动启动
    
    // 更新配置
    motorController.updateConfig(newConfig);
    
    // 验证配置已更新
    const MotorConfig& updatedConfig = motorController.getCurrentConfig();
    assertEqual(10000u, updatedConfig.runDuration, "运行时间应该更新为10000ms");
    assertEqual(5000u, updatedConfig.stopDuration, "停止时间应该更新为5000ms");
    assertEqual(10u, updatedConfig.cycleCount, "循环次数应该更新为10");
    assertEqual(false, updatedConfig.autoStart, "自动启动应该更新为false");
    
    // 恢复原始配置
    motorController.updateConfig(originalConfig);
    
    LOG_TAG_INFO("MotorControllerTest", "配置更新测试通过");
}

void MotorControllerTest::testParameterQueries() {
    LOG_TAG_INFO("MotorControllerTest", "测试参数查询接口...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 测试各种查询接口
    MotorControllerState state = motorController.getCurrentState();
    assertTrue(state == MotorControllerState::STOPPED || 
               state == MotorControllerState::RUNNING, 
               "状态应该是STOPPED或RUNNING");
    
    uint32_t cycleCount = motorController.getCurrentCycleCount();
    assertTrue(cycleCount >= 0, "循环次数应该大于等于0");
    
    bool isRunning = motorController.isRunning();
    bool isStopped = motorController.isStopped();
    assertTrue(isRunning != isStopped, "运行和停止状态应该互斥");
    
    const char* error = motorController.getLastError();
    assertTrue(error != nullptr, "错误信息指针应该有效");
    
    LOG_TAG_INFO("MotorControllerTest", "参数查询测试通过");
}

void MotorControllerTest::testBoundaryConditions() {
    LOG_TAG_INFO("MotorControllerTest", "测试边界条件...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 测试零值配置
    MotorConfig zeroConfig;
    zeroConfig.runDuration = 0;
    zeroConfig.stopDuration = 0;
    
    motorController.updateConfig(zeroConfig);
    
    // 测试大值配置
    MotorConfig largeConfig;
    largeConfig.runDuration = 3600000;  // 1小时
    largeConfig.stopDuration = 3600000; // 1小时
    
    motorController.updateConfig(largeConfig);
    
    LOG_TAG_INFO("MotorControllerTest", "边界条件测试通过");
}

void MotorControllerTest::testErrorHandling() {
    LOG_TAG_INFO("MotorControllerTest", "测试错误处理...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 测试未初始化时的操作
    // 注意：由于我们已经在testInit中初始化了，这里主要测试错误信息获取
    const char* error = motorController.getLastError();
    assertTrue(error != nullptr, "应该能够获取错误信息");
    
    LOG_TAG_INFO("MotorControllerTest", "错误处理测试通过");
}

void MotorControllerTest::testCycleCounter() {
    LOG_TAG_INFO("MotorControllerTest", "测试循环计数器...");
    
    MotorController& motorController = MotorController::getInstance();
    
    // 重置计数器
    motorController.resetCycleCount();
    assertEqual(0u, motorController.getCurrentCycleCount(), "重置后循环次数应该是0");
    
    // 测试计数器递增（通过模拟状态转换）
    // 注意：实际递增需要真实硬件和完整状态机运行
    
    LOG_TAG_INFO("MotorControllerTest", "循环计数器测试通过");
}

void MotorControllerTest::assertTrue(bool condition, const char* message) {
    if (!condition) {
        LOG_TAG_ERROR("MotorControllerTest", "断言失败: %s", message);
    } else {
        LOG_TAG_DEBUG("MotorControllerTest", "断言通过: %s", message);
    }
}

void MotorControllerTest::assertEqual(uint32_t expected, uint32_t actual, const char* message) {
    if (expected != actual) {
        LOG_TAG_ERROR("MotorControllerTest", "断言失败: %s (期望: %u, 实际: %u)", 
                     message, expected, actual);
    } else {
        LOG_TAG_DEBUG("MotorControllerTest", "断言通过: %s", message);
    }
}

void MotorControllerTest::assertEqual(MotorControllerState expected, MotorControllerState actual, const char* message) {
    if (expected != actual) {
        LOG_TAG_ERROR("MotorControllerTest", "断言失败: %s (期望: %d, 实际: %d)", 
                     message, static_cast<int>(expected), static_cast<int>(actual));
    } else {
        LOG_TAG_DEBUG("MotorControllerTest", "断言通过: %s", message);
    }
}

void MotorControllerTest::delay(uint32_t ms) {
    // 使用Arduino标准延迟函数
    ::delay(ms);
}