#include <Arduino.h>
#include "common/Config.h"
#include "common/Logger.h"
#include "drivers/GPIODriver.h"
#include "tests/GPIOTest.h"
#include "tests/TimerTest.h"
#include "drivers/WS2812Driver.h"
#include "tests/WS2812Test.h"
#include "drivers/NVSStorageDriver.h"
#include "tests/NVSStorageTest.h"
#include "controllers/LEDController.h"
#include "tests/LEDControllerTest.h"
#include "controllers/ConfigManager.h"
#include "tests/ConfigManagerTest.h"
#include "controllers/MotorController.h"
#include "tests/MotorControllerTest.h"
#include "controllers/MotorBLEServer.h"
#include "tests/MotorBLEServerTest.h"
#include "tests/EventManagerTest.h"
#include "tests/StateManagerTest.h"

// 全局对象
GPIODriver gpioDriver;
GPIOTest gpioTest(&gpioDriver);
TimerTest timerTest;
WS2812Driver ws2812Driver(21, 1);  // GPIO 21, 1个LED
WS2812Test ws2812Test(&ws2812Driver);
NVSStorageDriver nvsStorageDriver;
LEDController ledController;

// 测试模式选择
enum TestMode {
    GPIO_TEST_MODE = 0,
    TIMER_TEST_MODE = 1,
    COMBINED_TEST_MODE = 2,
    WS2812_TEST_MODE = 3,
    NVS_STORAGE_TEST_MODE = 4,
    LED_CONTROLLER_TEST_MODE = 5,
    CONFIG_MANAGER_TEST_MODE = 6,
    MOTOR_CONTROLLER_TEST_MODE = 7,
    BLE_SERVER_TEST_MODE = 8,
    EVENT_MANAGER_TEST_MODE = 9,
    STATE_MANAGER_TEST_MODE = 10
};

// 当前测试模式
TestMode currentTestMode = STATE_MANAGER_TEST_MODE; // 运行StateManager集成测试

// 函数声明
void runGPIOTests();
void runTimerTests();
void runCombinedTests();
void runWS2812Tests();
void runNVSStorageTests();
void runLEDControllerTests();
void runConfigManagerTests();
void runMotorControllerTests();
void runBLEServerTests();
void runEventManagerTests();
void runStateManagerTests();

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 配置日志系统
    LoggerConfig logConfig;
    logConfig.showTimestamp = LOG_SHOW_TIMESTAMP;
    logConfig.showLevel = LOG_SHOW_LEVEL;
    logConfig.showTag = LOG_SHOW_TAG;
    logConfig.useColors = LOG_ENABLE_COLORS;
    logConfig.useMilliseconds = LOG_SHOW_MILLISECONDS;
    logConfig.bufferSize = LOG_BUFFER_SIZE;
    
    // 初始化日志系统
    Logger::getInstance().begin(&Serial, LOG_DEFAULT_LEVEL, logConfig);
    
    // 使用新的日志宏
    LOG_TAG_INFO("System", "=== ESP32-S3-Zero 驱动测试程序 ===");
    LOG_TAG_INFO("System", "固件版本: 1.0.0");
    LOG_TAG_INFO("System", "编译时间: " __DATE__ " " __TIME__);
    LOG_TAG_INFO("System", "测试环境模式已启用");
    
    // 根据测试模式执行相应的测试
    switch (currentTestMode) {
        case GPIO_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: GPIO驱动测试");
            runGPIOTests();
            break;
            
        case TIMER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: 定时器驱动测试");
            runTimerTests();
            break;
            
        case COMBINED_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: 综合驱动测试");
            runCombinedTests();
            break;
            
        case WS2812_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: WS2812驱动测试");
            runWS2812Tests();
            break;
            
        case NVS_STORAGE_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: NVS存储驱动测试");
            runNVSStorageTests();
            break;
            
        case LED_CONTROLLER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: LED控制器测试");
            runLEDControllerTests();
            break;
            
        case CONFIG_MANAGER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: ConfigManager测试");
            runConfigManagerTests();
            break;
            
        case MOTOR_CONTROLLER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: MotorController测试");
            runMotorControllerTests();
            break;
            
        case BLE_SERVER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: BLE服务器测试");
            runBLEServerTests();
            break;
            
        case EVENT_MANAGER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: EventManager测试");
            runEventManagerTests();
            break;
            
        case STATE_MANAGER_TEST_MODE:
            LOG_TAG_INFO("System", "运行模式: StateManager测试");
            runStateManagerTests();
            break;
            
        default:
            LOG_TAG_ERROR("System", "未知的测试模式");
            break;
    }
}

