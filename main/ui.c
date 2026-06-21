#include "ui.h"
#include "game_task.h"
#include "renderer.h"
#include "icons.h"
#include "coin_anim.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
#include "esp_random.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static void draw_border(void) {
    ltapp_render_circle(LCD_WIDTH_HALF, LCD_HEIGHT_HALF, 119, COLOR_MAGENTA);
}

static void draw_page_dots(uint8_t current, uint8_t total, uint8_t y) {
    uint8_t dot_r = 3;
    uint8_t gap = 6;
    uint8_t total_w = (2 * dot_r + gap) * total - gap;
    for (uint8_t i = 0; i < total; i++) {
        if (i == current)
            ltapp_render_circle_fill(LCD_WIDTH_HALF - total_w / 2 + i * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_MAGENTA);
        else
            ltapp_render_circle(LCD_WIDTH_HALF - total_w / 2 + i * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_MAGENTA);
    }
}

typedef enum { BUBBLE_SMALL, BUBBLE_MEDIUM, BUBBLE_LARGE } bubble_size_t;

static void render_cmd_damage_opps_bubble(uint8_t opp_index, uint8_t x, uint8_t y, uint16_t color, bubble_size_t size) {
    const jvlt_match_t *m = jvlt_match_get();
    const jvlt_player_t *me = &m->players[m->my_index];
    const jvlt_player_t *opp = &m->players[opp_index + 1];

    if (opp) {
        int icon_sz, name_font, dmg_font, name_gap, dmg_gap, dmg_space_gap;
        const uint8_t *icon;
        if (size == BUBBLE_LARGE) {
            icon = icon_user;
            icon_sz = 30;
            name_font = FONT_BASE_9;
            dmg_font = FONT_BASE_18;
            name_gap = icon_sz / 2 + 8;
            dmg_gap = icon_sz / 2 + 8;
            dmg_space_gap = 4;
        } else if (size == BUBBLE_MEDIUM) {
            icon = icon_user_24;
            icon_sz = 24;
            name_font = FONT_BASE_6;
            dmg_font = FONT_BASE_12;
            name_gap = icon_sz / 2 + 8;
            dmg_gap = icon_sz / 2 + 8;
            dmg_space_gap = 4;
        } else {
            icon = icon_user_24;
            icon_sz = 24;
            name_font = FONT_BASE_6;
            dmg_font = FONT_BASE_9;
            name_gap = icon_sz / 2 + 2;
            dmg_gap = icon_sz / 2 + 2;
            dmg_space_gap = 2;
        }

        ltapp_render_string(x - ltapp_render_text_width(opp->name, name_font, 1) / 2, y - name_gap - ltapp_render_text_height(name_font, 1), opp->name, color, name_font, 1);
        ltapp_render_icon(x - icon_sz / 2, y - icon_sz / 2, icon, icon_sz, icon_sz, color);

        uint8_t num_commanders = opp->num_commanders;
        char cmd_dmg_buf[24];
        if (num_commanders == 1) {
            snprintf(cmd_dmg_buf, sizeof(cmd_dmg_buf), "%d", me->cmd_dmg[opp_index + 1][0]);
        } else {
            snprintf(cmd_dmg_buf, sizeof(cmd_dmg_buf), "%d , %d", me->cmd_dmg[opp_index + 1][0], me->cmd_dmg[opp_index + 1][1]);
        }
        ltapp_render_string_spaced(x - ltapp_render_text_width_spaced(cmd_dmg_buf, dmg_font, 1, 4) / 2, y + dmg_gap, cmd_dmg_buf, COLOR_RED, dmg_font, 1, dmg_space_gap);
    }
}

