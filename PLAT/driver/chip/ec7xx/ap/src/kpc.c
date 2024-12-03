/****************************************************************************
 *
 * Copy right:   2020-, Copyrigths of AirM2M Ltd.
 * File name:    kpc_ec718.c
 * Description:  EC718 kpc driver source file
 * History:      Rev1.0   2022-07-23
 *
 ****************************************************************************/

#include "kpc.h"
#include "ic.h"
#include "clock.h"
#include "slpman.h"
#include "sctdef.h"

#if defined(KPC_IP_VERSION_B1)

#define KPC_ROW_NUM         (8U)
#define KPC_ROW_NUM_MASK    (0x7U)
#define KPC_ROW_BITMAP_MASK (0xFFU)

#define KPC_COL_NUM         (8U)
#define KPC_COL_NUM_MASK    (0x7U)
#define KPC_COL_NUM_BITS    (3U)
#define KPC_COL_BITMAP_MASK (0xFFU)

typedef struct
{
    uint32_t map[2];
} KpcBitMap_t;

static void KPC_bitMapAND(KpcBitMap_t* input0, KpcBitMap_t* input1, KpcBitMap_t* output)
{
    output->map[0] = input0->map[0] & input1->map[0];
    output->map[1] = input0->map[1] & input1->map[1];
}

static void KPC_bitMapXOR(KpcBitMap_t* input0, KpcBitMap_t* input1, KpcBitMap_t* output)
{
    output->map[0] = input0->map[0] ^ input1->map[0];
    output->map[1] = input0->map[1] ^ input1->map[1];
}

static void KPC_bitMapClear(KpcBitMap_t* output)
{
    output->map[0] = 0;
    output->map[1] = 0;
}


static __FORCEINLINE uint32_t KPC_findFirstSet(uint32_t value)
{
    return __CLZ(__RBIT(value));
}

static uint32_t KPC_bitMapfindFirstSet(KpcBitMap_t* input)
{
    return (input->map[0] == 0) ? 32 + KPC_findFirstSet(input->map[1]) : KPC_findFirstSet(input->map[0]);
}

static uint32_t KPC_bitMapIsZero(KpcBitMap_t* input)
{
    return (input->map[0] == 0) && (input->map[1] == 0);
}

static uint32_t KPC_bitMapBitValue(KpcBitMap_t* input, uint32_t pos)
{
    return (pos >= 32) ? (input->map[1] & (1 << (pos - 32))) : (input->map[0] & (1 << pos));
}


#else
#define KPC_ROW_BITMAP_MASK (0x1FU)
#define KPC_COL_BITMAP_MASK (0x1FU)
#endif

//#define KPC_DEBUG

#define KPC_CTRL_MASK  (KPC_KPENCTL_ENABLE_Msk | KPC_KPENCTL_PULL_EN_Msk)

/** \brief Internal used data structure */
typedef struct
{
    uint8_t                            isInited;                      /**< flag indicating intialized or not */

    uint8_t                            enableAutoRepeat;              /**< autorepeat feature is enabled */
    uint8_t                            autoRepeatDelay;               /**< autorepeat event delay */
    uint8_t                            autoRepeatPeriod;              /**< autorepeat event period */
    uint32_t                           autoRepeatCount;               /**< counter for autorepeat */

#if defined(KPC_IP_VERSION_B1)
    KpcBitMap_t                        keyEnableMask;                 /**< Bitmap of enabled keys */
    KpcBitMap_t                        keyState;                      /**< Bitmap of key state, 0-release, 1-press */
#else
    uint32_t                           keyEnableMask;                 /**< Bitmap of enabled keys */
    uint32_t                           keyState;                      /**< Bitmap of key state, 0-release, 1-press */
#endif
    struct
    {
        uint32_t DEBCTL;                           /**< Debounce Control Register */
        uint32_t KPCTL;                            /**< Keypad Control Register */
        uint32_t DICTL;                            /**< Direct Input Control Register */
        uint32_t KPENCTL;                          /**< Keypad Enable Register */
        uint32_t DIENCTL;                          /**< Direct Input Enable Register */
    } configRegisters;

    kpc_callback_t                  eventCallback;                 /**< Callback function passed in by application */
} KpcDataBase_t;

