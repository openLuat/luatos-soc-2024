 /****************************************************************************
  *
  * Copy right:   2019-, Copyrigths of AirM2M Ltd.
  * File name:    i2s.c
  * Description:  Audio use in EC718. The interface is similar with CMSIS Driver API V2.0.
  * History:      Rev1.0   2021-06-23
  *
  ****************************************************************************/
#include "i2s.h"
#include "slpman.h"

#ifdef FEATURE_OS_ENABLE
#include DEBUG_LOG_HEADER_FILE
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// I2S Setting field Start
// All the I2S's parameters that need user to set are all put here
//////////////////////////////////////////////////////////////////////////////////////////////
static DmaTransferConfig_t g_dmaTxConfig =
{
    NULL,
    NULL,
    DMA_FLOW_CONTROL_TARGET,
    DMA_ADDRESS_INCREMENT_SOURCE,
    DMA_DATA_WIDTH_FOUR_BYTES,
    DMA_BURST_32_BYTES, 
    0
};

static DmaTransferConfig_t g_dmaRxConfig =
{
    NULL,
    NULL,
    DMA_FLOW_CONTROL_SOURCE,
    DMA_ADDRESS_INCREMENT_TARGET,
    DMA_DATA_WIDTH_FOUR_BYTES,
    DMA_BURST_32_BYTES, 
    0
};


#if (RTE_I2S0)
static PIN i2s0PinMCLK  = {RTE_I2S0_MCLK_PAD_ADDR,   RTE_I2S0_MCLK_FUNC};
static PIN i2s0PinBCLK  = {RTE_I2S0_BCLK_PAD_ADDR,   RTE_I2S0_BCLK_FUNC};
static PIN i2s0PinLRCK  = {RTE_I2S0_LRCK_PAD_ADDR,   RTE_I2S0_LRCK_FUNC};
static PIN i2s0PinDin   = {RTE_I2S0_DIN_PAD_ADDR,    RTE_I2S0_DIN_FUNC};
static PIN i2s0PinDout  = {RTE_I2S0_DOUT_PAD_ADDR,   RTE_I2S0_DOUT_FUNC};
#elif (RTE_I2S1)
static PIN i2s1PinMCLK  = {RTE_I2S1_MCLK_PAD_ADDR,   RTE_I2S1_MCLK_FUNC};
static PIN i2s1PinBCLK  = {RTE_I2S1_BCLK_PAD_ADDR,   RTE_I2S1_BCLK_FUNC};
static PIN i2s1PinLRCK  = {RTE_I2S1_LRCK_PAD_ADDR,   RTE_I2S1_LRCK_FUNC};
static PIN i2s1PinDin   = {RTE_I2S1_DIN_PAD_ADDR,    RTE_I2S1_DIN_FUNC};
static PIN i2s1PinDout  = {RTE_I2S1_DOUT_PAD_ADDR,   RTE_I2S1_DOUT_FUNC};
#endif


// Sample rate, fs, fracdiv part, integer part. Generate MClk accoding to the sample rate.
const uint32_t i2sSampleRateTbl[][4] = 
{
    {SAMPLE_RATE_8K,    256, 0,        0x32},   // 256fs, 8k
    {SAMPLE_RATE_16K,   256, 0,        0x19},   // 256fs, 16k
    {SAMPLE_RATE_22_05K,256, 0x23fdad, 0x12},   // 256fs, 22.05k
    {SAMPLE_RATE_24K,	256, 0xaaaaaa, 0x10},   // 256fs, 24k
    {SAMPLE_RATE_32K,   256, 0x800000, 0x0C},   // 256fs, 32k
    {SAMPLE_RATE_44_1K, 256, 0x11eb85, 0x09},   // 256fs, 44.1k
    {SAMPLE_RATE_48K,   256, 0x555555, 0x08},   // 256fs, 48k
    {SAMPLE_RATE_96K,   256, 0x2aaaaa, 0x04},   // 256fs, 96k
};


// Data Format 
I2sDataFmt_t i2sDataFmt = 
{
    .slaveModeEn            = 0x1, // 0:Master; 1:Slave mode
    .slotSize               = 0xf, // Slot size
    .wordSize               = 0xf, // Real word size
    .alignMode              = 0,   // 1: Right alignment; 0: Left alignment
    .endianMode             = 0x1, // 1: MSB;  0: LSB
    .dataDly                = 0,   // Used by I2S format
    .txPad                  = 0,   // Tx padding
    .rxSignExt              = 0,   // Rx sign external
    .txPack                 = 1,   // 0: not compress; 1: 1word; 2: 2word
    .rxPack                 = 1,   // 0: not compress; 1: 1word; 2: 2word
    .txFifoEndianMode       = 0,   // I2s use or cspi use?
    .rxFifoEndianMode       = 0,   // I2s use or cspi use?
};

