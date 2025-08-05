#ifndef NVS_STORAGE_TEST_H
#define NVS_STORAGE_TEST_H

#include <Arduino.h>
#include "../drivers/NVSStorageDriver.h"

class NVSStorageTest {
public:
    /**
     * 运行所有NVS存储测试
     * @return 测试是否全部通过
     */
    static bool runAllTests();
    
    /**
     * 测试NVS存储初始化功能
     * @return 测试是否通过
     */
    static bool testInit();
    
    /**
     * 测试MotorConfig配置保存功能
     * @return 测试是否通过
     */
    static bool testSaveConfig();
    
    /**
     * 测试MotorConfig配置读取功能
     * @return 测试是否通过
     */
    static bool testLoadConfig();
    
    /**
     * 测试MotorConfig配置删除功能
     * @return 测试是否通过
     */
    static bool testDeleteConfig();
    
    /**
     * 测试数据持久化功能
     * @return 测试是否通过
     */
    static bool testPersistence();
};

#endif // NVS_STORAGE_TEST_H