void loop() {
    // 根据测试模式执行循环测试
    switch (currentTestMode) {
        case GPIO_TEST_MODE:
            // 运行GPIO循环测试
            gpioTest.runLoopTest();
            delay(10);
            break;
            
        case TIMER_TEST_MODE:
            // 定时器测试通常在setup中完成，这里只做简单的状态监控
            delay(1000);
            LOG_TAG_DEBUG("System", "定时器测试运行中...");
            break;
            
        case COMBINED_TEST_MODE:
            // 运行综合测试
            gpioTest.runLoopTest();
            delay(100);
            break;
            
        case WS2812_TEST_MODE:
            // 运行WS2812循环测试
            ws2812Test.runLoopTest();
            delay(10);
            break;
            
        case NVS_STORAGE_TEST_MODE:
            // NVS存储测试通常在setup中完成，这里只做简单的状态监控
            delay(5000);
            LOG_TAG_DEBUG("System", "NVS存储测试运行中...");
            break;
            
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
                            LOG_TAG_INFO("System", "切换到系统初始化状态");
                            ledController.setState(LEDState::SYSTEM_INIT);
                            break;
                        case 1:
                            LOG_TAG_INFO("System", "切换到电机运行状态");
                            ledController.setState(LEDState::MOTOR_RUNNING);
                            break;
                        case 2:
                            LOG_TAG_INFO("System", "切换到电机停止状态");
                            ledController.setState(LEDState::MOTOR_STOPPED);
                            break;
                        case 3:
                            LOG_TAG_INFO("System", "切换到BLE连接状态");
                            ledController.setState(LEDState::BLE_CONNECTED);
                            break;
                        case 4:
                            LOG_TAG_INFO("System", "切换到BLE断开状态");
                            ledController.setState(LEDState::BLE_DISCONNECTED);
                            break;
                        case 5:
                            LOG_TAG_INFO("System", "切换到错误状态");
                            ledController.setState(LEDState::ERROR_STATE);
                            break;
                    }
                    
                    currentTestState = (currentTestState + 1) % 6;
                }
                
                ledController.update();
                delay(100);
            }
            break;
            
        case CONFIG_MANAGER_TEST_MODE:
            // ConfigManager测试通常在setup中完成，这里只做简单的状态监控
            delay(5000);
            LOG_TAG_DEBUG("System", "ConfigManager测试运行中...");
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
                    
                    LOG_TAG_INFO("System", "电机状态: %d, 剩余运行: %us, 剩余停止: %us, 循环次数: %lu",
                                 static_cast<int>(state), runTime, stopTime, cycles);
                }
                
                // 每10秒启动/停止一次电机
                if (millis() % 10000 == 0 && !motorStarted) {
                    LOG_TAG_INFO("System", "启动电机...");
                    motorController.startMotor();
                    motorStarted = true;
                } else if (millis() % 10000 == 5000 && motorStarted) {
                    LOG_TAG_INFO("System", "停止电机...");
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
                LOG_TAG_INFO("System", "BLE状态: %s", connected ? "已连接" : "未连接");
                
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
                
        case EVENT_MANAGER_TEST_MODE:
            // EventManager测试通常在setup中完成，这里只做简单的状态监控
            delay(5000);
            LOG_TAG_DEBUG("System", "EventManager测试运行中...");
            break;
            
        case STATE_MANAGER_TEST_MODE:
            // StateManager集成测试 - 简化版本避免内存问题
            {
                static unsigned long lastStateChange = 0;
                static int currentTestState = 0;
                static bool testCompleted = false;
                static bool listenerRegistered = false;
                
                StateManager& stateManager = StateManager::getInstance();
                
                // 只注册一次监听器
                if (!listenerRegistered) {
                    auto simpleListener = [](const StateChangeEvent& event) {
                        Serial.printf("Simple listener: %s -> %s\n",
                                    StateManager::getStateName(event.oldState).c_str(),
                                    StateManager::getStateName(event.newState).c_str());
                    };
                    stateManager.registerStateListener(simpleListener);
                    listenerRegistered = true;
                }
                
                // 每10秒切换一次状态进行集成测试（减少频率）
                if (!testCompleted && millis() - lastStateChange > 10000) {
                    lastStateChange = millis();
                    
                    switch (currentTestState) {
                        case 0:
                            LOG_TAG_INFO("System", "=== StateManager集成测试 ===");
                            LOG_TAG_INFO("System", "测试: INIT -> IDLE");
                            stateManager.setState(SystemState::IDLE, "Test1");
                            break;
                            
                        case 1:
                            LOG_TAG_INFO("System", "测试: IDLE -> RUNNING");
                            stateManager.setState(SystemState::RUNNING, "Test2");
                            break;
                            
                        case 2:
                            LOG_TAG_INFO("System", "测试: RUNNING -> IDLE");
                            stateManager.setState(SystemState::IDLE, "Test3");
                            break;
                            
                        case 3:
                            LOG_TAG_INFO("System", "=== 测试完成 ===");
                            testCompleted = true;
                            break;
                    }
                    
                    currentTestState++;
                }
                
                // 测试完成后每30秒显示一次当前状态
                if (testCompleted) {
                    static unsigned long lastStatusUpdate = 0;
                    if (millis() - lastStatusUpdate > 30000) {
                        lastStatusUpdate = millis();
                        LOG_TAG_INFO("System", "当前状态: %s",
                                   StateManager::getStateName(stateManager.getCurrentState()).c_str());
                    }
                }
                
                delay(2000);  // 增加延迟减少CPU负载
            }
            break;
                
        default:
            delay(1000);
            break;
    }
}

