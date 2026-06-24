#ifndef LED_SPI_H
#define LED_SPI_H

#include <Arduino.h>
#include <SPI.h>

#define DATA_PIN   11
#define CLK_PIN    12
#define LATCH_PIN  10

void led_spi_init();
void hardware_spi_control(uint16_t led_state);

#endif
