#include <Wire.h>

#define ATH20_ADDR 0X38

void setup() {

  Serial.begin(9600);

  Wire.begin();
  Wire.setClock(100000);
 
}

bool readATH20(float *temperature,float *humidity){
  uint8_t data[7];

  //发送测试命令
  Wire.beginTransmission(ATH20_ADDR);
  Wire.write(0xAC);
  Wire.write(0x33);
  Wire.write(0x00);
  if(Wire.endTransmission()!=0){
    return false;
  }
  //等待80ms
  delay(80);

  //读取温湿度
  Wire.requestFrom(ATH20_ADDR,7);
  if(Wire.available()<7){
    return false;
  }
  for(int i=0;i<7;i++){
    data[i]=Wire.read();
  }
  //判断Status的状态
  if(data[0]&0x80){
    return false;
  }

  //拼接数据
  uint32_t rawhumidity=((uint32_t)data[1]<<12|(uint32_t)data[2]<<4|(uint32_t)data[3]>>4);
  uint32_t rawtemperature=(((uint32_t)data[3]&0x0F)<<16|(uint32_t)data[4]<<8|(uint32_t)data[5]);

  //处理数据
  *humidity = rawhumidity*100.0/1048576.0;
  *temperature = rawtemperature/1048576.0 *200.0-50.0;

  return true;





}




void loop() {
  float temp,humi;
  if(readATH20(&temp,&humi)){
    Serial.println("温度是：");
    Serial.println(temp);
    Serial.println("C");
    
    Serial.println("湿度是");
    Serial.println(humi);
    Serial.println("%RH");
  }else {
    Serial.println("Read AHT20 failed");
  }
  delay(1000); 
}
