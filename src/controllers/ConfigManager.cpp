#include "ConfigManager.h"
#include <cstring>
#include "../common/Logger.h"

/**
 * 获取配置管理器单例实例
 */
ConfigManager& ConfigManager::getInstance() {
    static ConfigManager instance;
    return instance;
}

/**
 * 构造函数
 */
ConfigManager::ConfigManager() : 
    isInitialized(false),
    isModified(false) {
    memset(lastError, 0, sizeof(lastError));
    memset(validationError, 0, sizeof(validationError));
    
    // 设置默认配置
    defaultConfig.runDuration = 5000;   // 5秒
    defaultConfig.stopDuration = 2000;  // 2秒
    defaultConfig.cycleCount = 0;       // 无限循环
    defaultConfig.autoStart = true;     // 自动启动
    
    // 初始化为默认配置
    currentConfig = defaultConfig;
    lastSavedConfig = defaultConfig;
}

/**
 * 析构函数
 */
ConfigManager::~ConfigManager() {
    // 清理资源
}

/**
 * 初始化配置管理器
 */
bool ConfigManager::init() {
    setLastError("");
    
    // 初始化NVS存储
    if (!nvsStorage.init("motor_config")) {
        setLastError("NVS存储初始化失败");
        LOG_TAG_ERROR("ConfigManager", "NVS存储初始化失败: %s", nvsStorage.getLastError());
        return false;
    }
    
    isInitialized = true;
    
    // 尝试加载已保存的配置
    if (!loadConfig()) {
        LOG_TAG_WARN("ConfigManager", "加载配置失败，使用默认配置");
        resetToDefaults();
    }
    
    LOG_TAG_INFO("ConfigManager", "配置管理器初始化成功");
    return true;
}

/**
 * 加载配置
 */
bool ConfigManager::loadConfig() {
    if (!isInitialized) {
        setLastError("配置管理器未初始化");
        LOG_TAG_ERROR("ConfigManager", "配置管理器未初始化");
        return false;
    }
    
    setLastError("");
    
    MotorConfig loadedConfig;
    
    // 从NVS加载配置
    if (!nvsStorage.loadConfig(loadedConfig)) {
        setLastError("从NVS加载配置失败");
        LOG_TAG_ERROR("ConfigManager", "从NVS加载配置失败: %s", nvsStorage.getLastError());
        return false;
    }
    
    // 验证加载的配置
    if (!validateConfig(loadedConfig)) {
        setLastError("加载的配置无效");
        LOG_TAG_WARN("ConfigManager", "加载的配置无效: %s", getValidationError());
        resetToDefaults();
        return false;
    }
    
    // 更新当前配置
    currentConfig = loadedConfig;
    lastSavedConfig = loadedConfig;
    isModified = false;
    
    LOG_TAG_INFO("ConfigManager", "配置加载成功");
    LOG_TAG_DEBUG("ConfigManager", "运行时长: %lu ms, 停止时长: %lu ms, 循环次数: %lu, 自动启动: %s",
                  currentConfig.runDuration, currentConfig.stopDuration,
                  currentConfig.cycleCount, currentConfig.autoStart ? "是" : "否");
    
    return true;
}

/**
 * 保存配置
 */
bool ConfigManager::saveConfig() {
    if (!isInitialized) {
        setLastError("配置管理器未初始化");
        LOG_TAG_ERROR("ConfigManager", "配置管理器未初始化");
        return false;
    }
    
    setLastError("");
    
    // 验证当前配置
    if (!validateConfig(currentConfig)) {
        setLastError("当前配置无效");
        LOG_TAG_ERROR("ConfigManager", "当前配置无效: %s", getValidationError());
        return false;
    }
    
    // 保存到NVS
    if (!nvsStorage.saveConfig(currentConfig)) {
        setLastError("保存配置到NVS失败");
        LOG_TAG_ERROR("ConfigManager", "保存配置到NVS失败: %s", nvsStorage.getLastError());
        return false;
    }
    
    // 更新最后保存的配置
    lastSavedConfig = currentConfig;
    isModified = false;
    
    LOG_TAG_INFO("ConfigManager", "配置保存成功");
    return true;
}

