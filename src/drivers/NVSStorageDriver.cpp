#include "NVSStorageDriver.h"
#include <cstring>

/**
 * 构造函数
 */
NVSStorageDriver::NVSStorageDriver() : 
    nvs_handle(0), 
    is_initialized(false) {
    memset(last_error, 0, sizeof(last_error));
}

/**
 * 析构函数
 */
NVSStorageDriver::~NVSStorageDriver() {
    if (is_initialized) {
        nvs_close(nvs_handle);
    }
}

/**
 * 初始化NVS存储
 * @param namespace_name 命名空间名称
 * @return 初始化是否成功
 */
bool NVSStorageDriver::init(const char* namespace_name) {
    setLastError("");
    
    // 初始化NVS flash
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        // NVS分区已满或版本不匹配，需要擦除并重新初始化
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    
    if (err != ESP_OK) {
        setLastError("NVS flash初始化失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "NVS flash初始化失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 打开NVS命名空间
    err = nvs_open(namespace_name, NVS_READWRITE, &nvs_handle);
    if (err != ESP_OK) {
        setLastError("打开NVS命名空间失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "打开NVS命名空间失败: %s", esp_err_to_name(err));
        return false;
    }
    
    is_initialized = true;
    Logger::getInstance().info(String("NVSStorageDriver"), "NVS存储初始化成功");
    return true;
}

/**
 * 保存MotorConfig配置
 * @param config 要保存的配置参数
 * @return 保存是否成功
 */
bool NVSStorageDriver::saveConfig(const MotorConfig& config) {
    if (!checkInitialized()) {
        return false;
    }
    
    setLastError("");
    
    // 保存runDuration
    esp_err_t err = nvs_set_u32(nvs_handle, "runDuration", config.runDuration);
    if (err != ESP_OK) {
        setLastError("保存runDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "保存runDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 保存stopDuration
    err = nvs_set_u32(nvs_handle, "stopDuration", config.stopDuration);
    if (err != ESP_OK) {
        setLastError("保存stopDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "保存stopDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 保存cycleCount
    err = nvs_set_u32(nvs_handle, "cycleCount", config.cycleCount);
    if (err != ESP_OK) {
        setLastError("保存cycleCount失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "保存cycleCount失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 保存autoStart
    err = nvs_set_u8(nvs_handle, "autoStart", config.autoStart ? 1 : 0);
    if (err != ESP_OK) {
        setLastError("保存autoStart失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "保存autoStart失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 提交更改
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        setLastError("提交NVS更改失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "提交NVS更改失败: %s", esp_err_to_name(err));
        return false;
    }
    
    Logger::getInstance().info("NVSStorageDriver", "配置保存成功");
    return true;
}

/**
 * 读取MotorConfig配置
 * @param config 用于存储读取结果的配置参数
 * @return 读取是否成功
 */
bool NVSStorageDriver::loadConfig(MotorConfig& config) {
    if (!checkInitialized()) {
        return false;
    }
    
    setLastError("");
    
    // 读取runDuration
    esp_err_t err = nvs_get_u32(nvs_handle, "runDuration", &config.runDuration);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("读取runDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "读取runDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 读取stopDuration
    err = nvs_get_u32(nvs_handle, "stopDuration", &config.stopDuration);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("读取stopDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "读取stopDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 读取cycleCount
    err = nvs_get_u32(nvs_handle, "cycleCount", &config.cycleCount);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("读取cycleCount失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "读取cycleCount失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 读取autoStart
    uint8_t autoStartValue = 0;
    err = nvs_get_u8(nvs_handle, "autoStart", &autoStartValue);
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("读取autoStart失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "读取autoStart失败: %s", esp_err_to_name(err));
        return false;
    }
    config.autoStart = (autoStartValue != 0);
    
    Logger::getInstance().info("NVSStorageDriver", "配置读取成功");
    return true;
}

/**
 * 删除MotorConfig配置
 * @return 删除是否成功
 */
bool NVSStorageDriver::deleteConfig() {
    if (!checkInitialized()) {
        return false;
    }
    
    setLastError("");
    
    // 删除runDuration
    esp_err_t err = nvs_erase_key(nvs_handle, "runDuration");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("删除runDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "删除runDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 删除stopDuration
    err = nvs_erase_key(nvs_handle, "stopDuration");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("删除stopDuration失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "删除stopDuration失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 删除cycleCount
    err = nvs_erase_key(nvs_handle, "cycleCount");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("删除cycleCount失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "删除cycleCount失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 删除autoStart
    err = nvs_erase_key(nvs_handle, "autoStart");
    if (err != ESP_OK && err != ESP_ERR_NVS_NOT_FOUND) {
        setLastError("删除autoStart失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "删除autoStart失败: %s", esp_err_to_name(err));
        return false;
    }
    
    // 提交更改
    err = nvs_commit(nvs_handle);
    if (err != ESP_OK) {
        setLastError("提交NVS更改失败");
        Logger::getInstance().error(String("NVSStorageDriver"), "提交NVS更改失败: %s", esp_err_to_name(err));
        return false;
    }
    
    Logger::getInstance().info("NVSStorageDriver", "配置删除成功");
    return true;
}

/**
 * 检查配置是否存在
 * @return 配置是否存在
 */
bool NVSStorageDriver::isConfigExist() {
    if (!checkInitialized()) {
        return false;
    }
    
    setLastError("");
    
    // 检查runDuration是否存在
    uint32_t value;
    esp_err_t err = nvs_get_u32(nvs_handle, "runDuration", &value);
    return (err == ESP_OK);
}

/**
 * 获取错误信息
 * @return 最近一次错误信息
 */
const char* NVSStorageDriver::getLastError() const {
    return last_error;
}

/**
 * 设置错误信息
 * @param error 错误信息
 */
void NVSStorageDriver::setLastError(const char* error) {
    if (error) {
        strncpy(last_error, error, sizeof(last_error) - 1);
        last_error[sizeof(last_error) - 1] = '\0';
    } else {
        memset(last_error, 0, sizeof(last_error));
    }
}

/**
 * 验证初始化状态
 * @return 是否已初始化
 */
bool NVSStorageDriver::checkInitialized() {
    if (!is_initialized) {
        setLastError("NVS存储未初始化");
        Logger::getInstance().error("NVSStorageDriver", "NVS存储未初始化");
        return false;
    }
    return true;
}