#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_77903.h"

extern lspiDrvInterface_t *lcdDrv;

static const initLine_t initTable77903[] = 
{
    {0xf0, 1, {0xc3}},
    {0xf0, 1, {0x96}},
    {0xf0, 1, {0xa5}},
    {0xc1, 4, {0x00,0x08,0xae,0x13}}, 
    {0xc2, 4, {0x00,0x08,0xa5,0x13}},
    {0xc3, 4, {0x44,0x04,0x44,0x04}},
    {0xc4, 4, {0x44,0x04,0x44,0x04}},
    {0xc5, 1, {0x51}},
    {0xd6, 1, {0x0}},
    {0xd7, 1, {0x0}},
    {0xe0, 14,{0xd2,0x07,0x0c,0x0a,0x09,0x26,0x34,0x44,0x4b,0x19,0x14,0x14,0x2d,0x33}},
    {0xe1, 14,{0xd2,0x07,0x0c,0x09,0x08,0x25,0x33,0x44,0x4a,0x19,0x14,0x13,0x2d,0x33}},
    {0xe5, 14,{0xbd,0xf5,0xc1,0x33,0x22,0x25,0x10,0x55,0x55,0x55,0x55,0x55,0x55,0x55}},
    {0xe6, 14,{0xbd,0xf5,0xc1,0x33,0x22,0x25,0x10,0x55,0x55,0x55,0x55,0x55,0x55,0x55}},
    {0xec, 6, {0x00,0x55,0x00,0x00,0x00,0x08}},
    {0x36, 1, {0x0c}},
#if (ST77903_BPP == 16)    
    {0x3a, 1, {5}}, // rgb565
#elif (ST77903_BPP == 24)
    {0x3a, 1, {7}}, // rgb888
#elif (ST77903_BPP == 18)    
    {0x3a, 1, {6}}, // rgb666
#endif 
    {0xb2, 1, {0x09}},
    {0xb3, 1, {0x01}},
    {0xb4, 1, {0x01}},
    {0xb5, 4, {0x00,0x08,0x00,0x08}},
    {0xb6, 2, {0xc7,0x31}},
    {0xa4, 2, {0xc0,0x63}},
    {0xa5, 9, {0x00,0x00,0x00,0x00,0x00,0x15,0x2a,0xba,0x02}},
    {0xa6, 9, {0x00,0x00,0x00,0x00,0x00,0x15,0x2a,0xba,0x02}},
    {0xba, 7, {0x1a,0x0a,0x45,0x00,0x23,0x01,0x00}},
    {0xbb, 8, {0x00,0x24,0x00,0x2f,0x83,0x07,0x18,0x00}},
    {0xbd, 11,{0x22,0x11,0xff,0xff,0x55,0x44,0x77,0x66,0xff,0xff,0x0f}},
    {0xff, 1, {200}},
    {0xed, 1, {0xc3}},
    {0xe4, 3, {0x40,0x0f,0x2f}},
    {0x21, 0, {}},
    {0x11, 0, {}},
    {0x29, 0, {}},
    
    {0xf0, 1, {0xc3}},
    {0xf0, 1, {0x96}},

#if (BIST_TEST == 1)
    {0xb0, 1, {0xa5}},
    {0xcc, 9, {0x40,0x00,0x3f,0x00,0x14,0x14,0x20,0x20,0x03}},
#endif
};

void st77903HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{   

}

void st77903HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
}

static int st77903Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{     
    lcdIoInit(false);
    lcdRst(10, 10);
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdInterfaceType(ST77903_INTERFACE);
    lcdRegInit(0x77903);
    lcdMspiHsyncSet(0x60, 0xde, 6, 6);   
    lcdWriteCmd(0x61);
    lcdMspiSet(1, 0, 2, DEFAULT_INST);
    lcdMspiVsyncSet(1, 0xde, lspiDiv);
    lcdCsnHighCycleMin(lspiDiv);
    return 0;
}

static void st77903BackLight(lcdDrvFunc_t *lcd, uint8_t level)
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
static uint32_t st77903AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint32_t len = 0;
#if 0 // how to set the coordinate?
    uint8_t set_x_cmd[] = {sx>>8, sx, ex>>8, ex};
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
    uint8_t set_y_cmd[] = {sy>>8, sy, ey>>8, ey};
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));
    
    lcdWriteCmd(0x2C);
#endif    

    if (ST77903_BPP == 24)
    {
        len = (ey-sy+1) * (ex-sx+1) * (ST77903_BPP/8+1);
    }
    else
    {
        len = (ey-sy+1) * (ex-sx+1) * (ST77903_BPP/8);
    }
    
    return len;
}

static void st77903StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static int st77903Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void st77903DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    st77903AddrSet(lcd, x, y, x, y);
    lspiFifoWrite(dataWrite);
    return;
}

static void st77903RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = st77903HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = st77903HandleUspIrq4Fill);
}

static void st77903UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}

static void st77903CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        st77903UnRegisterUspIrqCb(lcd, cbForFill);
        st77903RegisterUspIrqCb(lcd, cbForCam);
        
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)
        // preview
        lcdWriteCmd(0x36);
        // lcdWriteData(0x00);// 0: normal;   0x20: reverse, mirror image   0x40: x mirror
        lcdWriteData(0xa0);
        lcdDrv->send(NULL, 0);
        
        lcdWriteCmd(0x2C);
        st77903AddrSet(lcd, 0, 0, 320-1, 240-1);

        lspiDataFmt.wordSize = 7;
        lspiDataFmt.txPack = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight = 480; // frame input height
        lspiInfo.frameWidth = 640; // frame input width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        
        lspiFrameInfoOut.frameHeightOut =320;//lcd->pra->height;//320; // frame output height
        lspiFrameInfoOut.frameWidthOut = 240;//lcd->pra->width;//240; // frame output width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        // lspiScaleInfo.rowScaleFrac =128;
        // lspiScaleInfo.colScaleFrac = 128;

        lspiScaleInfo.rowScaleFrac =64;
        lspiScaleInfo.colScaleFrac = 64;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
#elif (CAMERA_ENABLE_GC6153)

#endif

    }
}

static int st77903Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

lcdDrvFunc_t st77903Drv = 
{
    .id                 = 0x77903,
    .width              = ST77903_WIDTH,
    .height             = ST77903_HEIGHT,
    .freq               = ST77903_FREQ,
    .bpp                = ST77903_BPP,
    .initRegTbl         = initTable77903,
    .initRegTblLen      = sizeof(initTable77903)/sizeof(initLine_t),
    .dir                = 0,

    .init               = st77903Init,
    .drawPoint          = st77903DrawPoint,
    .setWindow          = st77903AddrSet,
    .fill               = st77903Fill,
    .backLight          = st77903BackLight,
    .startStop          = st77903StartStop,
    .startStopPreview   = st77903CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = st77903RegisterUspIrqCb,
    .unregisterUspIrqCb = st77903UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = st77903Close,
};

