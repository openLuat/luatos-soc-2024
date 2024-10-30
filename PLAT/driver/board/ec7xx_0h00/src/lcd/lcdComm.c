#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdarg.h>

#include "bsp.h"
#include "bsp_custom.h"
#include "lspi.h"
#include "lcdDrv.h"
#include "lcdComm.h"
#include "timer.h"
#ifdef FEATURE_OS_ENABLE
#include "osasys.h"
#include "cmsis_os2.h"
#endif


extern void slpManAONIOPowerOn(void);
lspiDrvInterface_t *lcdDrv  = &lspiDrvInterface2; 
int32_t lcdDmaTxCh          = 0;
static lcdUspCb userUspCb   = NULL;
static lcdUspCb userDmaCb   = NULL;
DmaDescriptor_t __ALIGNED(16) lcdDmaTxDesc[DMA_DESC_MAX];
DmaTransferConfig_t lcdDmaTxCfg =
{
    NULL,
    (void *)&(LSPI2->TFIFO),
    DMA_FLOW_CONTROL_TARGET,
    DMA_ADDRESS_INCREMENT_SOURCE,
    DMA_DATA_WIDTH_FOUR_BYTES,
    DMA_BURST_32_BYTES, 
    DMA_BULK_NUM
};

void lcdDrvDelay(uint32_t ms)
{
#ifdef FEATURE_OS_ENABLE
    if (ms < 1)
    {
        osDelay(1);
    }
    else
    {
        osDelay(ms);
    }
#else
    extern void delay_us(uint32_t us);
    delay_us(ms*1000);
#endif
}

static void uspIrqCb(void)
{
    LSPI2->STAS |= (1<<31);

    if (userUspCb) 
    {
        userUspCb();
    }
}

static void dmaIrqCb(uint32_t event)
{
    switch(event)
    {
        case DMA_EVENT_END:
        {           
            //isFrameDmaWaiting = false;

            if(userDmaCb)
            {
                userDmaCb(event);
            } 
            
            break;
        }
            
        case DMA_EVENT_ERROR:
            break;
        default:
            break;
    }
}

int dmaInit(lcdDmaCb cb)
{
    if (lcdDmaTxCh) 
    {
        DMA_closeChannel(DMA_INSTANCE_MP,lcdDmaTxCh);
    }
    
    lcdDmaTxCh = DMA_openChannel(DMA_INSTANCE_MP);
    if (ARM_DMA_ERROR_CHANNEL_ALLOC == lcdDmaTxCh)
    {
        EC_ASSERT(0,0,0,0);
    }
    
    if(cb) 
    {
        userDmaCb = cb;
    }
    
    DMA_setChannelRequestSource(DMA_INSTANCE_MP, lcdDmaTxCh, DMA_REQUEST_USP2_TX);
    DMA_rigisterChannelCallback(DMA_INSTANCE_MP, lcdDmaTxCh, dmaIrqCb);
    DMA_enableChannelInterrupts(DMA_INSTANCE_MP, lcdDmaTxCh, DMA_END_INTERRUPT_ENABLE);
    return lcdDmaTxCh;
}

void dmaStartStop(bool start)
{
    if(start)
    {
        DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, lcdDmaTxCh, lcdDmaTxDesc);
    }
    else
    {    
        extern void DMA_stopChannelNoWait(DmaInstance_e instance, uint32_t channel);
        DMA_stopChannelNoWait(DMA_INSTANCE_MP, lcdDmaTxCh);
    }
}

void lcdEnable()
{   
    lspiBusSel.lspiBusEn = 1; // choose lspi bus
    lspiCtrl.enable      = 1;
    lcdDrv->ctrl(LSPI_CTRL_BUS_SEL, 0);
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
}

#if defined CHIP_EC719

void lcdMspiSet(uint8_t enable, uint8_t addrLane, uint8_t dataLane, uint8_t instruction)
{
    lspiMspiCtrl.mspiEn         = enable;
    lspiMspiCtrl.mspiAddrLane   = addrLane;
    lspiMspiCtrl.mspiDataLane   = dataLane;
    lspiMspiCtrl.mspiInst       = instruction;
    lcdDrv->ctrl(LSPI_MSPI_CTRL, 0);
}

