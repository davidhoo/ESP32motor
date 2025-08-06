#include "StateManager.h"
#include <algorithm>

// 定义有效的状态转换规则
const std::map<SystemState, std::vector<SystemState>> StateManager::m_validTransitions = {
    {SystemState::INIT,     {SystemState::IDLE, SystemState::ERROR}},
    {SystemState::IDLE,     {SystemState::RUNNING, SystemState::SHUTDOWN, SystemState::ERROR}},
    {SystemState::RUNNING,  {SystemState::PAUSED, SystemState::IDLE, SystemState::SHUTDOWN, SystemState::ERROR}},
    {SystemState::PAUSED,   {SystemState::RUNNING, SystemState::IDLE, SystemState::SHUTDOWN, SystemState::ERROR}},
    {SystemState::ERROR,    {SystemState::INIT, SystemState::SHUTDOWN}},
    {SystemState::SHUTDOWN, {SystemState::INIT}}
};

StateManager& StateManager::getInstance() {
    static StateManager instance;
    return instance;
}

bool StateManager::init() {
    m_currentState = SystemState::INIT;
    
    // 记录初始状态
    StateChangeEvent initialEvent;
    initialEvent.oldState = SystemState::INIT;
    initialEvent.newState = SystemState::INIT;
    initialEvent.reason = "System initialization";
    initialEvent.timestamp = millis();
    
    m_stateHistory.push_back(initialEvent);
    
    // 限制历史记录大小
    if (m_stateHistory.size() > 50) {
        m_stateHistory.erase(m_stateHistory.begin());
    }
    
    return true;
}

SystemState StateManager::getCurrentState() const {
    return m_currentState;
}

bool StateManager::setState(SystemState newState, const String& reason) {
    // 验证状态转换
    auto validation = validateStateTransition(m_currentState, newState);
    if (!validation.isValid) {
        Serial.printf("[StateManager] Invalid state transition: %s -> %s, reason: %s\n",
                     getStateName(m_currentState).c_str(),
                     getStateName(newState).c_str(),
                     validation.errorMessage.c_str());
        return false;
    }
    
    // 创建状态变更事件
    StateChangeEvent event;
    event.oldState = m_currentState;
    event.newState = newState;
    event.reason = reason;
    event.timestamp = millis();
    
    // 更新当前状态
    m_currentState = newState;
    
    // 记录到历史
    m_stateHistory.push_back(event);
    
    // 限制历史记录大小
    if (m_stateHistory.size() > 50) {
        m_stateHistory.erase(m_stateHistory.begin());
    }
    
    // 通知监听器
    notifyStateChange(event);
    
    // 记录状态变更
    Serial.printf("[StateManager] State changed: %s -> %s, reason: %s\n",
                 getStateName(event.oldState).c_str(),
                 getStateName(event.newState).c_str(),
                 reason.c_str());
    
    return true;
}

StateValidationResult StateManager::validateStateTransition(SystemState fromState, SystemState toState) const {
    StateValidationResult result;
    result.isValid = false;
    
    // 检查是否相同状态
    if (fromState == toState) {
        result.isValid = true;
        result.errorMessage = "Same state transition";
        return result;
    }
    
    // 查找有效的状态转换
    auto it = m_validTransitions.find(fromState);
    if (it == m_validTransitions.end()) {
        result.errorMessage = "Invalid from state";
        return result;
    }
    
    const auto& validTargets = it->second;
    auto targetIt = std::find(validTargets.begin(), validTargets.end(), toState);
    
    if (targetIt == validTargets.end()) {
        result.errorMessage = "Transition not allowed from " + getStateName(fromState) + " to " + getStateName(toState);
        return result;
    }
    
    result.isValid = true;
    result.errorMessage = "";
    return result;
}

void StateManager::registerStateListener(std::function<void(const StateChangeEvent&)> listener) {
    m_listeners.push_back(listener);
}

void StateManager::unregisterStateListener(std::function<void(const StateChangeEvent&)> listener) {
    // 注意：由于std::function的限制，这个方法目前只是一个占位符
    // 在实际使用中，建议使用ID或其他方式来标识监听器
    // 这里我们简单地清空所有监听器作为临时解决方案
    Serial.println("[StateManager] Warning: unregisterStateListener is not fully implemented");
}

String StateManager::getStateName(SystemState state) {
    switch (state) {
        case SystemState::INIT:     return "INIT";
        case SystemState::IDLE:     return "IDLE";
        case SystemState::RUNNING:  return "RUNNING";
        case SystemState::PAUSED:   return "PAUSED";
        case SystemState::ERROR:    return "ERROR";
        case SystemState::SHUTDOWN: return "SHUTDOWN";
        default:                    return "UNKNOWN";
    }
}

std::vector<StateChangeEvent> StateManager::getStateHistory(size_t maxEntries) const {
    std::vector<StateChangeEvent> result;
    
    size_t startIndex = 0;
    if (m_stateHistory.size() > maxEntries) {
        startIndex = m_stateHistory.size() - maxEntries;
    }
    
    for (size_t i = startIndex; i < m_stateHistory.size(); ++i) {
        result.push_back(m_stateHistory[i]);
    }
    
    return result;
}

void StateManager::notifyStateChange(const StateChangeEvent& event) {
    for (const auto& listener : m_listeners) {
        try {
            listener(event);
        } catch (const std::exception& e) {
            Serial.printf("[StateManager] Error in state listener: %s\n", e.what());
        }
    }
}