#if defined(KPC_IP_VERSION_B1)
AP_PLAT_COMMON_BSS static KpcDataBase_t __ALIGNED(4) gKpcDataBase = {0};
#else
AP_PLAT_COMMON_BSS static KpcDataBase_t gKpcDataBase = {0};
#endif


#ifdef PM_FEATURE_ENABLE
/**
  \fn        void KPC_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Backup KPC configurations before sleep.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
void KPC_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            if(gKpcDataBase.isInited == 1)
            {
                gKpcDataBase.configRegisters.DEBCTL = KPC->DEBCTL;
                gKpcDataBase.configRegisters.KPCTL = KPC->KPCTL;
                gKpcDataBase.configRegisters.DICTL = KPC->DICTL;
                gKpcDataBase.configRegisters.KPENCTL = KPC->KPENCTL;
                gKpcDataBase.configRegisters.DIENCTL = KPC->DIENCTL;
            }

            break;

        default:
            break;
    }

}

/**
 \fn        void KPC_exitLowPowerStateRestore(void* pdata, slpManLpState state)
 \brief     Restore KPC configurations after exit from sleep.
 \param[in] pdata pointer to user data, not used now
 \param[in] state low power state
 */
void KPC_exitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    switch(state)
    {
        case SLPMAN_SLEEP1_STATE:

            if(gKpcDataBase.isInited == 1)
            {
                KPC->DEBCTL = gKpcDataBase.configRegisters.DEBCTL;
                KPC->KPCTL = gKpcDataBase.configRegisters.KPCTL;
                KPC->DICTL = gKpcDataBase.configRegisters.DICTL;
                KPC->KPENCTL = gKpcDataBase.configRegisters.KPENCTL;
                KPC->DIENCTL = gKpcDataBase.configRegisters.DIENCTL;
            }

            break;

        default:
            break;
    }

}
#endif

#if defined(KPC_IP_VERSION_B1)
/**
  construct key enable bitmap

    39              32 31              24 23              16 15               8 7                 0
   +------------------+------------------+------------------+------------------+------------------+
   |  row 4, col[7:0] |  row 3, col[7:0] |  row 2, col[7:0] |  row 1,col[7:0]  |  row 0, col[7:0] |
   +------------------+------------------+------------------+------------------+------------------+

 */
static void KPC_setKeyMask(uint32_t rowMask, uint32_t colMask, KpcBitMap_t* keyEnableMask, uint32_t* rowCounts)
{
    KPC_bitMapClear(keyEnableMask);
    uint32_t rowIndex = 0;
    uint32_t cnt = 0;

    uint32_t rowMaskLowNibble = (rowMask & KPC_ROW_BITMAP_MASK) & 0xFU;
    uint32_t rowMaskHighNibble = ((rowMask & KPC_ROW_BITMAP_MASK) >> 4) & 0xFU;

    colMask &= KPC_COL_BITMAP_MASK;

    while(rowMaskLowNibble)
    {
        rowIndex = KPC_findFirstSet(rowMaskLowNibble);


        keyEnableMask->map[0] |= (colMask << (rowIndex * KPC_COL_NUM));

        rowMaskLowNibble &= (rowMaskLowNibble - 1);
        cnt++;
    }

    while(rowMaskHighNibble)
    {
        rowIndex = KPC_findFirstSet(rowMaskHighNibble);

        keyEnableMask->map[1] |= (colMask << ((rowIndex) * KPC_COL_NUM));

        rowMaskHighNibble &= (rowMaskHighNibble - 1);
        cnt++;
    }

    *rowCounts = cnt;
}