void lcdMspiHsyncSet(uint8_t hsyncAddr, uint8_t hsyncInst, uint16_t vbpNum, uint16_t vfpNum)
{
    lspiVsyncCtrl.hsyncAddr     = hsyncAddr;
    lspiVsyncCtrl.hsyncInst     = hsyncInst;
    lspiVsyncCtrl.vbp           = vbpNum;
    lspi8080Ctrl.vfp            = vfpNum;
    lcdDrv->ctrl(LSPI_VSYNC_CTRL, 0);
    lcdDrv->ctrl(LSPI_8080_CTRL, 0);
}

void lcdMspiVsyncSet(uint8_t vsyncEnable, uint8_t vsyncInst, uint8_t lspiDiv)
{
    lspiMspiCtrl.mspiVsyncEn    = vsyncEnable;
    lspiMspiCtrl.mspiInst       = vsyncInst;
    lspiMspiCtrl.vsyncLineCycle = (40000*70)/(1000*lspiDiv) + 100;
    lcdDrv->ctrl(LSPI_MSPI_CTRL, 0);
}

void lcdCsnHighCycleMin(uint8_t lspiDiv)
{  
    lspiCmdAddr.csnHighCycleMin = (500*70)/(1000*lspiDiv) + 10;
    lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
}
#endif

void lcdInterfaceType(uint8_t type)
{   
#if defined CHIP_EC718
    switch(type)
    {
        case SPI_3W_I:
            lspiCtrl.busType = 0;
            lspiCtrl.line4 = 0;
            break;
        case SPI_3W_II:
            lspiCtrl.busType = 1;
            lspiCtrl.line4 = 0;
            break;   
        case SPI_4W_I:
            lspiCtrl.busType = 0;
            lspiCtrl.line4 = 1;
            break; 
        case SPI_4W_II:
            lspiCtrl.busType = 1;
            lspiCtrl.line4 = 1;
            break; 
        default:
            lspiCtrl.busType = 1;
            lspiCtrl.line4 = 1;
    }
    
#if (SPI_2_DATA_LANE == 1)
    lspiCtrl.data2Lane = 1;
#endif    
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
    
#else // CHIP 719
    switch(type)
    {
        case SPI_3W_I:
            lspiCmdAddr.busType = 0;
            lspiCtrl.line4 = 0;
            break;
        case SPI_3W_II:
            lspiCmdAddr.busType = 1;
            lspiCtrl.line4 = 0;
            break;   
        case SPI_4W_I:
            lspiCmdAddr.busType = 0;
            lspiCtrl.line4 = 1;
            break; 
        case SPI_4W_II:
            lspiCmdAddr.busType = 1;
            lspiCtrl.line4 = 1;
            break; 
        case MSPI_4W_II:
            #if (LCD_INTERFACE_MSPI == 1)
            lcdMspiSet(1, 0, 0, DEFAULT_INST);
            #endif
            break;
        case INTERFACE_8080:
            // config cpol cpha
            *(uint32_t*)0x4d042028 |= (1<<4) | (1<<5);
            lspi8080Ctrl.lspi8080En = 1;
            break;
        default:
            lspiCmdAddr.busType = 1;
            lspiCtrl.line4 = 1;
    }
    
#if (SPI_2_DATA_LANE == 1)
    lspiCtrl.dspiEn = 1;
#endif 

    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
    lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
    lcdDrv->ctrl(LSPI_8080_CTRL, 0);  
#endif    
}

