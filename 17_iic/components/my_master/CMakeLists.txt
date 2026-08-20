idf_component_register(SRCS "my_master.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_gpio esp_driver_i2c
        INCLUDE_DIRS "."
)