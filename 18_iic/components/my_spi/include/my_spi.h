//
// Created by rain on 2026/8/22.
//
#include "esp_err.h"
#ifndef INC_18_SPI_MY_SPI_H
#define INC_18_SPI_MY_SPI_H
esp_err_t my_spi_init(void);
esp_err_t read_my_reg(uint8_t cmd,uint32_t addr,uint8_t *buffer,uint32_t len);
#define MY_SPI_CMD_READ_DATA 0X03
#define MY_SPI_NO_ADDR         0xFFFFFFFFUL
#endif //INC_18_SPI_MY_SPI_H