static void KPC_eventReport(void)
{
    uint32_t lsb, report = 0;

    KpcBitMap_t keyScanResult, keyScanResultBackup, changed;

    KpcReportEvent_t event;

    // Get current key scan result
    KPC_bitMapAND((KpcBitMap_t *)&KPC->KPSTAT[0], &gKpcDataBase.keyEnableMask, &keyScanResult);

    //keyScanResult = KPC->KPSTAT & gKpcDataBase.keyEnableMask;

    keyScanResultBackup =  keyScanResult;

    // Find out changed key
    KPC_bitMapXOR(&keyScanResult, &gKpcDataBase.keyState, &changed);

    //changed = keyScanResult ^ gKpcDataBase.keyState;

#ifdef KPC_DEBUG
    printf("report: [%x-%x]-[%x-%x]-[%x-%x]-[%x]\r\n", keyScanResult.map[1], keyScanResult.map[0], gKpcDataBase.keyState.map[1], gKpcDataBase.keyState.map[0],
                                                       changed.map[1], changed.map[0], KPC->KPENCTL);
#endif

    // First to handle key release
    if(!KPC_bitMapIsZero(&changed))
    {
        lsb = KPC_bitMapfindFirstSet(&changed);

        // Check key released during two consecutive rounds of scan
        //if((keyScanResultBackup & ( 1 << lsb)) == 0)
        if(KPC_bitMapBitValue(&keyScanResultBackup, lsb) == 0)
        {
            event.row = lsb >> KPC_COL_NUM_BITS; // => lsb / 8
            event.column = lsb & KPC_COL_NUM_MASK; // => lsb % 8
            event.value = KPC_REPORT_KEY_RELEASE;

            gKpcDataBase.autoRepeatCount = 0;

            if(gKpcDataBase.eventCallback)
            {
                gKpcDataBase.eventCallback(event);
            }

        }
    }

    // Check key press
    if(!KPC_bitMapIsZero(&keyScanResult))
    {
        lsb = KPC_bitMapfindFirstSet(&keyScanResult);

        // repeat key?
        //if((gKpcDataBase.keyState & (1 << lsb)) && (gKpcDataBase.enableAutoRepeat == 1))
        if(KPC_bitMapBitValue(&gKpcDataBase.keyState, lsb) && (gKpcDataBase.enableAutoRepeat == 1))
        {
            gKpcDataBase.autoRepeatCount++;

            if((gKpcDataBase.autoRepeatCount >= gKpcDataBase.autoRepeatDelay) && ((gKpcDataBase.autoRepeatCount - gKpcDataBase.autoRepeatDelay) % gKpcDataBase.autoRepeatPeriod == 0))
            {
                event.row = lsb >> KPC_COL_NUM_BITS;
                event.column = lsb & KPC_COL_NUM_MASK;
                event.value = KPC_REPORT_KEY_REPEAT;
                report = 1;
            }
        }
        else
        {
            event.row = lsb >> KPC_COL_NUM_BITS;
            event.column = lsb & KPC_COL_NUM_MASK;
            event.value = KPC_REPORT_KEY_PRESS;
            gKpcDataBase.autoRepeatCount = 0;
            report = 1;
        }

        if(gKpcDataBase.eventCallback && report)
        {
            gKpcDataBase.eventCallback(event);
        }
    }

    // save current result
    gKpcDataBase.keyState = keyScanResultBackup;
}

#else

static __FORCEINLINE uint32_t KPC_findFirstSet(uint32_t value)
{
    return __CLZ(__RBIT(value));
}

/**
  construct key enable bitmap

    24              20 19              15 14              10 9                5 4                 0
   +------------------+------------------+------------------+------------------+------------------+
   |  row 4, col[4:0] |  row 3, col[4:0] |  row 2, col[4:0] |  row 1,col[4:0]  |  row 0, col[4:0] |
   +------------------+------------------+------------------+------------------+------------------+

 */
static void KPC_setKeyMask(uint32_t rowMask, uint32_t colMask, uint32_t* keyEnableMask, uint32_t* rowCounts)
{
    uint32_t keyMask = 0;
    uint32_t rowIndex = 0;
    uint32_t cnt = 0;

    // Max 5x5 matrix keypad
    rowMask &= 0x1f;
    colMask &= 0x1f;

    while(rowMask)
    {
        rowIndex = KPC_findFirstSet(rowMask);
        keyMask |= (colMask << (rowIndex * 5));
        rowMask &= (rowMask - 1);
        cnt++;
    }

    *keyEnableMask = keyMask;
    *rowCounts = cnt;
}

