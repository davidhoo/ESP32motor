#include "NVSStorageTest.h"
#include "../common/Logger.h"

bool NVSStorageTest::runAllTests() {
    LOG_TAG_INFO("NVSTest", "开始NVS存储驱动测试...");
    
    bool allPassed = true;
    
    // 测试初始化功能
    if (!testInit()) {
        LOG_TAG_ERROR("NVSTest", "❌ 初始化测试失败");
        allPassed = false;
    } else {
        LOG_TAG_INFO("NVSTest", "✅ 初始化测试通过");
    }
    
    // 测试保存配置功能
    if (!testSaveConfig()) {
        LOG_TAG_ERROR("NVSTest", "❌ 保存配置测试失败");
        allPassed = false;
    } else {
        LOG_TAG_INFO("NVSTest", "✅ 保存配置测试通过");
    }
    
    // 测试读取配置功能
    if (!testLoadConfig()) {
        LOG_TAG_ERROR("NVSTest", "❌ 读取配置测试失败");
        allPassed = false;
    } else {
        LOG_TAG_INFO("NVSTest", "✅ 读取配置测试通过");
    }
    
    // 测试删除配置功能
    if (!testDeleteConfig()) {
        LOG_TAG_ERROR("NVSTest", "❌ 删除配置测试失败");
        allPassed = false;
    } else {
        LOG_TAG_INFO("NVSTest", "✅ 删除配置测试通过");
    }
    
    // 测试数据持久化功能
    if (!testPersistence()) {
        LOG_TAG_ERROR("NVSTest", "❌ 数据持久化测试失败");
        allPassed = false;
    } else {
        LOG_TAG_INFO("NVSTest", "✅ 数据持久化测试通过");
    }
    
    if (allPassed) {
        LOG_TAG_INFO("NVSTest", "🎉 所有NVS存储驱动测试通过!");
    } else {
        LOG_TAG_ERROR("NVSTest", "💥 部分NVS存储驱动测试失败!");
    }
    
    return allPassed;
}

bool NVSStorageTest::testInit() {
    NVSStorageDriver nvsStorage;
    return nvsStorage.init();
}

bool NVSStorageTest::testSaveConfig() {
    NVSStorageDriver nvsStorage;
    if (!nvsStorage.init()) {
        return false;
    }
    
    MotorConfig config;
    config.runDuration = 10;
    config.stopDuration = 5;
    config.cycleCount = 10;
    config.autoStart = true;
    
    return nvsStorage.saveConfig(config);
}

bool NVSStorageTest::testLoadConfig() {
    NVSStorageDriver nvsStorage;
    if (!nvsStorage.init()) {
        return false;
    }
    
    MotorConfig config;
    if (!nvsStorage.loadConfig(config)) {
        return false;
    }
    
    // 验证读取的数据是否正确
    return (config.runDuration == 10) &&
           (config.stopDuration == 5) &&
           (config.cycleCount == 10) &&
           (config.autoStart == true);
}

bool NVSStorageTest::testDeleteConfig() {
    NVSStorageDriver nvsStorage;
    if (!nvsStorage.init()) {
        return false;
    }
    
    // 先保存一个配置
    MotorConfig config;
    config.runDuration = 20;
    config.stopDuration = 10;
    config.cycleCount = 20;
    config.autoStart = false;
    
    if (!nvsStorage.saveConfig(config)) {
        return false;
    }
    
    // 删除配置
    if (!nvsStorage.deleteConfig()) {
        return false;
    }
    
    // 验证配置是否已删除
    MotorConfig loadedConfig;
    // 读取应该失败或返回默认值
    nvsStorage.loadConfig(loadedConfig);
    // 由于loadConfig不会在键不存在时返回false，我们需要检查值是否为默认值
    return (loadedConfig.runDuration != 20);
}

bool NVSStorageTest::testPersistence() {
    NVSStorageDriver nvsStorage1;
    if (!nvsStorage1.init()) {
        return false;
    }
    
    // 保存配置
    MotorConfig config;
    config.runDuration = 30;
    config.stopDuration = 15;
    config.cycleCount = 30;
    config.autoStart = true;
    
    if (!nvsStorage1.saveConfig(config)) {
        return false;
    }
    
    // 创建新的实例来模拟重启
    NVSStorageDriver nvsStorage2;
    if (!nvsStorage2.init()) {
        return false;
    }
    
    // 读取配置
    MotorConfig loadedConfig;
    if (!nvsStorage2.loadConfig(loadedConfig)) {
        return false;
    }
    
    // 验证数据是否持久化
    return (loadedConfig.runDuration == 30) &&
           (loadedConfig.stopDuration == 15) &&
           (loadedConfig.cycleCount == 30) &&
           (loadedConfig.autoStart == true);
}