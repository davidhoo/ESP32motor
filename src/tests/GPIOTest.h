#ifndef GPIO_TEST_H
#define GPIO_TEST_H

#include "drivers/GPIODriver.h"
#include "common/Logger.h"
#include "common/Config.h"

/**
 * GPIO驱动测试类
 * 负责执行所有GPIO相关的测试功能
 */
class GPIOTest {
private:
    GPIODriver* gpioDriver;
    unsigned long lastToggle;
    bool motorState;
    int cycleCount;
    
public:
    /**
     * 构造函数
     * @param driver GPIO驱动实例指针
     */
    GPIOTest(GPIODriver* driver);
    
    /**
     * 初始化测试
     * 执行GPIO驱动的初始化测试
     * @return 测试是否成功
     */
    bool initializeTest();
    
    /**
     * 运行循环测试
     * 执行电机控制的循环测试
     */
    void runLoopTest();
    
    /**
     * 测试引脚初始化
     * @return 测试是否通过
     */
    bool testPinInitialization();
    
    /**
     * 测试无效引脚处理
     * @return 测试是否通过
     */
    bool testInvalidPin();
    
    /**
     * 测试引脚状态查询
     * @return 测试是否通过
     */
    bool testPinStatusQuery();
    
    /**
     * 测试电机控制
     * @return 测试是否成功
     */
    bool testMotorControl();
    
    /**
     * 测试引脚切换功能
     */
    void testPinToggle();
    
    /**
     * 显示系统状态
     */
    void showSystemStatus();
};

#endif // GPIO_TEST_H