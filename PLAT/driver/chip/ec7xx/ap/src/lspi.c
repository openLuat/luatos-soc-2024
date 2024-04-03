/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    lspi.c
 * Description:  Dedicated spi for LCD use in EC718. The interface is similar with CMSIS Driver API V2.0.
 * History:      Rev1.0   2021-11-25
 *
 ****************************************************************************/
#include "lspi.h"
#include "slpman.h"


#if (RTE_LSPI1)
static PIN lspi1Ds   = {RTE_USP1_MCLK_PAD_ADDR,   RTE_USP1_MCLK_FUNC};
static PIN lspi1Clk  = {RTE_USP1_BCLK_PAD_ADDR,   RTE_USP1_BCLK_FUNC};
static PIN lspi1Cs   = {RTE_USP1_LRCK_PAD_ADDR,   RTE_USP1_LRCK_FUNC};
static PIN lspi1Miso = {RTE_USP1_DIN_PAD_ADDR,    RTE_USP1_DIN_FUNC};
static PIN lspi1Mosi = {RTE_USP1_DOUT_PAD_ADDR,   RTE_USP1_DOUT_FUNC};
#endif

#if (RTE_LSPI2)
static PIN lspi2Ds   = {RTE_USP2_MCLK_PAD_ADDR,   RTE_USP2_MCLK_FUNC};
static PIN lspi2Clk  = {RTE_USP2_BCLK_PAD_ADDR,   RTE_USP2_BCLK_FUNC};
static PIN lspi2Cs   = {RTE_USP2_LRCK_PAD_ADDR,   RTE_USP2_LRCK_FUNC};
static PIN lspi2Miso = {RTE_USP2_DIN_PAD_ADDR,    RTE_USP2_DIN_FUNC};
static PIN lspi2Mosi = {RTE_USP2_DOUT_PAD_ADDR,   RTE_USP2_DOUT_FUNC};
#endif


// Data Format 
lspiDataFmt_t lspiDataFmt = 
{
    .slaveModeEn            = 0,
    .slotSize               = 0,
    .wordSize               = 7,
    .alignMode              = 0,
    .endianMode             = 0, // 0:LSB  1: MSB
    .dataDly                = 0,
    .txPad                  = 0,
    .rxSignExt              = 0,
    .txPack                 = 0,
    .rxPack                 = 0,
    .txFifoEndianMode       = 0,
    .rxFifoEndianMode       = 0,
    .eorMode                = 0,
};

// DMA Control
lspiDmaCtrl_t lspiDmaCtrl =
{
    .rxDmaReqEn             = 0,
    .txDmaReqEn             = 0,
    .rxDmaTimeOutEn         = 0,
    .dmaWorkWaitCycle       = 31,
    .rxDmaBurstSizeSub1     = 7,
    .txDmaBurstSizeSub1     = 7,
    .rxDmaThreadHold        = 7,
    .txDmaThreadHold        = 7,
    .rxFifoFlush            = 0,
    .txFifoFlush            = 0
};

// INT Control
lspiIntCtrl_t lspiIntCtrl =
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
    .lspiRamWrBreakIntEn    = 0,
    .lspiRamWrFrameStartEn  = 0,
    .lspiRamWrFrameEndEn    = 0,
    .lspiCmdEndEn           = 0,
    .cspiOtsuEndEn          = 0,
    .lspiRamWrEndEn         = 0,
    .txIntThreshHold        = 8,
    .rxIntThreshHold        = 8,
};


lspiCtrl_t lspiCtrl =
{
    .enable                 = 1,           ///< lspi Enable
    .data2Lane              = 0,           ///< 2 data lane enable
    .line4                  = 0,           ///< 0= not use port as DCX, 1= use port as DCX
    .datSrc                 = 0,           ///< data from camera or memory
    .colorModeIn            = 0,           ///< Input data color mode
    .colorModeOut           = 0,           ///< Output data color mode
    .yAdjEn                 = 0,           ///< Y adjust enable
    .yAdjSel                = 0,           ///< Y adjustment from cspi or sw
    .yAdjBound              = 0,           ///< Y adjustment bound
    .dcDummy                = 0,           ///< Send DCX or dummy
    .busType                = 0,

};

