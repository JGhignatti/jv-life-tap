idf_component_register(
    SRCS "jvlt_init.c" "jvlt_nav.c" "jvlt_match.c" "jvlt_settings.c" "jvlt_dice.c" "jvlt_coin.c" "jvlt_net.c" "jvlt_comm.c"
    INCLUDE_DIRS "include"
    REQUIRES esp_wifi nvs_flash esp_event esp_netif esp_timer freertos
)
