#ifndef __PS_DIAL_IMS_H__
#define __PS_DIAL_IMS_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of AirM2M Ltd.
 File name:      - psdial_ims.h
 Description:    - psdial ims header
 History:        - 2023/11/27, Originated
 ******************************************************************************
******************************************************************************/
#ifdef FEATURE_IMS_ENABLE

#include "commontypedef.h"

/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/



/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/



/******************************************************************************
 ******************************************************************************
 * External global variable
 ******************************************************************************
******************************************************************************/



/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/

/**
 * PS dial create IMS tasks
*/
void psDialCreateImsTasks(void);

/**
 * Wake up IMS (modem), PS can't process any other signal/command before wake up confiramtion,
 *  here, all signal need to pending (enqueue) before wake up confiramtion from IMS
*/
void psDialWakeupIms(void);


void psDialGetImsMedStackMem(UINT16 statckSize, void **pStackMem, void **pTaskCbMem);

void *psDialAllocImsMemory(UINT16 size);

void *psDialAllocImsZeroMemory(UINT16 size);

void *psDialAllocImsMemoryNoAssert(UINT16 size);

void *psDialAllocImsZeroMemoryNoAssert(UINT16 size);

void psDialFreeImsMemory(void **pMem);

BOOL psDialImsMemoryIsPsram(void);

void psDialGetImsMedSigQueueMem(UINT16 sigQSize, void **pSigQMem);

void psDialGetImsMedDtmfQueueMem(UINT16 dtmfQSize, void **pDtmfQMem);

#endif

#endif

