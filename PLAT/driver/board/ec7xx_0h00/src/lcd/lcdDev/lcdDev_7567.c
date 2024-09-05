#include "lcdDev_7567.h"

extern lspiDrvInterface_t lspiDrvInterface2;
lspiDrvInterface_t *lspiDrv7567 = &lspiDrvInterface2; // We use lspi2 by default

DmaDescriptor_t __ALIGNED(16) lcdDmaTxDesc7567[HEIGHT_7567];
int8_t lcdDmaTxCh7567; // dma tx channel
DmaTransferConfig_t lcdDmaTxCfg7567 =
{
    NULL,
    (void *)&(LSPI2->TFIFO),
    DMA_FLOW_CONTROL_TARGET,
    DMA_ADDRESS_INCREMENT_SOURCE,
    DMA_DATA_WIDTH_ONE_BYTE,
    DMA_BURST_8_BYTES, 
    WIDTH_7567 * 2
};

void st7567CamPreviewStartStop(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop)
{

}

void st7567Reginit(lcdDev_t *lcd)
{

    {
        //--------------------------------ST7567 reset sequence------------------------------------//
        LSPI_RST_LOW;
        mDelay(50); //Delay 100ms
        LSPI_RST_HIGH;
        mDelay(150); //Delay 200ms
    }    
	
	lcdWriteCmd(0xE2);
    lspiDrv7567->send(NULL, 0);
    
    lcdWriteCmd(0xA2);
    lspiDrv7567->send(NULL, 0);

	lcdWriteCmd(0xA0);	
    lspiDrv7567->send(NULL, 0);

    lcdWriteCmd(0xA7);	
    lspiDrv7567->send(NULL, 0);

	lcdWriteCmd(0xC0);	
    lspiDrv7567->send(NULL, 0);

	lcdWriteCmd(0xA4);	
    lspiDrv7567->send(NULL, 0);  

	lcdWriteCmd(0x40);	
    lspiDrv7567->send(NULL, 0);   

	lcdWriteCmd(0x24);	
    lspiDrv7567->send(NULL, 0);   

	lcdWriteCmd(0x81);	
    lspiDrv7567->send(NULL, 0);   

	
	lcdWriteCmd(0x2C);	
    lspiDrv7567->send(NULL, 0);   

	lcdWriteCmd(0xF8);	
    lspiDrv7567->send(NULL, 0);   
    
	lcdWriteCmd(0x00);	
    lspiDrv7567->send(NULL, 0);   

	lcdWriteCmd(0x2C);	
    lspiDrv7567->send(NULL, 0);  
    mDelay(50); //Delay 100ms

	lcdWriteCmd(0x2E);
    lspiDrv7567->send(NULL, 0); 
    mDelay(50); //Delay 100ms
    
	lcdWriteCmd(0x2F);
    lspiDrv7567->send(NULL, 0);   
    mDelay(50); //Delay 100ms

    lcdWriteCmd(0xAF);
    lspiDrv7567->send(NULL, 0);
    lspiCmdCtrl.wrRdn = 1;
    lspiCmdCtrl.ramWr = 1;
    lspiCmdCtrl.dataLen = 0x3ffff;
    lspiDrv7567->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    mDelay(50); //Delay 100ms
}


