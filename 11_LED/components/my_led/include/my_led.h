//
// Created by rain on 2026/7/31.
//

#ifndef DEMO1_MY_LED_H
#define DEMO1_MY_LED_H
#include "driver/gpio.h"

#define LED_PIN GPIO_NUM_1
#define BUTTON_PIN GPIO_NUM_42

#define LED(X) do{ (X==1)?\
    gpio_set_level(LED_PIN, 1):\
    gpio_set_level(LED_PIN,0);}while(0)

void led_init(void);
void button_init(void);
bool button_state(void);
#endif //DEMO1_MY_LED_H