/**
 * 运行GPIO测试
 */
void runGPIOTests() {
    LOG_TAG_INFO("System", "开始GPIO驱动测试");
    
    bool testResult = gpioTest.initializeTest();
    if (!testResult) {
        LOG_TAG_ERROR("System", "GPIO初始化测试失败");
        return;
    }
    
    LOG_TAG_INFO("System", "GPIO驱动测试初始化完成");
}

/**
 * 运行定时器测试
 */
void runTimerTests() {
    LOG_TAG_INFO("System", "开始定时器驱动测试");
    
    // 运行所有定时器测试
    bool testResult = timerTest.runAllTests();
    
    if (testResult) {
        LOG_TAG_INFO("System", "定时器驱动测试全部通过！");
    } else {
        LOG_TAG_ERROR("System", "定时器驱动测试存在失败项");
    }
    
    LOG_TAG_INFO("System", "定时器驱动测试完成");
}

/**
 * 运行综合测试
 */
void runCombinedTests() {
    LOG_TAG_INFO("System", "开始综合驱动测试");
    
    // 先运行GPIO测试
    bool gpioResult = gpioTest.initializeTest();
    if (!gpioResult) {
        LOG_TAG_ERROR("System", "GPIO测试失败");
        return;
    }
    
    // 再运行定时器测试
    bool timerResult = timerTest.runAllTests();
    
    // 运行WS2812测试
    bool ws2812Result = ws2812Test.initializeTest();
    if (!ws2812Result) {
        LOG_TAG_ERROR("System", "WS2812测试失败");
        return;
    }
    
    // 运行NVS存储测试
    bool nvsResult = NVSStorageTest::runAllTests();
    
    // 最后运行LED控制器测试
    bool ledResult = ledController.init();
    if (ledResult) {
        LEDControllerTest::runAllTests();
    }
    
    // 输出综合测试结果
    if (gpioResult && timerResult && ws2812Result && nvsResult && ledResult) {
        LOG_TAG_INFO("System", "综合驱动测试全部通过！");
    } else {
        LOG_TAG_ERROR("System", "综合驱动测试存在失败项");
        LOG_TAG_INFO("System", "GPIO测试: %s", gpioResult ? "通过" : "失败");
        LOG_TAG_INFO("System", "定时器测试: %s", timerResult ? "通过" : "失败");
        LOG_TAG_INFO("System", "WS2812测试: %s", ws2812Result ? "通过" : "失败");
        LOG_TAG_INFO("System", "NVS存储测试: %s", nvsResult ? "通过" : "失败");
        LOG_TAG_INFO("System", "LED控制器测试: %s", ledResult ? "通过" : "失败");
    }
    
    LOG_TAG_INFO("System", "综合驱动测试完成");
}

/**
 * 运行WS2812测试
 */
void runWS2812Tests() {
    LOG_TAG_INFO("System", "开始WS2812驱动测试");
    
    bool testResult = ws2812Test.initializeTest();
    if (!testResult) {
        LOG_TAG_ERROR("System", "WS2812初始化测试失败");
        return;
    }
    
    LOG_TAG_INFO("System", "WS2812驱动测试初始化完成");
}

/**
 * 运行NVS存储测试
 */
void runNVSStorageTests() {
    LOG_TAG_INFO("System", "开始NVS存储驱动测试");
    
    // 运行所有NVS存储测试
    bool testResult = NVSStorageTest::runAllTests();
    
    if (testResult) {
        LOG_TAG_INFO("System", "NVS存储驱动测试全部通过！");
    } else {
        LOG_TAG_ERROR("System", "NVS存储驱动测试存在失败项");
    }
    
    LOG_TAG_INFO("System", "NVS存储驱动测试完成");
}

/**
 * 运行LED控制器测试
 */
