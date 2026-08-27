//
// system_event.cpp — 全局事件组实体
//

#include "system_event.h"

EventGroupHandle_t g_system_event_group = nullptr;

void system_event_init() {
    if (g_system_event_group == nullptr) {
        g_system_event_group = xEventGroupCreate();
    }
}
