idf_component_register(
    SRCS "main.c" "game_task.c" "keyboard.c" "touch_input.c" "display.c" "renderer.c" "ui.c"
    INCLUDE_DIRS "."
    REQUIRES jvlt_core esp_driver_ledc esp_driver_i2c esp_driver_gpio esp_driver_spi esp_lcd nvs_flash
)
