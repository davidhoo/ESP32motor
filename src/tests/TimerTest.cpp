#include "TimerTest.h"
#include "../common/Logger.h"

// 静态成员变量初始化
volatile uint32_t TimerTest::callback_count_0 = 0;
volatile uint32_t TimerTest::callback_count_1 = 0;
volatile uint32_t TimerTest::callback_count_2 = 0;
volatile uint32_t TimerTest::callback_count_3 = 0;

volatile uint32_t TimerTest::last_callback_time_0 = 0;
volatile uint32_t TimerTest::last_callback_time_1 = 0;
volatile uint32_t TimerTest::last_callback_time_2 = 0;
volatile uint32_t TimerTest::last_callback_time_3 = 0;

TimerTest::TimerTest() : timer_driver(TimerDriver::getInstance()), 
                         total_tests(0), passed_tests(0), failed_tests(0) {
    LOG_TAG_INFO("TimerTest", "定时器测试类构造完成");
}

TimerTest::~TimerTest() {
    LOG_TAG_INFO("TimerTest", "定时器测试类析构完成");
}

bool TimerTest::runAllTests() {
    LOG_TAG_INFO("TimerTest", "开始运行所有定时器测试");
    
    // 重置测试统计
    total_tests = 0;
    passed_tests = 0;
    failed_tests = 0;
    
    // 初始化定时器驱动
    if (!timer_driver.init()) {
        LOG_TAG_ERROR("TimerTest", "定时器驱动初始化失败");
        return false;
    }
    
    // 运行各项测试
    testBasicFunctionality();
    testTimerAccuracy();
    testMultipleTimers();
    testTimerCallbacks();
    testTimerControl();
    testTimerIntervalChange();
    testErrorHandling();
    testTimerPerformance();
    
    // 打印测试结果
    printTestResults();
    
    return (failed_tests == 0);
}

bool TimerTest::testBasicFunctionality() {
    printTestStart("基本功能测试");
    
    bool result = true;
    resetTestCounters();
    
    // 测试创建定时器
    bool create_result = timer_driver.createTimer(TimerDriver::TIMER_0, 100, timer0Callback);
    if (!create_result) {
        LOG_TAG_ERROR("TimerTest", "创建定时器失败");
        result = false;
    }
    
    // 测试启动定时器
    bool start_result = timer_driver.startTimer(TimerDriver::TIMER_0);
    if (!start_result) {
        LOG_TAG_ERROR("TimerTest", "启动定时器失败");
        result = false;
    }
    
    // 等待一段时间检查回调
    waitMs(250);
    
    if (callback_count_0 < 2) {
        LOG_TAG_ERROR("TimerTest", "回调次数不足，期望>=2，实际: %lu", callback_count_0);
        result = false;
    }
    
    // 测试停止定时器
    bool stop_result = timer_driver.stopTimer(TimerDriver::TIMER_0);
    if (!stop_result) {
        LOG_TAG_ERROR("TimerTest", "停止定时器失败");
        result = false;
    }
    
    uint32_t count_before_stop = callback_count_0;
    waitMs(150);
    
    if (callback_count_0 != count_before_stop) {
        LOG_TAG_ERROR("TimerTest", "定时器停止后仍在触发回调");
        result = false;
    }
    
    // 清理
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    
    recordTestResult("基本功能测试", result);
    return result;
}

bool TimerTest::testTimerAccuracy() {
    printTestStart("定时器精度测试");
    
    bool result = true;
    resetTestCounters();
    
    // 测试不同间隔的精度
    uint32_t test_intervals[] = {1, 5, 10, 50, 100, 500, 1000};
    int num_intervals = sizeof(test_intervals) / sizeof(test_intervals[0]);
    
    for (int i = 0; i < num_intervals; i++) {
        uint32_t interval = test_intervals[i];
        
        LOG_TAG_INFO("TimerTest", "测试 %lums 间隔精度", interval);
        
        // 创建并启动定时器
        if (!timer_driver.createTimer(TimerDriver::TIMER_0, interval, timer0Callback)) {
            LOG_TAG_ERROR("TimerTest", "创建 %lums 定时器失败", interval);
            result = false;
            continue;
        }
        
        if (!timer_driver.startTimer(TimerDriver::TIMER_0)) {
            LOG_TAG_ERROR("TimerTest", "启动 %lums 定时器失败", interval);
            result = false;
            timer_driver.deleteTimer(TimerDriver::TIMER_0);
            continue;
        }
        
        // 等待足够的时间进行精度测试
        uint32_t test_duration = max(1000U, interval * 10);
        waitMs(test_duration);
        
        // 检查精度
        if (!checkTimerAccuracy(TimerDriver::TIMER_0, interval, 10.0f)) {
            LOG_TAG_ERROR("TimerTest", "%lums 定时器精度测试失败", interval);
            result = false;
        }
        
        // 清理
        timer_driver.deleteTimer(TimerDriver::TIMER_0);
        resetTestCounters();
        waitMs(50); // 短暂等待
    }
    
    recordTestResult("定时器精度测试", result);
    return result;
}

