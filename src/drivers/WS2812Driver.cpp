#include "WS2812Driver.h"
#include <algorithm>
#include <driver/rmt.h>
#include <esp_log.h>

static const char *TAG = "WS2812Driver";

WS2812Driver::WS2812Driver(uint8_t pin, uint16_t ledCount)
    : pin(pin), ledCount(ledCount), brightness(255) {
    // 为LED数据分配内存 (每个LED需要3个字节：GRB格式)
    ledData = new uint8_t[ledCount * 3];
    // 初始化所有LED为黑色
    clear();
}

WS2812Driver::~WS2812Driver() {
    // 释放LED数据内存
    delete[] ledData;
}

void WS2812Driver::begin() {
    // 配置RMT
    rmt_config_t config = RMT_DEFAULT_CONFIG_TX(static_cast<gpio_num_t>(pin), RMT_CHANNEL_0);
    config.clk_div = 2;  // 80MHz/2 = 40MHz
    
    // WS2812时序要求：
    // 0码：高电平0.4μs，低电平0.85μs
    // 1码：高电平0.8μs，低电平0.45μs
    // 周期：1.25μs±600ns
    
    // 以40MHz时钟计算（25ns/tick）：
    // 0码：高电平16 ticks，低电平34 ticks
    // 1码：高电平32 ticks，低电平18 ticks
    config.tx_config.carrier_en = false;
    config.tx_config.loop_en = false;
    config.tx_config.idle_output_en = true;
    config.tx_config.idle_level = RMT_IDLE_LEVEL_LOW;
    
    // 初始化RMT
    rmt_config(&config);
    rmt_driver_install(config.channel, 0, 0);
}

void WS2812Driver::setColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b) {
    // 检查索引是否有效
    if (index >= ledCount) {
        return;
    }
    
    // 确保颜色值在有效范围内
    r = std::min(r, static_cast<uint8_t>(255));
    g = std::min(g, static_cast<uint8_t>(255));
    b = std::min(b, static_cast<uint8_t>(255));
    
    // 应用亮度调整
    if (brightness < 255) {
        r = (r * brightness) >> 8;
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;
    }
    
    // WS2812使用GRB格式
    ledData[index * 3] = g;      // 绿色
    ledData[index * 3 + 1] = r;  // 红色
    ledData[index * 3 + 2] = b;  // 蓝色
}

void WS2812Driver::setColorHSV(uint16_t index, uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r, g, b;
    hsvToRgb(h, s, v, r, g, b);
    setColor(index, r, g, b);
}

void WS2812Driver::setAllColor(uint8_t r, uint8_t g, uint8_t b) {
    // 确保颜色值在有效范围内
    r = std::min(r, static_cast<uint8_t>(255));
    g = std::min(g, static_cast<uint8_t>(255));
    b = std::min(b, static_cast<uint8_t>(255));
    
    // 应用亮度调整
    if (brightness < 255) {
        r = (r * brightness) >> 8;
        g = (g * brightness) >> 8;
        b = (b * brightness) >> 8;
    }
    
    // 为所有LED设置颜色
    for (uint16_t i = 0; i < ledCount; i++) {
        // WS2812使用GRB格式
        ledData[i * 3] = g;      // 绿色
        ledData[i * 3 + 1] = r;  // 红色
        ledData[i * 3 + 2] = b;  // 蓝色
    }
}

void WS2812Driver::setAllColorHSV(uint8_t h, uint8_t s, uint8_t v) {
    uint8_t r, g, b;
    hsvToRgb(h, s, v, r, g, b);
    setAllColor(r, g, b);
}

void WS2812Driver::setBrightness(uint8_t brightness) {
    // 确保亮度值在有效范围内
    this->brightness = std::min(brightness, static_cast<uint8_t>(255));
}

void WS2812Driver::show() {
    // 将ledData转换为RMT格式并发送
    rmt_item32_t* items = new rmt_item32_t[ledCount * 24 + 1];  // 每个LED需要24位，额外1个用于结束信号
    
    int itemIndex = 0;
    for (uint16_t i = 0; i < ledCount; i++) {
        // 获取当前LED的GRB值
        uint8_t g = ledData[i * 3];
        uint8_t r = ledData[i * 3 + 1];
        uint8_t b = ledData[i * 3 + 2];
        
        // 发送绿色数据 (8位)
        for (int bit = 7; bit >= 0; bit--) {
            if (g & (1 << bit)) {
                // 发送1码
                items[itemIndex].duration0 = 32;  // 0.8μs高电平 (32 ticks at 40MHz)
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 18;  // 0.45μs低电平 (18 ticks at 40MHz)
                items[itemIndex].level1 = 0;
            } else {
                // 发送0码
                items[itemIndex].duration0 = 16;  // 0.4μs高电平 (16 ticks at 40MHz)
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 34;  // 0.85μs低电平 (34 ticks at 40MHz)
                items[itemIndex].level1 = 0;
            }
            itemIndex++;
        }
        
        // 发送红色数据 (8位)
        for (int bit = 7; bit >= 0; bit--) {
            if (r & (1 << bit)) {
                // 发送1码
                items[itemIndex].duration0 = 32;
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 18;
                items[itemIndex].level1 = 0;
            } else {
                // 发送0码
                items[itemIndex].duration0 = 16;
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 34;
                items[itemIndex].level1 = 0;
            }
            itemIndex++;
        }
        
        // 发送蓝色数据 (8位)
        for (int bit = 7; bit >= 0; bit--) {
            if (b & (1 << bit)) {
                // 发送1码
                items[itemIndex].duration0 = 32;
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 18;
                items[itemIndex].level1 = 0;
            } else {
                // 发送0码
                items[itemIndex].duration0 = 16;
                items[itemIndex].level0 = 1;
                items[itemIndex].duration1 = 34;
                items[itemIndex].level1 = 0;
            }
            itemIndex++;
        }
    }
    
    // 添加结束信号 (至少50μs低电平)
    items[itemIndex].duration0 = 2000;  // 50μs低电平 (2000 ticks at 40MHz)
    items[itemIndex].level0 = 0;
    items[itemIndex].duration1 = 0;
    items[itemIndex].level1 = 0;
    itemIndex++;
    
    // 通过RMT发送数据
    rmt_write_items(RMT_CHANNEL_0, items, itemIndex, true);
    
    // 释放内存
    delete[] items;
}

void WS2812Driver::clear() {
    // 将所有LED设置为黑色
    for (uint16_t i = 0; i < ledCount * 3; i++) {
        ledData[i] = 0;
    }
}

void WS2812Driver::hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b) {
    // 将HSV值转换为RGB值
    if (s == 0) {
        // 灰度色
        r = g = b = v;
        return;
    }
    
    // 将h转换为0-191范围（6个区域）
    uint8_t region = (h * 6) / 255;
    uint8_t remainder = (h * 2) % 255;
    
    uint8_t p = (v * (255 - s)) / 255;
    uint8_t q = (v * (255 - (s * remainder) / 255)) / 255;
    uint8_t t = (v * (255 - (s * (255 - remainder)) / 255)) / 255;
    
    switch (region) {
        case 0:
            r = v; g = t; b = p;
            break;
        case 1:
            r = q; g = v; b = p;
            break;
        case 2:
            r = p; g = v; b = t;
            break;
        case 3:
            r = p; g = q; b = v;
            break;
        case 4:
            r = t; g = p; b = v;
            break;
        default:
            r = v; g = p; b = q;
            break;
    }
}