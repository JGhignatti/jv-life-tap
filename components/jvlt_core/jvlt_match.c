#include "jvlt_internal.h"
#include <string.h>
#include <stdio.h>

jvlt_match_t g_match;

const jvlt_match_t *jvlt_match_get(void) {
    return g_match.active ? &g_match : NULL;
}

void jvlt_match_begin_solo(uint8_t opponents, int16_t starting_life) {
    memset(&g_match, 0, sizeof(g_match));
    g_match.active = true;
    g_match.my_index = 0;
    g_match.starting_life = starting_life;
    g_match.player_count = opponents + 1;

    strncpy(g_match.players[0].name, g_player_name, sizeof(g_match.players[0].name) - 1);
    g_match.players[0].life = starting_life;
    g_match.players[0].num_commanders = 1;

    for (int i = 1; i <= opponents; i++) {
        snprintf(g_match.players[i].name, sizeof(g_match.players[i].name), "opp_%d", i);
        g_match.players[i].life = starting_life;
        g_match.players[i].num_commanders = 1;
    }
    g_dirty = true;
}

void jvlt_match_end(void) {
    g_match.active = false;
    g_dirty = true;
}

void jvlt_match_reset_counters(void) {
    if (!g_match.active) return;
    for (int i = 0; i < g_match.player_count; i++) {
        g_match.players[i].life = g_match.starting_life;
        g_match.players[i].poison = 0;
        memset(g_match.players[i].cmd_dmg, 0, sizeof(g_match.players[i].cmd_dmg));
    }
    g_dirty = true;
}

void jvlt_match_update_life(int16_t delta) {
    if (!g_match.active) return;
    g_match.players[g_match.my_index].life += delta;
    g_dirty = true;
}

void jvlt_match_add_cmd_dmg(uint8_t opp, uint8_t commander_idx, int16_t delta) {
    if (!g_match.active) return;
    int16_t *dmg = &g_match.players[g_match.my_index].cmd_dmg[opp][commander_idx];
    if (delta < 0 && *dmg + delta < 0) return;
    *dmg += delta;
    g_match.players[g_match.my_index].life -= delta;
    g_dirty = true;
}

void jvlt_match_add_poison(int16_t delta) {
    if (!g_match.active) return;
    int val = g_match.players[g_match.my_index].poison + delta;
    if (val < 0) val = 0;
    if (val > 10) val = 10;
    g_match.players[g_match.my_index].poison = (uint8_t)val;
    g_dirty = true;
}

void jvlt_match_set_num_commanders(uint8_t opp, uint8_t count) {
    if (!g_match.active) return;
    if (count < 1) count = 1;
    if (count > JVLT_MAX_COMMANDERS) count = JVLT_MAX_COMMANDERS;
    if (count < g_match.players[opp].num_commanders)
        g_match.players[g_match.my_index].cmd_dmg[opp][1] = 0;
    g_match.players[opp].num_commanders = count;
    g_dirty = true;
}

void jvlt_match_nav_opponent(int delta) {
    if (!g_match.active || g_match.player_count <= 1) return;
    uint8_t opp = g_match.cmd_sel_opp;
    do {
        opp = (opp + delta + g_match.player_count) % g_match.player_count;
    } while (opp == g_match.my_index);
    g_match.cmd_sel_opp = opp;
    g_dirty = true;
}

void jvlt_match_select_opponent(uint8_t opp_idx) {
    if (!g_match.active || opp_idx >= g_match.player_count || opp_idx == g_match.my_index) return;
    g_match.cmd_sel_opp = opp_idx;
    g_dirty = true;
}

void jvlt_match_rename_player(uint8_t player_idx, const char *name) {
    if (!g_match.active || player_idx >= g_match.player_count) return;
    strncpy(g_match.players[player_idx].name, name, sizeof(g_match.players[0].name) - 1);
    g_match.players[player_idx].name[sizeof(g_match.players[0].name) - 1] = '\0';
    g_dirty = true;
}

void jvlt_match_set_starting_life(int16_t life) {
    g_match.starting_life = life;
    g_dirty = true;
}

void jvlt_match_cycle_starting_life(int delta) {
    static const int16_t lives[] = {20, 30, 40};
    g_match.starting_life = jvlt_list_cycle(lives, 3, g_match.starting_life, delta);
    g_dirty = true;
}

static const int16_t s_life_options[] = {20, 30, 40};
const int16_t *jvlt_match_peek_starting_life(int8_t dir) {
    return jvlt_list_peek(s_life_options, 3, g_match.starting_life, dir);
}

static const int16_t s_opp_options[] = {1, 2, 3, 4, 5};
const int16_t *jvlt_match_peek_opponents(int8_t dir) {
    int16_t opp = g_match.player_count - 1;
    return jvlt_list_peek(s_opp_options, 5, opp, dir);
}

void jvlt_match_set_opponents(uint8_t count) {
    if (count > JVLT_MAX_PLAYERS - 1) count = JVLT_MAX_PLAYERS - 1;
    g_match.player_count = count + 1;
    for (int i = 1; i <= count; i++) {
        snprintf(g_match.players[i].name, sizeof(g_match.players[i].name), "opp_%d", i);
        g_match.players[i].num_commanders = 1;
    }
    g_dirty = true;
}
