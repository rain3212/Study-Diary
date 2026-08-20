#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/i2c_master.h"
#include "driver/i2c_slave.h"
#include "my_master.h"

static const char* TAG = "example";

extern "C" void app_main()
{
    uint8_t chip_id = 0;
    my_master_init();

    i2c_scan();
    ESP_LOGI(TAG, "I2C INIT");
    //读取数据
    esp_err_t ret = read_BME280_data(&chip_id);
    if (ret == ESP_OK)
    {
        ESP_LOGI(TAG, "BME280 DATA，0x%02X", chip_id);
    }
    while (1)
    {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}
