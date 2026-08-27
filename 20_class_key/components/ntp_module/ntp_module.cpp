//
// ntp_module.cpp — NTP 时间同步模块实现
//
// 采用 esp_event 订阅模型：
//   NTP 自己注册 IP_EVENT / WIFI_EVENT 的处理函数，
//   通过 xTaskNotify 即时通知 daemon task，零轮询。
//

#include "ntp_module.h"
#include "system_event.h"
#include "esp_log.h"
#include "esp_sntp.h"
#include "esp_wifi.h"
#include <sys/time.h>

static const char *TAG = "NTP";

// ═══════════════════════════════════════════════════
//  初始化
// ═══════════════════════════════════════════════════
void NtpModule::init(const char *timezone) {
    // 1. 设置时区
    setenv("TZ", timezone, 1);
    tzset();
    ESP_LOGI(TAG, "Timezone: %s", timezone);

    // 2. 预配置 SNTP (不启动，由 daemon task 在收到 WiFi 连接通知后启动)
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    esp_sntp_setservername(0, "ntp.aliyun.com");
    esp_sntp_setservername(1, "pool.ntp.org");
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);

    // 3. 创建 esp_timer
    const esp_timer_create_args_t periodic_args = {
        .callback = timer_callback,
        .arg      = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name     = "ntp_500ms",
        .skip_unhandled_events = false
    };
    ESP_ERROR_CHECK(esp_timer_create(&periodic_args, &periodic_timer));

    const esp_timer_create_args_t align_args = {
        .callback = align_callback,
        .arg      = this,
        .dispatch_method = ESP_TIMER_TASK,
        .name     = "ntp_align",
        .skip_unhandled_events = false
    };
    ESP_ERROR_CHECK(esp_timer_create(&align_args, &align_timer));

    // 4. 创建 FreeRTOS 任务 (优先级：half_sec > daemon > minute)
    xTaskCreate(half_second_task, "ntp_halfsec", 2048, this, 6, &half_sec_task_handle);
    xTaskCreate(minute_task,      "ntp_minute",  2048, this, 4, &minute_task_handle);
    xTaskCreate(ntp_daemon_task,  "ntp_daemon",  4096, this, 5, &daemon_task_handle);

    // 5. 订阅 ESP-IDF 平台事件 (WiFi 模块完全不知道 NTP 的存在)
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP,
        &wifi_event_handler, this, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED,
        &wifi_event_handler, this, nullptr));

    ESP_LOGI(TAG, "NTP module initialized, subscribed to WiFi events");
}

// ═══════════════════════════════════════════════════
//  回调注册
// ═══════════════════════════════════════════════════
void NtpModule::registerHalfSecondTick(HalfSecondCallback callback) {
    onHalfSecondTick = std::move(callback);
}

void NtpModule::registerMinuteTick(MinuteCallback callback) {
    onMinuteTick = std::move(callback);
}

// ═══════════════════════════════════════════════════
//  状态查询
// ═══════════════════════════════════════════════════
bool NtpModule::isNtpSynced() const {
    return (xEventGroupGetBits(g_system_event_group) & SYS_EVT_NTP_SYNCED) != 0;
}

std::string NtpModule::getLastSyncTimeStr() const {
    if (last_sync_time == 0) return "Not synced yet";

    tm timeinfo{};
    localtime_r(&last_sync_time, &timeinfo);
    char buf[64];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &timeinfo);
    return buf;
}

// ═══════════════════════════════════════════════════
//  ESP-IDF 平台事件回调 → 通知 daemon task
//  (运行在 esp_event 默认循环的任务上下文中)
// ═══════════════════════════════════════════════════
void NtpModule::wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data) {
    auto *self = static_cast<NtpModule *>(arg);

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        xTaskNotify(self->daemon_task_handle, NOTIFY_WIFI_CONNECTED, eSetBits);
    }

    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        xTaskNotify(self->daemon_task_handle, NOTIFY_WIFI_DISCONNECTED, eSetBits);
    }
}

// ═══════════════════════════════════════════════════
//  SNTP 同步成功回调 (由 lwIP 内部任务调用)
// ═══════════════════════════════════════════════════
void NtpModule::time_sync_notification_cb(timeval *tv) {
    NtpModule &self = NtpModule::getInstance();
    self.last_sync_time = tv->tv_sec;

    // 停止当前定时器
    if (self.timer_running) {
        esp_timer_stop(self.periodic_timer);
        self.timer_running = false;
    }
    esp_timer_stop(self.align_timer);

    // 计算到下一个整秒边界的微秒数
    int64_t usec_to_next = 1000000 - tv->tv_usec;
    if (usec_to_next < 10000) {
        usec_to_next += 1000000;  // 不足 10ms 则等到下一个整秒
    }

    // 启动对齐定时器 → 到达整秒后启动 500ms 周期定时器
    esp_timer_start_once(self.align_timer, usec_to_next);

    // 设置全局标志位
    xEventGroupSetBits(g_system_event_group, SYS_EVT_NTP_SYNCED);

    ESP_LOGI(TAG, "NTP synced! Aligning timer, offset: %lld us", usec_to_next);
}

