#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

auto TAG = "Main";

extern "C" void app_main() {
    while (true) {
        ESP_LOGI(TAG, "Hello SMALL WANGWANG");
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