int lspiDefaultCfg(lcdDrvFunc_t *lcd, lcdUspCb cb, uint32_t freq, uint8_t bpp)
{
    lcdDrv->deInit();
    lcdDrv->init();
    lcdDrv->powerCtrl(LSPI_POWER_FULL);

    lcdDrv->ctrl(LSPI_CTRL_BUS_SPEED, freq);
    lspiCtrl.datSrc = 1; // 0: data from camera; 1: data from memory
    
    switch (bpp)
    {
        case 12: // rgb444
        {
            lspiCtrl.colorModeIn  = 3;   // RGB565
            lspiCtrl.colorModeOut = 0;   // RGB444
        }
        break;

        case 16: // rgb565
        {
            lspiCtrl.colorModeIn  = 3;   // RGB565
            lspiCtrl.colorModeOut = 1;   // RGB565
        }
        break;

        case 18: // rgb666
        {
            lspiCtrl.colorModeIn  = 3;   // RGB565
            lspiCtrl.colorModeOut = 2;   // RGB565
        }
        break;

#if defined CHIP_EC719
        case 24: // rgb888
        {
            lspiCtrl.colorModeIn  = 4;   // RGB888
            lspiCtrl.colorModeOut = 6;   // RGB888
        }
        break;
#endif
        case 1:
        break;

        case 2:
        break;

        case 8:
        break;

        default:
        EC_ASSERT(0,0,0,0);
        break;
    }

    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
#if defined CHIP_EC719
    lcdDrv->ctrl(LSPI_PRE_PARA0_CTRL, 0);
    lcdDrv->ctrl(LSPI_POST_PARA0_CTRL, 0);
#endif    
    lcdEnable();
   
    if(cb)
    {
        userUspCb = cb;
        lspiIntCtrl.lspiRamWrEndEn  = 1;
        lcdDrv->ctrl(LSPI_CTRL_INT_CTRL, 0);
        XIC_SetVector(PXIC0_USP2_IRQn, uspIrqCb);
        XIC_EnableIRQ(PXIC0_USP2_IRQn);	
    } 

    lspiDataFmt.wordSize	        = 31;
    lspiDataFmt.txPack		        = 0;
    lspiInfo.frameHeight	        = lcd->height;  // frame input height
    lspiInfo.frameWidth		        = lcd->width;   // frame input width
    lspiFrameInfoOut.frameHeightOut = lcd->height;  // frame output height
    lspiFrameInfoOut.frameWidthOut 	= lcd->width;   // frame output width
    lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
    lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
    lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO_OUT, 0);
    return 0;
}


