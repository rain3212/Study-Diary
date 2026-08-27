//
// project_config.h — 全局配置
//
// 所有跨模块共用的系统级常量集中在此处定义。
// 各 component 的 CMakeLists.txt 中 REQUIRES project_config 即可使用。
//
// ⚠ 此文件不得 #include 任何 ESP-IDF 或第三方头文件，
//   以保证它可以被任意层级的 component 无副作用地引入。
//

#ifndef PROJECT_CONFIG_H
#define PROJECT_CONFIG_H

// ═══════════════ WiFi ═══════════════
#define WIFI_SCAN_MAX    10

// ═══════════════ NVS Keys ═══════════════
#define NVS_NS_WIFI      "wifi_config"
#define NVS_KEY_WIFI     "wifi_config"

// ═══════════════ GPIO Pins ═══════════════
#define PIN_LED_HALF_SEC   13   // LED 半秒闪烁 (NTP 对齐北京时间)
#define PIN_LED_MINUTE     14   // LED 整分钟闪烁

#endif // PROJECT_CONFIG_H
