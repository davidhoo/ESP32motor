#include <Arduino.h>
#include "../src/controllers/ConfigManager.h"
#include "../src/tests/ConfigManagerTest.h"

/**
 * ConfigManager功能验证示例
 * 演示配置管理器的所有功能
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ConfigManager 功能验证示例 ===");
    Serial.println();
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG);
    
    // 获取配置管理器实例
    ConfigManager& configManager = ConfigManager::getInstance();
    
    Serial.println("1. 初始化配置管理器...");
    if (configManager.init()) {
        Serial.println("✅ 配置管理器初始化成功");
    } else {
        Serial.println("❌ 配置管理器初始化失败");
        Serial.println(configManager.getLastError());
        return;
    }
    
    Serial.println();
    Serial.println("2. 运行单元测试...");
    ConfigManagerTest::runAllTests();
    
    Serial.println();
    Serial.println("3. 演示配置操作...");
    
    // 显示当前配置
    const MotorConfig& currentConfig = configManager.getConfig();
    Serial.println("当前配置:");
    Serial.print("  运行时长: "); Serial.print(currentConfig.runDuration); Serial.println(" ms");
    Serial.print("  停止时长: "); Serial.print(currentConfig.stopDuration); Serial.println(" ms");
    Serial.print("  循环次数: "); Serial.println(currentConfig.cycleCount);
    Serial.print("  自动启动: "); Serial.println(currentConfig.autoStart ? "是" : "否");
    
    Serial.println();
    Serial.println("4. 更新配置...");
    
    // 创建新配置
    MotorConfig newConfig;
    newConfig.runDuration = 10;     // 10秒
    newConfig.stopDuration = 3;     // 3秒
    newConfig.cycleCount = 5;       // 5次循环
    newConfig.autoStart = false;    // 不自动启动
    
    configManager.updateConfig(newConfig);
    
    // 显示更新后的配置
    const MotorConfig& updatedConfig = configManager.getConfig();
    Serial.println("更新后的配置:");
    Serial.print("  运行时长: "); Serial.print(updatedConfig.runDuration); Serial.println(" ms");
    Serial.print("  停止时长: "); Serial.print(updatedConfig.stopDuration); Serial.println(" ms");
    Serial.print("  循环次数: "); Serial.println(updatedConfig.cycleCount);
    Serial.print("  自动启动: "); Serial.println(updatedConfig.autoStart ? "是" : "否");
    
    Serial.println();
    Serial.println("5. 保存配置...");
    if (configManager.saveConfig()) {
        Serial.println("✅ 配置保存成功");
    } else {
        Serial.println("❌ 配置保存失败");
        Serial.println(configManager.getLastError());
    }
    
    Serial.println();
    Serial.println("6. 重置为默认值...");
    configManager.resetToDefaults();
    
    const MotorConfig& defaultConfig = configManager.getConfig();
    Serial.println("重置后的配置:");
    Serial.print("  运行时长: "); Serial.print(defaultConfig.runDuration); Serial.println(" ms");
    Serial.print("  停止时长: "); Serial.print(defaultConfig.stopDuration); Serial.println(" ms");
    Serial.print("  循环次数: "); Serial.println(defaultConfig.cycleCount);
    Serial.print("  自动启动: "); Serial.println(defaultConfig.autoStart ? "是" : "否");
    
    Serial.println();
    Serial.println("7. 重新加载保存的配置...");
    if (configManager.loadConfig()) {
        Serial.println("✅ 配置加载成功");
        
        const MotorConfig& loadedConfig = configManager.getConfig();
        Serial.println("加载的配置:");
        Serial.print("  运行时长: "); Serial.print(loadedConfig.runDuration); Serial.println(" ms");
        Serial.print("  停止时长: "); Serial.print(loadedConfig.stopDuration); Serial.println(" ms");
        Serial.print("  循环次数: "); Serial.println(loadedConfig.cycleCount);
        Serial.print("  自动启动: "); Serial.println(loadedConfig.autoStart ? "是" : "否");
    } else {
        Serial.println("❌ 配置加载失败");
        Serial.println(configManager.getLastError());
    }
    
    Serial.println();
    Serial.println("8. 测试配置验证...");
    
    // 测试无效配置
    MotorConfig invalidConfig;
    invalidConfig.runDuration = 50;  // 无效值
    
    Serial.print("测试无效配置验证: ");
    if (configManager.validateConfig(invalidConfig)) {
        Serial.println("❌ 应该失败");
    } else {
        Serial.print("✅ 正确失败，错误: ");
        Serial.println(configManager.getValidationError());
    }
    
    Serial.println();
    Serial.println("=== ConfigManager 功能验证完成 ===");
}

void loop() {
    // 空循环
}