static void KPC_eventReport(void)
{
    uint32_t lsb, keyScanResult, keyScanResultBackup, changed, report = 0;

    KpcReportEvent_t event;

    // Get current key scan result
    keyScanResult = KPC->KPSTAT & gKpcDataBase.keyEnableMask;

    keyScanResultBackup =  keyScanResult;

    // Find out changed key
    changed = keyScanResult ^ gKpcDataBase.keyState;

#ifdef KPC_DEBUG
    printf("report: [%x]-[%x]-[%x]-[%x]\r\n", keyScanResult, gKpcDataBase.keyState, changed, KPC->KPENCTL);
#endif

    // First to handle key release
    if(changed)
    {
        lsb = KPC_findFirstSet(changed);

        // Check key released during two consecutive rounds of scan
        if((keyScanResultBackup & ( 1 << lsb)) == 0)
        {
            event.row = lsb / 5;
            event.column = lsb % 5;
            event.value = KPC_REPORT_KEY_RELEASE;

            gKpcDataBase.autoRepeatCount = 0;

            if(gKpcDataBase.eventCallback)
            {
                gKpcDataBase.eventCallback(event);
            }

        }
    }

    // Check key press
    if(keyScanResult)
    {
        lsb = KPC_findFirstSet(keyScanResult);

        // repeat key?
        if((gKpcDataBase.keyState & (1 << lsb)) && (gKpcDataBase.enableAutoRepeat == 1))
        {
            gKpcDataBase.autoRepeatCount++;

            if((gKpcDataBase.autoRepeatCount >= gKpcDataBase.autoRepeatDelay) && ((gKpcDataBase.autoRepeatCount - gKpcDataBase.autoRepeatDelay) % gKpcDataBase.autoRepeatPeriod == 0))
            {
                event.row = lsb / 5;
                event.column = lsb % 5;
                event.value = KPC_REPORT_KEY_REPEAT;
                report = 1;
            }
        }
        else
        {
            event.row = lsb / 5;
            event.column = lsb % 5;
            event.value = KPC_REPORT_KEY_PRESS;
            gKpcDataBase.autoRepeatCount = 0;
            report = 1;
        }

        if(gKpcDataBase.eventCallback && report)
        {
            gKpcDataBase.eventCallback(event);
        }
    }

    // save current result
    gKpcDataBase.keyState = keyScanResultBackup;
}

#endif

static void KPC_keyPadModeIsr(void)
{
    KPC_eventReport();

    // Check if all keys are released
#if defined(KPC_IP_VERSION_B1)
    if((KPC->KPSTAT[0] == 0) && (KPC->KPSTAT[1] == 0))
#else
    if(KPC->KPSTAT == 0)
#endif
    {
        // If all keys are released, change back to hw controlled scan mode(trigger irq when any key is pressed)
        KPC->KPENCTL = KPC_CTRL_MASK;
    }
    else
    {
        // If any key is pressed, change to sw controlled scan mode(trigger irq at the end of every scan round)
        KPC->KPENCTL = KPC_CTRL_MASK | KPC_KPENCTL_WORK_MODE_Msk;
    }

}

void KPC_getDefaultConfig(KpcConfig_t *config)
{
    ASSERT(config);

    config->debounceConfig.debounceClkDivRatio = KPC_DEBOUNCE_CLK_DIV_RATIO_16384;
    config->debounceConfig.debounceWidth       = KPC_DEBOUNCE_WIDTH_7_CYCLES;

    config->validRowMask                       = KPC_ROW_ALL;
    config->validColumnMask                    = KPC_COLUMN_ALL;

    config->scanPolarity                       = KPC_SCAN_POLARITY_0;
    config->scanDivRatio                       = KPC_SCAN_DIV_RATIO_16;

    config->autoRepeat.enable                  = 1;
    config->autoRepeat.delay                   = 10;
    config->autoRepeat.period                  = 1;
}

