idf_component_register(SRCS "my_gptimer.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_driver_gptimer my_led
        INCLUDE_DIRS "."
)