#include "LEDControllerTest.h"
#include "../common/Logger.h"

void LEDControllerTest::runAllTests() {
    LOG_TAG_INFO("LEDControllerTest", "开始LED控制器测试...");
    
    testInit();
    testSetState();
    testBlinking();  // 重新启用闪烁测试
    testStop();
    testGetCurrentState();
    testBoundaryConditions();
    
    LOG_TAG_INFO("LEDControllerTest", "所有测试完成！");
}

void LEDControllerTest::testInit() {
    LOG_TAG_INFO("LEDControllerTest", "测试初始化...");
    
    LEDController ledController;
    bool result = ledController.init();
    
    assertTrue(result, "LED控制器初始化应该成功");
    
    LOG_TAG_INFO("LEDControllerTest", "初始化测试通过");
}

void LEDControllerTest::testSetState() {
    LOG_TAG_INFO("LEDControllerTest", "测试状态设置...");
    
    LEDController ledController;
    ledController.init();
    
    // 测试常亮状态
    ledController.setState(LEDState::MOTOR_RUNNING);
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_RUNNING, 
               "应该设置为MOTOR_RUNNING状态");
    
    ledController.setState(LEDState::MOTOR_STOPPED);
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_STOPPED, 
               "应该设置为MOTOR_STOPPED状态");
    
    ledController.setState(LEDState::BLE_CONNECTED);
    assertTrue(ledController.getCurrentState() == LEDState::BLE_CONNECTED, 
               "应该设置为BLE_CONNECTED状态");
    
    LOG_TAG_INFO("LEDControllerTest", "状态设置测试通过");
}

void LEDControllerTest::testBlinking() {
    LOG_TAG_INFO("LEDControllerTest", "测试闪烁效果...");
    
    LEDController ledController;
    ledController.init();
    
    // 测试1：有限次数闪烁
    LOG_TAG_DEBUG("LEDControllerTest", "测试有限次数闪烁(3次)...");
    ledController.setState(LEDState::SYSTEM_INIT, 3);
    
    // 验证闪烁状态
    assertTrue(ledController.isCurrentlyBlinking(), "应该开始闪烁");
    assertTrue(ledController.getMaxBlinkCount() == 3, "最大闪烁次数应该是3");
    assertTrue(ledController.getCurrentState() == LEDState::SYSTEM_INIT, "状态应该是SYSTEM_INIT");
    
    // 等待闪烁完成 (3次闪烁 * 2个状态切换 * 500ms间隔 + 缓冲时间)
    uint32_t waitTime = 3 * 2 * 500 + 1000;  // 4秒
    uint32_t startTime = millis();
    
    while (ledController.isCurrentlyBlinking() && (millis() - startTime) < waitTime) {
        ledController.update();  // 处理闪烁逻辑
        delay(50);  // 短暂延迟
    }
    
    // 验证闪烁已停止
    assertTrue(!ledController.isCurrentlyBlinking(), "有限次数闪烁应该已停止");
    assertTrue(ledController.getBlinkCount() >= 6, "应该完成6次状态切换(3次闪烁)");
    
    // 测试2：无限闪烁
    LOG_TAG_DEBUG("LEDControllerTest", "测试无限闪烁...");
    ledController.setState(LEDState::ERROR_STATE);
    
    // 验证无限闪烁状态
    assertTrue(ledController.isCurrentlyBlinking(), "应该开始无限闪烁");
    assertTrue(ledController.getMaxBlinkCount() == 0, "最大闪烁次数应该是0(无限)");
    assertTrue(ledController.getCurrentState() == LEDState::ERROR_STATE, "状态应该是ERROR_STATE");
    
    // 等待一段时间验证持续闪烁
    startTime = millis();
    uint8_t initialBlinkCount = ledController.getBlinkCount();
    
    while ((millis() - startTime) < 1000) {  // 等待1秒
        ledController.update();
        delay(50);
    }
    
    // 验证闪烁仍在继续
    assertTrue(ledController.isCurrentlyBlinking(), "无限闪烁应该仍在继续");
    assertTrue(ledController.getBlinkCount() > initialBlinkCount, "闪烁计数应该增加");
    
    // 停止闪烁
    ledController.stop();
    assertTrue(!ledController.isCurrentlyBlinking(), "停止后应该不再闪烁");
    
    // 测试3：常亮状态（不闪烁）
    LOG_TAG_DEBUG("LEDControllerTest", "测试常亮状态...");
    ledController.setState(LEDState::MOTOR_RUNNING);
    
    assertTrue(!ledController.isCurrentlyBlinking(), "常亮状态不应该闪烁");
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_RUNNING, "状态应该是MOTOR_RUNNING");
    
    delay(500);  // 等待一段时间
    assertTrue(!ledController.isCurrentlyBlinking(), "常亮状态应该保持不闪烁");
    
    LOG_TAG_INFO("LEDControllerTest", "闪烁测试通过");
}

void LEDControllerTest::testStop() {
    LOG_TAG_INFO("LEDControllerTest", "测试停止功能...");
    
    LEDController ledController;
    ledController.init();
    
    // 设置一个状态
    ledController.setState(LEDState::MOTOR_RUNNING);
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_RUNNING, 
               "应该成功设置状态");
    
    // 停止所有效果
    ledController.stop();
    assertTrue(ledController.getCurrentState() == LEDState::SYSTEM_INIT, 
               "停止后应该回到SYSTEM_INIT状态");
    
    LOG_TAG_INFO("LEDControllerTest", "停止功能测试通过");
}

void LEDControllerTest::testGetCurrentState() {
    LOG_TAG_INFO("LEDControllerTest", "测试状态获取...");
    
    LEDController ledController;
    
    // 初始状态应该是SYSTEM_INIT
    assertTrue(ledController.getCurrentState() == LEDState::SYSTEM_INIT, 
               "初始状态应该是SYSTEM_INIT");
    
    ledController.init();
    ledController.setState(LEDState::MOTOR_RUNNING);
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_RUNNING, 
               "应该正确获取当前状态");
    
    LOG_TAG_INFO("LEDControllerTest", "状态获取测试通过");
}

void LEDControllerTest::testBoundaryConditions() {
    LOG_TAG_INFO("LEDControllerTest", "测试边界条件...");
    
    LEDController ledController;
    ledController.init();
    
    // 测试大量闪烁次数
    ledController.setState(LEDState::SYSTEM_INIT, 255);
    assertTrue(ledController.getCurrentState() == LEDState::SYSTEM_INIT, 
               "应该处理大量闪烁次数");
    
    // 测试0次闪烁（常亮）
    ledController.setState(LEDState::MOTOR_RUNNING, 0);
    assertTrue(ledController.getCurrentState() == LEDState::MOTOR_RUNNING, 
               "应该处理0次闪烁");
    
    LOG_TAG_INFO("LEDControllerTest", "边界条件测试通过");
}

void LEDControllerTest::assertTrue(bool condition, const char* message) {
    if (!condition) {
        LOG_TAG_ERROR("LEDControllerTest", "断言失败: %s", message);
    } else {
        LOG_TAG_DEBUG("LEDControllerTest", "断言通过: %s", message);
    }
}

void LEDControllerTest::delay(uint32_t ms) {
    // 使用Arduino标准延迟函数，避免在定时器中断上下文中死锁
    ::delay(ms);
}