#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_8601.h"

extern lspiDrvInterface_t *lcdDrv;

static const initLine_t initTable8601[] = 
{
    {0x11, 0, {}},
    {0xff, 1, {20}},
#if (SH8601_BPP == 16)    
    {0x3a, 1, {0x5}}, // rgb565
#elif (SH8601_BPP == 18)
    {0x3a, 1, {0x6}}, // rgb666
#elif (SH8601_BPP == 24)    
    {0x3a, 1, {0x7}}, // rgb888
#endif 
    {0xff, 1, {20}},
    {0x4a, 1, {0xff}},
    {0xff, 1, {20}},
    {0x4b, 1, {0xff}}, 
    {0xff, 1, {20}},
    {0x35, 1, {0x00}},
    {0xff, 1, {20}},
    {0x44, 1, {0x00}},
    {0xff, 1, {20}},
    {0x51, 1, {0xff}},
    {0xff, 1, {20}},
    {0x52, 1, {0xff}},
    {0xff, 1, {20}},
    {0x53, 1, {0x20}},
    {0xff, 1, {20}},
    {0x51, 1, {0xff}},
    {0xff, 1, {20}},
    {0x29, 0, {}},
    {0xff, 1, {20}},
};

void sh8601HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{   
    lspiCtrl.enable = 0;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

    LSPI2->DMACTL |= 1<<25; // tx fifo
    
    lspiCtrl.enable = 1;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);

    lspiCmdCtrl.wrRdn           = 1; // 1: wr   0: rd
    lspiCmdCtrl.ramWr           = 1; // start ramwr
    lspiCmdCtrl.ramWrHaltMode   = 1; // maintain cs as low between cmd and data
    lspiCmdCtrl.dataLen         = LCD_WIDTH*LCD_HEIGHT; // infinite data, used in camera to lspi
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);

    if (LSPI2->STAS & LSPI_STATS_TX_FIFO_LEVEL_Msk)
    {
        printf("tx error");
    }

    if ((LSPI2->STAS & 1<<1) != 0)
    {
        printf("tx overflow");
    }
}

void sh8601HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
    printf("enter fill\n");
}


static int sh8601Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{
    lcdIoInit(false);
    lcdRst(20, 20);
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdInterfaceType(SH8601_INTERFACE);
    lcdRegInit(0x8601);
    //*(uint32_t*)0x4d0420dc = 1 | (1<<1) | (1<<4); // te para0
    //*(uint32_t*)0x4d0420e0 = 2; // te para1
    
    #if 0 // test read id
    uint32_t read8601[3] = {0};
    lspiReadReg(0xda, read8601,   1, 0);
    lspiReadReg(0xdb, read8601+1, 1, 0);
    lspiReadReg(0xdc, read8601+2, 1, 0);
    #endif
    return 0;
}

static void sh8601BackLight(lcdDrvFunc_t *lcd, uint8_t level)
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
static uint32_t sh8601AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint32_t len = 0;

    lcdMspiSet(1, 0, 0, 0x02); // 1wire: 0x02;    4wire: 0x32
    
    // column addr set. p112    
    uint8_t set_x_cmd[] = {sx>>8, sx, ex>>8, ex};
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
    uint8_t set_y_cmd[] = {sy>>8, sy, ey>>8, ey};
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));

    if (SH8601_BPP == 24)
    {
        len = (ey-sy+1) * (ex-sx+1) * (SH8601_BPP/8+1);
    }
    else
    {
        len = (ey-sy+1) * (ex-sx+1) * (SH8601_BPP/8);
    }

    // send 0x2c  
    lcdMspiSet(1, 0, 2, 0x32); // 1wire: 0x02;    4wire: 0x32
    lcdWriteCmd(0x2c);
    
    return len;
}

static void sh8601StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static void sh8601RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = sh8601HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = sh8601HandleUspIrq4Fill);
}

static void sh8601UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}

static int sh8601Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void sh8601DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    sh8601AddrSet(lcd, x, y, x, y);
    lspiFifoWrite(dataWrite);
    return;
}

