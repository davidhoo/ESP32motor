#ifndef WS2812_TEST_H
#define WS2812_TEST_H

#include "drivers/WS2812Driver.h"
#include "common/Logger.h"
#include "common/Config.h"

/**
 * WS2812驱动测试类
 * 负责执行所有WS2812 LED相关的测试功能
 */
class WS2812Test {
private:
    WS2812Driver* ledDriver;
    unsigned long lastUpdate;
    int testPhase;
    
    // RGB测试状态
    int rgbTestStep;
    unsigned long rgbTestLastUpdate;
    
    // HSV测试状态
    int hsvTestStep;
    unsigned long hsvTestLastUpdate;
    
    // 亮度测试状态
    int brightnessTestStep;
    unsigned long brightnessTestLastUpdate;
    bool brightnessTestDirection;
    
    // 动画测试状态
    int animationTestStep;
    int animationTestCycle;
    unsigned long animationTestLastUpdate;
    
public:
    /**
     * 构造函数
     * @param driver WS2812驱动实例指针
     */
    WS2812Test(WS2812Driver* driver);
    
    /**
     * 初始化测试
     * 执行WS2812驱动的初始化测试
     * @return 测试是否成功
     */
    bool initializeTest();
    
    /**
     * 运行循环测试
     * 执行LED显示效果的循环测试
     */
    void runLoopTest();
    
    /**
     * 测试引脚初始化
     * @return 测试是否通过
     */
    bool testInitialization();
    
    /**
     * 测试RGB颜色显示
     * @return 测试是否通过
     */
    bool testRGBColors();
    
    /**
     * 测试HSV颜色显示
     * @return 测试是否通过
     */
    bool testHSVColors();
    
    /**
     * 测试亮度控制
     * @return 测试是否通过
     */
    bool testBrightnessControl();
    
    /**
     * 测试动画效果
     */
    void testAnimations();
    
    /**
     * 显示系统状态
     */
    void showSystemStatus();
};

#endif // WS2812_TEST_H