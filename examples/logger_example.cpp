/**
 * Logger使用示例
 * 展示改进后的Logger功能
 */

#include <Arduino.h>
#include "../src/common/Logger.h"
#include "../src/common/Config.h"

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // === 基本使用示例 ===
    Serial.println("=== Logger改进功能演示 ===\n");
    
    // 1. 使用默认配置初始化Logger
    Logger::getInstance().begin(&Serial, LogLevel::DEBUG);
    LOG_INFO("Logger初始化完成 - 使用默认配置");
    
    // 2. 演示不同日志级别
    LOG_DEBUG("这是DEBUG级别的日志");
    LOG_INFO("这是INFO级别的日志");
    LOG_WARN("这是WARN级别的日志");
    LOG_ERROR("这是ERROR级别的日志");
    
    delay(1000);
    
    // 3. 演示带标签的日志
    LOG_TAG_DEBUG("GPIO", "GPIO引脚%d初始化", 7);
    LOG_TAG_INFO("TIMER", "定时器间隔设置为%dms", 1000);
    LOG_TAG_WARN("MEMORY", "内存使用率: %.1f%%", 75.5);
    LOG_TAG_ERROR("NETWORK", "连接失败，错误代码: %d", -1);
    
    delay(1000);
    
    // 4. 演示自定义配置
    Serial.println("\n--- 自定义配置演示 ---");
    LoggerConfig customConfig;
    customConfig.showTimestamp = true;
    customConfig.showLevel = true;
    customConfig.showTag = true;
    customConfig.useColors = false;  // 串口监视器通常不支持颜色
    customConfig.useMilliseconds = true;
    customConfig.bufferSize = 1024;  // 更大的缓冲区
    
    Logger::getInstance().setConfig(customConfig);
    LOG_TAG_INFO("CONFIG", "已应用自定义配置");
    
    // 5. 演示级别过滤
    Serial.println("\n--- 日志级别过滤演示 ---");
    Logger::getInstance().setLevel(LogLevel::WARN);
    LOG_TAG_INFO("FILTER", "这条INFO日志不会显示");
    LOG_TAG_DEBUG("FILTER", "这条DEBUG日志不会显示");
    LOG_TAG_WARN("FILTER", "这条WARN日志会显示");
    LOG_TAG_ERROR("FILTER", "这条ERROR日志会显示");
    
    // 恢复DEBUG级别
    Logger::getInstance().setLevel(LogLevel::DEBUG);
    
    delay(1000);
    
    // 6. 演示性能优化的宏
    Serial.println("\n--- 性能优化演示 ---");
    LOG_TAG_INFO("PERF", "使用优化宏，只有在级别启用时才会执行格式化");
    
    // 7. 演示便捷宏（使用文件名作为标签）
    Serial.println("\n--- 便捷宏演示 ---");
    LOG_D("使用文件名作为标签的DEBUG日志");
    LOG_I("使用文件名作为标签的INFO日志");
    LOG_W("使用文件名作为标签的WARN日志");
    LOG_E("使用文件名作为标签的ERROR日志");
    
    delay(1000);
    
    // 8. 演示缓冲区管理
    Serial.println("\n--- 缓冲区管理演示 ---");
    String longMessage = "这是一个很长的日志消息，用来测试缓冲区管理功能。";
    for (int i = 0; i < 10; i++) {
        longMessage += "重复内容" + String(i) + " ";
    }
    LOG_TAG_INFO("BUFFER", longMessage.c_str());
    
    // 9. 演示错误处理
    Serial.println("\n--- 错误处理演示 ---");
    LOG_TAG_INFO("ERROR_HANDLE", "Logger具有完善的错误处理机制");
    LOG_TAG_INFO("ERROR_HANDLE", "包括缓冲区溢出保护和空指针检查");
    
    // 10. 演示配置查询
    Serial.println("\n--- 配置查询演示 ---");
    LoggerConfig currentConfig = Logger::getInstance().getConfig();
    LOG_TAG_INFO("CONFIG", "当前缓冲区大小: %d", currentConfig.bufferSize);
    LOG_TAG_INFO("CONFIG", "显示时间戳: %s", currentConfig.showTimestamp ? "是" : "否");
    LOG_TAG_INFO("CONFIG", "显示毫秒: %s", currentConfig.useMilliseconds ? "是" : "否");
    
    Serial.println("\n=== Logger功能演示完成 ===");
}

void loop() {
    // 演示运行时日志
    static unsigned long lastTime = 0;
    static int counter = 0;
    
    if (millis() - lastTime > 5000) {  // 每5秒输出一次
        lastTime = millis();
        counter++;
        
        LOG_TAG_DEBUG("LOOP", "循环计数: %d, 运行时间: %lu ms", counter, millis());
        
        // 演示不同类型的运行时信息
        if (counter % 3 == 0) {
            LOG_TAG_WARN("LOOP", "这是第%d次警告消息", counter / 3);
        }
        
        if (counter >= 10) {
            LOG_TAG_INFO("LOOP", "演示循环结束，重置计数器");
            counter = 0;
        }
    }
    
    delay(100);
}