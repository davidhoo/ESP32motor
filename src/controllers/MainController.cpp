#include "MainController.h"
#include "common/Logger.h"
#include "../common/EventManager.h"
#include <Arduino.h>
#include <cstring>

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
    , bleServerInitialized(false)
    , initRetryCount(0)
    , criticalModulesFailed(false) {
    
    memset(lastInitError, 0, sizeof(lastInitError));
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
    
    Logger::getInstance().info("MainController", "开始系统启动流程...");
    
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
    
    // === 5.1 系统启动流程实现 ===
    // 步骤1: LED初始化指示
    Logger::getInstance().info("MainController", "步骤1: 初始化LED指示系统...");
    if (!initializeWithRetry("LED控制器", [this]() { return initializeLEDController(); }, true)) {
        Logger::getInstance().error("MainController", "LED控制器初始化失败，进入安全模式");
        enterSafeMode();
        return false;
    }
    // 设置系统初始化LED指示（蓝色闪烁）
    ledController.setState(LEDState::SYSTEM_INIT);
    delay(500); // 短暂延时让用户看到LED指示
    
    // 步骤2: 初始化事件管理器
    Logger::getInstance().info("MainController", "步骤2: 初始化事件管理器...");
    if (!initializeWithRetry("事件管理器", [this]() { return initializeEventManager(); }, true)) {
        Logger::getInstance().error("MainController", "事件管理器初始化失败，进入安全模式");
        if (ledControllerInitialized) {
            ledController.setState(LEDState::ERROR_STATE);
        }
        enterSafeMode();
        return false;
    }
    
    // 步骤3: NVS参数加载
    Logger::getInstance().info("MainController", "步骤3: 加载NVS配置参数...");
    if (!initializeWithRetry("配置管理器", [this]() { return initializeConfigManager(); }, true)) {
        Logger::getInstance().error("MainController", "配置管理器初始化失败，使用默认配置继续");
        if (ledControllerInitialized) {
            ledController.setState(LEDState::ERROR_STATE);
        }
        // 配置管理器失败时，尝试使用默认配置继续运行
        if (!canContinueWithoutModule("配置管理器")) {
            enterSafeMode();
            return false;
        }
    } else {
        Logger::getInstance().info("MainController", "NVS配置参数加载完成");
    }
    
    // 步骤4: 初始化电机控制器
    Logger::getInstance().info("MainController", "步骤4: 初始化电机控制器...");
    if (!initializeWithRetry("电机控制器", [this]() { return initializeMotorController(); }, true)) {
        Logger::getInstance().error("MainController", "电机控制器初始化失败，进入安全模式");
        if (ledControllerInitialized) {
            ledController.setState(LEDState::ERROR_STATE);
        }
        enterSafeMode();
        return false;
    }
    
    // 步骤5: BLE服务启动
    Logger::getInstance().info("MainController", "步骤5: 启动BLE服务...");
    if (!initializeWithRetry("BLE服务器", [this]() { return initializeBLEServer(); }, false)) {
        Logger::getInstance().warn("MainController", "BLE服务器初始化失败，系统将在无BLE模式下运行");
        if (ledControllerInitialized) {
            ledController.setState(LEDState::BLE_DISCONNECTED);
        }
        // BLE不是关键模块，可以继续运行
    } else {
        Logger::getInstance().info("MainController", "BLE服务启动完成");
    }
    
    // 步骤6: 设置事件监听器
    Logger::getInstance().info("MainController", "步骤6: 设置事件监听器...");
    setupEventListeners();
    
    initialized = true;
    Logger::getInstance().info("MainController", "=== 系统启动流程完成 ===");
    
    // 步骤7: 电机自动启动（如果配置了自动启动）
    if (configManagerInitialized && motorControllerInitialized) {
        const MotorConfig& config = ConfigManager::getInstance().getConfig();
        if (config.autoStart) {
            Logger::getInstance().info("MainController", "步骤7: 电机自动启动...");
            MotorController::getInstance().startMotor();
            Logger::getInstance().info("MainController", "电机自动启动完成");
        } else {
            Logger::getInstance().info("MainController", "电机自动启动已禁用");
        }
    }
    
    // 设置初始LED状态为BLE未连接（黄色闪烁）
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
    
    // 检查是否处于安全模式
    if (criticalModulesFailed) {
        Logger::getInstance().error("MainController", "系统处于安全模式，功能受限");
        // 在安全模式下，只保持基本的LED指示
        while (running) {
            if (ledControllerInitialized) {
                ledController.update();
            }
            delay(100); // 安全模式下降低更新频率
        }
        return;
    }
    
    running = true;
    Logger::getInstance().info("MainController", "系统开始运行");
    
    // 发布系统启动事件
    EventManager::getInstance().publish(EventData(EventType::SYSTEM_STARTUP, "MainController", "系统启动"));
    
    while (running) {
        // 处理事件队列
        EventManager::getInstance().processEvents();
        
        // 更新BLE通信（如果可用）
        if (bleServerInitialized) {
            try {
                MotorBLEServer::getInstance().update();
            } catch (...) {
                Logger::getInstance().error("MainController", "BLE更新异常，停用BLE服务");
                bleServerInitialized = false;
            }
        }
        
        // 更新电机状态（如果可用）
        if (motorControllerInitialized) {
            try {
                MotorController::getInstance().update();
            } catch (...) {
                Logger::getInstance().error("MainController", "电机控制器更新异常");
                // 电机控制器异常是严重问题，进入安全模式
                enterSafeMode();
                break;
            }
        }
        
        // 更新LED状态（如果可用）
        if (ledControllerInitialized) {
            try {
                ledController.update();
            } catch (...) {
                Logger::getInstance().error("MainController", "LED控制器更新异常，停用LED");
                ledControllerInitialized = false;
            }
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
            // 通知BLE客户端电机状态变化
            if (bleServerInitialized) {
                MotorBLEServer::getInstance().sendStatusNotification(
                    MotorBLEServer::getInstance().generateStatusJson()
                );
            }
            break;
            
        case EventType::MOTOR_STOP:
            if (ledControllerInitialized) {
                // 根据BLE连接状态设置LED
                if (bleServerInitialized && MotorBLEServer::getInstance().isConnected()) {
                    ledController.setState(LEDState::BLE_CONNECTED);
                } else {
                    ledController.setState(LEDState::MOTOR_STOPPED);
                }
            }
            // 通知BLE客户端电机状态变化
            if (bleServerInitialized) {
                MotorBLEServer::getInstance().sendStatusNotification(
                    MotorBLEServer::getInstance().generateStatusJson()
                );
            }
            break;
            
        case EventType::MOTOR_SPEED_CHANGED:
            // 通知BLE客户端电机参数变化
            if (bleServerInitialized) {
                MotorBLEServer::getInstance().sendStatusNotification(
                    MotorBLEServer::getInstance().generateStatusJson()
                );
            }
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
                // 如果电机正在运行，优先显示电机状态
                if (motorControllerInitialized && MotorController::getInstance().isRunning()) {
                    ledController.setState(LEDState::MOTOR_RUNNING);
                } else {
                    ledController.setState(LEDState::BLE_CONNECTED);
                }
            }
            // === 5.3.3 实时状态推送机制 - BLE连接时立即推送状态 ===
            if (bleServerInitialized) {
                // BLE连接后立即推送当前状态
                MotorBLEServer::getInstance().sendStatusNotification(
                    MotorBLEServer::getInstance().generateStatusJson()
                );
                Logger::getInstance().info("MainController", "BLE连接后已推送初始状态");
            }
            break;
            
        case EventType::BLE_DISCONNECTED:
            if (ledControllerInitialized) {
                // 如果电机正在运行，优先显示电机状态
                if (motorControllerInitialized && MotorController::getInstance().isRunning()) {
                    ledController.setState(LEDState::MOTOR_RUNNING);
                } else {
                    ledController.setState(LEDState::BLE_DISCONNECTED);
                }
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

// === 5.4.1 模块初始化失败的错误处理机制 ===

/**
 * 带重试机制的模块初始化
 */
bool MainController::initializeWithRetry(const char* moduleName, std::function<bool()> initFunc, bool isCritical) {
    initRetryCount = 0;
    
    while (initRetryCount < MAX_INIT_RETRIES) {
        Logger::getInstance().info("MainController", "尝试初始化%s (第%d次)", moduleName, initRetryCount + 1);
        
        if (initFunc()) {
            Logger::getInstance().info("MainController", "%s初始化成功", moduleName);
            return true;
        }
        
        initRetryCount++;
        
        if (initRetryCount < MAX_INIT_RETRIES) {
            Logger::getInstance().warn("MainController", "%s初始化失败，%d秒后重试 (第%d次)",
                                     moduleName, initRetryCount, initRetryCount);
            delay(initRetryCount * 1000); // 递增延时：1s, 2s, 3s
        }
    }
    
    // 所有重试都失败
    setInitError(String(moduleName + String("初始化失败，已重试") + String(MAX_INIT_RETRIES) + "次").c_str());
    Logger::getInstance().error("MainController", "%s初始化最终失败", moduleName);
    
    if (isCritical) {
        criticalModulesFailed = true;
        Logger::getInstance().error("MainController", "关键模块%s初始化失败，系统无法正常运行", moduleName);
    }
    
    return false;
}

/**
 * 设置初始化错误信息
 */
void MainController::setInitError(const char* error) {
    if (error) {
        strncpy(lastInitError, error, sizeof(lastInitError) - 1);
        lastInitError[sizeof(lastInitError) - 1] = '\0';
    } else {
        memset(lastInitError, 0, sizeof(lastInitError));
    }
}

/**
 * 检查是否可以在没有某个模块的情况下继续运行
 */
bool MainController::canContinueWithoutModule(const char* moduleName) {
    String module(moduleName);
    
    // 配置管理器失败时，可以使用默认配置继续
    if (module == "配置管理器") {
        Logger::getInstance().warn("MainController", "配置管理器不可用，将使用默认配置");
        // 这里可以设置一些默认的配置值
        return true;
    }
    
    // BLE服务器失败时，可以在离线模式下继续
    if (module == "BLE服务器") {
        Logger::getInstance().warn("MainController", "BLE服务器不可用，系统将在离线模式下运行");
        return true;
    }
    
    // LED控制器、电机控制器、事件管理器是关键模块
    return false;
}

/**
 * 进入安全模式
 */
void MainController::enterSafeMode() {
    Logger::getInstance().error("MainController", "系统进入安全模式");
    
    // 设置LED为错误状态（如果可用）
    if (ledControllerInitialized) {
        ledController.setState(LEDState::ERROR_STATE);
    }
    
    // 停止所有非关键服务
    if (bleServerInitialized) {
        Logger::getInstance().info("MainController", "安全模式：停止BLE服务");
        MotorBLEServer::getInstance().stop();
        bleServerInitialized = false;
    }
    
    if (motorControllerInitialized) {
        Logger::getInstance().info("MainController", "安全模式：停止电机控制器");
        MotorController::getInstance().stopMotor();
        motorControllerInitialized = false;
    }
    
    // 标记系统为未初始化状态
    initialized = false;
    criticalModulesFailed = true;
    
    Logger::getInstance().error("MainController", "安全模式激活，系统功能受限");
    Logger::getInstance().error("MainController", "最后错误: %s", lastInitError);
}