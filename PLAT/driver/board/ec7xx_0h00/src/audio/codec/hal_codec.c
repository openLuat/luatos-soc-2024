/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    hal_codec.c
 * Description:  EC7XX codec hal 
 * History:      Rev1.0   2021-9-18
 *
 ****************************************************************************/

#include "hal_codec.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/
const HalCodecTlvDefault_t codecTlvDefaultVal = 
{
    .isDmic         = false,                
    .isExPa         = true,              
    .exPaGain       = 0, 
    .txDigGain      = 0xbf,     
    .txAnaGain      = 0x8, 

    .rxDigGain0     = 0,
    .rxAnaGain0     = 0,
    .rxDigGain50    = 0,
    .rxAnaGain50    = 0,
    .rxDigGain100   = 0xff,
    .rxAnaGain100   = 0xff,
};

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/

#ifdef FEATURE_OS_ENABLE

HalCodecSts_e halCodecLock(void* arg)
{
    osSemaphoreId_t codecLock = (osSemaphoreId_t)arg;

    if (osSemaphoreAcquire(codecLock, 1000) != osOK)
    {
        return CODEC_TIMEOUT;
    }
    return  CODEC_EOK;
}

HalCodecSts_e halCodecUnlock(void* arg)
{
    osSemaphoreId_t codecLock = (osSemaphoreId_t)arg;

    if (osSemaphoreRelease(codecLock) != osOK)
    {
        return CODEC_TIMEOUT;
    }
    return  CODEC_EOK;
}

HalCodecSts_e halCodecLockDestroy(void *arg)
{
    osSemaphoreId_t codecLock = (osSemaphoreId_t)arg;

    if (osSemaphoreDelete(codecLock) != osOK)
    {
        return CODEC_TIMEOUT;
    }
    return  CODEC_EOK;
}

#else

HalCodecSts_e halCodecLock(void* arg)
{
    return  CODEC_EOK;
}

HalCodecSts_e halCodecUnlock(void* arg)
{
    return  CODEC_EOK;
}

HalCodecSts_e halCodecLockDestroy(void *arg)
{
    return  CODEC_EOK;
}

#endif


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
 
HalCodecCfg_t halCodecGetDefaultCfg(HalCodecFuncList_t* codecHalFunc)
{
    return codecHalFunc->halCodecGetDefaultCfg();
}

HalCodecFuncList_t* halCodecInit(HalCodecCfg_t* codecHalCfg, HalCodecFuncList_t* codecHalFunc, bool needLock)
{
    HalCodecSts_e ret = CODEC_EOK;
    HalCodecFuncList_t* codecHal = codecHalFunc;
    
#ifdef FEATURE_OS_ENABLE
    if (codecHal->halCodecLock == NULL)
    {
        codecHal->halCodecLock = osSemaphoreNew(1, 1, NULL);
        if (codecHal->halCodecLock == PNULL)
        {
            DEBUG_PRINT(UNILOG_PLA_DRIVER, halCodecInit_0, P_WARNING, "codec hal can't create semaphore");
            return NULL;
        }
    }
#endif

    if (needLock) halCodecLock(codecHal->halCodecLock);

    ret |= codecHal->halCodecInitFunc(codecHalCfg);
    if (ret != CODEC_EOK)
    {        
        halCodecUnlock(codecHal->halCodecLock);
        DEBUG_PRINT(UNILOG_PLA_DRIVER, halCodecInit_2, P_DEBUG, "codec init fail. ret=%d", ret);
        EC_ASSERT(false, 0, 0, 0);
        return NULL;
    }

    codecHal->halCodecCfgIfaceFunc(codecHalCfg->codecMode, &codecHalCfg->codecIface);
    
    codecHal->handle = codecHal;
    codecHalFunc->handle = codecHal;
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return codecHal;
}

HalCodecSts_e halCodecDeinit(HalCodecFuncList_t* codecHal)
{
    int32_t ret = CODEC_EOK;
    
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    halCodecLockDestroy(codecHal->halCodecLock);
    codecHal->halCodecDeinitFunc();
    codecHal->halCodecLock = NULL;
    codecHal->handle = NULL;
    codecHal = NULL;
    return ret;
}

HalCodecSts_e halCodecCtrlState(HalCodecFuncList_t* codecHal, HalCodecMode_e mode, HalCodecCtrlState_e codecStartStop, bool needLock)
{
    int32_t ret;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
    DEBUG_PRINT(UNILOG_PLA_DRIVER, halCodecCtrl_1, P_DEBUG, "codec mode=%d, startStop=%d", mode, codecStartStop);
    ret = codecHal->halCodecCtrlStateFunc(mode, codecStartStop);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}

HalCodecSts_e halCodecIfaceCfg(HalCodecFuncList_t* codecHal, HalCodecMode_e mode, HalCodecIface_t *iface, bool needLock)
{
    int32_t ret = CODEC_EOK;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    HAL_CODEC_CHECK_NULL(iface, "Get volume para is null", -1); 
    if (needLock) halCodecLock(codecHal->halCodecLock);
    ret = codecHal->halCodecCfgIfaceFunc(mode, iface);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);

    return ret;
}

HalCodecSts_e halCodecSetMute(HalCodecFuncList_t* codecHal, HalCodecCfg_t* codecHalCfg, bool mute, bool needLock)
{
    int32_t ret;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
    ret = codecHal->halCodecSetMuteFunc(codecHalCfg, mute);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}

#if 0
HalCodecSts_e halCodecEnablePA(HalCodecFuncList_t* codecHal, bool enable, bool needLock)
{
    int32_t ret = CODEC_EOK;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
        codecHal->halCodecEnablePAFunc(enable);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}
#endif

HalCodecSts_e halCodecSetVolume(HalCodecFuncList_t* codecHal, HalCodecCfg_t* codecHalCfg, int volume, bool needLock)
{
    int32_t ret;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
    ret = codecHal->halCodecSetVolumeFunc(codecHalCfg, volume);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}

HalCodecSts_e halCodecGetVolume(HalCodecFuncList_t* codecHal, HalCodecCfg_t* codecHalCfg, int *volume, bool needLock)
{
    int32_t ret;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    HAL_CODEC_CHECK_NULL(volume, "Get volume para is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
    ret = codecHal->halCodecGetVolumeFunc(codecHalCfg, volume);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}

HalCodecSts_e halCodecSetMicVolume(HalCodecFuncList_t* codecHal, HalCodecCfg_t* codecHalCfg, uint8_t micGain, int volume, bool needLock)
{
    int32_t ret;
    HAL_CODEC_CHECK_NULL(codecHal, "audio_hal handle is null", -1);
    if (needLock) halCodecLock(codecHal->halCodecLock);
    ret = codecHal->halCodecSetMicVolumeFunc(codecHalCfg, micGain, volume);
    if (needLock) halCodecUnlock(codecHal->halCodecLock);
    return ret;
}


