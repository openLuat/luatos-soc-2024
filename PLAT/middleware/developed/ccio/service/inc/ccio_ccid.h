/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_opaq.h
*
*  Description:
*
*  History: 2023/1/31 created by bchang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_CCID_H
#define CCIO_CCID_H

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


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
  \fn     int32_t ccidDataInput(UlPduBlock_t *ulpdu, void *extras)
  \brief  to further handle ccid data received from ccid device
  \return
  \note   invoked by RxTask automatically once ccid data is received
*/
int32_t ccidDataInput(UlPduBlock_t *ulpdu, void *extras);

/**
  \fn     int32_t ccidDataOutput( uint8_t *sendBuf, uint16_t len)
  \brief  to send ccid data out of ccid device
  \return
  \note   invoked by customer's AppTask for outputting ccid data
*/
int32_t ccidDataOutput( uint8_t *sendBuf, uint16_t len);



#ifdef __cplusplus
}
#endif
#endif

