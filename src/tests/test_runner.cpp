#include <Arduino.h>
#include "../src/common/Logger.h"
#include "../src/tests/GPIOTest.h"
#include "../src/tests/TimerTest.h"
#include "../src/tests/WS2812Test.h"
#include "../src/tests/NVSStorageTest.h"
#include "../src/tests/LEDControllerTest.h"
#include "../src/tests/ConfigManagerTest.h"
#include "../src/tests/MotorControllerTest.h"
#include "../src/tests/MotorBLEServerTest.h"
#include "../src/tests/EventManagerTest.h"
#include "../src/tests/StateManagerTest.h"
#include "../src/tests/MotorCycleTest.h"
#include "../src/tests/BLEInteractionTest.h"
#include "../src/tests/ErrorHandlingTest.h"
#include "../src/tests/ModbusTest.h"

// 全局对象
GPIODriver gpioDriver;
GPIOTest gpioTest(&gpioDriver);
TimerTest timerTest;
WS2812Driver ws2812Driver(21, 1);  // GPIO 21, 1个LED
WS2812Test ws2812Test(&ws2812Driver);
NVSStorageDriver nvsStorageDriver;
LEDController ledController;

MotorModbusController modbusController;
ModbusTest modbusTest(modbusController);

// 测试模式选择
enum TestMode {
    ALL_TESTS_MODE = 0,
    GPIO_TEST_MODE = 1,
    TIMER_TEST_MODE = 2,
    WS2812_TEST_MODE = 3,
    NVS_STORAGE_TEST_MODE = 4,
    LED_CONTROLLER_TEST_MODE = 5,
    CONFIG_MANAGER_TEST_MODE = 6,
    MOTOR_CONTROLLER_TEST_MODE = 7,
    BLE_SERVER_TEST_MODE = 8,
    EVENT_MANAGER_TEST_MODE = 9,
    STATE_MANAGER_TEST_MODE = 10,
    MOTOR_CYCLE_TEST_MODE = 11,
    BLE_INTERACTION_TEST_MODE = 12,
    ERROR_HANDLING_TEST_MODE = 13,
    MODBUS_TEST_MODE = 14,
    MODBUS_INIT_TEST_MODE = 15,
    MODBUS_READ_STATUS_TEST_MODE = 16,
    MODBUS_READ_FREQUENCY_TEST_MODE = 17,
    MODBUS_READ_DUTY_TEST_MODE = 18,
    MODBUS_READ_CONFIG_TEST_MODE = 19,
    MODBUS_SET_FREQUENCY_TEST_MODE = 20,
    MODBUS_SET_DUTY_TEST_MODE = 21,
    MODBUS_START_MOTOR_TEST_MODE = 22,
    MODBUS_STOP_MOTOR_TEST_MODE = 23,
    MODBUS_GET_ALL_CONFIG_TEST_MODE = 24
};

// 当前测试模式
TestMode currentTestMode = ALL_TESTS_MODE;

// 函数声明
void runAllTests();
void runGPIOTests();
void runTimerTests();
void runWS2812Tests();
void runNVSStorageTests();
void runLEDControllerTests();
void runConfigManagerTests();
void runMotorControllerTests();
void runBLEServerTests();
void runEventManagerTests();
void runStateManagerTests();
void runMotorCycleTests();
void runBLEInteractionTests();
void runErrorHandlingTests();
void runModbusTests();
void runModbusInitTests();
void runModbusReadStatusTests();
void runModbusReadFrequencyTests();
void runModbusReadDutyTests();
void runModbusReadConfigTests();
void runModbusSetFrequencyTests();
void runModbusSetDutyTests();
void runModbusStartMotorTests();
void runModbusStopMotorTests();
void runModbusGetAllConfigTests();

