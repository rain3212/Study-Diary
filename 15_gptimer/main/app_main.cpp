#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_led.h"
#include "my_gptimer.h"

extern "C" void app_main()
{

        led_init();

        gptimer_init();
    while (1)
    {
        vTaskDelay(10);
    }
}