// Slot Control 
I2sSlotCtrl_t i2sSlotCtrl = 
{
    .slotEn                 = 0x1,  // Total 8 channels
    .slotNum                = 0x1   // For I2S, this value should be 1; For PCM, it can change
};

// BclkFs Control 
I2sBclkFsCtrl_t i2sBclkFsCtrl = 
{
    .bclkPolarity           = 1,    // 0: Rising edge send, falling edge sample; 1: falling edge send, rising edge sample 
    .fsPolarity             = 0,    // 0: rising edge start; 1: falling edge start
    .fsWidth                = 0xf   // fsWidth = slotNum *  slotSize
};

// I2S Control 
I2sCtrl_t i2sCtrl = 
{
	.i2sMode                = 0,    // 0: disable; 1: Only send; 2: Only receive; 3: Send and Receive
};

// I2S INT Control 
I2sIntCtrl_t i2sIntCtrl = 
{
    .txUnderRunIntEn        = 0,
    .txDmaErrIntEn          = 0,
    .txDatIntEn             = 0,
    .rxOverFlowIntEn        = 0,
    .rxDmaErrIntEn          = 0,
    .rxDatIntEn             = 0,
    .rxTimeOutIntEn         = 0,
    .fsErrIntEn             = 0,
    .frameStartIntEn        = 0,
    .frameEndIntEn          = 0,
    .cspiBusTimeOutIntEn    = 0,
    .txIntThreshHold        = 8,
    .rxIntThreshHold        = 8
};


// DMA Control
I2sDmaCtrl_t i2sDmaCtrl = 
{
    .rxDmaReqEn             = 1,    // rx dma enable
    .txDmaReqEn             = 1,    // tx dma enable
    .rxDmaTimeOutEn         = 0,    // rx dma timeout enable
    .dmaWorkWaitCycle       = 31,   // dma wait cycle number
    .rxDmaBurstSizeSub1     = 7,    // rx dma burst size -1
    .txDmaBurstSizeSub1     = 7,    // tx dma burst size -1
    .rxDmaThreadHold        = 7,    // rx dma threshold
    .txDmaThreadHold        = 7,    // tx dma threshold
    .rxFifoFlush            = 0,    // flush rx fifo
    .txFifoFlush            = 0     // flush tx fifo
};


//////////////////////////////////////////////////////////////////////////////////////////////
// I2S Setting field End
//////////////////////////////////////////////////////////////////////////////////////////////
#if (RTE_I2S0)
static I2sInfo_t  i2s0Info = {0};
void i2s0DmaRxEvent(uint32_t event);
void i2s0DmaTxEvent(uint32_t event);
DmaDescriptor_t __ALIGNED(16) i2s0DmaTxDesc[I2S_DMA_TX_DESCRIPTOR_CHAIN_NUM];
DmaDescriptor_t __ALIGNED(16) i2s0DmaRxDesc[I2S_DMA_RX_DESCRIPTOR_CHAIN_NUM];

static I2sDma_t i2s0Dma =
{
    DMA_INSTANCE_MP,
    -1,
    DMA_REQUEST_USP0_TX,
    i2s0DmaTxEvent,
    i2s0DmaTxDesc,

    DMA_INSTANCE_MP,
    -1,
    DMA_REQUEST_USP0_RX,
    i2s0DmaRxEvent,
    i2s0DmaRxDesc,
};

static I2sResources_t i2s0Res = {
    I2S0,
    {
        &i2s0PinMCLK,
        &i2s0PinBCLK,
        &i2s0PinLRCK,
        &i2s0PinDin,
        &i2s0PinDout,
    },
    &i2s0Dma,
    &i2s0Info
};
#endif

#if (RTE_I2S1)

static I2sInfo_t i2s1Info = {0};
void i2s1DmaRxEvent(uint32_t event);
void i2s1DmaTxEvent(uint32_t event);
DmaDescriptor_t __ALIGNED(16) i2s1DmaTxDesc[I2S_DMA_TX_DESCRIPTOR_CHAIN_NUM];
DmaDescriptor_t __ALIGNED(16) i2s1DmaRxDesc[I2S_DMA_RX_DESCRIPTOR_CHAIN_NUM];

static I2sDma_t i2s1Dma =
{
    DMA_INSTANCE_MP,
    -1,
    RTE_I2S1_DMA_TX_REQID,
    i2s1DmaTxEvent,
    i2s1DmaTxDesc,

    DMA_INSTANCE_MP,
    -1,
    RTE_I2S1_DMA_RX_REQID,
    i2s1DmaRxEvent,
    i2s1DmaRxDesc,
};

static I2sResources_t i2s1Res = {
    I2S1,
    {
        &i2s1PinMCLK,
        &i2s1PinBCLK,
        &i2s1PinLRCK,
        &i2s1PinDin,
        &i2s1PinDout,
    },
    &i2s1Dma,
    &i2s1Info
};
#endif


