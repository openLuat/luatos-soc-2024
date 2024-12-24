#include <string.h>
#include "hal_powBackoff.h"
#include DEBUG_LOG_HEADER_FILE
#include "FreeRTOS.h"
#include "cmsis_os2.h"
#include "queue.h"
#include "ps_lib_api.h"
#include "hal_adcproxy.h"
#include "slpman.h"
/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define POWBACKOFF_TASK_STATK_SIZE    2048
#define POWBACKOFF_EVENT_QUEUE_SIZE   2

#define POWBACKOFF_THRESHOLD_LEVEL0       55        // level to auto cfun=1
#define POWBACKOFF_THRESHOLD_LEVEL1       75        // level to limit the max power
#define POWBACKOFF_THRESHOLD_LEVEL2       100       // level to auto cfun=0

#define POWBACKOFF_MINUS_VALUE            8         // backoff value
/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/
typedef enum
{
    POWBACKOFF_INTTEMP = 0
        
}powBackOffMsgType_e;

typedef enum
{
    POWBACKOFF_DISABLED = 0,
    POWBACKOFF_ENABLED = 1,
    POWBACKOFF_CFUN_0 = 2,
}powBackOffStatus_e;

typedef struct {
    powBackOffMsgType_e type;
    uint8_t rsvd;
    int16_t adcIntTemp;            // internal temperature, fwl = 4
}powBackOffMsg_t;


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/
static QueueHandle_t powBackOffQueueHandle;
static bool powBackOffTaskExist = false;
static powBackOffStatus_e gPowBackOffStatus = POWBACKOFF_DISABLED;
static uint8_t powBackOffVoteHandle = 0xFF;
osTimerId_t powBackOffReconnetTimer = NULL;
osThreadId_t powBackOffTaskId = NULL;


static void powBackOffDisableSleep1(void)
{
    slpManSlpState_t pstate;
    uint8_t counter;
    slpManCheckVoteState(powBackOffVoteHandle, &pstate, &counter);
    if(counter == 0)
    {
        slpManPlatVoteDisableSleep(powBackOffVoteHandle, SLP_SLP1_STATE);
    }
}


static void powBackOffEnableSleep1(void)
{
    slpManSlpState_t pstate;
    uint8_t counter;
    slpManCheckVoteState(powBackOffVoteHandle, &pstate, &counter);
    if(counter == 1)
    {
        slpManPlatVoteEnableSleep(powBackOffVoteHandle, SLP_SLP1_STATE);
    }
}


static void powBackOffSendIntTemperature(int16_t value)
{
    powBackOffMsg_t msg;

    msg.type = POWBACKOFF_INTTEMP;
    msg.adcIntTemp = value;

    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffSendIntTemperature_0, P_VALUE, "power BackOff message send: type = %d, value = %d", msg.type, msg.adcIntTemp);
    
    if (powBackOffQueueHandle)
    {
        if (osOK != osMessageQueuePut(powBackOffQueueHandle,&msg, 0, 0))
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffSendIntTemperature_1, P_ERROR, "power BackOff message send in isr error");
        }
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffSendIntTemperature_2, P_ERROR, "power BackOff queue not ready in isr");
    }
}

static void powBackOffUpdateStatus(powBackOffStatus_e status)
{
    uint32_t mask = SaveAndSetIRQMask();
    gPowBackOffStatus = status;
    RestoreIRQMask(mask);
}

static powBackOffStatus_e powBackOffGetStatus(void)
{
    return gPowBackOffStatus;
}


static void  powBackOffCheck(void *argument)
{
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffCheck_1, P_VALUE, "Power BackOff Timer Expired, sample internal temperature again");
    cpADCProxyStartChannel((1<<ADC_PROXY_INTTEMP));
}


