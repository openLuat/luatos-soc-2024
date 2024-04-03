#ifndef TCPIP_HDR_HIB_API_H
#define TCPIP_HDR_HIB_API_H

/******************************************************************************
 ******************************************************************************
 Copyright:      - 2019- Copyrights of AirM2M Ltd.
 File name:      - tcpiphibapi.h
 Description:    - tcpip layer hib/sleep2 mode related API
 History:        - 04/24/2019, Originated by xwang
 ******************************************************************************
******************************************************************************/
#ifdef WIN32
#include "winsys.h"
#else
#include "commontypedef.h"
#endif


#define PSIF_DNSCACHE_MAX_URL_LEN 68

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

enum pcb_hib_state {
  PCB_HIB_DISABLE_DEACTIVE     = 0,
  PCB_HIB_ENABLE_DEACTIVE      = 1,
  PCB_HIB_ENABLE_ACTIVE        = 2,
};

enum pcb_sleep2_state {
  PCB_SLEEP2_DISABLE_DEACTIVE     = 0,
  PCB_SLEEP2_ENABLE_DEACTIVE      = 1,
  PCB_SLEEP2_ENABLE_ACTIVE        = 2,
};

typedef enum PsifSleepAllowType_Tag
{
    PSIFSLEEPALLOW_SLEEP1 = 0,
    PSIFSLEEPALLOW_SLEEP2 = 1,
    PSIFSLEEPALLOW_HIB    = 3,
}PsifSleepAllowType;




/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/

UINT8 PsifIsTcpipAllowEnterHIB(void);

void PsifRequestRecoverContextFromHib(void);

BOOL PsifGetPreTcpipHibContextInfo(UINT8 type);

BOOL PsifGetCurrTcpipHibContextInfo(UINT8 type);

void PsifSetCurrTcpipHibContext(BOOL exist, UINT8 type);

void PsifSetTcpipHibContextChanged(BOOL change, UINT8 type);

BOOL PsifGetTcpipHibContextChanged(UINT8 type);

void PisfCheckTcpipHibContextNeedChang(void);

void PsifTcpipAllowEnterHib(UINT8 sleepAllowType);

void PsifTcpipNotAllowEnterHib(void);

void PsifTcpipHibModeInit(void);

UINT32 PsifGetCurrHibTicks(void);

BOOL PsifCheckWetherRecoverFromHib(void);


#endif

