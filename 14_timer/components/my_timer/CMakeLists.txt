idf_component_register(SRCS "my_timer.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_timer my_led
        INCLUDE_DIRS "."
)