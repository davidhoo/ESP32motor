#ifndef CONFIG_MANAGER_H
#define CONFIG_MANAGER_H

#include "../common/Config.h"
#include "../drivers/NVSStorageDriver.h"
#include "../common/Logger.h"
#include "../common/StateManager.h"

/**
 * 配置管理器类
 * 负责管理电机运行参数的加载、保存和验证
 */
class ConfigManager {
public:
    /**
     * 获取配置管理器单例实例
     * @return ConfigManager实例引用
     */
    static ConfigManager& getInstance();
    
    /**
     * 初始化配置管理器
     * @return 初始化是否成功
     */
    bool init();
    
    /**
     * 加载配置
     * 从NVS存储加载配置，如果失败则使用默认值
     * @return 加载是否成功
     */
    bool loadConfig();
    
    /**
     * 保存配置
     * 将当前配置保存到NVS存储
     * @return 保存是否成功
     */
    bool saveConfig();
    
    /**
     * 重置配置为默认值
     */
    void resetToDefaults();
    
    /**
     * 删除存储的配置
     * @return 删除是否成功
     */
    bool deleteStoredConfig();
    
    /**
     * 获取当前配置
     * @return 当前配置引用
     */
    const MotorConfig& getConfig() const;
    
    /**
     * 更新配置
     * @param config 新配置
     */
    void updateConfig(const MotorConfig& config);
    
    /**
     * 验证配置参数的有效性
     * @param config 要验证的配置
     * @return 配置是否有效
     */
    bool validateConfig(const MotorConfig& config) const;
    
    /**
     * 验证并修正配置参数（5.4.2 参数越界检查和默认值回退）
     * @param config 要验证和修正的配置（引用传递，会被修改）
     * @return 是否进行了修正
     */
    bool validateAndSanitizeConfig(MotorConfig& config) const;
    
    /**
     * 获取配置验证错误信息
     * @return 错误信息
     */
    const char* getValidationError() const;
    
    /**
     * 检查配置是否已修改
     * @return 配置是否已修改
     */
    bool isConfigModified() const;
    
    /**
     * 标记配置为已保存
     */
    void markConfigSaved();
    
    /**
     * 获取错误信息
     * @return 最近一次错误信息
     */
    const char* getLastError() const;
    
    /**
     * @brief 系统状态变更回调
     * @param event 状态变更事件
     */
    void onSystemStateChanged(const StateChangeEvent& event);

private:
    /**
     * 构造函数 - 私有，使用单例模式
     */
    ConfigManager();
    
    /**
     * 析构函数
     */
    ~ConfigManager();
    
    /**
     * 复制构造函数 - 禁用
     */
    ConfigManager(const ConfigManager&) = delete;
    
    /**
     * 赋值运算符 - 禁用
     */
    ConfigManager& operator=(const ConfigManager&) = delete;
    
    MotorConfig currentConfig;      // 当前配置
    MotorConfig defaultConfig;      // 默认配置
    MotorConfig lastSavedConfig;    // 最后保存的配置
    NVSStorageDriver nvsStorage;    // NVS存储驱动
    StateManager& stateManager;     // 状态管理器引用
    bool isInitialized;             // 是否已初始化
    bool isModified;                // 配置是否已修改
    char lastError[100];            // 最近一次错误信息
    char validationError[100];      // 验证错误信息
    
    /**
     * 设置错误信息
     * @param error 错误信息
     */
    void setLastError(const char* error);
    
    /**
     * 设置验证错误信息
     * @param error 验证错误信息
     */
    void setValidationError(const char* error);
};

#endif // CONFIG_MANAGER_H