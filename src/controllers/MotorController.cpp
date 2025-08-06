#include "MotorController.h"
#include "../drivers/GPIODriver.h"

// 单例实例
MotorController& MotorController::getInstance() {
    static MotorController instance;
    return instance;
}

// 构造函数
MotorController::MotorController() 
    : currentState(MotorControllerState::STOPPED)
    , timer(TimerDriver::getInstance())
    , gpioDriver(nullptr)
    , stateStartTime(0)
    , remainingRunTime(0)
    , remainingStopTime(0)
    , cycleCount(0)
    , isInitialized(false)
    , configUpdated(false) {
    
    memset(lastError, 0, sizeof(lastError));
    
    // 设置默认配置
    currentConfig.runDuration = 5000;    // 默认5秒
    currentConfig.stopDuration = 2000;   // 默认2秒
    currentConfig.cycleCount = 0;        // 默认无限循环
    currentConfig.autoStart = true;      // 默认自动启动
}

// 析构函数
MotorController::~MotorController() {
    if (isInitialized && gpioDriver) {
        stopMotor();
        delete gpioDriver;
        gpioDriver = nullptr;
    }
}

// 初始化
bool MotorController::init() {
    LOG_TAG_INFO("MotorController", "初始化电机控制器...");
    
    if (isInitialized) {
        LOG_TAG_WARN("MotorController", "电机控制器已初始化");
        return true;
    }
    
    // 创建GPIO驱动实例
    gpioDriver = new GPIODriver();
    if (!gpioDriver) {
        setLastError("无法创建GPIO驱动");
        return false;
    }
    
    // 初始化GPIO
    if (!gpioDriver->init(MOTOR_PIN, OUTPUT, MOTOR_OFF)) {
        setLastError("GPIO驱动初始化失败");
        delete gpioDriver;
        gpioDriver = nullptr;
        return false;
    }
    
    isInitialized = true;
    setState(MotorControllerState::STOPPED);
    
    LOG_TAG_INFO("MotorController", "电机控制器初始化成功");
    return true;
}

// 启动电机
bool MotorController::startMotor() {
    if (!isInitialized) {
        setLastError("电机控制器未初始化");
        return false;
    }
    
    if (currentState == MotorControllerState::RUNNING || currentState == MotorControllerState::STARTING) {
        LOG_TAG_WARN("MotorController", "电机已在运行中");
        return true;
    }
    
    setState(MotorControllerState::STARTING);
    return true;
}

// 停止电机
bool MotorController::stopMotor() {
    if (!isInitialized) {
        setLastError("电机控制器未初始化");
        return false;
    }
    
    if (currentState == MotorControllerState::STOPPED || currentState == MotorControllerState::STOPPING) {
        LOG_TAG_WARN("MotorController", "电机已停止");
        return true;
    }
    
    setState(MotorControllerState::STOPPING);
    return true;
}

// 更新电机状态
void MotorController::update() {
    if (!isInitialized) {
        return;
    }
    
    // 根据当前状态处理状态机
    switch (currentState) {
        case MotorControllerState::STOPPED:
            handleStoppedState();
            break;
        case MotorControllerState::RUNNING:
            handleRunningState();
            break;
        case MotorControllerState::STOPPING:
            handleStoppingState();
            break;
        case MotorControllerState::STARTING:
            handleStartingState();
            break;
        case MotorControllerState::ERROR_STATE:
            handleErrorState();
            break;
    }
}

// 处理停止状态
void MotorController::handleStoppedState() {
    // 检查是否需要开始新的运行周期
    if (remainingStopTime == 0 && currentConfig.stopDuration > 0) {
        // 如果是第一次启动或者需要等待停止时间
        remainingStopTime = currentConfig.stopDuration / 1000; // 转换为秒
        stateStartTime = millis();
    } else if (currentConfig.stopDuration == 0) {
        // 如果停止时间为0，立即开始运行
        setState(MotorControllerState::STARTING);
    } else if (remainingStopTime > 0) {
        // 倒计时停止时间
        uint32_t elapsed = (millis() - stateStartTime) / 1000;
        if (elapsed >= remainingStopTime) {
            remainingStopTime = 0;
            setState(MotorControllerState::STARTING);
        } else {
            remainingStopTime = (currentConfig.stopDuration / 1000) - elapsed;
        }
    }
}

