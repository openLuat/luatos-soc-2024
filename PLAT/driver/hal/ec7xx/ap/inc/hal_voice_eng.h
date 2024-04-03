#ifndef __HAL_VOICE_ENG_H__
#define __HAL_VOICE_ENG_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of AirM2M Ltd.
 File name:      - hal_voice_eng.h
 Description:    - voice codec/engine hal layer
 History:        - 2023/04/07, Originated by Xili
 ******************************************************************************
******************************************************************************/
#include "commontypedef.h"


/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/
/*
 * codec return code
*/
typedef enum
{
    HAL_VC_RC_SUCC      = 0,
    HAL_VC_RC_NO_MEM,
    HAL_VC_RC_BUSY,

    HAL_VC_RC_MAX       = 0xFF
}HalVoiceEngRc;

/*
*/
typedef enum
{
    HAL_VC_AMR_NB       = 0,
    HAL_VC_AMR_WB
}HalVoiceCodecType;


/*
 * AMR-NB frame type and mapped bitrate
*/
typedef enum
{
    HAL_AMR_NB_FT_0     = 0,
    HAL_AMR_NB_4750_BPS_95BIT   = HAL_AMR_NB_FT_0,
    HAL_AMR_NB_FT_1     = 1,
    HAL_AMR_NB_5150_BPS_103BIT  = HAL_AMR_NB_FT_1,
    HAL_AMR_NB_FT_2     = 2,
    HAL_AMR_NB_5900_BPS_118BIT  = HAL_AMR_NB_FT_2,
    HAL_AMR_NB_FT_3     = 3,
    HAL_AMR_NB_6700_BPS_134BIT  = HAL_AMR_NB_FT_3,
    HAL_AMR_NB_FT_4     = 4,
    HAL_AMR_NB_7400_BPS_148BIT  = HAL_AMR_NB_FT_4,
    HAL_AMR_NB_FT_5     = 5,
    HAL_AMR_NB_7950_BPS_159BIT  = HAL_AMR_NB_FT_5,
    HAL_AMR_NB_FT_6     = 6,
    HAL_AMR_NB_10200_BPS_204BIT = HAL_AMR_NB_FT_6,
    HAL_AMR_NB_FT_7     = 7,
    HAL_AMR_NB_12200_BPS_244BIT = HAL_AMR_NB_FT_7,
    HAL_AMR_NB_FT_8     = 8,
    HAL_AMR_NB_SID_39BIT        = HAL_AMR_NB_FT_8,

    /* 9 ~ 14 rsvd */
    HAL_AMR_NB_FT_15    = 15,
    HAL_AMR_NB_NO_DATA          = HAL_AMR_NB_FT_15,

    /* Folllowing are internal frame type */
    HAL_AMR_NB_FT_PLC           = 29,   /* only used for decode */
    HAL_AMR_NB_FT_CNG           = 30,   /* only used for decode */
    HAL_AMR_NB_FT_UNKNOWN       = 31,
    HAL_AMR_NB_BPS_UNKNOWN      = HAL_AMR_NB_FT_UNKNOWN
}HalAmrNbFrameType;

/*
 * AMR-WB frame type and mapped bitrate
*/
typedef enum
{
    HAL_AMR_WB_FT_0     = 0,
    HAL_AMR_WB_6600_BPS_132BIT  = HAL_AMR_WB_FT_0,
    HAL_AMR_WB_FT_1     = 1,
    HAL_AMR_WB_8850_BPS_177BIT  = HAL_AMR_WB_FT_1,
    HAL_AMR_WB_FT_2     = 2,
    HAL_AMR_WB_12650_BPS_253BIT = HAL_AMR_WB_FT_2,
    HAL_AMR_WB_FT_3     = 3,
    HAL_AMR_WB_14250_BPS_285BIT = HAL_AMR_WB_FT_3,
    HAL_AMR_WB_FT_4     = 4,
    HAL_AMR_WB_15850_BPS_317BIT = HAL_AMR_WB_FT_4,
    HAL_AMR_WB_FT_5     = 5,
    HAL_AMR_WB_18250_BPS_365BIT = HAL_AMR_WB_FT_5,
    HAL_AMR_WB_FT_6     = 6,
    HAL_AMR_WB_19850_BPS_397BIT = HAL_AMR_WB_FT_6,
    HAL_AMR_WB_FT_7     = 7,
    HAL_AMR_WB_23050_BPS_461BIT = HAL_AMR_WB_FT_7,
    HAL_AMR_WB_FT_8     = 8,
    HAL_AMR_WB_23850_BPS_477BIT = HAL_AMR_WB_FT_8,
    HAL_AMR_WB_FT_9     = 9,
    HAL_AMR_WB_SID_40BIT        = HAL_AMR_WB_FT_9,

    /* 10 ~ 13 rsvd */
    HAL_AMR_WB_FT_14    = 14,
    HAL_AMR_WB_SPEECH_LOST      = HAL_AMR_WB_FT_14,
    HAL_AMR_WB_FT_15    = 15,
    HAL_AMR_WB_NO_DATA          = HAL_AMR_WB_FT_15,

    /* Folllowing are internal frame type */
    HAL_AMR_WB_FT_PLC           = 29,   /* only used for decode */
    HAL_AMR_WB_FT_CNG           = 30,   /* only used for decode */
    HAL_AMR_WB_FT_UNKNOWN       = 31,
    HAL_AMR_WB_BPS_UNKNOWN      = HAL_AMR_WB_FT_UNKNOWN
}HalAmrWbFrameType;


