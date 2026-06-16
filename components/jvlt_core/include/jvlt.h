#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

// =============================================================================
// Input events
// =============================================================================

typedef enum {
    INPUT_TAP,
    INPUT_DOUBLE_TAP,
    INPUT_LONG_PRESS,
    INPUT_SWIPE_UP,
    INPUT_SWIPE_DOWN,
    INPUT_SWIPE_LEFT,
    INPUT_SWIPE_RIGHT,
    INPUT_TOUCH_DOWN,
    INPUT_TOUCH_UP,
    INPUT_TOUCH_MOVE,
} jvlt_input_type_t;

typedef struct {
    jvlt_input_type_t type;
    int16_t x, y;
    int16_t x0, y0;
} jvlt_input_event_t;

QueueHandle_t jvlt_input_queue(void);

// =============================================================================
// Navigation — current screen
// =============================================================================

typedef enum {
    SCREEN_HOME,
    SCREEN_SETTINGS_NAME,
    SCREEN_SETTINGS_BRIGHTNESS,
    SCREEN_SOLO_SETUP,
    SCREEN_NET_LOBBY,
    SCREEN_MATCH,
    SCREEN_MATCH_CMD_DAMAGE,
    SCREEN_MATCH_POISON_DAMAGE,
    SCREEN_MATCH_SETTINGS,
    SCREEN_EXTRAS,
    SCREEN_MATCH_CMD_DAMAGE_OPP,
    SCREEN_DICE,
    SCREEN_FLIP_COIN,
    SCREEN_CONFIRM_RESET,
    SCREEN_CONFIRM_LEAVE,
} jvlt_screen_t;

jvlt_screen_t jvlt_screen(void);
void          jvlt_set_screen(jvlt_screen_t screen);

// =============================================================================
// Settings (persisted NVS)
// =============================================================================

const char *jvlt_player_name(void);
void        jvlt_set_player_name(const char *name);

uint8_t     jvlt_brightness(void);
void        jvlt_set_brightness(uint8_t level);
void        jvlt_adjust_brightness(int delta);

// =============================================================================
// Match — game session (independent of comms)
// =============================================================================

#define JVLT_MAX_PLAYERS     6
#define JVLT_MAX_COMMANDERS  2

typedef struct {
    char    name[12];
    int16_t life;
    uint8_t poison;
    uint8_t num_commanders;
    int16_t cmd_dmg[JVLT_MAX_PLAYERS][JVLT_MAX_COMMANDERS];
} jvlt_player_t;

typedef struct {
    bool    active;
    uint8_t my_index;
    uint8_t player_count;
    int16_t starting_life;
    uint8_t cmd_sel_opp;        // Currently viewed opponent in CMD_DAMAGE
    jvlt_player_t players[JVLT_MAX_PLAYERS];
} jvlt_match_t;

// Access current match (NULL if no active match)
const jvlt_match_t *jvlt_match_get(void);

// Setup
void jvlt_match_begin_solo(uint8_t opponents, int16_t starting_life);
void jvlt_match_reset_counters(void);
void jvlt_match_end(void);

// In-match actions
void jvlt_match_update_life(int16_t delta);
void jvlt_match_add_cmd_dmg(uint8_t opp, uint8_t commander_idx, int16_t delta);
void jvlt_match_add_poison(int16_t delta);
void jvlt_match_set_num_commanders(uint8_t opp, uint8_t count);
void jvlt_match_nav_opponent(int delta);
void jvlt_match_select_opponent(uint8_t opp_idx);
void jvlt_match_rename_player(uint8_t player_idx, const char *name);
void jvlt_match_set_starting_life(int16_t life);
void jvlt_match_cycle_starting_life(int delta);
const int16_t *jvlt_match_peek_starting_life(int8_t dir);
void jvlt_match_set_opponents(uint8_t count);
const int16_t *jvlt_match_peek_opponents(int8_t dir);

// =============================================================================
// Dice
// =============================================================================

typedef struct {
    uint8_t sides;      // e.g. 6, 20
    uint8_t result;     // last roll result (1..sides)
} jvlt_dice_t;

