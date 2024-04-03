#ifdef FEATURE_LCD_NV3037_ENABLE
#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_3037.h"

#define LCD_CHIPID  0x04  
#define LCD_RDDST   0x09 
#define LCD_MADCTL  0x36

static uint8_t s_MADCTL = 0x0; 
/*
 *  reset lcd driver IC
 *  Parameter: delay ms + hold ms
 */
static void nv3037Rst(uint16_t dly_ms,uint16_t hold_ms)
{   
#ifdef LCD_RST_GPIO_PIN
    if(dly_ms){
        // #ifdef LCD_RST_GPIO_PIN
        // GPIO_pinWrite(LCD_RST_GPIO_INSTANCE, 1 << LCD_RST_GPIO_PIN, 0);
        // #endif
        LCD_RST_LOW;
        mDelay(dly_ms); 
    }
    // #ifdef LCD_RST_GPIO_PIN
    // GPIO_pinWrite(LCD_RST_GPIO_INSTANCE, 1 << LCD_RST_GPIO_PIN, 1 << LCD_RST_GPIO_PIN);
    // #endif
    LCD_RST_HIGH;
    mDelay(hold_ms); 
#endif
}

/*
 *  Write Configuration to Driver Chip Registers
 *  Parameter: none
 */
static void nv3037RegInit(void)
{
    //NV3037A2N_BOE350
    uint8_t set_cmd_xx1[] = {0x06,0x08};
    lspiCmdSend(0xFD,set_cmd_xx1,sizeof(set_cmd_xx1));
    uint8_t set_cmd_xx2[] = {0x08,0x08,0x14,0x14};
    lspiCmdSend(0x75,set_cmd_xx2,sizeof(set_cmd_xx2));
    uint8_t set_cmd_xx3[] = {0x00,0x00,0x01,0x40};
    lspiCmdSend(0x76,set_cmd_xx3,sizeof(set_cmd_xx3));
    uint8_t set_cmd_reg77[] = {0x00,0x00,0x01,0xE0};
    lspiCmdSend(0x77,set_cmd_reg77,sizeof(set_cmd_reg77));
    lspiCmdSet(0x83,0x13);
    lspiCmdSet(0x86,0x07);
    uint8_t set_cmd_reg89[] = {0x03,0x05};
    lspiCmdSend(0x89,set_cmd_reg89,sizeof(set_cmd_reg89));
    uint8_t set_cmd_regc0[] = {0x00,0x28,0x01,0x00};
    lspiCmdSend(0xC0,set_cmd_regc0,sizeof(set_cmd_regc0));
    lspiCmdSet(0xC5,0x15);
    lspiCmdSet(0xC6,0x01);//01:dot inversion 00:column
    lspiCmdSet(0xC7,0x12);
    lspiCmdSet(0xC8,0x00);
    lspiCmdSet(0xCA,0xB2);
    uint8_t set_cmd_0xCB[] = {0x20,0x1A};
    lspiCmdSend(0xCB,set_cmd_0xCB,sizeof(set_cmd_0xCB));
    uint8_t set_cmd_0xCC[] = {0x20,0x26};
    lspiCmdSend(0xCC,set_cmd_0xCC,sizeof(set_cmd_0xCC));
    lspiCmdSet(0xCD,0x20);
    lspiCmdSet(0xF1,0x51); //rev/gs/ss --TN--ips
    lspiCmdSet(0xF7,0x02);
    lspiCmdSet(0x78,0x84); //GVCL
    lspiCmdSet(0x79,0x90); //GVDD
    lspiCmdSet(0x7A,0x2c); //GVSP//2C
    uint8_t set_cmd_0xb2[] = {0x00,0x07,0x08,0x2c,0x2d,0x30};
    lspiCmdSend(0xb2,set_cmd_0xb2,sizeof(set_cmd_0xb2));
    uint8_t set_cmd_0xb5[] = {0x00,0x07,0x08,0x2c,0x2d,0x30};
    lspiCmdSend(0xb5,set_cmd_0xb5,sizeof(set_cmd_0xb5));
    uint8_t set_cmd_0xb1[] = {0x18,0x33};
    lspiCmdSend(0xb1,set_cmd_0xb1,sizeof(set_cmd_0xb1));
    uint8_t set_cmd_0xb4[] = {0x17,0x34};
    lspiCmdSend(0xb4,set_cmd_0xb4,sizeof(set_cmd_0xb4));
    uint8_t set_cmd_0xb0[] = {0x0D,0x04,0x11,0x11,0x11,0x13,0x17,0x0f};
    lspiCmdSend(0xb0,set_cmd_0xb0,sizeof(set_cmd_0xb0));
    uint8_t set_cmd_0xb3[] = {0x0D,0x04,0x11,0x11,0x11,0x14,0x18,0x10};
    lspiCmdSend(0xb3,set_cmd_0xb3,sizeof(set_cmd_0xb3));
    lspiCmdSet(0xB6,0x01); 
    lspiCmdSet(0x69,0x88); 
    uint8_t set_cmd_0xF6[] = {0x01,0x10,0x01};
    lspiCmdSend(0xF6,set_cmd_0xF6,sizeof(set_cmd_0xF6));
    uint8_t set_cmd_0xFD[] = {0xFA,0xFC};
    lspiCmdSend(0xFD,set_cmd_0xFD,sizeof(set_cmd_0xFD));
    lspiCmdSet(0x36,0x00);
    lspiCmdSet(0x35,0x00);
    lspiCmdSet(0x3A,0x55);
    lspiCmdSet(0x46,0x10);
    lspiCmdSet(0x21,0x0);
    lspiCmdSet(0x11,0x0);
    lspiCmdSet(0x29,0x0);
}

