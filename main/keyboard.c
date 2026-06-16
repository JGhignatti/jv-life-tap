#include "keyboard.h"
#include "renderer.h"
#include <string.h>

static ltapp_kbd_t s_kbd;
static bool s_active = false;
static ltapp_kbd_done_cb_t s_done_cb = NULL;

static const char *s_keys[] = {
    "._",   // key 0 (row0 col0)
    "ABC",  // key 1 (row0 col1)
    "DEF",  // key 2 (row0 col2)
    "GHI",  // key 3 (row1 col0)
    "JKL",  // key 4 (row1 col1)
    "MNO",  // key 5 (row1 col2)
    "PQRS", // key 6 (row2 col0)
    "TUV",  // key 7 (row2 col1)
    "WXYZ", // key 8 (row2 col2)
};

static void commit(void) {
    s_kbd.committed = true;
    // Single shift consumed after commit
    if (s_kbd.shift == KBD_SHIFT_ONCE) s_kbd.shift = KBD_SHIFT_OFF;
}

void ltapp_kbd_open(const char *initial, ltapp_kbd_done_cb_t on_done) {
    strncpy(s_kbd.buf, initial ? initial : "", 11);
    s_kbd.buf[11] = '\0';
    s_kbd.len = strlen(s_kbd.buf);
    s_kbd.key = 0xFF;
    s_kbd.tap_count = 0;
    s_kbd.last_tap_tick = 0;
    s_kbd.committed = true;
    s_kbd.shift = KBD_SHIFT_OFF;
    s_done_cb = on_done;
    s_active = true;
    jvlt_mark_dirty();
}

bool ltapp_kbd_is_active(void) { return s_active; }
const ltapp_kbd_t *ltapp_kbd_get(void) { return &s_kbd; }

bool ltapp_kbd_handle_input(const jvlt_input_event_t *ev) {
    if (!s_active) return false;
    ltapp_kbd_t *k = &s_kbd;

    if (ev->type == INPUT_TAP) {
        int tx = ev->x0, ty = ev->y0;

        // Layout constants
        const uint8_t keyboard_y = 47;
        const uint8_t keyboard_h = 147;
        const uint8_t row_h = keyboard_h / 4;
        const uint8_t bottom_line_y = 194;

        // Cancel/Confirm: below bottom_line_y
        if (ty > bottom_line_y) {
            if (!k->committed) commit();
            if (tx < LCD_WIDTH_HALF) {
                // Cancel
                s_active = false;
            } else {
                // Confirm
                if (s_done_cb && k->len > 0) s_done_cb(k->buf);
                s_active = false;
            }
            jvlt_mark_dirty();
            return true;
        }

        // Above keyboard — ignore
        if (ty < keyboard_y) { jvlt_mark_dirty(); return true; }

        int row = (ty - keyboard_y) / row_h;
        if (row > 3) row = 3;
        int col = (tx - 27) / 62;
        if (col < 0) col = 0;
        if (col > 2) col = 2;

        if (row < 3) {
            // Letter/punctuation key
            int key_idx = row * 3 + col;
            const char *chars = s_keys[key_idx];
            int nchars = strlen(chars);
            bool upper = (k->shift != KBD_SHIFT_OFF);

            if (k->key == key_idx && !k->committed) {
                // Same key, still cycling
                k->tap_count = (k->tap_count + 1) % nchars;
                char c = chars[k->tap_count];
                k->buf[k->len - 1] = (upper && c >= 'A' && c <= 'Z') ? c : (c >= 'A' && c <= 'Z' ? c + 32 : c);
            } else {
                // Different key or first press
                if (!k->committed) commit();  // commit previous character
                // After commit, re-check upper (shift may have been consumed)
                upper = (k->shift != KBD_SHIFT_OFF);
                if (k->len < 11) {
                    char c = chars[0];
                    k->buf[k->len] = (upper && c >= 'A' && c <= 'Z') ? c : (c >= 'A' && c <= 'Z' ? c + 32 : c);
                    k->len++;
                    k->buf[k->len] = '\0';
                    k->tap_count = 0;
                    k->committed = false;
                }
            }
            k->key = key_idx;
            k->last_tap_tick = xTaskGetTickCount();
        } else {
            // Row 3: shift (left half), delete (right half)
            if (!k->committed) commit();
            if (tx < LCD_WIDTH_HALF) {
                // Shift toggle
                if (k->shift == KBD_SHIFT_OFF) k->shift = KBD_SHIFT_ONCE;
                else if (k->shift == KBD_SHIFT_ONCE) k->shift = KBD_SHIFT_LOCK;
                else k->shift = KBD_SHIFT_OFF;
            } else {
                // Delete
                if (k->len > 0) { k->len--; k->buf[k->len] = '\0'; }
            }
        }
        jvlt_mark_dirty();
    } else if (ev->type == INPUT_LONG_PRESS) {
        int tx = ev->x0, ty = ev->y0;
        const uint8_t keyboard_y = 47;
        const uint8_t keyboard_h = 147;
        const uint8_t row_h = keyboard_h / 4;

        if (!k->committed) commit();

        if (ty >= keyboard_y && ty < keyboard_y + 3 * row_h) {
            int row = (ty - keyboard_y) / row_h;
            int col = (tx - 27) / 62;
            if (col < 0) col = 0;
            if (col > 2) col = 2;
            if (k->len < 11) {
                k->buf[k->len] = '1' + (row * 3 + col);
                k->len++;
                k->buf[k->len] = '\0';
                k->committed = true;
            }
            jvlt_mark_dirty();
        } else if (ty >= keyboard_y + 3 * row_h && tx < LCD_WIDTH_HALF) {
            // Long press shift → insert '0'
            if (k->len < 11) {
                k->buf[k->len] = '0';
                k->len++;
                k->buf[k->len] = '\0';
                k->committed = true;
            }
            jvlt_mark_dirty();
        }
    }

    return true;
}

void ltapp_kbd_tick(void) {
    if (!s_active) return;
    ltapp_kbd_t *k = &s_kbd;
    if (!k->committed && k->key != 0xFF) {
        uint32_t now = xTaskGetTickCount();
        if ((now - k->last_tap_tick) * portTICK_PERIOD_MS > 500) {
            commit();
        }
        jvlt_mark_dirty();
    }
}
