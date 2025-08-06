#include "StateManagerTest.h"
#include "../common/StateManager.h"
#include <Arduino.h>

void StateManagerTest::runAllTests() {
    Serial.println("=== StateManager Tests ===");
    
    testInitialState();
    testValidStateTransitions();
    testInvalidStateTransitions();
    testStateListeners();
    testStateHistory();
    testStateNames();
    
    Serial.println("=== StateManager Tests Complete ===");
}

void StateManagerTest::testInitialState() {
    Serial.println("Testing initial state...");
    
    StateManager& manager = StateManager::getInstance();
    manager.init();
    
    SM_TEST_ASSERT_EQUAL(SystemState::INIT, manager.getCurrentState());
    
    Serial.println("✓ Initial state test passed");
}

void StateManagerTest::testValidStateTransitions() {
    Serial.println("Testing valid state transitions...");
    
    StateManager& manager = StateManager::getInstance();
    manager.init();
    
    // INIT -> IDLE
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::IDLE, "Initialization complete"));
    SM_TEST_ASSERT_EQUAL(SystemState::IDLE, manager.getCurrentState());
    
    // IDLE -> RUNNING
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::RUNNING, "Start motor"));
    SM_TEST_ASSERT_EQUAL(SystemState::RUNNING, manager.getCurrentState());
    
    // RUNNING -> PAUSED
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::PAUSED, "User pause"));
    SM_TEST_ASSERT_EQUAL(SystemState::PAUSED, manager.getCurrentState());
    
    // PAUSED -> RUNNING
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::RUNNING, "Resume operation"));
    SM_TEST_ASSERT_EQUAL(SystemState::RUNNING, manager.getCurrentState());
    
    // RUNNING -> IDLE
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::IDLE, "Stop motor"));
    SM_TEST_ASSERT_EQUAL(SystemState::IDLE, manager.getCurrentState());
    
    Serial.println("✓ Valid state transitions test passed");
}

void StateManagerTest::testInvalidStateTransitions() {
    Serial.println("Testing invalid state transitions...");
    
    StateManager& manager = StateManager::getInstance();
    manager.init();
    
    // INIT -> RUNNING (invalid)
    SM_TEST_ASSERT_FALSE(manager.setState(SystemState::RUNNING, "Invalid transition"));
    SM_TEST_ASSERT_EQUAL(SystemState::INIT, manager.getCurrentState());
    
    // INIT -> PAUSED (invalid)
    SM_TEST_ASSERT_FALSE(manager.setState(SystemState::PAUSED, "Invalid transition"));
    SM_TEST_ASSERT_EQUAL(SystemState::INIT, manager.getCurrentState());
    
    // Set to IDLE first
    manager.setState(SystemState::IDLE, "Valid transition");
    
    // IDLE -> ERROR (valid)
    SM_TEST_ASSERT_TRUE(manager.setState(SystemState::ERROR, "Error occurred"));
    
    // ERROR -> RUNNING (invalid)
    SM_TEST_ASSERT_FALSE(manager.setState(SystemState::RUNNING, "Invalid transition from ERROR"));
    SM_TEST_ASSERT_EQUAL(SystemState::ERROR, manager.getCurrentState());
    
    Serial.println("✓ Invalid state transitions test passed");
}

void StateManagerTest::testStateListeners() {
    Serial.println("Testing state listeners...");
    
    StateManager& manager = StateManager::getInstance();
    manager.init();
    
    bool listenerCalled = false;
    SystemState capturedOldState;
    SystemState capturedNewState;
    String capturedReason;
    
    auto listener = [&listenerCalled, &capturedOldState, &capturedNewState, &capturedReason](const StateChangeEvent& event) {
        listenerCalled = true;
        capturedOldState = event.oldState;
        capturedNewState = event.newState;
        capturedReason = event.reason;
    };
    
    manager.registerStateListener(listener);
    
    // 触发状态变更
    manager.setState(SystemState::IDLE, "Test listener");
    
    // 验证监听器被调用
    SM_TEST_ASSERT_TRUE(listenerCalled);
    SM_TEST_ASSERT_EQUAL(SystemState::INIT, capturedOldState);
    SM_TEST_ASSERT_EQUAL(SystemState::IDLE, capturedNewState);
    SM_TEST_ASSERT_EQUAL_STRING("Test listener", capturedReason.c_str());
    
    // 注销监听器
    manager.unregisterStateListener(listener);
    
    // 重置标志
    listenerCalled = false;
    
    // 再次触发状态变更，验证监听器不再被调用
    manager.setState(SystemState::RUNNING, "Should not trigger listener");
    SM_TEST_ASSERT_FALSE(listenerCalled);
    
    Serial.println("✓ State listeners test passed");
}

void StateManagerTest::testStateHistory() {
    Serial.println("Testing state history...");
    
    StateManager& manager = StateManager::getInstance();
    manager.init();
    
    // 进行一系列状态变更
    manager.setState(SystemState::IDLE, "First transition");
    manager.setState(SystemState::RUNNING, "Second transition");
    manager.setState(SystemState::PAUSED, "Third transition");
    
    // 获取历史记录
    auto history = manager.getStateHistory(5);
    
    // 验证历史记录数量
    SM_TEST_ASSERT_EQUAL(4, history.size()); // 包括初始状态
    
    // 验证历史记录内容
    SM_TEST_ASSERT_EQUAL(SystemState::INIT, history[0].newState);
    SM_TEST_ASSERT_EQUAL(SystemState::IDLE, history[1].newState);
    SM_TEST_ASSERT_EQUAL(SystemState::RUNNING, history[2].newState);
    SM_TEST_ASSERT_EQUAL(SystemState::PAUSED, history[3].newState);
    
    // 测试历史记录限制
    auto limitedHistory = manager.getStateHistory(2);
    SM_TEST_ASSERT_EQUAL(2, limitedHistory.size());
    
    Serial.println("✓ State history test passed");
}

void StateManagerTest::testStateNames() {
    Serial.println("Testing state names...");
    
    SM_TEST_ASSERT_EQUAL_STRING("INIT", StateManager::getStateName(SystemState::INIT).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("IDLE", StateManager::getStateName(SystemState::IDLE).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("RUNNING", StateManager::getStateName(SystemState::RUNNING).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("PAUSED", StateManager::getStateName(SystemState::PAUSED).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("ERROR", StateManager::getStateName(SystemState::ERROR).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("SHUTDOWN", StateManager::getStateName(SystemState::SHUTDOWN).c_str());
    SM_TEST_ASSERT_EQUAL_STRING("UNKNOWN", StateManager::getStateName(static_cast<SystemState>(99)).c_str());
    
    Serial.println("✓ State names test passed");
}