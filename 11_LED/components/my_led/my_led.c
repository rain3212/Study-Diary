//
// Created by rain on 2026/7/31.
//

#include "include/my_led.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "hal/gpio_types.h"

void led_init(void)
{
    gpio_config_t gpio_init_structure;
    gpio_init_structure.mode = GPIO_MODE_OUTPUT;
    gpio_init_structure.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_init_structure.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_init_structure.intr_type = GPIO_INTR_DISABLE;
    gpio_init_structure.pin_bit_mask = 1ULL << LED_PIN;
    gpio_config(&gpio_init_structure); //写到相关的寄存器中
    LED(0); //初始默认状态
}

void button_init(void)
{
    gpio_config_t button_init_structure;
    button_init_structure.mode = GPIO_MODE_INPUT;
    button_init_structure.pin_bit_mask = 1ULL << BUTTON_PIN;
    button_init_structure.pull_up_en = GPIO_PULLUP_ENABLE;
    button_init_structure.pull_down_en = GPIO_PULLDOWN_DISABLE;
    button_init_structure.intr_type = GPIO_INTR_DISABLE;
    gpio_config(&button_init_structure);

}

bool button_state(void)
{

    return (gpio_get_level(BUTTON_PIN) == 1);
}