/*
*/
typedef enum
{
    HAL_VOICE_ENG_RET_INVALID_MSGID = 0,

    HAL_VOICE_ENG_START_CNF         = 1,        /* HalVoiceEngStartCnf */

    HAL_VOICE_ENG_STOP_CNF          = 3,        /* HalVoiceEngStopCnf */

    HAL_VOICE_CODEC_CONFIG_CNF      = 5,        /* HalVoiceCodecConfigCnf */

    HAL_VOICE_ENCODE_CNF            = 7,        /* HalVoiceEncodeCnf */

    HAL_VOICE_DECODE_CNF            = 9         /* HalVoiceDecodeCnf */
}HalVoiceEngRetMsgId;

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/


/*
 *
*/
typedef UINT32  HalVoiceEngStartReq;

/*
 * HAL_VOICE_ENG_START_CNF
*/
typedef struct
{
    UINT8       rc;                 /* HalVoiceEngRc */
    UINT8       rsvd0;
    UINT16      suptCodecBitmap;    //bit 0: AMR-NB, bit 1: AMR-WB
}HalVoiceEngStartCnf;


/*
 * stop maybe cost 20ms
*/
typedef UINT32  HalVoiceEngStopReq;
typedef UINT32  HalVoiceEngStopCnf;


/*
 * config maybe cost 20ms
*/
typedef struct
{
    UINT8       codecType;      /* ACVOICECODECTYPE */
    UINT8       encBitRate;     /* ACAMRNBFRAMETYPE / ACAMRWBFRAMETYPE */
    UINT8       encOutBitOffset;  //0-7, encode AMR frame output bit offset
    UINT8       rsvd;
}HalVoiceCodecConfigReq;

typedef UINT32  HalVoiceCodecConfigCnf;

/*
 *
*/
typedef struct
{
    UINT8       codecType;      /* HalVoiceCodecType, AMR task double check if not same as config, could assert, or config fail
                                */
    UINT8       bitRate;        /* HalAmrNbFrameType / HalAmrWbFrameType */
    UINT16      sn;             /* serial number, act as a index/order of input encode request
                                 * Note, optional optimization later:
                                 *  if CP AMR codec detect more AMR encode reqest pending (etc, > 3 frame),
                                 *   could abort the oldest SN request (but need to confirm with a error RC: HAL_VC_RC_BUSY)
                                */
    UINT8       pcmBitWidth;    /* reserved for FFU, now set to 16, and AMR cound ingore it */
    UINT8       amrBufSize;     /* output AMR buffer size, in byte */
    UINT8       encOutBitOffset;/* AMR encode output bit offset (from MSB to LSB), range: [0:7]
                                */
    UINT8       rsvd0;

    UINT16      pcmLen;         /* input PCM data len, in byte */
    UINT16      rsvd1;

    UINT8       *pPcmData;      /* input PCM data buffer */

    UINT8       *pAmrData;      /* output AMR data buffer */
}HalVoiceEncodeReq;    //20 bytes

/*
 * HAL_VOICE_ENCODE_CNF
*/
typedef struct
{
    UINT8       rc;             /* HalVoiceEngRc */
    UINT8       codecType;      /* HalVoiceCodecType */
    UINT16      sn;             /* serial number */

    UINT8       frameType;      /* HalAmrNbFrameType / HalAmrWbFrameType */
    UINT8       outBitOffset;   /* AMR encode output bit offset (from MSB to LSB) */
    UINT16      amrBitLen;      /* AMR output bitlen, should be same as indicated in: frameType */

    UINT8       *pAmrData;      /* AMR encode output, format£º
                                 * +---------------|----------------------------------------------------------------------|-------+
                                 * |encOutBitOffset| speech bit (Class A ClassB Class C), 3GPP-26201/26101: d(0)...d(k-1) |padding|
                                 * +---------------|----------------------------------------------------------------------|-------+
                                */
    UINT8       *pPcmData;      /* input PCM buffer */

    UINT8       *pPostVemPcmData;/* output PCM data buffer for app PCM+VEM record path, AP need to free it*/
    void        *pExtra0;
    void        *pExtra1;
}HalVoiceEncodeCnf;     //16 bytes

