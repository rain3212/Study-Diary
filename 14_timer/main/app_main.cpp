#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_log.h"
#include "my_led.h"
#include "my_timer.h"

extern "C" void app_main()
{
    led_init();
    my_timer_init(5000000);
    while (true) {
        vTaskDelay(pdMS_TO_TICKS(1000));  // 每秒让出CPU
    }
}
