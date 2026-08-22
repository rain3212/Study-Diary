//
// Created by rain on 2026/8/22.
//

#include "include/my_spi.h"
#include "driver/spi_master.h"
#include "esp_log.h"
#include  "esp_err.h"
#define MOSI_IO_NUM 12
#define MISO_IO_NUM 10
#define CLK_IO_NUM 11
#define CS_IO_NUM 9
//一共有四个spi但是，01都是内部专用的
#define host_id  SPI2_HOST
//设置一个句柄，用于后续判断是否初始化好了
static spi_device_handle_t s_spi = NULL;
const char * TAG = "my_spi";
esp_err_t my_spi_init()
{
    //配置spi总线
    spi_bus_config_t buscfg = {};
    buscfg.mosi_io_num = MOSI_IO_NUM;
    buscfg.miso_io_num = MISO_IO_NUM;
    buscfg.sclk_io_num = CLK_IO_NUM;
    buscfg.quadhd_io_num = -1;
    buscfg.quadwp_io_num = -1;
    buscfg.max_transfer_sz = 4096;
    //初始化总线
    esp_err_t ret = spi_bus_initialize(host_id, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "总线失败");
        return ret;
    }
    //现在要把设备挂在总线上,这个spi设备平时怎么工作的
    spi_device_interface_config_t devcfg = {};
    //必须配置的是模式，时钟频率和cs引脚
    devcfg.command_bits = 8;
    devcfg.clock_speed_hz = 100000;
    devcfg.mode = 0;
    devcfg.address_bits = 24;
    devcfg.spics_io_num = CS_IO_NUM;
    devcfg.queue_size = 1;
    //0x03读数据这类"先发命令/地址、再从MISO收数据"的协议是半双工时序，
    //必须加这个标志，否则驱动按全双工校验会拒绝"只有接收没有发送"的事务
    devcfg.flags = SPI_DEVICE_HALFDUPLEX;
    //在SPI总线上分配一个设备
    ret = spi_bus_add_device(SPI2_HOST, &devcfg, &s_spi);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG, "初始化失败");
        return ret;
    }

    ESP_LOGI(TAG, "初始化完成");
    return ESP_OK;
    //到这里初始化工作就差不多了
}
//现在要开始读寄存器了
esp_err_t read_my_reg(uint8_t cmd,uint32_t addr,uint8_t *buffer,uint32_t len)
{
    //先判断是否初始化完成
    if (s_spi==NULL)
    {
        ESP_LOGI(TAG,"先进行初始化");
        return ESP_FAIL;
    }
    if (buffer==NULL||len==0)
    {
        ESP_LOGI(TAG,"指令错误");
        return ESP_FAIL;
    }
    //这次spi通信具体干什么
    spi_transaction_ext_t t={};
    t.base.flags=SPI_TRANS_VARIABLE_ADDR;
    t.base.cmd=cmd;
    t.base.rxlength=len*8;
    t.base.rx_buffer=buffer;
    if (addr==MY_SPI_NO_ADDR)
    {
        t.address_bits=0;
    }else
    {
        t.base.addr=addr;
        t.address_bits=24;
    }
    //轮询传输事务
    esp_err_t ret =spi_device_polling_transmit(s_spi,(spi_transaction_t *)&t);
    if (ret != ESP_OK)
    {
        ESP_LOGI(TAG,"失败");
        return ret;
    }
    return ESP_OK;

}
