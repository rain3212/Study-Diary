idf_component_register(SRCS "my_ledc.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_driver_uart esp_driver_ledc
        INCLUDE_DIRS "."
)
