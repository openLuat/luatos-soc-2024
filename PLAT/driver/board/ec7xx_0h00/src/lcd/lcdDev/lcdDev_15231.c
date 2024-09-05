#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_15231.h"

#define LCD_MADCTL  0x36


extern lspiDrvInterface_t *lcdDrv;
//static uint8_t s_MADCTL = 0x0; 
static uint16_t previewWidth;
static uint16_t previewHeight;

static initLine_t initTable15231[] = 
{
#if 1
    {0xBB, 8,  {0x00,0x00,0x00,0x00,0x00,0x00,0x5A,0xA5}},
    {0xA0, 17, {0xC0,0x10,0x00,0x02,0x00,0x00,0x64,0x3F,0x20,0x05,0x3F,0x3F,0x00,0x00,0x00,0x00,0x00}},
    {0xA2, 31, {0x31,0x3C,0x29,0x14,0xFF,0xA6,0xCB,0xE0,0x40,0x19,0x80,0x80,0x80,0x20,0xf9,0x10,0x02,0xff,0xff,0xF0,0x90,0x01,0x32,0xA0,0x91,0x40,0x20,0x7F,0xFF,0x00,0x14}},
    {0xD0, 30, {0xE0,0x40,0x51,0x24,0x08,0x05,0x10,0x01,0x04,0x14,0xC2,0x42,0x22,0x22,0xAA,0x03,0x10,0x12,0x60,0x14,0x1E,0x51,0x15,0x00,0xFF,0x10,0x00,0x03,0x3D,0x12}},
    {0xA3, 22, {0xA0,0x06,0xAa,0x00,0x08,0x02,0x0A,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x04,0x00,0x55,0x55}},
    {0xA4, 16, {0x85,0x85,0x95,0x82,0xAF,0xED,0xED,0x80,0x10,0x30,0x40,0x40,0x20,0x50,0x60,0x35}},
    {0xC1, 30, {0x33,0x04,0x02,0x02,0x71,0x05,0x27,0x55,0x02,0x00,0x41,0x00,0x53,0xFF,0xFF,0xFF,0x4F,0x52,0x00,0x4F,0x52,0x00,0x45,0x3B,0x0B,0x02,0x0D,0x00,0xFF,0x40}},
    {0xC3, 11, {0x00,0x00,0x00,0x50,0x03,0x00,0x00,0x00,0x01,0x80,0x01}},
    {0xC4, 29, {0x00,0x24,0x33,0x80,0x00,0xea,0x64,0x32,0xC8,0x64,0xC8,0x32,0x90,0x90,0x11,0x06,0xDC,0xFA,0x00,0x00,0x80,0xFE,0x10,0x10,0x00,0x0A,0x0A,0x44,0x50}},
    {0xC5, 23, {0x18,0x00,0x00,0x03,0xFE,0x50,0x38,0x20,0x30,0x10,0x88,0xDE,0x0D,0x08,0x0F,0x0F,0x01,0x50,0x38,0x20,0x10,0x10,0x00}},
    {0xC6, 20, {0x05,0x0A,0x05,0x0A,0x00,0xE0,0x2E,0x0B,0x12,0x22,0x12,0x22,0x01,0x03,0x00,0x02,0x6A,0x18,0xC8,0x22}},
    {0xC7, 27, {0x50,0x32,0x28,0x00,0xa2,0x80,0x8f,0x00,0x80,0xff,0x07,0x11,0x9c,0x67,0xff,0x28,0x0c,0x0d,0x0e,0x0f,0x01,0x01,0x01,0x01,0x3F,0x07,0xFF}},
    {0xC9, 4,  {0x33,0x44,0x44,0x01}},
    {0xCF, 27, {0x2C,0x1E,0x88,0x58,0x13,0x18,0x56,0x18,0x1E,0x68,0x88,0x00,0x65,0x09,0x22,0xC4,0x0C,0x77,0x22,0x44,0xAA,0x55,0x08,0x08,0x12,0xA0,0x08}},
    {0xD5, 30, {0x38,0x38,0x89,0x01,0x35,0x04,0x92,0x6F,0x04,0x92,0x6F,0x04,0x08,0x6A,0x04,0x46,0x03,0x03,0x03,0x03,0x00,0x01,0x03,0x00,0xE0,0x51,0xa1,0x00,0x00,0x00}},
    {0xD6, 30, {0x10,0x32,0x54,0x76,0x98,0xBA,0xDC,0xFE,0x93,0x00,0x01,0x83,0x07,0x07,0x00,0x07,0x07,0x00,0x03,0x03,0x03,0x03,0x03,0x03,0x00,0x84,0x00,0x20,0x01,0x00}},
    {0xD7, 19, {0x03,0x01,0x0b,0x09,0x0f,0x0d,0x1E,0x1F,0x18,0x1d,0x1f,0x19,0x38,0x38,0x04,0x00,0x1d,0x40,0x1F}},
    {0xD8, 12, {0x02,0x00,0x0a,0x08,0x0e,0x0c,0x1E,0x1F,0x18,0x1d,0x1f,0x19}},
    {0xD9, 12, {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
    {0xDD, 12, {0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F,0x1F}},
    {0xDF, 8,  {0x44,0x73,0x4B,0x69,0x00,0x0A,0x02,0x90}},
    {0xE0, 17, {0x3B,0x00,0x0f,0x14,0x0c,0x03,0x11,0x26,0x4b,0x21,0x0d,0x36,0x13,0x2a,0x2f,0x28,0x0D}},
    {0xE1, 17, {0x37,0x00,0x0f,0x14,0x0b,0x03,0x11,0x26,0x4b,0x21,0x0d,0x36,0x13,0x2a,0x2D,0x28,0x0F}},
    {0xE2, 17, {0x3B,0x07,0x12,0x18,0x0E,0x0D,0x17,0x35,0x44,0x32,0x0C,0x14,0x14,0x36,0x3A,0x0F,0x0D}},
    {0xE3, 17, {0x37,0x07,0x12,0x18,0x0E,0x0D,0x17,0x35,0x44,0x32,0x0C,0x14,0x14,0x36,0x32,0x2F,0x0F}},
    {0xE4, 17, {0x3B,0x07,0x12,0x18,0x0E,0x0D,0x17,0x39,0x44,0x2E,0x0C,0x14,0x14,0x36,0x3A,0x2F,0x0D}},
    {0xE5, 17, {0x37,0x07,0x12,0x18,0x0E,0x0D,0x17,0x39,0x44,0x2E,0x0C,0x14,0x14,0x36,0x3A,0x2F,0x0F}},
    {0xBB, 8,  {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
    {0x11, 1,  {0}},
    {0x29, 1,  {0}},
#endif

    {0xbb, 8,  {0x00,0x00,0x00,0x00,0x00,0x00,0x5a,0xa5}},
    {0xa0, 1,  {0x29}},
};

/* 
    sx: start x
    sy: start y
    ex: end x
    ey: end y
*/
static uint32_t axs15231AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint8_t set_x_cmd[] = {sx>>8, sx, ex>>8, ex};
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
    uint8_t set_y_cmd[] = {sy>>8, sy, ey>>8, ey};
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));
    
    lcdWriteCmd(0x2C);

    return (ey-sy+1) * (ex-sx+1) * (AXS15231_BPP/8);
}


void axs15231HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{       
    uint32_t fillLen = 0;
    
    lspiCtrl.enable = 0;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

    LSPI2->DMACTL |= 1<<25; // tx fifo
    
    lspiCtrl.enable = 1;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

    fillLen = axs15231AddrSet(lcd, 0, 0, previewWidth-1, previewHeight-1);

    lspiCmdCtrl.wrRdn           = 1; // 1: wr   0: rd
    lspiCmdCtrl.ramWr           = 1; // start ramwr
#if (SPI_2_DATA_LANE == 1)
    lspiCmdCtrl.dataLen         = fillLen/2; // 2 datalane is pixel num
#else
    lspiCmdCtrl.dataLen         = AXS15231_WIDTH*AXS15231_HEIGHT*2; // pixel * bpp/8
#endif
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
}

void axs15231HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
}


