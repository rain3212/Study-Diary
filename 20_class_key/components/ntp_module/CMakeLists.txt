idf_component_register(SRCS "ntp_module.cpp"
        INCLUDE_DIRS "include"
        REQUIRES system_event esp_wifi esp_timer
)
