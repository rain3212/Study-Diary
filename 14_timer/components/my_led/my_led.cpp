
#include "my_led.h"
#include "driver/gpio.h"

void led_init(void)
{
    gpio_config_t gpio_init_structure;
    gpio_init_structure.mode = GPIO_MODE_INPUT_OUTPUT;
    gpio_init_structure.pull_up_en = GPIO_PULLUP_DISABLE;
    gpio_init_structure.pull_down_en = GPIO_PULLDOWN_ENABLE;
    gpio_init_structure.intr_type = GPIO_INTR_DISABLE;
    gpio_init_structure.pin_bit_mask = 1ULL << LED_PIN;
    gpio_config(&gpio_init_structure); //写到相关的寄存器中
    LED(0); //初始默认状态
}