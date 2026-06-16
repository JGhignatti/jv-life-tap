#pragma once

#include "jvlt.h"

// --- Shared internal state ---

// Navigation
extern jvlt_screen_t g_screen;

// Match
extern jvlt_match_t  g_match;

// Dice
extern jvlt_dice_t   g_dice;

// Coin
extern jvlt_coin_t   g_coin;

// Net
extern jvlt_net_state_t g_net;

// Settings (stored in match player[0] name and brightness)
extern uint8_t g_brightness;
extern char    g_player_name[12];

// Dirty flag
extern volatile bool g_dirty;

// Queues
extern QueueHandle_t g_pkt_queue;
extern QueueHandle_t g_input_queue;

// Internal comm functions
void jvlt_comm_init(void);
void jvlt_comm_broadcast(const jvlt_packet_t *pkt);
void jvlt_comm_send(const uint8_t *dest_mac, const jvlt_packet_t *pkt);
void jvlt_comm_get_mac(uint8_t *out);
