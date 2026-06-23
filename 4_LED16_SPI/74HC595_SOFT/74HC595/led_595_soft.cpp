#include "led_595_soft.h"

void led_595_soft_init() {
  pinMode(DATA_PIN, OUTPUT);
  pinMode(LATCH_PIN, OUTPUT);
  pinMode(CLOCK_PIN, OUTPUT);

  digitalWrite(DATA_PIN, LOW);
  digitalWrite(CLOCK_PIN, LOW);
  digitalWrite(LATCH_PIN, LOW);

  write595(0x0000);   // 开机先全部熄灭
}

// 写入 16 位数据到两片 74HC595
void write595(uint16_t value) {
  if (LED_ACTIVE_LOW) {
    value = ~value;
  }

  // 把RCK拉低，准备移入数据
  digitalWrite(LATCH_PIN, LOW);

  shiftOutSoft((value >> 8) & 0xFF);
  shiftOutSoft(value & 0xFF);

  // 刷新到输出端
  digitalWrite(LATCH_PIN, HIGH);
}

// 软件 SPI，手动移位
void shiftOutSoft(byte data) {
  for (int i = 7; i >= 0; i--) {
    // 先拉低时钟，准备设置数据
    digitalWrite(CLOCK_PIN, LOW);

    if (data & (1 << i)) {
      digitalWrite(DATA_PIN, HIGH);
    } else {
      digitalWrite(DATA_PIN, LOW);
    }

    digitalWrite(CLOCK_PIN, HIGH);
  }
}