static void render_confirm(const char *action, const char *description, uint16_t color) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    
    uint8_t line_y = LCD_HEIGHT_HALF + 20;
    uint8_t line_w = ltapp_circle_width_at(LCD_WIDTH_HALF, line_y - LCD_HEIGHT_HALF);
    ltapp_render_rect_fill(0, line_y, LCD_WIDTH, LCD_HEIGHT - line_y, COLOR_BLACK);
    ltapp_render_hline(LCD_WIDTH_HALF - line_w / 2 + 1, line_y, line_w - 2, COLOR_MAGENTA);

    // Question
    ltapp_render_string_centered(LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2), description, COLOR_WHITE, FONT_BASE_12, 1);

    // Action
    ltapp_render_string_centered(line_y + 30 - ltapp_render_text_height(FONT_BASE_18, 1) / 2, action, color, FONT_BASE_18, 1);

    // Cancel
    line_y += 60;
    line_w = ltapp_circle_width_at(LCD_WIDTH_HALF, line_y - LCD_HEIGHT_HALF) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - line_w / 2, line_y, line_w, 2, 2, COLOR_WHITE);
    ltapp_render_string_centered(line_y + (LCD_HEIGHT - line_y) / 2 - ltapp_render_text_height(FONT_BASE_9, 1) / 2, "CANCEL", COLOR_WHITE, FONT_BASE_9, 1);

    // TOUCH AREAS
    // - Action touch area
    // // x0: 0
    // // y0: LCD_HEIGHT_HALF + 21
    // // x1: x0 + LCD_WIDTH
    // // y1: y0 + 58
    // ltapp_render_rect(0, LCD_HEIGHT_HALF + 21, LCD_WIDTH, 58, COLOR_WHITE);

    // // - Cancel touch area
    // // x0: 0
    // // y0: LCD_HEIGHT_HALF + 81
    // // x1: x0 + LCD_WIDTH
    // // y1: y0 + 58
    // ltapp_render_rect(0, LCD_HEIGHT_HALF + 81, LCD_WIDTH, 58, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_home(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    const uint8_t bottom_y = LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_9, 1) - 12;
    const uint8_t w_at_bottom_y = ltapp_circle_width_at(119, bottom_y - LCD_HEIGHT_HALF);

    // BORDER
    draw_border();
    ltapp_render_rect_fill(LCD_WIDTH_HALF - w_at_bottom_y / 2, bottom_y, w_at_bottom_y, LCD_HEIGHT - bottom_y, COLOR_BLACK);

    ltapp_render_hline(LCD_WIDTH_HALF - w_at_bottom_y / 2, bottom_y, w_at_bottom_y, COLOR_MAGENTA);

    const uint8_t extras_outer_r = 56;
    const uint8_t extras_cx = LCD_WIDTH_HALF + w_at_bottom_y / 2;
    const uint8_t extras_cy = bottom_y - extras_outer_r + 8;

    ltapp_render_circle(extras_cx, extras_cy, extras_outer_r, COLOR_MAGENTA);
    ltapp_render_rect_fill(extras_cx - 27, bottom_y, 28, 8, COLOR_BLACK);
    ltapp_render_rect_fill(extras_cx, extras_cy - 34, extras_outer_r, extras_outer_r + 26, COLOR_BLACK);

    // Title
    uint8_t y = 16;

    ltapp_render_string_centered(y, "JV", COLOR_MAGENTA, FONT_BASE_12, 1);

    y += ltapp_render_text_height(FONT_BASE_12, 1) + 4;
    ltapp_render_string_spaced_centered(y, "Life Tap", COLOR_MAGENTA, FONT_BASE_18, 1, 7);

    y += ltapp_render_text_height(FONT_BASE_18, 1) + 12;
    const uint8_t top_line_w = ltapp_render_text_width_spaced("Life Tap", FONT_BASE_18, 1, 7) + 8;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_line_w / 2, y, top_line_w, 2, 2, COLOR_MAGENTA);
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_line_w / 2, y + 1, top_line_w, 2, 2, COLOR_MAGENTA);

    // Menu options
    const uint8_t ys[2] = {
        (y + bottom_y) / 2 - ltapp_render_text_height(FONT_BASE_18, 1) - 16,
        (y + bottom_y) / 2 + 8,
    };
    const uint8_t indicator_w = ltapp_render_text_width_spaced("> ", FONT_BASE_12, 1, 7);
    const int8_t indicator_delta_y = ltapp_render_text_height(FONT_BASE_18, 1) / 2 - ltapp_render_text_height(FONT_BASE_12, 1) / 2;

    // Solo play
    ltapp_render_string_spaced(20, ys[0] + indicator_delta_y, "> ", COLOR_WHITE, FONT_BASE_12, 1, 7);
    ltapp_render_string(20 + indicator_w, ys[0], "SOLO", COLOR_CYAN, FONT_BASE_18, 1);

    // Net play
    ltapp_render_string_spaced(32, ys[1] + indicator_delta_y, "> ", COLOR_WHITE, FONT_BASE_12, 1, 7);
    ltapp_render_string(32 + indicator_w, ys[1], "NET", COLOR_GREEN, FONT_BASE_18, 1);

    // Settings
    ltapp_render_string_centered(bottom_y + 4, "SETTINGS", COLOR_WHITE, FONT_BASE_9, 1);

    // Extras
    const uint8_t extras_r = extras_outer_r - 6;
    ltapp_render_circle(extras_cx, extras_cy, extras_r, COLOR_YELLOW);

    ltapp_render_icon(extras_cx - 30, extras_cy - 18, icon_die_60, 60, 60, COLOR_YELLOW);

    const uint8_t extras_width = ltapp_render_text_width("EXTRAS", FONT_BASE_9, 1);
    ltapp_render_string(extras_cx - extras_width / 2, extras_cy - 36, "EXTRAS", COLOR_YELLOW, FONT_BASE_9, 1);

    // TOUCH AREAS
    // uint8_t y_touch = (y + bottom_y) / 2 - ltapp_render_text_height(FONT_BASE_18, 1) - 32;

    // // - Solo touch area
    // // x0: 0
    // // y0: y_touch - 16
    // // x1: x0 + ltapp_render_text_width("> Solo", FONT_BASE, 2) + 32
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 23
    // ltapp_render_rect(0, y_touch, ltapp_render_text_width("SOLO", FONT_BASE_18, 1) + 36 + indicator_w, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // y_touch = (y + bottom_y) / 2;
    // // - Net touch area
    // // x0: 0
    // // y0: y_touch + 2
    // // x1: x0 + ltapp_render_text_width("> Net", FONT_BASE, 2) + 52
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(0, y_touch, ltapp_render_text_width("NET", FONT_BASE_18, 1) + 56 + indicator_w, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // // - Settings touch area
    // // x0: LCD_WIDTH_HALF - ltapp_render_text_width("SETTINGS", FONT_BASE_9, 1) / 2 - 12
    // // y0: bottom_y
    // // x1: x0 + ltapp_render_text_width("SETTINGS", FONT_BASE_9, 1) + 24
    // // y1: y0 + LCD_HEIGHT - bottom_y
    // ltapp_render_rect(LCD_WIDTH_HALF - ltapp_render_text_width("SETTINGS", FONT_BASE_9, 1) / 2 - 12, bottom_y, ltapp_render_text_width("SETTINGS", FONT_BASE_9, 1) + 24, LCD_HEIGHT - bottom_y, COLOR_WHITE);

    // // - Extras touch area
    // // cx: extras_cx
    // // cy: extras_cy
    // // r: extras_r + 2
    // ltapp_render_circle(extras_cx, extras_cy, extras_r + 2, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_settings_name(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0", COLOR_DGRAY, FONT_SMALL, 1);
    
    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "NAME", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Name
    y = LCD_HEIGHT_HALF - 24;
    ltapp_render_string_centered(y, jvlt_player_name(), COLOR_CYAN, FONT_BASE_12, 1);

    y += ltapp_render_text_height(FONT_BASE_12, 1) + 8;
    const uint8_t base_name_width = ltapp_render_text_width("ABCDEFGHIJK", FONT_BASE_12, 1) + 16;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - base_name_width / 2, y, base_name_width, 2, 2, COLOR_WHITE);

    y += 8;
    ltapp_render_string_centered(y, "tap to edit", COLOR_YELLOW, FONT_BASE_6, 1);

    draw_page_dots(0, 2, LCD_HEIGHT - 30);

    ltapp_render_flush();
}

