#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

#include "../drivers/WS2812Driver.h"
#include "../drivers/TimerDriver.h"
#include "../common/Logger.h"

/**
 * @brief LED状态枚举
 * 定义系统6种不同的LED指示状态
 */
enum class LEDState {
    SYSTEM_INIT,        // 系统初始化 - 蓝色闪烁
    MOTOR_RUNNING,      // 电机运行中 - 绿色常亮
    MOTOR_STOPPED,      // 电机停止 - 红色常亮
    BLE_CONNECTED,      // BLE已连接 - 青色常亮
    BLE_DISCONNECTED,   // BLE未连接 - 黄色闪烁
    ERROR_STATE         // 错误状态 - 紫色快速闪烁
};

/**
 * @brief LED控制器类
 * 管理WS2812 LED的状态指示和闪烁效果
 */
class LEDController {
private:
    WS2812Driver* ws2812;           // WS2812驱动指针
    TimerDriver& timer;             // 定时器驱动引用
    LEDState currentState;          // 当前LED状态
    bool isBlinking;                // 是否正在闪烁
    bool ledOn;                     // LED当前开关状态
    uint8_t blinkCount;             // 闪烁计数器
    uint8_t maxBlinkCount;          // 最大闪烁次数(0表示无限)
    
    // 颜色定义
    static const uint8_t COLOR_BLUE[3];      // 蓝色
    static const uint8_t COLOR_GREEN[3];     // 绿色
    static const uint8_t COLOR_RED[3];       // 红色
    static const uint8_t COLOR_CYAN[3];      // 青色
    static const uint8_t COLOR_YELLOW[3];    // 黄色
    static const uint8_t COLOR_PURPLE[3];    // 紫色
    static const uint8_t COLOR_OFF[3];       // 关闭

public:
    /**
     * @brief 构造函数
     */
    LEDController();
    
    /**
     * @brief 析构函数
     */
    ~LEDController();
    
    /**
     * @brief 初始化LED控制器
     * @return 初始化是否成功
     */
    bool init();
    
    /**
     * @brief 设置LED状态
     * @param state LED状态
     * @param blinkCount 闪烁次数(0表示无限闪烁)
     */
    void setState(LEDState state, uint8_t blinkCount = 0);
    
    /**
     * @brief 获取当前LED状态
     * @return 当前LED状态
     */
    LEDState getCurrentState() const;
    
    /**
     * @brief 更新LED状态(需要在主循环中调用)
     */
    void update();
    
    /**
     * @brief 停止所有LED效果
     */
    void stop();
    
    /**
     * @brief 测试LED(循环显示所有颜色)
     */
    void testLED();
    
    /**
     * @brief 检查是否正在闪烁
     * @return 是否正在闪烁
     */
    bool isCurrentlyBlinking() const;
    
    /**
     * @brief 获取当前闪烁计数
     * @return 闪烁计数
     */
    uint8_t getBlinkCount() const;
    
    /**
     * @brief 获取最大闪烁次数
     * @return 最大闪烁次数
     */
    uint8_t getMaxBlinkCount() const;

private:
    /**
     * @brief 根据状态获取对应颜色
     * @param state LED状态
     * @return 颜色数组指针(RGB)
     */
    const uint8_t* getColorForState(LEDState state);
    
    /**
     * @brief 根据状态获取闪烁间隔
     * @param state LED状态
     * @return 闪烁间隔(毫秒)
     */
    uint32_t getBlinkIntervalForState(LEDState state);
    
    /**
     * @brief 设置LED颜色
     * @param color RGB颜色数组
     */
    void setLEDColor(const uint8_t color[3]);
    
    /**
     * @brief 闪烁定时器回调函数
     */
    void blinkCallback();
    
    /**
     * @brief 清除LED显示
     */
    void clearLED();
};

#endif // LED_CONTROLLER_H