int lcdDmaTrans(lcdDrvFunc_t *lcd, void *sourceAddress, uint32_t totalLength)
{
    uint32_t res = 0, patch = 0, tmp = 0;
    int dmaChainCnt = 0, ret = 0;
    uint8_t version = *(uint32_t*)0x4d0420f4;

    if(lcd == NULL || sourceAddress == NULL) 
    {
        EC_ASSERT(0,0,0,0);
    }

    res = totalLength;
    
    if( totalLength % 4) 
    {
        res += (4 - totalLength%4);
    }

    // step1: config lspi
    lspiDmaCtrl.txDmaReqEn          = 1;
    lspiDmaCtrl.dmaWorkWaitCycle    = 15;
    lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
    lspiCmdCtrl.wrRdn               = 1; // 1: wr   0: rd
    lspiCmdCtrl.ramWr               = 1; // start ramwr    
#if defined CHIP_EC719 // CHIP 719
    lspiCmdCtrl.ramWrHaltMode       = 1; // maintain cs as low between cmd and data
#endif

    if (version != 0xb2)
    {
        #if (SPI_2_DATA_LANE == 1)
        tmp = res/2;
        #endif
        
        if (tmp >= 0x3ffff)
        {
            lspiCmdCtrl.dataLen = 0x3ffff; // infinate
        }
        else
        {
            #if (SPI_2_DATA_LANE == 1)
            lspiCmdCtrl.dataLen = res/2;
            #else
            lspiCmdCtrl.dataLen = res;
            #endif
        }
    }
    else // 0xb2, B2 chip
    {
#if defined CHIP_EC719
        if (totalLength >= 0x3fffff)
        {
            lspiCmdCtrl.dataLen = 0x3fffff; // infinate
        }
        else
        {
            #if (SPI_2_DATA_LANE == 1)
            lspiCmdCtrl.dataLen = res/2;
            #else
            if (lcd->bpp == 24)
            {
                lspiCmdCtrl.dataLen = res/(lcd->bpp/8+1);
            }
            else
            {
                lspiCmdCtrl.dataLen = res/(lcd->bpp/8);
            }
            #endif
        }
#endif        
    }

    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    
    // step2: config DMA
    lcdDmaTxCfg.totalLength         = DMA_BULK_NUM;
    lcdDmaTxCfg.addressIncrement    = DMA_ADDRESS_INCREMENT_SOURCE;
    lcdDmaTxCfg.targetAddress       = (void *)&(LSPI2->TFIFO);
    lcdDmaTxCfg.sourceAddress       = (void *)sourceAddress;

    patch               = DMA_BULK_NUM - res%DMA_BULK_NUM;
    dmaChainCnt         = (res + patch) / DMA_BULK_NUM;
    uint32_t package    = DMA_BULK_NUM;
    
    if (patch % dmaChainCnt == 0) 
    {
        package                 -= (patch / dmaChainCnt);   
        lcdDmaTxCfg.totalLength  = package;  
        DMA_buildDescriptorChain(lcdDmaTxDesc, &lcdDmaTxCfg, dmaChainCnt, true, true, true);
        ret                     += dmaChainCnt;
        res                     -= dmaChainCnt * package;
    }
    else
    {
        dmaChainCnt             -= 1;
        res                     -= dmaChainCnt * DMA_BULK_NUM;
        lcdDmaTxCfg.totalLength  = res;
        DMA_buildDescriptorChain(lcdDmaTxDesc, &lcdDmaTxCfg, 1, false, false, false);
        
        lcdDmaTxCfg.sourceAddress = (void *)sourceAddress + ret * DMA_BULK_NUM + res;
        lcdDmaTxCfg.totalLength   = DMA_BULK_NUM;
        DMA_buildDescriptorChain(lcdDmaTxDesc+1, &lcdDmaTxCfg, dmaChainCnt, true, true, true);
        ret += dmaChainCnt;
    }  

    //GPIO_pinWrite(0, 1 << 12, 0);
    dmaStartStop(true);
    return ret;
}

void lspiFifoWrite(uint32_t data)
{
    lspiDmaCtrl.txDmaReqEn = 0;
    lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);

    lspiCmdCtrl.wrRdn   = 1; // 1: wr   0: rd
    lspiCmdCtrl.ramWr   = 1;
    lspiCmdCtrl.dataLen = 2;
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    LSPI2->STAS |= (1<<31);
    
    if(lspiDataFmt.txFifoEndianMode)
    {
        LSPI2->TFIFO =  (data >> 16)|(data << 16);
    }
    else 
    {
        LSPI2->TFIFO = data;
    }
}

static void leftShift1Bit(uint8_t *data, uint8_t dataLen)
{        
    #define READ_REG_MSB 0x80
    #define READ_REG_LSB 0x01

    for (int i = 1; i <= dataLen; i++)
    {
        data[i-1] = data[i-1] << 1;

        if (i < dataLen && data[i] & READ_REG_MSB)
        {
            data[i-1] = data[i-1] | READ_REG_LSB;
        }
    }
}