static void render_settings_brightness(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0", COLOR_DGRAY, FONT_SMALL, 1);
    
    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "BRIGHTNESS", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Controls
    const uint8_t control_r = 20;
    const uint8_t control_char_h = ltapp_render_text_height(FONT_BASE_18, 1);
    const uint8_t control_outer_r = control_r + 4;

    // Minus
    ltapp_render_circle_fill(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_BLACK);
    ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_MAGENTA);
    ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_PINK_RED);
    ltapp_render_char(10 + control_r - ltapp_render_text_width("-", FONT_BASE_18, 1) / 2, LCD_HEIGHT_HALF + 2 * control_r - control_char_h / 2, '-', COLOR_PINK_RED, FONT_BASE_18, 1);

    // Plus
    ltapp_render_circle_fill(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_MAGENTA);
    ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_CYAN_BRIGHT);
    ltapp_render_char(LCD_WIDTH - 10 - control_r - ltapp_render_text_width("+", FONT_BASE_18, 1) / 2, LCD_HEIGHT_HALF + 2 * control_r - control_char_h / 2, '+', COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);

    // Visual display
    uint8_t brightness = ((jvlt_brightness() * 20 + 127) / 255) * 5;
    const uint8_t bar_w = LCD_WIDTH - 2 * (20 + 2 * control_r);
    const uint8_t bar_h = 60;
    const uint8_t bar_x = LCD_WIDTH_HALF - bar_w / 2;
    const uint8_t bar_y = LCD_HEIGHT_HALF - bar_h / 2 - 24;

    // Brightness bars (10 steps, each = 10%)
    uint8_t steps_full = brightness / 10;  // fully lit bars
    bool half = (brightness % 10) == 5;    // half-lit next bar
    const uint8_t num_bars = 10;
    const uint8_t slot_w = bar_w / num_bars;
    const uint8_t bw = slot_w - 3;
    const uint8_t br = 2;  // corner radius
    for (int i = 0; i < num_bars; i++) {
        uint8_t h = 6 + (uint8_t)((bar_h - 10) * (i + 1) / num_bars);
        uint8_t x = bar_x + i * slot_w + 2;
        uint8_t y_bar = bar_y + bar_h - h - 2;
        if (i < steps_full) {
            ltapp_render_rounded_rect_fill(x, y_bar, bw, h, br, COLOR_YELLOW);
        } else if (i == steps_full && half) {
            ltapp_render_rounded_rect_fill(x, y_bar, bw, h, br, COLOR_DGRAY);
            ltapp_render_rounded_rect_fill(x, y_bar + h / 2, bw, h / 2, br, COLOR_YELLOW);
        } else {
            ltapp_render_rounded_rect_fill(x, y_bar, bw, h, br, COLOR_DGRAY);
        }
    }
    
    // Number display
    char label[8];
    snprintf(label, sizeof(label), "%d%%", brightness);
    ltapp_render_string_centered(LCD_HEIGHT_HALF + 2 * control_r - ltapp_render_text_height(FONT_BASE_24, 1) / 2, label, COLOR_WHITE, FONT_BASE_24, 1);

    draw_page_dots(1, 2, LCD_HEIGHT - 30);

    // TOUCH AREAS
    // // - Minus brightness touch area
    // // cx: 10 + control_r
    // // cy: LCD_HEIGHT_HALF + 2 * control_r
    // // r: 80
    // ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, 80, COLOR_WHITE);

    // // - Plus brightness touch area
    // // cx: LCD_WIDTH - 10 - control_r
    // // cy: LCD_HEIGHT_HALF + 2 * control_r
    // // r: 80
    // ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, 80, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_solo_setup(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    const jvlt_match_t *m = jvlt_match_get();
    const uint8_t starting_life = m ? m->starting_life : 40;
    const uint8_t opponents = m ? m->player_count - 1 : 3;

    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "SOLO PLAY", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    const uint8_t bottom_y = LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_12, 1) - 32;

    const uint8_t container_y = y;
    const uint8_t container_h = bottom_y - container_y;

    const uint8_t peek_distance_from_center = ltapp_render_text_width("00", FONT_BASE_18, 1) / 2 + 4;

    // Starting life
    y = container_y + 16;
    ltapp_render_string_spaced_centered(y, "STARTING LIFE", COLOR_CYAN, FONT_BASE_9, 1, 7);

    y += ltapp_render_text_height(FONT_BASE_9, 1) + 8 + ltapp_render_text_height(FONT_BASE_18, 1) / 2;

    ltapp_render_char(LCD_WIDTH_HALF / 2 - 8 - ltapp_render_text_width("<", FONT_BASE_18, 1), y - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '<', COLOR_CYAN, FONT_BASE_18, 1);
    ltapp_render_char(3 * LCD_WIDTH / 4 + 8, y - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '>', COLOR_CYAN, FONT_BASE_18, 1);

    const int16_t *starting_life_prev = jvlt_match_peek_starting_life(-1);
    if (starting_life_prev) {
        const uint8_t w = ltapp_render_number_width((int)*starting_life_prev, FONT_BASE_9, 1);
        ltapp_render_number(LCD_WIDTH_HALF - peek_distance_from_center - w, y - ltapp_render_number_height(FONT_BASE_9, 1) / 2, (int)*starting_life_prev, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    ltapp_render_number_centered(y - ltapp_render_number_height(FONT_BASE_18, 1) / 2, starting_life, COLOR_CYAN, FONT_BASE_18, 1);

    const int16_t *starting_life_next = jvlt_match_peek_starting_life(1);
    if (starting_life_next) {
        ltapp_render_number(LCD_WIDTH_HALF + peek_distance_from_center, y - ltapp_render_number_height(FONT_BASE_9, 1) / 2, (int)*starting_life_next, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    // Opponents
    y = container_y + container_h / 2 + 8;
    ltapp_render_string_centered(y, "OPPONENTS", COLOR_YELLOW, FONT_BASE_9, 1);

    y += ltapp_render_text_height(FONT_BASE_9, 1) + 8 + ltapp_render_text_height(FONT_BASE_18, 1) / 2;

    ltapp_render_char(LCD_WIDTH_HALF / 2 - 8 - ltapp_render_text_width("<", FONT_BASE_18, 1), y - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '<', COLOR_YELLOW, FONT_BASE_18, 1);
    ltapp_render_char(3 * LCD_WIDTH / 4 + 8, y - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '>', COLOR_YELLOW, FONT_BASE_18, 1);

    const int16_t *opp_prev = jvlt_match_peek_opponents(-1);
    if (opp_prev) {
        const uint8_t w = ltapp_render_number_width((int)*opp_prev, FONT_BASE_9, 1);
        ltapp_render_number(LCD_WIDTH_HALF - peek_distance_from_center - w, y - ltapp_render_number_height(FONT_BASE_9, 1) / 2, (int)*opp_prev, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    ltapp_render_number_centered(y - ltapp_render_number_height(FONT_BASE_18, 1) / 2, opponents, COLOR_YELLOW, FONT_BASE_18, 1);

    const int16_t *opp_next = jvlt_match_peek_opponents(1);
    if (opp_next) {
        ltapp_render_number(LCD_WIDTH_HALF + peek_distance_from_center, y - ltapp_render_number_height(FONT_BASE_9, 1) / 2, (int)*opp_next, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    // Start button
    y = bottom_y;

    const uint8_t bottom_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - bottom_dash_line_width / 2, y, bottom_dash_line_width, 2, 2, COLOR_DGRAY);

    y += (LCD_HEIGHT - y) / 2 - ltapp_render_text_height(FONT_BASE_12, 1) / 2;

    ltapp_render_string_spaced(LCD_WIDTH_HALF - ltapp_render_text_width("START >", FONT_BASE_12, 1) / 2 + 6, y - 4, "START >", COLOR_GREEN, FONT_BASE_12, 1, 7);

    // TOUCH AREAS
    // uint8_t y_touch = container_y + 16 + ltapp_render_text_height(FONT_BASE_9, 1) / 2;

    // // - Starting life "<" touch area
    // // x0: 0
    // // y0: y_touch
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE_18, 1) + 32
    // ltapp_render_rect(0, y_touch, 80, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // // - Starting life ">" touch area
    // // x0: LCD_WIDTH - 80
    // // y0: y_touch
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE_18, 1) + 32
    // ltapp_render_rect(LCD_WIDTH - 80, y_touch, 80, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // y_touch = container_y + container_h / 2 + 8 + ltapp_render_text_height(FONT_BASE_9, 1) / 2;
    // // - Opponents "<" touch area
    // // x0: 0
    // // y0: y_touch
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE_18, 1) + 32
    // ltapp_render_rect(0, y_touch, 80, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // // - Opponents ">" touch area
    // // x0: LCD_WIDTH - 80
    // // y0: y_touch
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE_18, 1) + 32
    // ltapp_render_rect(LCD_WIDTH - 80, y_touch, 80, ltapp_render_text_height(FONT_BASE_18, 1) + 32, COLOR_WHITE);

    // // - Start match touch area
    // // bellow the horizontal line where y = bottom_y
    // ltapp_render_hline(0, bottom_y, LCD_WIDTH, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_match(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    // Border
    draw_border();

    const uint8_t control_r = 35;
    const uint8_t control_margin = 2;
    const uint8_t control_border_r = control_r + control_margin;
    const uint8_t control_perim_dist_from_center = 15;
    const uint8_t control_cy_from_bottom = LCD_HEIGHT - control_r - 5;

    ltapp_render_circle_fill(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_MAGENTA);
    ltapp_render_rect_fill(LCD_WIDTH_HALF - 90, LCD_HEIGHT - control_r - 4, 10, 10, COLOR_BLACK);

    ltapp_render_circle_fill(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_MAGENTA);
    ltapp_render_rect_fill(LCD_WIDTH_HALF + 90, LCD_HEIGHT - control_r - 4, 10, 10, COLOR_BLACK);

    ltapp_render_hline(LCD_WIDTH_HALF - 22, LCD_HEIGHT - 16, 45, COLOR_MAGENTA);
    ltapp_render_rect_fill(LCD_WIDTH_HALF - 50, LCD_HEIGHT - 15, 100, 20, COLOR_BLACK);

    // Swipe up for settings hint
    ltapp_render_string(LCD_WIDTH_HALF - ltapp_render_text_width("sett.", FONT_SMALL, 1) / 2 - 4, LCD_HEIGHT - 12, "sett.", COLOR_DGRAY, FONT_SMALL, 1);
    ltapp_render_string(LCD_WIDTH_HALF + 13, LCD_HEIGHT - 10, "^", COLOR_DGRAY, FONT_SMALL, 1);

    // Damage counters
    ltapp_render_circle(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center, control_cy_from_bottom, control_r - 2, COLOR_CYAN);
    ltapp_render_icon(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center - 20, control_cy_from_bottom - 20, icon_crossed_swords, 40, 40, COLOR_CYAN);

    // Extras
    ltapp_render_circle(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center, control_cy_from_bottom, control_r - 2, COLOR_YELLOW);
    ltapp_render_icon(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center - 20, control_cy_from_bottom - 20, icon_die, 40, 40, COLOR_YELLOW);

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        int16_t life = m->players[m->my_index].life;
        uint8_t poison = m->players[m->my_index].poison;
        
        // Player name
        ltapp_render_string_centered(16, m->players[m->my_index].name, COLOR_GRAY, FONT_BASE_6, 1);

        // Life delta toast
        uint32_t delta_elapsed = (xTaskGetTickCount() - ltapp_life_delta_tick()) * portTICK_PERIOD_MS;
        if (delta_elapsed < LIFE_DELTA_SHOW_MS && ltapp_life_delta() != 0) {
            char dbuf[8];
            snprintf(dbuf, sizeof(dbuf), "%+d", ltapp_life_delta());
            uint16_t dcolor = ltapp_life_delta() > 0 ? COLOR_GREEN : COLOR_RED;
            ltapp_render_string_centered(20 + ltapp_render_text_height(FONT_BASE_6, 1), dbuf, dcolor, FONT_BASE_12, 1);
            jvlt_mark_dirty();
        }

        // Life counter
        uint16_t life_color = COLOR_WHITE;
        if (life <= 0){
            life_color = COLOR_RED;
        } else if (life <= 10) {
            life_color = COLOR_ORANGE;
        }

        // Render life as individual digits, tighter spacing, minus at smaller scale
        char buf[12];
        snprintf(buf, sizeof(buf), "%d", life);
        int digit_scale = 2;
        int gap = -16; // tighter than default xAdvance
        
        // Calculate total width using text_width for digits, manual for minus
        int minus_w = ltapp_render_text_width("-", FONT_BASE_36, 1);
        int total_w = 0;
        char *p = buf;
        while (*p) {
            if (*p == '-') {
                total_w += minus_w;
            } else {
                total_w += ltapp_render_text_width((char[]){*p, 0}, FONT_BASE_36, digit_scale) + gap;
            }
            p++;
        }
        total_w -= gap; // remove trailing gap

        int cx = LCD_WIDTH_HALF - total_w / 2;
        int cy = 2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_BASE_36, digit_scale);
        p = buf;
        while (*p) {
            if (*p == '-') {
                ltapp_render_char(cx, cy + ltapp_render_text_height(FONT_BASE_36, digit_scale) / 2 - ltapp_render_text_height(FONT_BASE_36, 1) / 2, '-', life_color, FONT_BASE_36, 1);
                cx += minus_w;
            } else {
                ltapp_render_char(cx, cy, *p, life_color, FONT_BASE_36, digit_scale);
                cx += ltapp_render_text_width((char[]){*p, 0}, FONT_BASE_36, digit_scale) + gap;
            }
            p++;
        }

        // Poison counter
        const uint8_t poison_initial_y = LCD_HEIGHT - 25;
        const uint8_t poison_row_h = 4;
        const uint8_t poison_row_gap = 1;
        const uint8_t poison_delta_y = poison_row_h + poison_row_gap;
        const uint8_t poison_row_r = 2;
        const uint8_t poison_row_w_padding = 8;
        for (uint8_t i = 1; i <= 10; i++) {
            const uint8_t _y = poison_initial_y - (poison_delta_y * (i - 1));
            const uint16_t _color = i <= poison ? COLOR_GREEN : COLOR_DGRAY;

            const uint8_t _circle_w_at_y = ltapp_circle_width_at(control_r, control_cy_from_bottom - _y - poison_row_h / 2);
            const uint8_t _row_w_half = control_r + control_perim_dist_from_center - _circle_w_at_y / 2;

            ltapp_render_rounded_rect_fill(LCD_WIDTH_HALF - _row_w_half + poison_row_w_padding, _y, 2 * _row_w_half - 2 * poison_row_w_padding, poison_row_h, poison_row_r, _color);
        }

        if (poison >= 10) {
            ltapp_render_string_centered(poison_initial_y - 10 * poison_delta_y - 5, "POISON!", COLOR_GREEN, FONT_SMALL, 1);
        }
    }

    // Life counter modifiers
    ltapp_render_char(16, 2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_BASE_36, 2) / 4 - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '<', COLOR_PINK_RED, FONT_BASE_18, 1);
    ltapp_render_char(LCD_WIDTH - 16 - ltapp_render_text_width(">", FONT_BASE_18, 1), 2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_BASE_36, 2) / 4 - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '>', COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);

    // TOUCH AREAS
    // // - Minus life touch area
    // // x0: 0
    // // y0: 0
    // // x1: x0 + LCD_WIDTH_HALF - 1
    // // y1: y0 + 2 * LCD_HEIGHT / 3
    // ltapp_render_rect(0, 0, LCD_WIDTH_HALF - 1, 2 * LCD_HEIGHT / 3, COLOR_WHITE);

    // // - Plus life touch area
    // // x0: LCD_WIDTH_HALF + 1
    // // y0: 0
    // // x1: x0 + LCD_WIDTH_HALF - 1
    // // y1: y0 + 2 * LCD_HEIGHT / 3
    // ltapp_render_rect(LCD_WIDTH_HALF + 1, 0, LCD_WIDTH_HALF - 1, 2 * LCD_HEIGHT / 3, COLOR_WHITE);

    // // - Damage touch area
    // // cx: LCD_WIDTH_HALF - control_r - control_perim_dist_from_center
    // // cy: control_cy_from_bottom
    // // r: control_border_r
    // ltapp_render_circle(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_WHITE);

    // // - Extras touch area
    // // cx: LCD_WIDTH_HALF + control_r + control_perim_dist_from_center
    // // cy: control_cy_from_bottom
    // // r: control_border_r
    // ltapp_render_circle(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center, control_cy_from_bottom, control_border_r, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_match_cmd_damage(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    
    // Page title
    uint8_t y = 12;
    ltapp_render_string_centered(y, "commander", COLOR_MAGENTA, FONT_SMALL, 1);

    y += ltapp_render_text_height(FONT_SMALL, 1);
    ltapp_render_string_centered(y, "DAMAGE", COLOR_MAGENTA, FONT_BASE_12, 1);

    y += ltapp_render_text_height(FONT_BASE_12, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Opponents

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        const uint8_t opponents = m->player_count - 1;

        switch (opponents) {
            case 1: {
                // Opponent 1
                render_cmd_damage_opps_bubble(0, LCD_WIDTH_HALF, LCD_HEIGHT_HALF - 15, COLOR_CYAN, BUBBLE_LARGE);
                break;
            }

            case 2: {
                const uint8_t points[2][2] = {
                    {LCD_WIDTH_HALF / 2, LCD_HEIGHT_HALF},
                    {3 * LCD_WIDTH / 4, LCD_HEIGHT_HALF},
                };

                // Opponent 1
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN, BUBBLE_MEDIUM);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW, BUBBLE_MEDIUM);
                break;
            }

            case 3: {
                y += 42;
                const uint8_t tri_side = 80;
                const uint8_t tri_h = (uint8_t)(tri_side * 0.86602540378);
                const uint8_t points[3][2] = {
                    {LCD_WIDTH_HALF - tri_side / 2 - 15, y + tri_h},
                    {LCD_WIDTH_HALF, y},
                    {LCD_WIDTH_HALF + tri_side / 2 + 15, y + tri_h},
                };

                // Opponent 1
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN, BUBBLE_MEDIUM);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW, BUBBLE_MEDIUM);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN, BUBBLE_MEDIUM);
                break;
            }

            case 4: {
                y += 42;
                const uint8_t square_side = 80;
                const uint8_t points[4][2] = {
                    {LCD_WIDTH_HALF - square_side / 2 - 20, y + square_side},
                    {LCD_WIDTH_HALF - square_side / 2, y},
                    {LCD_WIDTH_HALF + square_side / 2, y},
                    {LCD_WIDTH_HALF + square_side / 2 + 20, y + square_side},
                };

                // Opponent 1
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN, BUBBLE_SMALL);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW, BUBBLE_SMALL);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN, BUBBLE_SMALL);

                // Opponent 4
                render_cmd_damage_opps_bubble(3, points[3][0], points[3][1], COLOR_ORANGE, BUBBLE_SMALL);
                break;
            }

            case 5: {
                y += 39;
                const uint8_t pent_diam = 96;
                const uint8_t pent_side = (uint8_t)(pent_diam * 0.5878f);
                const uint8_t points[5][2] = {
                    {LCD_WIDTH_HALF - pent_side / 2 - 15, y + pent_diam},
                    {LCD_WIDTH_HALF - pent_diam / 2 - 18, y + 20},
                    {LCD_WIDTH_HALF, y},
                    {LCD_WIDTH_HALF + pent_diam / 2 + 18, y + 20},
                    {LCD_WIDTH_HALF + pent_side / 2 + 15, y + pent_diam},
                };

                // Opponent 1
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN, BUBBLE_SMALL);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW, BUBBLE_SMALL);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN, BUBBLE_SMALL);

                // Opponent 4
                render_cmd_damage_opps_bubble(3, points[3][0], points[3][1], COLOR_ORANGE, BUBBLE_SMALL);

                // Opponent 5
                render_cmd_damage_opps_bubble(4, points[4][0], points[4][1], COLOR_PINK_RED, BUBBLE_SMALL);
                break;
            }

            default:
                char opponents_str[16];
                snprintf(opponents_str, sizeof(opponents_str), "%d opponents", opponents);
                ltapp_render_string_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2, opponents_str, COLOR_RED, FONT_BASE, 2);
                break;
        }
    }

    draw_page_dots(0, 2, LCD_HEIGHT - 30);

    ltapp_render_flush();
}

