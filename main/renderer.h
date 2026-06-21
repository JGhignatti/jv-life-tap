#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "display.h"

#define RENDER_BUF_LINES  40

#define LCD_WIDTH_HALF  (LCD_WIDTH / 2)
#define LCD_HEIGHT_HALF (LCD_HEIGHT / 2)

#define RGB565_RAW(r, g, b) ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | (((b) & 0xF8) >> 3))
#define RGB565(r, g, b) __builtin_bswap16(RGB565_RAW(r, g, b))

#define COLOR_BLACK         RGB565(0x05, 0x07, 0x0A)
#define COLOR_WHITE         RGB565(0xF2, 0xF4, 0xF8)
#define COLOR_RED           RGB565(0xFF, 0x31, 0x31)
#define COLOR_PINK_RED      RGB565(0xFF, 0x5A, 0x9F)
#define COLOR_GREEN         RGB565(0x5D, 0xFF, 0x6A)
#define COLOR_YELLOW        RGB565(0xFF, 0xD8, 0x3D)
#define COLOR_CYAN          RGB565(0x22, 0xD7, 0xFF)
#define COLOR_CYAN_BRIGHT   RGB565(0x3A, 0xE8, 0xFF)
#define COLOR_MAGENTA       RGB565(0xFF, 0x4F, 0xD8)
#define COLOR_ORANGE        RGB565(0xFF, 0x9B, 0x3D)
#define COLOR_GRAY          RGB565(0xA8, 0xB0, 0xBC)
#define COLOR_DGRAY         RGB565(0x2A, 0x2F, 0x38)

// ─── Fonts ───────────────────────────────────────────────────────────────────

#define FONT_SMALL   0   // 5x7, original LED matrix
#define FONT_BASE_6  1   // FreeMonoBold 6pt
#define FONT_BASE_9  2   // FreeMonoBold 9pt
#define FONT_BASE_12 3   // FreeMonoBold 12pt
#define FONT_BASE_18 4   // FreeMonoBold 18pt
#define FONT_BASE_24 5   // FreeMonoBold 24pt
#define FONT_BASE_9_N 6  // FreeMono 9pt (normal weight)
#define FONT_BASE_36 7   // FreeMonoBold 36pt (digits + minus only)
#define FONT_BASE    FONT_BASE_12  // Default base font

// GFXfont types (used internally by the renderer)
typedef struct {
    uint16_t bitmapOffset;
    uint8_t  width;
    uint8_t  height;
    uint8_t  xAdvance;
    int8_t   xOffset;
    int8_t   yOffset;
} ltapp_gfx_glyph_t;

typedef struct {
    const uint8_t           *bitmap;
    const ltapp_gfx_glyph_t *glyph;
    uint8_t                  first;
    uint8_t                  last;
    uint8_t                  yAdvance;
} ltapp_gfx_font_t;

// ─── Lifecycle ───────────────────────────────────────────────────────────────

void ltapp_render_init(void);
void ltapp_render_begin(uint16_t bg_color);
void ltapp_render_flush(void);

// ─── Clipping ────────────────────────────────────────────────────────────────

void ltapp_render_set_clip_circle(int cx, int cy, int r);
void ltapp_render_clear_clip(void);

// ─── Primitives ──────────────────────────────────────────────────────────────

void ltapp_render_rect(int x, int y, int w, int h, uint16_t color);
void ltapp_render_rect_fill(int x, int y, int w, int h, uint16_t color);
void ltapp_render_rounded_rect(int x, int y, int w, int h, int r, uint16_t color);
void ltapp_render_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color);
void ltapp_render_circle(int cx, int cy, int r, uint16_t color);
void ltapp_render_circle_fill(int cx, int cy, int r, uint16_t color);
void ltapp_render_hline(int x, int y, int w, uint16_t color);
void ltapp_render_vline(int x, int y, int h, uint16_t color);
void ltapp_render_hline_dashed(int x, int y, int w, int dash, int gap, uint16_t color);
void ltapp_render_vline_dashed(int x, int y, int h, int dash, int gap, uint16_t color);

// ─── Text ────────────────────────────────────────────────────────────────────

void ltapp_render_char(int x, int y, char c, uint16_t color, int font, int scale);
void ltapp_render_string(int x, int y, const char *str, uint16_t color, int font, int scale);
void ltapp_render_string_centered(int y, const char *str, uint16_t color, int font, int scale);
void ltapp_render_string_spaced(int x, int y, const char *str, uint16_t color, int font, int scale, int space_w);
void ltapp_render_string_spaced_centered(int y, const char *str, uint16_t color, int font, int scale, int space_w);
void ltapp_render_number(int x, int y, int num, uint16_t color, int font, int scale);
void ltapp_render_number_centered(int y, int num, uint16_t color, int font, int scale);

// ─── Icons ───────────────────────────────────────────────────────────────────

void ltapp_render_icon(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color);
void ltapp_render_icon_bg(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color);

// ─── Measurement ─────────────────────────────────────────────────────────────

int ltapp_render_text_width(const char *str, int font, int scale);
int ltapp_render_text_width_spaced(const char *str, int font, int scale, int space_w);
int ltapp_render_number_width(int num, int font, int scale);
int ltapp_render_text_height(int font, int scale);
int ltapp_render_number_height(int font, int scale);
int ltapp_circle_width_at(int r, int y_from_center);
int ltapp_circle_height_at(int r, int x_from_center);
