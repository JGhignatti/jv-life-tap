#include "renderer.h"
#include "display.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

// ─── 5x7 Font ────────────────────────────────────────────────────────────────
#define FONT5_W 5
#define FONT5_H 7
static const uint8_t font5x7[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x00,0x07,0x00,0x07,0x00}, // "
    {0x14,0x7F,0x14,0x7F,0x14}, // #
    {0x24,0x2A,0x7F,0x2A,0x12}, // $
    {0x23,0x13,0x08,0x64,0x62}, // %
    {0x36,0x49,0x55,0x22,0x50}, // &
    {0x00,0x05,0x03,0x00,0x00}, // '
    {0x00,0x1C,0x22,0x41,0x00}, // (
    {0x00,0x41,0x22,0x1C,0x00}, // )
    {0x08,0x2A,0x1C,0x2A,0x08}, // *
    {0x08,0x08,0x3E,0x08,0x08}, // +
    {0x00,0x50,0x30,0x00,0x00}, // ,
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}, // 9
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x00,0x56,0x36,0x00,0x00}, // ;
    {0x00,0x08,0x14,0x22,0x41}, // <
    {0x14,0x14,0x14,0x14,0x14}, // =
    {0x41,0x22,0x14,0x08,0x00}, // >
    {0x02,0x01,0x51,0x09,0x06}, // ?
    {0x32,0x49,0x79,0x41,0x3E}, // @
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x01,0x01}, // F
    {0x3E,0x41,0x41,0x51,0x32}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x04,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x03,0x04,0x78,0x04,0x03}, // Y
    {0x61,0x51,0x49,0x45,0x43}, // Z
    {0x00,0x00,0x7F,0x41,0x41}, // [
    {0x02,0x04,0x08,0x10,0x20}, // backslash
    {0x41,0x41,0x7F,0x00,0x00}, // ]
    {0x04,0x02,0x01,0x02,0x04}, // ^
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x00,0x01,0x02,0x04,0x00}, // `
    {0x20,0x54,0x54,0x54,0x78}, // a
    {0x7F,0x48,0x44,0x44,0x38}, // b
    {0x38,0x44,0x44,0x44,0x20}, // c
    {0x38,0x44,0x44,0x48,0x7F}, // d
    {0x38,0x54,0x54,0x54,0x18}, // e
    {0x08,0x7E,0x09,0x01,0x02}, // f
    {0x08,0x54,0x54,0x54,0x3C}, // g
    {0x7F,0x08,0x04,0x04,0x78}, // h
    {0x00,0x44,0x7D,0x40,0x00}, // i
    {0x20,0x40,0x44,0x3D,0x00}, // j
    {0x00,0x7F,0x10,0x28,0x44}, // k
    {0x00,0x41,0x7F,0x40,0x00}, // l
    {0x7C,0x04,0x18,0x04,0x78}, // m
    {0x7C,0x08,0x04,0x04,0x78}, // n
    {0x38,0x44,0x44,0x44,0x38}, // o
    {0x7C,0x14,0x14,0x14,0x08}, // p
    {0x08,0x14,0x14,0x18,0x7C}, // q
    {0x7C,0x08,0x04,0x04,0x08}, // r
    {0x48,0x54,0x54,0x54,0x20}, // s
    {0x04,0x3F,0x44,0x40,0x20}, // t
    {0x3C,0x40,0x40,0x20,0x7C}, // u
    {0x1C,0x20,0x40,0x20,0x1C}, // v
    {0x3C,0x40,0x30,0x40,0x3C}, // w
    {0x44,0x28,0x10,0x28,0x44}, // x
    {0x0C,0x50,0x50,0x50,0x3C}, // y
    {0x44,0x64,0x54,0x4C,0x44}, // z
    {0x00,0x08,0x36,0x41,0x00}, // {
    {0x00,0x00,0x7F,0x00,0x00}, // |
    {0x00,0x41,0x36,0x08,0x00}, // }
    {0x08,0x04,0x08,0x10,0x08}, // ~
    {0x7F,0x3E,0x1C,0x08,0x00}, // ▶ (char 127) '\x7F'
};

