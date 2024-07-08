/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: radio_device.h
*
*  Description:
*
*  History: 2023/7/17 created by hyang
*
*  Notes:
*
******************************************************************************/
#ifndef RADIO_DEVICE_H
#define RADIO_DEVICE_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_base.h"
#include "ccio_misc.h"
#include "ccio_opaq.h"
#include "ccio_pub.h"
#include "i2s.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

/* only one device for ctrl plane */
#define I2S_DEV_REAL_MAXNUM    1

#define I2S_STOP_RECORD_CMD         0x1234bad9
#define I2S_STOP_PLAY_CMD           0xace59876

#define I2S_SEM_INTERNAL_TIMEOUT    3000

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

/**
 * configuration about a i2s device.
 */
typedef struct
{
    I2sDrvInterface_t  *drvHandler;      /**< i2s driver handler */
    uint8_t             mainUsage;       /**< refer to 'CsioDevType_e' */
    uint8_t             bmCreateFlag;    /**< refer to 'CcioTaskOperFlag_e', bitmap type */
    uint16_t            rbufFlags   :4;  /**< which rbuf will be used? refer to 'CcioRbufUsage_e' */
    uint16_t            rsvdBits    :12;
    uint32_t            custFlags   :4;  /**< flags for customers' private purpose */
    uint32_t            custExtras  :16; /**< extra info for customers' private purpose */
    uint32_t            rsvdBits2   :12;
}I2sDevConf_t;

typedef struct
{
    I2sDrvInterface_t  *drvHandler;

    uint16_t  isTriggEn    :1;
    uint16_t  isThresEn    :1;
    uint16_t  isInputEn    :1;
    uint16_t  xbatchFsm    :2;

    uint16_t  xferCnt;
    uint16_t  readIdx;
    uint16_t  writeIdx;
    uint16_t  cfgRecvSize;
    uint16_t  totalSize;
    uint8_t   *rxBuffer;
    uint8_t   codecType;
    uint16_t  freeSize;
    bool      needPlayTone;
    bool      needCodecInitAgain;
    uint8_t   needRecord;
    bool      isStopRec;
    bool      isToneOn;
    bool      isInitDone;
}I2sDevRxWrap_t;

typedef struct
{
    uint8_t   voiceType;
    uint8_t   sampleRate;
    uint8_t   pcmBitWidth;
    uint16_t  speechBufSize;
    uint32_t  speechBufAddr;
}I2sPlayParams_t;

typedef struct
{
    void*     ulpduBk;
    void*     playParamBk;
}I2sChdevExtraParam_t;

typedef enum
{
    AUDIO_OWNER_NONE          = 0,
    AUDIO_OWNER_APP_PLAY      = 1,
    AUDIO_OWNER_APP_RECORD    = 2,
    AUDIO_OWNER_IMS           = 3,
    AUDIO_OWNER_AT            = 4
}I2sOwner_e;

typedef struct
{
    uint16_t   onDmaCnt;
    uint16_t   onDmaCntBak;
    uint16_t   offDmaCnt;
    uint16_t   offDmaCntBak;
    uint8_t    *pToneSrc;
}I2sStandardTone_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief i2sDevCreate(uint8_t cid)
 * @details create a radio device of dtype with 'cid'
 *
 * @param pdpCid    pdp context id
 * @param radioConf The configuration about a radio device
 * @return 0 succ; < 0 failure with errno.
 */
int32_t i2sCreate(I2sDevConf_t *i2sConf);

int32_t i2sDevRecv(uint8_t type, uint8_t isCodecNeedInit);
int32_t i2sDevDeInit(bool isHangup);
int32_t i2sDevPlay(CcioDevice_t *chdev);
int32_t i2sStopHandle(uint8_t stopType, uint8_t audioType);


/**
 * @brief Set ring tone on local.
 *
 * @param toneSrc               Tone PCM src array. Defined in rawData.c/callAlertRing16k, change it to your own pcm data.
 * @param toneSize              Tone total size, the "callAlertRing16k" array size.
 * @param dmaTrunkSize      DMA trunk size every transfer, maximux value is 8k-1.
 *                                          Change it to a value that can be divided by toneSize, it means dmaDescriptorNum = toneSize /dmaTrunkSize.
 * @param sampleRateUse    If use 8k samplerate, set it to 8000; If use 16k samplerate, set it to 16000;
 * @return null
 */
void i2sChangeRingTone(uint8_t* toneSrc, uint32_t toneSize, uint16_t dmaTrunkSize, uint8_t sampleRateUse);
void i2sFastCreate();



#ifdef __cplusplus
}
#endif
#endif