/*
 *
*/
typedef struct
{
    UINT8       codecType;      /* HalVoiceCodecType, AMR task double check if not same as config, could assert, or config fail
                                */
    UINT8       inBitOffset;    /* 0-7, input AMR frame start bit offset (MSB -> LSB) */
    UINT16      sn;

    UINT16      amrBitLen;      /* input AMR frame bitlen, and this also indicate the frame type,
                                 * if PLC/CNG, set to 0
                                */
    UINT16      pcmBufLen;      /* output PCM buffer length
                                */

    UINT8       pcmBitWidth;    /* reserved for FFU, output PCM bitwidth, now set to 16, and AMR cound ingore it */
    UINT8       rsvd0;
    UINT16      rsvd1;

    UINT8       *pAmrData;      /* input AMR data, if PLC/CNG frame, set to PNULL */
    UINT8       *pPcmData;      /* output, PCM output buffer*/
}HalVoiceDecodeReq;    //20 bytes

/*
 * HAL_VOICE_DECODE_CNF
*/
typedef struct
{
    UINT8       rc;             /* HalVoiceEngRc */
    UINT8       codecType;      /* HalVoiceCodecType */
    UINT16      sn;

    UINT8       frameType;      /* HalAmrNbFrameType / HalAmrWbFrameType, notify output maybe: PLC or CNG */
    UINT8       rsvd0;
    UINT16      pcmDataLen;     /* output PCM data length in bytes */

    UINT8       *pPcmData;      /* output PCM data address */
    UINT8       *pAmrData;
}HalVoiceDecodeCnf;    //16 bytes

/*
* Voice call record notify params
*/
typedef struct HalVcRecordNotifyParamTag
{
    BOOL        bCallEnd; /*indicated whether the call is end, if bCallEnd == TRUE, discard this frame,
                            call halSetVoiceCallRecordMemory(null) to clear memory buffer
                           */
    UINT8       resved;
    UINT8       ulAmrLen; /* UL AMR frame length, >0 means have valid data in pUlAmrBuf */
    UINT8       dlAmrLen; /* DL AMR frame length, >0 means have valid data in pDlAmrBuf */
    UINT16      dlPcmLen; /* DL PCM frame length, >0 means have valid data in pDlPcmBuf */
    UINT16      ulVemPcmLen; /* UL PCM with VEM frame length, >0 means have valid data in pUlVemPcmBuf */
    UINT8       *pUlVemPcmBuf; /* UL PCM with VEM buffer, allocated by ccio_audio, free by app (record) task */

}
HalVcRecordNotifyParam;

/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/

/*
 * Voice engine start request, asyn API, result return in: HAL_VOICE_ENG_START_CNF
*/
void halVoiceEngStartReq(void);

/*
 * Voice engine stop request, asyn API, result return in: HAL_VOICE_ENG_STOP_CNF
*/
void halVoiceEngStopReq(void);

/*
 * Voice codec config, asyn API, result return in: HAL_VOICE_CODEC_CONFIG_CNF
*/
void halVoiceCodecConfigReq(HalVoiceCodecConfigReq *pCodecCfg);

/*
 * Voice encode request, asyn API, result return in: HAL_VOICE_ENCODE_CNF
*/
void halVoiceEncodeReq(HalVoiceEncodeReq *pEncReq);

/*
 * Voice decode request, asyn API, result return in: HAL_VOICE_DECODE_CNF
*/
void halVoiceDecodeReq(HalVoiceDecodeReq *pDecReq);

/*
 * voice engine return callback, set by caller/APP
*/
typedef void (*HalVoiceEngRetCallback)(UINT32 retMsgId, void *pRetMsg);
void halSetVoiceEngRetCallback(HalVoiceEngRetCallback voiceEngRetCallback);

BOOL halIsAppSetVoiceEngRetCallback(void);
BOOL halIsImsSetVoiceEngRetCallback(void);

/*
* set/allocate voice call record memory for one frame, called by APP (voice record) task
*/
void halSetVoiceCallRecordMemory(UINT8 *pUlAmrBuf, UINT8 *pDlAmrBuf,UINT8 *pDlPcmBuf);

/*
 * voice call record notify callback, trigger a signal to APP (voice record) task
 * shall call halSetVoiceCallRecordMemory to allocate memory for next frame beore trigger signal
 * if bCallEnd == TRUE, discard this frame, call halSetVoiceCallRecordMemory to clear memory buffer
 * Warning! This callback is called by media task, blocked process is not alllowed.
*/
typedef void (*HalVoiceCallRecordNotifyCallback)(HalVcRecordNotifyParam *pVcRecordNotifyParam);
void halSetVoiceCallRecordNotifyCallback(HalVoiceCallRecordNotifyCallback voiceCallRecordNotifyCallback);

/*
* Get voice call record memory, called by media task
*/
void halGetVoiceCallRecordMemory(UINT8 **pUlAmrBuf, UINT8 **pDlAmrBuf,UINT8 **pDlPcmBuf);
/*
* Get voice call record notify callback, called by media task
*/
HalVoiceCallRecordNotifyCallback halGetVoiceCallRecordNotifyCallback(void);


void halamrRecordIpc(uint8_t codecType, uint8_t* recordBuf, uint8_t* amrBuf, uint8_t* ulpduAddr, uint8_t* postVemPcmData);

#endif