// ─── 16x32 Terminus Font ─────────────────────────────────────────────────────
// Digits only. Column-encoded: each column is uint32_t, bit 31 = top row.

// ─── Strip buffers (double-buffered) ─────────────────────────────────────────
static uint16_t s_strip[2][LCD_WIDTH * RENDER_BUF_LINES];
static int s_buf_idx = 0;
static uint16_t s_bg_color;

// ─── Clipping ────────────────────────────────────────────────────────────────
static bool s_clip_enabled = false;
static int  s_clip_cx, s_clip_cy, s_clip_r_sq;

// ─── GFXfont data (FreeMonoBold) ─────────────────────────────────────────────
typedef ltapp_gfx_glyph_t GFXglyph;
typedef ltapp_gfx_font_t GFXfont;

#include "fonts/FreeMonoBold6pt7b.h"
#include "fonts/FreeMonoBold9pt7b.h"
#include "fonts/FreeMonoBold12pt7b.h"
#include "fonts/FreeMonoBold18pt7b.h"
#include "fonts/FreeMonoBold24pt7b.h"
#include "fonts/FreeMono9pt7b.h"
#include "fonts/FreeMonoBold36pt_num.h"

static const ltapp_gfx_font_t *s_gfxfonts[] = {
    [FONT_BASE_6]  = &FreeMonoBold6pt7b,
    [FONT_BASE_9]  = &FreeMonoBold9pt7b,
    [FONT_BASE_12] = &FreeMonoBold12pt7b,
    [FONT_BASE_18] = &FreeMonoBold18pt7b,
    [FONT_BASE_24] = &FreeMonoBold24pt7b,
    [FONT_BASE_9_N] = &FreeMono9pt7b,
    [FONT_BASE_36] = &FreeMonoBold36pt7b,
};

static const ltapp_gfx_font_t *get_gfxfont(int font_id) {
    if (font_id >= FONT_BASE_6 && font_id <= FONT_BASE_36) return s_gfxfonts[font_id];
    return NULL;
}

// ─── Primitive list ──────────────────────────────────────────────────────────
typedef enum {
    PRIM_RECT_FILL, PRIM_RECT, PRIM_CIRCLE_FILL, PRIM_CIRCLE,
    PRIM_HLINE, PRIM_VLINE, PRIM_RRECT, PRIM_RRECT_FILL, PRIM_CHAR
} prim_type_t;

typedef struct {
    prim_type_t type;
    int16_t x, y, w, h, r;
    uint16_t color;
    char ch;
    uint8_t scale;
    uint8_t font;
} prim_t;

#define MAX_PRIMS 256
static prim_t s_prims[MAX_PRIMS];
static int s_prim_count = 0;

// Icon entries (rendered separately, not as prims)
typedef struct {
    const uint8_t *data;
    int16_t x, y, w, h;
    uint16_t color;
} icon_t;

#define MAX_ICONS 8
static icon_t s_icons[MAX_ICONS];
static int s_icon_count = 0;

static icon_t s_bg_icon;
static bool s_bg_icon_set = false;

// ─── Helpers ─────────────────────────────────────────────────────────────────

static inline bool clip_test(int px, int py) {
    if (!s_clip_enabled) return true;
    int dx = px - s_clip_cx;
    int dy = py - s_clip_cy;
    return (dx * dx + dy * dy) <= s_clip_r_sq;
}

static inline void put_pixel(uint16_t *buf, int strip_y, int px, int py, uint16_t color) {
    if (px < 0 || px >= LCD_WIDTH) return;
    if (py < strip_y || py >= strip_y + RENDER_BUF_LINES) return;
    if (!clip_test(px, py)) return;
    buf[(py - strip_y) * LCD_WIDTH + px] = color;
}

