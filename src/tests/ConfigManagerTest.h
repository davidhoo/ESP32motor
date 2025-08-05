#ifndef CONFIG_MANAGER_TEST_H
#define CONFIG_MANAGER_TEST_H

#include <Arduino.h>
#include "../controllers/ConfigManager.h"

/**
 * ConfigManager单元测试类
 */
class ConfigManagerTest {
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
     * 测试初始化
     */
    static void testInit();
    
    /**
     * 测试配置验证
     */
    static void testConfigValidation();
    
    /**
     * 测试配置加载和保存
     */
    static void testLoadSaveConfig();
    
    /**
     * 测试默认值处理
     */
    static void testDefaultValues();
    
    /**
     * 测试配置修改检测
     */
    static void testConfigModification();
    
    /**
     * 测试错误处理
     */
    static void testErrorHandling();
    
    /**
     * 测试边界值
     */
    static void testBoundaryValues();
    
    /**
     * 断言辅助函数
     */
    static void assertTrue(bool condition, const char* message);
    static void assertFalse(bool condition, const char* message);
    static void assertEqual(uint32_t expected, uint32_t actual, const char* message);
    static void assertEqual(bool expected, bool actual, const char* message);
    static void assertEqualString(const char* expected, const char* actual, const char* message);
};

#endif // CONFIG_MANAGER_TEST_H