static I2S_TypeDef* const i2sInstance[I2S_INSTANCE_NUM] = {I2S0, I2S1};

static ClockId_e i2sClk[I2S_INSTANCE_NUM * 2] =
{
    PCLK_USP0,
    FCLK_USP0,
    PCLK_USP1,
    FCLK_USP1
};

#ifdef PM_FEATURE_ENABLE
static ClockId_e i2sMClk[I2S_INSTANCE_NUM] = 
{
    MCLK0,
    MCLK1,
};
#endif
static ClockResetId_e i2sRstClk[I2S_INSTANCE_NUM * 2] =
{
    RST_PCLK_USP0,
    RST_FCLK_USP0,
    RST_PCLK_USP1,
    RST_FCLK_USP1
};


#ifdef PM_FEATURE_ENABLE

/** \brief Internal used data structure */
typedef struct
{
    bool              isInited;                       /**< Whether spi has been initialized */
    bool              mclkHasBeenClosed;              /**< 1: mclk has been closed; 0: mclk hasn't been closed */
    struct
    {
        __IO uint32_t DFMT;                           /**< Data Format Register,                offset: 0x0 */
        __IO uint32_t SLOTCTL;                        /**< Slot Control Register,               offset: 0x4 */
        __IO uint32_t CLKCTL;                         /**< Clock Control Register,              offset: 0x8 */
        __IO uint32_t DMACTL;                         /**< DMA Control Register,                offset: 0xC */
        __IO uint32_t INTCTL;                         /**< Interrupt Control Register,          offset: 0x10 */
        __IO uint32_t TIMEOUTCTL;                     /**< Timeout Control Register,            offset: 0x14 */
        __IO uint32_t STAS;                           /**< Status Register,                     offset: 0x18 */
        __IO uint32_t RFIFO;                          /**< Rx Buffer Register,                  offset: 0x1c */
        __IO uint32_t TFIFO;                          /**< Tx Buffer Register,                  offset: 0x20 */
        __IO uint32_t I2SCTL;                         /**< I2S Control Register,                offset: 0x28 */
        __IO uint32_t I2SBUSSEL;                      /**< I2S Bussel Register,                 offset: 0xe0 */
    }regsBackup;
} I2sDataBase_t;

static I2sDataBase_t i2sDataBase[I2S_INSTANCE_NUM] = {0};
/**
  \brief i2s initialization counter, for lower power callback register/de-register
 */
static uint32_t i2sInitCnt = 0;

/**
  \brief Bitmap of I2S working status, each instance is assigned 2 bits representing tx and rx status,
         when all I2S instances are not working, we can vote to enter to low power state.
 */

volatile uint32_t i2sWorkingStats = 0;


