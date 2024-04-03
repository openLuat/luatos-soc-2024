#ifdef FEATURE_LCD_ST7796_ENABLE
#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_7796.h"

#define ST7796_RESOLUTION_HOR 320
#define ST7796_RESOLUTION_VER 480

#define LCD_CHIPID  0xD3  //4B:0x00 0x77 0x96
#define LCD_RDDST   0x09  //5B:Read Display Status 
/** commands of ST7796 */
#define LCD_SWRESET 0x01  // Software Reset
#define LCD_RDDID   0x04  // Read Display ID
#define LCD_INVOFF  0x20  // Display Inversion Off
#define LCD_INVON   0x21  // Display Inversion On
#define LCD_CASET   0x2A  // Column Address Set
#define LCD_PASET   0x2B  // Row Address Set
#define LCD_RAMWR   0x2C  // Memory Writ
#define LCD_RAMRD   0x2E  // Memory Read
#define LCD_MADCTL  0x36  // Memory Data Access Control

static uint8_t s_MADCTL = 0x40;

/*
 *  reset lcd driver IC
 *  Parameter: delay ms + hold ms
 */
static void st7796Rst(uint16_t dly_ms,uint16_t hold_ms)
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
static void st7796RegInit(void)
{
    lspiCmdSet(0x36,s_MADCTL);//MADCTL
    lspiCmdSet(0x21,0x0);//Display Inversion On 
    lspiCmdSet(0x3a,0x5);//RGB565
    lspiCmdSet(0xFB,0x10);//SPI Read Control
    // uint8_t set_rate_cmd[] = {0x0c,0x0c,0x00,0x33,0x33};
    // lspiCmdSend(0xb2,set_rate_cmd,sizeof(set_rate_cmd));
    // lspiCmdSet(0xb7,0x35);// Entry Mode Set 

    // lspiCmdSet(0xbb,0x20);

    lspiCmdSet(0xc0,0x2c);

    lspiCmdSet(0xc2,0x01);

    lspiCmdSet(0xc3,0x0b);

    lspiCmdSet(0xc4,0x20);

    lspiCmdSet(0xc6,0x0f);
    lspiCmdSet(0x13,0x0);       //norm disp
    lspiCmdSet(0x11,0x0);       //sleepout
    lspiCmdSet(0x29,0x0);       //DISPON
    // lspiCmdSet(0x28,0x0);    //DISPOFF
}

/*
 *  st7796 init entry 
 *  Parameter: callback function
 */
static int st7796Init(lcdDev_t *lcd, lspiCbEvent_fn cb)
{   
    lcdGpioInit(lcd);
    dmaInit(NULL);
    st7796Rst(10,1);
    lcdSpiInit(LCD_HEIGHT_7796,LCD_WIDTH_7796,cb);
    lcdInterfaceType(SPI_3W_II);
    lcdColorMode(DIS_FMT_RGB565);
    st7796RegInit();
    return 0;
}

/**
  \fn          
  \brief        
  \return
*/
/* MADCTL Defines */
#define MADCTL_MY  0x80
#define MADCTL_MX  0x40
#define MADCTL_MV  0x20
#define MADCTL_ML  0x10
#define MADCTL_RGB 0x08
#define MADCTL_MH  0x04
static int st7796Direction(lcdDev_t* lcd,DisDirection_e Dir)
{
    switch(Dir)
    {
        case DIS_MIRROR_X:  
            s_MADCTL ^= BIT(6);
            break;
        case DIS_MIRROR_Y:  
            s_MADCTL ^= BIT(7);
            break;   
        case DIS_SWAP_XY:   
            s_MADCTL ^= 0xC0;
            break; 
        case DIS_DIR_LRTB: 
            s_MADCTL &= ~(BIT(2));   
            s_MADCTL &= ~(BIT(4)); 
            s_MADCTL &= ~(BIT(5));
            break;
        case DIS_DIR_LRBT: 
            s_MADCTL &= ~(BIT(2)); 
            s_MADCTL &= ~(BIT(5)); 
            s_MADCTL |= BIT(4);
            break;   
        case DIS_DIR_RLTB:   
            s_MADCTL &= ~(BIT(4)); 
            s_MADCTL &= ~(BIT(5)); 
            s_MADCTL |= BIT(2);
            break; 
        case DIS_DIR_RLBT: 
            s_MADCTL &= ~(BIT(5));
            s_MADCTL |=  BIT(4);  
            s_MADCTL |=  BIT(2); 
            break; 
        case DIS_DIR_TBLR: 
            s_MADCTL &= ~(BIT(2)); 
            s_MADCTL &= ~(BIT(4));
            s_MADCTL |= BIT(5);  
            break; 
        case DIS_DIR_BTLR:  
            s_MADCTL &= ~(BIT(2));
            s_MADCTL |= BIT(4); 
            s_MADCTL |= BIT(5);   
            break; 
        case DIS_DIR_TBRL:  
            s_MADCTL &= ~(BIT(4));
            s_MADCTL |= BIT(2); 
            s_MADCTL |= BIT(5);
            break; 
        case DIS_DIR_BTRL: 
            s_MADCTL |= BIT(2); 
            s_MADCTL |= BIT(4); 
            s_MADCTL |= BIT(5);  
            break; 
    }
    lspiCmdSet(LCD_MADCTL,s_MADCTL);
    return s_MADCTL;
}

