idf_component_register(SRCS "app_main.cpp"
        REQUIRES my_wifi my_gettime nvs_flash freertos
        INCLUDE_DIRS "."
)
