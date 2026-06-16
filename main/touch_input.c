#include "touch_input.h"
#include "jvlt.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include <string.h>

static const char *TAG = "lt_touch";

#define CST816D_ADDR       0x15
#define REG_GESTURE_ID     0x01
#define REG_FINGER_NUM     0x02
#define REG_XPOS_H         0x03
#define REG_XPOS_L         0x04
#define REG_YPOS_H         0x05
#define REG_YPOS_L         0x06

// CST816D gesture IDs
#define GEST_NONE          0x00
#define GEST_SWIPE_UP      0x01
#define GEST_SWIPE_DOWN    0x02
#define GEST_SWIPE_LEFT    0x03
#define GEST_SWIPE_RIGHT   0x04
#define GEST_SINGLE_TAP    0x05
#define GEST_DOUBLE_TAP    0x0B
#define GEST_LONG_PRESS    0x0C

static i2c_master_bus_handle_t s_bus;
static i2c_master_dev_handle_t s_dev;
static SemaphoreHandle_t s_touch_sem;

// Gesture detection state
static bool     s_touching = false;
static bool     s_long_fired = false;
static int16_t  s_start_x, s_start_y;
static uint32_t s_touch_start_tick;

#define LONG_PRESS_MS  500
#define SWIPE_THRESH   30

static esp_err_t touch_read_reg(uint8_t reg, uint8_t *buf, size_t len) {
    return i2c_master_transmit_receive(s_dev, &reg, 1, buf, len, 50);
}

static void touch_reset(void) {
    gpio_set_direction(TOUCH_RST_PIN, GPIO_MODE_OUTPUT);
    gpio_set_level(TOUCH_RST_PIN, 0);
    vTaskDelay(pdMS_TO_TICKS(10));
    gpio_set_level(TOUCH_RST_PIN, 1);
    vTaskDelay(pdMS_TO_TICKS(50));
}

static void send_event(jvlt_input_type_t type, int16_t x, int16_t y) {
    jvlt_input_event_t ev = { .type = type, .x = x, .y = y, .x0 = s_start_x, .y0 = s_start_y };
    xQueueSend(jvlt_input_queue(), &ev, 0);
}

static void IRAM_ATTR touch_isr(void *arg) {
    BaseType_t woken = pdFALSE;
    xSemaphoreGiveFromISR(s_touch_sem, &woken);
    if (woken) portYIELD_FROM_ISR();
}

static void touch_task(void *arg) {
    uint8_t data[6];

    while (1) {
        // Block until INT pin fires (or timeout for release detection while touching)
        TickType_t wait = s_touching ? pdMS_TO_TICKS(50) : portMAX_DELAY;
        xSemaphoreTake(s_touch_sem, wait);

        if (touch_read_reg(REG_GESTURE_ID, data, 6) != ESP_OK) continue;

        uint8_t gesture  = data[0];
        uint8_t fingers  = data[1];
        int16_t x = ((data[2] & 0x0F) << 8) | data[3];
        int16_t y = ((data[4] & 0x0F) << 8) | data[5];

        bool touching = (fingers > 0);

        if (touching && !s_touching) {
            // Finger down
            s_touching = true;
            s_long_fired = false;
            s_start_x = x;
            s_start_y = y;
            s_touch_start_tick = xTaskGetTickCount();
            send_event(INPUT_TOUCH_DOWN, x, y);
        } else if (touching && s_touching) {
            // Still held — check for long press
            if (!s_long_fired) {
                uint32_t held_ms = (xTaskGetTickCount() - s_touch_start_tick) * portTICK_PERIOD_MS;
                if (held_ms >= LONG_PRESS_MS) {
                    send_event(INPUT_LONG_PRESS, x, y);
                    s_long_fired = true;
                }
            }
            send_event(INPUT_TOUCH_MOVE, x, y);
        } else if (!touching && s_touching) {
            // Finger up — classify gesture (skip if long press already fired)
            s_touching = false;
            if (s_long_fired) continue;
            uint32_t held_ms = (xTaskGetTickCount() - s_touch_start_tick) * portTICK_PERIOD_MS;
            int16_t dx = x - s_start_x;
            int16_t dy = y - s_start_y;

            // Check hardware gesture register first
            if (gesture == GEST_SWIPE_UP)         send_event(INPUT_SWIPE_DOWN, x, y);
            else if (gesture == GEST_SWIPE_DOWN)  send_event(INPUT_SWIPE_UP, x, y);
            else if (gesture == GEST_SWIPE_LEFT)  send_event(INPUT_SWIPE_LEFT, x, y);
            else if (gesture == GEST_SWIPE_RIGHT) send_event(INPUT_SWIPE_RIGHT, x, y);
            else if (gesture == GEST_LONG_PRESS)  send_event(INPUT_LONG_PRESS, x, y);
            else if (gesture == GEST_DOUBLE_TAP)  send_event(INPUT_DOUBLE_TAP, x, y);
            else if (gesture == GEST_SINGLE_TAP)  send_event(INPUT_TAP, x, y);
            // Software fallback
            else if (dy < -SWIPE_THRESH)          send_event(INPUT_SWIPE_UP, x, y);
            else if (dy > SWIPE_THRESH)           send_event(INPUT_SWIPE_DOWN, x, y);
            else if (dx < -SWIPE_THRESH)          send_event(INPUT_SWIPE_LEFT, x, y);
            else if (dx > SWIPE_THRESH)           send_event(INPUT_SWIPE_RIGHT, x, y);
            else                                  send_event(INPUT_TAP, x, y);

            send_event(INPUT_TOUCH_UP, x, y);
        }
    }
}

esp_err_t ltapp_touch_init(void) {
    touch_reset();

    // I2C bus
    i2c_master_bus_config_t bus_cfg = {
        .i2c_port = -1,
        .sda_io_num = TOUCH_I2C_SDA,
        .scl_io_num = TOUCH_I2C_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_cfg, &s_bus));

    // Device
    i2c_device_config_t dev_cfg = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = CST816D_ADDR,
        .scl_speed_hz = TOUCH_I2C_FREQ_HZ,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(s_bus, &dev_cfg, &s_dev));

    // INT pin — interrupt on falling edge (CST816D pulls low on touch)
    s_touch_sem = xSemaphoreCreateBinary();
    gpio_config_t int_cfg = {
        .pin_bit_mask = (1ULL << TOUCH_INT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .intr_type = GPIO_INTR_NEGEDGE,
    };
    gpio_config(&int_cfg);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(TOUCH_INT_PIN, touch_isr, NULL);

    xTaskCreate(touch_task, "touch", 2048, NULL, 5, NULL);
    ESP_LOGI(TAG, "Touch input ready (CST816D @ 0x%02X, interrupt-driven)", CST816D_ADDR);
    return ESP_OK;
}