static void add_prim(prim_t p) {
    if (s_prim_count < MAX_PRIMS) s_prims[s_prim_count++] = p;
}

// ─── Public API ──────────────────────────────────────────────────────────────

void ltapp_render_init(void) {
    s_prim_count = 0;
    s_icon_count = 0;
}

void ltapp_render_begin(uint16_t bg_color) {
    s_bg_color = bg_color;
    s_prim_count = 0;
    s_icon_count = 0;
    s_bg_icon_set = false;
    s_clip_enabled = false;
}

void ltapp_render_set_clip_circle(int cx, int cy, int r) {
    s_clip_enabled = true;
    s_clip_cx = cx;
    s_clip_cy = cy;
    s_clip_r_sq = r * r;
}

void ltapp_render_clear_clip(void) {
    s_clip_enabled = false;
}

void ltapp_render_rect(int x, int y, int w, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RECT, .x = x, .y = y, .w = w, .h = h, .color = color});
}

void ltapp_render_rect_fill(int x, int y, int w, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RECT_FILL, .x = x, .y = y, .w = w, .h = h, .color = color});
}

void ltapp_render_rounded_rect(int x, int y, int w, int h, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RRECT, .x = x, .y = y, .w = w, .h = h, .r = r, .color = color});
}

void ltapp_render_rounded_rect_fill(int x, int y, int w, int h, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_RRECT_FILL, .x = x, .y = y, .w = w, .h = h, .r = r, .color = color});
}

void ltapp_render_circle(int cx, int cy, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_CIRCLE, .x = cx, .y = cy, .r = r, .color = color});
}

void ltapp_render_circle_fill(int cx, int cy, int r, uint16_t color) {
    add_prim((prim_t){.type = PRIM_CIRCLE_FILL, .x = cx, .y = cy, .r = r, .color = color});
}

void ltapp_render_hline(int x, int y, int w, uint16_t color) {
    add_prim((prim_t){.type = PRIM_HLINE, .x = x, .y = y, .w = w, .color = color});
}

void ltapp_render_vline(int x, int y, int h, uint16_t color) {
    add_prim((prim_t){.type = PRIM_VLINE, .x = x, .y = y, .h = h, .color = color});
}

void ltapp_render_hline_dashed(int x, int y, int w, int dash, int gap, uint16_t color) {
    int step = dash + gap;
    for (int i = 0; i < w; i += step) {
        int len = (i + dash > w) ? w - i : dash;
        add_prim((prim_t){.type = PRIM_HLINE, .x = x + i, .y = y, .w = len, .color = color});
    }
}

void ltapp_render_vline_dashed(int x, int y, int h, int dash, int gap, uint16_t color) {
    int step = dash + gap;
    for (int i = 0; i < h; i += step) {
        int len = (i + dash > h) ? h - i : dash;
        add_prim((prim_t){.type = PRIM_VLINE, .x = x, .y = y + i, .h = len, .color = color});
    }
}

void ltapp_render_char(int x, int y, char c, uint16_t color, int font, int scale) {
    add_prim((prim_t){.type = PRIM_CHAR, .x = x, .y = y, .color = color, .ch = c, .scale = scale, .font = font});
}

void ltapp_render_string(int x, int y, const char *str, uint16_t color, int font, int scale) {
    const ltapp_gfx_font_t *gfx = get_gfxfont(font);
    if (gfx) {
        while (*str) {
            if (*str >= gfx->first && *str <= gfx->last) {
                ltapp_render_char(x, y, *str, color, font, scale);
                x += gfx->glyph[*str - gfx->first].xAdvance * scale;
            }
            str++;
        }
        return;
    }
    int cw;
    
    cw = FONT5_W;
    while (*str) {
        ltapp_render_char(x, y, *str, color, font, scale);
        x += cw * scale + 1;
        str++;
    }
}