static int axs15231Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{     
    lcdIoInit(false);
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdRst(100, 100);
    lcdInterfaceType(AXS15231_INTERFACE);
    lcdRegInit(0x15231);
    
    #if 0 // test read id
    uint8_t data = 0x29;
    uint32_t read1523113[40] = {0x1, 0x2, 0x3};
    lspiReadReg(0xda, read1523113, 3, 0);

    uint32_t read15231[5] = {0x1, 0x2, 0x3};
    
    lspiCmdSend(0xa0, &data, 1);
    lspiReadReg(0x0c, read15231, 4, 0);

    uint32_t read1523112[5] = {0x1, 0x2, 0x3};
    lspiCmdSend(0xa0, &data, 1);
    lspiReadReg(0x52, read1523112, 5, 0);
    #endif
    return 0;
}

#if 0
static int axs15231Direction(lcdDrvFunc_t *lcd,DisDirection_e Dir)
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
#endif

static void axs15231BackLight(lcdDrvFunc_t *lcd, uint8_t level)
{
#if (BK_USE_GPIO == 1)
    lcdGpioBkLevel(level);
#elif (BK_USE_PWM == 1)
    lcdPwmBkLevel(level);
#endif
}



static void axs15231StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static int axs15231Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void axs15231DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    axs15231AddrSet(lcd, x, y, x, y);
    lspiFifoWrite(dataWrite);
    return;
}

