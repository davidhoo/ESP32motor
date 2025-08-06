#include <Arduino.h>
#include "../src/common/StateManager.h"

// 状态变更监听器示例
void onStateChanged(const StateChangeEvent& event) {
    Serial.printf("[Example] State changed from %s to %s, reason: %s\n",
                 StateManager::getStateName(event.oldState).c_str(),
                 StateManager::getStateName(event.newState).c_str(),
                 event.reason.c_str());
    
    // 根据状态执行相应操作
    switch (event.newState) {
        case SystemState::INIT:
            Serial.println("[Example] System initializing...");
            break;
            
        case SystemState::IDLE:
            Serial.println("[Example] System ready and idle");
            break;
            
        case SystemState::RUNNING:
            Serial.println("[Example] System running - start motor operations");
            break;
            
        case SystemState::PAUSED:
            Serial.println("[Example] System paused - motor stopped");
            break;
            
        case SystemState::ERROR:
            Serial.println("[Example] System error - check logs and recover");
            break;
            
        case SystemState::SHUTDOWN:
            Serial.println("[Example] System shutting down...");
            break;
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println("=== StateManager Example ===");
    
    // 获取状态管理器实例
    StateManager& stateManager = StateManager::getInstance();
    
    // 初始化状态管理器
    stateManager.init();
    
    // 注册状态变更监听器
    stateManager.registerStateListener(onStateChanged);
    
    Serial.println("\n--- Testing State Transitions ---");
    
    // 模拟正常操作流程
    stateManager.setState(SystemState::IDLE, "System initialized successfully");
    delay(1000);
    
    stateManager.setState(SystemState::RUNNING, "User started motor");
    delay(1000);
    
    stateManager.setState(SystemState::PAUSED, "User requested pause");
    delay(1000);
    
    stateManager.setState(SystemState::RUNNING, "User resumed operation");
    delay(1000);
    
    stateManager.setState(SystemState::IDLE, "User stopped motor");
    delay(1000);
    
    Serial.println("\n--- Testing Invalid Transitions ---");
    
    // 测试无效的状态转换
    stateManager.setState(SystemState::RUNNING, "This should fail - already in IDLE");
    
    Serial.println("\n--- State History ---");
    
    // 显示状态历史
    auto history = stateManager.getStateHistory(10);
    Serial.println("Recent state changes:");
    for (const auto& event : history) {
        Serial.printf("  %s -> %s (%s)\n",
                     StateManager::getStateName(event.oldState).c_str(),
                     StateManager::getStateName(event.newState).c_str(),
                     event.reason.c_str());
    }
    
    Serial.println("\n--- Example Complete ---");
}

void loop() {
    // 在实际应用中，这里可以处理其他任务
    // 状态管理器会在后台处理状态变更
    
    delay(1000);
}