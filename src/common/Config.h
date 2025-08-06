#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>

// ==========================
// 硬件引脚定义
// ==========================

// 电机控制引脚
#define MOTOR_PIN 7           // GPIO 7 - 电机控制输出

// WS2812 RGB LED引脚
#define LED_PIN 21            // GPIO 21 - WS2812数据输入

// ==========================
// 系统常量定义
// ==========================

// 定时器配置
#define TIMER_INTERVAL_MS 1   // 1ms定时器间隔
#define TIMER_PRESCALER 80    // 80MHz时钟，80分频得到1MHz

// LED配置
#define LED_COUNT 1           // WS2812 LED数量
#define LED_BRIGHTNESS 50     // LED亮度 (0-255)

// 电机控制配置
#define MOTOR_ON HIGH         // 电机开启电平
#define MOTOR_OFF LOW         // 电机关闭电平

// 日志配置
#define LOG_BUFFER_SIZE 512       // 日志缓冲区大小
#define LOG_DEFAULT_LEVEL LogLevel::INFO  // 默认日志级别
#define LOG_ENABLE_COLORS false   // 是否启用颜色输出（串口监视器通常不支持）
#define LOG_SHOW_TIMESTAMP true   // 是否显示时间戳
#define LOG_SHOW_MILLISECONDS true // 时间戳是否包含毫秒
#define LOG_SHOW_LEVEL true       // 是否显示日志级别
#define LOG_SHOW_TAG true         // 是否显示标签

// BLE配置
#define BLE_DEVICE_NAME "ESP32-Motor-Control"
#define BLE_SERVICE_UUID "12345678-1234-1234-1234-123456789abc"
#define BLE_CHAR_CONFIG_UUID "87654321-4321-4321-4321-cba987654321"
#define BLE_CHAR_STATUS_UUID "abcdef12-3456-7890-abcd-ef1234567890"
#define BLE_CHAR_COMMAND_UUID "fedcba98-7654-3210-fedc-ba0987654321"
#define BLE_CHAR_INFO_UUID "11223344-5566-7788-99aa-bbccddeeff00"

// 电机状态枚举
enum class MotorState {
    STOPPED,        // 停止
    RUNNING,        // 运行
    PAUSED          // 暂停
};


// 配置参数结构体
struct MotorConfig {
    uint32_t runDuration;     // 运行时长 (毫秒)
    uint32_t stopDuration;    // 停止时长 (毫秒)
    uint32_t cycleCount;      // 循环次数 (0表示无限循环)
    bool autoStart;          // 是否自动启动
    
    // 构造函数
    MotorConfig() : 
        runDuration(5000),    // 默认5秒
        stopDuration(2000),   // 默认2秒
        cycleCount(0),        // 默认无限循环
        autoStart(true)       // 默认自动启动
    {}
};

// 系统信息结构体
struct SystemInfo {
    String firmwareVersion;
    String buildDate;
    String deviceId;
    
    // 构造函数
    SystemInfo() :
        firmwareVersion("1.0.0"),
        buildDate(__DATE__ " " __TIME__),
        deviceId("")
    {}
};

#endif // CONFIG_H