/**
  \fn          
  \brief        
  \return
*/
static int st7796Check(lcdDev_t* lcd)
{
    uint32_t buffer[2]={0};
    // lspiCmdSet(0xF0,0xC3); 
    // lspiCmdSet(0xF0,0x96);
    lspiReadReg(LCD_CHIPID,buffer,2);
    // lspiCmdSet(0xF0,0x3C); 
    // lspiCmdSet(0xF0,0x69);
    // printf("0x%X,0x%X,0x%X\r\n",buffer[0],buffer[1],buffer[2]);
    if(buffer[0]==0x77 && buffer[1]==0x96 ){
        return 0x7796;
    }
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static int st7796PrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static void st7796OnOff(lcdDev_t* lcd,bool sta)
{
    #ifdef LCD_BL_PWM_ENABLE
    lcdBackLightOn(sta);
    #endif
}

static int16_t st7796BackLight(lcdDev_t* lcd,uint16_t level)
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
static void st7796AddrSet(lcdDev_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8,sx,ex>>8,ex};
    lspiCmdSend(0x2A,set_x_cmd,sizeof(set_x_cmd));
    uint8_t set_y_cmd[] = {sy>>8,sy,ey>>8,ey};
    lspiCmdSend(0x2B,set_y_cmd,sizeof(set_y_cmd));
    lcdWriteCmd(0x2C);
}

static void st7796StartStop(lcdDev_t* lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}
/**
  \fn          
  \brief        
  \return
*/
static int st7796Fill(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    uint32_t TotalBytes = (ey-sy+1)*(ex-sx+1)*2;    //RGB565 = uint16
    st7796AddrSet(lcd,sx,sy,ex,ey); 
    // lcdWriteCmd(0x3C);    
    return spidmaTrans(lcd,buf,TotalBytes);
}
/**
  \fn          
  \brief        
  \return
*/
static int st7796Dump(lcdDev_t* lcd,uint16_t sx, uint16_t sy,uint32_t *buf, uint32_t Length)
{
    // uint8_t set_x_cmd[] = {sx>>8,sx,ex>>8,ex};
    // lspiCmdSend(0x2A,set_x_cmd,sizeof(set_x_cmd));
    // uint8_t set_y_cmd[] = {sy>>8,sy,ey>>8,ey};
    // lspiCmdSend(0x2B,set_y_cmd,sizeof(set_y_cmd));
    lspiReadRam(buf,Length);
    printf("\r\n[%d]:",Length);
    for (int m = 0; m < Length; m++)
    {
        printf("0x%X ",buf[m]);
    } 
}

static int st7796DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    st7796AddrSet(lcd, x, y, x, y);
    // lcdWriteCmd(0x2C);
    return lspiWrite(dataWrite);
}

static void st7796Clear(lcdDev_t* lcd,  uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    uint32_t dmaTotalLength = lcdHeight * lcdWidth * 2; //RGB565 = uint16 
    st7796AddrSet(lcd, 0, 0, lcdWidth, lcdHeight);  
    // lcdWriteCmd(0x2C);
    return spidmaTrans(lcd,buf,dmaTotalLength);
}

static void st7796CamPreviewStartStop(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        // preview
        spiPreviewMode(lcd);
        st7796AddrSet(lcd, 40, 0, 240+40-1, 240-1);
        // lcdWriteCmd(0x2C);
        mDelay (120); //Delay 120ms
    }
}

lcdDrvFunc_t st7796Drv = 
{
    .id             = 0x7796,

    .init           = st7796Init,
    .drawPoint      = st7796DrawPoint,
    .fill           = st7796Fill,
    .dump           = st7796Dump,
    .prepareDisplay = st7796PrepareDisplay,
    .onoff          = st7796OnOff,
    .backLight      = st7796BackLight,
    .startStop      = st7796StartStop,
    .clear          = st7796Clear,
    .startStopPreview = st7796CamPreviewStartStop,
    .check          = st7796Check,
    .direction      = st7796Direction,
    //.startRamWrite  = st7796StartRamWrite,
    //.addrSet        = st7796AddrSet,
};

lcdDrvPra_t st7796Pra =
{
    .id     = 0x7796,
    .width  = LCD_WIDTH_7796,
    .height = LCD_HEIGHT_7796,
};

#endif