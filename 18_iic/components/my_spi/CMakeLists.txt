idf_component_register(
        SRCS "my_spi.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_driver_spi
        INCLUDE_DIRS "."
)