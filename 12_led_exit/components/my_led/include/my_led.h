//
// Created by rain on 2026/8/3.
//

#ifndef MY_LED_H
#define MY_LED_H

#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_1

#define LED(X) do{ (X==1)?\
    gpio_set_level(LED_PIN, 1):\
    gpio_set_level(LED_PIN,0);}while(0)

void led_init(void);

#endif
