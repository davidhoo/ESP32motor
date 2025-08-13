#include <Arduino.h>
#include "../src/controllers/MotorModbusController.h"

MotorModbusController motorController;

void setup() {
    Serial.begin(115200);
    while (!Serial) {
        delay(100);
    }
    
    Serial.println("=== MODBUS-RTU 调速器测试程序 ===");
    Serial.println("初始化MODBUS通信...");
    
    if (!motorController.begin(0x01)) {
        Serial.println("MODBUS初始化失败！");
        return;
    }
    
    Serial.println("MODBUS初始化成功！");
    Serial.println("GPIO8: RX (连接调速器TX)");
    Serial.println("GPIO9: TX (连接调速器RX)");
    Serial.println("波特率: 9600 bps");
    Serial.println("数据格式: 8N1");
    Serial.println("--------------------------------");
}

void loop() {
    static unsigned long lastReadTime = 0;
    
    if (millis() - lastReadTime >= 2000) {  // 每2秒读取一次
        lastReadTime = millis();
        
        Serial.println("\n=== 读取调速器参数 ===");
        
        // 获取运行状态
        bool running;
        if (motorController.getRunStatus(running)) {
            Serial.print("运行状态: ");
            Serial.println(running ? "运行中" : "停止");
        } else {
            Serial.print("读取运行状态失败: ");
            Serial.println(motorController.getLastError());
        }
        
        // 获取频率
        uint32_t frequency;
        if (motorController.getFrequency(frequency)) {
            Serial.print("当前频率: ");
            Serial.print(frequency);
            Serial.println(" Hz");
        } else {
            Serial.print("读取频率失败: ");
            Serial.println(motorController.getLastError());
        }
        
        // 获取占空比
        uint8_t dutyCycle;
        if (motorController.getDutyCycle(dutyCycle)) {
            Serial.print("当前占空比: ");
            Serial.print(dutyCycle);
            Serial.println(" %");
        } else {
            Serial.print("读取占空比失败: ");
            Serial.println(motorController.getLastError());
        }
        
        // 获取完整配置
        MotorModbusController::MotorConfig config;
        if (motorController.getConfig(config)) {
            Serial.println("\n当前配置:");
            Serial.print("  模块地址: ");
            Serial.println(config.moduleAddress);
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
        
        Serial.println("================================");
    }
    
    // 处理串口命令
    if (Serial.available()) {
        String command = Serial.readStringUntil('\n');
        command.trim();
        
        if (command.startsWith("freq ")) {
            uint32_t freq = command.substring(5).toInt();
            if (motorController.setFrequency(freq)) {
                Serial.print("成功设置频率: ");
                Serial.println(freq);
            } else {
                Serial.print("设置频率失败: ");
                Serial.println(motorController.getLastError());
            }
        } else if (command.startsWith("duty ")) {
            uint8_t duty = command.substring(5).toInt();
            if (duty <= 100 && motorController.setDutyCycle(duty)) {
                Serial.print("成功设置占空比: ");
                Serial.println(duty);
            } else {
                Serial.println("占空比范围: 0-100%");
            }
        } else if (command == "start") {
            if (motorController.start()) {
                Serial.println("成功启动输出");
            } else {
                Serial.print("启动失败: ");
                Serial.println(motorController.getLastError());
            }
        } else if (command == "stop") {
            if (motorController.stop()) {
                Serial.println("成功停止输出");
            } else {
                Serial.print("停止失败: ");
                Serial.println(motorController.getLastError());
            }
        } else if (command == "help") {
            Serial.println("可用命令:");
            Serial.println("  freq <值> - 设置频率 (Hz)");
            Serial.println("  duty <值> - 设置占空比 (0-100)");
            Serial.println("  start - 启动输出");
            Serial.println("  stop - 停止输出");
            Serial.println("  help - 显示帮助");
        }
    }
}