void ltapp_render_string_centered(int y, const char *str, uint16_t color, int font, int scale) {
    int w = ltapp_render_text_width(str, font, scale);
    ltapp_render_string((LCD_WIDTH - w) / 2, y, str, color, font, scale);
}

void ltapp_render_string_spaced(int x, int y, const char *str, uint16_t color, int font, int scale, int space_w) {
    const ltapp_gfx_font_t *gfx = get_gfxfont(font);
    if (gfx) {
        while (*str) {
            if (*str >= gfx->first && *str <= gfx->last) {
                if (*str == ' ') { x += space_w * scale; }
                else {
                    ltapp_render_char(x, y, *str, color, font, scale);
                    x += gfx->glyph[*str - gfx->first].xAdvance * scale;
                }
            }
            str++;
        }
        return;
    }
    ltapp_render_string(x, y, str, color, font, scale);
}

void ltapp_render_string_spaced_centered(int y, const char *str, uint16_t color, int font, int scale, int space_w) {
    int w = ltapp_render_text_width_spaced(str, font, scale, space_w);
    ltapp_render_string_spaced((LCD_WIDTH - w) / 2, y, str, color, font, scale, space_w);
}

void ltapp_render_number(int x, int y, int num, uint16_t color, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    ltapp_render_string(x, y, buf, color, font, scale);
}

void ltapp_render_number_centered(int y, int num, uint16_t color, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    ltapp_render_string_centered(y, buf, color, font, scale);
}

void ltapp_render_icon(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color) {
    if (s_icon_count >= MAX_ICONS) return;
    s_icons[s_icon_count++] = (icon_t){.data = bitmap, .x = x, .y = y, .w = w, .h = h, .color = color};
}

void ltapp_render_icon_bg(int x, int y, const uint8_t *bitmap, int w, int h, uint16_t color) {
    s_bg_icon = (icon_t){.data = bitmap, .x = x, .y = y, .w = w, .h = h, .color = color};
    s_bg_icon_set = true;
}

int ltapp_render_text_width(const char *str, int font, int scale) {
    int len = strlen(str);
    if (len == 0) return 0;
    const ltapp_gfx_font_t *gfx = get_gfxfont(font);
    if (gfx) {
        int w = 0;
        while (*str) {
            if (*str >= gfx->first && *str <= gfx->last)
                w += gfx->glyph[*str - gfx->first].xAdvance * scale;
            str++;
        }
        return w;
    }
    int cw;
    
    cw = FONT5_W;
    return len * (cw * scale + 1) - 1;
}

int ltapp_render_text_width_spaced(const char *str, int font, int scale, int space_w) {
    int len = strlen(str);
    if (len == 0) return 0;
    const ltapp_gfx_font_t *gfx = get_gfxfont(font);
    if (gfx) {
        int w = 0;
        while (*str) {
            if (*str >= gfx->first && *str <= gfx->last) {
                if (*str == ' ') w += space_w * scale;
                else w += gfx->glyph[*str - gfx->first].xAdvance * scale;
            }
            str++;
        }
        return w;
    }
    int cw;
    
    cw = FONT5_W;
    return len * (cw * scale + 1) - 1;
}

int ltapp_render_number_width(int num, int font, int scale) {
    char buf[12];
    snprintf(buf, sizeof(buf), "%d", num);
    return ltapp_render_text_width(buf, font, scale);
}

int ltapp_render_text_height(int font, int scale) {
    const ltapp_gfx_font_t *gfx = get_gfxfont(font);
    if (gfx) {
        uint8_t max_h = 0;
        for (int i = 0; i <= gfx->last - gfx->first; i++) {
            if (gfx->glyph[i].height > max_h) max_h = gfx->glyph[i].height;
        }
        return max_h * scale;
    }
    
    return 7 * scale;
}

int ltapp_render_number_height(int font, int scale) {
    return ltapp_render_text_height(font, scale);
}

int ltapp_circle_width_at(int r, int y_from_center) {
    if (abs(y_from_center) >= r) return 0;
    return 2 * (int)sqrtf((float)(r * r - y_from_center * y_from_center));
}

