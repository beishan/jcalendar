
#ifndef __WIRING_H__
#define __WIRING_H__

// 板型通过 platformio.ini build_flags 定义，不要在此处手动切换
// #define LOLIN32_LITE
// #define ESP32S3_N16R8

#ifdef ESP32S3_N16R8
// ESP32-S3 可用引脚：0-21, 26-48（22-25保留给USB-JTAG不可用）
// 与 LOLIN32_LITE 仅改3个不可用引脚，其余保持一致
#define SPI_MOSI GPIO_NUM_21 // 原GPIO23(S3不可用) → GPIO21
#define SPI_MISO GPIO_NUM_19 // Reserved
#define SPI_SCK GPIO_NUM_18
#define SPI_CS GPIO_NUM_5
#define SPI_DC GPIO_NUM_17
#define SPI_RST GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4
// I2C
#define I2C_SDA GPIO_NUM_21 // SDA保持GPIO21（与MOSI共用，如不接I2C可忽略）
#define I2C_SCL GPIO_NUM_15 // 原GPIO22(S3不可用) → GPIO15
// Other PIN
#define KEY_M GPIO_NUM_14 // S3: 使用 esp_deep_sleep_enable_gpio_wakeup() 替代 ext0 RTC 唤醒（无需改引脚）
#define PIN_LED_R GPIO_NUM_48 // 原GPIO22(S3不可用) → GPIO48（部分S3板载RGB）

#define PIN_ADC GPIO_NUM_32 // ADC
#endif

#ifdef LOLIN32_LITE
// Output PIN
#define SPI_MOSI GPIO_NUM_23
#define SPI_MISO GPIO_NUM_19 // Reserved
#define SPI_SCK GPIO_NUM_18
#define SPI_CS GPIO_NUM_5
#define SPI_DC GPIO_NUM_17
#define SPI_RST GPIO_NUM_16
#define SPI_BUSY GPIO_NUM_4
// I2C
#define I2C_SDA GPIO_NUM_21
#define I2C_SCL GPIO_NUM_22
// Other PIN
#define KEY_M GPIO_NUM_14 // 注意：由于此按键负责唤醒，因此需要选择支持RTC唤醒的PIN脚。
#define PIN_LED_R GPIO_NUM_22

#define PIN_ADC GPIO_NUM_32 // ADC
#endif

#endif
