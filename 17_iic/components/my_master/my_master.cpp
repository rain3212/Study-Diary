//
// Created by rain on 2026/8/20.
//

#include "driver/i2c_master.h"
#include "esp_err.h"
#include "driver/gpio.h"
#include "my_master.h"
#include "esp_log.h"
#define SDA_PIN GPIO_NUM_20
#define SCL_PIN GPIO_NUM_21

i2c_master_bus_handle_t i2c_master_bus;
i2c_master_dev_handle_t i2c_device;

void my_master_init()
{
    //主机总线,然后esp32作为主机掌控总线
    i2c_master_bus_config_t i2c_mst_config = {
        .i2c_port = I2C_NUM_1,
        .sda_io_num = SDA_PIN,
        .scl_io_num = SCL_PIN,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .intr_priority = 0,
        .trans_queue_depth = 0,
        .flags = {
            .enable_internal_pullup = 1,
            .allow_pd = 0,
        }
    };

    //分配初始化主机总线
    ESP_ERROR_CHECK(
        i2c_new_master_bus(&i2c_mst_config, &i2c_master_bus)
    );
    //从
    i2c_device_config_t i2c_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = 0x76, //要通信的从机的设备地址
        .scl_speed_hz = 100000,
        .scl_wait_us = 0,
        .flags = {
            .disable_ack_check = 0,
        }

    };
    ESP_ERROR_CHECK(
        i2c_master_bus_add_device(i2c_master_bus, &i2c_config, &i2c_device)
    );
    //配置完毕，可以读了
}

esp_err_t read_BME280_data(uint8_t* id)
{
    uint8_t reg = 0xD0; //id寄存器的地址
    //可以开始读了
    esp_err_t ret = i2c_master_transmit_receive(i2c_device, &reg, 1, id, 1, -1);
    //esp_err_t这种要有返回值
    return ret;
}

//扫描一下
void i2c_scan(void)
{
    ESP_LOGI("SCAN", "开始扫描 I2C 总线...");

    for (uint8_t addr = 0x08; addr <= 0x77; addr++)
    {
        //这个函数不支持异步模式，所以trans_queue_depth不能设置成10
        esp_err_t ret = i2c_master_probe(i2c_master_bus, addr, -1);
        if (ret == ESP_OK)
        {
            ESP_LOGI("SCAN", "✅ 发现设备: 0x%02X", addr);
        }
    }
    ESP_LOGI("SCAN", "扫描完成");
}
