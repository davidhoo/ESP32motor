#ifndef GPIO_DRIVER_H
#define GPIO_DRIVER_H

#include <Arduino.h>
#include "../common/Logger.h"

/**
 * GPIO驱动类
 * 提供GPIO引脚的初始化、输出控制和状态读取功能
 */
class GPIODriver {
public:
    /**
     * 构造函数
     */
    GPIODriver();
    
    /**
     * 析构函数
     */
    ~GPIODriver();
    
    /**
     * 初始化GPIO引脚
     * @param pin GPIO引脚号
     * @param mode 引脚模式 (INPUT, OUTPUT, INPUT_PULLUP, INPUT_PULLDOWN)
     * @param initial_state 初始状态 (仅对OUTPUT模式有效)
     * @return 初始化是否成功
     */
    bool init(uint8_t pin, uint8_t mode, uint8_t initial_state = LOW);
    
    /**
     * 设置GPIO输出电平
     * @param pin GPIO引脚号
     * @param state 输出电平 (HIGH/LOW)
     * @return 操作是否成功
     */
    bool digitalWrite(uint8_t pin, uint8_t state);
    
    /**
     * 读取GPIO输入电平
     * @param pin GPIO引脚号
     * @return 引脚电平状态 (HIGH/LOW)，失败返回-1
     */
    int digitalRead(uint8_t pin);
    
    /**
     * 切换GPIO输出电平
     * @param pin GPIO引脚号
     * @return 操作是否成功
     */
    bool togglePin(uint8_t pin);
    
    /**
     * 检查引脚是否已初始化
     * @param pin GPIO引脚号
     * @return 是否已初始化
     */
    bool isPinInitialized(uint8_t pin);
    
    /**
     * 获取引脚当前模式
     * @param pin GPIO引脚号
     * @return 引脚模式，未初始化返回-1
     */
    int getPinMode(uint8_t pin);
    
    /**
     * 重置引脚配置
     * @param pin GPIO引脚号
     * @return 操作是否成功
     */
    bool resetPin(uint8_t pin);
    
    /**
     * 批量初始化多个GPIO引脚
     * @param pins 引脚数组
     * @param modes 模式数组
     * @param initial_states 初始状态数组
     * @param count 引脚数量
     * @return 成功初始化的引脚数量
     */
    int initMultiplePins(const uint8_t* pins, const uint8_t* modes, 
                        const uint8_t* initial_states, uint8_t count);

private:
    static const uint8_t MAX_PINS = 48;  // ESP32-S3最大GPIO数量
    
    struct PinInfo {
        bool initialized;
        uint8_t mode;
        uint8_t last_state;
    };
    
    PinInfo pin_info[MAX_PINS];  // 引脚信息数组
    
    /**
     * 验证引脚号是否有效
     * @param pin GPIO引脚号
     * @return 是否有效
     */
    bool isValidPin(uint8_t pin);
    
    /**
     * 验证引脚模式是否有效
     * @param mode 引脚模式
     * @return 是否有效
     */
    bool isValidMode(uint8_t mode);
};

#endif // GPIO_DRIVER_H