static void axs15231RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    (who == cbForCam)? (lcd->uspIrq4CamCb = axs15231HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = axs15231HandleUspIrq4Fill);
}

static void axs15231UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}


static void axs15231CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    uint32_t fillLen = 0;
    
#if defined CHIP_EC719 // chip 719
    if (previewStartStop)
    {
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)

#elif (CAMERA_ENABLE_GC6153)

#endif

    }

#else // chip 718
    if (previewStartStop)
    {
        axs15231UnRegisterUspIrqCb(lcd, cbForFill);
        axs15231RegisterUspIrqCb(lcd, cbForCam);
    
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)
        previewWidth  = lcd->width;
        previewHeight = lcd->height;
        
        fillLen = axs15231AddrSet(lcd, 0, 0, previewWidth-1, previewHeight-1);
        
        lspiDataFmt.wordSize            = 7;
        lspiDataFmt.txPack              = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight            = PIC_SRC_HEIGHT;   // frame input height
        lspiInfo.frameWidth             = PIC_SRC_WIDTH;    // frame input width        
        lspiFrameInfoOut.frameHeightOut = previewWidth;     // frame output height
        lspiFrameInfoOut.frameWidthOut  = previewHeight;    // frame output width
        lspiTailorInfo.tailorLeft       = (lspiInfo.frameWidth  - lspiFrameInfoOut.frameWidthOut) /2;
        lspiTailorInfo.tailorRight      = (lspiInfo.frameWidth  - lspiFrameInfoOut.frameWidthOut) /2;
        lspiTailorInfo0.tailorBottom    = (lspiInfo.frameHeight - lspiFrameInfoOut.frameHeightOut)/2;
        lspiTailorInfo0.tailorTop       = (lspiInfo.frameHeight - lspiFrameInfoOut.frameHeightOut)/2;
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);

        lspiCtrl.datSrc                 = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn            = 0; // YUV422, every item is 8bit
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr               = 1; // start ramwr
        #if (SPI_2_DATA_LANE == 1)
        lspiCmdCtrl.dataLen             = fillLen/2; // 2 datalane is pixel num
        #else
        lspiCmdCtrl.dataLen             = AXS15231_WIDTH*AXS15231_HEIGHT*2; // pixel * bpp/8
        #endif
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        
#elif (CAMERA_ENABLE_GC6153)
        previewWidth  = 240;
        previewHeight = 320;

        // preview      
        fillLen = axs15231AddrSet(lcd, 0, 0, previewWidth-1, previewHeight-1);

        lspiDataFmt.wordSize            = 7;
        lspiDataFmt.txPack              = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight            = PIC_SRC_HEIGHT;  // frame input height
        lspiInfo.frameWidth             = PIC_SRC_WIDTH;   // frame input width
        lspiFrameInfoOut.frameHeightOut = previewHeight;   // frame output height
        lspiFrameInfoOut.frameWidthOut  = previewWidth;    // frame output width
        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lspiTailorInfo.tailorLeft       = 0;
        lspiTailorInfo.tailorRight      = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        lspiCtrl.datSrc                 = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn            = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut           = 1; // RGB565
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr               = 1; // start ramwr
        #if (SPI_2_DATA_LANE == 1)
        lspiCmdCtrl.dataLen             = fillLen/2; // 2 datalane is pixel num
        #else
        lspiCmdCtrl.dataLen             = AXS15231_WIDTH*AXS15231_HEIGHT*2; // pixel * bpp/8
        #endif
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        
#elif (CAMERA_ENABLE_GC6133)
        previewWidth  = 240;
        previewHeight = 320;

        // preview      
        fillLen = axs15231AddrSet(lcd, 0, 0, previewWidth-1, previewHeight-1);

        lspiDataFmt.wordSize            = 7;
        lspiDataFmt.txPack              = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight            = PIC_SRC_HEIGHT;  // frame input height
        lspiInfo.frameWidth             = PIC_SRC_WIDTH;   // frame input width
        lspiFrameInfoOut.frameHeightOut = previewHeight;   // frame output height
        lspiFrameInfoOut.frameWidthOut  = previewWidth;    // frame output width
        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lspiTailorInfo.tailorLeft       = 0;
        lspiTailorInfo.tailorRight      = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        lspiCtrl.datSrc                 = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn            = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut           = 1; // RGB565
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

        lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr               = 1; // start ramwr
        #if (SPI_2_DATA_LANE == 1)
        lspiCmdCtrl.dataLen             = fillLen/2; // 2 datalane is pixel num
        #else
        lspiCmdCtrl.dataLen             = AXS15231_WIDTH*AXS15231_HEIGHT*2; // pixel * bpp/8
        #endif
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);        
#endif

    }
    else
    {
        axs15231UnRegisterUspIrqCb(lcd, cbForCam);
        axs15231RegisterUspIrqCb(lcd, cbForFill);
        
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
        lspiTailorInfo0.tailorBottom    = 0;
        lspiTailorInfo0.tailorTop       = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);

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
    }
#endif    
}

static int axs15231Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

lcdDrvFunc_t axs15231Drv = 
{
    .id                 = 0x15231,
    .width              = AXS15231_WIDTH,
    .height             = AXS15231_HEIGHT,
    .freq               = AXS15231_FREQ,
    .bpp                = AXS15231_BPP,
    .initRegTbl         = initTable15231,
    .initRegTblLen      = sizeof(initTable15231)/sizeof(initLine_t),
    .dir                = 0,

    .init               = axs15231Init,
    .drawPoint          = axs15231DrawPoint,
    .setWindow          = axs15231AddrSet,
    .fill               = axs15231Fill,
    .backLight          = axs15231BackLight,
    .startStop          = axs15231StartStop,
    .startStopPreview   = axs15231CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = axs15231RegisterUspIrqCb,
    .unregisterUspIrqCb = axs15231UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = axs15231Close,
};