// dataLen includes dummyLen
void lspiReadReg(uint8_t addr, uint8_t *data, uint16_t dataLen, uint8_t dummyCycleLen)
{
    if( data == NULL) return;
    uint8_t index=0, remainder=0, outputLen=dataLen, tmp1[20]={0};


    uint8_t version = *(uint32_t*)0x4d0420f4;

    if (version != 0xb2) // b1 chip
    {
#if (LCD_AXS15231_ENABLE == 1)
        PadConfig_t config1;
        // config gpio36
        PAD_getDefaultConfig(&config1);
        config1.mux = RTE_USP2_DIN_FUNC;
        PAD_setPinConfig(RTE_USP2_DIN_PAD_ADDR, &config1);
        lspiCtrl.busType            = 1; // 0: type1, sda inout;   1:type2, sda input, sdo output
#endif
    
        lspiCmdAddr.addr            = addr;
        lspiCmdCtrl.wrRdn           = 0;      // 1: wr   0: rd
        lspiCmdCtrl.dataLen         = dataLen;
        lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
        while (!LSPI2->LSPI_STAT);
       
        for (int i = 0; i < dataLen; i++)
        {
            tmp1[i] = LSPI2->RFIFO;
        }

        if (dummyCycleLen > 0)
        {
            index       = dummyCycleLen / 8;
            remainder   = dummyCycleLen % 8;
            outputLen   = dataLen - (index + ((remainder > 0)? 1 : 0));
            
            memcpy(&tmp1[0], &tmp1[index], outputLen + ((remainder > 0)? 1 : 0));
            
            for (int j = 0; j < remainder; j++)
            {
                leftShift1Bit(tmp1, outputLen+1);
            }
        }

        memcpy(data, tmp1, outputLen);        
    }
    else // b2 chip
    {
        uint32_t dummyRead;
        
        if (LCD_INTERFACE == MSPI_4W_II)
        {
            #if (LCD_INTERFACE_MSPI == 1)
            // read test
            lcdMspiSet(1, 0, 0, 0x03); // 1wire read: 0x03
            
            // config gpio36
            //PadConfig_t config;
            PadConfig_t config2;
            PAD_getDefaultConfig(&config2);
            config2.mux = RTE_USP2_SDI_FUNC;
            PAD_setPinConfig(RTE_USP2_SDI_PAD_ADDR, &config2);
            
            lspiCmdAddr.addr            = addr;
            //lspiCmdAddr.busType         = 1;
            lspiCmdCtrl.wrRdn           = 0;      // 1: wr   0: rd
            lspiCmdCtrl.rdatDummyCycle  = dummyCycleLen;
            lspiCmdCtrl.dataLen         = 1;
            lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
            lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);

            while ((LSPI2->LSPI_STAT & LSPI_STATS_RX_FIFO_LEVEL_Msk) == 0);

            for (int j = 0; j < dummyCycleLen/8; j++)
            {
                dummyRead = LSPI2->RFIFO;
                (void)dummyRead;
            }

            for (int i = 0; i < dataLen; i++)
            {
                data[i] = LSPI2->RFIFO;
            }
            #endif
        }
        else
        {
            lspiCmdAddr.addr            = addr;
            lspiCmdCtrl.wrRdn           = 0;      // 1: wr   0: rd
            lspiCmdCtrl.rdatDummyCycle  = dummyCycleLen;
            lspiCmdCtrl.dataLen         = dataLen;            
            lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
            lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
            while (!LSPI2->LSPI_STAT);

            for (int j = 0; j < dummyCycleLen/8; j++)
            {
                dummyRead = LSPI2->RFIFO;
                (void)dummyRead;
            }
            
            for (int i = 0; i < dataLen; i++)
            {
                data[i] = LSPI2->RFIFO;
            }
        }
    }
}   

void lspiReadRam(uint32_t *data, uint32_t dataLen)
{
    if (data == NULL) return;

    uint8_t version = *(uint32_t*)0x4d0420f4;

    if (version != 0xb2)
    {
        lspiCmdAddr.addr            = 0x3E; // RDMEMC
        lspiCmdCtrl.wrRdn           = 0;    
        lspiCmdCtrl.rdatDummyCycle  = 1;
        lspiCmdCtrl.dataLen         = dataLen+1;        
        lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
        lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
        while (!LSPI2->LSPI_STAT);
        uint32_t dummy = LSPI2->RFIFO;
        (void)dummy;
        for (int i = 0; i < dataLen; i++)
        {
            data[i] = LSPI2->RFIFO;
        }
    }
    else // b2 chip
    {
        
    }
}

void lcdWriteData(uint8_t data)
{
    lcdDrv->prepareSend(data);
}

void lcdWriteCmd(uint8_t cmd)
{
    lspiCmdAddr.addr = cmd;
    lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
}

