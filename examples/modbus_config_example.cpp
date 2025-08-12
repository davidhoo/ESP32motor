#include <Arduino.h>
#include "../src/controllers/MotorModbusController.h"

MotorModbusController motorController;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println("=== MODBUS-RTU 调速器配置示例 ===");
    
    if (!motorController.begin(0x01)) {
        Serial.println("MODBUS初始化失败！");
        return;
    }
    
    Serial.println("MODBUS初始化成功！");
    Serial.println("准备配置调速器参数...");
    
    Serial.println("\n=== 配置调速器参数 ===");
    
    // 设置外接开关功能关闭
    if (motorController.setExternalSwitch(false)) {
        Serial.println("✓ 外接开关功能已关闭");
    } else {
        Serial.println("✗ 设置外接开关功能失败");
    }
    
    // 设置0-10V控制功能关闭
    if (motorController.setAnalogControl(false)) {
        Serial.println("✓ 0-10V控制功能已关闭");
    } else {
        Serial.println("✗ 设置0-10V控制功能失败");
    }
    
    // 设置开机默认状态为停止
    if (motorController.setPowerOnState(false)) {
        Serial.println("✓ 开机默认状态设置为停止");
    } else {
        Serial.println("✗ 设置开机默认状态失败");
    }
    
    // 设置输出限制
    if (motorController.setOutputLimits(10, 90)) {  // 10%-90%
        Serial.println("✓ 输出限制设置为10%-90%");
    } else {
        Serial.println("✗ 设置输出限制失败");
    }
    
    // 设置缓启动/停止时间
    if (motorController.setSoftTimes(50, 50)) {  // 5秒
        Serial.println("✓ 缓启动/停止时间设置为5秒");
    } else {
        Serial.println("✗ 设置缓启动/停止时间失败");
    }
    
    Serial.println("\n配置完成！");
}

void loop() {
    static bool configured = false;
    
    if (!configured) {
        delay(2000);
        Serial.println("\n=== 验证配置结果 ===");
        
        MotorModbusController::MotorConfig config;
        if (motorController.getConfig(config)) {
            Serial.println("当前配置:");
            Serial.print("  模块地址: ");
            Serial.println(config.moduleAddress);
            Serial.print("  外接开关: ");
            Serial.println(config.externalSwitch ? "开启" : "关闭");
            Serial.print("  0-10V控制: ");
            Serial.println(config.analogControl ? "开启" : "关闭");
            Serial.print("  开机状态: ");
            Serial.println(config.powerOnState ? "运行" : "停止");
            Serial.print("  最小输出: ");
            Serial.print(config.minOutput);
            Serial.println(" %");
            Serial.print("  最大输出: ");
            Serial.print(config.maxOutput);
            Serial.println(" %");
            Serial.print("  缓启动时间: ");
            Serial.print(config.softStartTime * 0.1);
            Serial.println(" 秒");
            Serial.print("  缓停止时间: ");
            Serial.print(config.softStopTime * 0.1);
            Serial.println(" 秒");
        } else {
            Serial.print("读取配置失败: ");
            Serial.println(motorController.getLastError());
        }
        
        configured = true;
        Serial.println("\n配置验证完成！");
    }
    
    delay(5000);
}