lspiCmdCtrl_t lspiCmdCtrl =
{
    .wrRdn                  = 0,           ///< 0=rd, 1=wr
    .ramWr                  = 0,           ///< start to fill frame memory
    .rdatDummyCycle         = 0,           ///< Dummy cycle before data read
    .dataLen                = 0,           ///< data len for wr/rd
    .init                   = 0,           ///< always be 0
};

lspiCmdAddr_t lspiCmdAddr =
{
    .addr                   = 0,
};

lspiInfo_t lspiInfo =
{
    .frameHeight            = 0,           ///< frame height
    .frameWidth             = 0,           ///< frame width
};

lspitailorInfo0_t lspiTailorInfo0 =
{
    .tailorBottom           = 0,           ///< cut bottom lines
    .tailorTop              = 0,           ///< cut top lines
};

lspitailorInfo_t lspiTailorInfo =
{
    .tailorLeft             = 0,           ///< cut upper lines
    .tailorRight            = 0,
};

lspiScaleInfo_t lspiScaleInfo =
{
    .rowScaleFrac           = 0,           ///< cut upper lines
    .colScaleFrac           = 0,
};

lspiQuartileCtrl_t lspiQuartileCtrl =
{
    .grayCtrl               = 0,            ///< gray ctrl
    .quartileSel            = 0,            ///< quartile from cspi or sw
    .quartile1              = 0,            ///< quartile 1
    .quartile2              = 0,            ///< quartile 2
    .quartile3              = 0,            ///< quartile 3
};

lspiYAdj_t lspiYAdj =
{
    .yadjYmin               = 0,            ///< y adj min
    .yadjYmax               = 0,            ///< y adj max
    .yadjStrech             = 0,            ///< y adj stretch
    .yadjStrechFwl          = 0,            ///< y adj stretch fwl
};

lspiGrayPageCmd0_t lspiGrayPageCmd0 =
{
    .pageCmd                = 0,            ///< page cmd
    .pageCmd0               = 0,            ///< page cmd 0
    .pageCmd01ByteNum       = 0,            ///< page cmd0 + page cmd1 byte num
};

lspiGrayPageCmd1_t lspiGrayPageCmd1 =
{
    .pageCmd1               = 0,            ///< page cmd1
};

lspiFrameInfoOut_t lspiFrameInfoOut =
{
    .frameHeightOut         = 0,            ///< frame height out
    .frameWidthOut          = 0,            ///< frame width out
};




lspiBusSel_t lspiBusSel =
{
    .i2sBusEn               = 0,            ///< I2S bus enable
    .cspiBusEn              = 0,            ///< Cspi bus enable
    .lspiBusEn              = 1,            ///< Lspi bus enable
};


//////////////////////////////////////////////////////////////////////////////////////////////
// LSPI Setting field End
//////////////////////////////////////////////////////////////////////////////////////////////

static LSPI_TypeDef* const lspiInstance[LSPI_INSTANCE_NUM] = {LSPI1, LSPI2};

static ClockId_e lspiClk[LSPI_INSTANCE_NUM * 2] =
{
    PCLK_USP1,
    FCLK_USP1,
    PCLK_USP2,
    FCLK_USP2
};

#if 0
static ClockResetId_e lspiRstClk[LSPI_INSTANCE_NUM * 2] =
{
    //RST_PCLK_I2S0,
    //RST_FCLK_I2S0,
    //RST_PCLK_I2S1,
    //RST_FCLK_I2S1
};
#endif

#ifdef PM_FEATURE_ENABLE
/**
  \brief spi initialization counter, for lower power callback register/de-register
 */
static uint32_t lspiInitCnt = 0;

/**
  \brief Bitmap of LSPI working status, each instance is assigned 2 bits representing tx and rx status,
         when all LSPI instances are not working, we can vote to enter to low power state.
 */
static uint32_t lspiWorkingStats = 0;

