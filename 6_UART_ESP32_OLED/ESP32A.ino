#define RXD2 18
#define TXD2 17

void setup() {
  Serial.begin(9600);
  Serial2.begin(9600, SERIAL_8N1, RXD2, TXD2);

  delay(1000);
  Serial.println("ESP32A ready");
}

void loop() {
  // 电脑输入 → 发给 ESP32B
  while (Serial.available()) {
    char c = Serial.read();
    Serial2.write(c);
    Serial.write(c);
  }
}
