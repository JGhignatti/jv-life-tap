#include "jvlt_internal.h"
#include "esp_log.h"
#include "esp_random.h"
#include <string.h>

static const char *TAG = "jvlt_net";

jvlt_net_state_t g_net;

const jvlt_net_state_t *jvlt_net_get(void) { return &g_net; }

void jvlt_net_create_room(void) {
    g_net.room_code = (uint16_t)(esp_random() & 0xFFFF);
    g_net.is_host = true;
    g_net.connected = true;
    g_net.player_count = 1;
    jvlt_comm_get_mac(g_net.players[0].mac);
    strncpy(g_net.players[0].name, g_player_name, sizeof(g_net.players[0].name) - 1);
    g_dirty = true;
    ESP_LOGI(TAG, "Room created: %04X", g_net.room_code);
}

void jvlt_net_scan(void) {
    // Scanning state — beacon responses handled in dispatch
    g_dirty = true;
}

void jvlt_net_start_match(void) {
    if (!g_net.connected) return;
    // Build match from net players
    jvlt_match_begin_solo(g_net.player_count - 1, 40);
    // Override names from net
    for (int i = 0; i < g_net.player_count; i++) {
        strncpy(g_match.players[i].name, g_net.players[i].name, 12);
    }
    // Broadcast start
    jvlt_packet_t pkt = {0};
    pkt.magic = JVLT_MAGIC;
    pkt.room_code = g_net.room_code;
    pkt.msg_type = MSG_START_MATCH;
    jvlt_comm_get_mac(pkt.sender_mac);
    jvlt_comm_broadcast(&pkt);
    jvlt_set_screen(SCREEN_MATCH);
}

static void send_beacon(void) {
    if (!g_net.is_host || !g_net.connected) return;
    jvlt_packet_t pkt = {0};
    pkt.magic = JVLT_MAGIC;
    pkt.room_code = g_net.room_code;
    pkt.msg_type = MSG_BEACON;
    jvlt_comm_get_mac(pkt.sender_mac);
    pkt.payload.beacon.room_code = g_net.room_code;
    pkt.payload.beacon.player_count = g_net.player_count;
    jvlt_comm_broadcast(&pkt);
}

void jvlt_net_dispatch(const jvlt_packet_t *pkt) {
    switch ((jvlt_msg_type_t)pkt->msg_type) {
    case MSG_BEACON:
        // If scanning, auto-join first room found
        if (jvlt_screen() == SCREEN_NET_LOBBY && !g_net.connected) {
            g_net.room_code = pkt->room_code;
            g_net.connected = true;
            g_net.is_host = false;
            // Send join request
            jvlt_packet_t req = {0};
            req.magic = JVLT_MAGIC;
            req.room_code = g_net.room_code;
            req.msg_type = MSG_JOIN_REQ;
            jvlt_comm_get_mac(req.sender_mac);
            strncpy(req.payload.join_req.name, g_player_name, 12);
            jvlt_comm_send(pkt->sender_mac, &req);
        }
        break;
    case MSG_JOIN_REQ:
        if (g_net.is_host && g_net.player_count < JVLT_MAX_PLAYERS) {
            uint8_t idx = g_net.player_count;
            memcpy(g_net.players[idx].mac, pkt->sender_mac, 6);
            memcpy(g_net.players[idx].name, pkt->payload.join_req.name, 12);
            g_net.player_count++;
            // ACK
            jvlt_packet_t ack = {0};
            ack.magic = JVLT_MAGIC;
            ack.room_code = g_net.room_code;
            ack.msg_type = MSG_JOIN_ACK;
            jvlt_comm_get_mac(ack.sender_mac);
            ack.payload.join_ack.accepted = true;
            ack.payload.join_ack.player_index = idx;
            jvlt_comm_send(pkt->sender_mac, &ack);
        }
        break;
    case MSG_JOIN_ACK:
        if (pkt->payload.join_ack.accepted) {
            ESP_LOGI(TAG, "Joined room %04X", g_net.room_code);
        }
        break;
    case MSG_PLAYER_LIST:
        g_net.player_count = pkt->payload.player_list.count;
        memcpy(g_net.players, pkt->payload.player_list.players,
               sizeof(jvlt_net_player_id_t) * g_net.player_count);
        break;
    case MSG_START_MATCH:
        jvlt_match_begin_solo(g_net.player_count - 1, 40);
        for (int i = 0; i < g_net.player_count; i++)
            strncpy(g_match.players[i].name, g_net.players[i].name, 12);
        jvlt_set_screen(SCREEN_MATCH);
        break;
    case MSG_LIFE_UPDATE: {
        // Find player by MAC and update
        for (int i = 0; i < g_match.player_count; i++) {
            if (memcmp(g_net.players[i].mac, pkt->sender_mac, 6) == 0) {
                g_match.players[i].life = pkt->payload.life_update.life;
                break;
            }
        }
        break;
    }
    default: break;
    }
    g_dirty = true;
}

void jvlt_net_beacon_tick(void) { send_beacon(); }

bool jvlt_packet_valid(const jvlt_packet_t *pkt, size_t len) {
    if (len < sizeof(uint16_t) * 2 + 6 + 2) return false;
    return pkt->magic == JVLT_MAGIC;
}
