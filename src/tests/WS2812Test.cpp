#include "WS2812Test.h"
#include "../common/Logger.h"
#include <Arduino.h>

WS2812Test::WS2812Test(WS2812Driver* driver)
    : ledDriver(driver), lastUpdate(0), testPhase(0),
      rgbTestStep(0), rgbTestLastUpdate(0),
      hsvTestStep(0), hsvTestLastUpdate(0),
      brightnessTestStep(0), brightnessTestLastUpdate(0), brightnessTestDirection(true),
      animationTestStep(0), animationTestCycle(0), animationTestLastUpdate(0) {
}

bool WS2812Test::initializeTest() {
    LOG_TAG_INFO("Test", "开始WS2812驱动测试...");
    
    // 测试WS2812驱动初始化
    bool success = true;
    
    // 测试初始化
    if (!testInitialization()) {
        success = false;
    }
    
    if (success) {
        LOG_TAG_INFO("Test", "WS2812驱动初始化测试完成");
        LOG_TAG_INFO("Test", "开始LED显示效果循环测试...");
    } else {
        LOG_TAG_ERROR("Test", "WS2812驱动初始化测试失败");
    }
    
    return success;
}

bool WS2812Test::testInitialization() {
    // 初始化WS2812驱动
    ledDriver->begin();
    LOG_TAG_INFO("Test", "WS2812驱动初始化成功");
    return true;
}

void WS2812Test::runLoopTest() {
    switch (testPhase) {
        case 0:
            if (testRGBColors()) {
                testPhase = 1;
            }
            break;
        case 1:
            if (testHSVColors()) {
                testPhase = 2;
            }
            break;
        case 2:
            if (testBrightnessControl()) {
                testPhase = 3;
            }
            break;
        case 3:
            testAnimations();
            break;
        case 4:
            showSystemStatus();
            testPhase = 0;
            break;
    }
}

bool WS2812Test::testRGBColors() {
    unsigned long currentTime = millis();
    
    // 每500ms切换一次颜色
    if (currentTime - rgbTestLastUpdate >= 500) {
        rgbTestLastUpdate = currentTime;
        
        switch (rgbTestStep) {
            case 0:
                LOG_TAG_INFO("Test", "测试RGB颜色显示...");
                ledDriver->setAllColor(255, 0, 0);  // 红色
                ledDriver->show();
                break;
            case 1:
                ledDriver->setAllColor(0, 255, 0);  // 绿色
                ledDriver->show();
                break;
            case 2:
                ledDriver->setAllColor(0, 0, 255);  // 蓝色
                ledDriver->show();
                break;
            case 3:
                ledDriver->setAllColor(255, 255, 255);  // 白色
                ledDriver->show();
                break;
            case 4:
                ledDriver->clear();
                ledDriver->show();
                LOG_TAG_INFO("Test", "RGB颜色测试完成");
                rgbTestStep = 0;  // 重置步骤
                return true;
        }
        
        rgbTestStep++;
    }
    
    return false;  // 测试未完成
}

bool WS2812Test::testHSVColors() {
    unsigned long currentTime = millis();
    
    // 每100ms更新一次颜色
    if (currentTime - hsvTestLastUpdate >= 100) {
        hsvTestLastUpdate = currentTime;
        
        if (hsvTestStep < 16) {
            // 通过HSV设置彩虹色效果
            ledDriver->setAllColorHSV(hsvTestStep * 16, 255, 255);
            ledDriver->show();
            hsvTestStep++;
            return false;  // 测试未完成
        } else {
            // 测试完成，清除LED
            ledDriver->clear();
            ledDriver->show();
            LOG_TAG_INFO("Test", "HSV颜色测试完成");
            hsvTestStep = 0;  // 重置步骤
            return true;
        }
    }
    
    return false;  // 测试未完成
}

bool WS2812Test::testBrightnessControl() {
    unsigned long currentTime = millis();
    
    // 每100ms更新一次亮度
    if (currentTime - brightnessTestLastUpdate >= 100) {
        brightnessTestLastUpdate = currentTime;
        
        // 设置红色
        ledDriver->setAllColor(255, 0, 0);
        
        if (brightnessTestDirection) {
            // 逐步增加亮度
            if (brightnessTestStep <= 17) {  // 255/15 = 17
                ledDriver->setBrightness(brightnessTestStep * 15);
                ledDriver->show();
                brightnessTestStep++;
                return false;  // 测试未完成
            } else {
                // 开始降低亮度
                brightnessTestDirection = false;
                brightnessTestStep = 17;
            }
        }
        
        if (!brightnessTestDirection) {
            // 逐步降低亮度
            if (brightnessTestStep >= 0) {
                ledDriver->setBrightness(brightnessTestStep * 15);
                ledDriver->show();
                brightnessTestStep--;
                return false;  // 测试未完成
            } else {
                // 测试完成，清除LED
                ledDriver->clear();
                ledDriver->show();
                LOG_TAG_INFO("Test", "亮度控制测试完成");
                // 重置状态
                brightnessTestStep = 0;
                brightnessTestDirection = true;
                return true;
            }
        }
    }
    
    return false;  // 测试未完成
}

void WS2812Test::testAnimations() {
    unsigned long currentTime = millis();
    
    // 每200ms更新一次动画
    if (currentTime - animationTestLastUpdate >= 200) {
        animationTestLastUpdate = currentTime;
        
        if (animationTestCycle < 3) {
            // 简单的流水灯效果
            if (animationTestStep < 1) {  // 假设只有1个LED
                ledDriver->clear();
                ledDriver->setColor(animationTestStep, 0, 255, 0);  // 绿色
                ledDriver->show();
                animationTestStep++;
            } else {
                animationTestStep = 0;
                animationTestCycle++;
            }
        } else {
            // 动画测试完成，清除LED
            ledDriver->clear();
            ledDriver->show();
            LOG_TAG_INFO("Test", "动画效果测试完成");
            // 重置状态
            animationTestStep = 0;
            animationTestCycle = 0;
        }
    }
}

void WS2812Test::showSystemStatus() {
    LOG_TAG_INFO("System", "系统运行时间: %lu秒", millis()/1000);
    LOG_TAG_INFO("System", "WS2812 LED测试进行中...");
}