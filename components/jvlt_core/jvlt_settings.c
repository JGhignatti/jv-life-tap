#include "jvlt_internal.h"
#include "nvs.h"
#include <string.h>

uint8_t g_brightness;
char    g_player_name[12];

const char *jvlt_player_name(void) {
    return g_player_name;
}

void jvlt_set_player_name(const char *name) {
    if (!name || name[0] == '\0') return;
    strncpy(g_player_name, name, sizeof(g_player_name) - 1);
    g_player_name[sizeof(g_player_name) - 1] = '\0';
    nvs_handle_t nvs;
    if (nvs_open("jvlt", NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_set_str(nvs, "name", g_player_name);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
}

uint8_t jvlt_brightness(void) {
    return g_brightness;
}

void jvlt_set_brightness(uint8_t level) {
    g_brightness = level;
    nvs_handle_t nvs;
    if (nvs_open("jvlt", NVS_READWRITE, &nvs) == ESP_OK) {
        nvs_set_u8(nvs, "brightness", level);
        nvs_commit(nvs);
        nvs_close(nvs);
    }
    g_dirty = true;
}

void jvlt_adjust_brightness(int delta) {
    // Convert to percentage, snap to 5% grid, apply delta
    int pct = (g_brightness * 20 + 127) / 255;  // 0-20 (steps of 5%)
    pct += (delta > 0) ? 1 : -1;
    if (pct < 1) pct = 1;    // min 5%
    if (pct > 20) pct = 20;  // max 100%
    jvlt_set_brightness((uint8_t)((pct * 255 + 10) / 20));
}
