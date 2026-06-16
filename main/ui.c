#include "ui.h"
#include "game_task.h"
#include "renderer.h"
#include "icons.h"
#include "coin_anim.h"
#include <math.h>
#include <stdio.h>
#include <string.h>
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

static void render_cmd_damage_opps_bubble(uint8_t opp_index, uint8_t x, uint8_t y, uint16_t color) {
    const jvlt_match_t *m = jvlt_match_get();
    const jvlt_player_t *me = &m->players[m->my_index];
    const jvlt_player_t *opp = &m->players[opp_index + 1];

    if (opp) {
        ltapp_render_string(x - ltapp_render_text_width(opp->name, FONT_BASE, 1) / 2, y - 17 - ltapp_render_text_height(FONT_BASE, 1), opp->name, color, FONT_BASE, 1);
        ltapp_render_icon(x - 15, y - 15, icon_user, 30, 30, color);
        
        uint8_t num_commanders = opp->num_commanders;
        char cmd_dmg_buf[24];
        if (num_commanders == 1) {
            snprintf(cmd_dmg_buf, sizeof(cmd_dmg_buf), "%d", me->cmd_dmg[opp_index + 1][0]);
        } else {
            snprintf(cmd_dmg_buf, sizeof(cmd_dmg_buf), "%d , %d", me->cmd_dmg[opp_index + 1][0], me->cmd_dmg[opp_index + 1][1]);
        }
        ltapp_render_string(x - ltapp_render_text_width(cmd_dmg_buf, FONT_BASE, 1) / 2, y + 17, cmd_dmg_buf, COLOR_RED, FONT_BASE, 1);
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
    ltapp_render_string_centered(LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2), description, COLOR_WHITE, FONT_BASE, 2);

    // Action
    ltapp_render_string_centered(line_y + 30 - ltapp_render_text_height(FONT_BASE, 2) / 2, action, color, FONT_BASE, 2);

    // Cancel
    line_y += 60;
    line_w = ltapp_circle_width_at(LCD_WIDTH_HALF, line_y - LCD_HEIGHT_HALF) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - line_w / 2, line_y, line_w, 2, 2, COLOR_WHITE);
    ltapp_render_string_centered(line_y + (LCD_HEIGHT - line_y) / 2 - ltapp_render_text_height(FONT_BASE, 1) / 2, "CANCEL", COLOR_WHITE, FONT_BASE, 1);

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

    draw_border();

    // Title
    uint8_t y = 24;
    ltapp_render_string_centered(y, "JV", COLOR_MAGENTA, FONT_BASE, 1);
    y += ltapp_render_text_height(FONT_BASE, 1);
    ltapp_render_string_centered(y, "Life Tap", COLOR_MAGENTA, FONT_BASE, 2);
    y += ltapp_render_text_height(FONT_BASE, 2) + 8;
    const uint8_t life_tap_dash_width = ltapp_render_text_width("Life Tap", FONT_BASE, 2) + 8;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - life_tap_dash_width / 2, y, life_tap_dash_width, 2, 2, COLOR_MAGENTA);

    // Menu options
    uint8_t menu_indicator_w = ltapp_render_text_width(">", FONT_BASE, 2) + 8;
    const uint8_t gap_x = 20;
    const uint8_t gap_y = 20;
    uint8_t half_w_at_h = ltapp_circle_width_at(LCD_WIDTH_HALF, 0) / 2;
    ltapp_render_string(LCD_WIDTH_HALF - half_w_at_h + gap_x, LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2, ">", COLOR_WHITE, FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH_HALF - half_w_at_h + gap_x + menu_indicator_w, LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2, "SOLO", COLOR_CYAN, FONT_BASE, 2);

    half_w_at_h = ltapp_circle_width_at(LCD_WIDTH_HALF, 3 * ltapp_render_text_height(FONT_BASE, 2) / 2 + gap_y) / 2;
    ltapp_render_string(LCD_WIDTH_HALF - half_w_at_h + gap_x, LCD_HEIGHT_HALF + ltapp_render_text_height(FONT_BASE, 2) / 2 + gap_y - 2, ">", COLOR_WHITE, FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH_HALF - half_w_at_h + gap_x + menu_indicator_w, LCD_HEIGHT_HALF + ltapp_render_text_height(FONT_BASE, 2) / 2 + gap_y - 2, "NET", COLOR_YELLOW, FONT_BASE, 2);

    menu_indicator_w = ltapp_render_text_width(">", FONT_BASE, 1) + 8;
    uint8_t full_settings_w = menu_indicator_w + ltapp_render_text_width("Settings", FONT_BASE, 1);
    ltapp_render_string(LCD_WIDTH_HALF - full_settings_w / 2, LCD_HEIGHT_HALF + 3 * ltapp_render_text_height(FONT_BASE, 2) / 2 + 3 * gap_y / 2 + 8, ">", COLOR_WHITE, FONT_BASE, 1);
    ltapp_render_string(LCD_WIDTH_HALF - full_settings_w / 2 + menu_indicator_w, LCD_HEIGHT_HALF + 3 * ltapp_render_text_height(FONT_BASE, 2) / 2 + 3 * gap_y / 2 + 8, "SETTINGS", COLOR_GREEN, FONT_BASE, 1);

    // Extras
    ltapp_render_circle_fill(LCD_WIDTH - 48, LCD_HEIGHT_HALF + 20, 56, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH - 48, LCD_HEIGHT_HALF + 20, 56, COLOR_MAGENTA);
    ltapp_render_rect_fill(LCD_WIDTH - 24, LCD_HEIGHT_HALF - 11, 24, 84, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH - 48, LCD_HEIGHT_HALF + 20, 50, COLOR_WHITE);
    ltapp_render_icon(LCD_WIDTH - 68, LCD_HEIGHT_HALF - 12, icon_die, 40, 40, COLOR_WHITE);
    const uint8_t extras_width = ltapp_render_text_width("EXTRAS!", FONT_BASE, 1);
    ltapp_render_string(LCD_WIDTH - 48 - extras_width / 2, LCD_HEIGHT_HALF + 36, "EXTRAS!", COLOR_GRAY, FONT_BASE, 1);

    // TOUCH AREAS
    // uint8_t y_touch = LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2;

    // // - Solo touch area
    // // x0: 0
    // // y0: y_touch - 16
    // // x1: x0 + ltapp_render_text_width("> Solo", FONT_BASE, 2) + 32
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 23
    // ltapp_render_rect(0, y_touch - 16, ltapp_render_text_width("> SOLO", FONT_BASE, 2) + 32, ltapp_render_text_height(FONT_BASE, 2) + 23, COLOR_WHITE);

    // y_touch += ltapp_render_text_height(FONT_BASE, 2) + 8;
    // // - Connect touch area
    // // x0: 0
    // // y0: y_touch + 2
    // // x1: x0 + ltapp_render_text_width("> Net", FONT_BASE, 2) + 52
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(0, y_touch + 2, ltapp_render_text_width("> NET", FONT_BASE, 2) + 52, ltapp_render_text_height(FONT_BASE, 2) + 16, COLOR_WHITE);

    // y_touch += ltapp_render_text_height(FONT_BASE, 2) + 24;
    // // - Settings touch area
    // // x0: margin_left_hpos - 16
    // // y0: y_touch - 8
    // // x1: x0 + ltapp_render_text_width("> Settings", FONT_SMALL, 2) + 32
    // // y1: y0 + ltapp_render_text_height(FONT_SMALL, 2) + 16
    // ltapp_render_rect(LCD_WIDTH_HALF - ltapp_render_text_width("> SETTINGS", FONT_BASE, 1) / 2 - 24, y_touch, ltapp_render_text_width("> SETTINGS", FONT_BASE, 1) + 48, LCD_HEIGHT - y_touch, COLOR_WHITE);

    // // - Extras touch area
    // // cx: LCD_WIDTH - 41
    // // cy: LCD_HEIGHT_HALF + 8
    // // r: 40
    // ltapp_render_circle(LCD_WIDTH - 48, LCD_HEIGHT_HALF + 20, 53, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_settings_name(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0.0", COLOR_DGRAY, FONT_SMALL, 1);
    
    // Page title
    uint8_t y = 24;
    ltapp_render_string_centered(y, "NAME", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Name
    y = LCD_HEIGHT_HALF - 24;
    ltapp_render_string_centered(y, jvlt_player_name(), COLOR_CYAN, FONT_BASE, 2);

    y += ltapp_render_text_height(FONT_BASE, 2) + 4;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - ltapp_render_text_width("ABCDEFGHIJK", FONT_BASE, 2) / 2, y, ltapp_render_text_width("ABCDEFGHIJK", FONT_BASE, 2), 2, 2, COLOR_WHITE);
    y += 8;
    ltapp_render_string_centered(y, "tap to edit", COLOR_GRAY, FONT_BASE, 1);

    draw_page_dots(0, 2, LCD_HEIGHT - 30);

    ltapp_render_flush();
}

