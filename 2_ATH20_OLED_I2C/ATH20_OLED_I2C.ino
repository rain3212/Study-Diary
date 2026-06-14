#include <Wire.h>
#include "AHT20.h"
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#include "project_conf.h"


Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
String text = "";

void setup() {

    Serial.begin(9600);
    Wire.begin();
    Wire.setClock(100000);
    //oled显示屏
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED 初始化失败");
        while (1);
    }

    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED Ready");
    display.display();
    //舵机
    ledcAttach(servo, freq, resolution);
}

void loop() {
    //读取温湿度
    float temp, humi;

    display.clearDisplay();
    display.setCursor(0, 0);

    if (readATH20(&temp, &humi)) {
        Serial.println("温度是：");
        Serial.println(temp);
        Serial.println("C");

        display.print("temp:");
        display.print(temp, 1);
        display.println("C");

        Serial.println("湿度是");
        Serial.println(humi);
        Serial.println("%RH");

        display.print("Humi: ");
        display.print(humi, 1);
        display.println(" %RH");

    } else {
        Serial.println("Read AHT20 failed");
    }
    delay(1000);
    //控制舵机
    if (temp > 20) {
        display.println("Servo: 1.7ms");
        ledcWrite(servo, (1.7 / 20) * pow(2, resolution));

    } else {
        display.println("Servo: 1.5ms");
        ledcWrite(servo, (1.5 / 20) * pow(2, resolution));
    }
    display.display();
    delay(1000);
}
