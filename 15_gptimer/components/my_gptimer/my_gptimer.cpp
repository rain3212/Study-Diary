//
// Created by rain on 2026/8/16.
//
#include "driver/gptimer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "my_led.h"
#include "esp_attr.h"

static bool IRAM_ATTR TimerCallback(gptimer_handle_t timer,
                                          const gptimer_alarm_event_data_t *edata,
                                          void *user_ctx)
{

    gpio_set_level(LED_PIN, !gpio_get_level(LED_PIN));

    return false;
}
static gptimer_handle_t gptimer_handle = NULL;
void gptimer_init()
{
    //定时器的基本配置
    gptimer_config_t gptimer_config = {
        .clk_src = GPTIMER_CLK_SRC_DEFAULT,
        .direction = GPTIMER_COUNT_UP,
        .resolution_hz = 1 * 1000 * 1000,
        .intr_priority=0,
        .flags = {
            .intr_shared = 0, // 不与其他外设共享中断
            .allow_pd = 0,
        },

    };

    //创建定时器
    ESP_ERROR_CHECK(gptimer_new_timer(&gptimer_config,&gptimer_handle));
    //报警设置
    gptimer_alarm_config_t alarm_config = {
        .alarm_count = 1000000, //微秒级的
        .reload_count = 0,
        .flags = {
            .auto_reload_on_alarm = true,
        },

    };
    //设置报警动作
     ESP_ERROR_CHECK(gptimer_set_alarm_action(gptimer_handle,&alarm_config));
    gptimer_event_callbacks_t  cbs = {//在这里触发的应该是回调函数
        .on_alarm= TimerCallback,
    };
    //注册定时器时间回调函数，允许用户携带上下文
     ESP_ERROR_CHECK(gptimer_register_event_callbacks(gptimer_handle,&cbs,NULL));
    //使能定时器
     ESP_ERROR_CHECK(gptimer_enable(gptimer_handle));
    //启动定时器
     ESP_ERROR_CHECK(gptimer_start(gptimer_handle));
}
