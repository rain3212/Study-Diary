#ifndef OLED_DISPLAY_H
#define OLED_DISPLAY_H

#include <Arduino.h>

void initOLED(int sdaPin, int sclPin);
void showText(const String& text);

#endif
