#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_7789.h"

#define LCD_MADCTL  0x36

extern lspiDrvInterface_t *lcdDrv;
//static uint8_t s_MADCTL = 0x0; 

static  initLine_t initTable7789[] = 
{
    {0x11, 1, {0}},
    {0xff, 1, {120}},
    {0x36, 1, {0}},
    {0x21, 1, {0}}, // display inverse
#if (ST7789_BPP == 16)    
    {0x3a, 1, {5}}, // rgb565
#elif (ST7789_BPP == 12)
    {0x3a, 1, {3}}, // rgb444
#elif (ST7789_BPP == 18)    
    {0x3a, 1, {6}}, // rgb666
#endif    
    {0x35, 1, {0}}, // te on
#if (SPI_2_DATA_LANE == 1)
    {0xe7, 1, {0x10}},
#endif    
    {0xb2, 5, {0x0c, 0x0c, 0x00, 0x33, 0x33}}, // rate
    {0xb7, 1, {0x35}},
    {0xbb, 1, {0x20}},
    {0xc0, 1, {0x2c}},
    {0xc2, 1, {0x01}},
    {0xc3, 1, {0x0b}},
    {0xc4, 1, {0x20}},
    {0xc6, 1, {0x0f}},
    {0xd0, 2, {0xa4, 0xa1}}, // pwctr
    {0xe0, 14, {0xd0,0x03,0x09,0x0e,0x11,0x3d,0x47,0x55,0x53,0x1a,0x16,0x14,0x1f,0x22}}, //Positive voltage gamma
    {0xe1, 14, {0xd0,0x02,0x08,0x0d,0x12,0x2c,0x43,0x55,0x53,0x1e,0x1b,0x19,0x20,0x22}}, //Negative voltage gamma
    {0x29, 1, {0x00}},
};

void st7789HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{   

}

void st7789HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
}


static int st7789Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{     
    lcdIoInit(false);
    lcdRst(10, 10);
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdInterfaceType(ST7789_INTERFACE);
    lcdRegInit(0x7789);
    
    #if 0 // test read id
    uint32_t read7789[3] = {0};
    lspiReadReg(0x04, read7789, 3, 0);
    #endif
    return 0;
}

#if 0
static int st7789Direction(lcdDrvFunc_t *lcd,DisDirection_e Dir)
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
        case DIS_DIR_MAX:
        break;
    }
    
    lspiCmdSend(LCD_MADCTL, &s_MADCTL, 1);
    return s_MADCTL;
}
#endif

static void st7789BackLight(lcdDrvFunc_t *lcd, uint8_t level)
{
#if (BK_USE_GPIO == 1)
    lcdGpioBkLevel(level);
#elif (BK_USE_PWM == 1)
    lcdPwmBkLevel(level);
#endif
}

/* 
    sx: start x
    sy: start y
    ex: end x
    ey: end y
*/
static uint32_t st7789AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8, sx, ex>>8, ex};
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
    uint8_t set_y_cmd[] = {sy>>8, sy, ey>>8, ey};
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));
    
    lcdWriteCmd(0x2C);

    return (ey-sy+1) * (ex-sx+1) * (ST7789_BPP/8);
}

static void st7789StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static int st7789Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void st7789DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    lspiFifoWrite(dataWrite);
    return;
}

static void st7789RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = st7789HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = st7789HandleUspIrq4Fill);
}

static void st7789UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}


static void st7789CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    st7789UnRegisterUspIrqCb(lcd, cbForFill);
    st7789RegisterUspIrqCb(lcd, cbForCam);

#if defined CHIP_EC719
    if (previewStartStop)
    {
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)

#elif (CAMERA_ENABLE_GC6153)

#endif

    }

