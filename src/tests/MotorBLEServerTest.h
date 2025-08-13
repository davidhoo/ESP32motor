#ifndef MOTOR_BLE_SERVER_TEST_H
#define MOTOR_BLE_SERVER_TEST_H

#include <Arduino.h>
#include "../controllers/MotorBLEServer.h"

/**
 * @brief BLE服务器测试类
 * 测试BLE服务器的各项功能
 */
class MotorBLEServerTest {
public:
    /**
     * @brief 运行所有BLE服务器测试
     */
    static void runAllTests();
    
private:
    /**
     * @brief 测试单例模式
     */
    static void testSingleton();
    
    /**
     * @brief 测试初始化功能
     */
    static void testInitialization();
    
    /**
     * @brief 测试配置JSON生成
     */
    static void testConfigJsonGeneration();
    
    /**
     * @brief 测试信息JSON生成
     */
    static void testInfoJsonGeneration();
    
    /**
     * @brief 测试命令处理
     */
    static void testCommandHandling();
    
    /**
     * @brief 测试配置处理
     */
    static void testConfigHandling();
    
    /**
     * @brief 测试调速器状态JSON生成
     */
    static void testSpeedControllerStatusJsonGeneration();
};

#endif // MOTOR_BLE_SERVER_TEST_H