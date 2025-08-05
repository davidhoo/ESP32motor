#include "LEDController.h"
#include "../common/Logger.h"

// 定义颜色常量
const uint8_t LEDController::COLOR_BLUE[3] = {0, 0, 255};
const uint8_t LEDController::COLOR_GREEN[3] = {0, 255, 0};
const uint8_t LEDController::COLOR_RED[3] = {255, 0, 0};
const uint8_t LEDController::COLOR_CYAN[3] = {0, 255, 255};
const uint8_t LEDController::COLOR_YELLOW[3] = {255, 255, 0};
const uint8_t LEDController::COLOR_PURPLE[3] = {255, 0, 255};
const uint8_t LEDController::COLOR_OFF[3] = {0, 0, 0};

LEDController::LEDController() 
    : ws2812(nullptr)
    , timer(TimerDriver::getInstance())
    , currentState(LEDState::SYSTEM_INIT)
    , isBlinking(false)
    , ledOn(false)
    , blinkCount(0)
    , maxBlinkCount(0) {
    ws2812 = new WS2812Driver(21, 1);  // GPIO 21, 1个LED
}

LEDController::~LEDController() {
    stop();
    if (ws2812 != nullptr) {
        delete ws2812;
        ws2812 = nullptr;
    }
}

bool LEDController::init() {
    LOG_TAG_INFO("LEDController", "初始化LED控制器...");
    
    if (ws2812 == nullptr) {
        LOG_TAG_ERROR("LEDController", "WS2812驱动未初始化");
        return false;
    }
    
    // 初始化WS2812驱动
    ws2812->begin();
    ws2812->setBrightness(50);  // 设置亮度为50%
    
    // 清除LED显示
    clearLED();
    
    LOG_TAG_INFO("LEDController", "LED控制器初始化完成");
    return true;
}

void LEDController::setState(LEDState state, uint8_t blinkCount) {
    if (currentState == state && isBlinking == (blinkCount > 0)) {
        return;  // 状态未改变，无需操作
    }
    
    LOG_TAG_DEBUG("LEDController", "设置LED状态: %d, 闪烁次数: %d", static_cast<int>(state), blinkCount);
    
    // 停止之前的闪烁
    if (isBlinking) {
        timer.stopTimer(TimerDriver::TIMER_0);
        isBlinking = false;
    }
    
    currentState = state;
    this->blinkCount = 0;
    this->maxBlinkCount = blinkCount;
    ledOn = true;
    
    const uint8_t* color = getColorForState(state);
    
    if (blinkCount > 0 || state == LEDState::SYSTEM_INIT ||
        state == LEDState::BLE_DISCONNECTED || state == LEDState::ERROR_STATE) {
        // 需要闪烁的状态
        isBlinking = true;
        uint32_t interval = getBlinkIntervalForState(state);
        
        // 创建闪烁定时器
        timer.createTimer(TimerDriver::TIMER_0, interval,
            [this]() { this->blinkCallback(); }, true);
        timer.startTimer(TimerDriver::TIMER_0);
        
        // 初始颜色将在update()中设置
    } else {
        // 常亮状态
        isBlinking = false;
        setLEDColor(color);
    }
}

LEDState LEDController::getCurrentState() const {
    return currentState;
}

void LEDController::update() {
    // 在主循环中调用，处理LED更新和闪烁停止逻辑
    if (isBlinking) {
        // 根据当前状态更新LED显示
        if (ledOn) {
            const uint8_t* color = getColorForState(currentState);
            setLEDColor(color);
        } else {
            clearLED();
        }
    }
    
    // 处理闪烁完成逻辑
    if (!isBlinking && maxBlinkCount > 0 && blinkCount >= maxBlinkCount * 2) {
        // 闪烁已完成，停止定时器并清除LED
        timer.stopTimer(TimerDriver::TIMER_0);
        clearLED();
        blinkCount = 0;
        maxBlinkCount = 0;
    }
}

void LEDController::stop() {
    if (isBlinking) {
        timer.stopTimer(TimerDriver::TIMER_0);
        timer.deleteTimer(TimerDriver::TIMER_0);
        isBlinking = false;
    }
    
    clearLED();
    currentState = LEDState::SYSTEM_INIT;
    blinkCount = 0;
    maxBlinkCount = 0;
    ledOn = false;
}

void LEDController::testLED() {
    LOG_TAG_INFO("LEDController", "开始LED测试...");
    
    // 测试所有颜色
    const uint8_t* colors[] = {
        COLOR_BLUE, COLOR_GREEN, COLOR_RED, 
        COLOR_CYAN, COLOR_YELLOW, COLOR_PURPLE
    };
    
    const char* colorNames[] = {
        "蓝色", "绿色", "红色", "青色", "黄色", "紫色"
    };
    
    for (int i = 0; i < 6; i++) {
        LOG_TAG_DEBUG("LEDController", "显示颜色: %s", colorNames[i]);
        setLEDColor(colors[i]);
        timer.delayMs(1000);
    }
    
    // 测试闪烁效果
    LOG_TAG_DEBUG("LEDController", "测试闪烁效果...");
    setState(LEDState::SYSTEM_INIT);
    timer.delayMs(3000);
    
    setState(LEDState::ERROR_STATE);
    timer.delayMs(3000);
    
    // 恢复初始状态
    setState(LEDState::SYSTEM_INIT);
    LOG_TAG_INFO("LEDController", "LED测试完成");
}

const uint8_t* LEDController::getColorForState(LEDState state) {
    switch (state) {
        case LEDState::SYSTEM_INIT:
            return COLOR_BLUE;
        case LEDState::MOTOR_RUNNING:
            return COLOR_GREEN;
        case LEDState::MOTOR_STOPPED:
            return COLOR_RED;
        case LEDState::BLE_CONNECTED:
            return COLOR_CYAN;
        case LEDState::BLE_DISCONNECTED:
            return COLOR_YELLOW;
        case LEDState::ERROR_STATE:
            return COLOR_PURPLE;
        default:
            return COLOR_OFF;
    }
}

uint32_t LEDController::getBlinkIntervalForState(LEDState state) {
    switch (state) {
        case LEDState::SYSTEM_INIT:
            return 500;  // 500ms闪烁
        case LEDState::BLE_DISCONNECTED:
            return 1000; // 1秒闪烁
        case LEDState::ERROR_STATE:
            return 200;  // 200ms快速闪烁
        default:
            return 500;  // 默认500ms
    }
}

void LEDController::setLEDColor(const uint8_t color[3]) {
    if (ws2812 != nullptr) {
        ws2812->setColor(0, color[0], color[1], color[2]);
        ws2812->show();
    }
}

void LEDController::blinkCallback() {
    if (!isBlinking) {
        return;
    }
    
    if (maxBlinkCount > 0 && blinkCount >= maxBlinkCount * 2) {
        // 达到最大闪烁次数，标记停止（在主循环中处理）
        isBlinking = false;
        ledOn = false;
        return;
    }
    
    // 在中断上下文中只切换状态，不直接操作硬件
    ledOn = !ledOn;
    blinkCount++;
    
    // 设置标志位，让主循环处理LED更新
    // 这样避免在中断上下文中调用可能阻塞的函数
}

void LEDController::clearLED() {
    setLEDColor(COLOR_OFF);
}

bool LEDController::isCurrentlyBlinking() const {
    return isBlinking;
}

uint8_t LEDController::getBlinkCount() const {
    return blinkCount;
}

uint8_t LEDController::getMaxBlinkCount() const {
    return maxBlinkCount;
}