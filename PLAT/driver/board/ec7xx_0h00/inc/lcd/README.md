# LCD 

```bash
├── lcdDev 
|    ├── disFormat.c
|    ├── lcdComm.c
|    ├── lcdDev_7796.c
|    ├── lcdDev_3037.c
|    └── lcdDev_7789.c
└──  lcdDrv.c
```

* 时钟测量接口用于分析执行耗时，并将所有耗时存储到外部flash导出（格式为csv）

```bash
uint32_t measure_execution(void (*func)(va_list), ...)
```

## LSPI

## Interface

* 3线I型 – SCL、CS、SDA - 3线1data 9bit SPI – 半双工，只有一根双向的数据线
* 3线II型 –SCL、CS、SDI、SDO - 3线2data 9bit SPI – 全双工，有独立的MOSI、MISO
* 4线I型 – SCL、DCX、CS、SDA - 4线1data 8bit SPI – 半双工，只有一根双向的数据线
* 4线II型 – SCL、DCX、CS、SDI、SDO - 4线2data 8bit SPI –全双工，有独立的MOSI、MISO

**SCL对应LSPI CLK,DCX对应LSPI WRX,多数LCD没有SDO**

```bash
    lspiCtrl.busType = 1;   // Interface II
    lspiCtrl.line4 = 1;
    lspiCtrl.data2Lane = 0;
```

## DataFormat

* 相较很多LCD的接收数据格式，LSPI默认先发送放低位（大端模式，bit0在低位），所以会导致颜色反转，读出的数据也会大小端反转。
* 如果采用32位寄存器长度，不进行数据压缩，是无法修法数据顺序的。

```bash
    lspiDataFmt.wordSize = 15;  
    lspiDataFmt.txPack = 1;
    lspiDataFmt.rxPack = 1;
    lspiDataFmt.endianMode     = 1;
    lspiDataFmt.rxFifoEndianMode     = 1;
    lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
```

在转换编码时，典型流程如将RGB565转换为YUV420的步骤：

* 将RGB565转换为RGB888
* 将RGB888转换为YUV444
* 将YUV444转换为YUV420

### 注意事项

* 1）使用DMA->LSPI需要使能相关时钟
```bash
    PSRAM_dmaAccessClkCtrl(true);
```

* 2）DMA传输RGB565奇数像素时最后一个像素无法输出，需要在填充DMA数据时补一个像素（RGB565 - 2bytes），而LSPI的传输长度不需要补，这样实际传输的像素不会多出一个。

## LCD 

| Driver | Interface | DataLane | Resolution | TransTime | FrameRate
| ------- | ------- | ------- | ------- | ------- | ------- |
| ST7789 |  4Wire-II |  1  | 240x320 | 24ms | 41 fps
| ST7789 |  4Wire-II |  2  | 240x320 | 14ms | 72 fps
| NV3037 |  4Wire-II |  2  | 320x480 | 27ms | 37 fps
| ST7796 |  4Wire-II |  1  | 320x480 | 41ms | 24 fps

* 最高速率在LSPI配置51MHz时钟条件下测得，相关配置**lspi.c**

### ST7796S

* 读数据的时钟推荐12MHz，时钟太高会出现数据无法对齐
* 读取LCD RAM数据使用0x3E寄存器
* MADCTL 0x36 / MADCTR影响RGB排列顺序
* RDDST 0x09 (0x53) 写入和读出的数据反色

MADCTL寄存器控制显示方向
```bash
Bit D7 MY
0：Top to Bottom
1：Bottom to Top

Bit D6 MX 
0：Left to Right (MX=0 MEM RGB)
1：Right to Left (MX=1 MEM BGR)

Bit D5 MV
1： Row/column exchange

Bit D4 ML (Vertical Refresh Order)
0：LCD refresh Top to Bottom
1：LCD refresh Bottom to Top

Bit D3 RGB 
0：RGB
1：BGR

Bit D2 MH (Horizontal Refresh Order)
0：Left to Right
1：Right to Left
```