int ltapp_circle_height_at(int r, int x_from_center) {
    if (abs(x_from_center) >= r) return 0;
    return 2 * (int)sqrtf((float)(r * r - x_from_center * x_from_center));
}

// ─── Rasterizer ──────────────────────────────────────────────────────────────

static void rast_hline(uint16_t *buf, int strip_y, int x0, int x1, int y, uint16_t color) {
    if (y < strip_y || y >= strip_y + RENDER_BUF_LINES) return;
    if (x0 < 0) x0 = 0;
    if (x1 > LCD_WIDTH) x1 = LCD_WIDTH;
    uint16_t *line = &buf[(y - strip_y) * LCD_WIDTH];
    if (s_clip_enabled) {
        for (int x = x0; x < x1; x++) {
            if (clip_test(x, y)) line[x] = color;
        }
    } else {
        for (int x = x0; x < x1; x++) line[x] = color;
    }
}

static void rast_circle_fill(uint16_t *buf, int strip_y, int cx, int cy, int r, uint16_t color) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    int y0 = cy - r < sy0 ? sy0 : cy - r;
    int y1 = cy + r >= sy1 ? sy1 - 1 : cy + r;
    int r_sq = r * r;
    for (int row = y0; row <= y1; row++) {
        int dy = row - cy;
        int dx = (int)sqrtf((float)(r_sq - dy * dy));
        rast_hline(buf, strip_y, cx - dx, cx + dx + 1, row, color);
    }
}

static void rast_rounded_rect_fill(uint16_t *buf, int strip_y, int x, int y, int w, int h, int r, uint16_t color) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    int y0 = y < sy0 ? sy0 : y;
    int y1 = (y + h) > sy1 ? sy1 : (y + h);

    for (int row = y0; row < y1; row++) {
        int lx = x, rx = x + w;
        // Top corners
        if (row < y + r) {
            int dy = (y + r) - row;
            int dx = r - (int)sqrtf((float)(r * r - dy * dy));
            lx += dx;
            rx -= dx;
        }
        // Bottom corners
        if (row >= y + h - r) {
            int dy = row - (y + h - r - 1);
            int dx = r - (int)sqrtf((float)(r * r - dy * dy));
            lx += dx;
            rx -= dx;
        }
        rast_hline(buf, strip_y, lx, rx, row, color);
    }
}

static void rast_rounded_rect(uint16_t *buf, int strip_y, int x, int y, int w, int h, int r, uint16_t color) {
    // Top/bottom edges (between corners)
    rast_hline(buf, strip_y, x + r, x + w - r, y, color);
    rast_hline(buf, strip_y, x + r, x + w - r, y + h - 1, color);
    // Left/right edges (between corners)
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;
    for (int row = y + r; row < y + h - r; row++) {
        if (row < sy0 || row >= sy1) continue;
        put_pixel(buf, strip_y, x, row, color);
        put_pixel(buf, strip_y, x + w - 1, row, color);
    }
    // Corner arcs (Bresenham quarter-circles)
    int cx, cy_c, px = r, py_c = 0, err = 1 - r;
    while (px >= py_c) {
        // Top-left
        cx = x + r; cy_c = y + r;
        put_pixel(buf, strip_y, cx - px, cy_c - py_c, color);
        put_pixel(buf, strip_y, cx - py_c, cy_c - px, color);
        // Top-right
        cx = x + w - 1 - r; cy_c = y + r;
        put_pixel(buf, strip_y, cx + px, cy_c - py_c, color);
        put_pixel(buf, strip_y, cx + py_c, cy_c - px, color);
        // Bottom-left
        cx = x + r; cy_c = y + h - 1 - r;
        put_pixel(buf, strip_y, cx - px, cy_c + py_c, color);
        put_pixel(buf, strip_y, cx - py_c, cy_c + px, color);
        // Bottom-right
        cx = x + w - 1 - r; cy_c = y + h - 1 - r;
        put_pixel(buf, strip_y, cx + px, cy_c + py_c, color);
        put_pixel(buf, strip_y, cx + py_c, cy_c + px, color);

        py_c++;
        if (err < 0) {
            err += 2 * py_c + 1;
        } else {
            px--;
            err += 2 * (py_c - px) + 1;
        }
    }
}

