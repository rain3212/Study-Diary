#define RXD2 18
#define TXD2 17

#define OLED_SDA 32
#define OLED_SCL 33

#include "OLEDDisplay.h"

String inputText = "";

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(1000);

  Serial.println("ESP32B ready");
  Serial.println("Waiting for data from ESP32A...");

  initOLED(OLED_SDA, OLED_SCL);
}

void loop() {
  while (Serial2.available()) {
    char c = Serial2.read();

    inputText += c;
    showText(inputText);

    // B 的串口监视器显示收到的数据
    Serial.write(c);
  }
}