void lspiCmdSend(uint8_t cmd, uint8_t *data, uint8_t dataLen)
{
    lspiCmdAddr.addr = cmd;
    lcdDrv->ctrl(LSPI_CTRL_CMD_ADDR, 0);
    
    for (uint8_t i = 0; i < dataLen; i++)
    {
        lcdDrv->prepareSend(data[i]);
    }
    lcdDrv->send(NULL, 0);
}

void lcdRst(uint32_t highMs, uint32_t lowMs)
{     
    LCD_RST_LOW;
    lcdDrvDelay(lowMs);
    LCD_RST_HIGH;
    lcdDrvDelay(highMs);
}


#if (BK_USE_GPIO == 1)
void lcdGpioBkLevel(uint8_t level)
{
    if (level > 0)
    {
        GPIO_pinWrite(LCD_BK_GPIO_INSTANCE, 1 << LCD_BK_GPIO_PIN, 1 << LCD_BK_GPIO_PIN);
        
    }
    else
    {
        GPIO_pinWrite(LCD_BK_GPIO_INSTANCE, 1 << LCD_BK_GPIO_PIN, 0);
    }
}
#endif

#if (BK_USE_PWM == 1)
volatile static uint32_t gpwmCnt = 0;
static bool isPwmOn = false;

uint32_t millis(void)
{
    return gpwmCnt;
}

static void pwmISR(void)
{
    if (TIMER_getInterruptFlags(LCD_PWM_INSTANCE) & TIMER_MATCH0_INTERRUPT_FLAG)
    {
		TIMER_clearInterruptFlags(LCD_PWM_INSTANCE, TIMER_MATCH0_INTERRUPT_FLAG);
		gpwmCnt++;
		
        #ifdef FEATURE_SUBSYS_GUI_LVGL_ENABLE
        lv_tick_inc(1);
        #endif
    }
}

// level: 0~100
uint8_t lcdPwmBkLevel(uint8_t level)
{
    if (level == 0)
    {
        TIMER_stop(LCD_PWM_INSTANCE);
        goto END;
    }

    if (level > 100)
    {
        level = 100;
    }

    // turn on TIMER first
    if (!isPwmOn)
    {
        TIMER_start(LCD_PWM_INSTANCE);
        isPwmOn = true;
    }

    TIMER_updatePwmDutyCycle(LCD_PWM_INSTANCE, level);

END:
    return level;
}

static void lcdPwmBkInit(void)
{
    PadConfig_t config;
    PAD_getDefaultConfig(&config);
    config.mux              = LCD_PWM_PAD_ALT_SEL;
    config.driveStrength    = PAD_DRIVE_STRENGTH_LOW;
    PAD_setPinConfig(LCD_PWM_OUT_PAD, &config);
    CLOCK_setClockSrc(LCD_PWM_CLOCK_ID, LCD_PWM_CLOCK_SOURCE);
    CLOCK_setClockDiv(LCD_PWM_CLOCK_ID, 1);
	TIMER_driverInit();
    TimerPwmConfig_t gLcdPwmConfig;
    gLcdPwmConfig.pwmFreq_HZ        = 1000;
    gLcdPwmConfig.srcClock_HZ       = GPR_getClockFreq(LCD_PWM_CLOCK_ID); 
    gLcdPwmConfig.stopOption        = TIMER_PWM_STOP_LOW;
    gLcdPwmConfig.dutyCyclePercent  = 0;

    TIMER_setupPwm(LCD_PWM_INSTANCE, &gLcdPwmConfig);

    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH0_INTERRUPT, TIMER_INTERRUPT_LEVEL);
    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH1_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH2_INTERRUPT, TIMER_INTERRUPT_DISABLE);

    XIC_SetVector(LCD_PWM_INSTANCE_IRQ, pwmISR);
    XIC_EnableIRQ(LCD_PWM_INSTANCE_IRQ);
}

void lcdPwmBkDeInit(void)
{
    TIMER_stop(LCD_PWM_INSTANCE);
    TIMER_deInit(LCD_PWM_INSTANCE);
}