static void st7567GpioInit(lcdDev_t *lcd)
{
    PadConfig_t config;

    PAD_getDefaultConfig(&config);
    
    GpioPinConfig_t gpioCfg;

    // Rst pin
    config.mux = LSPI_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(LSPI_RST_GPIO_PIN, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    GPIO_pinConfig(LSPI_RST_GPIO_INSTANCE, LSPI_RST_GPIO_PIN, &gpioCfg);    
}


static int st7567Init(lcdDev_t *lcd, lspiCbEvent_fn cb)
{
    //DmaRequestSource_e request;
    
    {
        //request = (DmaRequestSource_e)LSPI_DMA_TX_REQID;
        
        lspiDrv7567->init(NULL); // pin init
        st7567GpioInit(lcd);

        lspiDrv7567->ctrl(LSPI_CTRL_BUS_SPEED, 0); // bus speed

        lspiDataFmt.wordSize = 7;
        lspiDrv7567->ctrl(LSPI_CTRL_DATA_FORMAT, 0);

        lspiBusSel.lspiBusEn = 1;
        lspiDrv7567->ctrl(LSPI_CTRL_BUS_SEL, 0);

        lspiCtrl.enable = 1;
        lspiCtrl.line4 = 1;
        lspiCtrl.datSrc = 0; // 0: data from camera; 1: data from memory
        lspiCtrl.colorModeIn = 0; // YUV422, every item is 8bit
        lspiCtrl.colorModeOut = 4; // 1bit gray mode
        lspiCtrl.busType = 1; // Interface II
        lspiDrv7567->ctrl(LSPI_CTRL_CTRL, 0);

        lspiInfo.frameHeight = 69; // frame input height
        lspiInfo.frameWidth = 92; // frame input width
        lspiDrv7567->ctrl(LSPI_CTRL_FRAME_INFO, 0);

        lspiFrameInfoOut.frameHeightOut = 64; // frame output height
        lspiFrameInfoOut.frameWidthOut = 92; // frame output width
        lspiDrv7567->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);

        lspiQuartileCtrl.grayCtrl = 2; // Histogram equalization
        lspiDrv7567->ctrl(LSPI_CTRL_QUARTILE_CTRL, 0);

        lspiTailorInfo.tailorLeft = 0; // cut left columns
        lspiTailorInfo.tailorRight = 0; // cut right columns
        lspiDrv7567->ctrl(LSPI_CTRL_TAILOR_INFO, 0);

        lspiTailorInfo0.tailorBottom = 2; // cut bottom lines
        lspiTailorInfo0.tailorTop = 3; // cut top lines
        lspiDrv7567->ctrl(LSPI_CTRL_TAILOR_INFO0, 0);

        lspiGrayPageCmd0.pageCmd = 0xb0; // start page 0, page cmd should be fixed with 0xb0+page_num
        lspiGrayPageCmd0.pageCmd0 = 0x10; // command followed with page_cmd. column_num high bits
        lspiGrayPageCmd0.pageCmd01ByteNum = 2; // page_cmd0+page_cmd1   byte num
        lspiDrv7567->ctrl(LSPI_CTRL_GRAY_PAGE_CMD0, 0);

        lspiGrayPageCmd1.pageCmd1 = 0; // followed with page_cmd0. column_num low bits
        lspiDrv7567->ctrl(LSPI_CTRL_GRAY_PAGE_CMD1, 0);     
    }

    st7567Reginit(lcd);

    #if 0
    // Tx config
    DMA_init(DMA_INSTANCE_MP);
    lcdDmaTxCh7789 = DMA_openChannel(DMA_INSTANCE_MP);

    DMA_setChannelRequestSource(DMA_INSTANCE_MP, lcdDmaTxCh7789, request);
    DMA_rigisterChannelCallback(DMA_INSTANCE_MP, lcdDmaTxCh7789, NULL);
    #endif
    return 0;
}


static int st7567DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    return 0;
}

static int st7567Fill(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    return 0;
}

static int stPrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}

static int st7567OnOff(lcdDev_t* lcd, uint8_t sta)
{
    return 0;
}

static void st7567BackLight(lcdDev_t* lcd, uint8_t sta)
{
}

void st7567BuildDmaChain(lcdDev_t* lcd, uint8_t* dataBuf, const uint16_t chainCnt,
                                bool needStop)
{    


}