static void render_settings_brightness(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0.0", COLOR_DGRAY, FONT_SMALL, 1);
    
    // Page title
    uint8_t y = 24;
    ltapp_render_string_centered(y, "BRIGHTNESS", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Controls
    // Minus
    const uint8_t control_r = 20;
    ltapp_render_circle_fill(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r + 4, COLOR_BLACK);
    ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r + 4, COLOR_MAGENTA);
    ltapp_render_circle(10 + control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_PINK_RED);
    ltapp_render_char(10 + control_r - ltapp_render_text_width("-", FONT_BASE, 2) / 2 + 1, LCD_HEIGHT_HALF + 2 * control_r - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '-', COLOR_PINK_RED, FONT_BASE, 2);

    // Plus
    ltapp_render_circle_fill(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r + 4, COLOR_BLACK);
    ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r + 4, COLOR_MAGENTA);
    ltapp_render_circle(LCD_WIDTH - 10 - control_r, LCD_HEIGHT_HALF + 2 * control_r, control_r, COLOR_CYAN_BRIGHT);
    ltapp_render_char(LCD_WIDTH - 10 - control_r - ltapp_render_text_width("+", FONT_BASE, 2) / 2 + 1, LCD_HEIGHT_HALF + 2 * control_r - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '+', COLOR_CYAN_BRIGHT, FONT_BASE, 2);

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
    ltapp_render_string_centered(LCD_HEIGHT_HALF + bar_h / 2 - 8, label, COLOR_WHITE, FONT_BASE, 3);

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
    uint8_t y = 24;
    ltapp_render_string_centered(y, "SOLO PLAY", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Starting life
    y += 16;
    ltapp_render_string_centered(y, "STARTING LIFE", COLOR_CYAN, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;
    
    const uint8_t row_h = ltapp_render_text_height(FONT_BASE, 1) + 16;

    const uint8_t display_control_w = ltapp_render_text_width("<", FONT_BASE, 2);
    const uint8_t display_control_h = ltapp_render_text_height(FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH_HALF - 80 - display_control_w / 2, y + row_h / 2 - display_control_h / 2, "<", COLOR_CYAN, FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH_HALF + 80 - display_control_w / 2, y + row_h / 2 - display_control_h / 2, ">", COLOR_CYAN, FONT_BASE, 2);

    const int16_t *starting_life_prev = jvlt_match_peek_starting_life(-1);
    if (starting_life_prev) {
        const uint8_t w = ltapp_render_number_width((int)*starting_life_prev, FONT_BASE, 1);
        const uint8_t h = ltapp_render_number_height(FONT_BASE, 1);
        ltapp_render_number(LCD_WIDTH_HALF - 32 - w / 2, y + row_h / 2 - h / 2, (int)*starting_life_prev, COLOR_DGRAY, FONT_BASE, 1);
    }

    const uint8_t central_starting_life_h = ltapp_render_number_height(FONT_BASE, 2);
    ltapp_render_number_centered(y + row_h / 2 - central_starting_life_h / 2, starting_life, COLOR_CYAN, FONT_BASE, 2);

    const int16_t *starting_life_next = jvlt_match_peek_starting_life(1);
    if (starting_life_next) {
        const uint8_t w = ltapp_render_number_width((int)*starting_life_next, FONT_BASE, 1);
        const uint8_t h = ltapp_render_number_height(FONT_BASE, 1);
        ltapp_render_number(LCD_WIDTH_HALF + 32 - w / 2, y + row_h / 2 - h / 2, (int)*starting_life_next, COLOR_DGRAY, FONT_BASE, 1);
    }

    // Opponents
    y += 44;
    ltapp_render_string_centered(y, "OPPONENTS", COLOR_YELLOW, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    ltapp_render_string(LCD_WIDTH_HALF - 80 - display_control_w / 2, y + row_h / 2 - display_control_h / 2, "<", COLOR_YELLOW, FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH_HALF + 80 - display_control_w / 2, y + row_h / 2 - display_control_h / 2, ">", COLOR_YELLOW, FONT_BASE, 2);

    const int16_t *opp_prev = jvlt_match_peek_opponents(-1);
    if (opp_prev) {
        const uint8_t w = ltapp_render_number_width((int)*opp_prev, FONT_BASE, 1);
        const uint8_t h = ltapp_render_number_height(FONT_BASE, 1);
        ltapp_render_number(LCD_WIDTH_HALF - 32 - w / 2, y + row_h / 2 - h / 2, (int)*opp_prev, COLOR_DGRAY, FONT_BASE, 1);
    }

    const uint8_t central_opp_h = ltapp_render_number_height(FONT_BASE, 2);
    ltapp_render_number_centered(y + row_h / 2 - central_opp_h / 2, opponents, COLOR_YELLOW, FONT_BASE, 2);

    const int16_t *opp_next = jvlt_match_peek_opponents(1);
    if (opp_next) {
        const uint8_t w = ltapp_render_number_width((int)*opp_next, FONT_BASE, 1);
        const uint8_t h = ltapp_render_number_height(FONT_BASE, 1);
        ltapp_render_number(LCD_WIDTH_HALF + 32 - w / 2, y + row_h / 2 - h / 2, (int)*opp_next, COLOR_DGRAY, FONT_BASE, 1);
    }

    // Start button
    y += 44;

    const uint8_t bottom_dash_y_from_center = y - R;
    const uint8_t bottom_dash_line_width = ltapp_circle_width_at(R, bottom_dash_y_from_center) - 32;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - bottom_dash_line_width / 2, y, bottom_dash_line_width, 2, 2, COLOR_DGRAY);

    const uint8_t rest = LCD_HEIGHT - y;
    y += rest / 2 - ltapp_render_text_height(FONT_SMALL, 2) / 2;

    ltapp_render_string(LCD_WIDTH_HALF - ltapp_render_text_width("START \x7F", FONT_SMALL, 2) / 2 + 6, y - 4, "START \x7F", COLOR_GREEN, FONT_SMALL, 2);

    // TOUCH AREAS
    // const uint8_t y_touch_initial = 44 + ltapp_render_text_height(FONT_BASE, 1) + ltapp_render_text_height(FONT_BASE, 1);
    // uint8_t y_touch = y_touch_initial;

    // // - Starting life "<" touch area
    // // x0: 0
    // // y0: y_touch
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(0, y_touch - 8, 80, ltapp_render_text_height(FONT_BASE, 2) + 16, COLOR_WHITE);

    // // - Starting life ">" touch area
    // // x0: LCD_WIDTH - 80
    // // y0: y_touch - 8
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(LCD_WIDTH - 80, y_touch - 8, 80, ltapp_render_text_height(FONT_BASE, 2) + 16, COLOR_WHITE);

    // y_touch += ltapp_render_text_height(FONT_BASE, 2) + 32;
    // // - Opponents "<" touch area
    // // x0: 0
    // // y0: y_touch - 8
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(0, y_touch - 8, 80, ltapp_render_text_height(FONT_BASE, 2) + 16, COLOR_WHITE);

    // // - Opponents ">" touch area
    // // x0: LCD_WIDTH - 80
    // // y0: y_touch - 8
    // // x1: x0 + 80
    // // y1: y0 + ltapp_render_text_height(FONT_BASE, 2) + 16
    // ltapp_render_rect(LCD_WIDTH - 80, y_touch - 8, 80, ltapp_render_text_height(FONT_BASE, 2) + 16, COLOR_WHITE);

    // y_touch += ltapp_render_text_height(FONT_BASE, 2) + 16;
    // // - Start match touch area
    // // bellow the horizontal line where y = y_touch
    // ltapp_render_hline(0, y_touch, LCD_WIDTH, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_match(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        int16_t life = m->players[m->my_index].life;
        uint8_t poison = m->players[m->my_index].poison;

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
        
        // Player name
        ltapp_render_string_centered(16, m->players[m->my_index].name, COLOR_GRAY, FONT_SMALL, 1);

        // Life counter
        uint16_t life_color = COLOR_WHITE;
        if (life <= 0) life_color = COLOR_RED;
        else if (life <= 10) life_color = COLOR_ORANGE;
        ltapp_render_number_centered(2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_XLARGE, 4), life, life_color, FONT_XLARGE, 4);

        ltapp_render_char(16, 2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_XLARGE, 4) / 4 - ltapp_render_text_height(FONT_BASE, 2) / 2, '<', COLOR_PINK_RED, FONT_BASE, 2);
        ltapp_render_char(LCD_WIDTH - 16 - ltapp_render_text_width(">", FONT_BASE, 2), 2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_XLARGE, 4) / 4 - ltapp_render_text_height(FONT_BASE, 2) / 2, '>', COLOR_CYAN_BRIGHT, FONT_BASE, 2);

        // Life delta toast
        uint32_t delta_elapsed = (xTaskGetTickCount() - ltapp_life_delta_tick()) * portTICK_PERIOD_MS;
        if (delta_elapsed < LIFE_DELTA_SHOW_MS && ltapp_life_delta() != 0) {
            char dbuf[8];
            snprintf(dbuf, sizeof(dbuf), "%+d", ltapp_life_delta());
            uint16_t dcolor = ltapp_life_delta() > 0 ? COLOR_GREEN : COLOR_RED;
            ltapp_render_string_centered(2 * LCD_HEIGHT / 3 - ltapp_render_number_height(FONT_XLARGE, 4) - 4, dbuf, dcolor, FONT_BASE, 2);
            jvlt_mark_dirty();
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

        // Damage counters
        ltapp_render_circle(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center, control_cy_from_bottom, control_r - 2, COLOR_CYAN);
        ltapp_render_icon(LCD_WIDTH_HALF - control_r - control_perim_dist_from_center - 20, control_cy_from_bottom - 20, icon_crossed_swords, 40, 40, COLOR_CYAN);

        // Extras
        ltapp_render_circle(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center, control_cy_from_bottom, control_r - 2, COLOR_YELLOW);
        ltapp_render_icon(LCD_WIDTH_HALF + control_r + control_perim_dist_from_center - 20, control_cy_from_bottom - 20, icon_die, 40, 40, COLOR_YELLOW);

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
    }

    ltapp_render_flush();
}

