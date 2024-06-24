#ifdef FEATURE_DRIVER_LCD_ENABLE
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

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#if ( LCD_WIDTH*LCD_HEIGHT/LCD_SPI_DATA_LANE > 0x3FFFF)
#error "The LSPI one-time transfer limit < 0x3FFFF"
#endif
#ifdef LCD_BL_PWM_ENABLE
static uint32_t cost_ms = 0;
volatile uint32_t dma_cost_ms = 0;
volatile uint32_t bus_cost_ms = 0;
#endif
extern void delay_us(uint32_t us);

extern lspiDrvInterface_t lspiDrvInterface2;
lspiDrvInterface_t *lcdDrv = &lspiDrvInterface2; 
static uint32_t lspi_freq = 13*1024*1024;   //real 12.44MHz
static int32_t lcdDmaTxCh = 0; // dma tx channel
static bool sDmaTxRunning = false;
#define DMA_BULK_NUM                (1023*8)
#define DMA_DESC_MAX                (40)

DmaDescriptor_t __ALIGNED(16) lcdDmaTxDesc[DMA_DESC_MAX]; // here set maximum dma descriptor will be no more then 20
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

/**
  \fn          
  \brief       
  \return
*/
static bool isFrameLspitransDone = false;
static lcd_usp_cb user_usp_cb = NULL;
static void spiEventHandler(void)
{
    LSPI2->STAS |= (1<<31);
    isFrameLspitransDone = true;
    if(user_usp_cb) user_usp_cb(LSPI2->STAS);
    #ifdef LCD_BL_PWM_ENABLE
    bus_cost_ms = millis()-cost_ms;
    #endif
}
/**
  \fn          
  \brief       
  \return
*/
static bool isFrameDmatransDone = false;
static lcd_dma_cb user_dma_cb = NULL;
static void dmaEventHandler(uint32_t event)
{
    switch(event)
    {
        case DMA_EVENT_END:
            #ifdef LCD_BL_PWM_ENABLE
            dma_cost_ms = millis()-cost_ms;
            #endif
            isFrameDmatransDone = true;
            if(user_dma_cb){
                user_dma_cb(event);
            } 
            break;
        case DMA_EVENT_ERROR:
            break;
        default:
            break;
    }
}

/**
  \fn          
  \brief       
  \return
*/
int dmaInit(lcd_dma_cb cb)
{
    if(lcdDmaTxCh) DMA_closeChannel(DMA_INSTANCE_MP,lcdDmaTxCh);
    lcdDmaTxCh = DMA_openChannel(DMA_INSTANCE_MP);
    if (ARM_DMA_ERROR_CHANNEL_ALLOC == lcdDmaTxCh)
    {
        return 0;
    }
    if(cb) user_dma_cb = cb;
    DMA_setChannelRequestSource(DMA_INSTANCE_MP, lcdDmaTxCh, DMA_REQUEST_USP2_TX);
    DMA_rigisterChannelCallback(DMA_INSTANCE_MP, lcdDmaTxCh, dmaEventHandler);
    DMA_enableChannelInterrupts(DMA_INSTANCE_MP, lcdDmaTxCh, DMA_END_INTERRUPT_ENABLE);
    return lcdDmaTxCh;
}
/**
  \fn          
  \brief       
  \return
*/
int dmaStartStop(bool start)
{
    // if(start == sDmaTxRunning) return 0;
    if(start)
    {
        DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, lcdDmaTxCh, lcdDmaTxDesc);
        sDmaTxRunning = true;
    }
    else
    {    
        extern void DMA_stopChannelNoWait(DmaInstance_e instance, uint32_t channel);
        DMA_stopChannelNoWait(DMA_INSTANCE_MP, lcdDmaTxCh);
        sDmaTxRunning = false;
    }
}
/**
  \fn          void lcdDataLane()
  \brief        
  \return
*/
void lcdIntEnable()
{   
    lspiIntCtrl.lspiRamWrEndEn = 1;
    lcdDrv->ctrl(LSPI_CTRL_INT_CTRL, 0);
}
/**
  \fn          void lcdEnable()
  \brief        
  \return
*/
void lcdEnable()
{   
    lspiBusSel.lspiBusEn = 1;
    lcdDrv->ctrl(LSPI_CTRL_BUS_SEL, 0);
    lspiCtrl.enable = 1;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
}
/**
  \fn          void lcdInterfaceType()
  \brief        
  \return
*/
void lcdColorMode(lcdcolorModeOut_e ModeOut)
{   
    lspiCtrl.colorModeOut = ModeOut;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
}
/**
  \fn          void lcdInterfaceType()
  \brief        
  \return
*/
void lcdInterfaceType(lcdInterfaceType_e type)
{   
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
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
}
/**
  \fn          void lcdDataFormat()
  \brief        
  \return
*/
void lcdDataFormat(int bpp)
{   
    // lspiDataFmt.wordSize = 15;  
    // lspiDataFmt.txPack = 1;
    lspiDataFmt.rxPack = 1;
    lspiDataFmt.endianMode     = 1;
    lspiDataFmt.rxFifoEndianMode     = 1;
    lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
}

