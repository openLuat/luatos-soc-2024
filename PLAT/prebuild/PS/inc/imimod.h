#ifndef __IMI_MOD_H__
#define __IMI_MOD_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imimod.h
 Description:    - IMS module/device interface
 History:        - 2023/04/10,   Original created
******************************************************************************/
#include "imicomm.h"

/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_MOD_PRIM_ID_TAG
{
    IMI_MOD_PRIM_BASE   = 0,

    IMI_MOD_POWER_WAKE_UP_REQ,      /* ImiModPowerWakeUpReq */
    IMI_MOD_POWER_WAKE_UP_CNF,

    IMI_MOD_PRIM_END    = 0x0fff
}IMI_MOD_PRIM_ID;


/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/


/**
 * PRIM ID: IMI_MOD_POWER_WAKE_UP_REQ
 * wakeup IMS from deep slp
*/
typedef ImiEmptySig ImiModPowerWakeUpReq;

/**
 * PRIM ID: IMI_MOD_POWER_WAKE_UP_CNF
*/
typedef ImiEmptySig ImiModPowerWakeUpCnf;



#endif