static void render_match_cmd_damage(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "commander", COLOR_MAGENTA, FONT_SMALL, 1);
    y = 24;
    ltapp_render_string_centered(y, "DAMAGE", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        const uint8_t opponents = m->player_count - 1;

        switch (opponents) {
            case 1: {
                // Opponent 1
                render_cmd_damage_opps_bubble(0, LCD_WIDTH_HALF, LCD_HEIGHT_HALF - 15, COLOR_CYAN);
                break;
            }

            case 2: {
                const uint8_t points[2][2] = {
                    {LCD_WIDTH_HALF / 2, LCD_HEIGHT_HALF},
                    {3 * LCD_WIDTH / 4, LCD_HEIGHT_HALF},
                };

                // Opponent 1
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW);
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
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN);
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
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN);

                // Opponent 4
                render_cmd_damage_opps_bubble(3, points[3][0], points[3][1], COLOR_ORANGE);
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
                render_cmd_damage_opps_bubble(0, points[0][0], points[0][1], COLOR_CYAN);

                // Opponent 2
                render_cmd_damage_opps_bubble(1, points[1][0], points[1][1], COLOR_YELLOW);

                // Opponent 3
                render_cmd_damage_opps_bubble(2, points[2][0], points[2][1], COLOR_GREEN);

                // Opponent 4
                render_cmd_damage_opps_bubble(3, points[3][0], points[3][1], COLOR_ORANGE);

                // Opponent 5
                render_cmd_damage_opps_bubble(4, points[4][0], points[4][1], COLOR_PINK_RED);
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
    uint8_t y = 24;
    ltapp_render_string_centered(y, opp->name, COLOR_CYAN, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Tap to edit hint
    y += 4;

    ltapp_render_string_centered(y, "tap to edit", COLOR_GRAY, FONT_SMALL, 1);

    const jvlt_player_t *me = &m->players[m->my_index];
    uint8_t ncmd = opp->num_commanders;

    const uint8_t control_r = 20;

    uint8_t y_points[2];
    if (ncmd == 1) {
        y_points[0] = LCD_HEIGHT_HALF;
    } else {
        y_points[0] = LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2) / 2;
        y_points[1] = LCD_HEIGHT_HALF + 24 + ltapp_render_text_height(FONT_BASE, 2) / 2;
    }

    for (uint8_t c = 0; c < ncmd; c++) {
        y = y_points[c];

        char label[8];
        snprintf(label, sizeof(label), "CMD %d:", c + 1);
        ltapp_render_string_centered(y - ltapp_render_text_height(FONT_BASE, 2) / 2 - ltapp_render_text_height(FONT_SMALL, 1) - 2, label, COLOR_GRAY, FONT_SMALL, 1);
        
        int16_t dmg = me->cmd_dmg[m->cmd_sel_opp][c];
        uint16_t color = COLOR_WHITE;
        if (dmg > 20) {
          color = COLOR_RED;
        } else if (dmg > 15) {
          color = COLOR_ORANGE;
        }
        ltapp_render_number_centered(y - ltapp_render_text_height(FONT_BASE, 2) / 2, dmg, color, FONT_BASE, 2);

        ltapp_render_circle(40 + control_r, y, control_r, COLOR_PINK_RED);
        ltapp_render_char(40 + control_r - ltapp_render_text_width("-", FONT_BASE, 2) / 2 + 1, y - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '-', COLOR_PINK_RED, FONT_BASE, 2);

        ltapp_render_circle(LCD_WIDTH - 40 - control_r, y, control_r, COLOR_CYAN_BRIGHT);
        ltapp_render_char(LCD_WIDTH - 40 - control_r - ltapp_render_text_width("+", FONT_BASE, 2) / 2 + 1, y - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '+', COLOR_CYAN_BRIGHT, FONT_BASE, 2);
    }

    if (ncmd < 2) {
        ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE, 1) - 24, "+ COMMANDER", COLOR_GREEN, FONT_BASE, 1);
    } else {
        ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE, 1) - 24, "- COMMANDER", COLOR_ORANGE, FONT_BASE, 1);
    }

    // TOUCH AREAS
    // // - Minus damage (1 commander) touch area
    // // cx: 40 + control_r
    // // cy: LCD_HEIGHT_HALF
    // // r: 3 * control_r
    // ltapp_render_circle(40 + control_r, LCD_HEIGHT_HALF, 3 * control_r, COLOR_WHITE);

    // // - Plus damage (1 commander) touch area
    // // cx: LCD_WIDTH - 40 - control_r
    // // cy: LCD_HEIGHT_HALF
    // // r: 3 * control_r
    // ltapp_render_circle(LCD_WIDTH - 40 - control_r, LCD_HEIGHT_HALF, 3 * control_r, COLOR_WHITE);

    // // - Minus damage 1st commander (2 commanders) touch area
    // // cx: 40 + control_r
    // // cy: LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2) / 2
    // // r: 3 * control_r / 2
    // ltapp_render_circle(40 + control_r, LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2) / 2, 3 * control_r / 2, COLOR_WHITE);

    // // - Plus damage 1st commander (2 commanders) touch area
    // // cx: LCD_WIDTH - 40 - control_r
    // // cy: LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2) / 2
    // // r: 3 * control_r / 2
    // ltapp_render_circle(LCD_WIDTH - 40 - control_r, LCD_HEIGHT_HALF - 8 - ltapp_render_text_height(FONT_BASE, 2) / 2, 3 * control_r / 2, COLOR_WHITE);

    // // - Minus damage 2nd commander (2 commanders) touch area
    // // cx: 40 + control_r
    // // cy: LCD_HEIGHT_HALF + 24 + ltapp_render_text_height(FONT_BASE, 2) / 2
    // // r: 3 * control_r / 2
    // ltapp_render_circle(40 + control_r, LCD_HEIGHT_HALF + 24 + ltapp_render_text_height(FONT_BASE, 2) / 2, 3 * control_r / 2, COLOR_WHITE);

    // // - Plus damage 2nd commander (2 commanders) touch area
    // // cx: LCD_WIDTH - 40 - control_r
    // // cy: LCD_HEIGHT_HALF + 24 + ltapp_render_text_height(FONT_BASE, 2) / 2
    // // r: 3 * control_r / 2
    // ltapp_render_circle(LCD_WIDTH - 40 - control_r, LCD_HEIGHT_HALF + 24 + ltapp_render_text_height(FONT_BASE, 2) / 2, 3 * control_r / 2, COLOR_WHITE);

    // // - Plus/minus commander touch area
    // ltapp_render_rect(0, LCD_HEIGHT - ltapp_render_text_height(FONT_BASE, 1) - 32, LCD_WIDTH, ltapp_render_text_height(FONT_BASE, 1) + 32, COLOR_WHITE);
    // END TOUCH AREAS

    ltapp_render_flush();
}

