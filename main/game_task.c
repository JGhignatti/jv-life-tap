#include "renderer.h"
#include "game_task.h"
#include "jvlt.h"
#include "keyboard.h"
#include <string.h>
#include "esp_random.h"

// --- Keyboard callbacks ---
static void ltapp_rename_selected_opp(const char *name) {
    const jvlt_match_t *m = jvlt_match_get();
    if (m) {
        jvlt_match_rename_player(m->cmd_sel_opp, name);
    }
}

static void ltapp_rename_cmd_0(const char *name) {
    const jvlt_match_t *m = jvlt_match_get();
    if (m) jvlt_match_rename_commander(m->cmd_sel_opp, 0, name);
}

static void ltapp_rename_cmd_1(const char *name) {
    const jvlt_match_t *m = jvlt_match_get();
    if (m) jvlt_match_rename_commander(m->cmd_sel_opp, 1, name);
}

// --- Life delta toast ---
static int16_t s_life_delta = 0;
static uint32_t s_life_delta_tick = 0;

int16_t ltapp_life_delta(void) {
    return s_life_delta;
}

uint32_t ltapp_life_delta_tick(void) {
    return s_life_delta_tick;
}

static void life_delta_add(int16_t delta) {
    uint32_t now = xTaskGetTickCount();
    uint32_t elapsed = (now - s_life_delta_tick) * portTICK_PERIOD_MS;
    if (elapsed >= LIFE_DELTA_SHOW_MS) {
        s_life_delta = 0;
    }
    s_life_delta += delta;
    s_life_delta_tick = now;
    jvlt_match_update_life(delta);
}

// --- Coin flip animation ---
#include "coin_anim.h"

static bool s_coin_animating = false;
static uint8_t s_coin_frame = 0;
static uint8_t s_coin_steps_done = 0;
static uint8_t s_coin_total_steps = 0;
static uint32_t s_coin_anim_tick = 0;

void ltapp_coin_start_anim(void) {
    jvlt_coin_flip();  // decide result now
    s_coin_animating = true;
    s_coin_frame = 0;
    s_coin_steps_done = 0;
    // Land on result: heads=0, tails=4. Approach from 1 step before.
    // 2 full rotations (16 steps) + steps to land one before result
    const jvlt_coin_t *c = jvlt_coin_get();
    uint8_t target = c->heads ? COIN_FRAME_HEADS : COIN_FRAME_TAILS;
    // We want the last step to advance INTO the target frame,
    // so we need (total_steps % 8) to equal target
    s_coin_total_steps = 24 + target; // 3 full spins + land on target
    s_coin_anim_tick = xTaskGetTickCount();
    jvlt_mark_dirty();
}

bool ltapp_coin_is_animating(void) {
    if (!s_coin_animating) {
        return false;
    }

    uint32_t now = xTaskGetTickCount();
    uint32_t elapsed = (now - s_coin_anim_tick) * portTICK_PERIOD_MS;

    // Easing: starts at 25ms, ends ~200ms
    uint32_t delay = 25 + (s_coin_steps_done * s_coin_steps_done * 175) / (s_coin_total_steps * s_coin_total_steps);

    if (elapsed >= delay) {
        s_coin_steps_done++;
        s_coin_frame = s_coin_steps_done % COIN_FRAME_COUNT;
        if (s_coin_steps_done >= s_coin_total_steps) {
            s_coin_animating = false;
        }
        s_coin_anim_tick = now;
        jvlt_mark_dirty();
    } else {
        jvlt_mark_dirty();  // keep ticking
    }

    return s_coin_animating;
}

uint8_t ltapp_coin_current_frame(void) {
    return s_coin_frame;
}

// --- Dice animation ---
static bool s_dice_animating = false;
static uint8_t s_dice_display = 0;
static uint8_t s_dice_steps_done = 0;
static uint8_t s_dice_total_steps = 0;
static uint32_t s_dice_anim_tick = 0;

