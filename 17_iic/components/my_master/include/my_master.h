//
// Created by rain on 2026/8/20.
//

#ifndef INC_17_IIC_MY_MASTER_H
#define INC_17_IIC_MY_MASTER_H
#define id_add 0xD0

void my_master_init();
esp_err_t read_BME280_data(uint8_t *id);
void i2c_scan(void);
#endif //INC_17_IIC_MY_MASTER_H