/**
 * 重置配置为默认值
 */
void ConfigManager::resetToDefaults() {
    currentConfig = defaultConfig;
    lastSavedConfig = defaultConfig;
    isModified = true;
    
    LOG_TAG_INFO("ConfigManager", "配置已重置为默认值");
}

/**
 * 删除存储的配置
 */
bool ConfigManager::deleteStoredConfig() {
    if (!isInitialized) {
        setLastError("配置管理器未初始化");
        LOG_TAG_ERROR("ConfigManager", "配置管理器未初始化");
        return false;
    }
    
    setLastError("");
    
    if (!nvsStorage.deleteConfig()) {
        setLastError("删除存储配置失败");
        LOG_TAG_ERROR("ConfigManager", "删除存储配置失败: %s", nvsStorage.getLastError());
        return false;
    }
    
    // 重置为默认配置
    resetToDefaults();
    
    LOG_TAG_INFO("ConfigManager", "存储的配置已删除");
    return true;
}

/**
 * 获取当前配置
 */
const MotorConfig& ConfigManager::getConfig() const {
    return currentConfig;
}

/**
 * 更新配置
 */
void ConfigManager::updateConfig(const MotorConfig& config) {
    if (validateConfig(config)) {
        currentConfig = config;
        isModified = true;
        
        LOG_TAG_INFO("ConfigManager", "配置已更新");
    } else {
        LOG_TAG_WARN("ConfigManager", "尝试更新无效配置: %s", getValidationError());
    }
}

/**
 * 验证配置参数的有效性
 */
bool ConfigManager::validateConfig(const MotorConfig& config) const {
    // 验证运行时长
    if (config.runDuration < 100 || config.runDuration > 3600000) {  // 100ms - 1小时
        const_cast<ConfigManager*>(this)->setValidationError("运行时长必须在100ms到3600000ms之间");
        return false;
    }
    
    // 验证停止时长
    if (config.stopDuration < 0 || config.stopDuration > 3600000) {  // 0ms - 1小时
        const_cast<ConfigManager*>(this)->setValidationError("停止时长必须在0ms到3600000ms之间");
        return false;
    }
    
    // 验证循环次数
    if (config.cycleCount > 1000000) {  // 最大100万次
        const_cast<ConfigManager*>(this)->setValidationError("循环次数不能超过1000000次");
        return false;
    }
    
    // 验证通过
    const_cast<ConfigManager*>(this)->setValidationError("");
    return true;
}

/**
 * 获取配置验证错误信息
 */
const char* ConfigManager::getValidationError() const {
    return validationError;
}

/**
 * 检查配置是否已修改
 */
bool ConfigManager::isConfigModified() const {
    return isModified;
}

/**
 * 标记配置为已保存
 */
void ConfigManager::markConfigSaved() {
    lastSavedConfig = currentConfig;
    isModified = false;
}

/**
 * 获取错误信息
 */
const char* ConfigManager::getLastError() const {
    return lastError;
}

/**
 * 设置错误信息
 */
void ConfigManager::setLastError(const char* error) {
    if (error) {
        strncpy(lastError, error, sizeof(lastError) - 1);
        lastError[sizeof(lastError) - 1] = '\0';
    } else {
        memset(lastError, 0, sizeof(lastError));
    }
}

/**
 * 设置验证错误信息
 */
void ConfigManager::setValidationError(const char* error) {
    if (error) {
        strncpy(validationError, error, sizeof(validationError) - 1);
        validationError[sizeof(validationError) - 1] = '\0';
    } else {
        memset(validationError, 0, sizeof(validationError));
    }
}