static void powBackOffStartReconnectTimer(uint32_t delayTicks)
{
    osStatus_t ret;

    if(powBackOffGetStatus() == POWBACKOFF_DISABLED)        // backoff is not enabled
        return;
    
    if(powBackOffReconnetTimer == NULL)
    {
        powBackOffReconnetTimer = osTimerNew((osTimerFunc_t)powBackOffCheck, osTimerPeriodic, NULL, NULL);
        if(powBackOffReconnetTimer != NULL)
        {
            ret = osTimerStart(powBackOffReconnetTimer, delayTicks);
            if(ret != osOK)
            {
                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffStartReconnectTimer_1, P_ERROR, "Power BackOff Timer restart failed, ret = %d", ret);
            }
            else
            {
                powBackOffDisableSleep1();
            }
        }
    }
}

static void powBackOffStopReconnectTimer(void)
{
    if(powBackOffReconnetTimer != NULL)
    {
        osTimerStop(powBackOffReconnetTimer);
        osTimerDelete(powBackOffReconnetTimer);
        powBackOffReconnetTimer = NULL;
        powBackOffEnableSleep1();
    }
}

static void powBackOffTask(void *arg)
{
    powBackOffMsg_t msg;
    CmsRetId cmsRet = CMS_RET_SUCC;
    int16_t temperature = 0;
    uint8_t cfun = 0;
    adcProxyRegisterResultInd(powBackOffSendIntTemperature, NULL);

    while(1)
    {
        if (osMessageQueueGet(powBackOffQueueHandle, &msg, 0, cmsisMAX_DELAY) == osOK)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_1, P_VALUE, "Power BackOff task: Type=%d, Temp=%d, status = %d", msg.type, msg.adcIntTemp>>4, powBackOffGetStatus());
            if(msg.type == POWBACKOFF_INTTEMP)
            {
                temperature = msg.adcIntTemp >> 4;

                if((temperature < POWBACKOFF_THRESHOLD_LEVEL0) && (powBackOffGetStatus() == POWBACKOFF_CFUN_0))
                {
                    cmsRet = CMS_RET_SUCC;
                    appGetCFUN(&cfun);
                    if(cfun == 0)
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_2, P_VALUE, "powBackoff setCFUN1");
                        cmsRet = appSetCFUN(1);
                        if(cmsRet != CMS_RET_SUCC)
                            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_3, P_WARNING, "powBackoff setCFUN1 failed ret=%d", cmsRet);
                    }
                    if(CMS_RET_SUCC == cmsRet)
                    {
                        powBackOffUpdateStatus(POWBACKOFF_DISABLED);
                        powBackOffStopReconnectTimer();
                    }
                }
                else if((temperature < POWBACKOFF_THRESHOLD_LEVEL0) && (powBackOffGetStatus() == POWBACKOFF_ENABLED))
                {
                    TxPowerSettingReq txPowerSettingReq;
                    memset(&txPowerSettingReq, 0, sizeof(txPowerSettingReq));
                    txPowerSettingReq.reqType = CMI_DEV_TX_POWER_SET_FIXED_POWER;
                    txPowerSettingReq.setTxPowerFixedReq.minPowerPresent = true;
                    txPowerSettingReq.setTxPowerFixedReq.maxPowerPresent = true;
                    txPowerSettingReq.setTxPowerFixedReq.maxPower = TX_POWER_VAL_MAX;
                    txPowerSettingReq.setTxPowerFixedReq.minPower = TX_POWER_VAL_MIN;
                    cmsRet = appSetTxPowerSetting(&txPowerSettingReq);
                    if(cmsRet == CMS_RET_SUCC)
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_4, P_VALUE, "powBackoff cancel Success, maxPower=%d", txPowerSettingReq.setTxPowerFixedReq.maxPower);
                        powBackOffUpdateStatus(POWBACKOFF_DISABLED);
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_5, P_VALUE, "powBackoff cancel failed, may set again later, cmsRet=%d", cmsRet);
                    }
                }
                else if((temperature >= POWBACKOFF_THRESHOLD_LEVEL1) && (temperature < POWBACKOFF_THRESHOLD_LEVEL2) && (powBackOffGetStatus() == POWBACKOFF_DISABLED))
                {
                    TxPowerSettingReq txPowerSettingReq;
                    memset(&txPowerSettingReq, 0, sizeof(txPowerSettingReq));
                    txPowerSettingReq.reqType = CMI_DEV_TX_POWER_SET_FIXED_POWER;
                    txPowerSettingReq.setTxPowerFixedReq.minPowerPresent = true;
                    txPowerSettingReq.setTxPowerFixedReq.maxPowerPresent = true;
                    txPowerSettingReq.setTxPowerFixedReq.maxPower = TX_POWER_VAL_MAX-POWBACKOFF_MINUS_VALUE;
                    txPowerSettingReq.setTxPowerFixedReq.minPower = TX_POWER_VAL_MIN;
                    cmsRet = appSetTxPowerSetting(&txPowerSettingReq);
                    if(cmsRet == CMS_RET_SUCC)
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_6, P_VALUE, "powBackoff apply Success, maxPower=%d", txPowerSettingReq.setTxPowerFixedReq.maxPower);
                        powBackOffUpdateStatus(POWBACKOFF_ENABLED);
                    }
                    else
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_7, P_VALUE, "powBackoff apply failed, may set again later, cmsRet=%d", cmsRet);
                    }
                }
                else if(temperature >= POWBACKOFF_THRESHOLD_LEVEL2)
                {
                    cmsRet = CMS_RET_SUCC;
                    appGetCFUN(&cfun);
                    if(cfun == 1)
                    {
                        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_8, P_VALUE, "powBackoff setCFUN0");
                        cmsRet = appSetCFUN(0);
                        if(cmsRet != CMS_RET_SUCC)
                            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffTask_9, P_WARNING, "powBackoff setCFUN0 failed ret=%d", cmsRet);
                    }
                    if(CMS_RET_SUCC == cmsRet)
                    {
                        powBackOffUpdateStatus(POWBACKOFF_CFUN_0);
                        powBackOffStartReconnectTimer(5000);
                    }
                }

            }

        }

    }


}




