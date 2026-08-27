idf_component_register(SRCS "wifi_module.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_wifi system_event project_config arduinojson nvs_module
)