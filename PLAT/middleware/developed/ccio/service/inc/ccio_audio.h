/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_opaq.h
*
*  Description:
*
*  History: 2023/7/27 created by hyang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_AUDIO_H
#define CCIO_AUDIO_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ccio_misc.h"
#include "mw_nvm_audio.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define CCIO_STOP_PLAY_FLAG      0x2
#define CCIO_STOP_RECORD_FLAG    0x3
#define CCIO_START_PLAY_FLAG     0x4
#define CCIO_START_RECORD_FLAG   0x5


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef void (*i2sToneCbFunc)(void*);
typedef void (*i2sHangupCbFunc)(void*);
typedef void (*i2sStopPlayCbFunc)(void*);
typedef void (*i2sStopRecordCbFunc)(void*);


/******************************************************************************
 *****************************************************************************
 * ENUM
 *****************************************************************************
******************************************************************************/
/*
* AudioSampleRate
*/
typedef enum audioSampleRateTag
{
    VOLTE_SAMPLE_RATE_INVALID     = 0,//invalid
    VOLTE_SAMPLE_RATE_8K          = 1,//sample rate 8000Hz
    VOLTE_SAMPLE_RATE_16K         = 2 //sample rate 16000Hz
    //...add if required
}AudioSampleRate_e;

/*
* AudioPlayType
*/
typedef enum audioPlayTypeTag
{
    PLAY_DIAL_TONE                  = 1, // dial tone
    PLAY_RINGING_TONE               = 2, // ringing tone
    PLAY_CONGESTION_TONE            = 3, // congestion tone
    PLAY_BUSY_TONE                  = 4, // busy tone
    PLAY_CALL_WAITING_TONE          = 5, // call waiting tone
    PLAY_MULTI_CALL_PROMPT_TONE     = 6, // multi call prompt tone
    PLAY_CALL_ALERT_RINGING         = 7, // incoming call alert ringing
    PLAY_SPEECH_PCM_DATA            = 8, // early media or voice in speech buffer
    PLAY_MULTI_MEDIA                = 9
    //...add if required
}AudioPlayType_e;

typedef struct
{
    uint8_t  getOrSet;
    uint32_t atHandle;
    uint8_t  speakerGain;
    uint8_t  speakerSetVal;
    uint8_t  micSetGain;
    uint8_t  micSetVol;
}AudioCodec_t;

typedef struct
{
    uint8_t  getOrSet;
    int32_t  rspVal;
    uint8_t  micGain;
    int32_t  micVol;
}AudioCodecRsp_t;

typedef struct
{
    // internal use, not used by IMS/APP
    uint8_t  toneType;
    uint8_t  toneTrunkIndex; // if toneSrc is not 20ms len, will use trunkIndex and trunkNum
    uint8_t  toneTrunkNum;
    uint8_t  *toneSrcAddr;
}AudioCombineToneInter_t;

typedef enum
{
    APP_DONE    = 0,
    APP_WORKING = 1
}AudioAppState_e;

#define    CODEC_FAST_INIT              (0)
#define    CODEC_FAST_DEINIT            (1)
#define    CODEC_SPEAKER_VAL_SET        (2)
#define    CODEC_SPEAKER_VAL_GET        (3)
#define    CODEC_MIC_VAL_SET            (4)
#define    CODEC_MIC_VAL_GET            (5)


#define    CODEC_IND_INDEX  (1)



/*----------------------------------------------------------------------------*
 *                    PRIVATE FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
int32_t audioDataInput(UlPduBlock_t *ulpdu, void *extras);
int32_t audioDataOutput(uint8_t chanNo, DlPduBlock_t *dlpdu, void *extras);
int32_t audioDataOutputEx(uint8_t audioCid, DlPduBlock_t *dlpdu, void *extras);

void audioFreeRecordBuf(void *ulpdu);

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION : called by IMS                         *
 *----------------------------------------------------------------------------*/

/**
  \fn          audioDrvInit
  \brief       this api called by media task to init CCIO rx/tx I2S, buffer... when call setup
  \param[in]   null
  \returns     BOOL, TRUE -init buffer memory, etc. ok; FALSE - init fail, shall stop call precesure
  \NOTE:       thsi api MUST be sync interface without block
*/
int32_t audioDrvInit(uint8_t owner);

/**
  \fn          audioDrvDeInit
  \brief       this api called by media task to deinit CCIO rx/tx I2S, free buffer... when call hangup
  \param[in]   null
  \returns     null
  \NOTE:       no block
*/
int32_t audioDrvDeInit();

/**
  \fn          audioStartRecordVoice
  \brief       this api called by media task to start ccio rx record voice
  \param[in]   UINT8 codecType, AMR-NB (0) or AMR-WB (1), refer to ACVOICECODECTYPE
  \returns     null
  \NOTE:       Async interface
*/
void audioStartRecordVoice(uint8_t codecType);

/**
  \fn          audioStopRecordVoice
  \brief       this api called by media task to stop ccio rx record voice
  \param[in]   UINT8 codecType, AMR-NB (0) or AMR-WB (1), refer to ACVOICECODECTYPE
  \returns     null
  \NOTE:       Sync interface, shall stop record done before return
*/
void audioStopRecordVoice(uint8_t codecType);