// 处理运行状态
void MotorController::handleRunningState() {
    if (remainingRunTime == 0) {
        remainingRunTime = currentConfig.runDuration / 1000; // 转换为秒
        stateStartTime = millis();
    }
    
    // 倒计时运行时间
    uint32_t elapsed = (millis() - stateStartTime) / 1000;
    if (elapsed >= remainingRunTime) {
        // 运行时间结束，开始停止
        remainingRunTime = 0;
        cycleCount++;
        LOG_TAG_INFO("MotorController", "运行周期完成，循环次数: %lu", cycleCount);
        setState(MotorControllerState::STOPPING);
    } else {
        remainingRunTime = (currentConfig.runDuration / 1000) - elapsed;
    }
}

// 处理停止中状态
void MotorController::handleStoppingState() {
    // 立即停止电机
    stopMotorInternal();
    setState(MotorControllerState::STOPPED);
}

// 处理启动中状态
void MotorController::handleStartingState() {
    // 启动电机
    startMotorInternal();
    setState(MotorControllerState::RUNNING);
}

// 处理错误状态
void MotorController::handleErrorState() {
    // 确保电机停止
    stopMotorInternal();
    
    // 可以在这里添加错误恢复逻辑
    // 例如：等待一段时间后重试
}

// 内部启动电机
void MotorController::startMotorInternal() {
    if (gpioDriver && gpioDriver->digitalWrite(MOTOR_PIN, MOTOR_ON)) {
        LOG_TAG_INFO("MotorController", "电机已启动");
    } else {
        setLastError("无法启动电机");
        setState(MotorControllerState::ERROR_STATE);
    }
}

// 内部停止电机
void MotorController::stopMotorInternal() {
    if (gpioDriver && gpioDriver->digitalWrite(MOTOR_PIN, MOTOR_OFF)) {
        LOG_TAG_INFO("MotorController", "电机已停止");
    } else {
        setLastError("无法停止电机");
        setState(MotorControllerState::ERROR_STATE);
    }
}

// 更新配置
void MotorController::updateConfig(const MotorConfig& config) {
    LOG_TAG_INFO("MotorController", "更新配置参数");
    
    // 保存新配置
    MotorConfig oldConfig = currentConfig;
    currentConfig = config;
    
    // 如果电机正在运行，根据新配置调整计时器
    if (currentState == MotorControllerState::RUNNING) {
        if (remainingRunTime > currentConfig.runDuration / 1000) {
            remainingRunTime = currentConfig.runDuration / 1000;
        }
    } else if (currentState == MotorControllerState::STOPPED) {
        if (remainingStopTime > currentConfig.stopDuration / 1000) {
            remainingStopTime = currentConfig.stopDuration / 1000;
        }
    }
    
    LOG_TAG_DEBUG("MotorController", "运行时间: %u -> %u 毫秒", 
                  oldConfig.runDuration, config.runDuration);
    LOG_TAG_DEBUG("MotorController", "停止时间: %u -> %u 毫秒", 
                  oldConfig.stopDuration, config.stopDuration);
}

// 设置状态
void MotorController::setState(MotorControllerState newState) {
    if (currentState != newState) {
        LOG_TAG_INFO("MotorController", "状态切换: %d -> %d", 
                     static_cast<int>(currentState), static_cast<int>(newState));
        currentState = newState;
        stateStartTime = millis();
    }
}

// 设置错误信息
void MotorController::setLastError(const char* error) {
    strncpy(lastError, error, sizeof(lastError) - 1);
    lastError[sizeof(lastError) - 1] = '\0';
    LOG_TAG_ERROR("MotorController", "%s", error);
}

// 获取当前电机状态
MotorControllerState MotorController::getCurrentState() const {
    return currentState;
}

// 获取剩余运行时间
uint32_t MotorController::getRemainingRunTime() const {
    return remainingRunTime;
}

// 获取剩余停止时间
uint32_t MotorController::getRemainingStopTime() const {
    return remainingStopTime;
}

// 获取当前循环次数
uint32_t MotorController::getCurrentCycleCount() const {
    return cycleCount;
}

// 获取当前配置
const MotorConfig& MotorController::getCurrentConfig() const {
    return currentConfig;
}

// 重置循环计数器
void MotorController::resetCycleCount() {
    cycleCount = 0;
    LOG_TAG_INFO("MotorController", "循环计数器已重置");
}

// 检查是否处于运行状态
bool MotorController::isRunning() const {
    return currentState == MotorControllerState::RUNNING;
}

// 检查是否处于停止状态
bool MotorController::isStopped() const {
    return currentState == MotorControllerState::STOPPED;
}

// 获取错误信息
const char* MotorController::getLastError() const {
    return lastError;
}