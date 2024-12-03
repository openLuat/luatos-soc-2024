#include "bsp.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "lcdDev_5300.h"
#include "sctdef.h"

extern lspiDrvInterface_t *lcdDrv;
AP_PLAT_COMMON_BSS uint8_t set_x_cmd[4] = {0};
AP_PLAT_COMMON_BSS uint8_t set_y_cmd[4] = {0};

#define PRE_CMD_ENABLE 	0
#define TEST_IRQ_NUM    1


static const initLine_t initTable5300[] = 
{
	{0xfe, 0, {}},
	{0xc4, 1, {0x80}},
#if (CO5300_BPP == 16)    
	{0x3a, 1, {0x55}}, // rgb565
#elif (CO5300_BPP == 18)
	{0x3a, 1, {0x6}}, // rgb666
#elif (CO5300_BPP == 24)    
	{0x3a, 1, {0x77}}, // rgb888
#endif 
	{0xff, 1, {20}},
	{0x35, 1, {0x00}},
	{0xff, 1, {20}},
	{0x53, 1, {0x20}}, 
	{0xff, 1, {20}},
	{0x51, 1, {0xff}},
	{0xff, 1, {20}},
	{0x63, 1, {0xff}},
	{0xff, 1, {20}},
	{0x2a, 4, {0x0, 0x0a, 0x01, 0xd5}}, 
	{0xff, 1, {20}},
	{0x2b, 4, {0x0, 0x0,  0x01, 0xcb}},
	{0xff, 1, {20}},
	{0x11, 0, {}},
	{0xff, 1, {60}},
	{0x29, 0, {}},
};

void co5300HandleUspIrq4Cam(lcdDrvFunc_t *lcd)
{   
#if (PRE_CMD_ENABLE)
    LSPI2->STAS |= (1<<13);
#endif
	
#if (!PRE_CMD_ENABLE)
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
#endif	
}

void co5300HandleUspIrq4Fill(lcdDrvFunc_t *lcd)
{
	//GPIO_pinWrite(0, 1 << 3, 0);
	//GPIO_pinWrite(0, 1 << 3, 1 << 3);
	
    printf("enter fill\n");
}


static int co5300Init(lcdDrvFunc_t *lcd, void* uspCb, void *dmaCb, uint32_t freq, uint8_t bpp)
{
    lcdIoInit(false);
    lcdRst(200, 200);
    dmaInit(dmaCb);
    lspiDefaultCfg(lcd, uspCb, freq, bpp);
    lcdInterfaceType(CO5300_INTERFACE);
    lcdRegInit(0x5300);

#if (TEST_IRQ_NUM)
	lspiIntCtrl.ramWrIntCtrl   = 1;
	lspiIntCtrl.ramWrIntCtrlEn = 1;
#endif
	lcdDrv->ctrl(LSPI_CTRL_INT_CTRL, 0);

	
    #if 0 // test read id
    uint32_t read5300[3] = {0};
    lspiReadReg(0xda, read5300,   1, 0);
    lspiReadReg(0xdb, read5300+1, 1, 0);
    lspiReadReg(0xdc, read5300+2, 1, 0);
    #endif
    return 0;
}

static void co5300BackLight(lcdDrvFunc_t *lcd, uint8_t level)
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
static uint32_t co5300AddrSet(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    uint32_t len = 0;

    lcdMspiSet(1, 0, 0, 0x02); // 1wire: 0x02;    4wire: 0x32
    
    // column addr set. p112    
    sx += LCD_X_OFFSET;
	ex += LCD_X_OFFSET;

	set_x_cmd[0] = sx>>8;
	set_x_cmd[1] = sx;
	set_x_cmd[2] = ex>>8;
	set_x_cmd[3] = ex;
    lspiCmdSend(0x2A, set_x_cmd, sizeof(set_x_cmd));
    
	set_y_cmd[0] = sy>>8;
	set_y_cmd[1] = sy;
	set_y_cmd[2] = ey>>8;
	set_y_cmd[3] = ey;
    lspiCmdSend(0x2B, set_y_cmd, sizeof(set_y_cmd));

    if (CO5300_BPP == 24)
    {
        len = (ey-sy+1) * (ex-sx+1) * (CO5300_BPP/8+1);
    }
    else
    {
        len = (ey-sy+1) * (ex-sx+1) * (CO5300_BPP/8);
    }

    // send 0x2c  
    lcdMspiSet(1, 0, 2, 0x32); // 1wire: 0x02;    4wire: 0x32
    lcdWriteCmd(0x2c);
    
    return len;
}

static void co5300StartStop(lcdDrvFunc_t *lcd, bool startOrStop)
{
    dmaStartStop(startOrStop);
}

static void co5300RegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = co5300HandleUspIrq4Cam) : (lcd->uspIrq4FillCb = co5300HandleUspIrq4Fill);
}

static void co5300UnRegisterUspIrqCb(lcdDrvFunc_t *lcd, uspCbRole_e who)
{
    if (lcd == NULL)
    {
        EC_ASSERT(0,0,0,0);
    }

    (who == cbForCam)? (lcd->uspIrq4CamCb = NULL) : (lcd->uspIrq4FillCb = NULL);
}