static void render_match_cmd_damage_opponent(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    const jvlt_match_t *m = jvlt_match_get();
    const jvlt_player_t *opp = &m->players[m->cmd_sel_opp];

    // Opponent name
    uint8_t y = 32;
    ltapp_render_string_centered(y, opp->name, COLOR_CYAN, FONT_BASE_12, 1);

    y += ltapp_render_text_height(FONT_BASE_12, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_render_text_width("ABCDEFGIHJK", FONT_BASE_12, 1) + 16;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_WHITE);

    // Tap to edit hint
    y += 4;
    ltapp_render_string_centered(y, "tap to edit", COLOR_YELLOW, FONT_BASE_6, 1);

    // Commanders
    const jvlt_player_t *me = &m->players[m->my_index];
    uint8_t ncmd = opp->num_commanders;
    
    uint8_t y_points[2];
    if (ncmd == 1) {
      y_points[0] = LCD_HEIGHT_HALF - 24;
    } else {
      y_points[0] = LCD_HEIGHT_HALF - 40;
      y_points[1] = LCD_HEIGHT_HALF + 24;
    }
    
    const uint8_t control_r = 20;

    for (uint8_t c = 0; c < ncmd; c++) {
        y = y_points[c];

        ltapp_render_string_centered(y, opp->cmd_name[c], COLOR_GRAY, FONT_BASE_9_N, 1);
        
        ltapp_render_hline_dashed(LCD_WIDTH_HALF - 50, y + ltapp_render_text_height(FONT_BASE_9_N, 1) + 2, 100, 2, 2, COLOR_YELLOW);

        int16_t dmg = me->cmd_dmg[m->cmd_sel_opp][c];
        uint16_t color = COLOR_WHITE;
        if (dmg > 20) {
          color = COLOR_RED;
        } else if (dmg > 15) {
          color = COLOR_ORANGE;
        }
        ltapp_render_number_centered(y + 48 - ltapp_render_number_height(FONT_BASE_18, 1), dmg, color, FONT_BASE_18, 1);

        ltapp_render_circle(20 + control_r, y + 24, control_r, COLOR_PINK_RED);
        ltapp_render_char(20 + control_r - ltapp_render_text_width("-", FONT_BASE_18, 1) / 2, y + 24 - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '-', COLOR_PINK_RED, FONT_BASE_18, 1);

        ltapp_render_circle(LCD_WIDTH - 20 - control_r, y + 24, control_r, COLOR_CYAN_BRIGHT);
        ltapp_render_char(LCD_WIDTH - 20 - control_r - ltapp_render_text_width("+", FONT_BASE_18, 1) / 2, y + 24 - ltapp_render_text_height(FONT_BASE_18, 1) / 2, '+', COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);
    }

    if (ncmd < 2) {
        ltapp_render_string_spaced_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_12, 1) - 16, "+ CMD", COLOR_GREEN, FONT_BASE_12, 1, 7);
    } else {
        ltapp_render_string_spaced_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_12, 1) - 16, "- CMD", COLOR_ORANGE, FONT_BASE_12, 1, 7);
    }

    // TOUCH AREAS
    // // - Edit opponent name
    // // x0: 0
    // // y0: 0
    // // x1: LCD_WIDTH
    // // y1: text_height + 40
    // ltapp_render_rect(0, 0, LCD_WIDTH, ltapp_render_text_height(FONT_BASE_12, 1) + 40, COLOR_WHITE);

    // if (ncmd == 1) {
    //     const uint8_t y_one = LCD_HEIGHT_HALF - 24;

    //     // - Minus damage (1 commander) touch area
    //     // cx: 20 + control_r
    //     // cy: LCD_HEIGHT_HALF
    //     // r: 3 * control_r
    //     ltapp_render_circle(20 + control_r, y_one + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Plus damage (1 commander) touch area
    //     // cx: LCD_WIDTH - 42 - control_r
    //     // cy: LCD_HEIGHT_HALF
    //     // r: 3 * control_r
    //     ltapp_render_circle(LCD_WIDTH - 20 - control_r, y_one + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Edit commander name (1 commander) touch area
    //     // x0: LCD_WIDTH_HALF - 50
    //     // y0: y_one - 16
    //     // x1: x0 + 100
    //     // y1: y0 + 48
    //     ltapp_render_rect(LCD_WIDTH_HALF - 50, y_one - 16, 100, 48, COLOR_WHITE);
    // }

    // if (ncmd == 2) {
    //     const uint8_t y_two_fst = LCD_HEIGHT_HALF - 40;
    //     const uint8_t y_two_snd = LCD_HEIGHT_HALF + 24;

    //     // - Minus damage 1st commander (2 commanders) touch area
    //     // cx: 20 + control_r
    //     // cy: y_two_fst + 24
    //     // r: 3 * control_r / 2
    //     ltapp_render_circle(20 + control_r, y_two_fst + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Plus damage 1st commander (2 commanders) touch area
    //     // cx: LCD_WIDTH - 20 - control_r
    //     // cy: y_two_fst + 24
    //     // r: 3 * control_r / 2
    //     ltapp_render_circle(LCD_WIDTH - 20 - control_r, y_two_fst + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Edit 1st commander name (2 commanders) touch area
    //     // x0: LCD_WIDTH_HALF - 50
    //     // y0: y_two_fst - 16
    //     // x1: x0 + 100
    //     // y1: y0 + 48
    //     ltapp_render_rect(LCD_WIDTH_HALF - 50, y_two_fst - 16, 100, 48, COLOR_WHITE);

    //     // - Minus damage 2nd commander (2 commanders) touch area
    //     // cx: 40 + control_r
    //     // cy: y_two_fst + 24
    //     // r: 3 * control_r / 2
    //     ltapp_render_circle(20 + control_r, y_two_snd + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Plus damage 2nd commander (2 commanders) touch area
    //     // cx: LCD_WIDTH - 40 - control_r
    //     // cy: y_two_fst + 24
    //     // r: 3 * control_r / 2
    //     ltapp_render_circle(LCD_WIDTH - 20 - control_r, y_two_snd + 24, 3 * control_r / 2, COLOR_WHITE);

    //     // - Edit 2nd commander name (2 commanders) touch area
    //     // x0: LCD_WIDTH_HALF - 50
    //     // y0: y_two_snd - 16
    //     // x1: x0 + 100
    //     // y1: y0 + 48
    //     ltapp_render_rect(LCD_WIDTH_HALF - 50, y_two_snd - 16, 100, 48, COLOR_WHITE);
    // }

    // // - Plus/minus commander touch area
    // // x0: 0
    // // y0: LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_12, 1) - 20
    // // x1: x0 + LCD_WIDTH
    // // y1: y0 + ltapp_render_text_height(FONT_BASE_12, 1) + 20
    // ltapp_render_rect(0, LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_12, 1) - 20, LCD_WIDTH, ltapp_render_text_height(FONT_BASE_12, 1) + 20, COLOR_WHITE);
    // END TOUCH AREAS

    ltapp_render_flush();
}

