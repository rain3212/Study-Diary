//
// wifi_module.h — WiFi STA 模块
//
// 职责：
//   1. 管理 WiFi STA 连接生命周期 (连接/断开/自动重连)
//   2. 通过 g_system_event_group 广播连接状态 (SYS_EVT_WIFI_CONNECTED)
//   3. 提供扫描热点、查询状态的 JSON API
//
// 不负责：NTP、MQTT 或任何上层业务。
//

#ifndef WIFI_MODULE_H
#define WIFI_MODULE_H

#include <string>
#include "esp_wifi.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// ═══════════════════════════════════════════════
//  WiFi 状态机
// ═══════════════════════════════════════════════
enum class WifiState {
    IDLE,            // 初始状态，尚未调用 connect
    SCANNING,        // 正在扫描附近热点
    CONNECTING,      // 正在连接 AP
    SWITCHING_AP,    // 正在切换到新 AP (阻止自动重连)
    CONNECTED,       // 已连接，已获得 IP
    DISCONNECTED     // 已断开
};

enum class WifiFailReason {
    NONE,
    WRONG_PASSWORD,
    AP_NOT_FOUND,
    NETWORK_UNSTABLE,
    UNKNOWN_ERROR
};

// ═══════════════════════════════════════════════
//  WiFi 模块 (全静态，系统级唯一)
// ═══════════════════════════════════════════════
class WifiModule {
public:
    /// 初始化 WiFi 协议栈 (STA 模式)，从 NVS 读取配置但不启动
    static void init();

    /// 启动 WiFi (将触发 STA_START 事件)
    static void start();

    /// 运行时切换到新 AP (阻塞等待结果，最多 10 秒)
    static bool connect_to_new_ap(const std::string &ssid, const std::string &password);

    /// 触发一次 WiFi 热点扫描 (阻塞直到扫描完成)
    static void refresh_wifi_list();

    /// 获取上一次扫描结果 (JSON 数组字符串)
    static std::string get_wifi_list();

    /// 获取当前连接状态 (JSON 字符串)
    static std::string get_status_json();

    /// 获取当前状态枚举
    static WifiState get_state() { return current_state; }

private:
    // ── 内部事件组 (仅用于 connect_to_new_ap 阻塞等待) ──
    static EventGroupHandle_t internal_event_group;
    static constexpr int INTERNAL_CONNECTED_BIT = BIT0;
    static constexpr int INTERNAL_FAIL_BIT      = BIT1;

    // ── 状态 ──
    static WifiState     current_state;
    static WifiFailReason last_fail_reason;
    static int           raw_disconnect_reason;

    // ── 扫描结果缓存 ──
    static std::string       cached_wifi_list;
    static SemaphoreHandle_t wifi_list_mutex;
    static bool              has_saved_wifi_config;

    // ── ESP-IDF 事件回调 ──
    static void wifi_event_handler(void *arg, esp_event_base_t event_base,
                                   int32_t event_id, void *event_data);
};

#endif // WIFI_MODULE_H