/**
  \fn          
  \brief        
  \return
*/
void lcdDataLane(int lane)
{   
    if(lane>1) lspiCtrl.data2Lane = 1;
    else lspiCtrl.data2Lane = 0;
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
}
/**
  \fn          void lcdSpiInit(void)
  \brief        
  \return
*/
int lcdSpiInit(uint32_t Height,uint32_t Width,lcd_usp_cb cb)
{   
    lcdDrv->deInit();
    lcdDrv->init(NULL);
    lcdDrv->powerCtrl(LSPI_POWER_FULL);
    if(lspi_freq<1024*1024){
        lspi_freq = 13*1024*1024;
    }
    lcdDrv->ctrl(LSPI_CTRL_BUS_SPEED, lspi_freq);
    lspiCtrl.datSrc = 1; // 0: data from camera; 1: data from memory
    lspiCtrl.colorModeIn = 3;   // RGB565
    lspiCtrl.colorModeOut = 1;  // RGB565
    lcdDrv->ctrl(LSPI_CTRL_CTRL, 0);
    lcdDataLane(LCD_SPI_DATA_LANE);
    lcdEnable();
   
    if(cb){
        user_usp_cb = cb;
        lcdIntEnable();
        XIC_SetVector(PXIC0_USP2_IRQn, spiEventHandler);
        XIC_EnableIRQ(PXIC0_USP2_IRQn);	
    } 
    // lspiDmaCtrl.txDmaReqEn = 1;
    // lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);

    lspiDataFmt.wordSize = 31;  
    lspiDataFmt.txPack = 0;
    // lspiDataFmt.rxPack = 1;
    // lspiDataFmt.endianMode     = 1;
    // lspiDataFmt.rxFifoEndianMode     = 1;
    lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);

    lspiInfo.frameHeight = Height;  // frame input height
    lspiInfo.frameWidth = Width;    // frame input width
    lcdDrv->ctrl(LSPI_CTRL_FRAME_INFO, 0);
    return 0;
}

