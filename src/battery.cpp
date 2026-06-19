/*
电压 (V)	近似剩余电量	状态说明
4.20	100%	刚刚充满，充电器断开
4.10	~90%	电量非常充足
4.00	~80%	电量充足
3.90	~60%	中等电量
3.80	~50%	中等电量（接近标称电压）
3.75	~40%	电量偏低
3.70	~30%	标称电压点，但电量已不多
3.65	~20%	低电量
3.50	~10%	极低电量，应立即充电
3.30	0%	放电截止电压，继续放电将损坏电池

充电截止电压 4.2
放电截止电压 3.3
 */
#include "battery.h"

#include <Arduino.h>
#include "wiring.h"

/**
 * 获取电池电压（mV）
 */
int readBatteryVoltage() {
    const uint8_t adcPin = static_cast<uint8_t>(PIN_ADC);
    pinMode(adcPin, INPUT);
    analogReadResolution(12);
#if defined(ESP32)
    analogSetPinAttenuation(adcPin, ADC_11db);
#endif
    delay(10);

    return analogReadMilliVolts(adcPin) * BATTERY_VOLTAGE_DIVIDER;
}
