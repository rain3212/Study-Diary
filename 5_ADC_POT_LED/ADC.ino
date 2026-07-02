const int adcPin = 4;
const int ledPin = 11;
const int pwmfre = 5000;
const int pwmresolution = 12;

void setup() {

    pinMode(adcPin, INPUT);
    pinMode(ledPin, OUTPUT);
    //ADC读数为12位，设置衰减,约3.3V
    analogReadResolution(12);
    analogSetPinAttenuation(adcPin, ADC_11db);
    //初始化串口
    Serial.begin(9600);
    delay(100);
    //初始化pwm
    if (!ledcAttach(ledPin, pwmfre, pwmresolution)) {
        Serial.println("初始化失败");
        while (1) {
            delay(100);
        }
    }
    Serial.println("初始化成功");

}

void loop() {
    //这是一个占比，并不是真正的电压值
    float voltageMv = analogRead(adcPin);
    Serial.print("    Voltage = ");
    Serial.print(1100/0.25*voltageMv/4095);
    Serial.println(" mV");
    ledcWrite(ledPin, voltageMv);
    delay(100);
}