int32_t KPC_init(const KpcConfig_t *config, kpc_callback_t callback)
{
    ASSERT(config);

    uint32_t mask, validRowCounts;

    mask = SaveAndSetIRQMask();

    // Initialization
    if(gKpcDataBase.isInited == 0)
    {
        CLOCK_setClockSrc(FCLK_KPC, FCLK_KPC_SEL_26M);

        CLOCK_clockEnable(FCLK_KPC);
        CLOCK_clockEnable(PCLK_KPC);

        GPR_swResetModule(RESET_VECTOR_PTR(KPC_RESET_VECTOR));

        // enable KPC IRQ
        XIC_SetVector(PXIC0_KPC_KEYPAD_IRQn, KPC_keyPadModeIsr);
        XIC_EnableIRQ(PXIC0_KPC_KEYPAD_IRQn);

    }
    else
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_OK;
    }

    RestoreIRQMask(mask);

    KPC_setKeyMask(config->validRowMask, config->validColumnMask, &gKpcDataBase.keyEnableMask, &validRowCounts);

    if(validRowCounts < 2)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    KPC->DEBCTL = EIGEN_VAL2FLD(KPC_DEBCTL_DEBOUNCER_DEPTH, config->debounceConfig.debounceWidth) | \
                  EIGEN_VAL2FLD(KPC_DEBCTL_DEBOUNCER_TO0_THRD, config->debounceConfig.debounceWidth) | \
                  EIGEN_VAL2FLD(KPC_DEBCTL_DEBOUNCER_TO1_THRD, config->debounceConfig.debounceWidth) | \
                  EIGEN_VAL2FLD(KPC_DEBCTL_DEBOUNCER_TO_MCLK_RATIO, config->debounceConfig.debounceClkDivRatio);

    KPC->KPCTL = EIGEN_VAL2FLD(KPC_KPCTL_POLARITY, config->scanPolarity) | \
                 EIGEN_VAL2FLD(KPC_KPCTL_ROW_VLD_BITMAP, config->validRowMask & KPC_ROW_BITMAP_MASK) | \
                 EIGEN_VAL2FLD(KPC_KPCTL_COL_VLD_BITMAP, config->validColumnMask & KPC_COL_BITMAP_MASK) | \
                 EIGEN_VAL2FLD(KPC_KPCTL_SCAN_TO_DEBOUNCE_RATIO, config->scanDivRatio);

    KPC->AUTOCG = KPC_AUTOCG_ENABLE_Msk;

    gKpcDataBase.eventCallback = callback;

    gKpcDataBase.enableAutoRepeat  = config->autoRepeat.enable;
    gKpcDataBase.autoRepeatDelay  = config->autoRepeat.delay;
    gKpcDataBase.autoRepeatPeriod = config->autoRepeat.period;

#ifdef PM_FEATURE_ENABLE
    slpManRegisterPredefinedBackupCb(SLP_CALLBACK_KPC_MODULE, KPC_enterLowPowerStatePrepare, NULL);
    slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_KPC_MODULE, KPC_exitLowPowerStateRestore, NULL);
#endif

    gKpcDataBase.isInited = 1;

    return ARM_DRIVER_OK;
}

void KPC_deInit(void)
{
    uint32_t mask = SaveAndSetIRQMask();

    if(gKpcDataBase.isInited == 1)
    {
        KPC_stopScan();

        // disable clock
        CLOCK_clockDisable(FCLK_KPC);
        CLOCK_clockDisable(PCLK_KPC);

        XIC_DisableIRQ(PXIC0_KPC_KEYPAD_IRQn);
        XIC_ClearPendingIRQ(PXIC0_KPC_KEYPAD_IRQn);

#ifdef PM_FEATURE_ENABLE
        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_KPC_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_KPC_MODULE);
#endif

        gKpcDataBase.isInited = 0;
    }

    RestoreIRQMask(mask);
}

void KPC_startScan(void)
{
    uint32_t mask = SaveAndSetIRQMask();

    if(gKpcDataBase.isInited == 1)
    {
        KPC->KPENCTL = KPC_CTRL_MASK;
    }

    RestoreIRQMask(mask);
}

void KPC_stopScan(void)
{
    uint32_t mask = SaveAndSetIRQMask();

    if(gKpcDataBase.isInited == 1)
    {
        KPC->KPENCTL = 0;
    }

    RestoreIRQMask(mask);

}