static void render_match_poison_damage(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 16;
    ltapp_render_string_centered(y, "poison", COLOR_MAGENTA, FONT_SMALL, 1);
    y = 24;
    ltapp_render_string_centered(y, "DAMAGE", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    const jvlt_match_t *m = jvlt_match_get();

    if (m) {
        const uint8_t poison = m->players[m->my_index].poison;

        const uint8_t dot_r = 8;
        const uint8_t gap = 4;
        const uint8_t total_w = (2 * dot_r + gap) * 10 - gap;

        uint8_t y = LCD_HEIGHT_HALF - 2 * dot_r;

        if (poison >= 10)
            ltapp_render_string_centered(y - ltapp_render_text_height(FONT_BASE, 2) - 16, "POISON!", COLOR_GREEN, FONT_BASE, 2);

        for (uint8_t i = 1; i <= 10; i++) {
            if (i <= poison)
                ltapp_render_circle_fill(LCD_WIDTH_HALF - total_w / 2 + (i - 1) * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_GREEN);
            else
                ltapp_render_circle(LCD_WIDTH_HALF - total_w / 2 + (i - 1) * (2 * dot_r + gap) + dot_r, y, dot_r, COLOR_GREEN);
        }

        y += 2 * dot_r + 16;
        
        const uint8_t control_r = 20;
        ltapp_render_circle(40 + control_r, y + control_r, control_r, COLOR_PINK_RED);
        ltapp_render_char(40 + control_r - ltapp_render_text_width("-", FONT_BASE, 2) / 2 + 1, y + control_r - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '-', COLOR_PINK_RED, FONT_BASE, 2);

        ltapp_render_circle(LCD_WIDTH - 40 - control_r, y + control_r, control_r, COLOR_CYAN_BRIGHT);
        ltapp_render_char(LCD_WIDTH - 40 - control_r - ltapp_render_text_width("+", FONT_BASE, 2) / 2 + 1, y + control_r - ltapp_render_text_height(FONT_BASE, 2) / 2 + 1, '+', COLOR_CYAN_BRIGHT, FONT_BASE, 2);

        // TOUCH AREAS
        // // - Minus poison touch area
        // // cx: 40 + control_r
        // // cy: y + control_r
        // // r: 3 * control_r - 8
        // ltapp_render_circle(40 + control_r, y + control_r, 3 * control_r - 8, COLOR_WHITE);

        // // - Plus poison touch area
        // // cx: LCD_WIDTH - 40 - control_r
        // // cy: y + control_r
        // // r: 3 * control_r - 8
        // ltapp_render_circle(LCD_WIDTH - 40 - control_r, y + control_r, 3 * control_r - 8, COLOR_WHITE);
        // END OF TOUCH AREAS
    }

    draw_page_dots(1, 2, LCD_HEIGHT - 20);

    ltapp_render_flush();
}

static void render_match_settings(void) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();
    ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_SMALL, 1) - 5, "v0.0.0", COLOR_DGRAY, FONT_SMALL, 1);

    // Page title
    uint8_t y = 24;
    ltapp_render_string_centered(y, "SETTINGS", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Options
    const uint8_t control_h = ltapp_render_text_height(FONT_SMALL, 2);
    const uint8_t offset_x = ltapp_render_text_width(">", FONT_SMALL, 2) / 2 + 4;

    // Reset match
    uint8_t reset_w = ltapp_render_text_width("RESET MATCH", FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - reset_w / 2 - offset_x, LCD_HEIGHT_HALF - 24 - control_h, ">", COLOR_WHITE, FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - reset_w / 2 + offset_x, LCD_HEIGHT_HALF - 24 - control_h, "RESET MATCH", COLOR_YELLOW, FONT_SMALL, 2);

    // Leave match
    uint8_t leave_w = ltapp_render_text_width("LEAVE MATCH", FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - leave_w / 2 - offset_x, LCD_HEIGHT_HALF + 24, ">", COLOR_WHITE, FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - leave_w / 2 + offset_x, LCD_HEIGHT_HALF + 24, "LEAVE MATCH", COLOR_RED, FONT_SMALL, 2);

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
    uint8_t y = 24;
    ltapp_render_string_centered(y, "EXTRAS", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Menu options
    const uint8_t control_h = ltapp_render_text_height(FONT_SMALL, 2);
    const uint8_t offset_x = ltapp_render_text_width(">", FONT_SMALL, 2) / 2 + 4;

    // Roll a die
    uint8_t roll_die_w = ltapp_render_text_width("ROLL A DIE", FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - roll_die_w / 2 - offset_x, LCD_HEIGHT_HALF - 16 - control_h, ">", COLOR_WHITE, FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - roll_die_w / 2 + offset_x, LCD_HEIGHT_HALF - 16 - control_h, "ROLL A DIE", COLOR_CYAN, FONT_SMALL, 2);

    // Flip a coin
    uint8_t flip_coin_w = ltapp_render_text_width("FLIP A COIN", FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - flip_coin_w / 2 - offset_x, LCD_HEIGHT_HALF + 32, ">", COLOR_WHITE, FONT_SMALL, 2);
    ltapp_render_string(LCD_WIDTH_HALF - flip_coin_w / 2 + offset_x, LCD_HEIGHT_HALF + 32, "FLIP A COIN", COLOR_YELLOW, FONT_SMALL, 2);

    // TOUCH AREAS
    // - Roll a die touch area
    // x0: 0
    // y0: LCD_HEIGHT_HALF - 40 - control_h
    // x1: x0 + LCD_WIDTH
    // y1: y0 + control_h + 48
    // ltapp_render_rect(0, LCD_HEIGHT_HALF - 40 - control_h, LCD_WIDTH, control_h + 48, COLOR_WHITE);
    
    // - Flip a coin touch area
    // x0: 0
    // y0: LCD_HEIGHT_HALF + 12
    // x1: x0 + LCD_WIDTH
    // y1: y0 + control_h + 48
    // ltapp_render_rect(0, LCD_HEIGHT_HALF + 12, LCD_WIDTH, control_h + 48, COLOR_WHITE);
    // END OF TOUCH AREAS

    ltapp_render_flush();
}

static void render_dice(void) {
    ltapp_dice_is_animating(); // tick animation

    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    // Page title
    uint8_t y = 24;
    ltapp_render_string_centered(y, "ROLL A DIE", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
    ltapp_render_hline_dashed(LCD_WIDTH_HALF - top_dash_line_width / 2, y, top_dash_line_width, 2, 2, COLOR_MAGENTA);

    // Select die
    const jvlt_dice_t *d = jvlt_dice_get();
    uint8_t selected_d_sides = d->sides ? d->sides : 20;

    y = LCD_HEIGHT - 16 - ltapp_render_text_height(FONT_BASE, 2);

    const uint8_t row_h = ltapp_render_text_height(FONT_BASE, 1) + 16;

    ltapp_render_string(12, LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2, "<", COLOR_CYAN_BRIGHT, FONT_BASE, 2);
    ltapp_render_string(LCD_WIDTH - 12 - ltapp_render_text_width(">", FONT_BASE, 2), LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 2) / 2, ">", COLOR_CYAN_BRIGHT, FONT_BASE, 2);

    char center_d_label[8];
    snprintf(center_d_label, sizeof(center_d_label), "D%d", selected_d_sides);
    const uint8_t center_die_w = ltapp_render_text_width(center_d_label, FONT_BASE, 2);
    const uint8_t center_die_h = ltapp_render_text_height(FONT_BASE, 2);
    ltapp_render_string_centered(LCD_HEIGHT - row_h / 2 - center_die_h / 2 - 4, center_d_label, COLOR_CYAN_BRIGHT, FONT_BASE, 2);
    
    const int16_t *dice_prev = jvlt_dice_peek_sides(-1);
    if (dice_prev) {
        char label[8];
        snprintf(label, sizeof(label), "D%d", (int)*dice_prev);
        const uint8_t w = ltapp_render_text_width(label, FONT_BASE, 1);
        ltapp_render_string(LCD_WIDTH_HALF - center_die_w / 2 - w / 2 - 20, LCD_HEIGHT - row_h / 2 - center_die_h / 2, label, COLOR_DGRAY, FONT_BASE, 1);
    }

    const int16_t *dice_next = jvlt_dice_peek_sides(1);
    if (dice_next) {
        char label[8];
        snprintf(label, sizeof(label), "D%d", (int)*dice_next);
        const uint8_t w = ltapp_render_text_width(label, FONT_BASE, 1);
        ltapp_render_string(LCD_WIDTH_HALF + center_die_w / 2 - w / 2 + 20, LCD_HEIGHT - row_h / 2 - center_die_h / 2, label, COLOR_DGRAY, FONT_BASE, 1);
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
        ltapp_render_icon_bg(LCD_WIDTH_HALF - 60, LCD_HEIGHT_HALF - 60, die_icon, 120, 120, COLOR_ORANGE);
        ltapp_render_number_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_XLARGE, 3) / 2, dice_val, COLOR_WHITE, FONT_XLARGE, 3);
    } else {
        // "Empty" state
        ltapp_render_string_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 1) / 2, "TAP TO ROLL!", COLOR_GRAY, FONT_BASE, 1);
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
    uint8_t y = 24;
    ltapp_render_string_centered(y, "FLIP A COIN", COLOR_MAGENTA, FONT_BASE, 1);

    y += ltapp_render_text_height(FONT_BASE, 1) + 4;

    const uint8_t R = LCD_WIDTH_HALF;
    const int16_t top_dash_y_from_center = y - R;
    const uint8_t top_dash_line_width = ltapp_circle_width_at(R, top_dash_y_from_center) - 48;
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
            ltapp_render_string_centered(LCD_HEIGHT - ltapp_render_text_height(FONT_BASE, 2) - 16, label, color, FONT_BASE, 2);
        }
    } else {
        // "Empty" state
        ltapp_render_string_centered(LCD_HEIGHT_HALF - ltapp_render_text_height(FONT_BASE, 1) / 2, "TAP TO FLIP!", COLOR_GRAY, FONT_BASE, 1);
    }

    ltapp_render_flush();
}

