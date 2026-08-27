//
// system_event.h — 全局 FreeRTOS 事件组
//
// 所有跨模块的状态通知都通过这个事件组完成。
// 每个模块只负责 set/clear 自己的 bit。
//

#ifndef SYSTEM_EVENT_H
#define SYSTEM_EVENT_H

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

// ═══════════════════════════════════════════════════
//  系统事件位定义 (FreeRTOS EventGroup 最多 24 bit)
//  规则：模块就绪 → SetBits，模块故障 → ClearBits
// ═══════════════════════════════════════════════════
#define SYS_EVT_WIFI_CONNECTED   BIT0   // WiFi STA 已获得 IP 地址
#define SYS_EVT_NTP_SYNCED       BIT1   // NTP 至少成功同步过一次

// ═══════════════════════════════════════════════════
//  全局句柄 (system_event.cpp 中定义实体)
// ═══════════════════════════════════════════════════
extern EventGroupHandle_t g_system_event_group;

// 初始化全局事件组 (必须在所有模块之前调用)
void system_event_init();

#endif // SYSTEM_EVENT_H
