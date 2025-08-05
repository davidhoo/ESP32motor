#ifndef TIMER_DRIVER_H
#define TIMER_DRIVER_H

#include <Arduino.h>
#include <functional>
#include "../common/Logger.h"

// 定时器预分频值，80MHz / 80 = 1MHz
#define TIMER_PRESCALER 80

/**
 * 定时器驱动类
 * 提供1ms精度的硬件定时器功能，支持回调注册和管理
 */
class TimerDriver {
public:
    /**
     * 定时器回调函数类型
     */
    typedef std::function<void()> TimerCallback;
    
    /**
     * 定时器ID枚举
     */
    enum TimerID {
        TIMER_0 = 0,
        TIMER_1 = 1,
        TIMER_2 = 2,
        TIMER_3 = 3,
        MAX_TIMERS = 4
    };
    
    /**
     * 构造函数
     */
    TimerDriver();
    
    /**
     * 析构函数
     */
    ~TimerDriver();
    
    /**
     * 初始化定时器驱动
     * @return 初始化是否成功
     */
    bool init();
    
    /**
     * 创建定时器
     * @param timer_id 定时器ID
     * @param interval_ms 定时器间隔(毫秒)
     * @param callback 回调函数
     * @param auto_reload 是否自动重载
     * @return 创建是否成功
     */
    bool createTimer(TimerID timer_id, uint32_t interval_ms, 
                    TimerCallback callback, bool auto_reload = true);
    
    /**
     * 启动定时器
     * @param timer_id 定时器ID
     * @return 启动是否成功
     */
    bool startTimer(TimerID timer_id);
    
    /**
     * 停止定时器
     * @param timer_id 定时器ID
     * @return 停止是否成功
     */
    bool stopTimer(TimerID timer_id);
    
    /**
     * 重启定时器
     * @param timer_id 定时器ID
     * @return 重启是否成功
     */
    bool restartTimer(TimerID timer_id);
    
    /**
     * 删除定时器
     * @param timer_id 定时器ID
     * @return 删除是否成功
     */
    bool deleteTimer(TimerID timer_id);
    
    /**
     * 修改定时器间隔
     * @param timer_id 定时器ID
     * @param new_interval_ms 新的间隔时间(毫秒)
     * @return 修改是否成功
     */
    bool changeTimerInterval(TimerID timer_id, uint32_t new_interval_ms);
    
    /**
     * 检查定时器是否运行中
     * @param timer_id 定时器ID
     * @return 是否运行中
     */
    bool isTimerRunning(TimerID timer_id);
    
    /**
     * 获取定时器间隔
     * @param timer_id 定时器ID
     * @return 定时器间隔(毫秒)，失败返回0
     */
    uint32_t getTimerInterval(TimerID timer_id);
    
    /**
     * 获取定时器触发次数
     * @param timer_id 定时器ID
     * @return 触发次数
     */
    uint32_t getTimerTriggerCount(TimerID timer_id);
    
    /**
     * 重置定时器触发次数
     * @param timer_id 定时器ID
     * @return 重置是否成功
     */
    bool resetTimerTriggerCount(TimerID timer_id);
    
    /**
     * 获取系统运行时间(毫秒)
     * @return 系统运行时间
     */
    uint32_t getSystemUptime();
    
    /**
     * 延时函数(毫秒)
     * @param delay_ms 延时时间(毫秒)
     */
    void delayMs(uint32_t delay_ms);
    
    /**
     * 延时函数(微秒)
     * @param delay_us 延时时间(微秒)
     */
    void delayUs(uint32_t delay_us);
    
    /**
     * 获取单例实例
     * @return TimerDriver实例引用
     */
    static TimerDriver& getInstance();

private:
    /**
     * 定时器信息结构体
     */
    struct TimerInfo {
        hw_timer_t* timer;          // 硬件定时器句柄
        TimerCallback callback;     // 回调函数
        uint32_t interval_ms;       // 间隔时间(毫秒)
        uint32_t trigger_count;     // 触发次数
        bool is_created;            // 是否已创建
        bool is_running;            // 是否运行中
        bool auto_reload;           // 是否自动重载
    };
    
    TimerInfo timer_info[MAX_TIMERS];  // 定时器信息数组
    bool is_initialized;               // 是否已初始化
    uint32_t system_start_time;        // 系统启动时间
    
    /**
     * 验证定时器ID是否有效
     * @param timer_id 定时器ID
     * @return 是否有效
     */
    bool isValidTimerID(TimerID timer_id);
    
    /**
     * 验证定时器间隔是否有效
     * @param interval_ms 间隔时间(毫秒)
     * @return 是否有效
     */
    bool isValidInterval(uint32_t interval_ms);
    
    /**
     * 定时器中断服务程序包装器
     * @param timer_id 定时器ID
     */
    static void IRAM_ATTR timerISR(TimerID timer_id);
    
    /**
     * 定时器0中断服务程序
     */
    static void IRAM_ATTR timer0ISR();
    
    /**
     * 定时器1中断服务程序
     */
    static void IRAM_ATTR timer1ISR();
    
    /**
     * 定时器2中断服务程序
     */
    static void IRAM_ATTR timer2ISR();
    
    /**
     * 定时器3中断服务程序
     */
    static void IRAM_ATTR timer3ISR();
    
    /**
     * 处理定时器中断
     * @param timer_id 定时器ID
     */
    void handleTimerInterrupt(TimerID timer_id);
    
    // 静态实例指针
    static TimerDriver* instance;
};

#endif // TIMER_DRIVER_H