#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "jvlt.h"

typedef void (*ltapp_kbd_done_cb_t)(const char *result);

typedef enum {
    KBD_SHIFT_OFF,   // lowercase
    KBD_SHIFT_ONCE,  // next letter uppercase, then back to off
    KBD_SHIFT_LOCK,  // all uppercase until toggled off
} ltapp_kbd_shift_t;

typedef struct {
    char     buf[12];
    uint8_t  len;
    uint8_t  key;
    uint8_t  tap_count;
    uint32_t last_tap_tick;
    bool     committed;
    ltapp_kbd_shift_t shift;
} ltapp_kbd_t;

// Open keyboard with initial text; on_done called with result on confirm
void ltapp_kbd_open(const char *initial, ltapp_kbd_done_cb_t on_done);
bool ltapp_kbd_is_active(void);
const ltapp_kbd_t *ltapp_kbd_get(void);

// Routes input to keyboard when active. Returns true if event was consumed.
bool ltapp_kbd_handle_input(const jvlt_input_event_t *ev);

// Tick from render loop — handles auto-commit timeout and shift expiry
void ltapp_kbd_tick(void);
