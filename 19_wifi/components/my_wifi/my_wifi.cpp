//
// Created by rain on 2026/8/23.
//
#include <string.h>

#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_wifi.h"

static const char *TAG = "my_wifi";

// 监听到 WiFi 事件时会触发的事件函数
void my_wifi_event_handler(void *event_handler_arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    if (event_base == WIFI_EVENT)
    {
        if (event_id == WIFI_EVENT_STA_START)
        {
            esp_wifi_connect();
        }
        else if (event_id == WIFI_EVENT_STA_CONNECTED)
        {
            ESP_LOGI(TAG, "Connected to the Wi-Fi");
        }
        else if (event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            ESP_LOGW(TAG, "Wi-Fi 断开，准备重新连接");
            // 稍作延时再重连，避免 AP 不可用时一直疯狂触发事件
            vTaskDelay(pdMS_TO_TICKS(3000));
            esp_wifi_connect();
        }
    }
    else if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
            ESP_LOGI(TAG, "got ip:", IPSTR, IP2STR(&event->ip_info.ip));
        }
    }
}

void my_wifi_init(void)
{
    // 初始化 LWIP 协议栈
    ESP_ERROR_CHECK(esp_netif_init());
    // 创建默认事件循环
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    // 注册事件监听：WIFI_EVENT 的所有事件 + IP_EVENT 的所有事件
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, my_wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, ESP_EVENT_ANY_ID, my_wifi_event_handler, NULL));
    // 将 STA 模块与 LWIP 协议相连接
    esp_netif_create_default_wifi_sta();
    // 初始化 WiFi 底层硬件
    wifi_init_config_t wifi_init_config = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_init_config));
    // 设置 WiFi 的工作模式为 STA
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    // 配置 STA 模式参数
    wifi_config_t wifi_config = {};
    strlcpy((char *)wifi_config.sta.ssid, WIFI_SSID, sizeof(wifi_config.sta.ssid));
    strlcpy((char *)wifi_config.sta.password, WIFI_PASSWORD, sizeof(wifi_config.sta.password));
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    // 启动 WiFi 模块
    ESP_ERROR_CHECK(esp_wifi_start());
}
