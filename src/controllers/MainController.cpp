#include "MainController.h"
#include "common/Logger.h"
#include "../common/EventManager.h"
#include <Arduino.h>

// 单例实例
MainController& MainController::getInstance() {
    static MainController instance;
    return instance;
}

// 构造函数
MainController::MainController() 
    : running(false)
    , initialized(false)
    , motorControllerInitialized(false)
    , ledControllerInitialized(false)
    , configManagerInitialized(false)
    , bleServerInitialized(false) {
    
    Logger::getInstance().info("MainController", "创建主控制器实例");
}

// 析构函数
MainController::~MainController() {
    Logger::getInstance().info("MainController", "销毁主控制器实例");
    cleanup();
}

// 初始化系统
bool MainController::init() {
    if (initialized) {
        Logger::getInstance().warn("MainController", "系统已经初始化，跳过重复初始化");
        return true;
    }
    
    Logger::getInstance().info("MainController", "开始系统初始化...");
    
    // 初始化日志系统配置
    LoggerConfig logConfig;
    logConfig.showTimestamp = LOG_SHOW_TIMESTAMP;
    logConfig.showLevel = LOG_SHOW_LEVEL;
    logConfig.showTag = LOG_SHOW_TAG;
    logConfig.useColors = LOG_ENABLE_COLORS;
    logConfig.useMilliseconds = LOG_SHOW_MILLISECONDS;
    logConfig.bufferSize = LOG_BUFFER_SIZE;
    
    Logger::getInstance().begin(&Serial, LOG_DEFAULT_LEVEL, logConfig);
    
    Logger::getInstance().info("MainController", "=== ESP32 电机控制系统启动 ===");
    Logger::getInstance().info("MainController", "固件版本: 1.0.0");
    Logger::getInstance().info("MainController", "编译时间: " __DATE__ " " __TIME__);
    Logger::getInstance().info("MainController", "生产环境模式");
    
    // 按照依赖顺序初始化模块
    // 0. 事件管理器 - 最先初始化，其他模块可能使用事件系统
    if (!initializeEventManager()) {
        Logger::getInstance().error("MainController", "事件管理器初始化失败");
        cleanup();
        return false;
    }
    
    // 1. 配置管理器 - 其他模块依赖配置
    if (!initializeConfigManager()) {
        Logger::getInstance().error("MainController", "配置管理器初始化失败");
        cleanup();
        return false;
    }
    
    // 2. LED控制器 - 提供系统状态指示
    if (!initializeLEDController()) {
        Logger::getInstance().error("MainController", "LED控制器初始化失败");
        cleanup();
        return false;
    }
    
    // 3. 电机控制器 - 核心功能模块
    if (!initializeMotorController()) {
        Logger::getInstance().error("MainController", "电机控制器初始化失败");
        cleanup();
        return false;
    }
    
    // 4. BLE服务器 - 通信模块
    if (!initializeBLEServer()) {
        Logger::getInstance().error("MainController", "BLE服务器初始化失败");
        cleanup();
        return false;
    }
    
    // 5. 设置事件监听器
    setupEventListeners();
    
    initialized = true;
    Logger::getInstance().info("MainController", "系统初始化完成");
    
    // 初始化成功后，设置运行状态指示
    if (ledControllerInitialized) {
        ledController.setState(LEDState::BLE_DISCONNECTED);
    }
    
    return true;
}

// 运行系统主循环
void MainController::run() {
    if (!initialized) {
        Logger::getInstance().error("MainController", "系统未初始化，无法运行");
        return;
    }
    
    running = true;
    Logger::getInstance().info("MainController", "系统开始运行");
    
    // 发布系统启动事件
    EventManager::getInstance().publish(EventData(EventType::SYSTEM_STARTUP, "MainController", "系统启动"));
    
    while (running) {
        // 处理事件队列
        EventManager::getInstance().processEvents();
        
        // 更新BLE通信
        if (bleServerInitialized) {
            MotorBLEServer::getInstance().update();
        }
        
        // 更新电机状态
        if (motorControllerInitialized) {
            MotorController::getInstance().update();
        }
        
        // 更新LED状态
        if (ledControllerInitialized) {
            ledController.update();
        }
        
        // 简单的延时，避免CPU占用过高
        delay(10);
    }
    
    // 发布系统关闭事件
    EventManager::getInstance().publish(EventData(EventType::SYSTEM_SHUTDOWN, "MainController", "系统关闭"));
    
    Logger::getInstance().info("MainController", "系统主循环结束");
}

