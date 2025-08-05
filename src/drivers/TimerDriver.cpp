#include "TimerDriver.h"

// 静态实例指针初始化
TimerDriver* TimerDriver::instance = nullptr;

TimerDriver::TimerDriver() : is_initialized(false), system_start_time(0) {
    // 初始化所有定时器信息
    for (int i = 0; i < MAX_TIMERS; i++) {
        timer_info[i].timer = nullptr;
        timer_info[i].callback = nullptr;
        timer_info[i].interval_ms = 0;
        timer_info[i].trigger_count = 0;
        timer_info[i].is_created = false;
        timer_info[i].is_running = false;
        timer_info[i].auto_reload = true;
    }
    
    Logger::getInstance().info("TimerDriver", "定时器驱动构造完成");
}

TimerDriver::~TimerDriver() {
    // 清理所有定时器
    for (int i = 0; i < MAX_TIMERS; i++) {
        if (timer_info[i].is_created) {
            deleteTimer(static_cast<TimerID>(i));
        }
    }
    
    Logger::getInstance().info("TimerDriver", "定时器驱动析构完成");
}

bool TimerDriver::init() {
    if (is_initialized) {
        Logger::getInstance().warn("TimerDriver", "定时器驱动已经初始化");
        return true;
    }
    
    try {
        // 记录系统启动时间
        system_start_time = millis();
        
        // 设置静态实例指针
        instance = this;
        
        is_initialized = true;
        Logger::getInstance().info("TimerDriver", "定时器驱动初始化成功");
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", "定时器驱动初始化失败");
        return false;
    }
}

bool TimerDriver::createTimer(TimerID timer_id, uint32_t interval_ms, 
                             TimerCallback callback, bool auto_reload) {
    // 验证参数
    if (!isValidTimerID(timer_id)) {
        Logger::getInstance().error("TimerDriver", ("无效的定时器ID: " + String(timer_id)).c_str());
        return false;
    }
    
    if (!isValidInterval(interval_ms)) {
        Logger::getInstance().error("TimerDriver", ("无效的定时器间隔: " + String(interval_ms) + "ms").c_str());
        return false;
    }
    
    if (!callback) {
        Logger::getInstance().error("TimerDriver", "定时器回调函数为空");
        return false;
    }
    
    if (!is_initialized) {
        Logger::getInstance().error("TimerDriver", "定时器驱动未初始化");
        return false;
    }
    
    // 检查定时器是否已存在
    if (timer_info[timer_id].is_created) {
        Logger::getInstance().warn("TimerDriver", ("定时器" + String(timer_id) + " 已存在，先删除").c_str());
        deleteTimer(timer_id);
    }
    
    try {
        // 创建硬件定时器
        timer_info[timer_id].timer = timerBegin(timer_id, TIMER_PRESCALER, true);
        if (!timer_info[timer_id].timer) {
            Logger::getInstance().error("TimerDriver", ("创建硬件定时器" + String(timer_id) + " 失败").c_str());
            return false;
        }
        
        // 设置定时器参数
        timer_info[timer_id].callback = callback;
        timer_info[timer_id].interval_ms = interval_ms;
        timer_info[timer_id].trigger_count = 0;
        timer_info[timer_id].is_created = true;
        timer_info[timer_id].is_running = false;
        timer_info[timer_id].auto_reload = auto_reload;
        
        // 计算定时器计数值 (1MHz时钟，1ms = 1000计数)
        uint64_t timer_count = interval_ms * 1000;
        timerAlarmWrite(timer_info[timer_id].timer, timer_count, auto_reload);
        
        // 绑定中断服务程序
        switch (timer_id) {
            case TIMER_0:
                timerAttachInterrupt(timer_info[timer_id].timer, &timer0ISR, true);
                break;
            case TIMER_1:
                timerAttachInterrupt(timer_info[timer_id].timer, &timer1ISR, true);
                break;
            case TIMER_2:
                timerAttachInterrupt(timer_info[timer_id].timer, &timer2ISR, true);
                break;
            case TIMER_3:
                timerAttachInterrupt(timer_info[timer_id].timer, &timer3ISR, true);
                break;
        }
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 创建成功，间隔: " + String(interval_ms) + "ms").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("创建定时器" + String(timer_id) + " 异常").c_str());
        return false;
    }
}

bool TimerDriver::startTimer(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        Logger::getInstance().error("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return false;
    }
    
    if (timer_info[timer_id].is_running) {
        Logger::getInstance().warn("TimerDriver", ("定时器" + String(timer_id) + " 已在运行").c_str());
        return true;
    }
    
    try {
        timerAlarmEnable(timer_info[timer_id].timer);
        timer_info[timer_id].is_running = true;
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 启动成功").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("启动定时器" + String(timer_id) + " 失败").c_str());
        return false;
    }
}

bool TimerDriver::stopTimer(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        Logger::getInstance().error("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return false;
    }
    
    if (!timer_info[timer_id].is_running) {
        Logger::getInstance().warn("TimerDriver", ("定时器" + String(timer_id) + " 已停止").c_str());
        return true;
    }
    
    try {
        timerAlarmDisable(timer_info[timer_id].timer);
        timer_info[timer_id].is_running = false;
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 停止成功").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("停止定时器" + String(timer_id) + " 失败").c_str());
        return false;
    }
}

bool TimerDriver::restartTimer(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        Logger::getInstance().error("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return false;
    }
    
    try {
        // 先停止定时器
        if (timer_info[timer_id].is_running) {
            timerAlarmDisable(timer_info[timer_id].timer);
        }
        
        // 重置定时器计数
        timerRestart(timer_info[timer_id].timer);
        
        // 重新启动
        timerAlarmEnable(timer_info[timer_id].timer);
        timer_info[timer_id].is_running = true;
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 重启成功").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("重启定时器" + String(timer_id) + " 失败").c_str());
        return false;
    }
}