static int co5300Fill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf)
{
    return lcdDmaTrans(lcd, buf, fillLen);
}

static void co5300DrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    co5300AddrSet(lcd, x, y, x, y);
    lspiFifoWrite(dataWrite);
    return;
}

static void co5300CamPreviewStartStop(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{   
    if (previewStartStop)
    {
        co5300UnRegisterUspIrqCb(lcd, cbForFill);
        co5300RegisterUspIrqCb(lcd, cbForCam);
        
#if (CAMERA_ENABLE_BF30A2)

#elif (CAMERA_ENABLE_GC032A)
        lspiScaleInfo.rowScaleFrac = 0;
        lspiScaleInfo.colScaleFrac = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);

        lspiTailorInfo.tailorLeft    = (PIC_SRC_WIDTH - CO5300_WIDTH)/2;
        lspiTailorInfo.tailorRight   = (PIC_SRC_WIDTH - CO5300_WIDTH)/2;
        lspiTailorInfo0.tailorBottom = (PIC_SRC_HEIGHT - CO5300_HEIGHT)/2;
        lspiTailorInfo0.tailorTop    = (PIC_SRC_HEIGHT - CO5300_HEIGHT)/2;
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);
    
        lspiDataFmt.wordSize = 7;
        lspiDataFmt.txPack   = 0;
        lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
        
        // lcd size
        lspiInfo.frameHeight 			= PIC_SRC_HEIGHT; // frame input height
        lspiInfo.frameWidth  			= PIC_SRC_WIDTH;  // frame input width
        lspiFrameInfoOut.frameHeightOut = CO5300_HEIGHT;  // frame output height
        lspiFrameInfoOut.frameWidthOut  = CO5300_WIDTH;   // frame output width
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
        lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);
        
        lspiScaleInfo.rowScaleFrac      = 0;
        lspiScaleInfo.colScaleFrac      = 0;
        lcdDrv->ctrl(LSPI_CTRL_SCALE_INFO, 0);
        
        lspiDmaCtrl.txDmaReqEn = 0;
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
        
		#if (PRE_CMD_ENABLE)
		//lspiIntCtrl.ramWrIntCtrlEn = 1;
		#else
        lspiIntCtrl.lspiRamWrEndEn = 1;
		#endif
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
                
        #if PRE_CMD_ENABLE
		lspiCmdAddr.addr 			= 0x2c;
		lspiCmdAddr.csnHighCycleMin = 0xa;
    	lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
	
		lspiPreParam0.lspiCmd0PreEn 		= 1;
		lspiPreParam0.lspiCmd1PreEn 		= 1;
		lspiPreParam0.lspiCmd0PreParaLen 	= 4;
		lspiPreParam0.lspiCmd1PreParaLen 	= 4;
		lspiPreParam2.lspiCmd0PrePara		= set_x_cmd[0] | (set_x_cmd[1]<<8) | (set_x_cmd[2]<<16) | (set_x_cmd[3]<<24);
		lspiPreParam3.lspiCmd1PrePara		= set_y_cmd[0] | (set_y_cmd[1]<<8) | (set_y_cmd[2]<<16) | (set_y_cmd[3]<<24);
    	lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
        #endif

        lspiCmdCtrl.wrRdn           = 1; // 1: wr   0: rd
        lspiCmdCtrl.ramWr           = 1; // start ramwr
        lspiCmdCtrl.ramWrHaltMode   = 1; // maintain cs as low between cmd and data
        #if PRE_CMD_ENABLE
        lspiCmdCtrl.dataLen = 0x3fffff;
		#else
        lspiCmdCtrl.dataLen = CO5300_HEIGHT * CO5300_WIDTH;
        #endif
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        
#elif (CAMERA_ENABLE_GC6153)

#endif
    }
}

static int co5300Close(lcdDrvFunc_t *lcd)
{
    return 0;
}

AP_PLAT_COMMON_DATA lcdDrvFunc_t co5300Drv = 
{
    .id                 = 0x5300,
    .width              = CO5300_WIDTH,
    .height             = CO5300_HEIGHT,
    .freq               = CO5300_FREQ,
    .bpp                = CO5300_BPP,
    .initRegTbl         = initTable5300,
    .initRegTblLen      = sizeof(initTable5300)/sizeof(initLine_t),
    .dir                = 0,

    .init               = co5300Init,
    .drawPoint          = co5300DrawPoint,
    .setWindow          = co5300AddrSet,
    .fill               = co5300Fill,
    .backLight          = co5300BackLight,
    .startStop          = co5300StartStop,
    .startStopPreview   = co5300CamPreviewStartStop,
    .uspIrq4CamCb       = NULL,
    .uspIrq4FillCb      = NULL,
    .registerUspIrqCb   = co5300RegisterUspIrqCb,
    .unregisterUspIrqCb = co5300UnRegisterUspIrqCb,
    .direction          = NULL,
    .close              = co5300Close,
};

