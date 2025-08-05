#include "ConfigManagerTest.h"
#include <Arduino.h>

/**
 * 运行所有测试
 */
void ConfigManagerTest::runAllTests() {
    Serial.println("=== ConfigManager 单元测试开始 ===");
    
    testSingleton();
    testInit();
    testConfigValidation();
    testLoadSaveConfig();
    testDefaultValues();
    testConfigModification();
    testErrorHandling();
    testBoundaryValues();
    
    Serial.println("=== ConfigManager 单元测试完成 ===");
}

/**
 * 测试单例模式
 */
void ConfigManagerTest::testSingleton() {
    Serial.println("测试单例模式...");
    
    ConfigManager& instance1 = ConfigManager::getInstance();
    ConfigManager& instance2 = ConfigManager::getInstance();
    
    assertTrue(&instance1 == &instance2, "单例模式应该返回相同实例");
    
    Serial.println("✓ 单例模式测试通过");
}

/**
 * 测试初始化
 */
void ConfigManagerTest::testInit() {
    Serial.println("测试初始化...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 测试初始化
    bool initResult = manager.init();
    assertTrue(initResult, "初始化应该成功");
    
    Serial.println("✓ 初始化测试通过");
}

/**
 * 测试配置验证
 */
void ConfigManagerTest::testConfigValidation() {
    Serial.println("测试配置验证...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 测试有效配置
    MotorConfig validConfig;
    validConfig.runDuration = 5000;
    validConfig.stopDuration = 2000;
    validConfig.cycleCount = 10;
    validConfig.autoStart = true;
    
    assertTrue(manager.validateConfig(validConfig), "有效配置应该通过验证");
    
    // 测试无效运行时长
    MotorConfig invalidRunDuration = validConfig;
    invalidRunDuration.runDuration = 50;  // 小于100ms
    assertFalse(manager.validateConfig(invalidRunDuration), "运行时长过小应该失败");
    
    invalidRunDuration.runDuration = 4000000;  // 大于1小时
    assertFalse(manager.validateConfig(invalidRunDuration), "运行时长过大应该失败");
    
    // 测试无效停止时长
    MotorConfig invalidStopDuration = validConfig;
    invalidStopDuration.stopDuration = 4000000;  // 大于1小时
    assertFalse(manager.validateConfig(invalidStopDuration), "停止时长过大应该失败");
    
    // 测试无效循环次数
    MotorConfig invalidCycleCount = validConfig;
    invalidCycleCount.cycleCount = 2000000;  // 大于100万
    assertFalse(manager.validateConfig(invalidCycleCount), "循环次数过大应该失败");
    
    Serial.println("✓ 配置验证测试通过");
}

/**
 * 测试配置加载和保存
 */
void ConfigManagerTest::testLoadSaveConfig() {
    Serial.println("测试配置加载和保存...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 确保已初始化
    manager.init();
    
    // 创建测试配置
    MotorConfig testConfig;
    testConfig.runDuration = 3000;
    testConfig.stopDuration = 1500;
    testConfig.cycleCount = 5;
    testConfig.autoStart = false;
    
    // 更新配置
    manager.updateConfig(testConfig);
    
    // 保存配置
    bool saveResult = manager.saveConfig();
    assertTrue(saveResult, "保存配置应该成功");
    
    // 重置为默认值
    manager.resetToDefaults();
    
    // 验证当前配置已重置
    const MotorConfig& defaultConfig = manager.getConfig();
    assertEqual(5000, defaultConfig.runDuration, "重置后应该使用默认运行时长");
    assertEqual(2000, defaultConfig.stopDuration, "重置后应该使用默认停止时长");
    assertEqual(0, defaultConfig.cycleCount, "重置后应该使用默认循环次数");
    assertEqual(true, defaultConfig.autoStart, "重置后应该使用默认自动启动设置");
    
    // 加载保存的配置
    bool loadResult = manager.loadConfig();
    assertTrue(loadResult, "加载配置应该成功");
    
    // 验证加载的配置
    const MotorConfig& loadedConfig = manager.getConfig();
    assertEqual(3000, loadedConfig.runDuration, "加载的运行时长应该匹配");
    assertEqual(1500, loadedConfig.stopDuration, "加载的停止时长应该匹配");
    assertEqual(5, loadedConfig.cycleCount, "加载的循环次数应该匹配");
    assertEqual(false, loadedConfig.autoStart, "加载的自动启动设置应该匹配");
    
    Serial.println("✓ 配置加载和保存测试通过");
}

/**
 * 测试默认值处理
 */
void ConfigManagerTest::testDefaultValues() {
    Serial.println("测试默认值处理...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 获取当前配置
    const MotorConfig& config = manager.getConfig();
    
    // 验证默认值
    assertEqual(5000, config.runDuration, "默认运行时长应该是5000ms");
    assertEqual(2000, config.stopDuration, "默认停止时长应该是2000ms");
    assertEqual(0, config.cycleCount, "默认循环次数应该是0（无限）");
    assertEqual(true, config.autoStart, "默认应该自动启动");
    
    Serial.println("✓ 默认值处理测试通过");
}

/**
 * 测试配置修改检测
 */
void ConfigManagerTest::testConfigModification() {
    Serial.println("测试配置修改检测...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 确保已初始化
    manager.init();
    
    // 初始状态应该未修改
    assertFalse(manager.isConfigModified(), "初始状态应该未修改");
    
    // 创建新配置
    MotorConfig newConfig;
    newConfig.runDuration = 8000;
    
    // 更新配置
    manager.updateConfig(newConfig);
    assertTrue(manager.isConfigModified(), "更新配置后应该标记为已修改");
    
    // 保存配置
    manager.saveConfig();
    assertFalse(manager.isConfigModified(), "保存配置后应该标记为未修改");
    
    Serial.println("✓ 配置修改检测测试通过");
}

/**
 * 测试错误处理
 */
void ConfigManagerTest::testErrorHandling() {
    Serial.println("测试错误处理...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 测试无效配置的错误信息
    MotorConfig invalidConfig;
    invalidConfig.runDuration = 50;  // 无效值
    
    assertFalse(manager.validateConfig(invalidConfig), "无效配置应该失败");
    assertEqualString("运行时长必须在100ms到3600000ms之间", manager.getValidationError(), "应该返回正确的验证错误");
    
    Serial.println("✓ 错误处理测试通过");
}

/**
 * 测试边界值
 */
void ConfigManagerTest::testBoundaryValues() {
    Serial.println("测试边界值...");
    
    ConfigManager& manager = ConfigManager::getInstance();
    
    // 测试边界值
    MotorConfig boundaryConfig;
    
    // 最小有效值
    boundaryConfig.runDuration = 100;
    boundaryConfig.stopDuration = 0;
    boundaryConfig.cycleCount = 0;
    assertTrue(manager.validateConfig(boundaryConfig), "最小边界值应该有效");
    
    // 最大有效值
    boundaryConfig.runDuration = 3600000;
    boundaryConfig.stopDuration = 3600000;
    boundaryConfig.cycleCount = 1000000;
    assertTrue(manager.validateConfig(boundaryConfig), "最大边界值应该有效");
    
    Serial.println("✓ 边界值测试通过");
}

/**
 * 断言辅助函数
 */
void ConfigManagerTest::assertTrue(bool condition, const char* message) {
    if (!condition) {
        Serial.print("❌ 断言失败: ");
        Serial.println(message);
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void ConfigManagerTest::assertFalse(bool condition, const char* message) {
    assertTrue(!condition, message);
}

void ConfigManagerTest::assertEqual(uint32_t expected, uint32_t actual, const char* message) {
    if (expected != actual) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: ");
        Serial.print(expected);
        Serial.print(", 实际: ");
        Serial.print(actual);
        Serial.println(")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void ConfigManagerTest::assertEqual(bool expected, bool actual, const char* message) {
    if (expected != actual) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: ");
        Serial.print(expected ? "true" : "false");
        Serial.print(", 实际: ");
        Serial.print(actual ? "true" : "false");
        Serial.println(")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void ConfigManagerTest::assertEqualString(const char* expected, const char* actual, const char* message) {
    if (strcmp(expected, actual) != 0) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: \"");
        Serial.print(expected);
        Serial.print("\", 实际: \"");
        Serial.print(actual);
        Serial.println("\")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}