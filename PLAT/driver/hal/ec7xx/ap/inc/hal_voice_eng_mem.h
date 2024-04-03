#ifndef __HAL_VOICE_ENG_MEM_H__
#define __HAL_VOICE_ENG_MEM_H__
/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of AirM2M Ltd.
 File name:      - hal_voice_eng_mem.h
 Description:    - voice codec/engine hal memory management
 History:        - 2023/07/17
 ******************************************************************************
******************************************************************************/

/**
 * Now only IMS using voice (AMR codec) engine
*/

#include "commontypedef.h"


/**
 * Three type buffer need special alloc/free here
 * 1> decoding direction (DL data). DL/Rx/Play PCM sample buffer
 *   a) 8KHz sample rate(AMR-NB), 1 frame: 8*20*16 bits = 320 bytes
 *   b) 16KHz sample rate(AMR-NB), 1 frame: 16*20*16 bits = 640 bytes
 *  Proposal:
 *   a) As CP need write this buffer, we will pre-alloc/reserve it here
 *   b)               v Decode write
 *      +--------+--------+--------+
 *      +--------+--------+--------+
 *      ^ play read
 *      i>  CCIO Tx will play the first frame, 30ms later after first frame send to AMR decode
 *      ii> AMR decode & 3A enhance operation, should finished in 30ms
 *      iii>If not, audio will play old frame data
 *   c) total size: 640*3 = 1920 bytes
 *
 * 2> encoding direction (UL data), UL/Tx AMR encode buffer.
 *   a) AMR-NB, 1 frame max: 244 bits/8 + RTP AMR payload header 2 bytes + RTP header 12 bytes ~= 45 bytes
 *   b) AMR-WB, 1 frame max: 477 bits/8 + RTP AMR payload header 2 bytes + RTP header 12 bytes ~= 74 bytes
 *  Proposal:
 *   a) As CP need write this buffer, we will pre-alloc/reserve it here
 *   b) As UL AMR frame RTP pkg send directly without caching in UE size, here
 *      FIFO size 4 is enough, cost: 80 * 4 = 320 bytes
 *
 * 3> for app recording functions
 * a) 8KHz sample rate(AMR-NB), 1 frame: 8*20*16 bits = 320 bytes
 * b) 16KHz sample rate(AMR-NB), 1 frame: 16*20*16 bits = 640 bytes
 *
*/
/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/

/**
 * DL/PLAY
*/
#define HAL_8K_PCM_FRAME_SIZE       320     /* 8KHz sample * 20ms * 16bits =  320 bytes */
#define HAL_16K_PCM_FRAME_SIZE      640     /* 16KHz sample * 20ms * 16bits =  640 bytes */
#define HAL_PCM_FRAME_SIZE          640     /* using bigger AMR-WB */

/*
*/
#define HAL_DL_8K_PCM_FRAME_FIFO_SIZE   6
#define HAL_DL_16K_PCM_FRAME_FIFO_SIZE  3   /* 3 FIFO PCM play buffer */
#define HAL_DL_PLAY_PCM_FRAME_FIFO_SIZE 3

#if FEATURE_SUPPORT_APP_PCM_MEM_POOL
/* default 3 APP PCM play buffer, should not modify.if need to increase the pcm memory pool block number,
 * need to modify up_buf_start addr in mem_map_ec718p.h and MPU configuration, please contact EC to review
*/
#define HAL_APP_PCM_FRAME_FIFO_SIZE 	3	
#endif

/**
 * UL/Tx
*/
/**
 * a) AMR-NB, 1 frame max: 244 bits/8 + RTP AMR payload header 2 bytes + RTP header 12 bytes ~= 45 bytes
 * b) AMR-WB, 1 frame max: 477 bits/8 + RTP AMR payload header 2 bytes + RTP header 12 bytes ~= 74 bytes
*/
#define HAL_RTP_ONE_AMR_FRAME_MAX_SIZE  76
#define HAL_UL_RTP_AMR_FRAME_FIFO_SIZE  4

#define HAL_RTP_HEADER_RSVD_SIZE        14  /* RTP AMR payload header 2 bytes + RTP header 12 bytes */

/**
 * magic word
*/
#define HAL_VE_UINT32_MAGIC_WORD        0x4E6D8CAB
#define HAL_VE_UINT16_MAGIC_WORD        0x9D7B

/**
 * sizeof(HalVPoolHdr) = 4
*/
#define HAL_VE_MEM_POOL_HDR_SIZE        4