static void powBackOffThreadInit(void)
{
    if(powBackOffTaskExist == false)
    {
        powBackOffTaskExist = true;

        powBackOffQueueHandle = osMessageQueueNew(POWBACKOFF_EVENT_QUEUE_SIZE, sizeof(powBackOffMsg_t), NULL);
        if(powBackOffQueueHandle == NULL)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffThreadInit_1, P_ERROR, "Power BackOff task queue init error");
        }

        osThreadAttr_t task_attr;
        memset(&task_attr,0,sizeof(task_attr));
        task_attr.name = "powBack";
        task_attr.stack_size = POWBACKOFF_TASK_STATK_SIZE;
        #if defined FEATURE_LITEOS_ENABLE
        task_attr.priority = osPriorityNormal2;
        #elif defined FEATURE_FREERTOS_ENABLE
        task_attr.priority = osPriorityNormal1;
        #endif
#ifdef TYPE_EC718M
        task_attr.reserved = osThreadDynamicStackAlloc;
#endif
        powBackOffTaskId = osThreadNew(powBackOffTask, NULL, &task_attr);
    }
}


void powBackOffEnable(void)
{
    slpManRet_t ret;
    ret = slpManApplyPlatVoteHandle("powBack", &powBackOffVoteHandle);
    if(ret != RET_TRUE)
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, powBackOffEnable_1, P_ERROR, "Power BackOff Vote handle error");

    powBackOffThreadInit();
}


void powBackOffDisable(void)
{
    if(powBackOffVoteHandle != 0xFF)
        slpManPlatVoteForceEnableSleep(powBackOffVoteHandle, SLP_SLP1_STATE);
    powBackOffVoteHandle = 0xFF;

    if(powBackOffTaskExist && (powBackOffTaskId != NULL))
    {
        osThreadTerminate(powBackOffTaskId);
        powBackOffTaskId = NULL;
        powBackOffTaskExist = false;
    }
}


