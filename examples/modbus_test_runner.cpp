#include <Arduino.h>
#include "../src/controllers/MotorModbusController.h"

MotorModbusController motor;

// 测试步骤
enum TestStep {
    TEST_INIT,
    TEST_READ_STATUS,
    TEST_READ_FREQUENCY,
    TEST_READ_DUTY,
    TEST_READ_CONFIG,
    TEST_SET_FREQUENCY,
    TEST_SET_DUTY,
    TEST_START_MOTOR,
    TEST_DELAY,
    TEST_STOP_MOTOR,
    TEST_COMPLETE
};

TestStep currentStep = TEST_INIT;
unsigned long stepStartTime = 0;
const unsigned long STEP_DELAY = 3000; // 每步3秒

void printTestHeader(const char* testName) {
    Serial.println("\n========================================");
    Serial.print("=== ");
    Serial.print(testName);
    Serial.println(" ===");
    Serial.println("========================================");
}

void printResult(bool success, const char* operation) {
    if (success) {
        Serial.print("✅ ");
        Serial.print(operation);
        Serial.println(" 成功");
    } else {
        Serial.print("❌ ");
        Serial.print(operation);
        Serial.print(" 失败: ");
        Serial.println(motor.getLastError());
    }
}

void showHelp() {
    Serial.println("\n========================================");
    Serial.println("🚀 MODBUS-RTU 测试程序命令菜单");
    Serial.println("========================================");
    Serial.println("1. 初始化通信");
    Serial.println("2. 读取运行状态");
    Serial.println("3. 读取频率");
    Serial.println("4. 读取占空比");
    Serial.println("5. 读取完整配置");
    Serial.println("6. 设置新频率 (1000Hz)");
    Serial.println("7. 设置新占空比 (75%)");
    Serial.println("8. 启动电机");
    Serial.println("9. 停止电机");
    Serial.println("a. 完整自动测试");
    Serial.println("h. 显示此帮助");
    Serial.println("========================================");
}

void testInit() {
    printTestHeader("初始化MODBUS通信");
    if (motor.begin(0x01)) {
        Serial.println("✅ MODBUS初始化成功");
        Serial.println("   GPIO8: RX (连接调速器TX)");
        Serial.println("   GPIO9: TX (连接调速器RX)");
        Serial.println("   波特率: 9600 bps");
    } else {
        Serial.println("❌ MODBUS初始化失败");
    }
}

void testReadStatus() {
    printTestHeader("读取运行状态");
    bool running;
    printResult(motor.getRunStatus(running), "读取运行状态");
    if (motor.getRunStatus(running)) {
        Serial.print("   当前状态: ");
        Serial.println(running ? "运行中" : "已停止");
    }
}

void testReadFrequency() {
    printTestHeader("读取频率");
    uint32_t freq;
    printResult(motor.getFrequency(freq), "读取频率");
    if (motor.getFrequency(freq)) {
        Serial.print("   当前频率: ");
        Serial.print(freq);
        Serial.println(" Hz");
    }
}

void testReadDuty() {
    printTestHeader("读取占空比");
    uint8_t duty;
    printResult(motor.getDutyCycle(duty), "读取占空比");
    if (motor.getDutyCycle(duty)) {
        Serial.print("   当前占空比: ");
        Serial.print(duty);
        Serial.println(" %");
    }
}

void testReadConfig() {
    printTestHeader("读取完整配置");
    MotorModbusController::MotorConfig config;
    printResult(motor.getConfig(config), "读取配置");
    if (motor.getConfig(config)) {
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

void testSetFrequency() {
    printTestHeader("设置新频率 (1000Hz)");
    printResult(motor.setFrequency(1000), "设置频率为1000Hz");
}

void testSetDuty() {
    printTestHeader("设置新占空比 (75%)");
    printResult(motor.setDutyCycle(75), "设置占空比为75%");
}

void testStartMotor() {
    printTestHeader("启动电机");
    printResult(motor.start(), "启动电机");
}

void testStopMotor() {
    printTestHeader("停止电机");
    printResult(motor.stop(), "停止电机");
}

void runFullTest() {
    Serial.println("\n🚀 开始完整测试流程...");
    
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
    
    Serial.println("\n✅ 完整测试流程完成！");
}

void setup() {
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    Serial.println("\n🚀 MODBUS-RTU 调速器测试程序");
    Serial.println("ESP32-S3-Zero <-> 调速器通信测试");
    showHelp();
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        // 清空缓冲区
        while (Serial.available()) Serial.read();
        
        switch (command) {
            case '1':
                testInit();
                break;
            case '2':
                testReadStatus();
                break;
            case '3':
                testReadFrequency();
                break;
            case '4':
                testReadDuty();
                break;
            case '5':
                testReadConfig();
                break;
            case '6':
                testSetFrequency();
                break;
            case '7':
                testSetDuty();
                break;
            case '8':
                testStartMotor();
                break;
            case '9':
                testStopMotor();
                break;
            case 'a':
            case 'A':
                runFullTest();
                break;
            case 'h':
            case 'H':
                showHelp();
                break;
            default:
                Serial.println("❌ 无效命令，输入h查看帮助");
                break;
        }
    }
}