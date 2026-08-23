# my_wifi 组件构建定义
idf_component_register(SRCS "my_wifi.cpp"
        INCLUDE_DIRS "include"
        REQUIRES esp_wifi esp_netif esp_event nvs_flash
        INCLUDE_DIRS ""
)
