/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: Hal_i2s.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: audio driver
*
******************************************************************************/


#ifndef _AUDIO_DRV_H
#define _AUDIO_DRV_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "i2s.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/




 /*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef enum
{
    SAMPLERATE_8K           = 0,        ///< Sample rate 8k
    SAMPLERATE_16K          = 1,        ///< Sample rate 16k
    SAMPLERATE_22_05K       = 2,        ///< Sample rate 22.05k
    SAMPLERATE_24K	        = 3,        ///< Sample rate 24k
    SAMPLERATE_32K          = 4,        ///< Sample rate 32k
    SAMPLERATE_44_1K        = 5,        ///< Sample rate 44.1k
    SAMPLERATE_48K          = 6,        ///< Sample rate 48k
    SAMPLERATE_96K          = 7,        ///< Sample rate 96k
}I2sSampleRate_e;

typedef enum
{
    FRAME_SIZE_16_16        = 0,        ///< WordSize 16bit, SlotSize 16bit
    FRAME_SIZE_16_32        = 1,        ///< WordSize 16bit, SlotSize 32bit
    FRAME_SIZE_24_32        = 2,        ///< WordSize 24bit, SlotSize 32bit
    FRAME_SIZE_32_32        = 3,        ///< WordSize 32bit, SlotSize 32bit
}I2sFrameSize_e;

typedef enum
{
    PLAY                    = 0,        ///< Audio play once
    RECORD                  = 1,        ///< Audio record
    PLAY_RECORD             = 2,        ///< Audio play/record
    PLAY_LOOP               = 3,        ///< Audio play loop
    PLAY_LOOP_IRQ           = 4,        ///< Audio play loop irq
    RECORD_LOOP_IRQ         = 5,        ///< Audio record loop irq
}I2sPlayRecord_e;

typedef enum
{
    MONO                    = 0,
    DUAL_CHANNEL            = 1,
}I2sChannelSel_e;

typedef enum
{
    I2S_TX                  = 0,
    I2S_RX                  = 1,
}I2sDirectionSel_e;

typedef enum
{
    POLARITY_0              = 0,
    POLARITY_1              = 1,
}I2sBclkPolarity_e;


typedef struct
{
    I2sMode_e               mode;       ///< Audio mode choose
    I2sRole_e               role;       ///< Role choose
    I2sSampleRate_e         sampleRate; ///< Sample rate choose
    I2sFrameSize_e          frameSize;  ///< Frame size choose
    I2sBclkPolarity_e       polarity;   ///< Bclk polarity choose
    I2sChannelSel_e         channelSel; ///< Mono or dual channel select
    uint32_t                totalNum;   ///< Audio source total num
}I2sParamCtrl_t;

// 2. Dynamic control part
typedef enum
{
    STOP_SEND               = 0,        // only stop send
    STOP_RECV               = 1,        // only stop recv
    STOP_ALL                = 2,        // stop send and recv
    START_SEND              = 3,        // only start send   
    START_RECV              = 4,        // only start recv 
    START_ALL               = 5,        // start send and recv
}I2sStartStop_e;

typedef enum
{
    VOLUMN_INCREASE         = 0,        ///< Volumn increase
    VOLUMN_DECREASE         = 1,        ///< Volumn decrease
}I2sVolumnCtrl_e;

typedef void (*i2sCbFunc_fn) (uint32_t event, uint32_t arg);  ///< I2S init callback event.


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

/**
  \brief Init I2S interface, include pinMux, and enable clock.
  \param[out] powerCtrl     Enable or disable I2S clock.
  \param[out] txCb     Indicate that i2s tx operation has been done.
  \param[out] rxCb     Indicate that i2s rx operation has been done.

  \return              
*/
void halI2sInit(i2sCbFunc_fn txCb, i2sCbFunc_fn rxCb);


