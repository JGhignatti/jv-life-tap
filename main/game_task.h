#pragma once

#include "jvlt.h"
#include "keyboard.h"

void ltapp_handle_input(const jvlt_input_event_t *ev);

// Life delta toast (shows +N/-N briefly after tapping)
#define LIFE_DELTA_SHOW_MS 1200
int16_t ltapp_life_delta(void);
uint32_t ltapp_life_delta_tick(void);

// Dice roll animation
void ltapp_dice_start_anim(void);
bool ltapp_dice_is_animating(void);
uint8_t ltapp_dice_display_value(void);
