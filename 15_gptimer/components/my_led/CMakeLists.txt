idf_component_register(SRCS "my_led.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_driver_uart
        INCLUDE_DIRS "."
)
