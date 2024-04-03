
#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_9307.h"

/*
 *  reset lcd driver IC
 *  Parameter: delay ms + hold ms
 */
static void gc9307Rst(uint16_t dly_ms,uint16_t hold_ms)
{   
#ifdef LCD_RST_GPIO_PIN
    if(dly_ms){
        LCD_RST_LOW;
        mDelay(dly_ms); 
    }
    LCD_RST_HIGH;
    mDelay(hold_ms); 
#endif
}

/*
 *  Write Configuration to Driver Chip Registers
 *  Parameter: none
 */
static void gc9307RegInit(void)
{
    lspiCmdSet(0xfe, 0);
    lspiCmdSet(0xef, 0);
    
    lspiCmdSet(0x36,0x40);      // 0x48
    lspiCmdSet(0x3a,0x05);      // 0x05
    lspiCmdSet(0x86,0x98);      //c9/c3/c7 en
    lspiCmdSet(0x89,0x01);      //0x03 --> e8/e9 en
    lspiCmdSet(0x8b,0x80);
    lspiCmdSet(0x8d,0x33);
    lspiCmdSet(0x8e,0x0f);

    uint8_t tbuf[] = {0x12, 0x00};
    lspiCmdSend(0xe8, tbuf, sizeof(tbuf));
    
    lspiCmdSet(0xc3,0x47);
    lspiCmdSet(0xc4,0x28);
    lspiCmdSet(0xc9,0x08);
    lspiCmdSet(0xff,0x62);
    lspiCmdSet(0x99,0x3e);
    lspiCmdSet(0x9d,0x4b);

    lspiCmdSet(0x98,0x3e);
    lspiCmdSet(0x9c,0x4b);

    uint8_t tbuf0[] = {0x07, 0x0b, 0x0c, 0x0a, 0x06, 0x31};
    lspiCmdSend(0xF0, tbuf0, sizeof(tbuf0));

    uint8_t tbuf2[] = {0x07, 0x07, 0x04, 0x06, 0x06, 0x21};
    lspiCmdSend(0xF2, tbuf2, sizeof(tbuf2));

    uint8_t tbuf1[] = {0x4a, 0x78, 0x76, 0x33, 0x2f, 0xaf};
    lspiCmdSend(0xF1, tbuf1, sizeof(tbuf1));

    uint8_t tbuf3[] = {0x38, 0x74, 0x72, 0x22, 0x28, 0x6f};
    lspiCmdSend(0xF3, tbuf3, sizeof(tbuf3));
    
    //lspiCmdSet(0xe9,0x00);  // 0x08 = 2 data lane, 0x00 = 1 data lane
    lspiCmdSet(0x35,0x00);

    uint8_t xbuf[] = {0x00, 0x0a};
    lspiCmdSend(0x44, xbuf, sizeof(xbuf));

    lspiCmdSet(0x11, 0);
    mDelay(120);
    lspiCmdSet(0x29, 0);
    lspiCmdSet(0x2c, 0);
}

static int gc9307Area(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8,sx,ex>>8,ex};
    lspiCmdSend(0x2A,set_x_cmd,sizeof(set_x_cmd));
    uint8_t set_y_cmd[] = {sy>>8,sy,ey>>8,ey};
    lspiCmdSend(0x2B,set_y_cmd,sizeof(set_y_cmd));
    lcdWriteCmd(0x2C);
    return 0;
}

static int gc9307Fill(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, void *buf)
{
    uint32_t TotalBytes = (ey-sy+1)*(ex-sx+1)*2;    //RGB565 = uint16
    gc9307Area(lcd,sx,sy,ex,ey);     
    return spidmaTrans(lcd,buf,TotalBytes);
}

/*
 *  Parameter: callback function
 */

static int gc9307Init(lcdDev_t *lcd,void* cb)
{   
    lcdGpioInit(lcd);
    dmaInit(NULL);
    gc9307Rst(50,120);
    lcdSpiInit(LCD_HEIGHT_9307,LCD_WIDTH_9307,cb);
    lcdInterfaceType(SPI_4W_II);
    lcdColorMode(DIS_FMT_RGB565);
    gc9307RegInit();
    return 0;
}

/**
  \fn          
  \brief        
  \return
*/
static int gc9307DeInit(lcdDev_t* lcd)
{
    // lcdBusDeInit();
    // lcdPwmDeInit();
    // LSPI_RST_LOW;
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static void gc9307OnOff(lcdDev_t* lcd,bool sta)
{
    #ifdef LCD_BL_PWM_ENABLE
    lcdBackLightOn(sta);
    #else
    lcdbkInit();
    #endif
}
/**
  \fn          
  \brief        
  \return
*/
static int16_t gc9307BackLight(lcdDev_t* lcd,uint16_t level)
{
    #ifdef LCD_BL_PWM_ENABLE
    return lcdBackLightLevel(level);
    #endif
}

lcdDrvFunc_t gc9307Drv = 
{
    .id             = 0x9307,
    .init           = gc9307Init,
    .fill           = gc9307Fill,
    // .area           = gc9307Area,
    .onoff          = gc9307OnOff,
    // .deinit         = gc9307DeInit,
    .backLight      = gc9307BackLight,
    .startStopPreview = NULL,
};

lcdDrvPra_t gc9307Pra =
{
    .id         = 0x9307,
    .width      = LCD_WIDTH_9307,
    .height     = LCD_HEIGHT_9307,
};
