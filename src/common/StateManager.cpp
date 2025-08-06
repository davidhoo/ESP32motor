#include "StateManager.h"
#include <algorithm>

// 定义有效的状态转换规则
const std::map<SystemState, std::vector<SystemState>> StateManager::m_validTransitions = {
    {SystemState::INIT,     {SystemState::IDLE, SystemState::ERROR, SystemState::INIT}},
    {SystemState::IDLE,     {SystemState::RUNNING, SystemState::SHUTDOWN, SystemState::ERROR, SystemState::INIT}},
    {SystemState::RUNNING,  {SystemState::PAUSED, SystemState::IDLE, SystemState::SHUTDOWN, SystemState::ERROR, SystemState::INIT}},
    {SystemState::PAUSED,   {SystemState::RUNNING, SystemState::IDLE, SystemState::SHUTDOWN, SystemState::ERROR, SystemState::INIT}},
    {SystemState::ERROR,    {SystemState::INIT, SystemState::SHUTDOWN}},
    {SystemState::SHUTDOWN, {SystemState::INIT}}
};

StateManager& StateManager::getInstance() {
    static StateManager instance;
    return instance;
}

bool StateManager::init() {
    m_currentState = SystemState::INIT;
    
    // 初始化环形缓冲区
    m_historyHead = 0;
    m_historyCount = 0;
    m_listenerCount = 0;
    
    // 初始化监听器有效性标记
    for (size_t i = 0; i < MAX_LISTENERS; i++) {
        m_listenerValid[i] = false;
    }
    
    // 记录初始状态
    StateChangeEvent initialEvent;
    initialEvent.oldState = SystemState::INIT;
    initialEvent.newState = SystemState::INIT;
    initialEvent.reason = "System initialization";
    initialEvent.timestamp = millis();
    
    addToHistory(initialEvent);
    
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
    addToHistory(event);
    
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
    if (m_listenerCount < MAX_LISTENERS) {
        m_listeners[m_listenerCount] = listener;
        m_listenerValid[m_listenerCount] = true;  // 标记为有效
        m_listenerCount++;
    } else {
        Serial.println("[StateManager] Warning: Maximum number of listeners reached");
    }
}

void StateManager::unregisterStateListener(std::function<void(const StateChangeEvent&)> listener) {
    // 由于std::function无法直接比较，我们清空所有监听器作为安全措施
    // 这是一个简化的实现，在生产环境中应该使用ID机制
    for (size_t i = 0; i < m_listenerCount; i++) {
        m_listenerValid[i] = false;
    }
    m_listenerCount = 0;
    Serial.println("[StateManager] Warning: All listeners have been cleared due to unregister operation");
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
    
    size_t entriesToReturn = (maxEntries < m_historyCount) ? maxEntries : m_historyCount;
    
    for (size_t i = 0; i < entriesToReturn; i++) {
        size_t index = (m_historyHead + m_historyCount - entriesToReturn + i) % MAX_HISTORY_SIZE;
        result.push_back(m_stateHistory[index]);
    }
    
    return result;
}

void StateManager::addToHistory(const StateChangeEvent& event) {
    // 添加到环形缓冲区
    m_stateHistory[m_historyHead] = event;
    m_historyHead = (m_historyHead + 1) % MAX_HISTORY_SIZE;
    
    if (m_historyCount < MAX_HISTORY_SIZE) {
        m_historyCount++;
    }
}

void StateManager::notifyStateChange(const StateChangeEvent& event) {
    for (size_t i = 0; i < m_listenerCount; i++) {
        // 检查监听器是否有效
        if (m_listenerValid[i] && m_listeners[i]) {
            try {
                m_listeners[i](event);
            } catch (const std::exception& e) {
                Serial.printf("[StateManager] Error in state listener %zu: %s\n", i, e.what());
                // 标记出错的监听器为无效
                m_listenerValid[i] = false;
            } catch (...) {
                Serial.printf("[StateManager] Unknown error in state listener %zu\n", i);
                // 标记出错的监听器为无效
                m_listenerValid[i] = false;
            }
        }
    }
}