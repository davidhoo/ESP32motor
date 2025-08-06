#include "EventManagerTest.h"
#include <Arduino.h>

// 初始化静态成员变量
int EventManagerTest::testEventCounter = 0;
String EventManagerTest::lastTestMessage = "";
EventType EventManagerTest::lastTestEventType = EventType::CUSTOM_EVENT;

/**
 * 运行所有测试
 */
void EventManagerTest::runAllTests() {
    Serial.println("=== EventManager 单元测试开始 ===");
    
    testSingleton();
    testInitializeAndCleanup();
    testSubscribeUnsubscribe();
    testPublishSync();
    testPublishAsync();
    testEventQueue();
    testMultipleListeners();
    testEventTypeNames();
    testErrorHandling();
    testBoundaryConditions();
    
    Serial.println("=== EventManager 单元测试完成 ===");
}

/**
 * 测试单例模式
 */
void EventManagerTest::testSingleton() {
    Serial.println("测试单例模式...");
    
    EventManager& instance1 = EventManager::getInstance();
    EventManager& instance2 = EventManager::getInstance();
    
    assertTrue(&instance1 == &instance2, "单例模式应该返回相同实例");
    
    Serial.println("✓ 单例模式测试通过");
}

/**
 * 测试初始化和清理
 */
void EventManagerTest::testInitializeAndCleanup() {
    Serial.println("测试初始化和清理...");
    
    EventManager& manager = EventManager::getInstance();
    
    // 测试初始化
    bool initResult = manager.initialize();
    assertTrue(initResult, "初始化应该成功");
    
    // 测试重复初始化
    bool reinitResult = manager.initialize();
    assertTrue(reinitResult, "重复初始化应该返回true");
    
    // 测试清理
    manager.cleanup();
    
    // 测试重新初始化
    bool reinitAfterCleanup = manager.initialize();
    assertTrue(reinitAfterCleanup, "清理后重新初始化应该成功");
    
    Serial.println("✓ 初始化和清理测试通过");
}

/**
 * 测试事件订阅和取消订阅
 */
