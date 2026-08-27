//
// app_main.cpp — 系统入口
//

#include "esp_log.h"
#include "driver/gpio.h"
#include "system_event.h"
#include "nvs_module.h"
#include "wifi_module.h"
#include "ntp_module.h"
#include "project_config.h"

static const char *TAG = "MAIN";

/// 初始化两个 LED 的 GPIO 为推挽输出
static void init_leds() {
    gpio_config_t io_conf = {};
    io_conf.mode         = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en   = GPIO_PULLUP_DISABLE;
    io_conf.intr_type    = GPIO_INTR_DISABLE;
    io_conf.pin_bit_mask = (1ULL << PIN_LED_HALF_SEC) | (1ULL << PIN_LED_MINUTE);
    ESP_ERROR_CHECK(gpio_config(&io_conf));

    gpio_set_level(static_cast<gpio_num_t>(PIN_LED_HALF_SEC), 0);
    gpio_set_level(static_cast<gpio_num_t>(PIN_LED_MINUTE), 0);
}

extern "C" void app_main() {
    ESP_LOGI(TAG, "========== System Boot ==========");

    // 1. 全局事件组 (所有模块通信的基础)
    system_event_init();

    // 2. NVS (WiFi 协议栈内部需要)
    ESP_ERROR_CHECK(NVSModule::init_nvs());

    // 3. LED GPIO
    init_leds();

    // 4. WiFi 初始化 (仅初始化协议栈和读取 NVS)
    WifiModule::init();

    // 5. NTP 模块 + LED 回调注册
    auto &ntp = NtpModule::getInstance();

    // LED1 (GPIO 13): 半秒闪烁，和北京时间整秒对齐
    //   整秒前半段 (0~500ms) → 亮
    //   整秒后半段 (500~1000ms) → 灭
    ntp.registerHalfSecondTick([](bool is_on) {
        gpio_set_level(static_cast<gpio_num_t>(PIN_LED_HALF_SEC), is_on ? 1 : 0);
    });

    // LED2 (GPIO 14): 整分钟闪烁一下 (亮 100ms)
    ntp.registerMinuteTick([](int hour, int minute) {
        gpio_set_level(static_cast<gpio_num_t>(PIN_LED_MINUTE), 1);
        vTaskDelay(pdMS_TO_TICKS(100));
        gpio_set_level(static_cast<gpio_num_t>(PIN_LED_MINUTE), 0);
        ESP_LOGI("LED", "Minute tick: %02d:%02d", hour, minute);
        ESP_LOGI("WIFI_STATUS", "WIFI JSON %s", WifiModule::get_status_json().c_str());
    });

    ntp.init();  // 内部创建 3 个 Task，订阅 WiFi 事件

    // 6. 启动 WiFi
    // 此时 NTP 已订阅完毕。启动 WiFi 后触发的任何事件都不会被漏掉 (如果先启动WiFi再启动NTP有可能NTP会错过WiFi的订阅通知)
    WifiModule::start();

    // 扫描附近的Wi-Fi，如果以后给客户图形化界面告诉他你附近有哪些Wi-Fi可以用可以调用这个函数
    // //或者发送给服务器或者其他需要获取Wi-Fi列表的场景
    {
        EventBits_t bits = xEventGroupWaitBits(
            g_system_event_group,
            SYS_EVT_WIFI_CONNECTED,
            pdFALSE,          // 不清除标志位
            pdTRUE,           // 等待的位全部为 1
            pdMS_TO_TICKS(10000));
        if (bits & SYS_EVT_WIFI_CONNECTED) {
            WifiModule::refresh_wifi_list();
            ESP_LOGI("WIFI_STATUS", "Nearby WIFI SSID: %s", WifiModule::get_wifi_list().c_str());
        } else {
            ESP_LOGW("WIFI_STATUS", "WiFi not connected within 10s, skip scan");
        }
    }

    // 7. 测试代码：如果 NVS 没有配过网，就强制配网
    // 第一次连上之后，把下面这行注释掉，下次开机会自动从 NVS 读 (模拟用户配置了一次账号密码)
    WifiModule::connect_to_new_ap("test_wifi_name", "test_wifi_password");

    ESP_LOGI(TAG, "========== Init Complete ==========");
}
