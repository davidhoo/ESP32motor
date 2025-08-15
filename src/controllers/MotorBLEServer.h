#ifndef MOTOR_BLE_SERVER_H
#define MOTOR_BLE_SERVER_H

#include <Arduino.h>
#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>
#include <ArduinoJson.h>
#include "../common/Logger.h"
#include "../common/StateManager.h"
#include "../controllers/MotorController.h"
#include "../controllers/ConfigManager.h"
#include "../controllers/MotorModbusController.h"

/**
 * @brief BLE服务器类
 * 提供BLE通信服务，支持参数配置和状态查询
 */
class MotorBLEServer {
public:
    /**
     * @brief 获取单例实例
     * @return MotorBLEServer& 单例引用
     */
    static MotorBLEServer& getInstance();
    
    /**
     * @brief 初始化BLE服务器
     * @return true 初始化成功，false 初始化失败
     */
    bool init();
    
    /**
     * @brief 启动BLE服务
     */
    void start();
    
    /**
     * @brief 停止BLE服务
     */
    void stop();
    
    /**
     * @brief 更新BLE状态
     */
    void update();
    
    /**
     * @brief 获取当前连接状态
     * @return true 已连接，false 未连接
     */
    bool isConnected() const;
    
    /**
     * @brief 发送状态通知
     * @param status JSON格式的状态信息
     */
    void sendStatusNotification(const String& status);
    
    /**
     * @brief 获取最后错误信息
     * @return const char* 错误信息
     */
    const char* getLastError() const { return lastError; }

    // 公开这些方法供测试使用
    void handleRunDurationWrite(const String& value);
    void handleStopIntervalWrite(const String& value);
    void handleSystemControlWrite(const String& value);
    void handleSpeedControllerConfigWrite(const String& value);
    String generateStatusJson();
    String generateSpeedControllerConfigJson();
    String generateInfoJson();
    void onSystemStateChanged(const StateChangeEvent& event);

private:
    // 单例模式
    MotorBLEServer();
    ~MotorBLEServer();
    MotorBLEServer(const MotorBLEServer&) = delete;
    MotorBLEServer& operator=(const MotorBLEServer&) = delete;
    
    // BLE相关对象
    BLEServer* pServer = nullptr;
    BLEService* pService = nullptr;
    BLECharacteristic* pRunDurationCharacteristic = nullptr;
    BLECharacteristic* pStopIntervalCharacteristic = nullptr;
    BLECharacteristic* pSystemControlCharacteristic = nullptr;
    BLECharacteristic* pStatusQueryCharacteristic = nullptr;
    BLECharacteristic* pSpeedControllerStatusCharacteristic = nullptr;
    BLECharacteristic* pSpeedControllerConfigCharacteristic = nullptr;
    
    // 状态
    // 状态
    bool deviceConnected = false;
    bool oldDeviceConnected = false;
    char lastError[128] = "";
    
    // 调速器状态读取保护
    uint32_t lastSpeedControllerStatusReadTime = 0;  // 上次读取调速器状态的时间
    
    // === 5.4.3 BLE断连时的系统稳定运行机制 ===
    bool disconnectionHandled = false;
    uint32_t lastConnectionTime = 0;
    uint32_t disconnectionCount = 0;
    static const uint32_t RECONNECTION_TIMEOUT = 30000; // 30秒重连超时
    // StateManager引用
    StateManager& stateManager;
    
    // MotorModbusController实例
    MotorModbusController* pMotorModbusController = nullptr;
    
    
    
    // BLE服务器回调类
    class ServerCallbacks : public BLEServerCallbacks {
    public:
        ServerCallbacks(MotorBLEServer* bleServer) : bleServer(bleServer) {}
        void onConnect(BLEServer* pServer) override;
        void onDisconnect(BLEServer* pServer) override;
    private:
        MotorBLEServer* bleServer;
    };
    
    // BLE特征值回调类
    class CharacteristicCallbacks : public BLECharacteristicCallbacks {
    public:
        CharacteristicCallbacks(MotorBLEServer* bleServer, const char* charUUID) 
            : bleServer(bleServer), charUUID(charUUID) {}
        void onWrite(BLECharacteristic* pCharacteristic) override;
        void onRead(BLECharacteristic* pCharacteristic) override;
    private:
        MotorBLEServer* bleServer;
        const char* charUUID;
    };
    
    // 内部方法
    void setError(const char* error);
    void configureBLELowPowerDirect();
    
    // === 5.4.3 BLE断连时的系统稳定运行机制 ===
    void handleDisconnection();
    void ensureSystemStability();
    bool shouldAttemptReconnection();
    void resetConnectionState();
};

#endif // MOTOR_BLE_SERVER_H