static void rast_char(uint16_t *buf, int strip_y, const prim_t *p) {
    if (p->ch < 32 || p->ch > 127) return;
    int idx = p->ch - 32;
    int scale = p->scale;

    // GFXfont (proportional) rendering
    const ltapp_gfx_font_t *gfx = get_gfxfont(p->font);
    if (gfx) {
        if (p->ch < gfx->first || p->ch > gfx->last) return;
        int8_t min_yoff = 0;
        for (int i = 0; i <= gfx->last - gfx->first; i++) {
            if (gfx->glyph[i].yOffset < min_yoff) min_yoff = gfx->glyph[i].yOffset;
        }
        int base_y = p->y - min_yoff * scale;
        const ltapp_gfx_glyph_t *g = &gfx->glyph[p->ch - gfx->first];
        const uint8_t *bmp = gfx->bitmap + g->bitmapOffset;
        int bit = 0;
        for (int row = 0; row < g->height; row++) {
            for (int col = 0; col < g->width; col++) {
                if (bmp[bit / 8] & (0x80 >> (bit % 8))) {
                    int px = p->x + (g->xOffset + col) * scale;
                    int py = base_y + (g->yOffset + row) * scale;
                    for (int sy = 0; sy < scale; sy++)
                        for (int sx = 0; sx < scale; sx++)
                            put_pixel(buf, strip_y, px + sx, py + sy, p->color);
                }
                bit++;
            }
        }
        return;
    }

    // FONT_SMALL fallback
    {
        const uint8_t *glyph = font5x7[idx];
        for (int col = 0; col < FONT5_W; col++) {
            uint8_t bits = glyph[col];
            for (int row = 0; row < FONT5_H; row++) {
                if (!(bits & (1 << row))) continue;
                int px_base = p->x + col * scale;
                int py_base = p->y + row * scale;
                for (int sy = 0; sy < scale; sy++) {
                    for (int sx = 0; sx < scale; sx++) {
                        put_pixel(buf, strip_y, px_base + sx, py_base + sy, p->color);
                    }
                }
            }
        }
    }
}

