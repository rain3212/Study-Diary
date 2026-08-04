#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_led.h"
#include "my_exit.h"
extern "C" void app_main()
{
    led_init();
    exit_init();
    while (1)
    {

        vTaskDelay(500);
        LED(0);

    }

}