// 停止系统
void MainController::stop() {
    Logger::getInstance().info("MainController", "收到停止信号");
    running = false;
}

// 初始化配置管理器
bool MainController::initializeConfigManager() {
    Logger::getInstance().info("MainController", "正在初始化配置管理器...");
    
    try {
        ConfigManager& config = ConfigManager::getInstance();
        
        if (!config.init()) {
            Logger::getInstance().error("MainController", "配置管理器init()失败");
            return false;
        }
        
        if (!config.loadConfig()) {
            Logger::getInstance().error("MainController", "配置管理器loadConfig()失败");
            return false;
        }
        
        configManagerInitialized = true;
        Logger::getInstance().info("MainController", "配置管理器初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("MainController", "配置管理器初始化发生异常");
        return false;
    }
}

// 初始化LED控制器
bool MainController::initializeLEDController() {
    Logger::getInstance().info("MainController", "正在初始化LED控制器...");
    
    try {
        if (!ledController.init()) {
            Logger::getInstance().error("MainController", "LED控制器init()失败");
            return false;
        }
        
        ledControllerInitialized = true;
        Logger::getInstance().info("MainController", "LED控制器初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("MainController", "LED控制器初始化发生异常");
        return false;
    }
}

// 初始化电机控制器
bool MainController::initializeMotorController() {
    Logger::getInstance().info("MainController", "正在初始化电机控制器...");
    
    try {
        MotorController& motor = MotorController::getInstance();
        
        if (!motor.init()) {
            Logger::getInstance().error("MainController", "电机控制器init()失败");
            return false;
        }
        
        motorControllerInitialized = true;
        Logger::getInstance().info("MainController", "电机控制器初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("MainController", "电机控制器初始化发生异常");
        return false;
    }
}

// 初始化BLE服务器
bool MainController::initializeBLEServer() {
    Logger::getInstance().info("MainController", "正在初始化BLE服务器...");
    
    try {
        MotorBLEServer& ble = MotorBLEServer::getInstance();
        
        if (!ble.init()) {
            Logger::getInstance().error("MainController", "BLE服务器init()失败");
            return false;
        }
        
        ble.start();
        bleServerInitialized = true;
        Logger::getInstance().info("MainController", "BLE服务器初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("MainController", "BLE服务器初始化发生异常");
        return false;
    }
}

// 清理资源
void MainController::cleanup() {
    Logger::getInstance().info("MainController", "开始清理资源...");
    
    // 按照初始化逆序清理
    if (bleServerInitialized) {
        Logger::getInstance().info("MainController", "停止BLE服务器...");
        MotorBLEServer::getInstance().stop();
        bleServerInitialized = false;
    }
    
    if (motorControllerInitialized) {
        Logger::getInstance().info("MainController", "停止电机控制器...");
        // 电机控制器没有stop方法，直接标记为未初始化
        motorControllerInitialized = false;
    }
    
    if (ledControllerInitialized) {
        Logger::getInstance().info("MainController", "停止LED控制器...");
        ledController.setState(LEDState::ERROR_STATE);
        ledController.stop();
        ledControllerInitialized = false;
    }
    
    if (configManagerInitialized) {
        Logger::getInstance().info("MainController", "停止配置管理器...");
        // 配置管理器没有stop方法，直接标记为未初始化
        configManagerInitialized = false;
    }
    
    initialized = false;
    Logger::getInstance().info("MainController", "资源清理完成");
}

// 初始化事件管理器
bool MainController::initializeEventManager() {
    Logger::getInstance().info("MainController", "正在初始化事件管理器...");
    
    try {
        EventManager& eventManager = EventManager::getInstance();
        
        if (!eventManager.initialize()) {
            Logger::getInstance().error("MainController", "事件管理器初始化失败");
            return false;
        }
        
        Logger::getInstance().info("MainController", "事件管理器初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("MainController", "事件管理器初始化发生异常");
        return false;
    }
}

// 设置事件监听器
void MainController::setupEventListeners() {
    Logger::getInstance().info("MainController", "设置事件监听器...");
    
    EventManager& eventManager = EventManager::getInstance();
    
    // 订阅系统事件
    eventManager.subscribe(EventType::SYSTEM_STARTUP, [this](const EventData& event) {
        handleSystemEvent(event);
    });
    
    eventManager.subscribe(EventType::SYSTEM_SHUTDOWN, [this](const EventData& event) {
        handleSystemEvent(event);
    });
    
    // 订阅电机事件
    eventManager.subscribe(EventType::MOTOR_START, [this](const EventData& event) {
        handleMotorEvent(event);
    });
    
    eventManager.subscribe(EventType::MOTOR_STOP, [this](const EventData& event) {
        handleMotorEvent(event);
    });
    
    eventManager.subscribe(EventType::MOTOR_SPEED_CHANGED, [this](const EventData& event) {
        handleMotorEvent(event);
    });
    
    // 订阅BLE事件
    eventManager.subscribe(EventType::BLE_CONNECTED, [this](const EventData& event) {
        handleBLEEvent(event);
    });
    
    eventManager.subscribe(EventType::BLE_DISCONNECTED, [this](const EventData& event) {
        handleBLEEvent(event);
    });
    
    // 订阅配置事件
    eventManager.subscribe(EventType::CONFIG_CHANGED, [this](const EventData& event) {
        handleConfigEvent(event);
    });
    
    Logger::getInstance().info("MainController", "事件监听器设置完成");
}

// 处理系统事件
void MainController::handleSystemEvent(const EventData& event) {
    String logMsg = "系统事件: " + EventManager::getEventTypeName(event.type);
    if (!event.message.isEmpty()) {
        logMsg += " - " + event.message;
    }
    Logger::getInstance().info("MainController", logMsg);
    
    switch (event.type) {
        case EventType::SYSTEM_STARTUP:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::BLE_DISCONNECTED);
            }
            break;
            
        case EventType::SYSTEM_SHUTDOWN:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::ERROR_STATE);
            }
            break;
            
        default:
            break;
    }
}

// 处理电机事件
void MainController::handleMotorEvent(const EventData& event) {
    String logMsg = "电机事件: " + EventManager::getEventTypeName(event.type);
    if (!event.message.isEmpty()) {
        logMsg += " - " + event.message;
    }
    if (event.value != 0) {
        logMsg += " (值: " + String(event.value) + ")";
    }
    Logger::getInstance().info("MainController", logMsg);
    
    switch (event.type) {
        case EventType::MOTOR_START:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::MOTOR_RUNNING);
            }
            break;
            
        case EventType::MOTOR_STOP:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::BLE_CONNECTED);
            }
            break;
            
        case EventType::MOTOR_SPEED_CHANGED:
            // 可以在这里添加速度变化的视觉反馈
            break;
            
        default:
            break;
    }
}

// 处理BLE事件
void MainController::handleBLEEvent(const EventData& event) {
    String logMsg = "BLE事件: " + EventManager::getEventTypeName(event.type);
    if (!event.message.isEmpty()) {
        logMsg += " - " + event.message;
    }
    Logger::getInstance().info("MainController", logMsg);
    
    switch (event.type) {
        case EventType::BLE_CONNECTED:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::BLE_CONNECTED);
            }
            break;
            
        case EventType::BLE_DISCONNECTED:
            if (ledControllerInitialized) {
                ledController.setState(LEDState::BLE_DISCONNECTED);
            }
            break;
            
        default:
            break;
    }
}

// 处理配置事件
void MainController::handleConfigEvent(const EventData& event) {
    String logMsg = "配置事件: " + EventManager::getEventTypeName(event.type);
    if (!event.message.isEmpty()) {
        logMsg += " - " + event.message;
    }
    Logger::getInstance().info("MainController", logMsg);
    
    // 配置改变时，可以重新加载相关模块
    if (event.type == EventType::CONFIG_CHANGED) {
        Logger::getInstance().info("MainController", "配置已更新，重新应用设置...");
        // 这里可以触发相关模块重新加载配置
    }
}