/**
  \fn          audioStartPlaySound
  \brief       this api called by media task to play sound according to call precedures
  \param[in]   UINT8 type, play type, refer to AudioPlayType_e. If type is PLAY_SPEECH_PCM_DATA, below params
  \            shall be valid, or else play local tone/ring.
  \            UINT8 *pSpeechBuf, the pointer to speech buffer
  \            UINT16 speechBufSize, speech buffer size
  \            UINT8 sampleRate, sample rate 8KHz(1)/16KHz(2), refer to AudioSampleRate_e
  \            UINT8 pcmBitWidth, bit width, now only 16
  \returns     null
  \NOTE:       1 Play tone (refer to TS 22.001 F.2.5 Comfort tones) for MO call (inital call) procedures;
  \            2 play alert ringing for MT call (incoming call) procedure;
  \            3 play speech buffer data (PCM) for early media for voice by DL RTP
  \            4 async interface
*/
void audioStartPlaySound(uint8_t type, uint8_t *pSpeechBuf, uint16_t speechBufSize, uint8_t sampleRate, uint8_t pcmBitWidth);

/**
  \fn          audioStopPlaySound
  \brief       this api called by media task to stop play sound according to call precedures
  \param[in]   UINT8 type, play type, refer to AudioPlayType_e
  \returns     null
  \NOTE:       sync interface, shall stop done before return
*/
void audioStopPlaySound(uint8_t type);


void audioGetToneDuration(uint8_t toneType, uint16_t *pPlayDuration, uint16_t *pStopDuration);

/**
  \fn          audioAllocateToneMem
  \brief       This api allocates tone mem, can be called by every 20ms. This mem will also by accessed by cp.
  \param[in]      uint8_t toneType,        tone type.
  \param[in]      uint8_t sampleRate,     samplerate. 1: 8k; 2: 16k.
  \param[out]   uint8_t** pToneData,     outPtr, will points to the global val "gToneCombine.ptr".
  \returns     int
*/
int32_t audioAllocateToneMem(uint8_t toneType, uint8_t sampleRate, uint8_t **pToneData);

/**
  \fn          audioFreeToneMem
  \brief      free the malloced mem, and clear the global val "gToneCombine".
  \returns     int
*/
int32_t audioFreeToneMem();



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION : called by app                         *
 *----------------------------------------------------------------------------*/
 
/**
  \fn          audioFastInit
  \brief      Just used for AT commands.
*/
void audioFastInit();

/**
  \fn          audioFastDeInit
  \brief      Just used for AT commands.
*/
void audioFastDeInit();


/**
  \fn          audioAppStartPlaySound
  \brief      This api is like audioStartPlaySound in functional, but can only be used by app player.
*/
int audioAppStartPlaySound(uint8_t type, uint8_t *pSpeechBuf, uint16_t speechBufSize, 
                            uint8_t sampleRate, uint8_t pcmBitWidth);

/**
  \fn          audioAppStopPlaySound
  \brief      This api is like audioStopPlaySound in functional, but can only be used by app player.
*/
int audioAppStopPlaySound(uint8_t type);

/**
  \fn          audioAppStartRecord
  \brief      This api is like audioStartRecordVoice in functional, but can only be used by app recoder.
*/
int audioAppStartRecord(uint8_t codecType);

/**
  \fn          audioAppStopRecord
  \brief      This api is like audioStopRecordVoice in functional, but can only be used by app recoder.
*/
int audioAppStopRecord(uint8_t codecType);


/**
  \fn          audioRegisterToneCb
  \brief       This api should register cb in play tone in order to trigger app task to start decode mp3 or others
  \             For example:
  \                  startDecode = 1; // trigger app to decode
  \                  status = osMessageQueuePut(gMsgqHandle, (const void*)&startDecode,  0, 0);
  \                  EC_ASSERT(status == osOK, status, 0, 0);
  \param[in]   i2sToneCbFunc cb, app ab 
  \returns     null
*/
void audioRegisterToneCb(i2sToneCbFunc cb);


/**
  \fn          audioRegisterHangCb
  \brief      Called by app, app send msg to ccio.
  \param[in]      uint8_t state,        app state.
  \returns     int
*/
void audioRegisterHangCb(i2sHangupCbFunc cb);


/**
  \fn          audioRegisterStopPlayCb
  \brief      Called by app, app send msg to ccio.
  \param[in]      uint8_t state,        app state.
  \returns     int
*/
void audioRegisterStopPlayCb(i2sStopPlayCbFunc cb);

/**
  \fn          audioUnRegisterStopPlayCb
  \brief      Called by app, unregister stop play cb.
  \returns     void
*/
void audioUnRegisterStopPlayCb();


/**
  \fn          audioRegisterStopRecordCb
  \brief      Called by app, app send msg to ccio.
  \param[in]      uint8_t state,        app state.
  \returns     int
*/
void audioRegisterStopRecordCb(i2sStopRecordCbFunc cb);


/**
  \fn          audioApp2Ccio
  \brief      Called by app, app send msg to ccio.
  \param[in]      uint8_t state,        app state.  0: done;   1: undone;
  \returns     int
*/
int32_t audioMsgApp2Ccio(uint8_t state);

/**
  \fn          audioSetCurrentMode
  \brief      Set current device mode.
  \param[in]      AudioParaCfgDeviceType_e  mode;
  \returns     void
*/
void audioSetCurrentMode(AudioParaCfgDeviceType_e mode);

/**
  \fn          audioGetCurrentMode
  \brief      Get current device mode.
  \param[in]      null;
  \returns     AudioParaCfgDeviceType_e
*/
AudioParaCfgDeviceType_e audioGetCurrentMode();

/**
  \fn          audioSetCurrentDirection
  \brief      Set current device direction.
  \param[in]      null;
  \returns     void
*/
void audioSetCurrentDirection(uint8_t direction);

/**
  \fn          audioGetCurrentDirection
  \brief      Get current device direction.
  \param[in]      NULL;
  \returns     direction
*/

uint8_t audioGetCurrentDirection();




#ifdef __cplusplus
}
#endif
#endif