// ═══════════════════════════════════════════════════
//  对齐定时器回调 → 启动 500ms 周期定时器
// ═══════════════════════════════════════════════════
void NtpModule::align_callback(void *arg) {
    auto *self = static_cast<NtpModule *>(arg);

    esp_timer_start_periodic(self->periodic_timer, 500000);
    self->timer_running = true;

    // 立即通知一次 (这是整秒的第一拍)
    if (self->half_sec_task_handle) {
        xTaskNotifyGive(self->half_sec_task_handle);
    }
    if (self->minute_task_handle) {
        xTaskNotifyGive(self->minute_task_handle);
    }

    ESP_LOGI(TAG, "Timer aligned, 500ms periodic started");
}

// ═══════════════════════════════════════════════════
//  500ms 周期定时器回调 → 通知两个 Task
// ═══════════════════════════════════════════════════
void NtpModule::timer_callback(void *arg) {
    auto *self = static_cast<NtpModule *>(arg);

    if (self->half_sec_task_handle) {
        xTaskNotifyGive(self->half_sec_task_handle);
    }
    if (self->minute_task_handle) {
        xTaskNotifyGive(self->minute_task_handle);
    }
}

// ═══════════════════════════════════════════════════
//  NTP 守护任务 — 纯事件驱动，零轮询
//
//  通过 xTaskNotifyWait 阻塞等待 WiFi 事件通知：
//    NOTIFY_WIFI_CONNECTED    → 启动 SNTP
//    NOTIFY_WIFI_DISCONNECTED → 停止 SNTP + 定时器
// ═══════════════════════════════════════════════════
void NtpModule::ntp_daemon_task(void *arg) {
    auto *self = static_cast<NtpModule *>(arg);

    ESP_LOGI(TAG, "Daemon: waiting for WiFi events...");

    while (true) {
        uint32_t notify_bits = 0;

        // 阻塞等待：WiFi 连接或断开通知 并 清空BITS
        xTaskNotifyWait(0, ULONG_MAX, &notify_bits, portMAX_DELAY);

        // ── 先处理断开 (若同时收到连接+断开，先停后启) ──
        if (notify_bits & NOTIFY_WIFI_DISCONNECTED) {
            ESP_LOGW(TAG, "Daemon: WiFi lost → stopping SNTP");

            if (esp_sntp_enabled()) {
                esp_sntp_stop();
            }

            if (self->timer_running) {
                esp_timer_stop(self->periodic_timer);
                self->timer_running = false;
            }
            esp_timer_stop(self->align_timer);

            xEventGroupClearBits(g_system_event_group, SYS_EVT_NTP_SYNCED);
            self->last_minute = -1;
        }

        // ── 再处理连接 ──
        if (notify_bits & NOTIFY_WIFI_CONNECTED) {
            ESP_LOGI(TAG, "Daemon: WiFi connected → starting SNTP");

            if (esp_sntp_enabled()) {
                esp_sntp_stop();  // 防止重复初始化
            }
            esp_sntp_init();
        }
    }
}

// ═══════════════════════════════════════════════════
//  半秒闪烁 Task — 等定时器通知，读时间判断亮/灭
// ═══════════════════════════════════════════════════
void NtpModule::half_second_task(void *arg) {
    auto *self = static_cast<NtpModule *>(arg);

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        EventBits_t bits = xEventGroupGetBits(g_system_event_group);
        if (!(bits & SYS_EVT_NTP_SYNCED)) {
            if (self->onHalfSecondTick) {
                self->onHalfSecondTick(false);
            }
            continue;
        }

        // 读取墙钟时间，前半秒亮、后半秒灭
        timeval tv;
        gettimeofday(&tv, nullptr);
        bool is_on = (tv.tv_usec < 500000);

        if (self->onHalfSecondTick) {
            self->onHalfSecondTick(is_on);
        }
    }
}

// ═══════════════════════════════════════════════════
//  整分钟 Task — 等定时器通知，检测分钟变化
// ═══════════════════════════════════════════════════
void NtpModule::minute_task(void *arg) {
    auto *self = static_cast<NtpModule *>(arg);

    while (true) {
        ulTaskNotifyTake(pdTRUE, portMAX_DELAY);

        EventBits_t bits = xEventGroupGetBits(g_system_event_group);
        if (!(bits & SYS_EVT_NTP_SYNCED)) {
            continue;
        }

        timeval tv;
        gettimeofday(&tv, nullptr);
        tm timeinfo;
        localtime_r(&tv.tv_sec, &timeinfo);

        if (timeinfo.tm_min != self->last_minute) {
            self->last_minute = timeinfo.tm_min;
            if (self->onMinuteTick) {
                self->onMinuteTick(timeinfo.tm_hour, timeinfo.tm_min);
            }
        }
    }
}
