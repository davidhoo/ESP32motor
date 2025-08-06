#include "EventManager.h"
#include <algorithm>

EventManager* EventManager::instance = nullptr;

EventManager::EventManager() : isInitialized(false), queueMutex(nullptr) {}

EventManager& EventManager::getInstance() {
    if (instance == nullptr) {
        instance = new EventManager();
    }
    return *instance;
}

bool EventManager::initialize() {
    if (isInitialized) {
        return true;
    }
    
    queueMutex = xSemaphoreCreateMutex();
    if (queueMutex == nullptr) {
        return false;
    }
    
    isInitialized = true;
    return true;
}

void EventManager::cleanup() {
    if (queueMutex != nullptr) {
        vSemaphoreDelete(queueMutex);
        queueMutex = nullptr;
    }
    
    listeners.clear();
    eventQueue.clear();
    isInitialized = false;
}

bool EventManager::subscribe(EventType type, EventListener listener) {
    if (!isInitialized || listener == nullptr) {
        return false;
    }
    
    listeners[type].push_back(listener);
    return true;
}

bool EventManager::unsubscribe(EventType type, EventListener listener) {
    if (!isInitialized || listener == nullptr) {
        return false;
    }
    
    auto it = listeners.find(type);
    if (it == listeners.end()) {
        return false;
    }
    
    auto& vec = it->second;
    
    // 由于std::function无法直接比较，我们清空该类型的所有监听器
    vec.clear();
    
    if (vec.empty()) {
        listeners.erase(it);
    }
    
    return true;
}

bool EventManager::publish(const EventData& event) {
    if (!isInitialized) {
        return false;
    }
    
    auto it = listeners.find(event.type);
    if (it == listeners.end()) {
        return false;
    }
    
    for (const auto& listener : it->second) {
        if (listener != nullptr) {
            listener(event);
        }
    }
    
    return true;
}

bool EventManager::publishAsync(const EventData& event) {
    if (!isInitialized || queueMutex == nullptr) {
        return false;
    }
    
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        eventQueue.push_back(event);
        xSemaphoreGive(queueMutex);
        return true;
    }
    
    return false;
}

void EventManager::processEvents() {
    if (!isInitialized || queueMutex == nullptr) {
        return;
    }
    
    std::vector<EventData> localQueue;
    
    // 复制事件到本地队列
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        localQueue = eventQueue;
        eventQueue.clear();
        xSemaphoreGive(queueMutex);
    }
    
    // 处理本地队列中的事件
    for (const auto& event : localQueue) {
        publish(event);
    }
}

size_t EventManager::getQueueSize() const {
    if (!isInitialized || queueMutex == nullptr) {
        return 0;
    }
    
    size_t size = 0;
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        size = eventQueue.size();
        xSemaphoreGive(queueMutex);
    }
    return size;
}

void EventManager::clearQueue() {
    if (!isInitialized || queueMutex == nullptr) {
        return;
    }
    
    if (xSemaphoreTake(queueMutex, portMAX_DELAY) == pdTRUE) {
        eventQueue.clear();
        xSemaphoreGive(queueMutex);
    }
}

String EventManager::getEventTypeName(EventType type) {
    switch (type) {
        case EventType::SYSTEM_STARTUP: return "SYSTEM_STARTUP";
        case EventType::SYSTEM_SHUTDOWN: return "SYSTEM_SHUTDOWN";
        case EventType::MOTOR_START: return "MOTOR_START";
        case EventType::MOTOR_STOP: return "MOTOR_STOP";
        case EventType::MOTOR_SPEED_CHANGED: return "MOTOR_SPEED_CHANGED";
        case EventType::BLE_CONNECTED: return "BLE_CONNECTED";
        case EventType::BLE_DISCONNECTED: return "BLE_DISCONNECTED";
        case EventType::CONFIG_CHANGED: return "CONFIG_CHANGED";
        case EventType::ERROR_OCCURRED: return "ERROR_OCCURRED";
        case EventType::WARNING_TRIGGERED: return "WARNING_TRIGGERED";
        case EventType::LED_STATE_CHANGED: return "LED_STATE_CHANGED";
        case EventType::BUTTON_PRESSED: return "BUTTON_PRESSED";
        case EventType::CUSTOM_EVENT: return "CUSTOM_EVENT";
        default: return "UNKNOWN";
    }
}