static void render_match_poison_damage(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    
    // Page title
    uint8_t y = 12;
    ltapp_render_string_centered(y, "poison", COLOR_MAGENTA, FONT_SMALL, 1);

    y += ltapp_render_text_height(FONT_SMALL, 1);
    ltapp_render_string_centered(y, "DAMAGE", COLOR_MAGENTA, FONT_BASE_12, 1);

    y += ltapp_render_text_height(FONT_BASE_12, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        const uint8_t poison = m->players[m->my_index].poison;

        const uint8_t dot_r = 8;
        const uint8_t gap = 4;
        const uint8_t total_w = (2 * dot_r + gap) * 10 - gap;

        uint8_t y = LCD_HEIGHT_HALF - 2 * dot_r;

        if (poison >= 10)
            ltapp_render_string_centered(y - ltapp_render_text_height(FONT_BASE_12, 1) - 16, "POISON!", COLOR_GREEN, FONT_BASE_12, 1);

        for (uint8_t i = 1; i <= 10; i++) {
            if (i <= poison)
                ltapp_render_circle_fill(LCD_WIDTH_HALF - total_w / 2 + (i - 1) * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_GREEN);
            else
                ltapp_render_circle(LCD_WIDTH_HALF - total_w / 2 + (i - 1) * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_GREEN);
        }

        // Controls
        const uint8_t control_r = 20;
        const uint8_t control_char_h = ltapp_render_text_height(FONT_BASE_18, 1);
        const uint8_t control_outer_r = control_r + 4;

        // Minus
        ltapp_render_circle_fill(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_BLACK);
        ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_MAGENTA);
        ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_PINK_RED);
        ltapp_render_char(10 + control_r - ltapp_render_text_width("-", FONT_BASE_18, 1) / 2, LCD_HEIGHT_HALF + 2 * control_r - control_char_h / 2, '-', COLOR_PINK_RED, FONT_BASE_18, 1);

        // Plus
        ltapp_render_circle_fill(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_BLACK);
        ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_outer_r, COLOR_MAGENTA);
        ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_CYAN_BRIGHT);
        ltapp_render_char(LCD_WIDTH - 10 - control_r - ltapp_render_text_width("+", FONT_BASE_18, 1) / 2, LCD_HEIGHT_HALF + 2 * control_r - control_char_h / 2, '+', COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);

        // Number display
        ltapp_render_number_centered(LCD_HEIGHT_HALF + 2 * control_r - ltapp_render_number_height(FONT_BASE_18, 1) / 2, poison, COLOR_WHITE, FONT_BASE_18, 1);

        // TOUCH AREAS
        // // - Minus poison touch area
        // // cx: 10 + control_r
        // // cy: LCD_HEIGHT_HALF + 2 * control_r
        // // r: 3 * control_r
        // ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, 3 * control_r, COLOR_WHITE);

        // // - Plus poison touch area
        // // cx: LCD_WIDTH - 10 - control_r
        // // cy: LCD_HEIGHT_HALF + 2 * control_r
        // // r: 3 * control_r
        // ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, 3 * control_r, COLOR_WHITE);
        // END OF TOUCH AREAS
    }

    draw_page_dots(1, 2, LCD_HEIGHT - 20);

    ltapp_render_flush();
}