void st7567StartStop(lcdDev_t* lcd, bool startOrStop)
{
    if (startOrStop)
    {
        DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, lcdDmaTxCh7567, lcdDmaTxDesc7567);
    }
    else
    {    
        extern void DMA_stopChannelNoWait(DmaInstance_e instance, uint32_t channel);
        DMA_stopChannelNoWait(DMA_INSTANCE_MP, lcdDmaTxCh7567);
    }

}

uint8_t check321 = 0;
void st7567Clear(lcdDev_t* lcd, uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{
    uint32_t bk1=0,bk2=0,bk3=0,bk4=0,bk5=0,bk6=0, bk7=0,bk8, bk9, bk10;

    bk1 = LSPI2->LSPI_CTRL;
    bk2 = LSPI2->LSPFINFO;
    bk3 = LSPI2->LSPFINFO0;
    bk4 = LSPI2->LSPIGPCMD0;
    bk5 = LSPI2->LSPIGPCMD1;
    bk6 = LSPI2->DFMT;
    bk7 = CSPI1->CSPIPROCLSPI;
    bk8 = LSPI2->LSPIQUARTCTRL;
    bk9 = LSPI2->LSPTINFO;
    bk10 = LSPI2->LSPTINFO0;

    CSPI1->CSPIPROCLSPI = 0;
    LSPI2->DFMT         = 7<<6 | 2<<18;
    LSPI2->LSPI_CTRL    = 1 | 1<<2 | 1<<3 | 1<<4 | 4<<6 | 1<<29; // 7789: 3<<4 | 1<<6;   7567: 1<<4 | 4<<6
    LSPI2->LSPFINFO     = 64<<0 | 128<<16;
    LSPI2->LSPFINFO0	= 64<<0 | 128<<10;
    
    LSPI2->LSPIGPCMD0   = 0<<0 | 0xb<<4 | 0x0010<<8 | 2<<28;
    LSPI2->LSPIGPCMD1   = 0x0000;
    LSPI2->LSPTINFO0 = 0;
    LSPI2->LSPTINFO = 0;
    LSPI2->LSPIQUARTCTRL = 0;

    
    //st7567Line(0xff, 0x00);
    int  i;

    for (int j=0; j<64; j++)
    {
        for (i=0; i<64; i++)
        {
            //lcdWriteData(0xff);
            //lcdWriteData(0xff);
            lspiDrv7567->prepareSend(0xff);
            lspiDrv7567->prepareSend(0xff);

            check321++;
            
            if (check321 == 2)
            {
                #if 0
                // txfifo
                tmp = ((LSPI2->STAS) >> 13) & 0x3f;
                //while (tmp > 15);
                while (!LSPI2->LSPI_STAT);
                LSPI2->TFIFO = dataList[0]; 
                check321 = 0;
                dataListIndex = 0;
                dataList[0]= 0;
                #endif
                lspiDrv7567->send(NULL, 0);
            }
        }
        
        delay_us(100*1000);

      //  st7567Line(0x00, 0xff);
        //delay_us(600*1000);
    }

    LSPI2->LSPI_CTRL = bk1;
    LSPI2->LSPFINFO  = bk2;
    LSPI2->LSPFINFO0 = bk3;
    LSPI2->LSPIGPCMD0 = bk4;
    LSPI2->LSPIGPCMD1 = bk5;
    LSPI2->DFMT         = bk6;
    CSPI1->CSPIPROCLSPI = bk7;
    LSPI2->LSPTINFO0 = bk10;
    LSPI2->LSPTINFO = bk9;
    LSPI2->LSPIQUARTCTRL = bk8;
}



lcdDrvFunc_t st7567Drv = 
{
    .id             = 0x7567,

    .init           = st7567Init,
    .drawPoint      = st7567DrawPoint,
    .fill           = st7567Fill,
    .prepareDisplay = stPrepareDisplay,
    .onoff          = st7567OnOff,
    .backLight      = st7567BackLight,
    .startStop      = st7567StartStop,
    .clear          = st7567Clear,
    .startStopPreview = st7567CamPreviewStartStop,
};

