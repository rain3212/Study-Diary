#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "my_led.h"
#include "my_uart.h"
#include "stdio.h"
#include <string.h>

extern "C" void app_main()
{
    unsigned char data[128];
    uint16_t time = 0;
    size_t len = 0;
    uart_init();
    led_init();
    while (1)
    {
        uart_get_buffered_data_len(UART_NUM_1, (size_t*)&len);
        if (len > 0)
        {
            memset(data, 0, 128);
            ESP_LOGI("Main", "收到一条消息");
            uart_read_bytes(UART_NUM_1, data, len, portMAX_DELAY);
            uart_write_bytes(UART_NUM_1, data, len);
        }
        else
        {
            time++;
            if (time % 5000 == 0)
            {
                printf("请输入数据：");
            }
        }
        vTaskDelay(pdMS_TO_TICKS(100)); // 这是0.1秒 不要写错了姐
    }
}
