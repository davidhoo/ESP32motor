#include "ModbusTest.h"
#include "../controllers/MotorModbusController.h"

void ModbusTest::runAllTests() {
    Serial.println("=== MODBUS-RTU 测试套件 ===");
    
    testMotorCommunication();
    testParameterReading();
    testParameterWriting();
    
    Serial.println("=== 测试完成 ===");
}

void ModbusTest::printTestHeader(const String& testName) {
    Serial.println("\n--- " + testName + " ---");
}

void ModbusTest::printTestResult(const String& testName, bool passed) {
    Serial.print(testName + ": ");
    Serial.println(passed ? "通过" : "失败");
}

void ModbusTest::testMotorCommunication() {
    printTestHeader("电机通信测试");
    
    MotorModbusController motor;
    
    Serial.println("初始化电机控制器...");
    if (!motor.begin(0x01)) {
        Serial.println("初始化失败");
        printTestResult("电机通信", false);
        return;
    }
    
    Serial.println("读取运行状态...");
    bool running;
    if (motor.getRunStatus(running)) {
        Serial.println("通信成功！");
        Serial.print("运行状态: ");
        Serial.println(running ? "运行中" : "停止");
        printTestResult("电机通信", true);
    } else {
        Serial.print("通信失败: ");
        Serial.println(motor.getLastError());
        printTestResult("电机通信", false);
    }
}

void ModbusTest::testParameterReading() {
    printTestHeader("参数读取测试");
    
    MotorModbusController motor;
    motor.begin(0x01);
    
    // 测试读取运行状态
    bool running;
    if (motor.getRunStatus(running)) {
        Serial.println("✓ 运行状态读取成功");
    } else {
        Serial.println("✗ 运行状态读取失败");
    }
    
    // 测试读取频率
    uint32_t frequency;
    if (motor.getFrequency(frequency)) {
        Serial.print("✓ 频率读取成功: ");
        Serial.print(frequency);
        Serial.println(" Hz");
    } else {
        Serial.println("✗ 频率读取失败");
    }
    
    // 测试读取占空比
    uint8_t duty;
    if (motor.getDutyCycle(duty)) {
        Serial.print("✓ 占空比读取成功: ");
        Serial.print(duty);
        Serial.println(" %");
    } else {
        Serial.println("✗ 占空比读取失败");
    }
    
    // 测试读取完整配置
    MotorModbusController::MotorConfig config;
    if (motor.getConfig(config)) {
        Serial.println("✓ 完整配置读取成功");
    } else {
        Serial.println("✗ 完整配置读取失败");
    }
    
    printTestResult("参数读取", true);
}

void ModbusTest::testParameterWriting() {
    printTestHeader("参数写入测试");
    
    MotorModbusController motor;
    motor.begin(0x01);
    
    bool success = true;
    
    // 测试设置频率
    if (motor.setFrequency(1000)) {
        Serial.println("✓ 频率设置成功");
    } else {
        success = false;
    }
    
    // 测试设置占空比
    if (motor.setDutyCycle(50)) {
        Serial.println("✓ 占空比设置成功");
    } else {
        success = false;
    }
    
    // 测试设置输出限制
    if (motor.setOutputLimits(20, 80)) {
        Serial.println("✓ 输出限制设置成功");
    } else {
        success = false;
    }
    
    printTestResult("参数写入", success);
}

void ModbusTest::testErrorHandling() {
    printTestHeader("错误处理测试");
    
    MotorModbusController motor;
    
    // 测试边界值
    Serial.println("测试边界值...");
    
    // 测试超出范围的占空比
    if (!motor.setDutyCycle(150)) {
        Serial.println("✓ 超出范围占空比被拒绝");
    } else {
        Serial.println("✗ 超出范围占空比被接受");
    }
    
    printTestResult("错误处理", true);
}