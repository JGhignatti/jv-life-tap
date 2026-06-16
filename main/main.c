#include "jvlt.h"
#include "game_task.h"
#include "touch_input.h"
#include "display.h"
#include "renderer.h"
#include "ui.h"
#include "esp_log.h"

static const char *TAG = "ltapp_main";

void app_main(void) {
    ESP_LOGI(TAG, "=== JV Life Tap ===");

    ltapp_display_init();
    ltapp_render_init();

    jvlt_init(&(jvlt_config_t){
        .on_input  = ltapp_handle_input,
        .on_render = ltapp_ui_render,
    });

    ltapp_display_set_brightness(jvlt_brightness());
    ltapp_touch_init();
}
