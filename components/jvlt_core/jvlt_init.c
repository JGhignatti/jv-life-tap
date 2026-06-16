#include "jvlt_internal.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs.h"
#include "freertos/task.h"
#include <string.h>

static const char *TAG = "jvlt";

// Dirty flag
volatile bool g_dirty = true;

// Queues
QueueHandle_t g_pkt_queue;
QueueHandle_t g_input_queue;

// Internal forward decls
void jvlt_net_dispatch(const jvlt_packet_t *pkt);
void jvlt_net_beacon_tick(void);

static void beacon_timer_cb(void *arg) { jvlt_net_beacon_tick(); }

static void render_task(void *arg) {
    jvlt_render_cb_t cb = (jvlt_render_cb_t)arg;
    while (1) {
        if (g_dirty) {
            g_dirty = false;
            cb();
        }
        vTaskDelay(pdMS_TO_TICKS(33));
    }
}

static void game_task(void *arg) {
    jvlt_input_handler_t handle_input = (jvlt_input_handler_t)arg;
    jvlt_input_event_t ev;
    jvlt_packet_t pkt;

    while (1) {
        while (xQueueReceive(g_pkt_queue, &pkt, 0) == pdTRUE)
            jvlt_net_dispatch(&pkt);

        if (xQueueReceive(g_input_queue, &ev, pdMS_TO_TICKS(20)) == pdTRUE)
            handle_input(&ev);
    }
}

void jvlt_init(const jvlt_config_t *cfg) {
    // Comms
    jvlt_comm_init();

    // Queues
    g_pkt_queue = xQueueCreate(8, sizeof(jvlt_packet_t));
    g_input_queue = xQueueCreate(16, sizeof(jvlt_input_event_t));

    // Settings — load from NVS
    g_brightness = 200;
    strncpy(g_player_name, "Player", sizeof(g_player_name) - 1);

    nvs_handle_t nvs;
    if (nvs_open("jvlt", NVS_READONLY, &nvs) == ESP_OK) {
        size_t len = sizeof(g_player_name);
        nvs_get_str(nvs, "name", g_player_name, &len);
        nvs_get_u8(nvs, "brightness", &g_brightness);
        nvs_close(nvs);
    }

    // Navigation — start at home
    g_screen = SCREEN_HOME;

    ESP_LOGI(TAG, "Player: %s, brightness: %d", g_player_name, g_brightness);

    // Beacon timer
    const esp_timer_create_args_t timer_args = { .callback = beacon_timer_cb, .name = "beacon" };
    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 1000000);

    // Tasks
    xTaskCreate(game_task, "game", 4096, (void *)cfg->on_input, 5, NULL);
    xTaskCreate(render_task, "render", 4096, (void *)cfg->on_render, 3, NULL);
}

// Dirty flag impls
bool jvlt_is_dirty(void) { return g_dirty; }
void jvlt_mark_dirty(void) { g_dirty = true; }
void jvlt_clear_dirty(void) { g_dirty = false; }

// Input queue accessor
QueueHandle_t jvlt_input_queue(void) { return g_input_queue; }