void showHelp() {
    Serial.println("\n========================================");
    Serial.println("🚀 ESP32电机控制器测试程序命令菜单");
    Serial.println("========================================");
    Serial.println("0. 运行所有测试");
    Serial.println("1. GPIO驱动测试");
    Serial.println("2. 定时器驱动测试");
    Serial.println("3. WS2812驱动测试");
    Serial.println("4. NVS存储驱动测试");
    Serial.println("5. LED控制器测试");
    Serial.println("6. ConfigManager测试");
    Serial.println("7. MotorController测试");
    Serial.println("8. BLE服务器测试");
    Serial.println("9. EventManager测试");
    Serial.println("a. StateManager测试");
    Serial.println("b. 电机循环控制测试");
    Serial.println("c. BLE交互流程测试");
    Serial.println("d. 错误处理测试");
    Serial.println("e. MODBUS初始化测试");
    Serial.println("f. MODBUS读取运行状态测试");
    Serial.println("g. MODBUS读取频率测试");
    Serial.println("i. MODBUS读取占空比测试");
    Serial.println("j. MODBUS读取配置测试");
    Serial.println("k. MODBUS设置频率测试");
    Serial.println("l. MODBUS设置占空比测试");
    Serial.println("m. MODBUS启动电机测试");
    Serial.println("n. MODBUS停止电机测试");
    Serial.println("o. MODBUS一次性读取所有配置测试");
    Serial.println("h. 显示此帮助");
    Serial.println("========================================");
}

void printTestHeader(const char* testName) {
    Serial.println("\n========================================");
    Serial.print("=== ");
    Serial.print(testName);
    Serial.println(" ===");
    Serial.println("========================================");
}

void setup() {
    // 初始化串口
    Serial.begin(115200);
    while (!Serial) delay(100);
    
    Serial.println("\n🚀 ESP32电机控制器测试程序");
    Serial.println("ESP32-S3-Zero 驱动测试");
    showHelp();
    
    // 初始化日志系统
    LoggerConfig logConfig;
    logConfig.showTimestamp = true;
    logConfig.showLevel = true;
    logConfig.showTag = true;
    logConfig.useColors = false;
    logConfig.useMilliseconds = true;
    logConfig.bufferSize = 256;
    
    Logger::getInstance().begin(&Serial, LogLevel::INFO, logConfig);
}

