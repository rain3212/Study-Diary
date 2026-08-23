//
// Created by rain on 2026/8/23.
//
#include "my_gettime.h"
#include "esp_sntp.h"

#include <stdlib.h>
#include <time.h>

void my_gettime_init(void)
{
    // 设置时间同步模式：轮询 NTP 服务器
    esp_sntp_setoperatingmode(ESP_SNTP_OPMODE_POLL);
    // 设置要访问的 NTP 服务器，按顺序依次尝试
    esp_sntp_setservername(0, "pool.ntp.org");
    esp_sntp_setservername(1, "cn.pool.ntp.org");
    esp_sntp_setservername(2, "ntp1.aliyuncs.com");
    // 初始化 SNTP
    esp_sntp_init();
    // 设置当前时区为东八区（北京时间），POSIX TZ 格式应为 "CST-8"
    setenv("TZ", "CST-8", 1);
    // 让时区设置立刻生效
    tzset();
}

bool my_gettime_is_synced(void)
{
    //相当于在比较,“当前 SNTP 状态，是不是等于‘同步完成’？”
    return esp_sntp_get_sync_status() == SNTP_SYNC_STATUS_COMPLETED;
}