static void render_match_settings(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0", COLOR_DGRAY, FONT_SMALL, 1);
    
    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "SETTINGS", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Options
    const uint8_t control_h = ltapp_render_text_height(FONT_BASE_12, 1);
    const uint8_t offset_x = ltapp_render_text_width(">", FONT_BASE_12, 1) / 2 + 4;

    // Reset match
    uint8_t reset_w = ltapp_render_text_width("RESET MATCH", FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - reset_w / 2 - offset_x, LCD_HEIGHT_HALF - 24 - control_h, ">", COLOR_WHITE, FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - reset_w / 2 + offset_x, LCD_HEIGHT_HALF - 24 - control_h, "RESET MATCH", COLOR_YELLOW, FONT_BASE_12, 1);

    // Leave match
    uint8_t leave_w = ltapp_render_text_width("LEAVE MATCH", FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - leave_w / 2 - offset_x, LCD_HEIGHT_HALF + 24, ">", COLOR_WHITE, FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - leave_w / 2 + offset_x, LCD_HEIGHT_HALF + 24, "LEAVE MATCH", COLOR_RED, FONT_BASE_12, 1);

    draw_page_dots(0, 2, LCD_HEIGHT - 30);

    // TOUCH AREAS
    // // - Reset match touch area
    // // x0: LCD_WIDTH_HALF - 100
    // // y0: LCD_HEIGHT_HALF - 40
    // // x1: x0 + 200
    // // y2: y0 + control_h + 32
    // ltapp_render_rect(LCD_WIDTH_HALF - 100, LCD_HEIGHT_HALF - 40 - control_h, 200, control_h + 32, COLOR_WHITE);

    // // - Leave touch area
    // // x0: LCD_WIDTH_HALF - 100
    // // y0: LCD_HEIGHT_HALF - 40
    // // x1: x0 + 200
    // // y2: y0 + control_h + 32
    // ltapp_render_rect(LCD_WIDTH_HALF - 100, LCD_HEIGHT_HALF + 8, 200, control_h + 32, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_extras(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "EXTRAS", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Options
    const uint8_t control_h = ltapp_render_text_height(FONT_BASE_12, 1);
    const uint8_t offset_x = ltapp_render_text_width(">", FONT_BASE_12, 1) / 2 + 4;
    const uint8_t offset_y = 12;

    // Roll a die
    uint8_t roll_die_w = ltapp_render_text_width("ROLL A DIE", FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - roll_die_w / 2 - offset_x, LCD_HEIGHT_HALF - 24 - control_h - offset_y, ">", COLOR_WHITE, FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - roll_die_w / 2 + offset_x, LCD_HEIGHT_HALF - 24 - control_h - offset_y, "ROLL A DIE", COLOR_CYAN, FONT_BASE_12, 1);

    // Flip a coin
    uint8_t flip_coin_w = ltapp_render_text_width("FLIP A COIN", FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - flip_coin_w / 2 - offset_x, LCD_HEIGHT_HALF + 24 - offset_y, ">", COLOR_WHITE, FONT_BASE_12, 1);
    ltapp_render_string(LCD_WIDTH_HALF - flip_coin_w / 2 + offset_x, LCD_HEIGHT_HALF + 24 - offset_y, "FLIP A COIN", COLOR_YELLOW, FONT_BASE_12, 1);

    // Icons
    ltapp_render_icon(LCD_WIDTH_HALF - 42, LCD_HEIGHT - 48, icon_die, 40, 40, COLOR_CYAN);
    ltapp_render_icon(LCD_WIDTH_HALF + 2, LCD_HEIGHT - 48, icon_coin_small, 40, 40, COLOR_YELLOW);

    // TOUCH AREAS
    // // - Roll a die touch area
    // // x0: LCD_WIDTH_HALF - 100
    // // y0: LCD_HEIGHT_HALF - 40 - control_h - offset_y
    // // x1: x0 + 200
    // // y2: y0 + control_h + 32 + offset_y
    // ltapp_render_rect(LCD_WIDTH_HALF - 100, LCD_HEIGHT_HALF - 40 - control_h - offset_y, 200, control_h + 32 + offset_y, COLOR_WHITE);
    
    // // - Flip a coin touch area
    // // x0: LCD_WIDTH_HALF - 100
    // // y0: LCD_HEIGHT_HALF + 8 - offset_y
    // // x1: x0 + 200
    // // y2: y0 + control_h + 32 + offset_y
    // ltapp_render_rect(LCD_WIDTH_HALF - 100, LCD_HEIGHT_HALF + 8 - offset_y, 200, control_h + 32 + offset_y, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_dice(void) {
    ltapp_dice_is_animating(); // tick animation

    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "ROLL A DIE", COLOR_MAGENTA, FONT_BASE_9_N, 1);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Controls
    const uint8_t control_text_h = ltapp_render_text_height(FONT_BASE_18, 1);
    ltapp_render_string(12, LCD_HEIGHT_HALF - control_text_h / 2, "<", COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);
    ltapp_render_string(LCD_WIDTH - 12 - ltapp_render_text_width(">", FONT_BASE_18, 1), LCD_HEIGHT_HALF - control_text_h / 2, ">", COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);

    // Select die
    const jvlt_dice_t *d = jvlt_dice_get();
    uint8_t selected_d_sides = d->sides ? d->sides : 20;
    
    const uint8_t row_h = ltapp_render_text_height(FONT_BASE_18, 1) + 8;

    y = LCD_HEIGHT - row_h;

    char center_d_label[8];
    snprintf(center_d_label, sizeof(center_d_label), "D%d", selected_d_sides);
    ltapp_render_string_centered(y, center_d_label, COLOR_CYAN_BRIGHT, FONT_BASE_18, 1);
    
    const uint8_t center_die_w = ltapp_render_text_width("DDD", FONT_BASE_18, 1);

    const int16_t *dice_prev = jvlt_dice_peek_sides(-1);
    if (dice_prev) {
        char label[8];
        snprintf(label, sizeof(label), "D%d", (int)*dice_prev);
        const uint8_t w = ltapp_render_text_width(label, FONT_BASE_9, 1);
        ltapp_render_string(LCD_WIDTH_HALF - center_die_w / 2 - w / 2 - 20, y, label, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    const int16_t *dice_next = jvlt_dice_peek_sides(1);
    if (dice_next) {
        char label[8];
        snprintf(label, sizeof(label), "D%d", (int)*dice_next);
        const uint8_t w = ltapp_render_text_width(label, FONT_BASE_9, 1);
        ltapp_render_string(LCD_WIDTH_HALF + center_die_w / 2 - w / 2 + 20, y, label, COLOR_DGRAY, FONT_BASE_9, 1);
    }

    // Result
    uint8_t dice_val = ltapp_dice_display_value();
    if (dice_val > 0) {
        const uint8_t *die_icon;
        switch (d->sides) {
            case 4:  die_icon = icon_die_d4;  break;
            case 6:  die_icon = icon_die_d6;  break;
            case 8:  die_icon = icon_die_d8;  break;
            case 12: die_icon = icon_die_d12; break;
            case 20: die_icon = icon_die_d20; break;
            default: die_icon = icon_die_d6; break;
        }

        static const uint16_t colors[] = {COLOR_CYAN, COLOR_MAGENTA, COLOR_YELLOW, COLOR_GREEN, COLOR_ORANGE};
        uint16_t color = colors[(d->result - 1) % 5];

        ltapp_render_icon_bg(LCD_WIDTH_HALF - 60, LCD_HEIGHT_HALF - 60, die_icon, 120, 120, color);
        ltapp_render_number_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE_24, 2) / 2, dice_val, COLOR_WHITE, FONT_BASE_24, 2);
    } else {
        // "Empty" state
        ltapp_render_string_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE_9, 1) / 2, "TAP TO ROLL!", COLOR_GRAY, FONT_BASE_9, 1);
    }

    // TOUCH AREAS
    // // - Cycle die delta -1
    // // cx: 12
    // // cy: LCD_HEIGHT_HALF
    // // r: 40
    // ltapp_render_circle(12, LCD_HEIGHT_HALF, 40, COLOR_WHITE);

    // // - Cycle die delta +1
    // // cx: LCD_WIDTH - 12
    // // cy: LCD_HEIGHT_HALF
    // // r: 40
    // ltapp_render_circle(LCD_WIDTH - 12, LCD_HEIGHT_HALF, 40, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_flip_coin(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 16;
    ltapp_render_string_spaced_centered(y, "FLIP A COIN", COLOR_MAGENTA, FONT_BASE_9_N, 1, 7);

    y += ltapp_render_text_height(FONT_BASE_9_N, 1) + 4;

    const uint8_t top_dash_line_width = ltapp_circle_width_at(LCD_WIDTH_HALF, y - LCD_HEIGHT_HALF) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Tick animation (advances frame if needed)
    bool animating = ltapp_coin_is_animating();

    const jvlt_coin_t *c = jvlt_coin_get();

    if (animating || c->flipped) {
        // Show current coin frame icon
        uint8_t frame = ltapp_coin_current_frame();
        const uint8_t *icon = coin_frames[frame];
        int ix = (LCD_WIDTH - COIN_ICON_W) / 2;
        int iy = (LCD_HEIGHT - COIN_ICON_H) / 2;
        uint16_t color = COLOR_WHITE;
        if (!animating) {
            // Final state — tint based on result
            color = c->heads ? COLOR_YELLOW : COLOR_CYAN;
        }
        ltapp_render_icon(ix, iy, icon, COIN_ICON_W, COIN_ICON_H, color);

        // Label below
        if (!animating) {
            const char *label = c->heads ? "HEADS" : "TAILS";
            ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE_18, 1) - 18, label, color, FONT_BASE_18, 1);
        }
    } else {
        // "Empty" state
        ltapp_render_string_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE_9, 1) / 2, "TAP TO FLIP!", COLOR_GRAY, FONT_BASE_9, 1);
    }

    ltapp_render_flush();
}

