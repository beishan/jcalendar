#ifndef __WIRING_H__
#define __WIRING_H__

#include "driver/gpio.h"

#if defined(JCAL_BOARD_ESP32_S3_N16R8)
// ESP32-S3-N16R8, 16 MB flash + 8 MB OPI PSRAM.
// Keep external e-paper wiring away from module flash/PSRAM pins.
#define SPI_MOSI GPIO_NUM_11
#define SPI_MISO GPIO_NUM_13 // Reserved by the e-paper wiring, but passed to SPI.begin().
#define SPI_SCK GPIO_NUM_12
#define SPI_CS GPIO_NUM_10
#define SPI_DC GPIO_NUM_9
#define SPI_RST GPIO_NUM_8
#define SPI_BUSY GPIO_NUM_7

#define I2C_SDA GPIO_NUM_18
#define I2C_SCL GPIO_NUM_17

#define KEY_M GPIO_NUM_14 // Must be RTC-capable for deep-sleep wakeup.
#define PIN_LED_R GPIO_NUM_48
#define PIN_LED_ON LOW
#define PIN_LED_OFF HIGH

#define PIN_ADC GPIO_NUM_1 // ADC1_CH0
#define BATTERY_VOLTAGE_DIVIDER 2

#else
#define LOLIN32_LITE

#ifdef LOLIN32_LITE
// Output PIN
#define SPI_MOSI GPIO_NUM_23
#define SPI_MISO GPIO_NUM_19 // Reserved
#define SPI_SCK GPIO_NUM_18
#define SPI_CS GPIO_NUM_5
#define SPI_DC GPIO_NUM_17
#define SPI_RST GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4

#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22

#define KEY_M GPIO_NUM_14 // Must be RTC-capable for deep-sleep wakeup.
#define PIN_LED_R GPIO_NUM_22
#define PIN_LED_ON LOW
#define PIN_LED_OFF HIGH

#define PIN_ADC GPIO_NUM_32 // ADC
#define BATTERY_VOLTAGE_DIVIDER 2
#endif

#endif

#endif
