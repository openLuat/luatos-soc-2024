/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_netdata.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_NETDATA_H
#define CCIO_NETDATA_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_base.h"
#include "ccio_misc.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
void  netifSetFastChkInfo(void *fastChkInfo, UINT16 infoLen);
void* netifGetFastChkInfo(void);

int32_t netifActivateLanCtx(CcioLanMediaType_e lanMedia, void *lanEnt, void *args);
int32_t netifDeactivateLanCtx(CcioLanMediaType_e lanMedia, void *lanEnt);

void* netifGetLanSubxEnt(uint8_t netCid);

void netifLanLinkUp(CcioLanLinkUp_t *linkUp);
void netifLanLinkDown(CcioLanLinkDown_t *linkDown);
void netifLanLinkStatusChange(CcioLanMedia_t *linkStatus);

int32_t ethDataInput(UlPduBlock_t *ulpdu, void *extras);
int32_t ethDataOutput(uint8_t lanType, DlPduBlock_t *head, DlPduBlock_t *tail);
int32_t ethDataFastOutput(uint8_t lanType);

int32_t ppposDataInput(void *pppPcb, uint8_t *data, uint16_t len);
int32_t ppposUlPduInput(void *pppPcb, UlPduBlock_t *ulpdu);
DlPduBlock_t* ppposDlPduEscape(void *pppPcb, DlPduBlock_t *dlpdu);

int32_t netDataInput(UlPduBlock_t *ulpdu, void *extras);
int32_t netDataOutput(DlPduBlock_t *head, DlPduBlock_t *tail);
int32_t netDataFastOutput(void);

int32_t netDataAddToInputList(UlPduBlock_t *ulpdu, void *extras);
int32_t netDataInputIfAny(void *extras);

#ifdef __cplusplus
}
#endif
#endif

