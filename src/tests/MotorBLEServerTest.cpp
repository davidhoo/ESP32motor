#include "MotorBLEServerTest.h"
#include "../controllers/MotorController.h"
#include "../controllers/ConfigManager.h"
#include "../common/Logger.h"
#include <ArduinoJson.h>

// 自定义测试宏，避免与Unity框架冲突
#define MB_TEST_ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        Serial.printf("TEST FAILED: Expected %d, got %d at %s:%d\n", \
                     (int)(expected), (int)(actual), __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define MB_TEST_ASSERT_TRUE(condition) \
    if (!(condition)) { \
        Serial.printf("TEST FAILED: Expected true, got false at %s:%d\n", \
                     __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define MB_TEST_ASSERT_FALSE(condition) \
    if (condition) { \
        Serial.printf("TEST FAILED: Expected false, got true at %s:%d\n", \
                     __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define MB_TEST_ASSERT_NOT_NULL(pointer) \
    if ((pointer) == NULL) { \
        Serial.printf("TEST FAILED: Expected non-null pointer at %s:%d\n", \
                     __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define MB_TEST_ASSERT_EQUAL_UINT32(expected, actual) \
    if ((expected) != (actual)) { \
        Serial.printf("TEST FAILED: Expected %lu, got %lu at %s:%d\n", \
                     (unsigned long)(expected), (unsigned long)(actual), __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

void MotorBLEServerTest::runAllTests() {
    Serial.println("=== 开始 MotorBLEServer 测试 ===");
    
    testSingleton();
    testInitialization();
    testConfigJsonGeneration();
    testInfoJsonGeneration();
    testCommandHandling();
    testConfigHandling();
    testSpeedControllerStatusJsonGeneration();
    
    Serial.println("=== MotorBLEServer 测试完成 ===");
}

void MotorBLEServerTest::testSingleton() {
    MotorBLEServer& instance1 = MotorBLEServer::getInstance();
    MotorBLEServer& instance2 = MotorBLEServer::getInstance();
    
    MB_TEST_ASSERT_EQUAL((uintptr_t)&instance1, (uintptr_t)&instance2);
    MB_TEST_ASSERT_TRUE(&instance1 == &instance2);
}

void MotorBLEServerTest::testInitialization() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 注意：由于BLE需要硬件支持，这里只测试初始化不会崩溃
    // 实际BLE功能需要在真实硬件上测试
    MB_TEST_ASSERT_NOT_NULL(bleServer.getLastError());
}

void MotorBLEServerTest::testConfigJsonGeneration() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 测试JSON生成
    String json = bleServer.generateStatusJson();
    MB_TEST_ASSERT_TRUE(json.length() > 0);
    
    // 验证JSON格式
    DynamicJsonDocument doc(512);
    DeserializationError error = deserializeJson(doc, json);
    MB_TEST_ASSERT_FALSE(error);
    
    // 验证必要字段
    MB_TEST_ASSERT_TRUE(doc.containsKey("state"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("stateName"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("remainingRunTime"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("remainingStopTime"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("currentCycleCount"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("runDuration"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("stopDuration"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("autoStart"));
}

void MotorBLEServerTest::testInfoJsonGeneration() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 测试信息JSON生成
    String json = bleServer.generateInfoJson();
    MB_TEST_ASSERT_TRUE(json.length() > 0);
    
    // 验证JSON格式
    DynamicJsonDocument doc(256);
    DeserializationError error = deserializeJson(doc, json);
    MB_TEST_ASSERT_FALSE(error);
    
    // 验证必要字段
    MB_TEST_ASSERT_TRUE(doc.containsKey("deviceName"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("serviceUUID"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("firmwareVersion"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("hardware"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("features"));
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
    MB_TEST_ASSERT_TRUE(motorController.isRunning() || motorController.getCurrentState() == MotorControllerState::STARTING);
    
    // 测试停止命令 (0 = 停止)
    bleServer.handleSystemControlWrite("0");
    MB_TEST_ASSERT_TRUE(motorController.isStopped() || motorController.getCurrentState() == MotorControllerState::STOPPING);
    
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
    MB_TEST_ASSERT_EQUAL_UINT32(150, newConfig.runDuration);
    MB_TEST_ASSERT_EQUAL_UINT32(8, newConfig.stopDuration);
    // 注意：autoStart 不能通过BLE特征值直接设置，需要其他方式
    
    // 恢复原始配置
    configManager.updateConfig(originalConfig);
    configManager.saveConfig();
}

void MotorBLEServerTest::testSpeedControllerStatusJsonGeneration() {
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    // 测试调速器状态JSON生成
    String json = bleServer.generateSpeedControllerConfigJson();
    MB_TEST_ASSERT_TRUE(json.length() > 0);
    
    // 验证JSON格式
    DynamicJsonDocument doc(1024);
    DeserializationError error = deserializeJson(doc, json);
    MB_TEST_ASSERT_FALSE(error);
    
    // 验证必要字段
    MB_TEST_ASSERT_TRUE(doc.containsKey("moduleAddress"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("isRunning"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("frequency"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("dutyCycle"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("externalSwitch"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("analogControl"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("powerOnState"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("minOutput"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("maxOutput"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("softStartTime"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("softStopTime"));
    MB_TEST_ASSERT_TRUE(doc.containsKey("communication"));
    
    // 验证通信状态字段
    JsonObject communication = doc["communication"];
    MB_TEST_ASSERT_TRUE(communication.containsKey("lastUpdateTime"));
    MB_TEST_ASSERT_TRUE(communication.containsKey("connectionStatus"));
    MB_TEST_ASSERT_TRUE(communication.containsKey("errorCount"));
    MB_TEST_ASSERT_TRUE(communication.containsKey("responseTime"));
    
    // 验证数据类型
    MB_TEST_ASSERT_TRUE(doc["moduleAddress"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["isRunning"].is<bool>());
    MB_TEST_ASSERT_TRUE(doc["frequency"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["dutyCycle"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["externalSwitch"].is<bool>());
    MB_TEST_ASSERT_TRUE(doc["analogControl"].is<bool>());
    MB_TEST_ASSERT_TRUE(doc["powerOnState"].is<bool>());
    MB_TEST_ASSERT_TRUE(doc["minOutput"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["maxOutput"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["softStartTime"].is<int>());
    MB_TEST_ASSERT_TRUE(doc["softStopTime"].is<int>());
    
    LOG_INFO("调速器状态JSON测试通过: %s", json.c_str());
}