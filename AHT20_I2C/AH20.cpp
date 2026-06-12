#include <Wire.h>
#include "AHT20.h"

#define ATH20_ADDR 0X38

bool readATH20(float *temperature, float *humidity) {
  uint8_t data[7];

  //发送测试命令
  Wire.beginTransmission(ATH20_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  if (Wire.endTransmission() != 0) {
    return false;
  }
  //等待80ms
  delay(80);

  //读取温湿度
  Wire.requestFrom(ATH20_ADDR, 7);
  if (Wire.available() < 7) {
    return false;
  }
  for (int i = 0; i < 7; i++) {
    data[i] = Wire.read();
  }
  //判断Status的状态
  if (data[0] & 0x80) {
    return false;
  }

  //拼接数据
  uint32_t rawhumidity = ((uint32_t)data[1] << 12 | (uint32_t)data[2] << 4 | (uint32_t)data[3] >> 4);
  uint32_t rawtemperature = (((uint32_t)data[3] & 0x0F) << 16 | (uint32_t)data[4] << 8 | (uint32_t)data[5]);

  //处理数据
  *humidity = rawhumidity * 100.0 / 1048576.0;
  *temperature = rawtemperature / 1048576.0 * 200.0 - 50.0;

  return true;
}