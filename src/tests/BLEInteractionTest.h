#ifndef BLE_INTERACTION_TEST_H
#define BLE_INTERACTION_TEST_H

#include <Arduino.h>

// 前向声明
class MotorBLEServer;
class MotorController;
class ConfigManager;

/**
 * @brief BLE交互流程测试类
 * 测试5.3部分的BLE交互流程功能
 */
class BLEInteractionTest {
public:
    /**
     * @brief 构造函数
     */
    BLEInteractionTest();
    
    /**
     * @brief 运行所有测试
     * @return true 所有测试通过，false 有测试失败
     */
    bool runAllTests();
    
    /**
     * @brief 获取测试结果摘要
     * @return 测试摘要字符串
     */
    String getTestSummary();

private:
    // 引用相关组件
    MotorBLEServer& bleServer;
    MotorController& motorController;
    ConfigManager& configManager;
    
    // 具体测试方法
    
    /**
     * @brief 测试1: 参数设置的即时生效逻辑
     * @return 测试是否通过
     */
    bool testConfigImmediateEffect();
    
    /**
     * @brief 测试2: 手动启动/停止命令的优先级处理
     * @return 测试是否通过
     */
    bool testCommandPriorityHandling();
    
    /**
     * @brief 测试3: 实时状态推送机制
     * @return 测试是否通过
     */
    bool testRealTimeStatusPush();
    
    /**
     * @brief 测试4: BLE连接状态处理
     * @return 测试是否通过
     */
    bool testBLEConnectionHandling();
    
    /**
     * @brief 测试5: 错误处理和恢复
     * @return 测试是否通过
     */
    bool testErrorHandlingAndRecovery();
    
    /**
     * @brief 初始化测试环境
     */
    void initializeTestEnvironment();
};

#endif // BLE_INTERACTION_TEST_H