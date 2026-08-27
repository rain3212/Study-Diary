//
// wifi_module.cpp — WiFi STA 模块实现
//

#include "wifi_module.h"
#include "system_event.h"
#include "project_config.h"
#include "nvs_module.h"
#include "esp_log.h"
#include <ArduinoJson.h>
#include <cstring>

static const char *TAG = "WiFi";

// ═══════════════════════════════════════════════
//  静态成员变量定义
// ═══════════════════════════════════════════════
EventGroupHandle_t  WifiModule::internal_event_group    = nullptr;
WifiState           WifiModule::current_state           = WifiState::IDLE;
WifiFailReason      WifiModule::last_fail_reason        = WifiFailReason::NONE;
int                 WifiModule::raw_disconnect_reason   = 0;
std::string         WifiModule::cached_wifi_list        = "[]";
SemaphoreHandle_t   WifiModule::wifi_list_mutex         = nullptr;
bool                WifiModule::has_saved_wifi_config   = false;

// ═══════════════════════════════════════════════
//  ESP-IDF 事件回调
// ═══════════════════════════════════════════════
void WifiModule::wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data) {
    // ── STA 启动 → 发起连接 ──
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        if (has_saved_wifi_config) {
            current_state = WifiState::CONNECTING;
            esp_wifi_connect();
            ESP_LOGI(TAG, "STA started, connecting to saved AP...");
        } else {
            ESP_LOGI(TAG, "STA started, waiting for manual connection (no saved AP).");
        }
        return;
    }

    // ── 获得 IP → 连接成功 ──
    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        auto *event = static_cast<ip_event_got_ip_t *>(event_data);
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));

        current_state    = WifiState::CONNECTED;
        last_fail_reason = WifiFailReason::NONE;

        // ① 全局广播：通知所有订阅者 (NTP 等模块)
        xEventGroupSetBits(g_system_event_group, SYS_EVT_WIFI_CONNECTED);

        // ② 内部通知：唤醒 connect_to_new_ap() 的阻塞等待
        xEventGroupSetBits(internal_event_group, INTERNAL_CONNECTED_BIT);
        return;
    }

    // ── 断开连接 ──
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        auto *event = static_cast<wifi_event_sta_disconnected_t *>(event_data);
        raw_disconnect_reason = event->reason;

        // 翻译底层错误码为前端可读原因
        switch (event->reason) {
            case WIFI_REASON_NO_AP_FOUND:
                last_fail_reason = WifiFailReason::AP_NOT_FOUND;
                break;
            case WIFI_REASON_AUTH_FAIL:
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
            case WIFI_REASON_MIC_FAILURE:
                last_fail_reason = WifiFailReason::WRONG_PASSWORD;
                break;
            case WIFI_REASON_BEACON_TIMEOUT:
                last_fail_reason = WifiFailReason::NETWORK_UNSTABLE;
                break;
            default:
                last_fail_reason = WifiFailReason::UNKNOWN_ERROR;
                break;
        }

        ESP_LOGW(TAG, "Disconnected, reason: %d", event->reason);

        // 全局广播：WiFi 已断开
        xEventGroupClearBits(g_system_event_group, SYS_EVT_WIFI_CONNECTED);

        // 状态机决策
        if (current_state == WifiState::SWITCHING_AP) {
            // 手动切换 AP 时，不自动重连
            current_state = WifiState::DISCONNECTED;

        } else if (event->reason == WIFI_REASON_AUTH_FAIL ||
                   event->reason == WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT ||
                   event->reason == WIFI_REASON_MIC_FAILURE) {
            // 密码错误 → 永久性失败，停止重连
            current_state = WifiState::DISCONNECTED;
            xEventGroupSetBits(internal_event_group, INTERNAL_FAIL_BIT);

        } else {
            // 暂时性故障 (路由器重启/信号丢失) → 自动重连
            current_state = WifiState::CONNECTING;
            ESP_LOGI(TAG, "Auto-reconnecting...");
            esp_wifi_connect();
        }
    }
}

// ═══════════════════════════════════════════════
//  初始化
// ═══════════════════════════════════════════════
void WifiModule::init() {
    internal_event_group = xEventGroupCreate();
    wifi_list_mutex      = xSemaphoreCreateMutex();

    // 网络协议栈
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    // WiFi 驱动
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    // 注册事件回调
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, nullptr, nullptr));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, nullptr, nullptr));

    // STA 模式
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));

    // 尝试从 NVS 读取配置
    wifi_config_t sta_config = {};
    if (NVSModule::load(NVS_NS_WIFI, NVS_KEY_WIFI, sta_config) == ESP_OK) {
        ESP_LOGI(TAG, "Found saved WiFi config for: %s", sta_config.sta.ssid);
        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &sta_config));
        has_saved_wifi_config = true;
    } else {
        ESP_LOGW(TAG, "No saved WiFi config found");
        has_saved_wifi_config = false;
    }

    ESP_LOGI(TAG, "WiFi initialized, waiting to start.");
}

