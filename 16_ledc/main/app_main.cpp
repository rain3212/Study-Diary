#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_ledc.h"
extern "C" void app_main()
{
    my_ledc_init();
    while (1)
    {
        breathe_led();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }


}