static void render_keyboard(void) {
    ltapp_kbd_tick();

    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);

    draw_border();

    uint8_t y = 24;
    const uint8_t input_h = ltapp_render_text_height(FONT_SMALL, 2) + 8;

    const uint8_t R = LCD_WIDTH_HALF;
    const uint8_t top_line_y = y + input_h;
    const uint8_t top_line_w = ltapp_circle_width_at(R, top_line_y - R);
    ltapp_render_hline(LCD_WIDTH_HALF - top_line_w / 2 + 1, top_line_y, top_line_w - 2, COLOR_MAGENTA);
    ltapp_render_rect_fill(0, 0, LCD_WIDTH, top_line_y, COLOR_BLACK);

    const uint8_t bottom_line_y = LCD_HEIGHT - y - input_h;
    const uint8_t bottom_line_w = ltapp_circle_width_at(R, bottom_line_y - R);
    ltapp_render_hline(LCD_WIDTH_HALF - bottom_line_w / 2 + 1, bottom_line_y, bottom_line_w - 2, COLOR_MAGENTA);
    ltapp_render_rect_fill(0, bottom_line_y + 1, LCD_WIDTH, LCD_HEIGHT - bottom_line_y, COLOR_BLACK);

    const ltapp_kbd_t *k = ltapp_kbd_get();

    // Input control
    const uint8_t input_w = ltapp_render_text_width("ABCDEFGHIJKL", FONT_SMALL, 2) + 16;
    ltapp_render_rect(LCD_WIDTH_HALF - input_w / 2, y + ltapp_render_text_height(FONT_SMALL, 2) - 2, input_w, 8, COLOR_CYAN);
    ltapp_render_rect_fill(LCD_WIDTH_HALF - input_w / 2 + 1, y + ltapp_render_text_height(FONT_SMALL, 2) - 2, input_w - 2, 7, COLOR_BLACK);
    ltapp_render_string_centered(y, k->buf, COLOR_CYAN, FONT_SMALL, 2);

    // Action controls
    // Cancel
    ltapp_render_string(LCD_WIDTH_HALF - 40, LCD_HEIGHT - (LCD_HEIGHT - bottom_line_y) / 2 - ltapp_render_text_height(FONT_SMALL, 2) / 2, "X", COLOR_RED, FONT_SMALL, 2);

    // Separator
    ltapp_render_vline_dashed(LCD_WIDTH_HALF, bottom_line_y + 8, LCD_HEIGHT - bottom_line_y - 16, 2, 2, COLOR_DGRAY);

    // Confirm
    ltapp_render_string(LCD_WIDTH_HALF + 40 - ltapp_render_text_width("\x7F", FONT_SMALL, 2), LCD_HEIGHT - (LCD_HEIGHT - bottom_line_y) / 2 - ltapp_render_text_height(FONT_SMALL, 2) / 2, "\x7F", COLOR_GREEN, FONT_SMALL, 2);

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
            ltapp_render_string(cx - col_w / 2 + 4, cy - ltapp_render_text_height(FONT_BASE, 1) / 2, nbuf, COLOR_DGRAY, FONT_BASE, 1);
            // Letters
            const char *lbl = labels[r][c];
            char lbuf[5];
            int len = strlen(lbl);
            for (int i = 0; i < len && i < 4; i++)
                lbuf[i] = upper ? lbl[i] : (lbl[i] >= 'A' && lbl[i] <= 'Z' ? lbl[i] + 32 : lbl[i]);
            lbuf[len] = '\0';
            uint8_t lw = ltapp_render_text_width(lbuf, FONT_SMALL, 1);
            ltapp_render_string(cx - lw / 2 + 2, cy - ltapp_render_text_height(FONT_BASE, 1) / 2, lbuf, COLOR_WHITE, FONT_BASE, 1);
        }
    }

    // Bottom row: shift (left half), delete (right half)
    uint8_t bot_y = keyboard_y + 3 * row_h + row_h / 2;

    // Grid lines
    for (int r = 1; r <= 3; r++)
        ltapp_render_hline(keyboard_x + 1, keyboard_y + r * row_h, keyboard_w - 2, COLOR_DGRAY);
    for (int c = 1; c < 3; c++)
        ltapp_render_vline(keyboard_x + c * col_w, keyboard_y + 1, 3 * row_h - 2, COLOR_DGRAY);
    // Bottom row vertical divider
    ltapp_render_vline(keyboard_x + keyboard_w / 2, keyboard_y + 3 * row_h + 1, row_h - 2, COLOR_DGRAY);

    // Shift
    const char *shift_label = k->shift == KBD_SHIFT_LOCK ? "CAPS" : k->shift == KBD_SHIFT_ONCE ? "Shift" : "shift";
    uint16_t shift_color = k->shift != KBD_SHIFT_OFF ? COLOR_YELLOW : COLOR_DGRAY;
    uint8_t sw = ltapp_render_text_width(shift_label, FONT_BASE, 1);
    ltapp_render_string(keyboard_x + keyboard_w / 4 - sw / 2, bot_y - ltapp_render_text_height(FONT_BASE, 1) / 2, shift_label, shift_color, FONT_BASE, 1);
    ltapp_render_string(keyboard_x + 2, bot_y - ltapp_render_text_height(FONT_SMALL, 1) / 2, "0", COLOR_DGRAY, FONT_SMALL, 1);
    // Delete
    const char *del_label = "< DELETE";
    uint8_t dw = ltapp_render_text_width(del_label, FONT_BASE, 1);
    ltapp_render_string(keyboard_x + 3 * keyboard_w / 4 - dw / 2, bot_y - ltapp_render_text_height(FONT_BASE, 1) / 2, del_label, COLOR_RED, FONT_BASE, 1);

    ltapp_render_flush();
}

static void render_placeholder(const char *title) {
    ltapp_render_begin(COLOR_BLACK);
    ltapp_render_set_clip_circle(120, 120, 120);
    draw_border();
    ltapp_render_string_centered(100, title, COLOR_WHITE, FONT_BASE, 2);
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
