#ifndef EVENT_MANAGER_H
#define EVENT_MANAGER_H

#include <Arduino.h>
#include <functional>
#include <vector>
#include <map>

// 事件类型枚举
enum class EventType {
    SYSTEM_STARTUP,
    SYSTEM_SHUTDOWN,
    MOTOR_START,
    MOTOR_STOP,
    MOTOR_SPEED_CHANGED,
    BLE_CONNECTED,
    BLE_DISCONNECTED,
    CONFIG_CHANGED,
    ERROR_OCCURRED,
    WARNING_TRIGGERED,
    LED_STATE_CHANGED,
    BUTTON_PRESSED,
    CUSTOM_EVENT
};

// 事件数据结构
struct EventData {
    EventType type;
    String source;
    String message;
    int32_t value;
    void* extraData;
    
    EventData(EventType t, const String& src = "", const String& msg = "", int32_t val = 0, void* ext = nullptr)
        : type(t), source(src), message(msg), value(val), extraData(ext) {}
};

// 事件监听器类型
using EventListener = std::function<void(const EventData&)>;

class EventManager {
private:
    static EventManager* instance;
    std::map<EventType, std::vector<EventListener>> listeners;
    std::vector<EventData> eventQueue;
    SemaphoreHandle_t queueMutex;
    bool isInitialized;
    
    EventManager();
    
public:
    static EventManager& getInstance();
    
    // 初始化事件管理器
    bool initialize();
    
    // 清理资源
    void cleanup();
    
    // 订阅事件
    bool subscribe(EventType type, EventListener listener);
    
    // 取消订阅
    bool unsubscribe(EventType type, EventListener listener);
    
    // 发布事件（立即执行）
    bool publish(const EventData& event);
    
    // 发布事件（异步，加入队列）
    bool publishAsync(const EventData& event);
    
    // 处理事件队列
    void processEvents();
    
    // 获取队列中的事件数量
    size_t getQueueSize() const;
    
    // 清空事件队列
    void clearQueue();
    
    // 获取事件类型名称
    static String getEventTypeName(EventType type);
};

#endif // EVENT_MANAGER_H