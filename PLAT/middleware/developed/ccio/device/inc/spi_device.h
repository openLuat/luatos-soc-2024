/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: spi_device.h
*
*  Description:
*
*  History: 2023/12/12 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef SPI_DEVICE_H
#define SPI_DEVICE_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_pub.h"
#include "ccio_base.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
/* 'flags' definition, used for getting RxBuf! */
typedef enum
{
    SPI_DEV_RBF_FOR_NEXT = 0,
    SPI_DEV_RBF_FOR_INIT
}SpiDevRxBufFlags_e;

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

/**
 * configuration about a spi device.
 */
typedef struct
{
    uint8_t    mainUsage;       /**< refer to 'CsioDevType_e' */
    uint8_t    bmCreateFlag;    /**< refer to 'CcioTaskOperFlag_e', bitmap type */
    uint16_t   rbufFlags   :4;  /**< which rbuf will be used? refer to 'CcioRbufUsage_e' */
    uint16_t   custFlags   :4;  /**< flags for customers' private purpose */
    uint16_t   rsvdBits    :8;
}SpiDevConf_t;



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief spiDevCreate(uint8_t spiIdx, SpiDevConf_t *devConf)
 * @details create a spi device of specific Idx
 *
 * @param spiIdx   spi device idx
 * @param devConf  The configuration about the spi device
 * @return 0 succ; < 0 failure with errno.
 */
int32_t spiDevCreate(uint8_t spiIdx, SpiDevConf_t *devConf);

/**
 * @brief spiDevDestroy(uint8_t spiIdx)
 * @details destroy/delete a spi device of specific Idx
 *
 * @param spiIdx   spi device idx
 * @return 0 succ; < 0 failure with errno.
 */
int32_t spiDevDestroy(uint8_t spiIdx);

/**
 * @brief spiDevTransform(uint32_t spiIdx, CsioDevType_e newType)
 * @details transform a spi device into a new type
 *
 * @param spiIdx   spi device idx
 * @param newType   The new stype of the device
 * @return NULL failure; !NULL the transformed device.
 */
CcioDevice_t* spiDevTransform(uint32_t spiIdx, CsioDevType_e newType);

/**
 * @brief spiDevInput(uint8_t spiIdx, uint32_t xferCnt, uint32_t extras)
 * @details input data via spi device
 *
 * @param spiIdx   spi device idx
 * @param xferCnt  data length, the same as the len field in spi hdr
 * @param extras   extra neccesary info
 * @return 0 succ; < 0 failure with errno.
 */
int32_t spiDevInput(uint8_t spiIdx, uint32_t xferCnt, uint32_t extras);

/**
 * @brief spiDevTryGetRxBuf(uint8_t spiIdx, uint8_t flags, uint16_t consumed)
 * @details try to get the rbuf for initialization or next xfer
 *
 * @param spiIdx   spi device idx
 * @param xferCnt  the data length that has already been received
 * @return 0 succ; < 0 failure with errno.
 */
void* spiDevTryGetRxBuf(uint8_t spiIdx, uint16_t xferCnt);

/**
 * @brief spiDevPickTxBuf(uint8_t spiIdx, CcioBufDesc_t *desc)
 * @details get the buf to be sent
 *
 * @param spiIdx   spi device idx
 * @param desc     details about the buf
 * @return 0 succ; < 0 failure with errno.
 */
int32_t spiDevPickTxBuf(uint8_t spiIdx, CcioBufDesc_t *desc);

/**
 * @brief spiDevFreeTxBuf(void *buf)
 * @details free the buf that has been sent
 *
 * @param spiIdx   spi device idx
 * @param buf      the buf addr
 * @return 0 succ; < 0 failure with errno.
 */
int32_t spiDevFreeTxBuf(uint8_t spiIdx, void *buf);


#ifdef __cplusplus
}
#endif
#endif

