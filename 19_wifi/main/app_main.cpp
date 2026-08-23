#include "esp_err.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_gettime.h"
#include "my_wifi.h"
#include "nvs_flash.h"

#include <time.h>

static const char *TAG = "app_main";

extern "C" void app_main()
{
    time_t timer;//相当于记录时间戳
    struct tm *local_time;//一个指针
    char time_str[64];
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND)
    {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    my_wifi_init();
    my_gettime_init();
    // 等待 SNTP 与 NTP 服务器完成时间同步
    ESP_LOGI(TAG, "正在等待 NTP 时间同步...");
    int wait_cnt = 0;
    while (!my_gettime_is_synced())
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
        wait_cnt++;
        if (wait_cnt % 10 == 0)
        {
            ESP_LOGW(TAG, "仍在等待 NTP 同步 (%d s)", wait_cnt);
        }
    }
    ESP_LOGI(TAG, "NTP 时间同步完成");

    // 定时读取并打印当前时间（获取时间存储单元中的时间戳）
    while (1)
    {
        timer = time(NULL);
        //高速localtime时间戳的数据，然后localtime起计算，计算完了返回类型所在地址
        local_time = localtime(&timer);
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", local_time);
        ESP_LOGI(TAG, "当前时间: %s", time_str);
        vTaskDelay(pdMS_TO_TICKS(50000));
    }
}
