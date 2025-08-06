#include "MotorBLEServerTest.h"
#include "../controllers/MotorController.h"
#include "../controllers/ConfigManager.h"
#include "../common/Logger.h"
#include <ArduinoJson.h>

void MotorBLEServerTest::runAllTests() {
    UNITY_BEGIN();
    
    RUN_TEST(testSingleton);
    RUN_TEST(testInitialization);
    RUN_TEST(testConfigJsonGeneration);
    RUN_TEST(testInfoJsonGeneration);
    RUN_TEST(testCommandHandling);
    RUN_TEST(testConfigHandling);
    
    UNITY_END();
}

void MotorBLEServerTest::testSingleton() {
    MotorBLEServer& instance1 = MotorBLEServer::getInstance();
    MotorBLEServer& instance2 = MotorBLEServer::getInstance();
    
    TEST_ASSERT_EQUAL_PTR(&instance1, &instance2);
    TEST_ASSERT_TRUE(&instance1 == &instance2);
}

void MotorBLEServerTest::testInitialization() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 注意：由于BLE需要硬件支持，这里只测试初始化不会崩溃
    // 实际BLE功能需要在真实硬件上测试
    TEST_ASSERT_NOT_NULL(bleServer.getLastError());
}

void MotorBLEServerTest::testConfigJsonGeneration() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 测试JSON生成
    String json = bleServer.generateStatusJson();
    TEST_ASSERT_TRUE(json.length() > 0);
    
    // 验证JSON格式
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, json);
    TEST_ASSERT_FALSE(error);
    
    // 验证必要字段
    TEST_ASSERT_TRUE(doc.containsKey("state"));
    TEST_ASSERT_TRUE(doc.containsKey("stateName"));
    TEST_ASSERT_TRUE(doc.containsKey("remainingRunTime"));
    TEST_ASSERT_TRUE(doc.containsKey("remainingStopTime"));
    TEST_ASSERT_TRUE(doc.containsKey("currentCycleCount"));
    TEST_ASSERT_TRUE(doc.containsKey("runDuration"));
    TEST_ASSERT_TRUE(doc.containsKey("stopDuration"));
    TEST_ASSERT_TRUE(doc.containsKey("autoStart"));
}

void MotorBLEServerTest::testInfoJsonGeneration() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 测试信息JSON生成
    String json = bleServer.generateInfoJson();
    TEST_ASSERT_TRUE(json.length() > 0);
    
    // 验证JSON格式
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    TEST_ASSERT_FALSE(error);
    
    // 验证必要字段
    TEST_ASSERT_TRUE(doc.containsKey("deviceName"));
    TEST_ASSERT_TRUE(doc.containsKey("serviceUUID"));
    TEST_ASSERT_TRUE(doc.containsKey("firmwareVersion"));
    TEST_ASSERT_TRUE(doc.containsKey("hardware"));
    TEST_ASSERT_TRUE(doc.containsKey("features"));
}

void MotorBLEServerTest::testCommandHandling() {
    // 测试命令处理逻辑
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    MotorController& motorController = MotorController::getInstance();
    
    // 初始化配置管理器
    ConfigManager& configManager = ConfigManager::getInstance();
    configManager.init();
    
    // 初始化电机控制器
    motorController.init();
    
    // 测试启动命令 (1 = 启动)
    bleServer.handleSystemControlWrite("1");
    TEST_ASSERT_TRUE(motorController.isRunning() || motorController.getCurrentState() == MotorControllerState::STARTING);
    
    // 测试停止命令 (0 = 停止)
    bleServer.handleSystemControlWrite("0");
    TEST_ASSERT_TRUE(motorController.isStopped() || motorController.getCurrentState() == MotorControllerState::STOPPING);
    
    // 注意：系统控制特征值只支持启动/停止，不支持重置命令
    // 重置功能需要通过其他方式实现，这里跳过重置测试
    LOG_INFO("系统控制特征值不支持重置命令，跳过重置测试");
}

void MotorBLEServerTest::testConfigHandling() {
    // 测试配置处理逻辑
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    ConfigManager& configManager = ConfigManager::getInstance();
    
    configManager.init();
    MotorConfig originalConfig = configManager.getConfig();
    
    // 测试配置更新 - 分别设置运行时长和停止间隔
    // 运行时长：150秒
    bleServer.handleRunDurationWrite("150");
    // 停止间隔：8秒
    bleServer.handleStopIntervalWrite("8");
    
    MotorConfig newConfig = configManager.getConfig();
    TEST_ASSERT_EQUAL_UINT32(150, newConfig.runDuration);
    TEST_ASSERT_EQUAL_UINT32(8, newConfig.stopDuration);
    // 注意：autoStart 不能通过BLE特征值直接设置，需要其他方式
    
    // 恢复原始配置
    configManager.updateConfig(originalConfig);
    configManager.saveConfig();
}