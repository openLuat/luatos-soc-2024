#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_9307.h"

#define LCD_CHIPID  0x04  
#define LCD_RDDST   0x09 
#define LCD_MADCTL  0x36

extern lspiDrvInterface_t *lcdDrv;
static uint8_t s_MADCTL = 0x0; 

static initLine_t initTable9307[] = 
{
    {0xfe, 1, {0}},
    {0xef, 1, {0}},
    {0x36, 1, {0x40}},
    {0x3a, 1, {0x05}},
    {0x86, 1, {0x98}},     
    {0x89, 1, {0x01}},
    {0x8b, 1, {0x80}},
    {0x8d, 1, {0x33}},
    {0x8e, 1, {0x0f}},
    {0xe8, 2, {0x12, 0x00}},
    {0xc3, 1, {0x47}},      
    {0xc4, 1, {0x28}},
    {0xc9, 1, {0x08}},
    {0xff, 1, {0x62}},
    {0x99, 1, {0x3e}},
    {0x9d, 1, {0x4b}},
    {0x98, 1, {0x3e}},
    {0x9c, 1, {0x4b}},
    {0xf0, 6, {0x07, 0x0b, 0x0c, 0x0a, 0x06, 0x31}},
    {0xf2, 6, {0x07, 0x07, 0x04, 0x06, 0x06, 0x21}},
    {0xf1, 6, {0x4a, 0x78, 0x76, 0x33, 0x2f, 0xaf}},
    {0xf3, 6, {0x38, 0x74, 0x72, 0x22, 0x28, 0x6f}},
    {0x35, 1, {0x00}},
    {0x44, 2, {0x00, 0x0a}},
    {0x11, 1, {0}},
    {0x29, 1, {0}},
};

void st9307HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{   

}

void st9307HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
}

uint8_t read9307[20] = {0x1, 0x2, 0x3};
static int st9307Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{     
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdIoInit(false);
    lcdRst(100, 100);
    lcdInterfaceType(GC9307_INTERFACE);
    lcdRegInit(0x9307);
    
#if 1 // test read
    lspiReadReg(0xda, read9307,   1, 0);
    lspiReadReg(0xdb, read9307+1, 1, 0);
    lspiReadReg(0xdc, read9307+2, 1, 0);
    lspiReadReg(0x04, read9307+3, 4, 9);
#endif

    return 0;
}

static int st9307Direction(lcdDrvFunc_t *lcd,DisDirection_e Dir)
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
    
    lspiCmdSend(LCD_MADCTL, &s_MADCTL, 1);
    return s_MADCTL;
}

static void st9307BackLight(lcdDrvFunc_t *lcd, uint8_t level)
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
static uint32_t st9307AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8, sx, ex>>8, ex};
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
    uint8_t set_y_cmd[] = {sy>>8, sy, ey>>8, ey};
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));
    
    lcdWriteCmd(0x2C);

    return (ey-sy+1) * (ex-sx+1) * (GC9307_BPP/8);
}

static void st9307StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static int st9307Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void st9307DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    st9307AddrSet(lcd, x, y, x, y);
    lspiFifoWrite(dataWrite);
    return;
}

static void st9307RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }
    
    (who == cbForCam)? (lcd->uspIrq4CamCb = st9307HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = st9307HandleUspIrq4Fill);
}

static void st9307UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}


static void st9307CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    st9307UnRegisterUspIrqCb(lcd, cbForFill);
    st9307RegisterUspIrqCb(lcd, cbForCam);

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

        st9307AddrSet(lcd, 0, 0, lcd->height-1, lcd->width-1); // height=320, width=240
        
        lspiDataFmt.wordSize            = 7;
        lspiDataFmt.txPack              = 0;
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
        
        st9307AddrSet(lcd, 0, 0, lcd->width-1, lcd->height-1);

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
#endif    
}

static int st9307Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

lcdDrvFunc_t gc9307Drv = 
{
    .id                 = 0x9307,
    .width              = GC9307_WIDTH,
    .height             = GC9307_HEIGHT,
    .freq               = GC9307_FREQ,
    .bpp                = GC9307_BPP,
    .initRegTbl         = initTable9307,
    .initRegTblLen      = sizeof(initTable9307)/sizeof(initLine_t),
    .dir                = 0,

    .init               = st9307Init,
    .drawPoint          = st9307DrawPoint,
    .setWindow          = st9307AddrSet,
    .fill               = st9307Fill,
    .backLight          = st9307BackLight,
    .startStop          = st9307StartStop,
    .startStopPreview   = st9307CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = st9307RegisterUspIrqCb,
    .unregisterUspIrqCb = st9307UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = st9307Close,
};

