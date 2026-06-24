#include "led_595_soft.h"

void setup() {
  led_595_soft_init();
}

void loop() {
  // 从第 1 个 LED 流到第 16 个 LED
  for (int i = 0; i < 16; i++) {
    uint16_t data = 1 << i;
    write595(data);
    delay(120);
  }

  // 再从第 16 个 LED 流回第 1 个 LED
  for (int i = 14; i >= 1; i--) {
    uint16_t data = 1 << i;
    write595(data);
    delay(120);
  }
}