void ltapp_dice_start_anim(void) {
    const jvlt_dice_t *d = jvlt_dice_get();
    uint8_t sides = d->sides ? d->sides : 20;
    jvlt_dice_roll(sides);
    s_dice_animating = true;
    s_dice_steps_done = 0;
    s_dice_total_steps = 18;
    s_dice_display = (esp_random() % sides) + 1;
    s_dice_anim_tick = xTaskGetTickCount();
    jvlt_mark_dirty();
}

bool ltapp_dice_is_animating(void) {
    if (!s_dice_animating) {
        return false;
    }

    uint32_t now = xTaskGetTickCount();
    uint32_t elapsed = (now - s_dice_anim_tick) * portTICK_PERIOD_MS;

    // Starts at 10ms, ends ~160ms
    uint32_t delay = 10 + (s_dice_steps_done * s_dice_steps_done * 150) / (s_dice_total_steps * s_dice_total_steps);

    if (elapsed >= delay) {
        s_dice_steps_done++;
        if (s_dice_steps_done >= s_dice_total_steps) {
            const jvlt_dice_t *d = jvlt_dice_get();
            s_dice_display = d->result;
            s_dice_animating = false;
        } else {
            const jvlt_dice_t *d = jvlt_dice_get();
            uint8_t sides = d->sides ? d->sides : 20;
            s_dice_display = (esp_random() % sides) + 1;
        }
        s_dice_anim_tick = now;
        jvlt_mark_dirty();
    } else {
        jvlt_mark_dirty();
    }

    return s_dice_animating;
}

uint8_t ltapp_dice_display_value(void) {
    return s_dice_display;
}

