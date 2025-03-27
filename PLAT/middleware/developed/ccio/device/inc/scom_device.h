/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: scom_device.h
*
*  Description:
*
*  History: 2025/1/19 created by xuwang
*
*  Notes: virtual Simulated COM
*
******************************************************************************/
#ifndef SCOM_DEVICE_H
#define SCOM_DEVICE_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_pub.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef struct
{
    ScomInstDrv_t  *drvHandler;      /**< scom driver handler */
    uint8_t         mainUsage;       /**< refer to 'CsioDevType_e' */
    uint8_t         bmCreateFlag;    /**< refer to 'CcioTaskOperFlag_e', bitmap type */
    uint8_t         rsvd[2];
    uint32_t        rbufFlags    :4;  /**< which rbuf will be used? refer to 'CcioRbufUsage_e' */
    uint32_t        custFlags    :4;  /**< flags for customers' private purpose */
    uint32_t        custExtras   :16; /**< extra info for customers' private purpose */
    uint32_t        isWaitDevRdy :1;  /**< wait for device ready to work or not? */
    uint32_t        rsvdBits     :7;
}ScomDevConf_t;


typedef union
{
    uint32_t  extras;
    struct
    {
        uint32_t  custFlags    :4;  /**< flags for customers' private purpose */
        uint32_t  custExtras   :16; /**< extra info for customers' private purpose */
        uint32_t  isWaitDevRdy :1;  /**< wait for device ready to work or not? */
        uint32_t  rsvdBits     :11;
    }config;
}ScomDevXformExtras_u;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief scomDevCreate(uint32_t scomIdx, ScomDevConf_t *scomConf)
 * @details create a scom device of 'scomIdx'
 *
 * @param scomIdx   The index of uart port
 * @param scomConf  The configuration about a uart device
 * @return NULL failure; !NULL the created scom device.
 */
CcioDevice_t* scomDevCreate(uint32_t scomIdx, ScomDevConf_t *scomConf);

/**
 * @brief scomDevDestroy(uint32_t uartIdx)
 * @details destroy/delete a scom device of 'scomIdx'
 *
 * @param scomIdx index of scom port
 * @return 0 succ; < 0 failure with errno.
 */
int32_t scomDevDestroy(uint32_t scomIdx);

/**
 * @brief scomDevTransform(uint32_t scomIdx, CsioDevType_e newType, uint32_t extras)
 * @details transform a scom device into a new type
 *
 * @param scomIdx   The index of scom port
 * @param newType   The new stype of the scom device
 * @param extras    The extra info for new device
 * @return NULL failure; !NULL the transformed scom device.
 */
CcioDevice_t* scomDevTransform(uint32_t scomIdx, CsioDevType_e newType, uint32_t/*ScomDevXformExtras_u*/ extras);


#ifdef __cplusplus
}
#endif
#endif