static void rasterize_prim(const prim_t *p, int strip_y, uint16_t *buf) {
    int sy0 = strip_y;
    int sy1 = strip_y + RENDER_BUF_LINES;

    switch (p->type) {
    case PRIM_RECT_FILL: {
        int y0 = p->y < sy0 ? sy0 : p->y;
        int y1 = (p->y + p->h) > sy1 ? sy1 : (p->y + p->h);
        for (int row = y0; row < y1; row++) {
            rast_hline(buf, strip_y, p->x, p->x + p->w, row, p->color);
        }
        break;
    }
    case PRIM_RECT: {
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y, p->color);
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y + p->h - 1, p->color);
        int y0 = p->y + 1 < sy0 ? sy0 : p->y + 1;
        int y1 = (p->y + p->h - 1) > sy1 ? sy1 : (p->y + p->h - 1);
        for (int row = y0; row < y1; row++) {
            put_pixel(buf, strip_y, p->x, row, p->color);
            put_pixel(buf, strip_y, p->x + p->w - 1, row, p->color);
        }
        break;
    }
    case PRIM_CIRCLE_FILL:
        rast_circle_fill(buf, strip_y, p->x, p->y, p->r, p->color);
        break;
    case PRIM_CIRCLE: {
        // Bresenham circle outline
        int cx = p->x, cy = p->y, r = p->r;
        int px = r, py = 0, err = 1 - r;
        while (px >= py) {
            put_pixel(buf, strip_y, cx + px, cy + py, p->color);
            put_pixel(buf, strip_y, cx - px, cy + py, p->color);
            put_pixel(buf, strip_y, cx + px, cy - py, p->color);
            put_pixel(buf, strip_y, cx - px, cy - py, p->color);
            put_pixel(buf, strip_y, cx + py, cy + px, p->color);
            put_pixel(buf, strip_y, cx - py, cy + px, p->color);
            put_pixel(buf, strip_y, cx + py, cy - px, p->color);
            put_pixel(buf, strip_y, cx - py, cy - px, p->color);
            py++;
            if (err < 0) {
                err += 2 * py + 1;
            } else {
                px--;
                err += 2 * (py - px) + 1;
            }
        }
        break;
    }
    case PRIM_HLINE:
        rast_hline(buf, strip_y, p->x, p->x + p->w, p->y, p->color);
        break;
    case PRIM_VLINE: {
        int y0 = p->y < sy0 ? sy0 : p->y;
        int y1 = (p->y + p->h) > sy1 ? sy1 : (p->y + p->h);
        for (int row = y0; row < y1; row++) {
            put_pixel(buf, strip_y, p->x, row, p->color);
        }
        break;
    }
    case PRIM_RRECT_FILL:
        rast_rounded_rect_fill(buf, strip_y, p->x, p->y, p->w, p->h, p->r, p->color);
        break;
    case PRIM_RRECT:
        rast_rounded_rect(buf, strip_y, p->x, p->y, p->w, p->h, p->r, p->color);
        break;
    case PRIM_CHAR:
        rast_char(buf, strip_y, p);
        break;
    }
}

void ltapp_render_flush(void) {
    for (int strip_y = 0; strip_y < LCD_HEIGHT; strip_y += RENDER_BUF_LINES) {
        int lines = (strip_y + RENDER_BUF_LINES > LCD_HEIGHT) ? (LCD_HEIGHT - strip_y) : RENDER_BUF_LINES;
        uint16_t *buf = s_strip[s_buf_idx];

        // Fill background (with clip)
        for (int row = 0; row < lines; row++) {
            for (int col = 0; col < LCD_WIDTH; col++) {
                buf[row * LCD_WIDTH + col] = s_bg_color;
            }
        }

        // Rasterize background icon (underneath primitives)
        if (s_bg_icon_set) {
            icon_t *ic = &s_bg_icon;
            int stride = (ic->w + 7) / 8;
            for (int row = 0; row < ic->h; row++) {
                int py = ic->y + row;
                if (py < strip_y || py >= strip_y + lines) continue;
                for (int col = 0; col < ic->w; col++) {
                    if (ic->data[row * stride + col / 8] & (0x80 >> (col % 8))) {
                        int px = ic->x + col;
                        if (px >= 0 && px < LCD_WIDTH && clip_test(px, py))
                            buf[(py - strip_y) * LCD_WIDTH + px] = ic->color;
                    }
                }
            }
        }

        // Rasterize all primitives
        for (int i = 0; i < s_prim_count; i++) {
            rasterize_prim(&s_prims[i], strip_y, buf);
        }

        // Rasterize icons
        for (int i = 0; i < s_icon_count; i++) {
            icon_t *ic = &s_icons[i];
            int stride = (ic->w + 7) / 8;
            for (int row = 0; row < ic->h; row++) {
                int py = ic->y + row;
                if (py < strip_y || py >= strip_y + lines) continue;
                for (int col = 0; col < ic->w; col++) {
                    if (ic->data[row * stride + col / 8] & (0x80 >> (col % 8))) {
                        int px = ic->x + col;
                        if (px >= 0 && px < LCD_WIDTH && clip_test(px, py))
                            buf[(py - strip_y) * LCD_WIDTH + px] = ic->color;
                    }
                }
            }
        }

        ltapp_display_flush_area(0, strip_y, LCD_WIDTH, lines, buf);
        s_buf_idx ^= 1;
    }
}