/** \brief Internal used data structure */
typedef struct
{
    bool              isInited;                       /**< Whether spi has been initialized */
#if 0

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
        __IO uint32_t LSPICTL;                        /**< Camera SPI Control Register,         offset: 0x28 */
        __IO uint32_t CCTL;                           /**< Auto Cg Control Register,            offset: 0x2c */
        __IO uint32_t LSPIINFO0;                      /**< Cspi Frame info0 Register,           offset: 0x30 */
        __IO uint32_t LSPIINFO1;                      /**< Cspi Frame info1 Register,           offset: 0x34 */
        __IO uint32_t LSPIDBG;                        /**< Cspi Debug Register,                 offset: 0x38 */
        __IO uint32_t LSPINIT;                        /**< Cspi Init Register,                  offset: 0x3c */
        __IO uint32_t CLSP;                           /**< Cspi Line Start Register,            offset: 0x40 */
        __IO uint32_t CDATP;                          /**< Cspi Data Packet Register,           offset: 0x44 */
        __IO uint32_t CLINFO;                         /**< Cspi Line Info Register,             offset: 0x48 */
    }regsBackup;
#endif
} lspiDataBase_t;

static lspiDataBase_t lspiDataBase[LSPI_INSTANCE_NUM] = {0};

/**
  \fn        static void lspiEnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LSPI hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void lspiEnterLpStatePrepare(void* pdata, slpManLpState state)
{
#if 0
    uint32_t i;
    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < LSPI_INSTANCE_NUM; i++)
            {
                if(lspiDataBase[i].isInited == true)
                {
                    lspiDataBase[i].regsBackup.DFMT         = lspiInstance[i]->DFMT;
                    lspiDataBase[i].regsBackup.SLOTCTL      = lspiInstance[i]->SLOTCTL;
                    lspiDataBase[i].regsBackup.CLKCTL       = lspiInstance[i]->CLKCTL;
                    lspiDataBase[i].regsBackup.DMACTL       = lspiInstance[i]->DMACTL;
                    lspiDataBase[i].regsBackup.INTCTL       = lspiInstance[i]->INTCTL;
                    lspiDataBase[i].regsBackup.TIMEOUTCTL   = lspiInstance[i]->TIMEOUTCTL;
                    lspiDataBase[i].regsBackup.STAS         = lspiInstance[i]->STAS;
                    lspiDataBase[i].regsBackup.LSPICTL      = lspiInstance[i]->LSPICTL;
                    lspiDataBase[i].regsBackup.CCTL         = lspiInstance[i]->CCTL;
                    lspiDataBase[i].regsBackup.LSPIINFO0    = lspiInstance[i]->LSPIINFO0;
                    lspiDataBase[i].regsBackup.LSPIINFO1    = lspiInstance[i]->LSPIINFO1;
                    lspiDataBase[i].regsBackup.LSPIDBG      = lspiInstance[i]->LSPIDBG;
                    lspiDataBase[i].regsBackup.LSPINIT      = lspiInstance[i]->LSPINIT;
                    lspiDataBase[i].regsBackup.CLSP         = lspiInstance[i]->CLSP;
                    lspiDataBase[i].regsBackup.CDATP        = lspiInstance[i]->CDATP;
                    lspiDataBase[i].regsBackup.CLINFO       = lspiInstance[i]->CLINFO;
                }
            }
            break;
        default:
            break;
    }
#endif
}

/**
  \fn        static void lspiExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LSPI hareware is repowered, we restore user's configurations
             by aidding of the stored registers.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void lspiExitLpStateRestore(void* pdata, slpManLpState state)
{
#if 0
    uint32_t i;
    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < LSPI_INSTANCE_NUM; i++)
            {
                if(lspiDataBase[i].isInited == true)
                {
                    GPR_clockEnable(lspiClk[2*i]);
                    GPR_clockEnable(lspiClk[2*i+1]);

                    lspiInstance[i]->DFMT       = lspiDataBase[i].regsBackup.DFMT;
                    lspiInstance[i]->SLOTCTL    = lspiDataBase[i].regsBackup.SLOTCTL;
                    lspiInstance[i]->CLKCTL     = lspiDataBase[i].regsBackup.CLKCTL;
                    lspiInstance[i]->DMACTL     = lspiDataBase[i].regsBackup.DMACTL;
                    lspiInstance[i]->INTCTL     = lspiDataBase[i].regsBackup.INTCTL;
                    lspiInstance[i]->TIMEOUTCTL = lspiDataBase[i].regsBackup.TIMEOUTCTL;
                    lspiInstance[i]->STAS       = lspiDataBase[i].regsBackup.STAS;
                    lspiInstance[i]->LSPICTL    = lspiDataBase[i].regsBackup.LSPICTL;
                    lspiInstance[i]->CCTL       = lspiDataBase[i].regsBackup.CCTL;
                    lspiInstance[i]->LSPIINFO0  = lspiDataBase[i].regsBackup.LSPIINFO0;
                    lspiInstance[i]->LSPIINFO1  = lspiDataBase[i].regsBackup.LSPIINFO1;
                    lspiInstance[i]->LSPIDBG    = lspiDataBase[i].regsBackup.LSPIDBG;
                    lspiInstance[i]->LSPINIT    = lspiDataBase[i].regsBackup.LSPINIT;
                    lspiInstance[i]->CLSP       = lspiDataBase[i].regsBackup.CLSP;
                    lspiInstance[i]->CDATP      = lspiDataBase[i].regsBackup.CDATP;
                    lspiInstance[i]->CLINFO     = lspiDataBase[i].regsBackup.CLINFO;
                }
            }
            break;

        default:
            break;
    }
#endif
}

#define  LOCK_SLEEP(instance)                                                                   \
                                    do                                                          \
                                    {                                                           \
                                    }                                                           \
                                    while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)                                                        \
                                    do                                                          \
                                    {                                                           \
                                    }                                                           \
                                    while(0)
#endif

#if (RTE_LSPI1)
static lspiRteInfo_t  lspi1Info = {0};
//lspiPrepareSendInfo_t prePareSendInfo = {0};

static lspiRes_t lspi1Res = {
    LSPI1,
    {
        &lspi1Ds,
        &lspi1Clk,
        &lspi1Cs,
        &lspi1Miso,
        &lspi1Mosi,
    },
    NULL,
    &lspi1Info
};

#endif

#if (RTE_LSPI2)
static lspiRteInfo_t lspi2Info = {0};
static lspiRes_t lspi2Res = {
    LSPI2,
    {
        &lspi2Ds,
        &lspi2Clk,
        &lspi2Cs,
        &lspi2Miso,
        &lspi2Mosi,
    },
    NULL,//&lspi2Dma,
    &lspi2Info
};
#endif

/**
  \fn          static uint32_t lspiGetInstanceNum(lspiRes_t *lspi)
  \brief       Get instance number
  \param[in]   spi       Pointer to LSPI resources. 0: LSPI1;  1: LSPI2
  \returns     instance number
*/
static uint32_t lspiGetInstanceNum(lspiRes_t *lspi)
{
    return ((uint32_t)lspi->reg - (uint32_t)(LSPI_TypeDef *)MP_USP1_BASE_ADDR) >> 12U;
}

