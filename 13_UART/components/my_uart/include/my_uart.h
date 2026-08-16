//
// Created by rain on 2026/8/5.
//

#ifndef DEMO1_MY_UART_H
#define DEMO1_MY_UART_H

#include "driver/gpio.h"
#include "driver/uart.h"
#include "driver/uart_select.h"
#include "string.h"

#define UART_NUM UART_NUM_1
extern QueueHandle_t uart_queue;

void uart_init();
#endif // DEMO1_MY_UART_H
