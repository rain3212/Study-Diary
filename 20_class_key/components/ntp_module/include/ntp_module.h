//
// ntp_module.h — NTP 时间同步模块
//
// 职责：
//   1. 订阅 ESP-IDF 平台事件 (IP_EVENT / WIFI_EVENT) → 管理 SNTP 生命周期
//   2. NTP 同步成功后对齐 esp_timer 到北京时间整秒边界
//   3. 通过 500ms 硬件定时器驱动两个 vTask：半秒回调 + 分钟回调
//   4. 通过 g_system_event_group 广播 SYS_EVT_NTP_SYNCED
//
// 不负责：GPIO 控制 (由 main 通过回调注入)
//

#ifndef NTP_MODULE_H
#define NTP_MODULE_H

#include <string>
#include <functional>
#include <sys/time.h>
#include "esp_timer.h"
#include "esp_event.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

class NtpModule {
public:
    // 回调类型定义
    using HalfSecondCallback = std::function<void(bool is_on)>;
    using MinuteCallback     = std::function<void(int hour, int minute)>;

    // 单例
    static NtpModule &getInstance() {
        static NtpModule instance;
        return instance;
    }

    NtpModule(const NtpModule &) = delete;
    NtpModule &operator=(const NtpModule &) = delete;

    /// 初始化 NTP 模块：设置时区、订阅 WiFi 事件、创建定时器和任务
    void init(const char *timezone = "CST-8");

    /// 注册半秒回调 (用于 LED 半秒闪烁，传入 true=亮 false=灭)
    void registerHalfSecondTick(HalfSecondCallback callback);

    /// 注册分钟回调 (用于整分钟闪烁，传入当前时/分)
    void registerMinuteTick(MinuteCallback callback);

    /// 查询 NTP 是否已同步
    [[nodiscard]] bool isNtpSynced() const;

    /// 获取最后一次同步时间的可读字符串
    [[nodiscard]] std::string getLastSyncTimeStr() const;

private:
    NtpModule() = default;
    ~NtpModule() = default;

    // ── TaskNotification 位定义 (daemon task 内部使用) ──
    static constexpr uint32_t NOTIFY_WIFI_CONNECTED     = BIT0;
    static constexpr uint32_t NOTIFY_WIFI_DISCONNECTED  = BIT1;

    // ── 用户回调 ──
    HalfSecondCallback onHalfSecondTick = nullptr;
    MinuteCallback     onMinuteTick     = nullptr;

    // ── esp_timer 句柄 ──
    esp_timer_handle_t periodic_timer = nullptr;   // 500ms 周期定时器
    esp_timer_handle_t align_timer    = nullptr;   // 一次性对齐定时器

    // ── FreeRTOS 任务句柄 ──
    TaskHandle_t daemon_task_handle   = nullptr;
    TaskHandle_t half_sec_task_handle = nullptr;
    TaskHandle_t minute_task_handle   = nullptr;

    // ── 状态 ──
    int    last_minute    = -1;
    time_t last_sync_time = 0;
    volatile bool timer_running = false;

    // ── ESP-IDF 平台事件回调 (订阅 WiFi/IP 事件) ──
    static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);

    // ── 任务入口函数 ──
    static void ntp_daemon_task(void *arg);     // 等 TaskNotify → 管理 SNTP 生命周期
    static void half_second_task(void *arg);    // 等定时器通知 → 调半秒回调
    static void minute_task(void *arg);         // 等定时器通知 → 检测整分钟

    // ── 定时器回调 ──
    static void timer_callback(void *arg);      // 500ms 周期回调 → 通知两个 Task
    static void align_callback(void *arg);      // 对齐完成 → 启动周期定时器

    // ── SNTP 同步回调 ──
    static void time_sync_notification_cb(timeval *tv);
};

#endif // NTP_MODULE_H
