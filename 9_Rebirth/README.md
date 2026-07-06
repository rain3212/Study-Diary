# 使用 ESPIDF 进行开发

> 从这个地方开始，算是真正开始学习工业级的项目，软件和硬件是同时进行的，缺一不可

## 核心原则

1.  遇到的 ESPIDF 所有问题，去查官方文档为准，[ESP-IDF 官方 API 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/api-reference/index.html) 

2.  遇到的硬件电路设计问题，找数据手册参考电路设计抄，物料选择合适（因为要考虑最终成品成本），也可以找外面的参考设计进行抄 例如 [立创开源平台](https://oshwhub.com/)

3.  代码不要AI写什么信什么，AI的知识容易过期，你需要复制相关信息给AI，代码作为参考，不要复制粘贴，自己写，搞懂这是为什么

4.  **面向项目学习**，先把基础外设，语法，简单的电路板绘制打样学好，**工科都是在做项目的时候，需要什么现场学什么**，工科是一个终身学习的东西，不太可能说，学好了这辈子就吃老本，每天技术都是在不断进化的，自己需要主动去关注新的技术

>  每学一个知识点，可以在main创建一个文件夹，然后新建分支提交PR我会看的
>
>  每个项目都要要求:
>
>  -  遇到过什么 bug
>  -  如何定位
>  -  用了日志，GDB，示波器，逻辑分析仪还是万用表？？
>  -  最后怎么修

## 任务规划

### 软件部分

>  学习路径: 走 `gcc/openocd/cmake/gdb/freertos` 路线(就算以后转Linux也方便)，知道这个是什么，并自己找相关教程进行学习(了解基础并学会基本的使用) 因为一般都是做项目用到什么学什么

1.  **先搞定基础外设用法**: 依次搞定 GPIO, UART, I2C, SPI, PWM, ADC，像我之前给你布置的task一样，自己设计一个场景，用ESPIDF语法进行解决，可以跟着 [正点原子教程](https://www.bilibili.com/video/BV1EPisBWEUX) 来
    开始设计外设实验的时候，先自己去看 [ESP-IDF 官方 API 文档](https://docs.espressif.com/projects/esp-idf/zh_CN/latest/esp32s3/api-reference/index.html)  和 [官方例子](https://github.com/espressif/esp-idf/tree/master/examples)，不要上来直接问AI这玩儿怎么写你对着答案抄，尝试看着官方文档和正点原子教程自己能不能写出来

    >  举个例子，点亮一个LED， 参考 [ESPIDF GPIO](https://docs.espressif.com/projects/esp-techpedia/zh_CN/latest/esp-friends/get-started/case-study/peripherals-examples/gpio-examples/index.html) 和 [官方例子GPIO](https://github.com/espressif/esp-idf/blob/master/examples/peripherals/gpio/generic_gpio/main/gpio_example_main.c) 两者对着看，可以写成两种形式
    >  ```c
    >  gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
    >  gpio_set_level(GPIO_NUM_0, 1);
    >  ```
    >
    >  也可以写成:
    >  ```c
    >  gpio_config_t io_conf = {
    >      .intr_type = GPIO_INTR_DISABLE,
    >      .mode = GPIO_MODE_OUTPUT,
    >      .pin_bit_mask = (1ULL << GPIO_NUM_0),
    >      .pull_down_en = GPIO_PULLDOWN_DISABLE,
    >      .pull_up_en = GPIO_PULLUP_DISABLE
    >  };
    >  gpio_config(&io_conf);
    >  gpio_set_level(GPIO_NUM_0, 1);
    >  ```

2.  **FreeRTOS**: 理解多任务是如何在单片机上并发运行的，自己写一段代码进行验证学习，我认为比较重点的是: 

    >  **Task 消息队列 任务通知 信号量 互斥锁 软件定时器 事件标志组 看门狗 中断不能塞阻塞API 检查是否会栈溢出**

3.  **使用C++**: 

    >  C++ 学到能写 “轻量级驱动封装” 和 “清晰业务状态机” 就够了

    -  **Class **：比如封装一个 `class AHT20`，把 I2C 的地址、初始化函数、读写函数都封在里面，对外只暴露 `getTemperature()` 这种干净的接口

    -  **构造函数与析构函数：** 在构造函数里做 GPIO 初始化，在析构函数里释放资源

    -  **虚函数与多态（我没做过巨型项目，听说有用?）：** 比如定义一个基类 `class Sensor`，然后让不同的传感器继承它

    -  **状态机**: 就是把一个设备/功能在不同阶段“能做什么，等什么，出错怎么办，下一步去哪”用明确状态和事件写出来，而不是到处用 if-else 瞎跳

    -  指针 数组 结构体 联合体 函数指针 volatile const static extern 位运算 宏 内存对齐 大小端 环形缓冲区 状态机 错误码设计 这种基础必须得会

       >  基础很多人面试就像之前面试官问你 static 是什么，你说 “静态” 。。。。这个算是最基础的确实需要搞懂
       >
       >  为什么寄存器变量要 volatile？
       >   UART 接收怎么做环形缓冲区？
       >   I2C 读传感器失败怎么重试？
       >   任务栈溢出怎么排查？
       >   中断里为什么不能 printf？
       >
       >  自己多问自己几个问题类似于上面的

4.  理解CMake工程，gcc openocd，gdb menuconfig 等等，可以找相关教程进行学习，学成 **熟练的使用者** 更好控制硬件是目的

    -  CMake: 理解 `CMakeLists.txt` 的作用

       熟练使用 `idf_component_register`：知道怎么把自己的 `.c/.cpp` 文件加进 `SRCS`，怎么把头文件路径加进 `INCLUDE_DIRS`，怎么在 `REQUIRES` 里依赖其他组件（比如依赖 `freertos` 或 `driver`）。

       知道怎么把一个开源的 C 语言库（比如一个 OLED 驱动）变成一个 ESP-IDF 可以调用的 Component。

    -  **OpenOCD / GDB (硬件调试)：** 

       >  目标：告别只会用 `printf` 查 Bug 的时代

       学会设断点 ，单步跳入/跳出，查看寄存器值，查看内存地址里的数据，直接修改寄存器值，看懂崩溃日志等

    -  gcc: 知道编译汇编链接产物等。。

    -  menuconfig: 会改esp32的分区表，日志等级，freertos等各种配置

5.  **高级特性**:  这些都是需要自己做例子学会的

    -  Wi-Fi/OTA升级/内存防泄漏/MQTT/ UART AT 指令状态机/HTTPS,HTTP/TCPIP/JSON/NVS参数保存/BLE等等。

### 硬件部分

1.  **PCB 打板：** 画原理图 -> PCB -> 打样，比如你之前的74HC595的16个LED面包板是不是接线很痛苦？先从这个开始，bilibili搜索 立创EDA 教程，绘制电路图和电路板，使用esp32C3，学会串口烧录，USB烧录，最小系统绘制，打样看看

2.  **PCB绘制**: 画电路图和PCB的时候，需要下载硬件对应的数据手册进行参考，上面一般都会有参考电路，以及需要注意每个引脚是做什么的

    >  ESP32C3模组电路过于简单，官方也没有模组的参考电路你可以参考立创开源平台
    >
    >  需要注意的点是，比如USB下载官方会有配置电阻的建议，以及如果使用串口自动下载电路你可以直接抄淘宝开发板子的或者立创开源平台上的设计，去哪里抄其实无所谓，东西跑起来了就行
    >
    >  以及ESP32的管脚，比如串口，IIC他其实是有推荐的引脚的，你可以参考数据手册，我这里就放几张图片，一个是启动上电顺序，一个是推荐引脚图
    >
    >  **（这地方我也不知道怎么系统的学，我也是抄别人的抄着抄着就会了）**
    >
    >  [ESP32C3技术规格书](https://documentation.espressif.com/esp32-c3_datasheet_cn.html) (相关图片我放在同目录Pictures下面了)

3.  硬件部分实际上就是原理图绘制，PCB layout (初期信号线速度都比较低，没有干扰等问题，到后期可能就需要考虑上电浪涌，电磁干扰，阻抗匹配，以及电源设计，这个到后期实际做项目的时候需要什么现场学什么)

## 小项目制作

>  目前设想的是，基础打好，实际做一个项目，直接从项目中学习，需要什么学什么
>
>  项目都要按照**商业级产品的标准来制作**(你可以问下AI，做一个商业级物联网台灯需要到达什么水平)，制作玩具学到的东西和做一个商业级的项目是完全两码事，需要考虑大量的冗余，可靠性测试，成本控制等等。（**如果有条件最好用SolidWorks画一个外壳，做一个能完全包揽整个工程的人)**

1.  小米台灯 （涉及DCDC电源设计，OTA，WI-FI手机APP控制，MQTT，云平台等)
2.  GPS轨迹追踪器 (BC20/AT指令/低功耗电源/电池储能充电，射频，MQTT，云平台等)
3.  FOC无刷电机控制（有感/无感） (涵盖PWM/中断/运放采样/大电流PCB/数学与实际结合等......)

## 结语

如果你这些都能完成的话，能力确实相当出色，可以找BOSS上面的公司，看看他们需要什么技术栈，直接对着他们的技术栈再学一下就可以了

我现在的眼界限制，我认为应该需要会做一整个**完整的产品设计**，而不是做一个小的部分，具有更强的不可替代性