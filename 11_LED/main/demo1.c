#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_led.h"

void app_main(void)
{
    led_init();
    button_init();
    while (1) {
        // 按住按键 → LED 亮，松开 → LED 灭
        if (button_state()) {
            LED(1);
        } else {
            LED(0);
        }

        vTaskDelay(pdMS_TO_TICKS(10));  // 每 10ms 检测一次
    }
}
