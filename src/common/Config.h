#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================
// 硬件引脚定义
// ==========================

// 电机控制引脚
#define MOTOR_PIN 7 // GPIO 7 - 电机控制输出

// WS2812 RGB LED引脚
#define LED_PIN 21 // GPIO 21 - WS2812数据输入

// ==========================
// 系统常量定义
// ==========================

// 定时器配置
#define TIMER_INTERVAL_MS 1 // 1ms定时器间隔
#define TIMER_PRESCALER 80  // 80MHz时钟，80分频得到1MHz

// LED配置
#define LED_COUNT 1       // WS2812 LED数量
#define LED_BRIGHTNESS 50 // LED亮度 (0-255)

// 电机控制配置
#define MOTOR_ON LOW   // 电机开启电平（低电平启动）
#define MOTOR_OFF HIGH // 电机关闭电平（默认高电平）

// 日志配置
#define LOG_BUFFER_SIZE 512              // 日志缓冲区大小
#define LOG_DEFAULT_LEVEL LogLevel::NONE // 默认日志级别
#define LOG_ENABLE_COLORS false          // 是否启用颜色输出（串口监视器通常不支持）
#define LOG_SHOW_TIMESTAMP true          // 是否显示时间戳
#define LOG_SHOW_MILLISECONDS true       // 时间戳是否包含毫秒
#define LOG_SHOW_LEVEL true              // 是否显示日志级别
#define LOG_SHOW_TAG true                // 是否显示标签

// BLE配置
#define BLE_DEVICE_NAME "ESP32-Motor-Control"
// BLE UUID定义 - 与需求文档保持一致
#define BLE_SERVICE_UUID "beb5483e-36e1-4688-b7f5-ea07361b26a8"
#define BLE_RUN_DURATION_CHAR_UUID "2f7a9c2e-6b1a-4b5e-8b2a-c1c2c3c4c5c6"
#define BLE_STOP_INTERVAL_CHAR_UUID "3f8a9c2e-6b1a-4b5e-8b2a-c1c2c3c4c5c7"
#define BLE_SYSTEM_CONTROL_CHAR_UUID "4f9a9c2e-6b1a-4b5e-8b2a-c1c2c3c4c5c8"
#define BLE_STATUS_QUERY_CHAR_UUID "5f9a9c2e-6b1a-4b5e-8b2a-c1c2c3c4c5c9"
#define BLE_SPEED_CONTROLLER_CONFIG_CHAR_UUID "6f9a9c2e-6b1a-4b5e-8b2a-c1c2c3c4c5ca"

// 配置参数结构体
struct MotorConfig
{
    uint32_t runDuration;  // 运行时长 (秒)
    uint32_t stopDuration; // 停止时长 (秒)
    uint32_t cycleCount;   // 循环次数 (0表示无限循环)
    bool autoStart;        // 是否自动启动

    // 构造函数
    MotorConfig() : runDuration(5),  // 默认5秒
                    stopDuration(2), // 默认2秒
                    cycleCount(0),   // 默认无限循环
                    autoStart(true)  // 默认自动启动
    {
    }
};

#endif // CONFIG_H