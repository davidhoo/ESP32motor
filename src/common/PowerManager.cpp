#include "PowerManager.h"
#include "Logger.h"
#include <esp_bt.h>
#include <esp_bt_main.h>
#include <esp_gap_ble_api.h>
#include <esp_gatts_api.h>
#include <soc/rtc.h>

// 静态成员变量初始化
bool PowerManager::lowPowerModeEnabled = false;

void PowerManager::enableLowPowerMode() {
    if (lowPowerModeEnabled) {
        return;
    }
    
    LOG_INFO("启用ESP32低功耗模式...");
    
    // 1. 降低CPU频率到80MHz
    setCpuFrequencyMhz(80);
    
    // 2. 禁用WiFi（如果启用了）
    #ifdef CONFIG_ESP32_WIFI_ENABLED
    esp_wifi_stop();
    esp_wifi_deinit();
    LOG_INFO("WiFi已禁用以节省功耗");
    #endif
    
    // 3. 配置BLE低功耗参数
    configureBLELowPower();
    
    // 4. 配置睡眠模式
    esp_sleep_pd_config(ESP_PD_DOMAIN_MAX, ESP_PD_OPTION_OFF);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_PERIPH, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_SLOW_MEM, ESP_PD_OPTION_ON);
    esp_sleep_pd_config(ESP_PD_DOMAIN_RTC_FAST_MEM, ESP_PD_OPTION_ON);
    
    lowPowerModeEnabled = true;
    LOG_INFO("低功耗模式已启用 - CPU: 80MHz, BLE: -12dBm");
}

void PowerManager::configureBLELowPower() {
    LOG_INFO("配置BLE低功耗参数...");
    
    // 设置BLE发射功率为最低
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_DEFAULT, ESP_PWR_LVL_N12);  // -12dBm
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_ADV, ESP_PWR_LVL_N12);     // 广播功率
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_SCAN, ESP_PWR_LVL_N12);    // 扫描功率
    esp_ble_tx_power_set(ESP_BLE_PWR_TYPE_CONN_HDL0, ESP_PWR_LVL_N12); // 连接功率
    
    LOG_INFO("BLE低功耗配置完成 - 发射功率: -12dBm");
}


void PowerManager::enterDeepSleep(uint32_t sleepTimeMs) {
    LOG_INFO("进入深度睡眠 %lu 毫秒", sleepTimeMs);
    
    // 配置唤醒源
    esp_sleep_enable_timer_wakeup(sleepTimeMs * 1000ULL);
    
    // 进入深度睡眠
    esp_deep_sleep_start();
}