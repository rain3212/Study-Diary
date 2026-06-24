#ifndef LED_595_SOFT_H
#define LED_595_SOFT_H

#include <Arduino.h>

const int DATA_PIN  = 35;
const int LATCH_PIN = 36;
const int CLOCK_PIN = 37;
const bool LED_ACTIVE_LOW = false;

void led_595_soft_init();
// 写入 16 位数据到两片 74HC595
void write595(uint16_t value);
// 软件 SPI，手动移位
void shiftOutSoft(byte data);

#endif
