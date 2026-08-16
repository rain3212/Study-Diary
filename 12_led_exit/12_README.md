## 1.在写组件的时候，目录是长这样的

<img src="C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260802102217618.png" alt="image-20260802102217618" style="zoom:80%;" />

| 功能  | 新组件名             |
| ----- | -------------------- |
| GPIO  | `esp_driver_gpio`    |
| UART  | `esp_driver_uart`    |
| I2C   | `esp_driver_i2c`     |
| SPI   | `esp_driver_spi`     |
| LEDC  | `esp_driver_ledc`    |
| ADC   | `esp_adc`            |
| Timer | `esp_driver_gptimer` |
| RMT   | `esp_driver_rmt`     |
| I2S   | `esp_driver_i2s`     |

## 3. 

X?a：b;  就是x为真的时候执行a否则执行b

## 4 .

gpio_config_t是结构体，用来一次性填好所有配置参数，然后用gpio_config(&变量ming)

## 5.

最外面的cmake就写三句话，比如这个，第二句话多出来了，然后就会一直报错，大概是这个原因？还有可能是我在一个组件里调用了另一个组件里的东西（这个也需要特殊处理吗，反正我处理了一下，他就好了）就改动了这两个东西，然后锤子，三角和虫子就出来了。

![image-20260804201615062](C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260804201615062.png)

![image-20260804201954938](C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260804201954938.png)

但是呢还有一个问题就是这个东西还是错叉，问了一下说是cmake还没成功...............但是很奇怪

然后呢，我就去powershell里面build了一下，也失败了，然后机器告诉我这个ESP-IDF的启动代码是C写的，而我这个是main中放的是.cpp然后extern "C" void app_main(void)这样改就行了。（嗯，很好又解决一个问题，但是那个叉还是不行）很神奇，为什么在powershell里面可以build，在Clion中不行.......

因为：

​       **两者是独立的**：

- PowerShell 里的 `idf.py build` 直接调用 ESP-IDF 的构建系统
- CLion 里的运行配置依赖它**自己的 CMake 解析结果**

我reload cmake project了，成功了，但是那个ALL依旧是错叉，CMake 配置成功了，但 Run/Debug Configuration（运行配置）本身还是坏的，然后莫名好了

![image-20260804202226158](C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260804202226158.png)

## 6

关于我得代码：如果在执行到vTaskDelay(500);（我的意思是刚好执行结束马上要执行下一句的那个临界点）这句代码的时候中断了去执行了中断，那是不是应该灯亮一下之后立马灭了

答：

**是的，完全正确。** 灯会在中断触发时点亮，然后在 `vTaskDelay(500)` 结束后立即熄灭。你看到的会是一个**短暂的闪烁**（亮 0~500ms，取决于中断发生在 delay 的哪个时刻）。