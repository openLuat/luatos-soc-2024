#ifdef FEATURE_LCD_ST7789_ENABLE
#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_7789.h"

#define LCD_CHIPID  0x04  
#define LCD_RDDST   0x09 
#define LCD_MADCTL  0x36

extern lspiDrvInterface_t *lcdDrv;

static uint8_t s_MADCTL = 0x0; 
/*
 *  reset lcd driver IC
 *  Parameter: delay ms + hold ms
 */
static void st7789Rst(uint16_t dly_ms,uint16_t hold_ms)
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
static void st7789RegInit(void)
{
    lspiCmdSet(0x11,0x0);   //sleepout
    lspiCmdSet(0x36,s_MADCTL);//MADCTL
    lspiCmdSet(0x21,0x0);//Display Inversion On 
    lspiCmdSet(0x3a,0x5);//RGB565
    #if LCD_SPI_DATA_LANE > 1
    lspiCmdSet(0xE7,0x10);
    #endif

    uint8_t set_rate_cmd[] = {0x0c,0x0c,0x00,0x33,0x33};
    lspiCmdSend(0xb2,set_rate_cmd,sizeof(set_rate_cmd));

    lspiCmdSet(0xb7,0x35);
    lspiCmdSet(0xbb,0x20);
    lspiCmdSet(0xc0,0x2c);
    lspiCmdSet(0xc2,0x01);
    lspiCmdSet(0xc3,0x0b);
    lspiCmdSet(0xc4,0x20);
    lspiCmdSet(0xc6,0x0f);

    uint8_t reg_PWCTRL1[] = {0xa4,0xa1};
    lspiCmdSend(0xd0,reg_PWCTRL1,sizeof(reg_PWCTRL1));
    uint8_t reg_PVGAMCTRL[] = {0xd0,0x03,0x09,0x0e,0x11,0x3d,0x47,0x55,
                                0x53,0x1a,0x16,0x14,0x1f,0x22};  //Positive voltage gamma
    lspiCmdSend(0xe0,reg_PVGAMCTRL,sizeof(reg_PVGAMCTRL));
    uint8_t reg_NVGAMCTRL[] = {0xd0,0x02,0x08,0x0d,0x12,0x2c,0x43,0x55,
                                0x53,0x1e,0x1b,0x19,0x20,0x22};  //Negative voltage gamma
    lspiCmdSend(0xe1,reg_NVGAMCTRL,sizeof(reg_NVGAMCTRL));
    lspiCmdSet(0x29,0x0);
}

/*
 *  st7789 init entry 
 *  Parameter: callback function
 */
static int st7789Init(lcdDev_t *lcd,void* cb)
{   
    lcdGpioInit(lcd);
    dmaInit(NULL);
    st7789Rst(10,1);
    lcdSpiInit(LCD_HEIGHT_7789,LCD_WIDTH_7789,cb);
    lcdInterfaceType(SPI_4W_II);
    lcdColorMode(DIS_FMT_RGB565);
    st7789RegInit();
    return 0;
}

/**
  \fn          
  \brief        
  \return
*/
static int st7789Direction(lcdDev_t* lcd,DisDirection_e Dir)
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
static uint16_t st7789Check(lcdDev_t* lcd)
{
    // uint32_t buffer[2]={0};
    // lspiReadReg(LCD_CHIPID,buffer,2);
    // printf("0x%X,0x%X,0x%X\r\n",buffer[0],buffer[1],buffer[2]);
    // if(buffer[0]==0x77 && buffer[1]==0x89 ){
    //     return 0x7789;
    // }
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static int st7789PrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static void st7789OnOff(lcdDev_t* lcd,bool sta)
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
static int16_t st7789BackLight(lcdDev_t* lcd,uint16_t level)
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
static void st7789AddrSet(lcdDev_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8,sx,ex>>8,ex};
    lspiCmdSend(0x2A,set_x_cmd,sizeof(set_x_cmd));
    uint8_t set_y_cmd[] = {sy>>8,sy,ey>>8,ey};
    lspiCmdSend(0x2B,set_y_cmd,sizeof(set_y_cmd));
    lcdWriteCmd(0x2C);
}
/**
  \fn          
  \brief        
  \return
*/
static void st7789StartStop(lcdDev_t* lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}
/**
  \fn          
  \brief        
  \return
*/
static int st7789Fill(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    uint32_t TotalBytes = (ey-sy+1)*(ex-sx+1)*2;    //RGB565 = uint16
    st7789AddrSet(lcd,sx,sy,ex,ey);     
    return spidmaTrans(lcd,buf,TotalBytes);
}

