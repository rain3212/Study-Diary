#include "OLEDinit.h"

#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128
#define SCREEN_HEIGHT 64
#define OLED_RESET -1

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

void OLEDinit() {
    if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
        Serial.println("OLED init failed");
        while (true);
    }

    display.clearDisplay();
    display.display();
}

void showOLEDReady() {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);
    display.setCursor(0, 0);
    display.println("OLED Ready");
    display.display();
}

void showSensorData(float temp, float humi) {
    display.clearDisplay();
    display.setTextSize(1);
    display.setTextColor(SSD1306_WHITE);

    display.setCursor(0, 0);
    display.print("Temp: ");
    display.print(temp);
    display.println(" C");

    display.setCursor(0, 16);
    display.print("Humi: ");
    display.print(humi);
    display.println(" %");

    display.display();
}