#endif

#if defined CHIP_EC718
void calTe(uint32_t totalBytes, uint16_t sy)
{   
    //uint32_t timeStampApp= 0;
    uint32_t teRunTimeMs = 0;
    uint16_t yte = 0;
    //uint32_t controllerBytesPerMs = 480*320*2/28;
    uint16_t waitTimeMs = 0;
    
    // cal te
#if 0
    timeStampApp = TIMER_getCount(0);
    
    if (timeStampApp < timeStampTe)
    {
        teRunTimeMs = (26000 - timeStampTe + timeStampApp) / 1000;
    }
    else
    {
        teRunTimeMs = (timeStampApp - timeStampTe) / 1000;
    }
#else
#if (BK_USE_PWM == 1)
        teRunTimeMs = millis();
#endif        
#endif
    if (teRunTimeMs > 16)
    {
        teRunTimeMs = 0;
    }
    
    yte = teRunTimeMs * 40 - 1;

    

    if (teRunTimeMs > 6)
    {
        if (sy > yte)
        {
            waitTimeMs = (sy-yte)/40;
            if (waitTimeMs == 0)
            {
                waitTimeMs = 1; // at least 1ms
            }
            
#ifdef FEATURE_OS_ENABLE
            osDelay(waitTimeMs);
#endif            
            dmaStartStop(true);
        }
        else
        {
            if (sy >= (480/2))
            {
                dmaStartStop(true);
            }
            else
            {
                waitTimeMs = 16 - teRunTimeMs + sy/40; // +1
#ifdef FEATURE_OS_ENABLE                
                osDelay(waitTimeMs);
#endif                
                dmaStartStop(true);
            }
        }
    }
    else
    {
        if (sy > yte)
        {
            waitTimeMs = (sy-yte)/40;           
            if (waitTimeMs == 0)
            {
                waitTimeMs = 1; // at least 1ms
            }

#ifdef FEATURE_OS_ENABLE            
            osDelay(waitTimeMs);
#endif            
            dmaStartStop(true);
        }
        else
        {
            if (totalBytes > ((16-teRunTimeMs+12)*40*320*2))
            {
                // must wait te irq
                //osDelay(16-teRunTimeMs);
                //osEventFlagsWait(lcdEvtHandle, 0x4, osFlagsWaitAll, osWaitForever); // in te gpio isr release
                dmaStartStop(true);
            }
            else
            {
                dmaStartStop(true);
            }
        }
    }
}

#else // chip 719

#endif


void lcdIoInit(bool isAonIO)
{   
    if (isAonIO)
    {
        slpManAONIOPowerOn();
    }
    
    PadConfig_t config;
    GpioPinConfig_t gpioCfg;
    PAD_getDefaultConfig(&config);
    
    // 1. rst pin init
    config.mux = LSPI_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(LSPI_RST_GPIO_ADDR, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LSPI_RST_GPIO_INSTANCE, LSPI_RST_GPIO_PIN, &gpioCfg);

    // 2. backLight init
#if (BK_USE_GPIO == 1)
    config.mux = LCD_BK_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_BK_PAD_INDEX, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 0;
    GPIO_pinConfig(LCD_BK_GPIO_INSTANCE, LCD_BK_GPIO_PIN, &gpioCfg);
#elif (BK_USE_PWM == 1)
    lcdPwmBkInit();
#endif

	// 3. ldo pin init
#if (ENABLE_LDO == 1)
    config.mux = LCD_EN_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_EN_PAD_INDEX, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LCD_EN_GPIO_INSTANCE, LCD_EN_GPIO_PIN, &gpioCfg);
#endif    

    // 4. te init
#if defined CHIP_EC718
    // 4.1 718 te init
    
#else
    // 4.2 719 te init
    config.mux = LCD_TE_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_TE_PAD_INDEX, &config);
#endif

    // test fill one frame need how much time
    #if 0
    config.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(27, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(0, 12, &gpioCfg);
    #endif
}


