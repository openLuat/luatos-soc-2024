/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_opaq.h
*
*  Description:
*
*  History: 2022/3/15 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_OPAQ_H
#define CCIO_OPAQ_H

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
typedef enum
{
    OPAQ_CHAN_0 = 0,
    OPAQ_CHAN_RSVD = OPAQ_CHAN_0,    /* OPAQ_CHAN_0 is rsvd! */
    OPAQ_CHAN_1,
    OPAQ_CHAN_CUST1 = OPAQ_CHAN_1,   /* rename OPAQ_CHAN_1 to whatever customer wants */
    OPAQ_CHAN_2,
    OPAQ_CHAN_CUST2 = OPAQ_CHAN_2,   /* rename OPAQ_CHAN_2 to whatever customer wants */
#ifdef __USER_CODE__
    OPAQ_CHAN_3,
    OPAQ_CHAN_CUST3 = OPAQ_CHAN_3,   /* rename OPAQ_CHAN_3 to whatever customer wants */
#endif
    OPAQ_CHAN_MAXNUM
}OpaqChanId_e;

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
  \fn     int32_t opaqDataInput(UlPduBlock_t *ulpdu, void *extras)
  \brief  to further handle opaq data received from serial device
  \return
  \note   invoked by RxTask automatically once opaq data is received
*/
int32_t opaqDataInput(UlPduBlock_t *ulpdu, void *extras);

#if 0
/**
  \fn     int32_t opaqDataOutput(uint8_t chanNo, DlPduBlock_t *dlpdu, void *extras)
  \brief  to send opaq data out of serial device via chanNo
  \return
  \note   invoked by customer's AppTask for outputting opaq data
*/
int32_t opaqDataOutput(uint8_t chanNo, DlPduBlock_t *dlpdu, void *extras);
#endif

/**
  \fn     int32_t opaqDataOutputEx(uint8_t opaqCid, DlPduBlock_t *dlpdu, void *extras)
  \brief  to send opaq data out of serial device via opaqCid
  \return
  \note   invoked by customer's AppTask for outputting opaq data
*/
int32_t opaqDataOutputEx(uint8_t opaqCid, DlPduBlock_t *dlpdu, void *extras);

#ifdef __cplusplus
}
#endif
#endif

