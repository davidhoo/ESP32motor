#ifndef MODBUS_TEST_H
#define MODBUS_TEST_H

#include <Arduino.h>
#include "../controllers/MotorModbusController.h"

class ModbusTest {
private:
    MotorModbusController& modbusController;

public:
    ModbusTest(MotorModbusController& controller);
    
    /**
     * 运行所有MODBUS测试
     */
    void runAllTests();
    
    /**
     * 初始化MODBUS通信测试
     */
    void testInit();
    
    /**
     * 读取运行状态测试
     */
    void testReadStatus();
    
    /**
     * 读取频率测试
     */
    void testReadFrequency();
    
    /**
     * 读取占空比测试
     */
    void testReadDuty();
    
    /**
     * 读取完整配置测试
     */
    void testReadConfig();
    
    /**
     * 一次性读取所有配置测试
     */
    void testGetAllConfig();
    
    /**
     * 设置新频率测试
     */
    void testSetFrequency();
    
    /**
     * 设置新占空比测试
     */
    void testSetDuty();
    
    /**
     * 启动电机测试
     */
    void testStartMotor();
    
    /**
     * 停止电机测试
     */
    void testStopMotor();
    
private:
    /**
     * 打印MODBUS操作结果
     */
    void printResult(bool success, const char* operation);
};

#endif // MODBUS_TEST_H