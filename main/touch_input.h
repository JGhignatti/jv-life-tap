#pragma once

#include "esp_err.h"
#include "driver/i2c_master.h"

// ESP32-2424S012 touch pins (CST816D)
#define TOUCH_I2C_SDA      GPIO_NUM_4
#define TOUCH_I2C_SCL      GPIO_NUM_5
#define TOUCH_INT_PIN      GPIO_NUM_0
#define TOUCH_RST_PIN      GPIO_NUM_1
#define TOUCH_I2C_FREQ_HZ  400000

esp_err_t ltapp_touch_init(void);
