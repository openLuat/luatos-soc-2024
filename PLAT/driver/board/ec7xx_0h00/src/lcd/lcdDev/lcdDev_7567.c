#ifdef FEATURE_LCD_ST7567_ENABLE
#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_7567.h"

/*
 *  reset lcd driver IC
 *  Parameter: delay ms + hold ms
 */
static void st7567Rst(uint16_t dly_ms,uint16_t hold_ms)
{   
    if(dly_ms){
        LCD_RST_LOW;
        mDelay(dly_ms); 
    }
    LCD_RST_HIGH;
    mDelay(hold_ms); 
}

/*
 *  Write Configuration to Driver Chip Registers
 *  Parameter: none
 */
static void st7567RegInit(void)
{ 
	lcdWriteCmd(0xE2);
    lcdDrv->send(NULL, 0);
    
    lcdWriteCmd(0xA2);
    lcdDrv->send(NULL, 0);

	lcdWriteCmd(0xA0);	
    lcdDrv->send(NULL, 0);

    lcdWriteCmd(0xA7);	
    lcdDrv->send(NULL, 0);

	lcdWriteCmd(0xC0);	
    lcdDrv->send(NULL, 0);

	lcdWriteCmd(0xA4);	
    lcdDrv->send(NULL, 0);  

	lcdWriteCmd(0x40);	
    lcdDrv->send(NULL, 0);   

	lcdWriteCmd(0x24);	
    lcdDrv->send(NULL, 0);   

	lcdWriteCmd(0x81);	
    lcdDrv->send(NULL, 0);   

	lcdWriteCmd(0x2C);	
    lcdDrv->send(NULL, 0);   

	lcdWriteCmd(0xF8);	
    lcdDrv->send(NULL, 0);   
    
	lcdWriteCmd(0x00);	
    lcdDrv->send(NULL, 0);   

	lcdWriteCmd(0x2C);	
    lcdDrv->send(NULL, 0);  
    mDelay(50); //Delay 100ms

	lcdWriteCmd(0x2E);
    lcdDrv->send(NULL, 0); 
    mDelay(50); //Delay 100ms
    
	lcdWriteCmd(0x2F);
    lcdDrv->send(NULL, 0);   
    mDelay(50); //Delay 100ms

    lcdWriteCmd(0xAF);
    // lcdDrv->send(NULL, 0);
    // lspiCmdCtrl.wrRdn = 1;
    // lspiCmdCtrl.ramWr = 1;
    // lspiCmdCtrl.dataLen = 0x3ffff;
    // lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    // mDelay(50); //Delay 100ms
}

/*
 *  st7567 init entry 
 *  Parameter: callback function
 */
static int st7567Init(lcdDev_t *lcd,void* cb)
{   
    lcdGpioInit(lcd);
    // dmaInit(NULL);
    st7567Rst(50,150);
    lcdSpiInit(LCD_HEIGHT_7567,LCD_WIDTH_7567,cb);
    lcdInterfaceType(SPI_4W_II);
    lcdColorMode(DIS_FMT_GRAY_1);
    lspiInfo.frameHeight = 69; // frame input height
    lspiInfo.frameWidth = 92; // frame input width
    lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);

    lspiFrameInfoOut.frameHeightOut = 64; // frame output height
    lspiFrameInfoOut.frameWidthOut = 92; // frame output width
    lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

    lspiQuartileCtrl.grayCtrl = 2; // Histogram equalization
    lcdDrv->ctrl(LSPI_CTRL_QUARTILE_CTRL, 0);

    lspiTailorInfo.tailorLeft = 0; // cut left columns
    lspiTailorInfo.tailorRight = 0; // cut right columns
    lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);

    lspiTailorInfo0.tailorBottom = 2; // cut bottom lines
    lspiTailorInfo0.tailorTop = 3; // cut top lines
    lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);

    lspiGrayPageCmd0.pageCmd = 0xb0; // start page 0, page cmd should be fixed with 0xb0+page_num
    lspiGrayPageCmd0.pageCmd0 = 0x10; // command followed with page_cmd. column_num high bits
    lspiGrayPageCmd0.pageCmd01ByteNum = 2; // page_cmd0+page_cmd1   byte num
    lcdDrv->ctrl(LSPI_CTRL_GRAY_PAGE_CMD0, 0);

    lspiGrayPageCmd1.pageCmd1 = 0; // followed with page_cmd0. column_num low bits
    lcdDrv->ctrl(LSPI_CTRL_GRAY_PAGE_CMD1, 0); 
    st7567RegInit();

    lspiCmdCtrl.wrRdn = 1;
    lspiCmdCtrl.ramWr = 1;
    lspiCmdCtrl.dataLen = 0x3ffff;
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    mDelay(50); //Delay 100ms
    return 0;
}


/**
  \fn          
  \brief        
  \return
*/
static uint16_t st7567Check(lcdDev_t* lcd)
{
    // uint32_t buffer[2]={0};
    // lspiReadReg(LCD_CHIPID,buffer,2);
    // printf("0x%X,0x%X,0x%X\r\n",buffer[0],buffer[1],buffer[2]);
    // if(buffer[0]==0x77 && buffer[1]==0x89 ){
    //     return 0x7567;
    // }
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static int st7567PrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}
/**
  \fn          
  \brief        
  \return
*/
static void st7567OnOff(lcdDev_t* lcd,bool sta)
{
    #ifdef LCD_BL_PWM_ENABLE
    lcdBackLightOn(sta);
    #endif
}
/**
  \fn          
  \brief        
  \return
*/
static int16_t st7567BackLight(lcdDev_t* lcd,uint16_t level)
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
static void st7567AddrSet(lcdDev_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
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
static void st7567StartStop(lcdDev_t* lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}
/**
  \fn          
  \brief        
  \return
*/
static int st7567Fill(lcdDev_t* lcd,uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    // uint32_t TotalBytes = (ey-sy+1)*(ex-sx+1)*2;    //RGB565 = uint16
    // st7567AddrSet(lcd,sx,sy,ex,ey);     
    // return spidmaTrans(lcd,buf,TotalBytes);
}

/**
  \fn          
  \brief        
  \return
*/
static int st7567DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    // st7567AddrSet(lcd, x, y, x, y);
    // return lspiWrite(dataWrite);
}
/**
  \fn          
  \brief        
  \return
*/
static void st7567Clear(lcdDev_t* lcd,  uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    uint32_t dmaTotalLength = lcdHeight * lcdWidth * 2; //RGB565 = uint16 
    st7567AddrSet(lcd, 0, 0, lcdWidth, lcdHeight);  
    return spidmaTrans(lcd,buf,dmaTotalLength);
}


/**
  \fn          
  \brief        
  \return
*/
static int st7567Close(lcdDev_t* lcd)
{

}
/**
  \fn          
  \brief        
  \return
*/
lcdDrvFunc_t st7567Drv = 
{
    .id             = 0x7567,

    .init           = st7567Init,
    .drawPoint      = st7567DrawPoint,
    .fill           = st7567Fill,
    .prepareDisplay = st7567PrepareDisplay,
    .onoff          = st7567OnOff,
    .backLight      = st7567BackLight,
    .startStop      = st7567StartStop,
    .clear          = st7567Clear,
    .close          = st7567Close,
};
/**
  \fn          
  \brief        
  \return
*/
lcdDrvPra_t st7567Pra =
{
    .id     = 0x7567,
    .width  = LCD_WIDTH_7567,
    .height = LCD_HEIGHT_7567,
};

#endif