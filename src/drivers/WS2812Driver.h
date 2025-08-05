#ifndef WS2812_DRIVER_H
#define WS2812_DRIVER_H

#include <Arduino.h>

class WS2812Driver {
private:
    uint8_t pin;
    uint16_t ledCount;
    uint8_t* ledData;  // 存储LED颜色数据
    uint8_t brightness;  // 亮度值

public:
    /**
     * @brief 构造函数
     * @param pin GPIO引脚号
     * @param ledCount LED数量
     */
    WS2812Driver(uint8_t pin = 21, uint16_t ledCount = 1);

    /**
     * @brief 析构函数
     */
    ~WS2812Driver();

    /**
     * @brief 初始化WS2812驱动
     */
    void begin();

    /**
     * @brief 设置LED颜色 (RGB)
     * @param index LED索引
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     */
    void setColor(uint16_t index, uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief 设置LED颜色 (HSV)
     * @param index LED索引
     * @param h 色相 (0-255)
     * @param s 饱和度 (0-255)
     * @param v 明度 (0-255)
     */
    void setColorHSV(uint16_t index, uint8_t h, uint8_t s, uint8_t v);

    /**
     * @brief 设置所有LED颜色 (RGB)
     * @param r 红色分量 (0-255)
     * @param g 绿色分量 (0-255)
     * @param b 蓝色分量 (0-255)
     */
    void setAllColor(uint8_t r, uint8_t g, uint8_t b);

    /**
     * @brief 设置所有LED颜色 (HSV)
     * @param h 色相 (0-255)
     * @param s 饱和度 (0-255)
     * @param v 明度 (0-255)
     */
    void setAllColorHSV(uint8_t h, uint8_t s, uint8_t v);

    /**
     * @brief 设置亮度
     * @param brightness 亮度值 (0-255)
     */
    void setBrightness(uint8_t brightness);

    /**
     * @brief 更新显示
     */
    void show();

    /**
     * @brief 清除所有LED
     */
    void clear();

private:
    /**
     * @brief HSV转RGB
     * @param h 色相 (0-255)
     * @param s 饱和度 (0-255)
     * @param v 明度 (0-255)
     * @param r 红色分量输出
     * @param g 绿色分量输出
     * @param b 蓝色分量输出
     */
    void hsvToRgb(uint8_t h, uint8_t s, uint8_t v, uint8_t& r, uint8_t& g, uint8_t& b);
};

#endif // WS2812_DRIVER_H