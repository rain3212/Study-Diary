//
// Created by rain on 2026/8/3.
//
#include "my_exit.h"
#include "my_led.h"
#include "driver/gpio.h"
#include "esp_attr.h"


//把这个函数放在RAN中运行，中断响应更快
static void IRAM_ATTR exit_gpio_isr_handle(void *arg)
{
    LED(1);

}


void exit_init(void)
{
    gpio_config_t gpio_init_structure;
    gpio_init_structure.mode = GPIO_MODE_INPUT;
    gpio_init_structure.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_init_structure.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_init_structure.intr_type = GPIO_INTR_NEGEDGE;
    gpio_init_structure.pin_bit_mask = 1ULL << MY_EXIT_PIN;
    gpio_config(&gpio_init_structure); //写到相关的寄存器

    gpio_install_isr_service(0);
    //注册中断
    gpio_isr_handler_add(MY_EXIT_PIN, exit_gpio_isr_handle, NULL);

}