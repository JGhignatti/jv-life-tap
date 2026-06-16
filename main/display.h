#pragma once

#include "esp_err.h"
#include <stdint.h>

// ESP32-2424S012 display pins (GC9A01, SPI)
#define LCD_SPI_HOST   SPI2_HOST
#define LCD_PIN_SCLK   GPIO_NUM_6
#define LCD_PIN_MOSI   GPIO_NUM_7
#define LCD_PIN_DC     GPIO_NUM_2
#define LCD_PIN_CS     GPIO_NUM_10
#define LCD_PIN_BL     GPIO_NUM_3

#define LCD_WIDTH      240
#define LCD_HEIGHT     240

esp_err_t ltapp_display_init(void);
void ltapp_display_flush(const uint16_t *buf);
void ltapp_display_flush_area(int x, int y, int w, int h, const uint16_t *buf);
void ltapp_display_backlight(bool on);
void ltapp_display_set_brightness(uint8_t level);
