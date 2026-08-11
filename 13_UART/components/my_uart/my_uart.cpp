//
// Created by rain on 2026/8/5.
//
#include "my_uart.h"
#include "esp_log.h" //µùÑσ┐ùΦ╛ôσç║
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
QueueHandle_t uart_queue;

void uart_init()
{
    const int uart_buffer_size = (1024 * 2);
    // 配置通信参数，写一下配置文件，
    uart_config_t uart_config = {};
    uart_config.baud_rate = 115200;
    uart_config.data_bits = UART_DATA_8_BITS;
    uart_config.parity = UART_PARITY_DISABLE;
    uart_config.stop_bits = UART_STOP_BITS_1;
    uart_config.rx_flow_ctrl_thresh = 122;
    ESP_ERROR_CHECK(uart_param_config(UART_NUM, &uart_config)); // 应用配置文件
    // 安装驱动
    ESP_ERROR_CHECK(uart_driver_install(UART_NUM, uart_buffer_size,
                                        uart_buffer_size, 10, &uart_queue, 0));
    // 设置通信管脚
    ESP_ERROR_CHECK(
        uart_set_pin(UART_NUM, 16, 17, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
}
