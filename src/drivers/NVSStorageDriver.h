#ifndef NVS_STORAGE_DRIVER_H
#define NVS_STORAGE_DRIVER_H

#include <Arduino.h>
#include <nvs.h>
#include <nvs_flash.h>
#include "../common/Config.h"
#include "../common/Logger.h"

/**
 * NVS存储驱动类
 * 提供对MotorConfig配置参数的持久化存储功能
 */
class NVSStorageDriver {
public:
    /**
     * 构造函数
     */
    NVSStorageDriver();
    
    /**
     * 析构函数
     */
    ~NVSStorageDriver();
    
    /**
     * 初始化NVS存储
     * @param namespace_name 命名空间名称
     * @return 初始化是否成功
     */
    bool init(const char* namespace_name = "motor_config");
    
    /**
     * 保存MotorConfig配置
     * @param config 要保存的配置参数
     * @return 保存是否成功
     */
    bool saveConfig(const MotorConfig& config);
    
    /**
     * 读取MotorConfig配置
     * @param config 用于存储读取结果的配置参数
     * @return 读取是否成功
     */
    bool loadConfig(MotorConfig& config);
    
    /**
     * 删除MotorConfig配置
     * @return 删除是否成功
     */
    bool deleteConfig();
    
    /**
     * 检查配置是否存在
     * @return 配置是否存在
     */
    bool isConfigExist();
    
    /**
     * 获取错误信息
     * @return 最近一次错误信息
     */
    const char* getLastError() const;
    
private:
    nvs_handle_t nvs_handle;     // NVS句柄
    bool is_initialized;         // 是否已初始化
    char last_error[100];        // 最近一次错误信息
    
    /**
     * 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const char* error);
    
    /**
     * 验证初始化状态
     * @return 是否已初始化
     */
    bool checkInitialized();
};

#endif // NVS_STORAGE_DRIVER_H