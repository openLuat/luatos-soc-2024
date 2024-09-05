#include "lcdDev_7571.h"

DmaDescriptor_t __ALIGNED(16) lcdDmaTxDesc7571[HEIGHT_7571];
int8_t lcdDmaTxCh7571; // dma tx channel
DmaTransferConfig_t lcdDmaTxCfg7571 =
{
    NULL,
    (void *)&(LSPI2->TFIFO),
    DMA_FLOW_CONTROL_TARGET,
    DMA_ADDRESS_INCREMENT_SOURCE,
    DMA_DATA_WIDTH_ONE_BYTE,
    DMA_BURST_8_BYTES, 
    WIDTH_7571 * 2
};

#if 0
spiPin_t  st7571SpiPin = 
{
//                  instance       pinNum    padAddr    func
    .cs   = {0,         8,      23,     PAD_MUX_ALT0},    
    .ds   = {1,         0,      31,     PAD_MUX_ALT0},    
    .rst  = {1          1,      32,     PAD_MUX_ALT0},    
    .clk  = {0,         11,     26,     PAD_MUX_ALT1},    
    .mosi = {0,         9,      24,     PAD_MUX_ALT1},    
    .miso = {0,         10,     25,     PAD_MUX_ALT1}, 
};

spiPin_t  st7571LspiPin = 
{
//                  instance       pinNum    padAddr    func
    .cs   = {0,         8,      23,     PAD_MUX_ALT0},    
    .ds   = {1,         0,      31,     PAD_MUX_ALT0},    
    .rst  = {1          1,      32,     PAD_MUX_ALT0},    
    .clk  = {0,         11,     26,     PAD_MUX_ALT1},    
    .mosi = {0,         9,      24,     PAD_MUX_ALT1},    
    .miso = {0,         10,     25,     PAD_MUX_ALT1}, 
};
#endif

void st7571CamPreviewStartStop(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop)
{

}


static int st7571Init(lcdDev_t *lcd, lspiCbEvent_fn cb)
{
    return 0;
}

static int  st7571DrawPoint(lcdDev_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    return 0;
}

static int st7571Fill(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength)
{
    return 0;
}

static int stPrepareDisplay(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey)
{
    return 0;
}

static int st7571OnOff(lcdDev_t* lcd, uint8_t sta)
{
    return 0;
}

static void st7571BackLight(lcdDev_t* lcd, uint8_t sta)
{
}

void st7571BuildDmaChain(lcdDev_t* lcd, uint8_t* dataBuf, const uint16_t chainCnt,
                                bool needStop)
{    
    lcdDmaTxCfg7571.sourceAddress = (void *)dataBuf;


}

void st7571StartStop(lcdDev_t* lcd, bool startOrStop)
{
    if (startOrStop)
    {
        DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, lcdDmaTxCh7571, lcdDmaTxDesc7571);
    }
    else
    {    
        extern void DMA_stopChannelNoWait(DmaInstance_e instance, uint32_t channel);
        DMA_stopChannelNoWait(DMA_INSTANCE_MP, lcdDmaTxCh7571);
    }

}

void st7571Clear(lcdDev_t* lcd, uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength)
{

}

lcdDrvFunc_t st7571Drv = 
{
    .id             = 0x7571,

    .init           = st7571Init,
    .drawPoint      = st7571DrawPoint,
    .fill           = st7571Fill,
    .prepareDisplay = stPrepareDisplay,
    .onoff          = st7571OnOff,
    .backLight      = st7571BackLight,
    .startStop      = st7571StartStop,
    .clear          = st7571Clear,
    .startStopPreview = st7571CamPreviewStartStop,
};



