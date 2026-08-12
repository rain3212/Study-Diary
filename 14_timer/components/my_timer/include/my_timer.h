//
// Created by rain on 2026/8/11.
//

#ifndef DEMO1_MY_TIMER_H
#define DEMO1_MY_TIMER_H
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "my_led.h"
void my_timer_init(uint64_t period_us);
void esp_timer_callback(void* arg);
#endif //DEMO1_MY_TIMER_H