// --- Input handler ---
void ltapp_handle_input(const jvlt_input_event_t *ev) {
    if (ltapp_kbd_handle_input(ev)) {
        return;
    }

    jvlt_screen_t screen = jvlt_screen();

    switch (screen) {
    case SCREEN_HOME: {
        if (ev->type == INPUT_TAP) {
            int16_t tx = ev->x, ty = ev->y;
            // Extras circle: cx≈194, cy≈170, r=52
            int16_t dx = tx - 194;
            int16_t dy = ty - 170;
            if (dx * dx + dy * dy <= 52 * 52)
                jvlt_set_screen(SCREEN_EXTRAS);
            // Solo: x∈[0,160], y∈[92,147]
            else if (ty >= 92 && ty <= 147 && tx <= 160) {
                if (!jvlt_match_get()) {
                    jvlt_match_begin_solo(3, 40);
                }
                jvlt_set_screen(SCREEN_SOLO_SETUP);
            }
            // Net: x∈[0,160], y∈[147,202]
            else if (ty > 147 && ty <= 202 && tx <= 160)
                jvlt_set_screen(SCREEN_NET_LOBBY);
            // Settings: y≥218
            else if (ty >= 218)
                jvlt_set_screen(SCREEN_SETTINGS_NAME);
        }
        break;
    }

    case SCREEN_SETTINGS_NAME: {
        if (ev->type == INPUT_TAP) {
            ltapp_kbd_open(jvlt_player_name(), (ltapp_kbd_done_cb_t)jvlt_set_player_name);
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_set_screen(SCREEN_HOME);
        } else if (ev->type == INPUT_SWIPE_LEFT) {
            jvlt_set_screen(SCREEN_SETTINGS_BRIGHTNESS);
        }
        break;
    }

    case SCREEN_SETTINGS_BRIGHTNESS: {
        if (ev->type == INPUT_TAP) {
            int16_t tx = ev->x;
            int16_t ty = ev->y;
            // Minus: cx=30, cy=160, r=80
            int16_t dx = tx - 30;
            int16_t dy = ty - 160;
            if (dx * dx + dy * dy <= 80 * 80) {
                jvlt_adjust_brightness(-5);
                ltapp_display_set_brightness(jvlt_brightness());
            } else {
                // Plus: cx=210, cy=160, r=80
                dx = tx - 210;
                dy = ty - 160;
                if (dx * dx + dy * dy <= 80 * 80) {
                    jvlt_adjust_brightness(5);
                    ltapp_display_set_brightness(jvlt_brightness());
                }
            }
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            if (jvlt_match_get())
                jvlt_set_screen(SCREEN_MATCH_SETTINGS);
            else
                jvlt_set_screen(SCREEN_SETTINGS_NAME);
        }
        break;
    }

    case SCREEN_EXTRAS: {
        const jvlt_match_t *m = jvlt_match_get();
        if (ev->type == INPUT_TAP) {
            int16_t ty = ev->y0;
            // Roll a die: y∈[52,112]
            if (ty >= 52 && ty <= 112)
                jvlt_set_screen(SCREEN_DICE);
            // Flip a coin: y∈[116,176]
            else if (ty >= 116 && ty <= 176)
                jvlt_set_screen(SCREEN_FLIP_COIN);
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            if (m == NULL)
                jvlt_set_screen(SCREEN_HOME);
            else
                jvlt_set_screen(SCREEN_MATCH);
        }
        break;
    }

    case SCREEN_SOLO_SETUP: {
        const jvlt_match_t *m = jvlt_match_get();
        if (ev->type == INPUT_TAP) {
            const int16_t tx = ev->x0;
            const int16_t ty = ev->y0;
            // Starting life "<": x∈[0,80], y∈[55,110]
            if (ty >= 55 && ty <= 110 && tx >= 0 && tx <= 80)
                jvlt_match_cycle_starting_life(-1);
            // Starting life ">": x∈[160,240], y∈[55,110]
            else if (ty >= 55 && ty <= 110 && tx >= 160 && tx <= 240)
                jvlt_match_cycle_starting_life(1);
            // Opponents "<": x∈[0,80], y∈[126,181]
            else if (ty >= 126 && ty <= 181 && tx >= 0 && tx <= 80) {
                static const int16_t opps[] = {1, 2, 3, 4, 5};
                uint8_t opp = m ? m->player_count - 1 : 3;
                jvlt_match_set_opponents((uint8_t)jvlt_list_cycle(opps, 5, opp, -1));
            }
            // Opponents ">": x∈[160,240], y∈[126,181]
            else if (ty >= 126 && ty <= 181 && tx >= 160 && tx <= 240) {
                static const int16_t opps[] = {1, 2, 3, 4, 5};
                uint8_t opp = m ? m->player_count - 1 : 3;
                jvlt_match_set_opponents((uint8_t)jvlt_list_cycle(opps, 5, opp, 1));
            }
            // Start match: y≥192
            else if (ty >= 192) {
                int16_t life = m ? m->starting_life : 40;
                uint8_t opp = m ? m->player_count - 1 : 3;
                jvlt_match_begin_solo(opp, life);
                jvlt_set_screen(SCREEN_MATCH);
            }
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_match_end();
            jvlt_set_screen(SCREEN_HOME);
        }
        break;
    }

    case SCREEN_NET_LOBBY: {
        if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_set_screen(SCREEN_HOME);
        }
        break;
    }

    case SCREEN_MATCH: {
        if (ev->type == INPUT_TAP) {
            int ty = ev->y, tx = ev->x;
            // Damage circle: center (70, 200), r=37
            int dx = tx - 70, dy = ty - 200;
            if (dx * dx + dy * dy <= 37 * 37)
                jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE);
            // Extras circle: center (170, 200), r=37
            else if ((tx-170)*(tx-170) + (ty-200)*(ty-200) <= 37*37)
                jvlt_set_screen(SCREEN_EXTRAS);
            // Minus life: x∈[0,119], y∈[0,160]
            else if (tx <= 119 && ty <= 160)
                life_delta_add(-1);
            // Plus life: x∈[121,240], y∈[0,160]
            else if (tx >= 121 && ty <= 160)
                life_delta_add(1);
        } else if (ev->type == INPUT_LONG_PRESS) {
            int ty = ev->y, tx = ev->x;
            // Long press minus: -5
            if (tx <= 119 && ty <= 160)
                life_delta_add(-5);
            // Long press plus: +5
            else if (tx >= 121 && ty <= 160)
                life_delta_add(5);
        } else if (ev->type == INPUT_SWIPE_UP) {
            jvlt_set_screen(SCREEN_MATCH_SETTINGS);
        }
        break;
    }

    case SCREEN_MATCH_CMD_DAMAGE: {
        const jvlt_match_t *m = jvlt_match_get();
        if (ev->type == INPUT_TAP && m) {
            const uint8_t opponents = m->player_count - 1;
            int16_t tx = ev->x, ty = ev->y;
            switch (opponents) {
                case 1: {
                    jvlt_match_select_opponent(1);
                    jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE_OPP);
                    break;
                }

                case 2: {
                    if (tx < LCD_WIDTH_HALF) {
                        jvlt_match_select_opponent(1);
                    } else {
                        jvlt_match_select_opponent(2);
                    }
                    jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE_OPP);
                    break;
                }

                case 3: {
                    const int32_t r2 = 44 * 44;
                    const uint8_t tri_side = 80;
                    const uint8_t tri_h = (uint8_t)(tri_side * 0.866f);
                    const uint8_t y_base = 44 + 42;
                    const int16_t pts[3][2] = {
                        {LCD_WIDTH_HALF - tri_side / 2 - 15, y_base + tri_h},
                        {LCD_WIDTH_HALF, y_base},
                        {LCD_WIDTH_HALF + tri_side / 2 + 15, y_base + tri_h},
                    };
                    for (int i = 0; i < 3; i++) {
                        int16_t dx = tx - pts[i][0];
                        int16_t dy = ty - pts[i][1];
                        if (dx * dx + dy * dy <= r2) {
                            jvlt_match_select_opponent(i + 1);
                            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE_OPP);
                            break;
                        }
                    }
                    break;
                }

                case 4: {
                    const int32_t r2 = 40 * 40;
                    const uint8_t square_side = 80;
                    const uint8_t y_base = 44 + 42;
                    const int16_t pts[4][2] = {
                        {LCD_WIDTH_HALF - square_side / 2 - 20, y_base + square_side},
                        {LCD_WIDTH_HALF - square_side / 2, y_base},
                        {LCD_WIDTH_HALF + square_side / 2, y_base},
                        {LCD_WIDTH_HALF + square_side / 2 + 20, y_base + square_side},
                    };
                    for (int i = 0; i < 4; i++) {
                        int16_t dx = tx - pts[i][0];
                        int16_t dy = ty - pts[i][1];
                        if (dx * dx + dy * dy <= r2) {
                            jvlt_match_select_opponent(i + 1);
                            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE_OPP);
                            break;
                        }
                    }
                    break;
                }

                case 5: {
                    const int32_t r2 = 34 * 34;
                    const uint8_t pent_diam = 96;
                    const uint8_t pent_side = (uint8_t)(pent_diam * 0.5878f);
                    const uint8_t y_base = 44 + 39;
                    const int16_t pts[5][2] = {
                        {LCD_WIDTH_HALF - pent_side / 2 - 15, y_base + pent_diam},
                        {LCD_WIDTH_HALF - pent_diam / 2 - 18, y_base + 20},
                        {LCD_WIDTH_HALF, y_base},
                        {LCD_WIDTH_HALF + pent_diam / 2 + 18, y_base + 20},
                        {LCD_WIDTH_HALF + pent_side / 2 + 15, y_base + pent_diam},
                    };
                    for (int i = 0; i < 5; i++) {
                        int16_t dx = tx - pts[i][0];
                        int16_t dy = ty - pts[i][1];
                        if (dx * dx + dy * dy <= r2) {
                            jvlt_match_select_opponent(i + 1);
                            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE_OPP);
                            break;
                        }
                    }
                    break;
                }
            }
        } else if (ev->type == INPUT_SWIPE_RIGHT)
            jvlt_set_screen(SCREEN_MATCH);
        else if (ev->type == INPUT_SWIPE_LEFT)
            jvlt_set_screen(SCREEN_MATCH_POISON_DAMAGE);
        break;
    }

    case SCREEN_MATCH_CMD_DAMAGE_OPP: {
        const jvlt_match_t *m = jvlt_match_get();
        if (!m || m->cmd_sel_opp == 0 || m->cmd_sel_opp >= m->player_count) {
            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE);
            break;
        }
        const jvlt_player_t *opp = &m->players[m->cmd_sel_opp];
        if (ev->type == INPUT_TAP) {
            int16_t tx = ev->x, ty = ev->y;
            // Edit opponent name: y∈[0,56]
            if (ty <= 56) {
                ltapp_kbd_open(opp->name, ltapp_rename_selected_opp);
                break;
            }
            uint8_t ncmd = opp->num_commanders;
            // Add/remove commander button: y≥220
            if (ty >= 220) {
                if (ncmd < 2)
                    jvlt_match_set_num_commanders(m->cmd_sel_opp, 2);
                else
                    jvlt_match_set_num_commanders(m->cmd_sel_opp, 1);
                break;
            }
            if (ncmd == 1) {
                // Edit cmd name: x∈[70,170], y∈[80,128]
                if (tx >= 70 && tx <= 170 && ty >= 80 && ty <= 128) {
                    ltapp_kbd_open(opp->cmd_name[0], ltapp_rename_cmd_0);
                    break;
                }
                // Minus: cx=40, cy=120, r=30
                int16_t dx = tx - 40, dy = ty - 120;
                if (dx * dx + dy * dy <= 30 * 30)
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 0, -1);
                // Plus: cx=200, cy=120, r=30
                else if ((tx-200)*(tx-200) + (ty-120)*(ty-120) <= 30*30)
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 0, 1);
            } else {
                // 1st commander
                // Edit cmd 1st name: x∈[70,170], y∈[64,112]
                if (tx >= 70 && tx <= 170 && ty >= 64 && ty <= 112) {
                    ltapp_kbd_open(opp->cmd_name[0], ltapp_rename_cmd_0);
                    break;
                }
                // Minus 1st: cx=40, cy=104, r=30
                int16_t dx = tx - 40, dy = ty - 104;
                if (dx * dx + dy * dy <= 30 * 30) {
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 0, -1);
                    break;
                }
                // Plus 1st: cx=200, cy=104, r=30
                if ((tx-200)*(tx-200) + (ty-104)*(ty-104) <= 30*30) {
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 0, 1);
                    break;
                }
                // 2nd commander
                // Edit cmd 2nd name: x∈[70,170], y∈[128,176]
                if (tx >= 70 && tx <= 170 && ty >= 128 && ty <= 176) {
                    ltapp_kbd_open(opp->cmd_name[1], ltapp_rename_cmd_1);
                    break;
                }
                // Minus 2nd: cx=40, cy=168, r=30
                dx = tx - 40; dy = ty - 168;
                if (dx * dx + dy * dy <= 30 * 30) {
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 1, -1);
                    break;
                }
                // Plus 2nd: cx=200, cy=168, r=30
                if ((tx-200)*(tx-200) + (ty-168)*(ty-168) <= 30*30) {
                    jvlt_match_add_cmd_dmg(m->cmd_sel_opp, 1, 1);
                    break;
                }
            }
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE);
        }
        break;
    }

    case SCREEN_MATCH_POISON_DAMAGE: {
        if (ev->type == INPUT_TAP) {
            int tx = ev->x, ty = ev->y;
            // Minus poison: cx=30, cy=160, r=60
            int dx = tx - 30, dy = ty - 160;
            if (dx * dx + dy * dy <= 60 * 60)
                jvlt_match_add_poison(-1);
            // Plus poison: cx=210, cy=160, r=60
            else if ((tx-210)*(tx-210) + (ty-160)*(ty-160) <= 60*60)
                jvlt_match_add_poison(1);
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_set_screen(SCREEN_MATCH_CMD_DAMAGE);
        }
        break;
    }

    case SCREEN_MATCH_SETTINGS: {
        if (ev->type == INPUT_TAP) {
            int16_t tx = ev->x0, ty = ev->y0;
            // Reset: x∈[20,220], y∈[66,112]
            if (tx >= 20 && tx <= 220 && ty >= 66 && ty <= 112)
                jvlt_set_screen(SCREEN_CONFIRM_RESET);
            // Leave: x∈[20,220], y∈[128,174]
            else if (tx >= 20 && tx <= 220 && ty >= 128 && ty <= 174)
                jvlt_set_screen(SCREEN_CONFIRM_LEAVE);
        } else if (ev->type == INPUT_SWIPE_LEFT) {
            jvlt_set_screen(SCREEN_SETTINGS_BRIGHTNESS);
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_set_screen(SCREEN_MATCH);
        }
        break;
    }

    case SCREEN_CONFIRM_RESET: {
        if (ev->type == INPUT_TAP) {
            int16_t ty = ev->y0;
            // Action (confirm): y∈[141,199]
            if (ty >= 141 && ty <= 199) {
                jvlt_match_reset_counters();
                jvlt_set_screen(SCREEN_MATCH);
            }
            // Cancel: y∈[201,239]
            else if (ty >= 201) {
                jvlt_set_screen(SCREEN_MATCH_SETTINGS);
            }
        }
        break;
    }

    case SCREEN_CONFIRM_LEAVE: {
        if (ev->type == INPUT_TAP) {
            int16_t ty = ev->y0;
            // Action (confirm): y∈[141,199]
            if (ty >= 141 && ty <= 199) {
                jvlt_match_end();
                jvlt_set_screen(SCREEN_HOME);
            }
            // Cancel: y∈[201,239]
            else if (ty >= 201) {
                jvlt_set_screen(SCREEN_MATCH_SETTINGS);
            }
        }
        break;
    }

    case SCREEN_DICE: {
        if (ltapp_dice_is_animating())
            break;
        if (ev->type == INPUT_TAP) {
            // Check if tap is on cycle areas (circles at edges, cy=120, r=40)
            int16_t dx, dy;
            int32_t dist2;
            // Cycle -1: cx=12, cy=120, r=40
            dx = ev->x - 12; dy = ev->y - 120;
            dist2 = dx*dx + dy*dy;
            if (dist2 <= 40*40) {
                jvlt_dice_cycle_sides(-1);
                s_dice_display = 0;
            } else {
                // Cycle +1: cx=228, cy=120, r=40
                dx = ev->x - 228; dy = ev->y - 120;
                dist2 = dx*dx + dy*dy;
                if (dist2 <= 40*40) {
                    jvlt_dice_cycle_sides(+1);
                    s_dice_display = 0;
                } else {
                    ltapp_dice_start_anim();
                }
            }
        } else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_dice_reset();
            s_dice_display = 0;
            jvlt_set_screen(SCREEN_EXTRAS);
        }
        break;
    }

    case SCREEN_FLIP_COIN: {
        if (ltapp_coin_is_animating())
            break;  // block input during animation
        if (ev->type == INPUT_TAP)
            ltapp_coin_start_anim();
        else if (ev->type == INPUT_SWIPE_RIGHT) {
            jvlt_coin_reset();
            jvlt_set_screen(SCREEN_EXTRAS);
        }
        break;
    }

    default:
        break;
    }
}
