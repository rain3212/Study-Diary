#include <Wire.h>
#include "AHT20.h"

#define ATH20_ADDR 0X38

void setup() {

  Serial.begin(9600);

  Wire.begin();
  Wire.setClock(100000);
}

void loop() {
  float temp, humi;
  if (readATH20(&temp, &humi)) {
    Serial.println("温度是：");
    Serial.println(temp);
    Serial.println("C");

    Serial.println("湿度是");
    Serial.println(humi);
    Serial.println("%RH");
  } else {
    Serial.println("Read AHT20 failed");
  }
  delay(1000);
}