void runLEDControllerTests() {
    LOG_TAG_INFO("System", "开始LED控制器测试");
    
    // 初始化定时器驱动（LED控制器依赖定时器）
    TimerDriver::getInstance().init();
    
    // 初始化LED控制器
    bool initResult = ledController.init();
    if (!initResult) {
        LOG_TAG_ERROR("System", "LED控制器初始化失败");
        return;
    }
    
    LOG_TAG_INFO("System", "LED控制器初始化完成");
    
    // 运行所有LED控制器测试
    LEDControllerTest::runAllTests();
    
    // 开始循环测试不同状态
    LOG_TAG_INFO("System", "开始LED状态循环测试...");
    LOG_TAG_INFO("System", "将在loop()中每5秒切换一次LED状态");
    
    LOG_TAG_INFO("System", "LED控制器测试完成");
}

/**
 * 运行ConfigManager测试
 */
void runConfigManagerTests() {
    LOG_TAG_INFO("System", "开始ConfigManager测试");
    
    // 获取配置管理器实例
    ConfigManager& configManager = ConfigManager::getInstance();
    
    LOG_TAG_INFO("System", "初始化配置管理器...");
    if (configManager.init()) {
        LOG_TAG_INFO("System", "✅ 配置管理器初始化成功");
    } else {
        LOG_TAG_ERROR("System", "❌ 配置管理器初始化失败");
        LOG_TAG_ERROR("System", "错误: %s", configManager.getLastError());
        return;
    }
    
    LOG_TAG_INFO("System", "运行ConfigManager单元测试...");
    ConfigManagerTest::runAllTests();
    
    LOG_TAG_INFO("System", "ConfigManager测试完成！");
}

/**
 * 运行MotorController测试
 */
void runMotorControllerTests() {
    LOG_TAG_INFO("System", "开始MotorController测试");
    
    // 获取电机控制器实例
    MotorController& motorController = MotorController::getInstance();
    
    LOG_TAG_INFO("System", "初始化电机控制器...");
    if (motorController.init()) {
        LOG_TAG_INFO("System", "✅ 电机控制器初始化成功");
    } else {
        LOG_TAG_ERROR("System", "❌ 电机控制器初始化失败");
        LOG_TAG_ERROR("System", "错误: %s", motorController.getLastError());
        return;
    }
    
    LOG_TAG_INFO("System", "运行MotorController单元测试...");
    MotorControllerTest::runAllTests();
    
    LOG_TAG_INFO("System", "MotorController测试完成！");
    LOG_TAG_INFO("System", "将在loop()中每10秒启动/停止一次电机进行循环测试");
}

/**
 * 运行BLE服务器测试
 */
void runBLEServerTests() {
    LOG_TAG_INFO("System", "开始BLE服务器测试");
    
    // 获取BLE服务器实例
    MotorBLEServer& bleServer = MotorBLEServer::getInstance();
    
    LOG_TAG_INFO("System", "初始化BLE服务器...");
    if (bleServer.init()) {
        LOG_TAG_INFO("System", "✅ BLE服务器初始化成功");
        bleServer.start();
        LOG_TAG_INFO("System", "✅ BLE服务已启动");
        LOG_TAG_INFO("System", "设备名称: ESP32-Motor-Control");
        LOG_TAG_INFO("System", "等待BLE客户端连接...");
    } else {
        LOG_TAG_ERROR("System", "❌ BLE服务器初始化失败");
        LOG_TAG_ERROR("System", "错误: %s", bleServer.getLastError());
        return;
    }
    
    LOG_TAG_INFO("System", "运行BLE服务器单元测试...");
    MotorBLEServerTest::runAllTests();
    
    LOG_TAG_INFO("System", "BLE服务器测试完成！");
    LOG_TAG_INFO("System", "将在loop()中持续更新BLE状态");
}

/**
 * 运行EventManager测试
 */
void runEventManagerTests() {
    LOG_TAG_INFO("System", "开始EventManager测试");
    
    // 运行所有EventManager单元测试
    EventManagerTest::runAllTests();
    
    LOG_TAG_INFO("System", "EventManager测试完成！");
}

/**
 * 运行StateManager测试
 */
void runStateManagerTests() {
    LOG_TAG_INFO("System", "开始StateManager集成测试");
    
    // 获取StateManager实例
    StateManager& stateManager = StateManager::getInstance();
    
    LOG_TAG_INFO("System", "初始化StateManager...");
    stateManager.init();
    LOG_TAG_INFO("System", "✅ StateManager初始化完成");
    LOG_TAG_INFO("System", "初始状态: %s", StateManager::getStateName(stateManager.getCurrentState()).c_str());
    
    // 运行基础单元测试
    LOG_TAG_INFO("System", "运行StateManager单元测试...");
    StateManagerTest::runAllTests();
    
    LOG_TAG_INFO("System", "StateManager集成测试初始化完成！");
    LOG_TAG_INFO("System", "将在loop()中进行状态转换测试");
}