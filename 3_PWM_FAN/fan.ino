const int fanPin=7;
const int buttonPin=15;
//代表按下几次
int x=0;

//设置电机速度
void setSpeed(int percent){
  int pwmValue=percent * 255 / 100;
  ledcWrite(fanPin,pwmValue);
}
//关闭电机操作
void closeFan(){
  ledcWrite(fanPin,0);
}

void setup(){
  pinMode(buttonPin,INPUT_PULLDOWN);
  ledcAttach(fanPin,20000,8);
  //初始的时候是关闭的状态
  ledcWrite(fanPin, 0);
}
void loop() {
  if (digitalRead(buttonPin) == HIGH) {
    delay(20);  // 消抖

    if (digitalRead(buttonPin) == HIGH) {
      //到这里就是确认按下了，开始算时间
      unsigned long startTime = millis();
      // 等待按钮松开，同时判断是否长按
      while (digitalRead(buttonPin) == HIGH) {
        if (millis() - startTime >= 2000) {
          closeFan();
          x = 0;
          // 只要手没有松开就一直是高电平，就一直在这里等
           while (digitalRead(buttonPin) == HIGH) {
            delay(10);
          }
          return;  // 长按已经处理完，直接退出本次 loop
        }
      }
      // 能走到这里说明 2 秒内松手了是短按
      x++;
      if (x % 3 == 1) {
        setSpeed(30);
      } else if (x % 3 == 2) {
        setSpeed(60);
      } else {
        setSpeed(100);
      }
    }
  }
}