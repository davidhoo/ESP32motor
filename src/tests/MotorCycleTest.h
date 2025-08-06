#ifndef MOTOR_CYCLE_TEST_H
#define MOTOR_CYCLE_TEST_H

/**
 * @brief 电机循环控制测试类
 * 
 * 用于测试电机循环控制流程的各种场景：
 * - 基本循环控制 (运行X时长 → 停止Y秒)
 * - 持续运行模式 (停止间隔为0)
 * - 无限循环模式 (循环次数为0)
 * - 配置动态更新
 */
class MotorCycleTest {
public:
    /**
     * @brief 测试基本循环控制
     * 测试运行X时长 → 停止Y秒的基本循环逻辑
     * @return true 测试通过，false 测试失败
     */
    static bool testBasicCycle();
    
    /**
     * @brief 测试持续运行模式
     * 测试停止间隔为0时的持续运行逻辑
     * @return true 测试通过，false 测试失败
     */
    static bool testContinuousMode();
    
    /**
     * @brief 测试无限循环模式
     * 测试循环次数为0时的无限循环逻辑
     * @return true 测试通过，false 测试失败
     */
    static bool testInfiniteMode();
    
    /**
     * @brief 测试配置动态更新
     * 测试运行时动态更新配置参数的功能
     * @return true 测试通过，false 测试失败
     */
    static bool testConfigUpdate();
    
    /**
     * @brief 运行所有测试用例
     * @return true 所有测试通过，false 部分测试失败
     */
    static bool runAllTests();
};

#endif // MOTOR_CYCLE_TEST_H