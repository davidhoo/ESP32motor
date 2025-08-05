#include <Arduino.h>
#include "../controllers/ConfigManager.h"
#include "../tests/ConfigManagerTest.h"

/**
 * ConfigManager测试程序
 * 用于验证配置管理器的所有功能
 */
void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== ConfigManager 功能验证测试 ===");
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
        Serial.print("错误: ");
        Serial.println(configManager.getLastError());
        return;
    }
    
    Serial.println();
    Serial.println("2. 运行单元测试...");
    ConfigManagerTest::runAllTests();
    
    Serial.println();
    Serial.println("3. 功能验证完成！");
}

void loop() {
    // 空循环，测试完成后停止
    delay(1000);
}