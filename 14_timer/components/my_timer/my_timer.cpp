//
// Created by rain on 2026/8/11.
//
#include "my_timer.h"
#include "esp_timer.h"
#include "my_led.h"
#include "esp_log.h"
static const char *TAG = "example";
static esp_timer_handle_t timer = NULL;
void esp_timer_callback(void* arg)
{
    LED_TOGGLE();
    ESP_LOGI(TAG, "Timer Callback");
}

//进行初始化
void my_timer_init(uint64_t period_us)
{
    esp_timer_create_args_t timer_itf = {};
    timer_itf.callback = &esp_timer_callback;
    timer_itf.arg = 0;
    timer_itf.name = "my_timer";
    timer_itf.dispatch_method=ESP_TIMER_TASK;
    timer_itf.skip_unhandled_events=true;
    
    //创建一个定时器
    esp_timer_create(&timer_itf, &timer);
    //启用定时器,周期性触发定时器，每5000ms触发一次
    esp_timer_start_periodic(timer, period_us);
}