/**
  \fn          void lcdDmaSend(void)
  \brief       
  \return
*/
void lcdDmaSend(uint32_t totalLength)
{
    #ifdef SPI_SPEED_MHz
    if(lspi_freq<SPI_SPEED_MHz*1024*1024){
        lspi_freq = SPI_SPEED_MHz*1024*1024;
        lcdDrv->ctrl(LSPI_CTRL_BUS_SPEED, lspi_freq);
    }
    #endif
    // lspiDataFmt.wordSize = 31;  
    // lspiDataFmt.txPack = 0;
    // lcdDrv->ctrl(LSPI_CTRL_DATA_FORMAT, 0);
    lspiDmaCtrl.txDmaReqEn = 1;
    lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
    lspiCmdCtrl.wrRdn = 1; // 1: wr   0: rd
    lspiCmdCtrl.ramWr = 1; // start ramwr 
    if(totalLength>0x3fffe) lspiCmdCtrl.dataLen = 0x3fffe;  //262142 = 255KB + 1022 (261120)
    else lspiCmdCtrl.dataLen = totalLength;
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
}
/**
  \fn          void spidmaTrans(void)
  \brief       
  \return
*/
int spidmaTrans(lcdDev_t* lcd,void *sourceAddress,uint32_t totalLength)
{
    if(lcdDmaTxCh<=0 || lcd==NULL || sourceAddress==NULL) return 0;
    #ifdef LCD_BL_PWM_ENABLE
    cost_ms = millis()-cost_ms;
    #endif
    uint32_t reg = LSPI2->STAS; //*(uint32_t *)0x4D042000;
    int ret = 0;
    uint32_t res = totalLength;
    if(totalLength % 4) res += (4-totalLength%4);
    lcdDmaTxCfg.totalLength = DMA_BULK_NUM;
    lcdDmaTxCfg.addressIncrement = DMA_ADDRESS_INCREMENT_SOURCE;
    lcdDmaTxCfg.targetAddress = (void *)&(LSPI2->TFIFO);
    lcdDmaTxCfg.sourceAddress = (void *)sourceAddress;
    lcdDmaSend(res/LCD_SPI_DATA_LANE);
    uint32_t patch = DMA_BULK_NUM - res%DMA_BULK_NUM;
    int cnt = (res+patch)/DMA_BULK_NUM;
    uint32_t package = DMA_BULK_NUM;
    if(patch%cnt==0) 
    {
        package -= (patch / cnt);   
        lcdDmaTxCfg.totalLength = package;  
        DMA_buildDescriptorChain(lcdDmaTxDesc, &lcdDmaTxCfg, cnt, true, true, true);
        ret += cnt;
        res -= cnt * package;
    }
    else
    {
        cnt -= 1;
        res -= cnt * DMA_BULK_NUM;
        lcdDmaTxCfg.totalLength = res;
        DMA_buildDescriptorChain(lcdDmaTxDesc, &lcdDmaTxCfg, 1, false, false, false);
        lcdDmaTxCfg.sourceAddress = (void *)sourceAddress + ret * DMA_BULK_NUM + res;
        lcdDmaTxCfg.totalLength = DMA_BULK_NUM;
        DMA_buildDescriptorChain(lcdDmaTxDesc+1, &lcdDmaTxCfg, cnt, true, true, true);
        ret += cnt;
    }
    dmaStartStop(true);
    // DMA_loadChannelDescriptorAndRun(DMA_INSTANCE_MP, lcdDmaTxCh, lcdDmaTxDesc);
    #ifdef LCD_BL_PWM_ENABLE
    // LCD_TRACE(spidmaTrans, 8, "%d,%d;%dx%d+%d=%d,0x%X->0x%X",  \
    //         cost_ms, bus_cost_ms, package, ret, res, totalLength, reg, LSPI2->STAS);
    cost_ms = millis();
    #endif
    return ret;
}

/**
  \fn          
  \brief        
  \return
*/
int lspiWrite(uint32_t data)
{
    //if (times == 0)
    {    
        lspiDmaCtrl.txDmaReqEn = 0;
        lcdDrv->ctrl(LSPI_CTRL_DMA_CTRL, 0);
    }

    lspiCmdCtrl.wrRdn = 1;      // 1: wr   0: rd
    lspiCmdCtrl.ramWr = 1;      //从内存写入，不填渐变
    lspiCmdCtrl.dataLen = 2;    //实际数据长度，错误黑色
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    LSPI2->STAS |= (1<<31);
    if(lspiDataFmt.txFifoEndianMode){
        LSPI2->TFIFO =  (data >> 16)|(data << 16);
    }
    else LSPI2->TFIFO = data;
}
        
/**
  \fn          
  \brief        
  \return
*/
void lspiReadReg(uint8_t addr,uint32_t *data,uint16_t num)
{
    if(data==NULL) return;
    if(lspi_freq>13*1024*1024){
        lspi_freq = 13*1024*1024;
        lcdDrv->ctrl(LSPI_CTRL_BUS_SPEED, lspi_freq);
    }
    LSPI2->LSPI_CADDR = addr;
    lspiCmdCtrl.wrRdn = 0;      // 1: wr   0: rd
    lspiCmdCtrl.rdatDummyCycle = 1;
    lspiCmdCtrl.dataLen = num+1;    
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    while (!LSPI2->LSPI_STAT);
    uint32_t dummy = LSPI2->RFIFO;  //add 1 dummy byte
    for (int i = 0; i < num; i++)
    {
        data[i] = LSPI2->RFIFO;
    }
    // uint32_t fmt = *(uint32_t*)&lspiDataFmt;
    // printf("%08X,freq%d,0x%08X,x%X,x%X\r\n",LSPI2->STAS,(lspi_freq/(1024*1024)),fmt,dummy,data[0]);
}   