void WifiModule::start() {
    ESP_LOGI(TAG, "Starting WiFi STA...");
    ESP_ERROR_CHECK(esp_wifi_start());
}

// ═══════════════════════════════════════════════
//  运行时切换 AP (阻塞等待)
// ═══════════════════════════════════════════════
bool WifiModule::connect_to_new_ap(const std::string &ssid, const std::string &password) {
    ESP_LOGI(TAG, "Switching AP → %s", ssid.c_str());

    // 进入切换状态，阻止事件处理器中的自动重连
    current_state = WifiState::SWITCHING_AP;
    esp_wifi_disconnect();
    vTaskDelay(pdMS_TO_TICKS(200));

    // 准备新凭证
    wifi_config_t wifi_config = {};
    strncpy(reinterpret_cast<char *>(wifi_config.sta.ssid),
            ssid.c_str(), sizeof(wifi_config.sta.ssid));
    strncpy(reinterpret_cast<char *>(wifi_config.sta.password),
            password.c_str(), sizeof(wifi_config.sta.password));

    if (esp_wifi_set_config(WIFI_IF_STA, &wifi_config) != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set WiFi config");
        current_state = WifiState::DISCONNECTED;
        return false;
    }

    // 清除旧标志位，发起连接
    current_state = WifiState::CONNECTING;
    xEventGroupClearBits(internal_event_group, INTERNAL_CONNECTED_BIT | INTERNAL_FAIL_BIT);
    esp_wifi_connect();

    // 阻塞等待结果 (最多 10 秒)
    EventBits_t bits = xEventGroupWaitBits(
        internal_event_group,
        INTERNAL_CONNECTED_BIT | INTERNAL_FAIL_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(10000));

    if (bits & INTERNAL_CONNECTED_BIT) {
        ESP_LOGI(TAG, "Switched to AP: %s", ssid.c_str());
        // 成功连接，保存到 NVS
        NVSModule::save(NVS_NS_WIFI, NVS_KEY_WIFI, wifi_config);
        has_saved_wifi_config = true;
        return true;
    }

    ESP_LOGE(TAG, "Failed to connect to: %s", ssid.c_str());
    return false;
}

// ═══════════════════════════════════════════════
//  WiFi 扫描
// ═══════════════════════════════════════════════
void WifiModule::refresh_wifi_list() {
    WifiState prev_state = current_state;
    current_state = WifiState::SCANNING;

    uint16_t number = WIFI_SCAN_MAX;
    wifi_ap_record_t ap_info[WIFI_SCAN_MAX];

    esp_wifi_scan_start(nullptr, true);
    esp_wifi_scan_get_ap_records(&number, ap_info);

    JsonDocument doc;
    doc.to<JsonArray>();

    for (int i = 0; i < number; i++) {
        auto item = doc.add<JsonObject>();
        item["ssid"] = reinterpret_cast<const char *>(ap_info[i].ssid);
        item["rssi"] = ap_info[i].rssi;
        item["auth"] = ap_info[i].authmode;
    }

    std::string temp;
    serializeJson(doc, temp);

    xSemaphoreTake(wifi_list_mutex, portMAX_DELAY);
    cached_wifi_list = std::move(temp);
    xSemaphoreGive(wifi_list_mutex);

    current_state = prev_state;
    ESP_LOGI(TAG, "Scan complete, found %d APs", number);
}

std::string WifiModule::get_wifi_list() {
    xSemaphoreTake(wifi_list_mutex, portMAX_DELAY);
    std::string result = cached_wifi_list;
    xSemaphoreGive(wifi_list_mutex);
    return result;
}

// ═══════════════════════════════════════════════
//  状态查询 (JSON)
// ═══════════════════════════════════════════════
std::string WifiModule::get_status_json() {
    JsonDocument doc;

    switch (current_state) {
        case WifiState::CONNECTING:   doc["state"] = "CONNECTING";   break;
        case WifiState::CONNECTED:    doc["state"] = "CONNECTED";    break;
        case WifiState::DISCONNECTED: doc["state"] = "DISCONNECTED"; break;
        case WifiState::SCANNING:     doc["state"] = "SCANNING";     break;
        case WifiState::SWITCHING_AP: doc["state"] = "SWITCHING";    break;
        default:                      doc["state"] = "IDLE";         break;
    }

    switch (last_fail_reason) {
        case WifiFailReason::WRONG_PASSWORD:  doc["error"] = "WRONG_PASSWORD";  break;
        case WifiFailReason::AP_NOT_FOUND:    doc["error"] = "AP_NOT_FOUND";    break;
        case WifiFailReason::NETWORK_UNSTABLE:doc["error"] = "NETWORK_UNSTABLE";break;
        case WifiFailReason::UNKNOWN_ERROR:   doc["error"] = "UNKNOWN";         break;
        default:                              doc["error"] = "NONE";            break;
    }

    wifi_config_t conf;
    esp_wifi_get_config(WIFI_IF_STA, &conf);
    doc["current_ssid"] = reinterpret_cast<char *>(conf.sta.ssid);

    std::string output;
    serializeJson(doc, output);
    return output;
}
