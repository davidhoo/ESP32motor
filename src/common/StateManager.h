#ifndef STATE_MANAGER_H
#define STATE_MANAGER_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <map>

/**
 * @brief 系统状态枚举
 * 定义了整个系统的可能状态
 */
enum class SystemState {
    INIT,           // 系统初始化
    IDLE,           // 空闲状态
    RUNNING,        // 运行中
    PAUSED,         // 暂停状态
    ERROR,          // 错误状态
    SHUTDOWN        // 关机状态
};

/**
 * @brief 状态变更事件结构体
 * 用于通知状态变更的详细信息
 */
struct StateChangeEvent {
    SystemState oldState;
    SystemState newState;
    String reason;
    uint32_t timestamp;
};

/**
 * @brief 状态验证结果
 */
struct StateValidationResult {
    bool isValid;
    String errorMessage;
};

/**
 * @brief 状态管理器类
 * 负责管理系统状态的变更、验证和通知
 */
class StateManager {
public:
    /**
     * @brief 获取状态管理器单例实例
     * @return StateManager& 状态管理器实例
     */
    static StateManager& getInstance();
    
    /**
     * @brief 初始化状态管理器
     * @return bool 初始化是否成功
     */
    bool init();
    
    /**
     * @brief 获取当前系统状态
     * @return SystemState 当前状态
     */
    SystemState getCurrentState() const;
    
    /**
     * @brief 设置系统状态
     * @param newState 新状态
     * @param reason 状态变更原因
     * @return bool 状态变更是否成功
     */
    bool setState(SystemState newState, const String& reason = "");
    
    /**
     * @brief 检查状态是否有效
     * @param fromState 起始状态
     * @param toState 目标状态
     * @return StateValidationResult 验证结果
     */
    StateValidationResult validateStateTransition(SystemState fromState, SystemState toState) const;
    
    /**
     * @brief 注册状态变更监听器
     * @param listener 监听器回调函数
     */
    void registerStateListener(std::function<void(const StateChangeEvent&)> listener);
    
    /**
     * @brief 注销状态变更监听器
     * @param listener 要注销的监听器
     */
    void unregisterStateListener(std::function<void(const StateChangeEvent&)> listener);
    
    /**
     * @brief 获取状态名称字符串
     * @param state 状态枚举
     * @return String 状态名称
     */
    static String getStateName(SystemState state);
    
    /**
     * @brief 获取状态变更历史
     * @param maxEntries 最大返回条目数
     * @return std::vector<StateChangeEvent> 状态变更历史
     */
    std::vector<StateChangeEvent> getStateHistory(size_t maxEntries = 10) const;

private:
    // 私有构造函数（单例模式）
    StateManager() = default;
    
    // 禁止拷贝和赋值
    StateManager(const StateManager&) = delete;
    StateManager& operator=(const StateManager&) = delete;
    
    // 状态变更通知
    void notifyStateChange(const StateChangeEvent& event);
    
    // 状态验证
    bool isValidStateTransition(SystemState fromState, SystemState toState) const;
    
    // 当前状态
    SystemState m_currentState;
    
    // 状态变更历史
    std::vector<StateChangeEvent> m_stateHistory;
    
    // 状态监听器列表
    std::vector<std::function<void(const StateChangeEvent&)>> m_listeners;
    
    // 状态转换规则映射
    static const std::map<SystemState, std::vector<SystemState>> m_validTransitions;
};

#endif // STATE_MANAGER_H