#include "led_spi.h"

SPIClass mySPI(FSPI);

void led_spi_init() {
  pinMode(LATCH_PIN, OUTPUT);
  digitalWrite(LATCH_PIN, LOW);

  // 参数顺序：SCK, MISO, MOSI, SS
  // 74HC595 不需要 MISO 和 SS，所以写 -1
  mySPI.begin(CLK_PIN, -1, DATA_PIN, -1);
}

void hardware_spi_control(uint16_t led_state) {
  uint8_t highByteData = (led_state >> 8) & 0xFF;
  uint8_t lowByteData  = led_state & 0xFF;

  digitalWrite(LATCH_PIN, LOW);

  mySPI.beginTransaction(SPISettings(1000000, MSBFIRST, SPI_MODE0));

  mySPI.transfer(highByteData);
  mySPI.transfer(lowByteData);

  mySPI.endTransaction();

  digitalWrite(LATCH_PIN, HIGH);
  delayMicroseconds(1);
  digitalWrite(LATCH_PIN, LOW);
}