/**
  \fn        static void i2sEnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, I2S hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void i2sEnterLpStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < I2S_INSTANCE_NUM; i++)
            {
                if(i2sDataBase[i].isInited == true)
                {
                    GPR_mclkDisable(i2sMClk[i]); // before sleep, disable MCLK
                    i2sDataBase[i].mclkHasBeenClosed = true;
                    
                    i2sDataBase[i].regsBackup.DFMT       = i2sInstance[i]->DFMT;
                    i2sDataBase[i].regsBackup.SLOTCTL    = i2sInstance[i]->SLOTCTL;
                    i2sDataBase[i].regsBackup.CLKCTL     = i2sInstance[i]->CLKCTL;
                    i2sDataBase[i].regsBackup.DMACTL     = i2sInstance[i]->DMACTL;
                    i2sDataBase[i].regsBackup.INTCTL     = i2sInstance[i]->INTCTL;
                    i2sDataBase[i].regsBackup.TIMEOUTCTL = i2sInstance[i]->TIMEOUTCTL;
                    i2sDataBase[i].regsBackup.STAS       = i2sInstance[i]->STAS;
                    i2sDataBase[i].regsBackup.I2SCTL     = i2sInstance[i]->I2SCTL;
                    i2sDataBase[i].regsBackup.I2SBUSSEL  = i2sInstance[i]->I2SBUSSEL;
                }
            }
            break;
        default:
            break;
    }
}

/**
  \fn        static void i2sExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, I2S hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void i2sExitLpStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < I2S_INSTANCE_NUM; i++)
            {
                if(i2sDataBase[i].isInited == true)
                {
                    GPR_clockEnable(i2sClk[i*2]);
                    GPR_clockEnable(i2sClk[i*2 + 1]);

                    i2sInstance[i]->DFMT        = i2sDataBase[i].regsBackup.DFMT;
                    i2sInstance[i]->SLOTCTL     = i2sDataBase[i].regsBackup.SLOTCTL;
                    i2sInstance[i]->CLKCTL      = i2sDataBase[i].regsBackup.CLKCTL;
                    i2sInstance[i]->DMACTL      = i2sDataBase[i].regsBackup.DMACTL;
                    i2sInstance[i]->INTCTL      = i2sDataBase[i].regsBackup.INTCTL;
                    i2sInstance[i]->TIMEOUTCTL  = i2sDataBase[i].regsBackup.TIMEOUTCTL;
                    i2sInstance[i]->STAS        = i2sDataBase[i].regsBackup.STAS;
                    i2sInstance[i]->I2SCTL      = i2sDataBase[i].regsBackup.I2SCTL;
                    i2sInstance[i]->I2SBUSSEL   = i2sDataBase[i].regsBackup.I2SBUSSEL;
                }
            }
#if (VOLTE_EC_OWN_BOARD_SUPPORT)
            extern i2sSlp1Cb_fn i2sSlp1CbFn;

            if (i2sSlp1CbFn)
            {
                i2sSlp1CbFn();
            }
#endif            
            
            break;

        default:
            break;
    }
}

#define  LOCK_SLEEP(instance)                                                                   \
                                    do                                                          \
                                    {                                                           \
                                        i2sWorkingStats |= (1U << instance);                    \
                                        slpManDrvVoteSleep(SLP_VOTE_I2S, SLP_ACTIVE_STATE);     \
                                    }                                                           \
                                    while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)                                                        \
                                    do                                                          \
                                    {                                                           \
                                        i2sWorkingStats &= ~(1U << instance);                   \
                                        if(i2sWorkingStats == 0)                                \
                                            slpManDrvVoteSleep(SLP_VOTE_I2S, SLP_SLP1_STATE);   \
                                    }                                                           \
                                    while(0)
#endif

static uint32_t i2sGetInstanceNum(I2sResources_t *i2s)
{
    return ((uint32_t)i2s->reg - (uint32_t)I2S0) >> 12U;
}

int32_t i2sInit(i2sCbEvent_fn txCbEvent, i2sCbEvent_fn rxCbEvent, I2sResources_t *i2s)
{
    int32_t returnCode;
    PadConfig_t config;

#ifdef PM_FEATURE_ENABLE
    uint32_t instance = i2sGetInstanceNum(i2s);
    i2sDataBase[instance].isInited = true;

    apmuVoteToDozeState(PMU_DOZE_USP_MOD, false);
#endif

    // Initialize I2S PINS
    PAD_getDefaultConfig(&config);
    config.mux = i2s->pins.mclk->funcNum;
    PAD_setPinConfig(i2s->pins.mclk->pinNum, &config);
    config.mux = i2s->pins.bclk->funcNum;
    PAD_setPinConfig(i2s->pins.bclk->pinNum, &config);
    config.mux = i2s->pins.lrck->funcNum;
    PAD_setPinConfig(i2s->pins.lrck->pinNum, &config);
    config.mux = i2s->pins.din->funcNum;
    PAD_setPinConfig(i2s->pins.din->pinNum, &config);
    config.mux = i2s->pins.dout->funcNum;
    PAD_setPinConfig(i2s->pins.dout->pinNum, &config);

    // Initialize I2S run-time resources
    i2s->info->txCbEvent     = txCbEvent;
    i2s->info->rxCbEvent     = rxCbEvent;
    i2s->info->isEnableFracdiv = true;

    // Configure DMA if necessary
    if (i2s->dma)
    {
        // Tx config
        DMA_init(i2s->dma->txInstance);
        returnCode = DMA_openChannel(i2s->dma->txInstance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            i2s->dma->txCh = returnCode;

        DMA_setChannelRequestSource(i2s->dma->txInstance, i2s->dma->txCh, (DmaRequestSource_e)i2s->dma->txReq);
        DMA_rigisterChannelCallback(i2s->dma->txInstance, i2s->dma->txCh, i2s->dma->txCb);

        // Rx config
        DMA_init(i2s->dma->rxInstance);
        returnCode = DMA_openChannel(i2s->dma->rxInstance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            i2s->dma->rxCh = returnCode;

        DMA_setChannelRequestSource(i2s->dma->rxInstance, i2s->dma->rxCh, (DmaRequestSource_e)i2s->dma->rxReq);
        DMA_rigisterChannelCallback(i2s->dma->rxInstance, i2s->dma->rxCh, i2s->dma->rxCb);
    }

#ifdef PM_FEATURE_ENABLE
    i2sInitCnt++;

    if(i2sInitCnt == 1U)
    {
        i2sWorkingStats = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE, i2sEnterLpStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE, i2sExitLpStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}


int32_t i2sDeInit(I2sResources_t *i2s)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance = i2sGetInstanceNum(i2s);
    i2sDataBase[instance].isInited = false;

    i2sInitCnt--;

    if(i2sInitCnt == 0)
    {
        i2sWorkingStats = 0;
        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE);
    }

    apmuVoteToDozeState(PMU_DOZE_USP_MOD, true);
    CHECK_TO_UNLOCK_SLEEP(instance);
#endif

    DMA_closeChannel(i2s->dma->rxInstance, i2s->dma->rxCh);
    DMA_closeChannel(i2s->dma->txInstance, i2s->dma->txCh);
    return ARM_DRIVER_OK;
}

int32_t i2sPowerCtrl(I2sPowerState_e state, I2sResources_t *i2s)
{
    uint32_t instance = i2sGetInstanceNum(i2s);
    int sampleRateIdx = 0;

    switch (state)
    {
        // when need to enter sleep, do not call this api. PMU will manage all the clocks by default.
        case I2S_POWER_OFF:
            if(i2s->dma)
            {
                DMA_stopChannel(i2s->dma->txInstance, i2s->dma->txCh, false);
            }

            // Reset register values
            if (instance == 0)
            {
                CLOCK_setClockSrc(FCLK_USP0, FCLK_USP0_SEL_26M);
                GPR_swReset(RST_PCLK_USP0);
            }
            else
            {
                CLOCK_setClockSrc(FCLK_USP1, FCLK_USP1_SEL_26M);
                GPR_swReset(RST_PCLK_USP1);
            }

            for (sampleRateIdx = 0; sampleRateIdx < sizeof(i2sSampleRateTbl) / sizeof(i2sSampleRateTbl[0]); sampleRateIdx++)
            {
                if (i2s->info->bps == i2sSampleRateTbl[sampleRateIdx][0])
                {
                    CLOCK_clockDisable(CLK_FRACDIV); // if effective bps, need to close fracdiv
                }
            }

            // Disable I2S fclk and pclk
            CLOCK_clockDisable(i2sClk[instance*2]);
            CLOCK_clockDisable(i2sClk[instance*2+1]); 
            break;

        case I2S_POWER_FULL:
            GPR_swReset(i2sRstClk[instance*2]);    // pclk
            GPR_swReset(i2sRstClk[instance*2+1]);  // fclk

            // Enable I2S clock            
            CLOCK_clockEnable(i2sClk[instance*2]);   // pclk
            CLOCK_clockEnable(i2sClk[instance*2+1]); // fclk

            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

int32_t i2sSend(bool needStop, bool needIrq, bool needChain, const void *data, uint32_t chunkNum, I2sResources_t *i2s)
{
    uint32_t instance = i2sGetInstanceNum(i2s);

    if ((data == NULL) || (chunkNum == 0))
        return ARM_DRIVER_ERROR_PARAMETER;

    // dma mode
    if(i2s->dma)
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif

        // Configure tx DMA and start it
        g_dmaTxConfig.sourceAddress = (void *)data;
        g_dmaTxConfig.targetAddress = (void *)&(i2sInstance[instance]->TFIFO);
        g_dmaTxConfig.totalLength   = chunkNum;
        i2s->info->trunkNum = chunkNum; // record chunkNum for upper layer use
        
        DMA_buildDescriptorChain(i2s->dma->txDescriptor, &g_dmaTxConfig, i2s->info->txDmaDescNum, needStop, needIrq, needChain);
        DMA_loadChannelDescriptorAndRun(i2s->dma->txInstance, i2s->dma->txCh, i2s->dma->txDescriptor);
    }

    return ARM_DRIVER_OK;
}

int32_t i2sRecv(bool needStop, bool needIrq, bool needChain, void *data, uint32_t chunkNum, I2sResources_t *i2s)
{
    uint32_t instance = i2sGetInstanceNum(i2s);;

    if ((data == NULL) || (chunkNum == 0))
        return ARM_DRIVER_ERROR_PARAMETER;

    // dma mode
    if(i2s->dma)
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif

        // Configure rx DMA and start it
        g_dmaRxConfig.sourceAddress = (void *)&(i2sInstance[instance]->RFIFO);
        g_dmaRxConfig.targetAddress = (void *)data;
        g_dmaRxConfig.totalLength   = chunkNum;
        i2s->info->trunkNum = chunkNum; // record chunkNum for upper layer use
        
        DMA_buildDescriptorChain(i2s->dma->rxDescriptor, &g_dmaRxConfig, i2s->info->rxDmaDescNum, needStop, needIrq, needChain);
        DMA_loadChannelDescriptorAndRun(i2s->dma->rxInstance, i2s->dma->rxCh, i2s->dma->rxDescriptor);
    }

    return ARM_DRIVER_OK;
}



/**
  \fn          static int32_t i2sSetBusSpeed(uint32_t bps, i2sResources_t *i2s)
  \brief       Set bus speed
  \param[in]   bps       bus speed to set
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
static int32_t i2sSetSampleRate(uint32_t bps, I2sResources_t *i2s, I2sRole_e i2sRole)
{
    uint32_t instance = i2sGetInstanceNum(i2s);
    
#ifdef PM_FEATURE_ENABLE
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, i2sSetSampleRate_1, P_INFO, "start to change samplerate");
#endif 

    i2s->info->bps = bps; // backup bps

    i2s->reg->I2SBUSSEL |= I2S_BUS_EN_Msk;
    if (instance == 0) // i2s0
    {
        CLOCK_fracDivOutCLkEnable(FRACDIV0_OUT0); // Fracdiv0 out0 enable
        CLOCK_setMclkSrc(MCLK0, MCLK_SRC_FRACDIV0_OUT0); // Choose Fracdiv0 out0 as MClk source
        //CLOCK_mclkEnable(MCLK0); // Mclk enable
        CLOCK_setFracDivOutClkDiv(FRACDIV0_OUT0, 6); // First step to generate MClk clock. 6 div

        // need to add gpr api for fracdiv
        int sampleRateIdx;
        for (sampleRateIdx = 0; sampleRateIdx < sizeof(i2sSampleRateTbl) / sizeof(i2sSampleRateTbl[0]); sampleRateIdx++)
        {
            if (bps == i2sSampleRateTbl[sampleRateIdx][0])
            {
                // Fracdiv clk selects 408M and set frac and integer clk
                FracDivConfig_t fracdivCfg;
                memset(&fracdivCfg, 0, sizeof(FracDivConfig_t));
                fracdivCfg.fracdivSel = FRACDIV_0;
                fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
                fracdivCfg.fracDiv0DivRatioInteger = i2sSampleRateTbl[sampleRateIdx][3];
                fracdivCfg.fracDiv0DivRatioFrac = i2sSampleRateTbl[sampleRateIdx][2];
                CLOCK_setFracDivConfig(&fracdivCfg); // Second step to generate MClk

#ifdef PM_FEATURE_ENABLE
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, i2sSetSampleRate_2, P_DEBUG, "i2s0: integer:%x, frac:%x", i2sSampleRateTbl[sampleRateIdx][3], i2sSampleRateTbl[sampleRateIdx][2]);
#endif 
            }
        }
    }
    else // i2s1
    {
        CLOCK_fracDivOutCLkEnable(FRACDIV1_OUT0); // Fracdiv1 out0 enable       
        CLOCK_setMclkSrc(MCLK1, MCLK_SRC_FRACDIV1_OUT0); // Choose Fracdiv1 out0 as MClk source
        //CLOCK_mclkEnable(MCLK1); // Mclk enable
        CLOCK_setFracDivOutClkDiv(FRACDIV1_OUT0, 6); // First step to generate MClk clock. 6 div

        // need to add gpr api for fracdiv
        int sampleRateIdx;
        for (sampleRateIdx = 0; sampleRateIdx < sizeof(i2sSampleRateTbl) / sizeof(i2sSampleRateTbl[0]); sampleRateIdx++)
        {
            if (bps == i2sSampleRateTbl[sampleRateIdx][0])
            {
                // Fracdiv clk selects 408M and set frac and integer clk
                FracDivConfig_t fracdivCfg;
                memset(&fracdivCfg, 0, sizeof(FracDivConfig_t));
                fracdivCfg.fracdivSel = FRACDIV_1;
                fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
                fracdivCfg.fracDiv1DivRatioInteger = i2sSampleRateTbl[sampleRateIdx][3];
                fracdivCfg.fracDiv1DivRatioFrac = i2sSampleRateTbl[sampleRateIdx][2];
                CLOCK_setFracDivConfig(&fracdivCfg); // Second step to generate MClk
                
#ifdef PM_FEATURE_ENABLE
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, i2sSetSampleRate_3, P_DEBUG, "i2s1: integer:%x, frac:%x", i2sSampleRateTbl[sampleRateIdx][3], i2sSampleRateTbl[sampleRateIdx][2]);
#endif                
            }
        }
    }

    if (i2s->info->isEnableFracdiv)
    {
        CLOCK_setClockSrc(CLK_FRACDIV, CLK_FRACDIV_SEL_612M);
        CLOCK_clockEnable(CLK_FRACDIV);
        i2s->info->isEnableFracdiv = false;
    }

    if (instance == 0)
    {
        // I2S master mode need to genetate LRCLK by MCU
        if (i2sRole == I2S_MASTER_MODE) // I2S controller act as master, codec is slave
        {       
            CLOCK_fracDivOutCLkEnable(FRACDIV0_OUT1); // Enable fracdiv0 out1
            CLOCK_setBclkSrc(BCLK0, BCLK_SRC_FRACDIV0_OUT1); // Use fracdiv0 out1 to generate bclk
            CLOCK_setFracDivOutClkDiv(FRACDIV0_OUT1, 12); // First step to generate BClk clock. 8 div
            CLOCK_setBclkDiv(BCLK0, 4); // Second step 4 div
            //CLOCK_bclkEnable(BCLK0); // Enable bclk
        }
    }
    else
    {
        // I2S master mode need to genetate LRCLK by MCU
        if (i2sRole == I2S_MASTER_MODE) // I2S controller act as master, codec is slave
        {       
            CLOCK_fracDivOutCLkEnable(FRACDIV1_OUT1); // Enable fracdiv1 out1
            CLOCK_setBclkSrc(BCLK1, BCLK_SRC_FRACDIV1_OUT1); // Use fracdiv1 out1 to generate bclk
            CLOCK_setFracDivOutClkDiv(FRACDIV1_OUT1, 12); // First step to generate BClk clock. 8 div
            CLOCK_setBclkDiv(BCLK1, 4); // Second step 4 div
            CLOCK_bclkEnable(BCLK1); // Enable bclk
        }
    }

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t i2sControl(uint32_t control, uint32_t arg, i2sRes_t *i2s)
  \brief       Control I2S Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   i2s      Pointer to I2S resources
  \return      common \ref execution_status and driver specific \ref spi_execution_status
*/
int32_t i2sControl(uint32_t control, uint32_t arg, I2sResources_t *i2s)
{
    uint32_t instance = i2sGetInstanceNum(i2s);

    switch(control & 0xFFFF)
    {
        // Set transport abort
        case I2S_CTRL_TRANSABORT:
        {
            // If DMA mode, disable DMA channel
            if(i2s->dma)
            {
                DMA_stopChannel(i2s->dma->txInstance, i2s->dma->txCh, true);
            }

            break;
        }

        case I2S_CTRL_SET_TOTAL_NUM:
        {
            i2s->info->totalNum = arg;
            break;
        }        

        // Set Bus Speed in bps; arg = value
        case I2S_CTRL_SAMPLE_RATE_SLAVE:
        {
            if(i2sSetSampleRate(arg, i2s, I2S_SLAVE_MODE) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            break;
        }

        case I2S_CTRL_SAMPLE_RATE_MASTER:
        {
            if(i2sSetSampleRate(arg, i2s, I2S_MASTER_MODE) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            break;
        }     

        // Set Data Format
        case I2S_CTRL_DATA_FORMAT:
        {
            memcpy((void*)&(i2sInstance[instance]->DFMT), &i2sDataFmt, sizeof(I2sDataFmt_t));
            break;
        }

        // Set Slot
        case I2S_CTRL_SLOT_CTRL:
        {
            memcpy((void*)&(i2sInstance[instance]->SLOTCTL), &i2sSlotCtrl, sizeof(I2sSlotCtrl_t));
            break;
        }

        // Set DMA Control
        case I2S_CTRL_DMA_CTRL:
        {
            memcpy((void*)&(i2sInstance[instance]->DMACTL), &i2sDmaCtrl, sizeof(I2sDmaCtrl_t));
            break;
        }

        // Set I2S INT Control
        case I2S_CTRL_INT_CTRL:
        {
            memcpy((void*)&(i2sInstance[instance]->INTCTL), &i2sIntCtrl, sizeof(I2sIntCtrl_t));
            break;
        }

        // Set Frame Info0
        case I2S_CTRL_BCLK_FS_CTRL:
        {
            memcpy((void*)&(i2sInstance[instance]->CLKCTL), &i2sBclkFsCtrl, sizeof(I2sBclkFsCtrl_t));
            break;
        }

        // Set RX dma descriptor num
        case I2S_CTRL_RX_DESCRIPTOR:
        {
            i2s->info->rxDmaDescNum = arg;            
            break;
        }

        // Set TX dma descriptor num
        case I2S_CTRL_TX_DESCRIPTOR:
        {   
            i2s->info->txDmaDescNum = arg;
            break;
        }

        // Start or stop audio play
        case I2S_CTRL_START_STOP:
        {
#ifdef PM_FEATURE_ENABLE
            if (i2sDataBase[instance].mclkHasBeenClosed == true)
            {
                GPR_mclkEnable(i2sMClk[instance]);
                i2sDataBase[instance].mclkHasBeenClosed = false; // already opened MCLK now
            }
#endif
            i2sInstance[instance]->I2SCTL = arg; // 0: disable i2s; 1: enable send; 2: enable recv; 3: enable send/recv
            break;
        }

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

void i2sDmaRxEvent(uint32_t event, I2sResources_t *i2s)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance = i2sGetInstanceNum(i2s);
#endif

    switch(event)
    {
        case DMA_EVENT_END:
            if(i2s->info->rxCbEvent)
            {
                i2s->info->rxCbEvent(ARM_I2S_EVENT_TRANSFER_COMPLETE, 0);
            }
#ifdef PM_FEATURE_ENABLE
             CHECK_TO_UNLOCK_SLEEP(instance);
#endif
            break;

        default:
            break;
    }
}

void i2sDmaTxEvent(uint32_t event, I2sResources_t *i2s)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance = i2sGetInstanceNum(i2s);
#endif

    switch(event)
    {
        case DMA_EVENT_END:
            if(i2s->info->txCbEvent)
            {
                i2s->info->txCbEvent(ARM_I2S_EVENT_TRANSFER_COMPLETE, i2s->info->totalNum);
            }
#ifdef PM_FEATURE_ENABLE
             CHECK_TO_UNLOCK_SLEEP(instance);
#endif
            break;

        default:
            break;
    }
}

uint32_t i2sGetTotalCnt(I2sResources_t *i2s)
{
	return i2s->info->totalNum;
}

uint32_t i2sGetTrunkCnt(I2sResources_t *i2s)
{
	return i2s->info->trunkNum;
}

uint32_t i2sGetCtrlReg(I2sResources_t *i2s)
{
	return i2s->reg->I2SCTL;
}


#if (RTE_I2S0)

static int32_t i2s0Init(i2sCbEvent_fn txCbEvent, i2sCbEvent_fn rxCbEvent)
{
    return i2sInit(txCbEvent, rxCbEvent, &i2s0Res);
}
static int32_t i2s0Deinit(void)
{
    return i2sDeInit(&i2s0Res);
}
static int32_t i2s0PowerCtrl(I2sPowerState_e state)
{
    return i2sPowerCtrl(state, &i2s0Res);
}

static int32_t i2s0Send(bool needStop, bool needIrq, bool needChain, void *data, uint32_t num)
{
    return i2sSend(needStop, needIrq, needChain, data, num, &i2s0Res);
}

static int32_t i2s0Recv(bool needStop, bool needIrq, bool needChain, void *data, uint32_t num)
{
    return i2sRecv(needStop, needIrq, needChain, data, num, &i2s0Res);
}

static int32_t i2s0Ctrl(uint32_t control, uint32_t arg)
{
    return i2sControl(control, arg, &i2s0Res);
}

void i2s0DmaTxEvent(uint32_t event)
{
    i2sDmaTxEvent(event, &i2s0Res);
}

void i2s0DmaRxEvent(uint32_t event)
{
    i2sDmaRxEvent(event, &i2s0Res);
}

uint32_t i2s0GetTotalCnt(void)
{
	return i2sGetTotalCnt(&i2s0Res);
}

uint32_t i2s0GetTrunkCnt(void)
{
	return i2sGetTrunkCnt(&i2s0Res);
}

uint32_t i2s0GetCtrlReg()
{
    return i2sGetCtrlReg(&i2s0Res);
}

// I2S0 Driver Control Block
I2sDrvInterface_t i2sDrvInterface0 = 
{
    i2s0Init,
    i2s0Deinit,
    i2s0PowerCtrl,
    i2s0Send,
    i2s0Recv,
    i2s0Ctrl,
    i2s0GetTotalCnt,
    i2s0GetTrunkCnt,
    i2s0GetCtrlReg,
};
#endif

#if (RTE_I2S1)
static int32_t i2s1Init(i2sCbEvent_fn txCbEvent, i2sCbEvent_fn rxCbEvent)
{
    return i2sInit(txCbEvent, rxCbEvent, &i2s1Res);
}
static int32_t i2s1Deinit(void)
{
    return i2sDeInit(&i2s1Res);
}
static int32_t i2s1PowerCtrl(I2sPowerState_e state)
{
    return i2sPowerCtrl(state, &i2s1Res);
}

static int32_t i2s1Send(bool needStop, bool needIrq, bool needChain, void *data, uint32_t num)
{
    return i2sSend(needStop, needIrq, needChain, data, num, &i2s1Res);
}

static int32_t i2s1Recv(bool needStop, bool needIrq, bool needChain, void *data, uint32_t num)
{
    return i2sRecv(needStop, needIrq, needChain, data, num, &i2s1Res);
}

static int32_t i2s1Ctrl(uint32_t control, uint32_t arg)
{
    return i2sControl(control, arg, &i2s1Res);
}

uint32_t i2s1GetTotalCnt(void)
{
	return i2sGetTotalCnt(&i2s1Res);
}

uint32_t i2s1GetTrunkCnt(void)
{
	return i2sGetTrunkCnt(&i2s1Res);
}


void i2s1DmaRxEvent(uint32_t event)
{
    i2sDmaRxEvent(event, &i2s1Res);
}

void i2s1DmaTxEvent(uint32_t event)
{
    i2sDmaTxEvent(event, &i2s1Res);
}

uint32_t i2s1GetCtrlReg()
{
    return i2sGetCtrlReg(&i2s1Res);
}

// I2S1 Driver Control Block
I2sDrvInterface_t i2sDrvInterface1 = 
{
    i2s1Init,
    i2s1Deinit,
    i2s1PowerCtrl,
    i2s1Send,
    i2s1Recv,
    i2s1Ctrl,
	i2s1GetTotalCnt,
    i2s1GetTrunkCnt,
    i2s1GetCtrlReg,
};
#endif

