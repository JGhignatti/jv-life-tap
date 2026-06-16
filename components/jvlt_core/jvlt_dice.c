#include "jvlt_internal.h"
#include "esp_random.h"

jvlt_dice_t g_dice;

const jvlt_dice_t *jvlt_dice_get(void) { return &g_dice; }

void jvlt_dice_roll(uint8_t sides) {
    if (sides < 2) sides = 2;
    g_dice.sides = sides;
    g_dice.result = (esp_random() % sides) + 1;
    g_dirty = true;
}

void jvlt_dice_reset(void) {
    g_dice.sides = 0;
    g_dice.result = 0;
}

static const int16_t s_dice_options[] = {4, 6, 8, 12, 20};
#define DICE_OPTIONS_COUNT 5

void jvlt_dice_cycle_sides(int8_t dir) {
    int16_t sides = g_dice.sides ? g_dice.sides : 20;
    g_dice.sides = (uint8_t)jvlt_list_cycle(s_dice_options, DICE_OPTIONS_COUNT, sides, dir);
    g_dice.result = 0;
    g_dirty = true;
}

const int16_t *jvlt_dice_peek_sides(int8_t dir) {
    int16_t sides = g_dice.sides ? g_dice.sides : 20;
    return jvlt_list_peek(s_dice_options, DICE_OPTIONS_COUNT, sides, dir);
}