/**
  \fn          
  \brief        
  \return
*/
void lspiReadRam(uint32_t *data,uint32_t num)
{
    if(data==NULL) return;
    if(lspi_freq>13*1024*1024){
        lspi_freq = 13*1024*1024;
        lcdDrv->ctrl(LSPI_CTRL_BUS_SPEED, lspi_freq);
    }
    LSPI2->LSPI_CADDR = 0x3E;   //RDMEMC
    lspiCmdCtrl.wrRdn = 0;    
    lspiCmdCtrl.rdatDummyCycle = 1;
    lspiCmdCtrl.dataLen = num+1;    
    lcdDrv->ctrl(LSPI_CTRL_CMD_CTRL, 0);
    while (!LSPI2->LSPI_STAT);
    uint32_t dummy = LSPI2->RFIFO;
    for (int i = 0; i < num; i++)
    {
        data[i] = LSPI2->RFIFO;
    }
}
/**
  \fn          
  \brief        
  \return
*/

int spiPreviewMode(lcdDev_t *lcd)
{

}

/**
  \fn          void lcdWriteData(void)
  \brief       wait for dma transfer.
  \return
*/
void lcdWriteData(uint8_t data)
{
    lcdDrv->prepareSend(data);
}

void lcdWriteCmd(uint8_t cmd)
{
    LSPI2->LSPI_CADDR   = cmd; 
}
/**
  \fn          
  \brief      only for USP2,no need for index
  \return
*/
void lspiCmdSet(uint8_t cmd,uint8_t data)
{
    LSPI2->LSPI_CADDR   = cmd; 
    lcdDrv->prepareSend(data);
    lcdDrv->send(NULL, 0);
}
/**
  \fn          
  \brief      only for USP2,no need for index
  \return
*/
void lspiCmdSend(uint8_t cmd,uint8_t *data,uint8_t allbytes)
{
    LSPI2->LSPI_CADDR   = cmd;
    for (uint8_t i = 0; i < allbytes; i++)
    {
        lcdDrv->prepareSend(data[i]);
    }
    lcdDrv->send(NULL, 0);
}



void mDelay(uint32_t ms)
{
    delay_us(ms * 1000);
    // osDelay(ms);
}

uint8_t lcdReadData()
{
    return 0;
}
/**
  \fn          
  \brief    
  \return
*/
void lcdGpioInit(lcdDev_t *lcd)
{   
    slpManAONIOPowerOn();
    PadConfig_t config;
    PAD_getDefaultConfig(&config);
    config.pullUpEnable = PAD_PULL_UP_DISABLE;
	config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    #ifdef LCD_RST_PAD_INDEX
    config.mux = LCD_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_RST_PAD_INDEX, &config);
    # elif defined(LSPI_RST_GPIO_ADDR)
    config.mux = LSPI_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(LSPI_RST_GPIO_ADDR, &config);
    #endif

    GpioPinConfig_t gpioCfg;
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    #ifdef LCD_RST_GPIO_PIN
    GPIO_pinConfig(LCD_RST_GPIO_INSTANCE, LCD_RST_GPIO_PIN, &gpioCfg); 
    # elif defined(LSPI_RST_GPIO_PIN)
    GPIO_pinConfig(LSPI_RST_GPIO_INSTANCE, LSPI_RST_GPIO_PIN, &gpioCfg);
    #endif

    #ifdef LCD_EN_PAD_INDEX
    config.mux = LCD_EN_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_EN_PAD_INDEX, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LCD_EN_GPIO_INSTANCE, LCD_EN_GPIO_PIN, &gpioCfg); 
    #endif
}

