#ifndef STATE_MANAGER_TEST_H
#define STATE_MANAGER_TEST_H

#include <Arduino.h>

class StateManagerTest {
public:
    static void runAllTests();
    
private:
    static void testInitialState();
    static void testValidStateTransitions();
    static void testInvalidStateTransitions();
    static void testStateListeners();
    static void testStateHistory();
    static void testStateNames();
};

// 测试辅助宏 - 使用自定义前缀避免与Unity框架冲突
#define SM_TEST_ASSERT_EQUAL(expected, actual) \
    if ((expected) != (actual)) { \
        Serial.printf("TEST FAILED: Expected %d, got %d at %s:%d\n", \
                     (int)(expected), (int)(actual), __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define SM_TEST_ASSERT_TRUE(condition) \
    if (!(condition)) { \
        Serial.printf("TEST FAILED: Expected true, got false at %s:%d\n", \
                     __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define SM_TEST_ASSERT_FALSE(condition) \
    if (condition) { \
        Serial.printf("TEST FAILED: Expected false, got true at %s:%d\n", \
                     __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#define SM_TEST_ASSERT_EQUAL_STRING(expected, actual) \
    if (String(expected) != String(actual)) { \
        Serial.printf("TEST FAILED: Expected '%s', got '%s' at %s:%d\n", \
                     expected, actual, __FILE__, __LINE__); \
    } else { \
        Serial.printf("TEST PASSED: %s\n", __FUNCTION__); \
    }

#endif // STATE_MANAGER_TEST_H