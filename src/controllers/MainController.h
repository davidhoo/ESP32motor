#ifndef MAIN_CONTROLLER_H
#define MAIN_CONTROLLER_H

#include "MotorController.h"
#include "LEDController.h"
#include "ConfigManager.h"
#include "MotorBLEServer.h"
#include "../common/EventManager.h"

/**
 * @brief 主控制器类 - 系统核心管理器
 * 
 * MainController负责整个系统的初始化和协调管理，
 * 采用单例模式确保全局只有一个实例。
 */
class MainController {
public:
    /**
     * @brief 获取MainController单例实例
     * @return MainController& 单例引用
     */
    static MainController& getInstance();
    
    /**
     * @brief 初始化系统
     * @return true 初始化成功
     * @return false 初始化失败
     */
    bool init();
    
    /**
     * @brief 运行系统主循环
     */
    void run();
    
    /**
     * @brief 停止系统
     */
    void stop();
    
    /**
     * @brief 获取电机控制器
     * @return MotorController& 电机控制器引用
     */
    MotorController& getMotorController() { return MotorController::getInstance(); }
    
    /**
     * @brief 获取LED控制器
     * @return LEDController& LED控制器引用
     */
    LEDController& getLEDController() { return ledController; }
    
    /**
     * @brief 获取配置管理器
     * @return ConfigManager& 配置管理器引用
     */
    ConfigManager& getConfigManager() { return ConfigManager::getInstance(); }
    
    /**
     * @brief 获取BLE服务器
     * @return MotorBLEServer& BLE服务器引用
     */
    MotorBLEServer& getBLEServer() { return MotorBLEServer::getInstance(); }
    
    /**
     * @brief 获取系统运行状态
     * @return true 系统运行中
     * @return false 系统已停止
     */
    bool isRunning() const { return running; }

private:
    // 私有构造函数 - 单例模式
    MainController();
    
    // 禁止拷贝构造和赋值
    MainController(const MainController&) = delete;
    MainController& operator=(const MainController&) = delete;
    
    // 析构函数
    ~MainController();
    
    // 初始化各个模块
    bool initializeMotorController();
    bool initializeLEDController();
    bool initializeConfigManager();
    bool initializeBLEServer();
    
    // 清理资源
    void cleanup();
    
    // LED控制器实例（非单例）
    LEDController ledController;
    
    // 系统状态
    bool running;
    bool initialized;
    
    // 模块初始化状态
    bool motorControllerInitialized;
    bool ledControllerInitialized;
    bool configManagerInitialized;
    bool bleServerInitialized;
    
    // 事件系统相关
    bool initializeEventManager();
    void setupEventListeners();
    void handleSystemEvent(const EventData& event);
    void handleMotorEvent(const EventData& event);
    void handleBLEEvent(const EventData& event);
    void handleConfigEvent(const EventData& event);
};

#endif // MAIN_CONTROLLER_H