const jvlt_dice_t *jvlt_dice_get(void);
void  jvlt_dice_roll(uint8_t sides);
void  jvlt_dice_cycle_sides(int8_t dir);  // +1 next, -1 prev
const int16_t *jvlt_dice_peek_sides(int8_t dir); // peek without changing state
void  jvlt_dice_reset(void);

// ─── Generic list cycle utility ──────────────────────────────────────────────
// Finds `value` in `list[count]`, returns list[(idx+dir) % count] (wrapping).
// Returns list[count-1] if value not found.
static inline int16_t jvlt_list_cycle(const int16_t *list, uint8_t count, int16_t value, int8_t dir) {
    int idx = count - 1;
    for (int i = 0; i < count; i++) { if (list[i] == value) { idx = i; break; } }
    return list[(idx + dir + count) % count];
}
// Same but returns NULL at boundaries (no wrap).
static inline const int16_t *jvlt_list_peek(const int16_t *list, uint8_t count, int16_t value, int8_t dir) {
    int idx = count - 1;
    for (int i = 0; i < count; i++) { if (list[i] == value) { idx = i; break; } }
    int next = idx + dir;
    return (next < 0 || next >= count) ? NULL : &list[next];
}

// =============================================================================
// Coin
// =============================================================================

typedef struct {
    bool flipped;       // has been flipped at least once
    bool heads;         // true = heads, false = tails
} jvlt_coin_t;

const jvlt_coin_t *jvlt_coin_get(void);
void  jvlt_coin_flip(void);
void  jvlt_coin_reset(void);

// =============================================================================
// Net — optional multiplayer (room/beacon/sync)
// =============================================================================

#define JVLT_MAGIC 0x4C54

typedef enum {
    MSG_BEACON = 0,
    MSG_JOIN_REQ,
    MSG_JOIN_ACK,
    MSG_PLAYER_LIST,
    MSG_SET_STARTING_LIFE,
    MSG_START_MATCH,
    MSG_LIFE_UPDATE,
    MSG_LEAVE,
} jvlt_msg_type_t;

typedef struct {
    uint8_t mac[6];
    char    name[12];
} jvlt_net_player_id_t;

typedef struct __attribute__((packed)) {
    uint16_t magic;
    uint16_t room_code;
    uint8_t  sender_mac[6];
    uint8_t  msg_type;
    uint8_t  _pad;
    union {
        struct { uint16_t room_code; uint8_t player_count; } beacon;
        struct { char name[12]; } join_req;
        struct { bool accepted; uint8_t player_index; } join_ack;
        struct { uint8_t count; jvlt_net_player_id_t players[JVLT_MAX_PLAYERS]; } player_list;
        struct { int16_t starting_life; } set_life;
        struct { int16_t life; int16_t commander_dmg[JVLT_MAX_PLAYERS]; } life_update;
    } payload;
} jvlt_packet_t;

typedef struct {
    bool     connected;
    bool     is_host;
    uint16_t room_code;
    uint8_t  player_count;
    jvlt_net_player_id_t players[JVLT_MAX_PLAYERS];
} jvlt_net_state_t;

const jvlt_net_state_t *jvlt_net_get(void);
void jvlt_net_create_room(void);
void jvlt_net_scan(void);
void jvlt_net_start_match(void);
bool jvlt_packet_valid(const jvlt_packet_t *pkt, size_t len);

// =============================================================================
// Dirty flag (signals UI needs redraw)
// =============================================================================

bool jvlt_is_dirty(void);
void jvlt_mark_dirty(void);
void jvlt_clear_dirty(void);

// =============================================================================
// Init
// =============================================================================

typedef void (*jvlt_render_cb_t)(void);
typedef void (*jvlt_input_handler_t)(const jvlt_input_event_t *ev);

typedef struct {
    jvlt_input_handler_t on_input;
    jvlt_render_cb_t     on_render;
} jvlt_config_t;

void jvlt_init(const jvlt_config_t *cfg);