#ifdef FEATURE_MES_LCD_ENABLE
/**
  \fn          
  \brief    
  \return
*/
void measure_init(void)
{
    CLOCK_setClockSrc(FCLK_TIMER1, FCLK_TIMER1_SEL_26M);
    CLOCK_setClockDiv(FCLK_TIMER1, 1);
    TIMER_driverInit();
    TimerConfig_t timerConfig;
    TIMER_getDefaultConfig(&timerConfig);
    timerConfig.initValue = 0;
    timerConfig.reloadOption = TIMER_RELOAD_ON_MATCH0;
    timerConfig.match0 = -1;
    TIMER_init(1, &timerConfig);
    TIMER_interruptConfig(1, TIMER_MATCH0_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(1, TIMER_MATCH1_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(1, TIMER_MATCH2_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_start(1);
}

uint32_t measure_time(uint32_t *val) 
{
    TIMER_stop(1);
    uint32_t ret = TIMER_getCount(1); 
    if(val != NULL) {
        *val = ret;
        // SYSLOG_PRINT(SL_INFO, "--%d\r\n",*val);
    }
    ret /= 26;
    // TIMER_setInitValue(1, 0);
    TIMER_start(1);
    return ret;
}
/**
  \fn          
  \brief 
  \return
*/
#ifdef FEATURE_SUBSYS_STORAGE_ENABLE
#include "storage.h"
const char *measurecsv = "D:/timelog.csv";
const char *csvtitle = "index,function,timestamp,used(ms)\n";

lfs_file_t file = {0};
static uint32_t timeindex = 0;
static void measure_csv_export(void *func, uint32_t usedtime)
{
    struct stat buf = {0};
    char csvBuf[64] = {0};
    FILE *measurefile = file_fopen(measurecsv, "a+");
    if (measurefile != NULL)
    {
        if (file_fstat((int)measurefile, &buf) != 0)
        {
        }
        if (buf.st_size < strlen(csvtitle))
        {
            timeindex = 0;
            file_fwrite(csvtitle, strlen(csvtitle), 1,measurefile);  
        }
        timeindex ++;
        sprintf(csvBuf, "%d,%08x,%d,%d\n",timeindex,func,millis(),usedtime);
        file_fwrite(csvBuf, strlen(csvBuf), 1,measurefile);  
        file_fclose(measurefile);
    }
}
#endif
/**
  \fn          
  \brief    测量执行时间的函数  
  \return
*/
uint32_t measure_execution(void (*func)(va_list), ...)
{
    uint32_t cost_time_us = 0;
    va_list arg_ptr; 
    va_start(arg_ptr, func);
    if(func!=NULL)
    {
        measure_time(NULL);
        func(arg_ptr);
        // measure_time(&cost_time_us);
        cost_time_us = measure_time(NULL);
        #ifdef FEATURE_SUBSYS_STORAGE_ENABLE
        measure_csv_export(func,cost_time_us);
        #endif
    } 
    va_end(arg_ptr);
    return cost_time_us;
}
#endif

#ifdef LCD_BL_PWM_ENABLE
/**
  * @brief  LV_TICK_CUSTOM
  * @param  none
  * @retval timer0 ticks ms
  */
volatile static uint32_t gpwmCnt = 0;
uint32_t millis(void)
{
    return gpwmCnt;
}



/**
  \fn          
  \brief    
  \return
*/
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


int16_t s_bk_pwmlevel = LCD_BK_PWM_VAL_DEF;
static bool s_lcd_bk_inited = false;
static bool s_lcd_bk_turnon = false;
int16_t lcdBackLightVal(void)
{
    return s_bk_pwmlevel;
}
void lcdBackLightOn(bool lightOn)
{
    if(!s_lcd_bk_inited){
        lcdbkInit();
    }
	if(lightOn){
        TIMER_start(LCD_PWM_INSTANCE);
        s_lcd_bk_turnon = true ;
    }
    else{
        TIMER_stop(LCD_PWM_INSTANCE);
        s_lcd_bk_turnon = false ;
    } 
}

int16_t lcdBackLightLevel(int16_t level)
{
    if(level>=LCD_BK_PWM_VAL_MAX)
    {
        if(!s_lcd_bk_turnon) lcdBackLightOn(true);
        TIMER_updatePwmDutyCycle(LCD_PWM_INSTANCE,LCD_BK_PWM_VAL_MAX-LCD_BK_PWM_VAL_MIM); 
    }
    else if(level<LCD_BK_PWM_VAL_MIM)
    {
        lcdBackLightOn(false);
    }
    else 
    {
        if(!s_lcd_bk_turnon) lcdBackLightOn(true);
        TIMER_updatePwmDutyCycle(LCD_PWM_INSTANCE,level);
    }
    return s_bk_pwmlevel;
}

int16_t lcdBackLightUp(int16_t vol)
{
    if(vol<LCD_BK_PWM_VAL_MIM) s_bk_pwmlevel += LCD_BK_PWM_VAL_MIM;
    else s_bk_pwmlevel += vol;
    
    if(s_bk_pwmlevel>=LCD_BK_PWM_VAL_DEF){
        s_bk_pwmlevel = LCD_BK_PWM_VAL_DEF;
    } 
    lcdBackLightLevel(s_bk_pwmlevel);
    return s_bk_pwmlevel;
}
int16_t lcdBackLightDown(int16_t vol)
{
    if(vol<LCD_BK_PWM_VAL_MIM) s_bk_pwmlevel -= LCD_BK_PWM_VAL_MIM;
    else s_bk_pwmlevel -= vol;
    if(s_bk_pwmlevel<LCD_BK_PWM_VAL_MIM){
        s_bk_pwmlevel = LCD_BK_PWM_VAL_MIM;
    }
    lcdBackLightLevel(s_bk_pwmlevel);
    return s_bk_pwmlevel;
}

void lcdbkInit(void)
{
    PadConfig_t config;
    PAD_getDefaultConfig(&config);
    config.mux = LCD_PWM_PAD_ALT_SEL;
    config.driveStrength = PAD_DRIVE_STRENGTH_LOW;
    PAD_setPinConfig(LCD_PWM_OUT_PAD, &config);
    CLOCK_setClockSrc(LCD_PWM_CLOCK_ID, LCD_PWM_CLOCK_SOURCE);
    CLOCK_setClockDiv(LCD_PWM_CLOCK_ID, 1);
	TIMER_driverInit();
    TimerPwmConfig_t gLcdPwmConfig;
    gLcdPwmConfig.pwmFreq_HZ = 1000;
    gLcdPwmConfig.srcClock_HZ = GPR_getClockFreq(LCD_PWM_CLOCK_ID); 
    gLcdPwmConfig.stopOption = TIMER_PWM_STOP_LOW;
    gLcdPwmConfig.dutyCyclePercent = LCD_BK_PWM_VAL_MAX;

    TIMER_deInit(LCD_PWM_INSTANCE);
    TIMER_setupPwm(LCD_PWM_INSTANCE, &gLcdPwmConfig);

    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH0_INTERRUPT, TIMER_INTERRUPT_LEVEL);
    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH1_INTERRUPT, TIMER_INTERRUPT_DISABLE);
    TIMER_interruptConfig(LCD_PWM_INSTANCE, TIMER_MATCH2_INTERRUPT, TIMER_INTERRUPT_DISABLE);

    XIC_SetVector(LCD_PWM_INSTANCE_IRQ, pwmISR);
    XIC_EnableIRQ(LCD_PWM_INSTANCE_IRQ);
    s_lcd_bk_inited = true;
}

void lcdPwmDeInit(void)
{
    TIMER_stop(LCD_PWM_INSTANCE);
    TIMER_deInit(LCD_PWM_INSTANCE);
}


#else
void lcdbkInit(void)
{
    slpManAONIOPowerOn();
    #ifdef LCD_BK_PAD_INDEX
    PadConfig_t config= {0};
    PAD_getDefaultConfig(&config);
    config.pullUpEnable = PAD_PULL_UP_DISABLE;
	config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    config.mux = LCD_BK_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_BK_PAD_INDEX, &config);
    #endif
    #ifdef LCD_BK_GPIO_PIN
    GpioPinConfig_t gpioCfg= {0};
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LCD_BK_GPIO_INSTANCE, LCD_BK_GPIO_PIN, &gpioCfg); 
    #endif
}
int16_t lcdBackLightVal(void)
{
    return 100;
}
int16_t lcdBackLightLevel(int16_t level)
{

}
int16_t lcdBackLightUp(int16_t vol)
{

}
int16_t lcdBackLightDown(int16_t vol)
{

}

#endif
#endif 