/*
 * Who call to allocate the memory from voice engine memory
*/
typedef enum HalVCallerId_Enum
{
    HAL_VE_MEM_INVALID_CALLER = 0,
    HAL_VE_MEM_FREE_CALLER,         /* this memeory is freed */
    HAL_VE_MEM_CCIO_TX_PLAY_CALLER, /* CCIO Tx/play task may alloc PCM buffer */
    HAL_VE_MEM_CCIO_RX_CALLER,      /* CCIO Rx task may alloc AMR frame buffer */
    HAL_VE_MEM_MEDIA_CALLER,        /* IMS media task */   
	HAL_VE_MEM_APP_CALLER,			/* APP Task alloc PCM Buffer*/
    /*
    */

    HAL_VE_MEM_CALLER_MAX_ID = 255
}HalVCallerId;

/*
 *
*/
typedef enum HalVPcmType_enum
{
    HAL_VE_8KHZ_16B_PCM,
    HAL_VE_16KHZ_16B_PCM
}HalVPcmType;

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

typedef struct HalVPoolHdr_Tag
{
    UINT16  magicWord;

    UINT8   callerId;
    UINT8   rsvd;
}HalVPoolHdr;       /* 4 bytes before alloc */

/******************************************************************************
 * HAL voice memory POOL
******************************************************************************/
typedef struct HalVMemPool_Tag
{
    HalVPoolHdr     poolHdr;
    struct HalVMemPool_Tag  *pNext;
}HalVMemPool;



/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/
/**
  \fn           void halVEAllocPlayPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc PCM play buffer, this buffer voice engine is writable
  \param[in]    pcmType         HalVPcmType, PCM type
  \param[in]    caller          HalVCallerId, who alloc this PCM buffer
  \param[out]   pRetBuf         return PCM start address
  \param[out]   pRetSize        alloc PCM buffer size
  \returns      void
  \Note: 1> HAL_VE_8KHZ_16B_PCM, return: 320 * 6 (6 AMR-NB frames) bytes
  \      1> HAL_VE_16KHZ_16B_PCM, return: 640 * 3 (3 AMR-NB frames) bytes
*/
void halVEAllocPlayPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize);


/**
  \fn           void halVEFreePlayPcmBuf(void **pFreeBuf)
  \brief        Free PCM play buffer
  \param[in/out]pFreeBuf        PCM buffer, set to PNULL when return
  \returns      void
*/
void halVEFreePlayPcmBuf(void **pFreeBuf);

#if FEATURE_SUPPORT_APP_PCM_MEM_POOL
/**
  \fn           void halVEAllocAppPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc PCM app buffer, this buffer voice engine is writable
  \param[in]    pcmType         HalVPcmType, PCM type
  \param[in]    caller          HalVCallerId, who alloc this PCM buffer
  \param[out]   pRetBuf         return PCM start address
  \param[out]   pRetSize        alloc PCM buffer size
  \returns      void
  \Note: 1> HAL_VE_8KHZ_16B_PCM, return: 320 * 6 (6 AMR-NB frames) bytes
  \      1> HAL_VE_16KHZ_16B_PCM, return: 640 * 3 (3 AMR-NB frames) bytes
*/
void halVEAllocAppPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize);


/**
  \fn           void halVEFreeAppPcmBuf(void **pFreeBuf)
  \brief        Free PCM play buffer
  \param[in/out]pFreeBuf        PCM buffer, set to PNULL when return
  \returns      void
*/
void halVEFreeAppPcmBuf(void **pFreeBuf);

/**
  \fn           void halVEAllocEncodeAmrFrameBuf(void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc AMR frame buffer, which could used by voice engine AMR encoder
  \param[in]    caller          who alloc this AMR frame buf
  \param[out]   pRetBuf         return AMR frame start address
  \param[out]   pRetSize        now fix to: 76
  \returns      void
  \Note: +--------+------------------------------+
  \      |  14    |           62                 |
  \      +--------+------------------------------+
  \      ^pRetBuf ^pAmrEn
  \ 1) Caller should offset/reserve 14 (HAL_RTP_HEADER_RSVD_SIZE) for RTP header
*/
#endif

void halVEAllocAmrEnFrameBuf(UINT8 caller, void **pRetBuf, UINT16 *pRetSize);

/**
  \fn           void halVEFreeAmrEnFrameBuf(void **pFreeBuf)
  \brief        Free AMR frame buffer, called after AMR frame send
  \param[in]    pFreeBuf        AMR frame buff need to free
  \returns      void
*/
void halVEFreeAmrEnFrameBuf(void **pFreeBuf);

/**
  \fn           void halVECheckAmrEnFrameBuf(void **pFreeBuf)
  \brief        Check the AMR encodec buffer be overflow by VE
  \param[in]    pOrigBuf        AMR frame buff
  \returns      void
*/
void halVECheckAmrEnFrameBuf(const void *pOrigBuf);

#endif