static void render_keyboard(void) {
    ltapp_kbd_tick();

    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    uint8_t y = 24;
    const uint8_t input_h = ltapp_render_text_height(FONT_BASE_9, 1) + 8;

    const uint8_t top_line_y = y + input_h;
    const uint8_t top_line_w = ltapp_circle_width_at(LCD_WIDTH_HALF, top_line_y - LCD_HEIGHT_HALF);
    ltapp_render_hline(LCD_WIDTH_HALF - top_line_w / 2 + 1, top_line_y, top_line_w - 2, COLOR_MAGENTA);
    ltapp_render_rect_fill(0, 0, LCD_WIDTH, top_line_y, COLOR_BLACK);

    const uint8_t bottom_line_y = LCD_HEIGHT - y - input_h;
    const uint8_t bottom_line_w = ltapp_circle_width_at(LCD_WIDTH_HALF, bottom_line_y - LCD_HEIGHT_HALF);
    ltapp_render_hline(LCD_WIDTH_HALF - bottom_line_w / 2 + 1, bottom_line_y, bottom_line_w - 2, COLOR_MAGENTA);
    ltapp_render_rect_fill(0, bottom_line_y + 1, LCD_WIDTH, LCD_HEIGHT - bottom_line_y, COLOR_BLACK);

    const ltapp_kbd_t *k = ltapp_kbd_get();

    // Input control
    const uint8_t input_w = ltapp_render_text_width("ABCDEFGHIJKL", FONT_BASE_9, 1) + 16;
    ltapp_render_rect(LCD_WIDTH_HALF - input_w / 2, y + ltapp_render_text_height(FONT_BASE_9, 1) - 2, input_w, 8, COLOR_CYAN);
    ltapp_render_rect_fill(LCD_WIDTH_HALF - input_w / 2 + 1, y + ltapp_render_text_height(FONT_BASE_9, 1) - 2, input_w - 2, 7, COLOR_BLACK);
    ltapp_render_string_centered(y, k->buf, COLOR_CYAN, FONT_BASE_9, 1);

    // Action controls
    y = LCD_HEIGHT - (LCD_HEIGHT - bottom_line_y) / 2;

    // Cancel
    ltapp_render_string(LCD_WIDTH_HALF - 40, y - ltapp_render_text_height(FONT_BASE_12, 1) / 2 - 4, "X", COLOR_RED, FONT_BASE_12, 1);

    // Separator
    ltapp_render_vline_dashed(LCD_WIDTH_HALF, bottom_line_y + 8, LCD_HEIGHT - bottom_line_y - 16, 2, 2, COLOR_DGRAY);

    // Confirm
    ltapp_render_string(LCD_WIDTH_HALF + 48 - ltapp_render_text_width(">", FONT_BASE_18, 1), y - ltapp_render_text_height(FONT_BASE_18, 1) / 2 - 4, ">", COLOR_GREEN, FONT_BASE_18, 1);

    // Keyboard
    const uint8_t keyboard_w = top_line_w;
    const uint8_t keyboard_h = bottom_line_y - top_line_y - 1;
    const uint8_t keyboard_x = LCD_WIDTH_HALF - keyboard_w / 2;
    const uint8_t keyboard_y = top_line_y + 1;

    // Keyboard grid: 3 cols × 4 rows inside the rect
    const uint8_t col_w = keyboard_w / 3;
    const uint8_t row_h = keyboard_h / 4;
    bool upper = (k->shift != KBD_SHIFT_OFF);

    const char *labels[4][3] = {
        {"._", "ABC", "DEF"},
        {"GHI", "JKL", "MNO"},
        {"PQRS", "TUV", "WXYZ"},
        {NULL, NULL, NULL},  // bottom row handled separately
    };
    const char nums[] = "123456789";

    for (int r = 0; r < 3; r++) {
        for (int c = 0; c < 3; c++) {
            uint8_t cx = keyboard_x + c * col_w + col_w / 2;
            uint8_t cy = keyboard_y + r * row_h + row_h / 2;
            // Number
            char nbuf[2] = {nums[r * 3 + c], '\0'};
            ltapp_render_string(cx - col_w / 2 + 4, cy - ltapp_render_text_height(FONT_BASE_9_N, 1) / 2, nbuf, COLOR_GRAY, FONT_BASE_9_N, 1);
            // Letters
            const char *lbl = labels[r][c];
            char lbuf[5];
            int len = strlen(lbl);
            for (int i = 0; i < len && i < 4; i++)
                lbuf[i] = upper ? lbl[i] : (lbl[i] >= 'A' && lbl[i] <= 'Z' ? lbl[i] + 32 : lbl[i]);
            lbuf[len] = '\0';
            uint8_t nw = ltapp_render_text_width(nbuf, FONT_BASE_9_N, 1);
            ltapp_render_string(cx - col_w / 2 + nw + 8, cy - ltapp_render_text_height(FONT_BASE_9_N, 1) / 2, lbuf, COLOR_WHITE, FONT_BASE_9_N, 1);
        }
    }

    // Grid lines
    for (int r = 1; r <= 3; r++) {
        ltapp_render_hline(keyboard_x + 1, keyboard_y + r * row_h, keyboard_w - 2, COLOR_DGRAY);
    }
    for (int c = 1; c < 3; c++) {
        ltapp_render_vline(keyboard_x + c * col_w, keyboard_y + 1, 3 * row_h - 2, COLOR_DGRAY);
    }
    // Bottom row vertical divider
    ltapp_render_vline(keyboard_x + keyboard_w / 2, keyboard_y + 3 * row_h + 1, row_h - 2, COLOR_DGRAY);

    // Bottom row: shift (left half), delete (right half)
    uint8_t bot_y = keyboard_y + 3 * row_h + row_h / 2;

    // Shift
    const char *shift_label = k->shift == KBD_SHIFT_LOCK ? "CAPS" : k->shift == KBD_SHIFT_ONCE ? "Shift" : "shift";
    uint16_t shift_color = k->shift != KBD_SHIFT_OFF ? COLOR_YELLOW : COLOR_DGRAY;
    uint8_t sw = ltapp_render_text_width(shift_label, FONT_BASE_9, 1);
    ltapp_render_string(keyboard_x + keyboard_w / 4 - sw / 2, bot_y - ltapp_render_text_height(FONT_BASE_9, 1) / 2, shift_label, shift_color, FONT_BASE_9, 1);
    ltapp_render_string(keyboard_x + 2, bot_y - ltapp_render_text_height(FONT_SMALL, 1) / 2, "0", COLOR_DGRAY, FONT_SMALL, 1);
    // Delete
    const char *del_label = "< DELETE";
    uint8_t dw = ltapp_render_text_width_spaced(del_label, FONT_BASE_9, 1, 6);
    ltapp_render_string_spaced(keyboard_x + 3 * keyboard_w / 4 - dw / 2, bot_y - ltapp_render_text_height(FONT_BASE_9, 1) / 2, del_label, COLOR_RED, FONT_BASE_9, 1, 6);

    ltapp_render_flush();
}