/*
 *  nv3037 init entry 
 *  Parameter: callback function
 */
static int nv3037Init(lcdDev_t *lcd, lspiCbEvent_fn cb)
{   
    lcdGpioInit(lcd);
    dmaInit(NULL);
    lcdSpiInit(LCD_HEIGHT,LCD_WIDTH,cb);
    lcdInterfaceType(SPI_4W_II);
    lcdColorMode(DIS_FMT_RGB565);
    nv3037Rst(100,100);
    nv3037RegInit();
    mDelay(100);
    return 0;
}

static int nv3037PrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}

static void nv3037OnOff(lcdDev_t* lcd,bool sta)
{
    #ifdef LCD_BL_PWM_ENABLE
    lcdBackLightOn(sta);
    #endif
}

static int16_t nv3037BackLight(lcdDev_t* lcd,uint16_t level)
{
    #ifdef LCD_BL_PWM_ENABLE
    return lcdBackLightLevel(level);
    #endif
}

/* 
    sx: start x
    sy: start y
    ex: end x
    ey: end y
*/
static void nv3037AddrSet(lcdDev_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8,sx,ex>>8,ex};
    lspiCmdSend(0x2A,set_x_cmd,sizeof(set_x_cmd));
    uint8_t set_y_cmd[] = {sy>>8,sy,ey>>8,ey};
    lspiCmdSend(0x2B,set_y_cmd,sizeof(set_y_cmd));
    lcdWriteCmd(0x2C);
}

static void nv3037StartStop(lcdDev_t* lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static int nv3037Fill(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    uint32_t TotalBytes = (ey-sy+1)*(ex-sx+1)*2;    //RGB565 = uint16
    nv3037AddrSet(lcd,sx,sy,ex,ey);     
    return spidmaTrans(lcd,buf,TotalBytes);
}


static int nv3037DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    nv3037AddrSet(lcd, x, y, x, y);
    return lspiWrite(dataWrite);
}

static void nv3037Clear(lcdDev_t* lcd,  uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    uint32_t dmaTotalLength = lcdHeight * lcdWidth * 2; //RGB565 = uint16 
    nv3037AddrSet(lcd, 0, 0, lcdWidth, lcdHeight);  
    return spidmaTrans(lcd,buf,dmaTotalLength);
}

static void nv3037CamPreviewStartStop(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        // preview
        spiPreviewMode(lcd);
        nv3037AddrSet(lcd, 40, 0, 240+40-1, 240-1);

        mDelay(120); //Delay 120ms
    }
}

lcdDrvFunc_t nv3037Drv = 
{
    .id             = 0x3037,

    .init           = nv3037Init,
    .drawPoint      = nv3037DrawPoint,
    .fill           = nv3037Fill,
    .prepareDisplay = nv3037PrepareDisplay,
    .onoff          = nv3037OnOff,
    .backLight      = nv3037BackLight,
    .startStop      = nv3037StartStop,
    .clear          = nv3037Clear,
    .startStopPreview = nv3037CamPreviewStartStop,
    //.startRamWrite  = nv3037StartRamWrite,
    //.addrSet        = nv3037AddrSet,
};

lcdDrvPra_t nv3037Pra =
{
    .id     = 0x3037,
    .width  = LCD_WIDTH_3037,
    .height = LCD_HEIGHT_3037,
};

#endif