#else // CHIP 718
    if (previewStartStop)
    {
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)
        // preview
        lcdWriteCmd(0x36);
        // lcdWriteData(0x00);// 0: normal;   0x20: reverse, mirror image   0x40: x mirror
        lcdWriteData(0xa0);
        lcdDrv->send(NULL, 0);

        st7789AddrSet(lcd, 0, 0, lcd->height-1, lcd->width-1); // height=320, width=240
        
        lspiDataFmt.wordSize = 7;
        lspiDataFmt.txPack = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        lspiDmaCtrl.txDmaReqEn          = 0; // preview close
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        
        // lcd size
        lspiInfo.frameHeight            = PIC_SRC_HEIGHT; // frame input height
        lspiInfo.frameWidth             = PIC_SRC_WIDTH;  // frame input width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        
        lspiFrameInfoOut.frameHeightOut = lcd->height;    // frame output height
        lspiFrameInfoOut.frameWidthOut  = lcd->width;     // frame output width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        lspiScaleInfo.rowScaleFrac      = 64;
        lspiScaleInfo.colScaleFrac      = 64;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);

        lspiTailorInfo.tailorLeft       = 0; // frame output height
        lspiTailorInfo.tailorRight      = 0; // frame output width
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);

        lspiCtrl.enable                 = 1;
        lspiCtrl.datSrc                 = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn            = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut           = 1; // RGB565
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr               = 1; // start ramwr
        lspiCmdCtrl.dataLen             = 0x3ffff; // infinite data, used in camera to lspi
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        
#elif (CAMERA_ENABLE_GC6153)
        // preview
        lcdWriteCmd(0x36);
        lcdWriteData(0x00);// 0: normal;   0x20: reverse, mirror image   0x40: x mirror
        lcdDrv->send(NULL, 0);
        
        st7789AddrSet(lcd, 0, 0, lcd->width-1, lcd->height-1);

        lspiDataFmt.wordSize            = 7;
        lspiDataFmt.txPack              = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);

        lspiDmaCtrl.txDmaReqEn          = 0; // preview close
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        
        // lcd size
        lspiInfo.frameHeight            = PIC_SRC_HEIGHT; // frame input height
        lspiInfo.frameWidth             = PIC_SRC_WIDTH;  // frame input width
        lspiFrameInfoOut.frameHeightOut = lcd->height;    // frame output height,320
        lspiFrameInfoOut.frameWidthOut  = lcd->width;     // frame output width,240
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lspiTailorInfo.tailorLeft       = 0;
        lspiTailorInfo.tailorRight      = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);

        lspiCtrl.datSrc                 = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn            = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut           = 1; // RGB565
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr               = 1; // start ramwr
        lspiCmdCtrl.dataLen             = 0x3ffff; // infinite data, used in camera to lspi
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
#endif
    }
    else
    {
        lspiDmaCtrl.txDmaReqEn          = 0;
        lspiDataFmt.wordSize	        = 31;
        lspiDataFmt.txPack		        = 0;
        lspiInfo.frameHeight	        = lcd->height;  // frame input height
        lspiInfo.frameWidth		        = lcd->width;   // frame input width
        lspiFrameInfoOut.frameHeightOut = lcd->height;  // frame output height
        lspiFrameInfoOut.frameWidthOut 	= lcd->width;   // frame output width
        lspiCtrl.colorModeIn            = 3; // rgb565
        lspiCtrl.colorModeOut           = 1; // rgb565
        lspiCtrl.datSrc                 = 1; // 0: data from camera; 1: data from memory
        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lspiTailorInfo.tailorLeft       = 0; // frame output height
        lspiTailorInfo.tailorRight      = 0; // frame output width
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);    

        // before send first cmd/data to lcd, clear unnessary status
        if (((LSPI2->STAS >> 27)& 0x1) > 0)
        {
            LSPI2->STAS |= 1<<27;
        }

        if (((LSPI2->STAS >> 28)& 0x1) > 0)
        {
            LSPI2->STAS |= 1<<28;
        }

        if (((LSPI2->STAS >> 13)& 0x3f) > 0)
        {
            LSPI2->DMACTL |= 1<<25;
        }
        
        LSPI2->DMACTL |= 1<<24; // clear rx fifo

        // write first dummy cmd to lcd. It's nessary to clear lspi fifo
        lcdWriteCmd(0xff);
        lcdWriteData(0x00);
        lcdDrv->send(NULL, 0);

        if (((LSPI2->STAS >> 13)& 0x3f) > 0)
        {
            LSPI2->DMACTL |= 1<<25;
        }

        // write lcd 7789 memCtrl reg
        lcdWriteCmd(0x36);
        lcdWriteData(0x00);
        lcdDrv->send(NULL, 0);
    }
#endif    
}

static int st7789Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

lcdDrvFunc_t st7789Drv = 
{
    .id                 = 0x7789,
    .width              = ST7789_WIDTH,
    .height             = ST7789_HEIGHT,
    .freq               = ST7789_FREQ,
    .bpp                = ST7789_BPP,
    .initRegTbl         = initTable7789,
    .initRegTblLen      = sizeof(initTable7789)/sizeof(initLine_t),
    .dir                = 0,

    .init               = st7789Init,
    .drawPoint          = st7789DrawPoint,
    .setWindow          = st7789AddrSet,
    .fill               = st7789Fill,
    .backLight          = st7789BackLight,
    .startStop          = st7789StartStop,
    .startStopPreview   = st7789CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = st7789RegisterUspIrqCb,
    .unregisterUspIrqCb = st7789UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = st7789Close,
};

