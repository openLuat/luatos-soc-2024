

/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: hal_charge.c
*
*  Description: api for charge status detect
*
*  History: initiated by Zhao Weiqi
*
*  Notes:
*
******************************************************************************/
/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <string.h>
#include "hal_charge.h"
#include"ec7xx.h"
#include "cmsis_os2.h"
#include DEBUG_LOG_HEADER_FILE
#include "cmsis_os2.h"
#include "sctdef.h"



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
AP_PLAT_COMMON_BSS osTimerId_t chargeMonTimer;
AP_PLAT_COMMON_BSS static uint8_t chargeMonTimerId = 0;
AP_PLAT_COMMON_BSS chargeStatusCb chargeStatusCbFunc;

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/
static void chargeMonTimerExp(void *argument)
{
    chargeStatus_e chargeStatus = chargeGetCurStatus();
    if(chargeStatusCbFunc != NULL)
        chargeStatusCbFunc(chargeStatus);
}
/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
void chargeIntHandler(void)
{
    chargeStatus_e chargeStatus = chargeGetCurStatus();
    ECPLAT_PRINTF(UNILOG_PMU, chargeIntHandler_1, P_VALUE, "Charger Int Enter, Status Update = %d", chargeStatus);
    if(chargeStatusCbFunc != NULL)
        chargeStatusCbFunc(chargeStatus);
}


void chargeDetectInit(chargeStatusCb cb, bool monitorEn, uint32_t sample_dly)
{
    chargeHwInit();

    chargeStatusCbFunc = cb;

    chargeStatus_e chargeStatus = chargeGetCurStatus();

    ECPLAT_PRINTF(UNILOG_PMU, chargeDetectInit_1, P_VALUE, "Charger Detect Init, Status Update = %d", chargeStatus);

    if(chargeStatusCbFunc != NULL)
        chargeStatusCbFunc(chargeStatus);

    if(monitorEn)
    {
        if(chargeMonTimer == NULL)
        {
            chargeMonTimer = osTimerNew((osTimerFunc_t)chargeMonTimerExp, osTimerPeriodic, (void *)(uint32_t)chargeMonTimerId, NULL);
        }
        osTimerStart(chargeMonTimer, sample_dly);
    }
    NVIC_EnableIRQ(ChrgpadWakeup_IRQn);

}


void chargeDetectDeinit(void)
{
    chargeHwDeinit();
    NVIC_DisableIRQ(ChrgpadWakeup_IRQn);

    chargeStatusCbFunc = NULL;

    if(chargeMonTimer != NULL)
    {
        if(osTimerIsRunning(chargeMonTimer))
        {
            osTimerStop(chargeMonTimer);
        }
        osTimerDelete(chargeMonTimer);
        chargeMonTimer = NULL;
    }
}


