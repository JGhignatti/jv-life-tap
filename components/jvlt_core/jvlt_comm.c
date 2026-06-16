#include "jvlt_internal.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "esp_mac.h"
#include "nvs_flash.h"
#include <string.h>

static const char *TAG = "jvlt_comm";
static uint8_t s_mac[6];
static const uint8_t BROADCAST[6] = {0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};

static void on_recv(const esp_now_recv_info_t *info, const uint8_t *data, int len) {
    if (len < (int)sizeof(jvlt_packet_t)) return;
    const jvlt_packet_t *pkt = (const jvlt_packet_t *)data;
    if (!jvlt_packet_valid(pkt, len)) return;
    xQueueSendFromISR(g_pkt_queue, pkt, NULL);
}

static void on_send(const esp_now_send_info_t *info, esp_now_send_status_t status) {
    if (status != ESP_NOW_SEND_SUCCESS)
        ESP_LOGW(TAG, "send failed to " MACSTR, MAC2STR(info->des_addr));
}

void jvlt_comm_init(void) {
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        nvs_flash_init();
    }

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
    ESP_ERROR_CHECK(esp_wifi_set_channel(1, WIFI_SECOND_CHAN_NONE));

    esp_read_mac(s_mac, ESP_MAC_WIFI_STA);

    ESP_ERROR_CHECK(esp_now_init());
    ESP_ERROR_CHECK(esp_now_register_recv_cb(on_recv));
    ESP_ERROR_CHECK(esp_now_register_send_cb(on_send));

    esp_now_peer_info_t peer = {0};
    memcpy(peer.peer_addr, BROADCAST, 6);
    peer.channel = 1;
    peer.encrypt = false;
    ESP_ERROR_CHECK(esp_now_add_peer(&peer));

    ESP_LOGI(TAG, "ESP-NOW ready, MAC=" MACSTR, MAC2STR(s_mac));
}

void jvlt_comm_broadcast(const jvlt_packet_t *pkt) {
    esp_now_send(BROADCAST, (const uint8_t *)pkt, sizeof(jvlt_packet_t));
}

void jvlt_comm_send(const uint8_t *dest_mac, const jvlt_packet_t *pkt) {
    if (!esp_now_is_peer_exist(dest_mac)) {
        esp_now_peer_info_t peer = {0};
        memcpy(peer.peer_addr, dest_mac, 6);
        peer.channel = 1;
        peer.encrypt = false;
        esp_now_add_peer(&peer);
    }
    esp_now_send(dest_mac, (const uint8_t *)pkt, sizeof(jvlt_packet_t));
}

void jvlt_comm_get_mac(uint8_t *out) {
    memcpy(out, s_mac, 6);
}
