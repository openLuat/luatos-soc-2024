/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_ipos.h
*
*  Description:
*
*  History: 2024/4/19 created by xuwang
*
*  Notes: IP packet Over Spi
*
******************************************************************************/
#ifndef CCIO_IPOS_H
#define CCIO_IPOS_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_misc.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define  IPOS_MULTI_CIDS_ENABLE    0


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint16_t bmPdpCid;  /* bitmap of cid */
    uint8_t  rsvd[2];
}IposPathConf_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
/**
  \fn     iint32_t iposDataPathInit(OpaqosEntity_t *iposEnt, IposPathConf_t *pathConf)
  \brief  ipos channel key context initialization
  \return
  \note   invoked in function 'opaqActivateCtx()'
*/
int32_t iposDataPathInit(OpaqosEntity_t *iposEnt, IposPathConf_t *pathConf);

/**
  \fn     iint32_t iposDataPathDeinit(OpaqosEntity_t *iposEnt)
  \brief  ipos channel key context de-initialization
  \return
  \note   invoked in function 'opaqDeactivateCtx()'
*/
int32_t iposDataPathDeinit(OpaqosEntity_t *iposEnt);

/**
  \fn     int32_t iposDataInput(UlPduBlock_t *ulpdu, OpaqosEntity_t *iposEnt)
  \brief  to further handle ipos data via opaq channel
  \return
  \note   invoked in function 'opaqDataInput()'
*/
int32_t iposDataInput(UlPduBlock_t *ulpdu, OpaqosEntity_t *iposEnt);

/**
  \fn     int32_t iposDataOutput(uint8_t pdpCid, DlPduBlock_t *dlpdu)
  \brief  to send ipos data via opaq channel
  \return
  \note   invoked by up taks
*/
int32_t iposDataOutput(uint8_t pdpCid, DlPduBlock_t *dlpdu);

#ifdef __cplusplus
}
#endif
#endif

