#ifndef TIMER_TEST_H
#define TIMER_TEST_H

#include <Arduino.h>
#include "../drivers/TimerDriver.h"
#include "../common/Logger.h"

/**
 * 定时器测试类
 * 用于测试TimerDriver的各种功能和精度
 */
class TimerTest {
public:
    /**
     * 构造函数
     */
    TimerTest();
    
    /**
     * 析构函数
     */
    ~TimerTest();
    
    /**
     * 运行所有测试
     * @return 测试是否全部通过
     */
    bool runAllTests();
    
    /**
     * 测试定时器基本功能
     * @return 测试是否通过
     */
    bool testBasicFunctionality();
    
    /**
     * 测试定时器精度
     * @return 测试是否通过
     */
    bool testTimerAccuracy();
    
    /**
     * 测试多定时器并发
     * @return 测试是否通过
     */
    bool testMultipleTimers();
    
    /**
     * 测试定时器回调功能
     * @return 测试是否通过
     */
    bool testTimerCallbacks();
    
    /**
     * 测试定时器控制功能
     * @return 测试是否通过
     */
    bool testTimerControl();
    
    /**
     * 测试定时器间隔修改
     * @return 测试是否通过
     */
    bool testTimerIntervalChange();
    
    /**
     * 测试定时器错误处理
     * @return 测试是否通过
     */
    bool testErrorHandling();
    
    /**
     * 测试定时器性能
     * @return 测试是否通过
     */
    bool testTimerPerformance();
    
    /**
     * 获取测试结果统计
     */
    void printTestResults();

private:
    TimerDriver& timer_driver;          // 定时器驱动引用
    int total_tests;                    // 总测试数
    int passed_tests;                   // 通过测试数
    int failed_tests;                   // 失败测试数
    
    // 测试用的回调计数器
    static volatile uint32_t callback_count_0;
    static volatile uint32_t callback_count_1;
    static volatile uint32_t callback_count_2;
    static volatile uint32_t callback_count_3;
    
    // 测试用的时间戳
    static volatile uint32_t last_callback_time_0;
    static volatile uint32_t last_callback_time_1;
    static volatile uint32_t last_callback_time_2;
    static volatile uint32_t last_callback_time_3;
    
    /**
     * 定时器0回调函数
     */
    static void timer0Callback();
    
    /**
     * 定时器1回调函数
     */
    static void timer1Callback();
    
    /**
     * 定时器2回调函数
     */
    static void timer2Callback();
    
    /**
     * 定时器3回调函数
     */
    static void timer3Callback();
    
    /**
     * 重置测试计数器
     */
    void resetTestCounters();
    
    /**
     * 等待指定时间
     * @param wait_ms 等待时间(毫秒)
     */
    void waitMs(uint32_t wait_ms);
    
    /**
     * 检查定时器精度
     * @param timer_id 定时器ID
     * @param expected_interval 期望间隔(毫秒)
     * @param tolerance_percent 容差百分比
     * @return 精度是否在容差范围内
     */
    bool checkTimerAccuracy(TimerDriver::TimerID timer_id, 
                           uint32_t expected_interval, 
                           float tolerance_percent = 5.0f);
    
    /**
     * 记录测试结果
     * @param test_name 测试名称
     * @param result 测试结果
     */
    void recordTestResult(const char* test_name, bool result);
    
    /**
     * 打印测试开始信息
     * @param test_name 测试名称
     */
    void printTestStart(const char* test_name);
    
    /**
     * 打印测试结果
     * @param test_name 测试名称
     * @param result 测试结果
     */
    void printTestResult(const char* test_name, bool result);
};

#endif // TIMER_TEST_H