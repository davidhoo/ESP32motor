#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

#include "../common/Config.h"
#include "../drivers/GPIODriver.h"
#include "../drivers/TimerDriver.h"
#include "../controllers/ConfigManager.h"
#include "../common/Logger.h"

/**
 * @brief 电机状态枚举
 * 定义电机的各种运行状态
 */
enum class MotorControllerState {
    STOPPED,        // 停止状态
    RUNNING,        // 运行状态
    STOPPING,       // 正在停止
    STARTING,       // 正在启动
    ERROR_STATE     // 错误状态
};

/**
 * @brief 电机控制器类
 * 管理电机的运行状态、循环控制和参数管理
 */
class MotorController {
public:
    /**
     * @brief 获取电机控制器单例实例
     * @return MotorController实例引用
     */
    static MotorController& getInstance();
    
    /**
     * @brief 初始化电机控制器
     * @return 初始化是否成功
     */
    bool init();
    
    /**
     * @brief 启动电机
     * @return 启动是否成功
     */
    bool startMotor();
    
    /**
     * @brief 停止电机
     * @return 停止是否成功
     */
    bool stopMotor();
    
    /**
     * @brief 更新电机状态（需要在主循环中调用）
     */
    void update();
    
    /**
     * @brief 获取当前电机状态
     * @return 当前电机状态
     */
    MotorControllerState getCurrentState() const;
    
    /**
     * @brief 获取剩余运行时间（秒）
     * @return 剩余运行时间
     */
    uint32_t getRemainingRunTime() const;
    
    /**
     * @brief 获取剩余停止时间（秒）
     * @return 剩余停止时间
     */
    uint32_t getRemainingStopTime() const;
    
    /**
     * @brief 获取当前循环次数
     * @return 当前循环次数
     */
    uint32_t getCurrentCycleCount() const;
    
    /**
     * @brief 更新配置参数
     * @param config 新配置
     */
    void updateConfig(const MotorConfig& config);
    
    /**
     * @brief 获取当前配置
     * @return 当前配置引用
     */
    const MotorConfig& getCurrentConfig() const;
    
    /**
     * @brief 重置循环计数器
     */
    void resetCycleCount();
    
    /**
     * @brief 检查是否处于运行状态
     * @return 是否正在运行
     */
    bool isRunning() const;
    
    /**
     * @brief 检查是否处于停止状态
     * @return 是否已停止
     */
    bool isStopped() const;
    
    /**
     * @brief 获取错误信息
     * @return 错误信息
     */
    const char* getLastError() const;

private:
    /**
     * @brief 构造函数 - 私有，使用单例模式
     */
    MotorController();
    
    /**
     * @brief 析构函数
     */
    ~MotorController();
    
    /**
     * @brief 复制构造函数 - 禁用
     */
    MotorController(const MotorController&) = delete;
    
    /**
     * @brief 赋值运算符 - 禁用
     */
    MotorController& operator=(const MotorController&) = delete;
    
    // 状态机相关方法
    void handleStoppedState();
    void handleRunningState();
    void handleStoppingState();
    void handleStartingState();
    void handleErrorState();
    
    // 辅助方法
    void startMotorInternal();
    void stopMotorInternal();
    void updateTimers();
    void checkStateTransitions();
    void setState(MotorControllerState newState);
    void setLastError(const char* error);
    
    // 成员变量
    MotorControllerState currentState;        // 当前状态
    MotorConfig currentConfig;      // 当前配置
    GPIODriver* gpioDriver;         // GPIO驱动
    TimerDriver& timer;             // 定时器驱动
    
    // 计时相关
    uint32_t stateStartTime;        // 状态开始时间
    uint32_t remainingRunTime;      // 剩余运行时间
    uint32_t remainingStopTime;     // 剩余停止时间
    uint32_t cycleCount;            // 循环次数
    
    // 状态标志
    bool isInitialized;             // 是否已初始化
    bool configUpdated;             // 配置是否已更新
    char lastError[100];            // 错误信息
};

#endif // MOTOR_CONTROLLER_H