/**
  \brief Configure i2s.
  \param[in] paramCtrl     Every i2s control parameters
  \return
  \details

  1. Parameter "paramCtrl" involves all the parameters that need to set in i2s development. 
    (1). i2sCodecType_e          codecType
    Choose the codec that you use.

    (2). i2sMode_e               mode
    We support 4 I2S interface modes, choose the one you need.
    Note! Since I2S mode will have phase difference with other 3 modes, so please note that if you connect a speaker
    in the LOUT1 or LOUT2, I2S mode will change to another speaker outer pin than other 3 modes.

    (3). i2sRole_e               role
    Codec acts as master or slave, so our controller will act as slave or master accordingly.

    (4). i2sSampleRate_e         sampleRate
    Choose the Sample rate you need.

    (5). i2sFrameSize_e          frameSize
    WordSize shouldn't bigger than slotSize. WordSize is the real size you use, and slotSize is the length that one
    frame occupys. 

    (6). i2sPlayRecord_e         playRecord
    Choose play audio or record audio.

    (7). i2sChannelSel_e		 channelSel
    Choose mono or dual channel.

    (8). uint32_t                totalNum
	When playing, the total size of audio source.
*/
void halI2sConfig(I2sParamCtrl_t paramCtrl);

/**
  \brief Use i2s interface to play or record audio.
  \param[in] playRecord     Play or record.
  \param[in,out] memAddr    Audio source buffer when playing or recording
  \param[in] trunkSize      The size of everytime send or receive data.
  \return
*/
void halI2sTransfer(I2sPlayRecord_e playRecord, uint8_t* memAddr, uint32_t trunkNum);

/**
  \brief Start play/record audio or stop play/record audio.
  \param[in] startStop     Start or stop play/record audio.
  \return              
*/
int32_t halI2sStartStop(I2sStartStop_e startStop);

/**
  \brief Set total num of sending data to i2s.
  \param[in] totalNum     Total num of data.
  \return              
*/
void halI2sSetTotalNum(uint32_t totalNum);

/**
  \brief Get total num of data.
  \return  Total num of data.            
*/
uint32_t halI2sGetTotalNum(void);


/**
  \brief Set dma descriptor chain num.  
  \param[in] direc     Tx/Rx.
  \param[in] num     dma descriptor num.
  \return  null.            
*/
void halI2sSetDmaDescriptorNum(I2sDirectionSel_e direc, uint8_t num);

/**
  \brief If codec don't have the ablity to adjust volumn, using this api to achieve it.
  \param[in] srcBuf        Audio source address of playing.
  \param[in] srcTotalNum   Total num of audio source for playing.
  \param[in] volScale      Increase or decrease the volumn. 
  \details
    1. Mute, write 00 for volScale.
  	1. Decrease volumn to 50% of original, write 05 for volScale.
  	2. Increase volumn to 5 times of original, write 50 for volScale.
  	3. Increase volumn to 10 times of original, write 100 for volScale.
  	4. "volScale" can change from 00(mute), 01(10%), 02(20%)......30(300%), 40(400%), 100(1000%)......
  \return              
*/
void halI2sSrcAdjustVolumn(int16_t* srcBuf, uint32_t srcTotalNum, uint16_t volScale);

/**
  \brief Set i2s samplerate.  
  \param[in] i2sRole     i2s acts as master or slave.
  \param[in] sampleRate     samplerate.
  \return  null.            
*/
void halI2sSetSampleRate(I2sRole_e i2sRole, I2sSampleRate_e sampleRate);

/**
  \brief Set i2s mono or dual channel.  
  \param[in] channelSel    select mono or dual channel.
  \return  null.            
*/
void halI2sSetChannel(I2sChannelSel_e channelSel);

/**
  \brief deinit i2s driver.  
  \return  null.            
*/
void halI2sDeInit();

/**
  \brief Get every dma transfer trunk num.  
  \return  null.            
*/
uint32_t halI2sGetTrunkNum();


/** \} */

#ifdef __cplusplus
}
#endif

#endif /* _AUDIO_DRV_H */
