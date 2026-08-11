#### 8.5今天准备学习uart，打算看ESP-IDF的编程指南学习

小任务：第一个示例实现同一 UART 接口完成两个独立任务的通信。其中一个任务定期发送 "Hello world"，另一个任务接收并打印 UART 接收到的数据。

### 1.

我写好了my_uart组件，然后再main中引用include ”my_uart“一直报错![image-20260805164034687](C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260805164034687.png)

#### 2.

具体流程（main中的）：设置一个信箱和一个计数器——>然后清除这个信箱里的内容（有的话）——>去读取数据，读完了让灯亮一下——>然后回显数据

![image-20260808161307573](C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260808161307573.png)

现在就是这样，我好像不能跟他互动，我往monitor里不能输入内容

答：IDE的是一个只读串口监视器面板，不接受输入，好啦，那么我就去powershell里面试一下下

UART_NUM_0已经默认给下载和打印，所以最好不要用用UART_NUM_1

##### 感觉不太对的样子，默默的找出USB-TTL，下面重新开始

##### uart_read_bytes 和uart_write_byte

读的话就是以esp32为基准，从外面读，这个数据从电脑经过TX线，来到了esp32的RX线，然后由UART自动接收后放在环形缓冲区里，然后呢我去读环形缓冲区里有多少数据，之后把他们搬到自己定义的数组里，这大概就是一个read的过程

写的话呢就反过来

##### 现在遇到一个问题就是，ESP32S3完全没反应，然后esptool连握手信号都收不到

短接CH340验证这个CH340是好的

<img src="C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260810180225655.png" alt="image-20260810180225655" style="zoom:50%;" />

朱帮我解决了问题：主要有三个问题

1.CH340没有和ESP32S3板连接3.3V的线

2.在初始化函数那里出了问题，我使用的那种写法主要适用于“动态修改”，就是程序已经跑起来了，这时候遇到突发情况，需要临时更改一个数据的情况，而不是初始化

<img src="C:\Users\rain\AppData\Roaming\Typora\typora-user-images\image-20260811152006101.png" alt="image-20260811152006101" style="zoom:50%;" />

3.main函数中延时太长了，相当于10s中打印一次数据

4.打印日志的时候用ESP_LOGI("MAIN","内容")；不能用printf

大概就这些问题，然后朱是使用的UART0往串口里面烧录,然后UART1进行数据传输，UART0是硬件个固定的烧录口，COM口可以自己选，但是必须接到芯片的UART0引脚。就是配置了UART1，数据就会从连接UART1的线传出去，但是烧录程序的时候还是只会走连接UART0的COM口