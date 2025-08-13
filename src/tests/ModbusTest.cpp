#include "ModbusTest.h"
#include "../common/Logger.h"

ModbusTest::ModbusTest(MotorModbusController& controller) : modbusController(controller) {}

void ModbusTest::runAllTests() {
    Serial.println("\n========================================");
    Serial.println("=== MODBUS测试 ===");
    Serial.println("========================================");
    
    // 初始化
    testInit();
    delay(1000);
    
    // 读取所有参数
    testReadStatus();
    delay(1000);
    
    testReadFrequency();
    delay(1000);
    
    testReadDuty();
    delay(1000);
    
    testReadConfig();
    delay(1000);
    
    testGetAllConfig();
    delay(1000);
    
    // 设置新参数
    testSetFrequency();
    delay(1000);
    
    testSetDuty();
    delay(1000);
    
    // 启动电机
    testStartMotor();
    delay(3000); // 运行3秒
    
    // 停止电机
    testStopMotor();
    
    Serial.println("\n✅ MODBUS测试完成！");
}

void ModbusTest::testInit() {
    Serial.println("\n========================================");
    Serial.println("=== 初始化MODBUS通信 ===");
    Serial.println("========================================");
    
    if (modbusController.begin(0x01)) {
        Serial.println("✅ MODBUS初始化成功");
        Serial.println("   GPIO8: RX (连接调速器TX)");
        Serial.println("   GPIO9: TX (连接调速器RX)");
        Serial.println("   波特率: 9600 bps");
    } else {
        Serial.println("❌ MODBUS初始化失败");
    }
}

void ModbusTest::testReadStatus() {
    Serial.println("\n========================================");
    Serial.println("=== 读取运行状态 ===");
    Serial.println("========================================");
    
    bool running;
    printResult(modbusController.getRunStatus(running), "读取运行状态");
    if (modbusController.getRunStatus(running)) {
        Serial.print("   当前状态: ");
        Serial.println(running ? "运行中" : "已停止");
    }
}

void ModbusTest::testReadFrequency() {
    Serial.println("\n========================================");
    Serial.println("=== 读取频率 ===");
    Serial.println("========================================");
    
    uint32_t freq;
    printResult(modbusController.getFrequency(freq), "读取频率");
    if (modbusController.getFrequency(freq)) {
        Serial.print("   当前频率: ");
        Serial.print(freq);
        Serial.println(" Hz");
    }
}

void ModbusTest::testReadDuty() {
    Serial.println("\n========================================");
    Serial.println("=== 读取占空比 ===");
    Serial.println("========================================");
    
    uint8_t duty;
    printResult(modbusController.getDutyCycle(duty), "读取占空比");
    if (modbusController.getDutyCycle(duty)) {
        Serial.print("   当前占空比: ");
        Serial.print(duty);
        Serial.println(" %");
    }
}

void ModbusTest::testReadConfig() {
    Serial.println("\n========================================");
    Serial.println("=== 读取完整配置 ===");
    Serial.println("========================================");
    
    MotorModbusController::MotorConfig config;
    printResult(modbusController.getConfig(config), "读取配置");
    if (modbusController.getConfig(config)) {
        Serial.println("   配置详情:");
        Serial.print("   - 模块地址: ");
        Serial.println(config.moduleAddress);
        Serial.print("   - 最小输出: ");
        Serial.print(config.minOutput);
        Serial.println(" %");
        Serial.print("   - 最大输出: ");
        Serial.print(config.maxOutput);
        Serial.println(" %");
        Serial.print("   - 缓启动时间: ");
        Serial.print(config.softStartTime * 0.1);
        Serial.println(" 秒");
        Serial.print("   - 缓停止时间: ");
        Serial.print(config.softStopTime * 0.1);
        Serial.println(" 秒");
    }
}

void ModbusTest::testGetAllConfig() {
    Serial.println("\n========================================");
    Serial.println("=== 一次性读取所有配置 ===");
    Serial.println("========================================");
    
    MotorModbusController::AllConfig config;
    printResult(modbusController.getAllConfig(config), "一次性读取所有配置");
    if (modbusController.getAllConfig(config)) {
        Serial.println("   所有配置详情:");
        Serial.print("   - 外接开关功能: ");
        Serial.println(config.externalSwitch ? "开启" : "关闭");
        Serial.print("   - 0-10V控制功能: ");
        Serial.println(config.analogControl ? "开启" : "关闭");
        Serial.print("   - 开机上电默认状态: ");
        Serial.println(config.powerOnState ? "运行" : "停止");
        Serial.print("   - 最小输出: ");
        Serial.print(config.minOutput);
        Serial.println(" %");
        Serial.print("   - 最大输出: ");
        Serial.print(config.maxOutput);
        Serial.println(" %");
        Serial.print("   - 缓启动时间: ");
        Serial.print(config.softStartTime * 0.1);
        Serial.println(" 秒");
        Serial.print("   - 缓停止时间: ");
        Serial.print(config.softStopTime * 0.1);
        Serial.println(" 秒");
        Serial.print("   - 运行状态: ");
        Serial.println(config.isRunning ? "运行中" : "已停止");
        Serial.print("   - 频率: ");
        Serial.print(config.frequency);
        Serial.println(" Hz");
        Serial.print("   - 占空比: ");
        Serial.print(config.dutyCycle);
        Serial.println(" %");
    }
}

void ModbusTest::testSetFrequency() {
    Serial.println("\n========================================");
    Serial.println("=== 设置新频率 (1000Hz) ===");
    Serial.println("========================================");
    
    printResult(modbusController.setFrequency(1000), "设置频率为1000Hz");
}

void ModbusTest::testSetDuty() {
    Serial.println("\n========================================");
    Serial.println("=== 设置新占空比 (75%) ===");
    Serial.println("========================================");
    
    printResult(modbusController.setDutyCycle(75), "设置占空比为75%");
}

void ModbusTest::testStartMotor() {
    Serial.println("\n========================================");
    Serial.println("=== 启动电机 ===");
    Serial.println("========================================");
    
    printResult(modbusController.start(), "启动电机");
}

void ModbusTest::testStopMotor() {
    Serial.println("\n========================================");
    Serial.println("=== 停止电机 ===");
    Serial.println("========================================");
    
    printResult(modbusController.stop(), "停止电机");
}

void ModbusTest::printResult(bool success, const char* operation) {
    if (success) {
        Serial.print("✅ ");
        Serial.print(operation);
        Serial.println(" 成功");
    } else {
        Serial.print("❌ ");
        Serial.print(operation);
        Serial.print(" 失败: ");
        Serial.println(modbusController.getLastError());
    }
}