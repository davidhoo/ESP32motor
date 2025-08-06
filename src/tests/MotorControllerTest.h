#ifndef MOTOR_CONTROLLER_TEST_H
#define MOTOR_CONTROLLER_TEST_H

#include <Arduino.h>
#include "../controllers/MotorController.h"

/**
 * @brief 电机控制器测试类
 * 用于测试MotorController的各项功能
 */
class MotorControllerTest {
public:
    /**
     * @brief 运行所有测试
     */
    static void runAllTests();
    
    /**
     * @brief 测试电机控制器初始化
     */
    static void testInit();
    
    /**
     * @brief 测试电机启动和停止
     */
    static void testStartStop();
    
    /**
     * @brief 测试状态机转换
     */
    static void testStateTransitions();
    
    /**
     * @brief 测试循环控制
     */
    static void testCycleControl();
    
    /**
     * @brief 测试倒计时功能
     */
    static void testCountdown();
    
    /**
     * @brief 测试配置更新
     */
    static void testConfigUpdate();
    
    /**
     * @brief 测试参数查询接口
     */
    static void testParameterQueries();
    
    /**
     * @brief 测试边界条件
     */
    static void testBoundaryConditions();
    
    /**
     * @brief 测试错误处理
     */
    static void testErrorHandling();
    
    /**
     * @brief 测试循环计数器
     */
    static void testCycleCounter();

private:
    /**
     * @brief 断言函数
     * @param condition 条件
     * @param message 消息
     */
    static void assertTrue(bool condition, const char* message);
    
    /**
     * @brief 断言相等
     * @param expected 期望值
     * @param actual 实际值
     * @param message 消息
     */
    static void assertEqual(uint32_t expected, uint32_t actual, const char* message);
    
    /**
     * @brief 断言状态相等
     * @param expected 期望状态
     * @param actual 实际状态
     * @param message 消息
     */
    static void assertEqual(MotorControllerState expected, MotorControllerState actual, const char* message);
    
    /**
     * @brief 延迟函数
     * @param ms 毫秒
     */
    static void delay(uint32_t ms);
};

#endif // MOTOR_CONTROLLER_TEST_H