static void render_placeholder(const char *title) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);
    draw_border();
    ltapp_render_string_centered(100, title, COLOR_WHITE, FONT_BASE_18, 1);
    ltapp_render_flush();
}

void ltapp_ui_render(void) {
    if (ltapp_kbd_is_active()) {
        render_keyboard();
        return;
    }

    switch (jvlt_screen()) {
        case SCREEN_HOME:                 render_home(); break;
        case SCREEN_SETTINGS_NAME:        render_settings_name(); break;
        case SCREEN_SETTINGS_BRIGHTNESS:  render_settings_brightness(); break;
        case SCREEN_SOLO_SETUP:           render_solo_setup(); break;
        case SCREEN_NET_LOBBY:            render_placeholder("Lobby"); break;
        case SCREEN_MATCH:                render_match(); break;
        case SCREEN_MATCH_CMD_DAMAGE:     render_match_cmd_damage(); break;
        case SCREEN_MATCH_POISON_DAMAGE:  render_match_poison_damage(); break;
        case SCREEN_MATCH_SETTINGS:       render_match_settings(); break;
        case SCREEN_EXTRAS:               render_extras(); break;
        case SCREEN_MATCH_CMD_DAMAGE_OPP: render_match_cmd_damage_opponent(); break;
        case SCREEN_DICE:                 render_dice(); break;
        case SCREEN_FLIP_COIN:            render_flip_coin(); break;
        case SCREEN_CONFIRM_RESET:        render_confirm("RESET", "RESET MATCH?", COLOR_YELLOW); break;
        case SCREEN_CONFIRM_LEAVE:        render_confirm("LEAVE", "LEAVE MATCH?", COLOR_RED); break;
        default:                          render_placeholder("???"); break;
    }
}
