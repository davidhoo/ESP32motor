#include <Arduino.h>
#include "controllers/MainController.h"

// 主控制器实例
MainController& mainController = MainController::getInstance();

void setup() {
    // 初始化串口
    Serial.begin(115200);
    delay(1000);
    
    // 初始化主控制器
    if (!mainController.init()) {
        Serial.println("❌ 主控制器初始化失败，系统停止");
        while (1) {
            delay(1000);
        }
    }
}

void loop() {
    // 运行主控制器
    mainController.run();
}