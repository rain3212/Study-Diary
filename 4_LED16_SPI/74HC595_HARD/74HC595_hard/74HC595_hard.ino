#include "led_spi.h"

void setup() {
  led_spi_init();
}

void loop() {
  uint16_t state = 0x0000;

  // 依次点亮
  for (int i = 0; i < 16; i++) {
    state |= (1 << i);
    hardware_spi_control(state);
    delay(200);
  }

  // 依次熄灭
  for (int i = 0; i < 16; i++) {
    state &= ~(1 << i);
    hardware_spi_control(state);
    delay(200);
  }
}
