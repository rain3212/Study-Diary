#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include <stdio.h>
#include "freertos/task.h"
#include "my_spi.h"
const static char * TAG = "main()";
extern "C" void app_main()
{
    esp_err_t ret =my_spi_init();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "my_spi_init() failed");
        return;
    }
    uint8_t dump[16];
    ret =read_my_reg(MY_SPI_CMD_READ_DATA,0x000001,dump,sizeof(dump));
    if (ret == ESP_OK)
    {
        ESP_LOGE(TAG, "read_my_reg() 成功");
        ESP_LOG_BUFFER_HEX(TAG, dump, sizeof(dump));
    }else
    {
        ESP_LOGI(TAG,"失败");
    }
}
