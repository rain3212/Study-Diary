### 定时器  8.15

实现：定时器到时间之前，灯一直亮，定时器到时间之后，灯开始一闪一闪的亮

像vtaskdelay（）这是属于软件中断，会占用CPU,而且精确的没那么高，定时器属于硬件中断不会占用CPU

上面的内容几天前已经提交属于systimer（系统定时器）内容

#### 今天学习gptimer（通用定时器）

首先我看了一下两者的区别，简单的来说，就是systimer的一些参数是固定的，比如时钟（16MHZ）这种，一般情况是操作系统调用，我是用不到的；然后gptimer是可以自己设定的，一般应用程序用的是这个。

然后我在手册上学找到了GPtimer准备学习一下，手册上有GPtimer驱动的一般使用流程

发怒了！！！！出去吃螺蛳粉！

回来了，没吃，继续学

```c++
typedef struct {
    gptimer_clock_source_t clk_src;      /*!< GPTimer clock source */
    gptimer_count_direction_t direction; /*!< Count direction */
    uint32_t resolution_hz;              
                                             
    int intr_priority;                       
    struct {
        uint32_t intr_shared: 1;        
        uint32_t allow_pd: 1;            
                                             
    } flags;                             /*!< GPTimer config flags*/
} gptimer_config_t;
```

这个是我有疑问的结构体，查了一下这是结构体里嵌套了匿名结构体（两个的区别就是匿名结构体struct后面没有名字）

```c++
 .flags = {
        .intr_shared = 0,                 // 不与其他外设共享中断
        .allow_pd = 1,                    // 允许睡眠时关闭电源域以省电
    },
```

可以这么用？反正现在这样写没有报错，这样写是可以的

注意，在函数内部不可以定义函数