/**
  \fn          
  \brief        
  \return
*/
static int st7789DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    st7789AddrSet(lcd, x, y, x, y);
    return lspiWrite(dataWrite);
}
/**
  \fn          
  \brief        
  \return
*/
static void st7789Clear(lcdDev_t* lcd,  uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    uint32_t dmaTotalLength = lcdHeight * lcdWidth * 2; //RGB565 = uint16 
    st7789AddrSet(lcd, 0, 0, lcdWidth, lcdHeight);  
    return spidmaTrans(lcd,buf,dmaTotalLength);
}
/**
  \fn          
  \brief        
  \return
*/
static void st7789CamPreviewStartStop(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        // preview
        lcdWriteCmd(0x36);
        // lcdWriteData(0x00);// 0: normal;   0x20: reverse, mirror image   0x40: x mirror
        lcdWriteData(0xa0);
        lcdDrv->send(NULL, 0);
        
        lcdWriteCmd(0x2C);
        st7789AddrSet(lcd, 0, 0, 320-1, 240-1);
        // st7789AddrSet(lcd, 0, 0, 240-1, 320-1);

        lspiDataFmt.wordSize = 7;
        lspiDataFmt.txPack = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight = 480; // frame input height
        lspiInfo.frameWidth = 640; // frame input width
        // lspiInfo.frameHeight = 320; // frame input height
        // lspiInfo.frameWidth = 240; // frame input width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        
        lspiFrameInfoOut.frameHeightOut =320;//lcd->pra->height;//320; // frame output height
        lspiFrameInfoOut.frameWidthOut = 240;//lcd->pra->width;//240; // frame output width

        // lspiFrameInfoOut.frameHeightOut =320;//lcd->pra->height;//320; // frame output height
        // lspiFrameInfoOut.frameWidthOut = 240;//lcd->pra->width;//240; // frame output width        
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        // lspiScaleInfo.rowScaleFrac =128;
        // lspiScaleInfo.colScaleFrac = 128;

        lspiScaleInfo.rowScaleFrac =64;
        lspiScaleInfo.colScaleFrac = 64;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);

        lspiTailorInfo.tailorLeft = 0; // frame output height
        lspiTailorInfo.tailorRight = 0; // frame output width
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);

        lspiCtrl.enable = 1;
        lspiCtrl.line4 = 1;
        lspiCtrl.datSrc = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut = 1; // RGB565
        lspiCtrl.busType = 1; // Interface II
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr = 1; // start ramwr
        lspiCmdCtrl.dataLen = 0x3ffff; // infinite data, used in camera to lspi
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);

        spiPreviewMode(lcd);
        // st7789AddrSet(lcd, 40, 0, 240+40-1, 240-1);

        mDelay (120); //Delay 120ms
    }
}

/**
  \fn          
  \brief        
  \return
*/
static int st7789Close(lcdDev_t* lcd)
{

}
/**
  \fn          
  \brief        
  \return
*/
lcdDrvFunc_t st7789Drv = 
{
    .id             = 0x7789,

    .init           = st7789Init,
    .drawPoint      = st7789DrawPoint,
    .fill           = st7789Fill,
    .prepareDisplay = st7789PrepareDisplay,
    .onoff          = st7789OnOff,
    .backLight      = st7789BackLight,
    .startStop      = st7789StartStop,
    .clear          = st7789Clear,
    .startStopPreview = st7789CamPreviewStartStop,
    .close          = st7789Close,
    //.startRamWrite  = st7789StartRamWrite,
    //.addrSet        = st7789AddrSet,
};

lcdDrvPra_t st7789Pra =
{
    .id     = 0x7789,
    .width  = LCD_WIDTH_7789,
    .height = LCD_HEIGHT_7789,
};

#endif