bool TimerDriver::deleteTimer(TimerID timer_id) {
    if (!isValidTimerID(timer_id)) {
        Logger::getInstance().error("TimerDriver", ("无效的定时器ID: " + String(timer_id)).c_str());
        return false;
    }
    
    if (!timer_info[timer_id].is_created) {
        Logger::getInstance().warn("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return true;
    }
    
    try {
        // 停止定时器
        if (timer_info[timer_id].is_running) {
            timerAlarmDisable(timer_info[timer_id].timer);
        }
        
        // 分离中断
        timerDetachInterrupt(timer_info[timer_id].timer);
        
        // 结束定时器
        timerEnd(timer_info[timer_id].timer);
        
        // 清理定时器信息
        timer_info[timer_id].timer = nullptr;
        timer_info[timer_id].callback = nullptr;
        timer_info[timer_id].interval_ms = 0;
        timer_info[timer_id].trigger_count = 0;
        timer_info[timer_id].is_created = false;
        timer_info[timer_id].is_running = false;
        timer_info[timer_id].auto_reload = true;
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 删除成功").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("删除定时器" + String(timer_id) + " 失败").c_str());
        return false;
    }
}

bool TimerDriver::changeTimerInterval(TimerID timer_id, uint32_t new_interval_ms) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        Logger::getInstance().error("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return false;
    }
    
    if (!isValidInterval(new_interval_ms)) {
        Logger::getInstance().error("TimerDriver", ("无效的定时器间隔: " + String(new_interval_ms) + "ms").c_str());
        return false;
    }
    
    try {
        bool was_running = timer_info[timer_id].is_running;
        
        // 如果定时器在运行，先停止
        if (was_running) {
            timerAlarmDisable(timer_info[timer_id].timer);
        }
        
        // 更新间隔时间
        timer_info[timer_id].interval_ms = new_interval_ms;
        
        // 计算新的定时器计数值
        uint64_t timer_count = new_interval_ms * 1000;
        timerAlarmWrite(timer_info[timer_id].timer, timer_count, timer_info[timer_id].auto_reload);
        
        // 如果之前在运行，重新启动
        if (was_running) {
            timerRestart(timer_info[timer_id].timer);
            timerAlarmEnable(timer_info[timer_id].timer);
        }
        
        Logger::getInstance().info("TimerDriver", ("定时器" + String(timer_id) + " 间隔更新为: " + String(new_interval_ms) + "ms").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("TimerDriver", ("更新定时器" + String(timer_id) + " 间隔失败").c_str());
        return false;
    }
}

bool TimerDriver::isTimerRunning(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        return false;
    }
    return timer_info[timer_id].is_running;
}

uint32_t TimerDriver::getTimerInterval(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        return 0;
    }
    return timer_info[timer_id].interval_ms;
}

uint32_t TimerDriver::getTimerTriggerCount(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        return 0;
    }
    return timer_info[timer_id].trigger_count;
}

bool TimerDriver::resetTimerTriggerCount(TimerID timer_id) {
    if (!isValidTimerID(timer_id) || !timer_info[timer_id].is_created) {
        Logger::getInstance().error("TimerDriver", ("定时器" + String(timer_id) + " 未创建").c_str());
        return false;
    }
    
    timer_info[timer_id].trigger_count = 0;
    Logger::getInstance().debug("TimerDriver", ("定时器" + String(timer_id) + " 触发次数已重置").c_str());
    return true;
}

uint32_t TimerDriver::getSystemUptime() {
    return millis() - system_start_time;
}

void TimerDriver::delayMs(uint32_t delay_ms) {
    delay(delay_ms);
}

void TimerDriver::delayUs(uint32_t delay_us) {
    delayMicroseconds(delay_us);
}

TimerDriver& TimerDriver::getInstance() {
    static TimerDriver instance;
    return instance;
}

bool TimerDriver::isValidTimerID(TimerID timer_id) {
    return (timer_id >= TIMER_0 && timer_id < MAX_TIMERS);
}

bool TimerDriver::isValidInterval(uint32_t interval_ms) {
    // 最小间隔1ms，最大间隔约49天
    return (interval_ms >= 1 && interval_ms <= 0xFFFFFFFF / 1000);
}

void IRAM_ATTR TimerDriver::timerISR(TimerID timer_id) {
    if (instance && instance->isValidTimerID(timer_id)) {
        instance->handleTimerInterrupt(timer_id);
    }
}

void IRAM_ATTR TimerDriver::timer0ISR() {
    timerISR(TIMER_0);
}

void IRAM_ATTR TimerDriver::timer1ISR() {
    timerISR(TIMER_1);
}

void IRAM_ATTR TimerDriver::timer2ISR() {
    timerISR(TIMER_2);
}

void IRAM_ATTR TimerDriver::timer3ISR() {
    timerISR(TIMER_3);
}

void TimerDriver::handleTimerInterrupt(TimerID timer_id) {
    if (!timer_info[timer_id].is_created || !timer_info[timer_id].is_running) {
        return;
    }
    
    // 增加触发次数
    timer_info[timer_id].trigger_count++;
    
    // 调用回调函数
    if (timer_info[timer_id].callback) {
        timer_info[timer_id].callback();
    }
    
    // 如果不是自动重载模式，停止定时器
    if (!timer_info[timer_id].auto_reload) {
        timer_info[timer_id].is_running = false;
    }
}