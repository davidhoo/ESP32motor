#ifndef EVENT_MANAGER_TEST_H
#define EVENT_MANAGER_TEST_H

#include <Arduino.h>
#include "../common/EventManager.h"

/**
 * EventManager单元测试类
 */
class EventManagerTest {
public:
    /**
     * 运行所有测试
     */
    static void runAllTests();
    
private:
    /**
     * 测试单例模式
     */
    static void testSingleton();
    
    /**
     * 测试初始化和清理
     */
    static void testInitializeAndCleanup();
    
    /**
     * 测试事件订阅和取消订阅
     */
    static void testSubscribeUnsubscribe();
    
    /**
     * 测试事件发布（同步）
     */
    static void testPublishSync();
    
    /**
     * 测试事件发布（异步）
     */
    static void testPublishAsync();
    
    /**
     * 测试事件队列处理
     */
    static void testEventQueue();
    
    /**
     * 测试多监听器处理
     */
    static void testMultipleListeners();
    
    /**
     * 测试事件类型名称获取
     */
    static void testEventTypeNames();
    
    /**
     * 测试错误处理
     */
    static void testErrorHandling();
    
    /**
     * 测试边界条件
     */
    static void testBoundaryConditions();
    
    /**
     * 断言辅助函数
     */
    static void assertTrue(bool condition, const char* message);
    static void assertFalse(bool condition, const char* message);
    static void assertEqual(uint32_t expected, uint32_t actual, const char* message);
    static void assertEqual(int expected, int actual, const char* message);
    static void assertEqual(bool expected, bool actual, const char* message);
    static void assertEqualString(const char* expected, const char* actual, const char* message);
    
    /**
     * 测试用的全局变量
     */
    static int testEventCounter;
    static String lastTestMessage;
    static EventType lastTestEventType;
    
    /**
     * 测试用的事件监听器
     */
    static void testEventListener(const EventData& event);
    static void testEventListener2(const EventData& event);
    static void testEventListener3(const EventData& event);
};

#endif // EVENT_MANAGER_TEST_H