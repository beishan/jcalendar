
#ifndef __WIRING_H__
#define __WIRING_H__

// ======== 开发板选择 ========
// 取消注释需要使用的开发板宏定义，只能选择一个
#define LOLIN32_LITE
// #define ESP32_DEVKIT

// ============================================================
// LOLIN32_LITE 引脚定义
// ============================================================
#ifdef LOLIN32_LITE

// 墨水屏 SPI (使用 VSPI)
#define SPI_MOSI GPIO_NUM_23
#define SPI_MISO GPIO_NUM_19  // Reserved
#define SPI_SCK  GPIO_NUM_18
#define SPI_CS   GPIO_NUM_5
#define SPI_DC   GPIO_NUM_17
#define SPI_RST  GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4

// I2C
#define I2C_SDA  GPIO_NUM_21
#define I2C_SCL  GPIO_NUM_22

// 按键
#define KEY_M    GPIO_NUM_14  // 注意：需要支持RTC唤醒的引脚

// LED
#define PIN_LED_R GPIO_NUM_22

// ADC
#define PIN_ADC  GPIO_NUM_32

#endif // LOLIN32_LITE

// ============================================================
// ESP32_DEVKIT 引脚定义
// ============================================================
#ifdef ESP32_DEVKIT

// 墨水屏 SPI (使用 VSPI)
#define SPI_MOSI GPIO_NUM_23   // D23
#define SPI_MISO GPIO_NUM_19   // D19 (Reserved)
#define SPI_SCK  GPIO_NUM_18   // D18
#define SPI_CS   GPIO_NUM_5    // D5
#define SPI_DC   GPIO_NUM_25   // D25
#define SPI_RST  GPIO_NUM_26   // D26
#define SPI_BUSY GPIO_NUM_4    // D4

// 按键
#define KEY_M    GPIO_NUM_14   // D14

// LED
#define PIN_LED_R GPIO_NUM_22  // D22

// ADC
#define PIN_ADC  GPIO_NUM_32   // D32

// I2C (预留)
#define I2C_SDA  GPIO_NUM_27   // D27
#define I2C_SCL  GPIO_NUM_13   // D13

#endif // ESP32_DEVKIT

#endif // __WIRING_H__