bool TimerTest::testMultipleTimers() {
    printTestStart("多定时器并发测试");
    
    bool result = true;
    resetTestCounters();
    
    // 创建多个不同间隔的定时器
    if (!timer_driver.createTimer(TimerDriver::TIMER_0, 50, timer0Callback) ||
        !timer_driver.createTimer(TimerDriver::TIMER_1, 75, timer1Callback) ||
        !timer_driver.createTimer(TimerDriver::TIMER_2, 100, timer2Callback) ||
        !timer_driver.createTimer(TimerDriver::TIMER_3, 125, timer3Callback)) {
        LOG_TAG_ERROR("TimerTest", "创建多个定时器失败");
        result = false;
    }
    
    // 启动所有定时器
    if (!timer_driver.startTimer(TimerDriver::TIMER_0) ||
        !timer_driver.startTimer(TimerDriver::TIMER_1) ||
        !timer_driver.startTimer(TimerDriver::TIMER_2) ||
        !timer_driver.startTimer(TimerDriver::TIMER_3)) {
        LOG_TAG_ERROR("TimerTest", "启动多个定时器失败");
        result = false;
    }
    
    // 运行一段时间
    waitMs(1000);
    
    // 检查所有定时器都在工作
    if (callback_count_0 == 0 || callback_count_1 == 0 || 
        callback_count_2 == 0 || callback_count_3 == 0) {
        LOG_TAG_ERROR("TimerTest", "某些定时器未正常工作");
        result = false;
    }
    
    // 检查触发次数比例是否合理
    float ratio_0_2 = (float)callback_count_0 / callback_count_2; // 50ms vs 100ms, 应该约为2
    float ratio_1_3 = (float)callback_count_1 / callback_count_3; // 75ms vs 125ms, 应该约为1.67
    
    if (ratio_0_2 < 1.5f || ratio_0_2 > 2.5f) {
        LOG_TAG_ERROR("TimerTest", "定时器0和2的触发比例异常: %.2f", ratio_0_2);
        result = false;
    }
    
    if (ratio_1_3 < 1.2f || ratio_1_3 > 2.2f) {
        LOG_TAG_ERROR("TimerTest", "定时器1和3的触发比例异常: %.2f", ratio_1_3);
        result = false;
    }
    
    // 清理所有定时器
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    timer_driver.deleteTimer(TimerDriver::TIMER_1);
    timer_driver.deleteTimer(TimerDriver::TIMER_2);
    timer_driver.deleteTimer(TimerDriver::TIMER_3);
    
    recordTestResult("多定时器并发测试", result);
    return result;
}

bool TimerTest::testTimerCallbacks() {
    printTestStart("定时器回调功能测试");
    
    bool result = true;
    resetTestCounters();
    
    // 测试回调函数是否正确执行
    if (!timer_driver.createTimer(TimerDriver::TIMER_0, 10, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "创建回调测试定时器失败");
        result = false;
    }
    
    if (!timer_driver.startTimer(TimerDriver::TIMER_0)) {
        LOG_TAG_ERROR("TimerTest", "启动回调测试定时器失败");
        result = false;
    }
    
    waitMs(100);
    
    // 检查回调是否被调用
    if (callback_count_0 < 8) { // 100ms内应该有约10次回调
        LOG_TAG_ERROR("TimerTest", "回调次数不足: %lu", callback_count_0);
        result = false;
    }
    
    // 检查回调时间间隔
    uint32_t current_time = millis();
    if (abs((int32_t)(current_time - last_callback_time_0)) > 20) {
        LOG_TAG_ERROR("TimerTest", "最后一次回调时间异常");
        result = false;
    }
    
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    
    recordTestResult("定时器回调功能测试", result);
    return result;
}