void loop() {
    if (Serial.available()) {
        char command = Serial.read();
        
        // 清空缓冲区
        while (Serial.available()) Serial.read();
        
        switch (command) {
            case '0':
                runAllTests();
                break;
            case '1':
                runGPIOTests();
                break;
            case '2':
                runTimerTests();
                break;
            case '3':
                runWS2812Tests();
                break;
            case '4':
                runNVSStorageTests();
                break;
            case '5':
                runLEDControllerTests();
                break;
            case '6':
                runConfigManagerTests();
                break;
            case '7':
                runMotorControllerTests();
                break;
            case '8':
                runBLEServerTests();
                break;
            case '9':
                runEventManagerTests();
                break;
            case 'a':
            case 'A':
                runStateManagerTests();
                break;
            case 'b':
            case 'B':
                runMotorCycleTests();
                break;
            case 'c':
            case 'C':
                runBLEInteractionTests();
                break;
            case 'd':
            case 'D':
                runErrorHandlingTests();
                break;
            case 'e':
            case 'E':
                runModbusInitTests();
                break;
            case 'f':
            case 'F':
                runModbusReadStatusTests();
                break;
            case 'g':
            case 'G':
                runModbusReadFrequencyTests();
                break;
            case 'i':
            case 'I':
                runModbusReadDutyTests();
                break;
            case 'j':
            case 'J':
                runModbusReadConfigTests();
                break;
            case 'k':
            case 'K':
                runModbusSetFrequencyTests();
                break;
            case 'l':
            case 'L':
                runModbusSetDutyTests();
                break;
            case 'm':
            case 'M':
                runModbusStartMotorTests();
                break;
            case 'n':
            case 'N':
                runModbusStopMotorTests();
                break;
            case 'o':
            case 'O':
                runModbusGetAllConfigTests();
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
    
    // 根据当前测试模式执行循环测试
    switch (currentTestMode) {
        case LED_CONTROLLER_TEST_MODE:
            // LED控制器测试需要持续更新
            {
                static unsigned long lastStateChange = 0;
                static int currentTestState = 0;
                
                // 每5秒切换一次状态
                if (millis() - lastStateChange > 5000) {
                    lastStateChange = millis();
                    
                    switch (currentTestState) {
                        case 0:
                            Serial.println("切换到系统初始化状态");
                            ledController.setState(LEDState::SYSTEM_INIT);
                            break;
                        case 1:
                            Serial.println("切换到电机运行状态");
                            ledController.setState(LEDState::MOTOR_RUNNING);
                            break;
                        case 2:
                            Serial.println("切换到电机停止状态");
                            ledController.setState(LEDState::MOTOR_STOPPED);
                            break;
                        case 3:
                            Serial.println("切换到BLE连接状态");
                            ledController.setState(LEDState::BLE_CONNECTED);
                            break;
                        case 4:
                            Serial.println("切换到BLE断开状态");
                            ledController.setState(LEDState::BLE_DISCONNECTED);
                            break;
                        case 5:
                            Serial.println("切换到错误状态");
                            ledController.setState(LEDState::ERROR_STATE);
                            break;
                    }
                    
                    currentTestState = (currentTestState + 1) % 6;
                }
                
                ledController.update();
                delay(100);
            }
            break;
            
        case MOTOR_CONTROLLER_TEST_MODE:
            // 电机控制器测试需要持续更新
            {
                static unsigned long lastUpdate = 0;
                static bool motorStarted = false;
                
                MotorController& motorController = MotorController::getInstance();
                
                // 每1秒更新一次状态
                if (millis() - lastUpdate > 1000) {
                    lastUpdate = millis();
                    
                    // 显示当前状态
                    MotorControllerState state = motorController.getCurrentState();
                    uint32_t runTime = motorController.getRemainingRunTime();
                    uint32_t stopTime = motorController.getRemainingStopTime();
                    uint32_t cycles = motorController.getCurrentCycleCount();
                    
                    Serial.printf("电机状态: %d, 剩余运行: %us, 剩余停止: %us, 循环次数: %lu\n",
                                 static_cast<int>(state), runTime, stopTime, cycles);
                }
                
                // 每10秒启动/停止一次电机
                if (millis() % 10000 == 0 && !motorStarted) {
                    Serial.println("启动电机...");
                    motorController.startMotor();
                    motorStarted = true;
                } else if (millis() % 10000 == 5000 && motorStarted) {
                    Serial.println("停止电机...");
                    motorController.stopMotor();
                    motorStarted = false;
                }
                
                // 更新电机控制器
                motorController.update();
                delay(100);
            }
            break;
                
        case BLE_SERVER_TEST_MODE:
            // BLE服务器测试需要持续更新
        {
            static unsigned long lastUpdate = 0;
            static bool bleInitialized = false;
            
            MotorBLEServer& bleServer = MotorBLEServer::getInstance();
            
            if (!bleInitialized) {
                // 初始化LED控制器用于状态指示
                static LEDController bleLedController;
                if (bleLedController.init()) {
                    bleLedController.setState(LEDState::BLE_CONNECTED);
                }
                bleInitialized = true;
            }
            
            // 每2秒更新一次状态
            if (millis() - lastUpdate > 2000) {
                lastUpdate = millis();
                
                // 显示当前状态
                bool connected = bleServer.isConnected();
                Serial.printf("BLE状态: %s\n", connected ? "已连接" : "未连接");
                
                // 发送状态通知
                if (connected) {
                    bleServer.update();
                }
            }
            
            // 更新BLE服务器
            bleServer.update();
            delay(100);
        }
            break;
            
        default:
            delay(100);
            break;
    }
}

/**
 * 运行所有测试
 */
void runAllTests() {
    printTestHeader("运行所有测试");
    
    runGPIOTests();
    delay(1000);
    
    runTimerTests();
    delay(1000);
    
    runWS2812Tests();
    delay(1000);
    
    runNVSStorageTests();
    delay(1000);
    
    runLEDControllerTests();
    delay(1000);
    
    runConfigManagerTests();
    delay(1000);
    
    runMotorControllerTests();
    delay(1000);
    
    runBLEServerTests();
    delay(1000);
    
    runEventManagerTests();
    delay(1000);
    
    runStateManagerTests();
    delay(1000);
    
    runMotorCycleTests();
    delay(1000);
    
    runBLEInteractionTests();
    delay(1000);
    
    runErrorHandlingTests();
    delay(1000);
    
    runModbusTests();
    
    Serial.println("\n✅ 所有测试完成！");
}

/**
 * 运行GPIO测试
 */
void runGPIOTests() {
    printTestHeader("GPIO驱动测试");
    gpioTest.initializeTest();
    Serial.println("✅ GPIO驱动测试完成");
    currentTestMode = GPIO_TEST_MODE;
}

/**
 * 运行定时器测试
 */
void runTimerTests() {
    printTestHeader("定时器驱动测试");
    timerTest.runAllTests();
    Serial.println("✅ 定时器驱动测试完成");
    currentTestMode = TIMER_TEST_MODE;
}

/**
 * 运行WS2812测试
 */
void runWS2812Tests() {
    printTestHeader("WS2812驱动测试");
    ws2812Test.initializeTest();
    Serial.println("✅ WS2812驱动测试完成");
    currentTestMode = WS2812_TEST_MODE;
}

/**
 * 运行NVS存储测试
 */
void runNVSStorageTests() {
    printTestHeader("NVS存储驱动测试");
    NVSStorageTest::runAllTests();
    Serial.println("✅ NVS存储驱动测试完成");
    currentTestMode = NVS_STORAGE_TEST_MODE;
}

/**
 * 运行LED控制器测试
 */
void runLEDControllerTests() {
    printTestHeader("LED控制器测试");
    
    // 初始化定时器驱动（LED控制器依赖定时器）
    TimerDriver::getInstance().init();
    
    // 初始化LED控制器
    if (ledController.init()) {
        LEDControllerTest::runAllTests();
        Serial.println("✅ LED控制器测试完成");
        Serial.println("将在loop()中每5秒切换一次LED状态进行演示");
        currentTestMode = LED_CONTROLLER_TEST_MODE;
    } else {
        Serial.println("❌ LED控制器初始化失败");
        currentTestMode = ALL_TESTS_MODE;
    }
}

/**
 * 运行ConfigManager测试
 */
void runConfigManagerTests() {
    printTestHeader("ConfigManager测试");
    
    // 获取配置管理器实例
    ConfigManager& configManager = ConfigManager::getInstance();
    
    if (configManager.init()) {
        ConfigManagerTest::runAllTests();
        Serial.println("✅ ConfigManager测试完成");
    } else {
        Serial.println("❌ ConfigManager初始化失败");
    }
    currentTestMode = CONFIG_MANAGER_TEST_MODE;
}

/**
 * 运行MotorController测试
 */
void runMotorControllerTests() {
    printTestHeader("MotorController测试");
    
    // 获取电机控制器实例
    MotorController& motorController = MotorController::getInstance();
    
    if (motorController.init()) {
        MotorControllerTest::runAllTests();
        Serial.println("✅ MotorController测试完成");
        Serial.println("将在loop()中每10秒启动/停止一次电机进行演示");
        currentTestMode = MOTOR_CONTROLLER_TEST_MODE;
    } else {
        Serial.println("❌ MotorController初始化失败");
        currentTestMode = ALL_TESTS_MODE;
    }
}

/**
 * 运行BLE服务器测试
 */
void runBLEServerTests() {
    printTestHeader("BLE服务器测试");
    
    // 获取BLE服务器实例
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    if (bleServer.init()) {
        bleServer.start();
        MotorBLEServerTest::runAllTests();
        Serial.println("✅ BLE服务器测试完成");
        Serial.println("将在loop()中持续更新BLE状态");
        currentTestMode = BLE_SERVER_TEST_MODE;
    } else {
        Serial.println("❌ BLE服务器初始化失败");
        currentTestMode = ALL_TESTS_MODE;
    }
}

/**
 * 运行EventManager测试
 */
void runEventManagerTests() {
    printTestHeader("EventManager测试");
    EventManagerTest::runAllTests();
    Serial.println("✅ EventManager测试完成");
    currentTestMode = EVENT_MANAGER_TEST_MODE;
}

/**
 * 运行StateManager测试
 */
void runStateManagerTests() {
    printTestHeader("StateManager测试");
    
    // 获取StateManager实例
    StateManager& stateManager = StateManager::getInstance();
    stateManager.init();
    
    StateManagerTest::runAllTests();
    Serial.println("✅ StateManager测试完成");
    currentTestMode = STATE_MANAGER_TEST_MODE;
}

/**
 * 运行电机循环控制测试
 */
void runMotorCycleTests() {
    printTestHeader("电机循环控制测试");
    MotorCycleTest::runAllTests();
    Serial.println("✅ 电机循环控制测试完成");
    currentTestMode = MOTOR_CYCLE_TEST_MODE;
}

/**
 * 运行BLE交互流程测试
 */
void runBLEInteractionTests() {
    printTestHeader("BLE交互流程测试");
    BLEInteractionTest bleInteractionTest;
    bleInteractionTest.runAllTests();
    Serial.println("✅ BLE交互流程测试完成");
    currentTestMode = BLE_INTERACTION_TEST_MODE;
}

/**
 * 运行错误处理测试
 */
void runErrorHandlingTests() {
    printTestHeader("错误处理测试");
    ErrorHandlingTest errorHandlingTest;
    errorHandlingTest.runAllTests();
    Serial.println("✅ 错误处理测试完成");
    currentTestMode = ERROR_HANDLING_TEST_MODE;
}
/**
 * 运行MODBUS测试
 */
void runModbusTests() {
    printTestHeader("MODBUS测试");
    modbusTest.runAllTests();
    Serial.println("✅ MODBUS测试完成");
    currentTestMode = MODBUS_TEST_MODE;
}

/**
 * 运行MODBUS初始化测试
 */
void runModbusInitTests() {
    printTestHeader("MODBUS初始化测试");
    modbusTest.testInit();
    Serial.println("✅ MODBUS初始化测试完成");
    currentTestMode = MODBUS_INIT_TEST_MODE;
}

/**
 * 运行MODBUS读取运行状态测试
 */
void runModbusReadStatusTests() {
    printTestHeader("MODBUS读取运行状态测试");
    modbusTest.testReadStatus();
    Serial.println("✅ MODBUS读取运行状态测试完成");
    currentTestMode = MODBUS_READ_STATUS_TEST_MODE;
}

/**
 * 运行MODBUS读取频率测试
 */
void runModbusReadFrequencyTests() {
    printTestHeader("MODBUS读取频率测试");
    modbusTest.testReadFrequency();
    Serial.println("✅ MODBUS读取频率测试完成");
    currentTestMode = MODBUS_READ_FREQUENCY_TEST_MODE;
}

/**
 * 运行MODBUS读取占空比测试
 */
void runModbusReadDutyTests() {
    printTestHeader("MODBUS读取占空比测试");
    modbusTest.testReadDuty();
    Serial.println("✅ MODBUS读取占空比测试完成");
    currentTestMode = MODBUS_READ_DUTY_TEST_MODE;
}

/**
 * 运行MODBUS读取配置测试
 */
void runModbusReadConfigTests() {
    printTestHeader("MODBUS读取配置测试");
    modbusTest.testReadConfig();
    Serial.println("✅ MODBUS读取配置测试完成");
    currentTestMode = MODBUS_READ_CONFIG_TEST_MODE;
}

/**
 * 运行MODBUS设置频率测试
 */
void runModbusSetFrequencyTests() {
    printTestHeader("MODBUS设置频率测试");
    modbusTest.testSetFrequency();
    Serial.println("✅ MODBUS设置频率测试完成");
    currentTestMode = MODBUS_SET_FREQUENCY_TEST_MODE;
}

/**
 * 运行MODBUS设置占空比测试
 */
void runModbusSetDutyTests() {
    printTestHeader("MODBUS设置占空比测试");
    modbusTest.testSetDuty();
    Serial.println("✅ MODBUS设置占空比测试完成");
    currentTestMode = MODBUS_SET_DUTY_TEST_MODE;
}

/**
 * 运行MODBUS启动电机测试
 */
void runModbusStartMotorTests() {
    printTestHeader("MODBUS启动电机测试");
    modbusTest.testStartMotor();
    Serial.println("✅ MODBUS启动电机测试完成");
    currentTestMode = MODBUS_START_MOTOR_TEST_MODE;
}

/**
 * 运行MODBUS停止电机测试
 */
void runModbusStopMotorTests() {
    printTestHeader("MODBUS停止电机测试");
    modbusTest.testStopMotor();
    Serial.println("✅ MODBUS停止电机测试完成");
    currentTestMode = MODBUS_STOP_MOTOR_TEST_MODE;
}

/**
 * 运行MODBUS一次性读取所有配置测试
 */
void runModbusGetAllConfigTests() {
    printTestHeader("MODBUS一次性读取所有配置测试");
    modbusTest.testGetAllConfig();
    Serial.println("✅ MODBUS一次性读取所有配置测试完成");
    currentTestMode = MODBUS_GET_ALL_CONFIG_TEST_MODE;
}