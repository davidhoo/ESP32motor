#ifndef POWER_MANAGER_H
#define POWER_MANAGER_H

#include <Arduino.h>
#include <esp_sleep.h>
#include <esp_bt.h>
#include <esp_wifi.h>
#include <esp_pm.h>
#include <driver/adc.h>

/**
 * ESP32 低功耗管理类
 * 提供BLE低功耗优化、温度监控和自动降频功能
 */
class PowerManager {
public:
    /**
     * 启用低功耗模式
     */
    static void enableLowPowerMode();
    
    /**
     * 配置BLE低功耗参数（简化版本，直接设置为低功耗模式）
     */
    static void configureBLELowPower();
    
    /**
     * 进入深度睡眠
     * @param sleepTimeMs 睡眠时间（毫秒）
     */
    static void enterDeepSleep(uint32_t sleepTimeMs);

private:
    static bool lowPowerModeEnabled;
};

#endif // POWER_MANAGER_H