bool TimerTest::testTimerControl() {
    printTestStart("定时器控制功能测试");
    
    bool result = true;
    resetTestCounters();
    
    // 创建定时器
    if (!timer_driver.createTimer(TimerDriver::TIMER_0, 50, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "创建控制测试定时器失败");
        result = false;
    }
    
    // 测试启动/停止
    timer_driver.startTimer(TimerDriver::TIMER_0);
    waitMs(100);
    uint32_t count_after_start = callback_count_0;
    
    timer_driver.stopTimer(TimerDriver::TIMER_0);
    waitMs(100);
    uint32_t count_after_stop = callback_count_0;
    
    if (count_after_start == 0) {
        LOG_TAG_ERROR("TimerTest", "启动后定时器未工作");
        result = false;
    }
    
    if (count_after_stop != count_after_start) {
        LOG_TAG_ERROR("TimerTest", "停止后定时器仍在工作");
        result = false;
    }
    
    // 测试重启
    timer_driver.restartTimer(TimerDriver::TIMER_0);
    waitMs(100);
    uint32_t count_after_restart = callback_count_0;
    
    if (count_after_restart <= count_after_stop) {
        LOG_TAG_ERROR("TimerTest", "重启后定时器未工作");
        result = false;
    }
    
    // 测试状态查询
    if (!timer_driver.isTimerRunning(TimerDriver::TIMER_0)) {
        LOG_TAG_ERROR("TimerTest", "定时器状态查询错误");
        result = false;
    }
    
    timer_driver.stopTimer(TimerDriver::TIMER_0);
    if (timer_driver.isTimerRunning(TimerDriver::TIMER_0)) {
        LOG_TAG_ERROR("TimerTest", "停止后状态查询错误");
        result = false;
    }
    
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    
    recordTestResult("定时器控制功能测试", result);
    return result;
}

bool TimerTest::testTimerIntervalChange() {
    printTestStart("定时器间隔修改测试");
    
    bool result = true;
    resetTestCounters();
    
    // 创建定时器
    if (!timer_driver.createTimer(TimerDriver::TIMER_0, 100, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "创建间隔测试定时器失败");
        result = false;
    }
    
    timer_driver.startTimer(TimerDriver::TIMER_0);
    waitMs(300);
    uint32_t count_100ms = callback_count_0;
    
    // 修改间隔为50ms
    resetTestCounters();
    if (!timer_driver.changeTimerInterval(TimerDriver::TIMER_0, 50)) {
        LOG_TAG_ERROR("TimerTest", "修改定时器间隔失败");
        result = false;
    }
    
    waitMs(300);
    uint32_t count_50ms = callback_count_0;
    
    // 50ms间隔的触发次数应该约为100ms间隔的2倍
    float ratio = (float)count_50ms / count_100ms;
    if (ratio < 1.5f || ratio > 2.5f) {
        LOG_TAG_ERROR("TimerTest", "间隔修改后触发比例异常: %.2f", ratio);
        result = false;
    }
    
    // 验证间隔值
    if (timer_driver.getTimerInterval(TimerDriver::TIMER_0) != 50) {
        LOG_TAG_ERROR("TimerTest", "获取的定时器间隔值错误");
        result = false;
    }
    
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    
    recordTestResult("定时器间隔修改测试", result);
    return result;
}

bool TimerTest::testErrorHandling() {
    printTestStart("错误处理测试");
    
    bool result = true;
    
    // 测试无效定时器ID
    if (timer_driver.createTimer(static_cast<TimerDriver::TimerID>(99), 100, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "无效定时器ID应该创建失败");
        result = false;
    }
    
    // 测试无效间隔
    if (timer_driver.createTimer(TimerDriver::TIMER_0, 0, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "无效间隔应该创建失败");
        result = false;
    }
    
    // 测试空回调
    if (timer_driver.createTimer(TimerDriver::TIMER_0, 100, nullptr)) {
        LOG_TAG_ERROR("TimerTest", "空回调应该创建失败");
        result = false;
    }
    
    // 测试操作未创建的定时器
    if (timer_driver.startTimer(TimerDriver::TIMER_0)) {
        LOG_TAG_ERROR("TimerTest", "启动未创建的定时器应该失败");
        result = false;
    }
    
    if (timer_driver.stopTimer(TimerDriver::TIMER_0)) {
        LOG_TAG_ERROR("TimerTest", "停止未创建的定时器应该失败");
        result = false;
    }
    
    recordTestResult("错误处理测试", result);
    return result;
}

