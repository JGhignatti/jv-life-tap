#include "jvlt_internal.h"
#include "esp_random.h"

jvlt_coin_t g_coin;

const jvlt_coin_t *jvlt_coin_get(void) { return &g_coin; }

void jvlt_coin_flip(void) {
    g_coin.flipped = true;
    g_coin.heads = (esp_random() & 1) != 0;
    g_dirty = true;
}

void jvlt_coin_reset(void) {
    g_coin.flipped = false;
    g_coin.heads = false;
}
