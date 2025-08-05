#ifndef LED_CONTROLLER_TEST_H
#define LED_CONTROLLER_TEST_H

#include <Arduino.h>
#include "../controllers/LEDController.h"

/**
 * @brief LED控制器测试类
 * 用于测试LEDController的各项功能
 */
class LEDControllerTest {
public:
    /**
     * @brief 运行所有测试
     */
    static void runAllTests();
    
    /**
     * @brief 测试LED控制器初始化
     */
    static void testInit();
    
    /**
     * @brief 测试LED状态设置
     */
    static void testSetState();
    
    /**
     * @brief 测试闪烁效果
     */
    static void testBlinking();
    
    /**
     * @brief 测试颜色映射
     */
    static void testColorMapping();
    
    /**
     * @brief 测试停止功能
     */
    static void testStop();
    
    /**
     * @brief 测试LED测试功能
     */
    static void testLEDTest();
    
    /**
     * @brief 测试状态获取
     */
    static void testGetCurrentState();
    
    /**
     * @brief 测试边界条件
     */
    static void testBoundaryConditions();

private:
    /**
     * @brief 断言函数
     * @param condition 条件
     * @param message 消息
     */
    static void assertTrue(bool condition, const char* message);
    
    /**
     * @brief 延迟函数
     * @param ms 毫秒
     */
    static void delay(uint32_t ms);
};

#endif // LED_CONTROLLER_TEST_H