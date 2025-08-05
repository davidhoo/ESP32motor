#include "GPIODriver.h"

GPIODriver::GPIODriver() {
    // 初始化所有引脚信息
    for (int i = 0; i < MAX_PINS; i++) {
        pin_info[i].initialized = false;
        pin_info[i].mode = 0;
        pin_info[i].last_state = LOW;
    }
    
    Logger::getInstance().info("GPIODriver", "GPIO驱动初始化完成");
}

GPIODriver::~GPIODriver() {
    Logger::getInstance().info("GPIODriver", "GPIO驱动析构");
}

bool GPIODriver::init(uint8_t pin, uint8_t mode, uint8_t initial_state) {
    // 验证引脚号
    if (!isValidPin(pin)) {
        Logger::getInstance().error("GPIODriver", ("无效的GPIO引脚号: " + String(pin)).c_str());
        return false;
    }
    
    // 验证引脚模式
    if (!isValidMode(mode)) {
        Logger::getInstance().error("GPIODriver", ("无效的GPIO模式: " + String(mode)).c_str());
        return false;
    }
    
    try {
        // 设置引脚模式
        pinMode(pin, mode);
        
        // 如果是输出模式，设置初始状态
        if (mode == OUTPUT) {
            ::digitalWrite(pin, initial_state);
            pin_info[pin].last_state = initial_state;
        }
        
        // 更新引脚信息
        pin_info[pin].initialized = true;
        pin_info[pin].mode = mode;
        
        Logger::getInstance().info("GPIODriver", ("GPIO" + String(pin) + " 初始化成功，模式: " + String(mode)).c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 初始化失败").c_str());
        return false;
    }
}

bool GPIODriver::digitalWrite(uint8_t pin, uint8_t state) {
    // 检查引脚是否已初始化
    if (!isPinInitialized(pin)) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 未初始化").c_str());
        return false;
    }
    
    // 检查是否为输出模式
    if (pin_info[pin].mode != OUTPUT) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 不是输出模式").c_str());
        return false;
    }
    
    try {
        ::digitalWrite(pin, state);
        pin_info[pin].last_state = state;
        
        Logger::getInstance().debug("GPIODriver", ("GPIO" + String(pin) + " 输出: " + (state ? "HIGH" : "LOW")).c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 输出失败").c_str());
        return false;
    }
}

int GPIODriver::digitalRead(uint8_t pin) {
    // 检查引脚是否已初始化
    if (!isPinInitialized(pin)) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 未初始化").c_str());
        return -1;
    }
    
    try {
        int state = ::digitalRead(pin);
        Logger::getInstance().debug("GPIODriver", ("GPIO" + String(pin) + " 读取: " + (state ? "HIGH" : "LOW")).c_str());
        return state;
        
    } catch (...) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 读取失败").c_str());
        return -1;
    }
}

bool GPIODriver::togglePin(uint8_t pin) {
    // 检查引脚是否已初始化
    if (!isPinInitialized(pin)) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 未初始化").c_str());
        return false;
    }
    
    // 检查是否为输出模式
    if (pin_info[pin].mode != OUTPUT) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 不是输出模式").c_str());
        return false;
    }
    
    // 切换状态
    uint8_t new_state = (pin_info[pin].last_state == HIGH) ? LOW : HIGH;
    return digitalWrite(pin, new_state);
}

bool GPIODriver::isPinInitialized(uint8_t pin) {
    if (!isValidPin(pin)) {
        return false;
    }
    return pin_info[pin].initialized;
}

int GPIODriver::getPinMode(uint8_t pin) {
    if (!isPinInitialized(pin)) {
        return -1;
    }
    return pin_info[pin].mode;
}

bool GPIODriver::resetPin(uint8_t pin) {
    if (!isValidPin(pin)) {
        Logger::getInstance().error("GPIODriver", ("无效的GPIO引脚号: " + String(pin)).c_str());
        return false;
    }
    
    try {
        // 重置为输入模式
        pinMode(pin, INPUT);
        
        // 清除引脚信息
        pin_info[pin].initialized = false;
        pin_info[pin].mode = 0;
        pin_info[pin].last_state = LOW;
        
        Logger::getInstance().info("GPIODriver", ("GPIO" + String(pin) + " 重置成功").c_str());
        return true;
        
    } catch (...) {
        Logger::getInstance().error("GPIODriver", ("GPIO" + String(pin) + " 重置失败").c_str());
        return false;
    }
}

int GPIODriver::initMultiplePins(const uint8_t* pins, const uint8_t* modes, 
                                const uint8_t* initial_states, uint8_t count) {
    if (pins == nullptr || modes == nullptr || initial_states == nullptr) {
        Logger::getInstance().error("GPIODriver", "批量初始化参数为空");
        return 0;
    }
    
    int success_count = 0;
    
    for (uint8_t i = 0; i < count; i++) {
        if (init(pins[i], modes[i], initial_states[i])) {
            success_count++;
        }
    }
    
    Logger::getInstance().info("GPIODriver", ("批量初始化完成，成功: " + String(success_count) + "/" + String(count)).c_str());
    return success_count;
}

bool GPIODriver::isValidPin(uint8_t pin) {
    // ESP32-S3的有效GPIO引脚范围
    // GPIO 0-21, 26-48 (排除一些特殊用途的引脚)
    if (pin > 48) {
        return false;
    }
    
    // 排除一些不建议使用的引脚
    // GPIO 22-25 用于SPI Flash
    if (pin >= 22 && pin <= 25) {
        return false;
    }
    
    return true;
}

bool GPIODriver::isValidMode(uint8_t mode) {
    return (mode == INPUT || mode == OUTPUT || 
            mode == INPUT_PULLUP || mode == INPUT_PULLDOWN);
}