static void sh8601CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        sh8601UnRegisterUspIrqCb(lcd, cbForFill);
        sh8601RegisterUspIrqCb(lcd, cbForCam);
        
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)
        lspiScaleInfo.rowScaleFrac = 0;
        lspiScaleInfo.colScaleFrac = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);

        lspiTailorInfo.tailorLeft    = (PIC_SRC_WIDTH - SH8601_WIDTH)/2;
        lspiTailorInfo.tailorRight   = (PIC_SRC_WIDTH - SH8601_WIDTH)/2;
        lspiTailorInfo0.tailorBottom = (PIC_SRC_HEIGHT - SH8601_HEIGHT)/2;
        lspiTailorInfo0.tailorTop    = (PIC_SRC_HEIGHT - SH8601_HEIGHT)/2;
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);
    
        lspiDataFmt.wordSize = 7;
        lspiDataFmt.txPack   = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight 			= PIC_SRC_HEIGHT; // frame input height
        lspiInfo.frameWidth  			= PIC_SRC_WIDTH;  // frame input width
        lspiFrameInfoOut.frameHeightOut = SH8601_HEIGHT;  // frame output height
        lspiFrameInfoOut.frameWidthOut  = SH8601_WIDTH;   // frame output width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);
        
        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        
        lspiDmaCtrl.txDmaReqEn = 0;
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        
		#if (PRE_CMD_TEST)
		lspiIntCtrl.ramWrIntCtrlEn = 1; // STATS bit13
		#else
        lspiIntCtrl.lspiRamWrEndEn = 1; // STATS bit31
		#endif
        //lspiIntCtrl.lspiRamWrFrameEndEn = 1;
        lcdDrv->ctrl(LSPI_CTRL_INT_CTRL, 0);
        
        LSPI2->DMACTL |= 1<<24; // flush dma
        LSPI2->DMACTL |= 1<<25;
        
        lspiMspiCtrl.mspiInst       = 0x32; // 1wire: 0x02;    4wire: 0x32    
        lspiMspiCtrl.mspiAddrLane   = 0;
        lspiMspiCtrl.mspiDataLane   = 2;
        lcdDrv->ctrl(LSPI_MSPI_CTRL, 0);
        
        lspiCtrl.enable         = 1;
        lspiCtrl.datSrc         = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn    = 0; // YUV422, every item is 8bit. 1:y only
        lspiCtrl.colorModeOut   = 1; // RGB565
        lspiCmdAddr.busType     = 1; // Interface II
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
                
        #if PRE_CMD_TEST
        LSPI2->LSPI_CADDR = 0xa2c;      
        *(uint32_t*)0x4d0420b8 = 1 | (1<<1) | (4<<2) | (4<<5);
        *(uint32_t*)0x4d0420c0 = 0 | (xs<<8) | (xeh<<16) | (xel<<24); // 2a  high_addr_16-low_addr_16
        *(uint32_t*)0x4d0420c4 = 0 | (ys<<8) | (yeh<<16) | (yel<<24); // 2b
        #endif

        lspiCmdCtrl.wrRdn           = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr           = 1; // start ramwr
        lspiCmdCtrl.ramWrHaltMode   = 1; // maintain cs as low between cmd and data
        #if PRE_CMD_TEST
        lspiCmdCtrl.dataLen = SH8601_HEIGHT * SH8601_WIDTH*2;//0x3fffff;
        #else
        lspiCmdCtrl.dataLen = SH8601_HEIGHT * SH8601_WIDTH; // infinite data, used in camera to lspi
        #endif
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        
#elif (CAMERA_ENABLE_GC6153)

#endif
    }
}

static int sh8601Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

lcdDrvFunc_t sh8601Drv = 
{
    .id                 = 0x8601,
    .width              = SH8601_WIDTH,
    .height             = SH8601_HEIGHT,
    .freq               = SH8601_FREQ,
    .bpp                = SH8601_BPP,
    .initRegTbl         = initTable8601,
    .initRegTblLen      = sizeof(initTable8601)/sizeof(initLine_t),
    .dir                = 0,

    .init               = sh8601Init,
    .drawPoint          = sh8601DrawPoint,
    .setWindow          = sh8601AddrSet,
    .fill               = sh8601Fill,
    .backLight          = sh8601BackLight,
    .startStop          = sh8601StartStop,
    .startStopPreview   = sh8601CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = sh8601RegisterUspIrqCb,
    .unregisterUspIrqCb = sh8601UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = sh8601Close,
};