/**
  \fn          static int32_t lspiSetBusSpeed(uint32_t bps, lspiRes_t *lspi)
  \brief       Set bus speed
  \param[in]   bps       bus speed to set
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
static int32_t lspiSetBusSpeed(uint32_t bps, lspiRes_t *lspi)
{
    uint32_t instance = lspiGetInstanceNum(lspi);
    uint8_t div = 1;
    if(bps<102*1024*1024) div = (uint8_t)((102*1024*1024)/(bps+1));
    if (instance == 0)
    {
        GPR_clockEnable(FCLK_USP0);
    }
    else // usp2
    {
        // printf("bps %d div %d\r\n",bps,div);
        // CLOCK_setClockDiv(FCLK_USP2, 1); // usp2 fclk div. 1 div means 26M itself
        #if 1
        CLOCK_setClockSrc(CLK_APB_MP,CLK_APB_MP_SEL_102M);
        CLOCK_clockEnable(CLK_HF102M);
        CLOCK_setClockSrc(FCLK_USP2,FCLK_USP2_SEL_102M);
        CLOCK_clockEnable(FCLK_USP2);
        *(uint32_t*)0x4f00007c = div << 16;
        #endif
    }

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t lspiInit(lspiCbEvent_fn cbEvent, lspiRes_t *lspi)
  \brief       Initialize SPI Interface.
  \param[in]   cbEvent  Pointer to \ref lspiCbEvent_fn
  \param[in]   spi       Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiInit(lspiCbEvent_fn cbEvent, lspiRes_t *lspi)
{
    int32_t returnCode;
    PadConfig_t config;

#ifdef PM_FEATURE_ENABLE
    uint32_t instance = lspiGetInstanceNum(lspi);
    lspiDataBase[instance].isInited = true;
    
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, false);
#endif

    // Initialize LSPI PINS
    PAD_getDefaultConfig(&config);
    config.mux = lspi->pins.pinDs->funcNum;
    PAD_setPinConfig(lspi->pins.pinDs->pinNum, &config);
    config.mux = lspi->pins.pinClk->funcNum;
    PAD_setPinConfig(lspi->pins.pinClk->pinNum, &config);
    config.mux = lspi->pins.pinCs->funcNum;
    PAD_setPinConfig(lspi->pins.pinCs->pinNum, &config);
    config.mux = lspi->pins.pinMiso->funcNum;
    PAD_setPinConfig(lspi->pins.pinMiso->pinNum, &config);
    config.mux = lspi->pins.pinMosi->funcNum;
    PAD_setPinConfig(lspi->pins.pinMosi->pinNum, &config);

    // Initialize SPI run-time resources
    lspi->info->cbEvent     = cbEvent;

    // Configure DMA if necessary
    if (lspi->dma)
    {
        DMA_init(lspi->dma->txInstance);
        returnCode = DMA_openChannel(lspi->dma->txInstance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            lspi->dma->txCh = returnCode;

        DMA_setChannelRequestSource(lspi->dma->txInstance, lspi->dma->txCh, (DmaRequestSource_e)lspi->dma->txReq);
        DMA_rigisterChannelCallback(lspi->dma->txInstance, lspi->dma->txCh, lspi->dma->txCb);
    }


#ifdef PM_FEATURE_ENABLE
    lspiInitCnt++;

    if(lspiInitCnt == 1U)
    {
        lspiWorkingStats = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE, lspiEnterLpStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE, lspiExitLpStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t lspiDeInit(lspiRes_t *lspi)
  \brief       De-initialize LSPI Interface.
  \param[in]   spi  Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiDeInit(lspiRes_t *lspi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;

    instance = lspiGetInstanceNum(lspi);

    lspiDataBase[instance].isInited = false;

    lspiInitCnt--;

    if(lspiInitCnt == 0)
    {
        lspiWorkingStats = 0;
        //slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE);
        //slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE);
    }
    
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, true);
#endif

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t lspiPowerControl(lspiPowerState_e state, lspiRes_t *lspi)
  \brief       Control LSPI Interface Power.
  \param[in]   state  Power state
  \param[in]   lspi    Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiPowerCtrl(lspiPowerState_e state, lspiRes_t *lspi)
{
    uint32_t instance = lspiGetInstanceNum(lspi);

    switch (state)
    {
        case LSPI_POWER_OFF:
            // DMA disable
            if(lspi->dma)
            {
                DMA_stopChannel(lspi->dma->txInstance, lspi->dma->txCh, true);
            }

            // Reset register values
            if (instance == 1)
            {
                //CLOCK_setClockSrc(FCLK_USP0, FCLK_I2S0_SEL_26M);
                //GPR_swReset(RST_PCLK_I2S0);
            }

            if (instance == 2)
            {
                //CLOCK_setClockSrc(FCLK_USP1, FCLK_I2S1_SEL_26M);
                //GPR_swReset(RST_PCLK_I2S1);
            }


            // Disable LSPI clock
            GPR_clockDisable(lspiClk[instance*2]);
            GPR_clockDisable(lspiClk[instance*2+1]);

            break;

        case LSPI_POWER_FULL:

            // Enable LSPI clock
            GPR_clockEnable(lspiClk[instance*2]);
            GPR_clockEnable(lspiClk[instance*2+1]);

            //GPR_swReset(lspiRstClk[instance*2]);
            //GPR_swReset(lspiRstClk[instance*2+1]);

            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t lspiReceive(void *data, uint32_t num, lspiRes_t *lspi)
  \brief       Start receiving data from SPI receiver.
  \param[out]  dataIn  Pointer to buffer for data to receive from LSPI receiver
  \param[in]   num   Number of data items to receive
  \param[in]   lspi   Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiSend(void *dataOut, uint32_t num, lspiRes_t *lspi)
{
    //uint32_t instance = lspiGetInstanceNum(lspi);

    uint8_t     *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    //uint8_t     *trans          = &(lspi->info->prePareSendInfo.trans);
    uint8_t     *dataLen        = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp            = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList       = lspi->info->prePareSendInfo.dataList;

    // Collect the remainder bytes which is less then 4byte
    if (*dataLen % 4 != 0)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
    }
    
    for (int i = 0; i < *dataListIndex; i++)
    {
        lspi->reg->TFIFO = dataList[i];
    }

    lspi->reg->LSPI_CCTRL = 1 | *dataLen<<8;

    // wait until finish
    while (!lspi->reg->LSPI_STAT);

    memset(dataList, 0, *dataListIndex * 4);
    lspi->info->prePareSendInfo.dataListIndex = 0;
    lspi->info->prePareSendInfo.tmp = 0;
    lspi->info->prePareSendInfo.trans = 0;
    lspi->info->prePareSendInfo.dataLen = 0;
    

    return ARM_DRIVER_OK;
}

void lspiPrepareSend(uint8_t data, lspiRes_t *lspi)
{
    //uint32_t instance = lspiGetInstanceNum(lspi);

    uint8_t     *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t     *trans          = &(lspi->info->prePareSendInfo.trans);
    uint8_t     *dataLen        = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp            = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList       = lspi->info->prePareSendInfo.dataList;

    *tmp |= data << *trans;
    *dataLen += 1;
    *trans += 8;

    // Round up to 4bytes, then store it into the array of dataList
    if (*trans == 32)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex += 1;
        *trans = 0;
        *tmp = 0;
    }
}

/**
  \fn          int32_t lspiControl(uint32_t control, uint32_t arg, lspiRes_t *lspi)
  \brief       Control LSPI Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   lspi      Pointer to LSPI resources
  \return      common \ref execution_status and driver specific \ref spi_execution_status
*/
int32_t lspiControl(uint32_t control, uint32_t arg, lspiRes_t *lspi)
{
    uint32_t instance = lspiGetInstanceNum(lspi);
    uint8_t *tmp = NULL;

    switch(control & 0xFFFFF)
    {

        // Set Data Format
        case LSPI_CTRL_DATA_FORMAT:
        {
            tmp = (uint8_t*)&lspiDataFmt;
            lspiInstance[instance]->DFMT = *(uint32_t*)tmp;
            break;
        }

        // Set Bus Speed in bps; arg = value
        case LSPI_CTRL_BUS_SPEED:
        {
            if(lspiSetBusSpeed(arg, lspi) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            break;
        }

        // Set DMA control
        case LSPI_CTRL_DMA_CTRL:
        {
            tmp = (uint8_t*)&lspiDmaCtrl;
            lspiInstance[instance]->DMACTL = *(uint32_t*)tmp;
            break;
        }

        // Set INT control
        case LSPI_CTRL_INT_CTRL:
        {
            tmp = (uint8_t*)&lspiIntCtrl;
            lspiInstance[instance]->INTCTL = *(uint32_t*)tmp;
            break;
        }

        // Lspi control
        case LSPI_CTRL_CTRL:
        {
            tmp = (uint8_t*)&lspiCtrl;
            lspiInstance[instance]->LSPI_CTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi command control
        case LSPI_CTRL_CMD_CTRL:
        {
            tmp = (uint8_t*)&lspiCmdCtrl;
            (lspiInstance[instance])->LSPI_CCTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi YUV
        case LSPI_CTRL_YUV2RGB_INFO0:
        {
            LSPI2->YUV2RGBINFO0 = 0x199 << 18 | 0x12a << 8 | 0x10 << 0;
            break;
        }

        // Lspi YUV
        case LSPI_CTRL_YUV2RGB_INFO1:
        {
            
            LSPI2->YUV2RGBINFO1 = 0x204 << 20 | 0x64 << 10 | 0xd0 << 0;
            
            break;
        }

        // Lspi command address
        case LSPI_CTRL_CMD_ADDR:
        {
            tmp = (uint8_t*)&lspiCmdAddr;
            lspiInstance[instance]->LSPI_CADDR = *(uint32_t*)tmp;
            break;
        }

        // Lspi frame info
        case LSPI_CTRL_FRAME_INFO:
        {
            tmp = (uint8_t*)&lspiInfo;
            lspiInstance[instance]->LSPFINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi tailor info0
        case LSPI_CTRL_TAILOR_INFO0:
        {
            tmp = (uint8_t*)&lspiTailorInfo0;
            lspiInstance[instance]->LSPTINFO0 = *(uint32_t*)tmp;
            break;
        }

        // Lspi tailor info
        case LSPI_CTRL_TAILOR_INFO:
        {
            tmp = (uint8_t*)&lspiTailorInfo;
            lspiInstance[instance]->LSPTINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi scale info
        case LSPI_CTRL_SCALE_INFO:
        {
            tmp = (uint8_t*)&lspiScaleInfo;
            lspiInstance[instance]->LSPSINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi quartile control
        case LSPI_CTRL_QUARTILE_CTRL:
        {
            tmp = (uint8_t*)&lspiQuartileCtrl;
            lspiInstance[instance]->LSPIQUARTCTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi Y adjustment
        case LSPI_CTRL_YADJ:
        {
            tmp = (uint8_t*)&lspiYAdj;
            lspiInstance[instance]->LSPIYADJ = *(uint32_t*)tmp;
            break;
        }

        // Lspi gray page cmd0
        case LSPI_CTRL_GRAY_PAGE_CMD0:
        {
            tmp = (uint8_t*)&lspiGrayPageCmd0;
            lspiInstance[instance]->LSPIGPCMD0 = *(uint32_t*)tmp;
            break;
        }

        // Lspi gray page cmd1
        case LSPI_CTRL_GRAY_PAGE_CMD1:
        {
            tmp = (uint8_t*)&lspiGrayPageCmd1;
            lspiInstance[instance]->LSPIGPCMD1 = *(uint32_t*)tmp;
            break;
        }

        // Lspi frame info out
        case LSPI_CTRL_FRAME_INFO_OUT:
        {
            tmp = (uint8_t*)&lspiFrameInfoOut;
            lspiInstance[instance]->LSPFINFO0 = *(uint32_t*)tmp;
            break;
        }

        // USP bus select
        case LSPI_CTRL_BUS_SEL:
        {
            tmp = (uint8_t*)&lspiBusSel;
            lspiInstance[instance]->I2SBUSSEL = *(uint32_t*)tmp;
            break;
        }

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}


#if (RTE_LSPI1)

static int32_t lspi1Init(lspiCbEvent_fn cbEvent)
{
    return lspiInit(cbEvent, &lspi1Res);
}
static int32_t lspi1Deinit(void)
{
    return lspiDeInit(&lspi1Res);
}
static int32_t lspi1PowerCtrl(lspiPowerState_e state)
{
    return lspiPowerCtrl(state, &lspi1Res);
}

static void lspi1PrepareSend(uint8_t data)
{
    return lspiPrepareSend(data, &lspi1Res);
}

static int32_t lspi1Send(void *data, uint32_t num)
{
    return lspiSend(data, num, &lspi1Res);
}

static int32_t lspi1Ctrl(uint32_t control, uint32_t arg)
{
    return lspiControl(control, arg, &lspi1Res);
}

// LSPI0 Driver Control Block
lspiDrvInterface_t lspiDrvInterface1 = {
    lspi1Init,
    lspi1Deinit,
    lspi1PowerCtrl,
    lspi2PrepareSend,
    lspi1Send,
    lspi1Ctrl,
};

#endif

#if (RTE_LSPI2)
static int32_t lspi2Init(lspiCbEvent_fn cbEvent)
{
    return lspiInit(cbEvent, &lspi2Res);
}
static int32_t lspi2Deinit(void)
{
    return lspiDeInit(&lspi2Res);
}
static int32_t lspi2PowerCtrl(lspiPowerState_e state)
{
    return lspiPowerCtrl(state, &lspi2Res);
}

static int32_t lspi2Send(void *data, uint32_t num)
{
    return lspiSend(data, num, &lspi2Res);
}

static void lspi2PrepareSend(uint8_t data)
{
    return lspiPrepareSend(data, &lspi2Res);
}

static int32_t lspi2Ctrl(uint32_t control, uint32_t arg)
{
    return lspiControl(control, arg, &lspi2Res);
}

// LSPI1 Driver Control Block
lspiDrvInterface_t lspiDrvInterface2 = {
    lspi2Init,
    lspi2Deinit,
    lspi2PowerCtrl,
    lspi2PrepareSend,
    lspi2Send,
    lspi2Ctrl,
};

#endif


