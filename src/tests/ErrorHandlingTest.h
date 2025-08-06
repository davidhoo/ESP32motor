#ifndef ERROR_HANDLING_TEST_H
#define ERROR_HANDLING_TEST_H

/**
 * @brief 错误处理测试类
 * 
 * 用于测试5.4部分实现的错误处理机制：
 * - 5.4.1 模块初始化失败的错误处理机制
 * - 5.4.2 参数越界检查和默认值回退功能
 * - 5.4.3 BLE断连时的系统稳定运行机制
 */
class ErrorHandlingTest {
public:
    /**
     * @brief 构造函数
     */
    ErrorHandlingTest();
    
    /**
     * @brief 析构函数
     */
    ~ErrorHandlingTest();
    
    /**
     * @brief 运行所有错误处理测试
     */
    void runAllTests();
    
    /**
     * @brief 检查是否所有测试都通过
     * @return true 所有测试通过，false 有测试失败
     */
    bool allTestsPassed() const;
    
    /**
     * @brief 获取通过的测试数量
     * @return 通过的测试数量
     */
    int getPassedCount() const;
    
    /**
     * @brief 获取失败的测试数量
     * @return 失败的测试数量
     */
    int getFailedCount() const;

private:
    // 测试计数器
    int testsPassed;
    int testsFailed;
    
    // === 5.4.1 模块初始化失败的错误处理机制测试 ===
    
    /**
     * @brief 测试模块初始化失败处理
     */
    void testModuleInitializationFailure();
    
    /**
     * @brief 测试初始化重试机制
     */
    void testInitializationRetryMechanism();
    
    /**
     * @brief 测试安全模式激活
     */
    void testSafeModeActivation();
    
    /**
     * @brief 测试非关键模块失败处理
     */
    void testNonCriticalModuleFailure();
    
    // === 5.4.2 参数越界检查和默认值回退功能测试 ===
    
    /**
     * @brief 测试参数验证和回退功能
     */
    void testParameterValidationAndFallback();
    
    /**
     * @brief 测试运行时长参数验证
     */
    void testRunDurationValidation();
    
    /**
     * @brief 测试停止时长参数验证
     */
    void testStopDurationValidation();
    
    /**
     * @brief 测试循环次数参数验证
     */
    void testCycleCountValidation();
    
    /**
     * @brief 测试参数自动修正功能
     */
    void testParameterAutoCorrection();
    
    // === 5.4.3 BLE断连时的系统稳定运行机制测试 ===
    
    /**
     * @brief 测试BLE断连稳定性
     */
    void testBLEDisconnectionStability();
    
    /**
     * @brief 测试断连处理机制
     */
    void testDisconnectionHandling();
    
    /**
     * @brief 测试断连后系统稳定性
     */
    void testSystemStabilityAfterDisconnection();
    
    /**
     * @brief 测试重连机制
     */
    void testReconnectionMechanism();
    
    /**
     * @brief 打印测试结果
     */
    void printTestResults();
};

#endif // ERROR_HANDLING_TEST_H