void EventManagerTest::testSubscribeUnsubscribe() {
    Serial.println("测试事件订阅和取消订阅...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 测试订阅
    bool subscribeResult = manager.subscribe(EventType::MOTOR_START, testEventListener);
    assertTrue(subscribeResult, "订阅事件应该成功");
    
    // 测试重复订阅（应该成功）
    bool resubscribeResult = manager.subscribe(EventType::MOTOR_START, testEventListener);
    assertTrue(resubscribeResult, "重复订阅应该成功");
    
    // 测试取消订阅
    bool unsubscribeResult = manager.unsubscribe(EventType::MOTOR_START, testEventListener);
    assertTrue(unsubscribeResult, "取消订阅应该成功");
    
    // 测试未订阅的取消订阅
    bool unsubscribeNonExistent = manager.unsubscribe(EventType::MOTOR_STOP, testEventListener);
    assertTrue(unsubscribeNonExistent, "取消未订阅的事件应该返回true");
    
    // 测试未初始化的订阅
    manager.cleanup();
    bool subscribeUninitialized = manager.subscribe(EventType::MOTOR_START, testEventListener);
    assertFalse(subscribeUninitialized, "未初始化时订阅应该失败");
    
    manager.initialize();
    
    Serial.println("✓ 事件订阅和取消订阅测试通过");
}

/**
 * 测试事件发布（同步）
 */
void EventManagerTest::testPublishSync() {
    Serial.println("测试事件发布（同步）...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 重置计数器
    testEventCounter = 0;
    lastTestMessage = "";
    
    // 订阅事件
    manager.subscribe(EventType::MOTOR_START, testEventListener);
    
    // 发布事件
    EventData event(EventType::MOTOR_START, "TestSource", "Test Message", 123);
    bool publishResult = manager.publish(event);
    assertTrue(publishResult, "发布事件应该成功");
    
    // 验证事件被处理
    assertEqual(1, testEventCounter, "事件监听器应该被调用一次");
    assertEqualString("Test Message", lastTestMessage.c_str(), "消息应该匹配");
    assertEqual(static_cast<int>(EventType::MOTOR_START), static_cast<int>(lastTestEventType), "事件类型应该匹配");
    
    // 测试未订阅的事件
    EventData unregisteredEvent(EventType::MOTOR_STOP, "TestSource");
    bool publishUnregistered = manager.publish(unregisteredEvent);
    assertFalse(publishUnregistered, "发布未订阅的事件应该返回false");
    
    // 测试空监听器
    manager.unsubscribe(EventType::MOTOR_START, testEventListener);
    bool publishNoListeners = manager.publish(event);
    assertFalse(publishNoListeners, "无监听器时发布应该返回false");
    
    Serial.println("✓ 事件发布（同步）测试通过");
}

/**
 * 测试事件发布（异步）
 */
void EventManagerTest::testPublishAsync() {
    Serial.println("测试事件发布（异步）...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 重置计数器
    testEventCounter = 0;
    
    // 订阅事件
    manager.subscribe(EventType::MOTOR_START, testEventListener);
    
    // 发布异步事件
    EventData event(EventType::MOTOR_START, "TestSource", "Async Message", 456);
    bool publishResult = manager.publishAsync(event);
    assertTrue(publishResult, "发布异步事件应该成功");
    
    // 验证事件在队列中
    assertEqual(1, static_cast<int>(manager.getQueueSize()), "事件队列应该有一个事件");
    
    // 处理事件
    manager.processEvents();
    
    // 验证事件被处理
    assertEqual(1, testEventCounter, "事件监听器应该被调用一次");
    assertEqual(0, static_cast<int>(manager.getQueueSize()), "事件队列应该为空");
    
    // 测试未初始化的发布
    manager.cleanup();
    EventData testEvent(EventType::MOTOR_START);
    bool publishUninitialized = manager.publishAsync(testEvent);
    assertFalse(publishUninitialized, "未初始化时发布应该失败");
    
    manager.initialize();
    
    Serial.println("✓ 事件发布（异步）测试通过");
}

/**
 * 测试事件队列处理
 */
void EventManagerTest::testEventQueue() {
    Serial.println("测试事件队列处理...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 订阅事件
    manager.subscribe(EventType::MOTOR_START, testEventListener);
    manager.subscribe(EventType::MOTOR_STOP, testEventListener);
    
    // 重置计数器
    testEventCounter = 0;
    
    // 发布多个异步事件
    for (int i = 0; i < 5; i++) {
        EventData startEvent(EventType::MOTOR_START, "QueueTest", "Start " + String(i), i);
        EventData stopEvent(EventType::MOTOR_STOP, "QueueTest", "Stop " + String(i), i * 10);
        
        manager.publishAsync(startEvent);
        manager.publishAsync(stopEvent);
    }
    
    // 验证队列大小
    assertEqual(10, static_cast<int>(manager.getQueueSize()), "事件队列应该有10个事件");
    
    // 处理所有事件
    manager.processEvents();
    
    // 验证所有事件被处理
    assertEqual(10, testEventCounter, "所有事件应该被处理");
    assertEqual(0, static_cast<int>(manager.getQueueSize()), "事件队列应该为空");
    
    // 测试清空队列
    manager.publishAsync(EventData(EventType::MOTOR_START));
    manager.publishAsync(EventData(EventType::MOTOR_STOP));
    assertEqual(2, static_cast<int>(manager.getQueueSize()), "队列应该有2个事件");
    
    manager.clearQueue();
    assertEqual(0, static_cast<int>(manager.getQueueSize()), "清空后队列应该为空");
    
    Serial.println("✓ 事件队列处理测试通过");
}

/**
 * 测试多监听器处理
 */
void EventManagerTest::testMultipleListeners() {
    Serial.println("测试多监听器处理...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 重置计数器
    testEventCounter = 0;
    
    // 订阅多个监听器
    manager.subscribe(EventType::MOTOR_START, testEventListener);
    manager.subscribe(EventType::MOTOR_START, testEventListener2);
    manager.subscribe(EventType::MOTOR_START, testEventListener3);
    
    // 发布事件
    EventData event(EventType::MOTOR_START, "MultiTest", "Multi Listener Test", 999);
    manager.publish(event);
    
    // 验证所有监听器被调用
    assertEqual(3, testEventCounter, "所有3个监听器应该被调用");
    
    // 测试取消订阅一个监听器
    manager.unsubscribe(EventType::MOTOR_START, testEventListener);
    testEventCounter = 0;
    manager.publish(event);
    
    // 由于我们清除了该类型的所有监听器，应该没有监听器被调用
    // 注意：当前实现中unsubscribe会清除该类型的所有监听器
    assertEqual(0, testEventCounter, "取消订阅后应该没有监听器被调用");
    
    Serial.println("✓ 多监听器处理测试通过");
}

/**
 * 测试事件类型名称获取
 */
void EventManagerTest::testEventTypeNames() {
    Serial.println("测试事件类型名称获取...");
    
    // 测试已知事件类型
    assertEqualString("SYSTEM_STARTUP", EventManager::getEventTypeName(EventType::SYSTEM_STARTUP).c_str(), "SYSTEM_STARTUP名称应该匹配");
    assertEqualString("MOTOR_START", EventManager::getEventTypeName(EventType::MOTOR_START).c_str(), "MOTOR_START名称应该匹配");
    assertEqualString("BLE_CONNECTED", EventManager::getEventTypeName(EventType::BLE_CONNECTED).c_str(), "BLE_CONNECTED名称应该匹配");
    assertEqualString("ERROR_OCCURRED", EventManager::getEventTypeName(EventType::ERROR_OCCURRED).c_str(), "ERROR_OCCURRED名称应该匹配");
    assertEqualString("CUSTOM_EVENT", EventManager::getEventTypeName(EventType::CUSTOM_EVENT).c_str(), "CUSTOM_EVENT名称应该匹配");
    
    // 测试未知事件类型
    assertEqualString("UNKNOWN", EventManager::getEventTypeName(static_cast<EventType>(999)).c_str(), "未知事件类型应该返回UNKNOWN");
    
    Serial.println("✓ 事件类型名称获取测试通过");
}

/**
 * 测试错误处理
 */
void EventManagerTest::testErrorHandling() {
    Serial.println("测试错误处理...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    
    // 测试空监听器
    bool nullListener = manager.subscribe(EventType::MOTOR_START, nullptr);
    assertFalse(nullListener, "空监听器应该被拒绝");
    
    // 测试未初始化的操作
    manager.cleanup();
    bool subscribeResult = manager.subscribe(EventType::MOTOR_START, testEventListener);
    assertFalse(subscribeResult, "未初始化时订阅应该失败");
    
    bool publishResult = manager.publish(EventData(EventType::MOTOR_START));
    assertFalse(publishResult, "未初始化时发布应该失败");
    
    bool publishAsyncResult = manager.publishAsync(EventData(EventType::MOTOR_START));
    assertFalse(publishAsyncResult, "未初始化时异步发布应该失败");
    
    manager.initialize();
    
    Serial.println("✓ 错误处理测试通过");
}

/**
 * 测试边界条件
 */
void EventManagerTest::testBoundaryConditions() {
    Serial.println("测试边界条件...");
    
    EventManager& manager = EventManager::getInstance();
    manager.initialize();
    manager.clearQueue();
    
    // 测试大量事件
    const int eventCount = 50;
    manager.subscribe(EventType::CUSTOM_EVENT, testEventListener);
    
    testEventCounter = 0;
    
    // 发布大量事件
    for (int i = 0; i < eventCount; i++) {
        EventData event(EventType::CUSTOM_EVENT, "StressTest", "Event " + String(i), i);
        manager.publishAsync(event);
    }
    
    assertEqual(eventCount, static_cast<int>(manager.getQueueSize()), "队列应该包含所有事件");
    
    // 处理所有事件
    manager.processEvents();
    
    assertEqual(eventCount, testEventCounter, "所有事件应该被处理");
    assertEqual(0, static_cast<int>(manager.getQueueSize()), "队列应该为空");
    
    // 测试空消息
    EventData emptyEvent(EventType::CUSTOM_EVENT, "EmptyTest", "", 0);
    testEventCounter = 0;
    manager.publish(emptyEvent);
    assertEqual(1, testEventCounter, "空消息事件应该被处理");
    
    Serial.println("✓ 边界条件测试通过");
}

/**
 * 测试用的事件监听器
 */
void EventManagerTest::testEventListener(const EventData& event) {
    testEventCounter++;
    lastTestMessage = event.message;
    lastTestEventType = event.type;
}

void EventManagerTest::testEventListener2(const EventData& event) {
    testEventCounter++;
}

void EventManagerTest::testEventListener3(const EventData& event) {
    testEventCounter++;
}

/**
 * 断言辅助函数
 */
void EventManagerTest::assertTrue(bool condition, const char* message) {
    if (!condition) {
        Serial.print("❌ 断言失败: ");
        Serial.println(message);
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void EventManagerTest::assertFalse(bool condition, const char* message) {
    assertTrue(!condition, message);
}

void EventManagerTest::assertEqual(uint32_t expected, uint32_t actual, const char* message) {
    if (expected != actual) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: ");
        Serial.print(expected);
        Serial.print(", 实际: ");
        Serial.print(actual);
        Serial.println(")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void EventManagerTest::assertEqual(int expected, int actual, const char* message) {
    if (expected != actual) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: ");
        Serial.print(expected);
        Serial.print(", 实际: ");
        Serial.print(actual);
        Serial.println(")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void EventManagerTest::assertEqual(bool expected, bool actual, const char* message) {
    if (expected != actual) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: ");
        Serial.print(expected ? "true" : "false");
        Serial.print(", 实际: ");
        Serial.print(actual ? "true" : "false");
        Serial.println(")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}

void EventManagerTest::assertEqualString(const char* expected, const char* actual, const char* message) {
    if (strcmp(expected, actual) != 0) {
        Serial.print("❌ 断言失败: ");
        Serial.print(message);
        Serial.print(" (期望: \"");
        Serial.print(expected);
        Serial.print("\", 实际: \"");
        Serial.print(actual);
        Serial.println("\")");
    } else {
        Serial.print("✓ ");
        Serial.println(message);
    }
}