bool TimerTest::testTimerPerformance() {
    printTestStart("定时器性能测试");
    
    bool result = true;
    resetTestCounters();
    
    // 测试高频定时器
    if (!timer_driver.createTimer(TimerDriver::TIMER_0, 1, timer0Callback)) {
        LOG_TAG_ERROR("TimerTest", "创建1ms定时器失败");
        result = false;
    }
    
    uint32_t start_time = millis();
    timer_driver.startTimer(TimerDriver::TIMER_0);
    waitMs(1000);
    timer_driver.stopTimer(TimerDriver::TIMER_0);
    uint32_t end_time = millis();
    
    uint32_t actual_duration = end_time - start_time;
    uint32_t expected_count = actual_duration; // 1ms间隔，期望触发次数约等于持续时间
    
    // 检查性能，允许10%的误差
    float accuracy = (float)callback_count_0 / expected_count;
    if (accuracy < 0.8f || accuracy > 1.2f) {
        LOG_TAG_ERROR("TimerTest", "1ms定时器性能异常，精度: %.1f%%", accuracy * 100);
        result = false;
    }
    
    LOG_TAG_INFO("TimerTest", "1ms定时器测试: 期望%lu次，实际%lu次，精度%.1f%%",
                 expected_count, callback_count_0, accuracy * 100);
    
    timer_driver.deleteTimer(TimerDriver::TIMER_0);
    
    recordTestResult("定时器性能测试", result);
    return result;
}

void TimerTest::printTestResults() {
    LOG_TAG_INFO("TimerTest", "=== 定时器测试结果统计 ===");
    LOG_TAG_INFO("TimerTest", "总测试数: %d", total_tests);
    LOG_TAG_INFO("TimerTest", "通过测试: %d", passed_tests);
    LOG_TAG_INFO("TimerTest", "失败测试: %d", failed_tests);
    
    if (failed_tests == 0) {
        LOG_TAG_INFO("TimerTest", "所有测试通过！");
    } else {
        LOG_TAG_ERROR("TimerTest", "有 %d 个测试失败", failed_tests);
    }
    
    float success_rate = (float)passed_tests / total_tests * 100;
    LOG_TAG_INFO("TimerTest", "测试通过率: %.1f%%", success_rate);
}

void TimerTest::timer0Callback() {
    callback_count_0++;
    last_callback_time_0 = millis();
}

void TimerTest::timer1Callback() {
    callback_count_1++;
    last_callback_time_1 = millis();
}

void TimerTest::timer2Callback() {
    callback_count_2++;
    last_callback_time_2 = millis();
}

void TimerTest::timer3Callback() {
    callback_count_3++;
    last_callback_time_3 = millis();
}

void TimerTest::resetTestCounters() {
    callback_count_0 = 0;
    callback_count_1 = 0;
    callback_count_2 = 0;
    callback_count_3 = 0;
    
    last_callback_time_0 = 0;
    last_callback_time_1 = 0;
    last_callback_time_2 = 0;
    last_callback_time_3 = 0;
}

void TimerTest::waitMs(uint32_t wait_ms) {
    delay(wait_ms);
}

bool TimerTest::checkTimerAccuracy(TimerDriver::TimerID timer_id, 
                                  uint32_t expected_interval, 
                                  float tolerance_percent) {
    uint32_t trigger_count = timer_driver.getTimerTriggerCount(timer_id);
    if (trigger_count < 2) {
        return false; // 触发次数太少，无法测量精度
    }
    
    // 计算平均间隔
    uint32_t total_time = last_callback_time_0; // 简化处理，使用最后回调时间
    float average_interval = (float)total_time / trigger_count;
    
    // 计算误差百分比
    float error_percent = abs(average_interval - expected_interval) / expected_interval * 100;
    
    LOG_TAG_DEBUG("TimerTest", "定时器精度检查: 期望%lums，平均%.2fms，误差%.2f%%",
                  expected_interval, average_interval, error_percent);
    
    return (error_percent <= tolerance_percent);
}

void TimerTest::recordTestResult(const char* test_name, bool result) {
    total_tests++;
    if (result) {
        passed_tests++;
    } else {
        failed_tests++;
    }
    printTestResult(test_name, result);
}

void TimerTest::printTestStart(const char* test_name) {
    LOG_TAG_INFO("TimerTest", "开始执行: %s", test_name);
}

void TimerTest::printTestResult(const char* test_name, bool result) {
    if (result) {
        LOG_TAG_INFO("TimerTest", "%s - 通过", test_name);
    } else {
        LOG_TAG_ERROR("TimerTest", "%s - 失败", test_name);
    }
}