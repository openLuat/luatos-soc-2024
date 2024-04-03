/******************************************************************************
 ******************************************************************************
 Copyright:      - 2017- Copyrights of AirM2M Ltd.
 File name:      - hal_voice_eng_mem.c
 Description:    - voice codec/engine hal memory management
 History:        - 2023/07/17
 ******************************************************************************
******************************************************************************/
#include "hal_voice_eng_mem.h"
#include "osasys.h"

typedef struct
{
    UINT32      bPlayPcmBufInit : 1;
	UINT32      bAppPcmBufInit 	: 1;
    UINT32      bRtpAmrPoolInit : 1;
    UINT32                      :21;
    UINT32      playPcmOwner    : 8;

    /* Allo from header, and free into tailer, to avoild one memory(header) used too much */
    HalVMemPool     *pRtpAmrFreeHead;
    HalVMemPool     *pRtpAmrFreeTail;
}HalVMemMgr;    //12 bytes

/******************************************************************************
 ******************************************************************************
 * GLOBAL VARIABLES
 ******************************************************************************
******************************************************************************/
#define __DEFINE_GLOBAL_VARIABLES__ //just for easy to find this position in SS
#if FEATURE_SUPPORT_APP_PCM_MEM_POOL
/**
 * 8 bytes magic word added
 * Total: 1928 bytes
*/
#define HAL_APP_PCM_BUFF_TOTAL_SIZE    (8 + (HAL_PCM_FRAME_SIZE*HAL_APP_PCM_FRAME_FIFO_SIZE))
__ALIGNED(4) PS_FM_VOICE_NOINIT UINT8  halAppPcmBuf[HAL_APP_PCM_BUFF_TOTAL_SIZE];
#endif

/**
 * 8 bytes magic word added
 * Total: 1928 bytes
*/
#define HAL_PLAY_PCM_BUFF_TOTAL_SIZE    (8 + (HAL_PCM_FRAME_SIZE*HAL_DL_PLAY_PCM_FRAME_FIFO_SIZE))
__ALIGNED(4) PS_FM_VOICE_NOINIT UINT8  halPlayPcmBuf[HAL_PLAY_PCM_BUFF_TOTAL_SIZE];


/**
 * (76+4)*4 = 320 bytes
*/
#define HAL_RTP_AMR_FRAME_BUFF_TOTAL_SIZE    ((HAL_VE_MEM_POOL_HDR_SIZE+HAL_RTP_ONE_AMR_FRAME_MAX_SIZE)*HAL_UL_RTP_AMR_FRAME_FIFO_SIZE)
__ALIGNED(4) PS_FM_VOICE_NOINIT UINT8  halRtpAmrFrameBuf[HAL_RTP_AMR_FRAME_BUFF_TOTAL_SIZE];

/**
 * HalVMemMgr,
*/
HalVMemMgr      gHalVMemMgr = {0};


/******************************************************************************
 ******************************************************************************
 * INERNAL/STATIC FUNCTION
 ******************************************************************************
******************************************************************************/
#define __DEFINE_STATIC_FUNCTION__ //just for easy to find this position in SS
#if FEATURE_SUPPORT_APP_PCM_MEM_POOL
/**
  \fn           void halVEInitAppPcmBuf(void)
  \brief        Init App PCM FIFO buffer
  \param[in]    void
  \returns      void
*/
static void halVEInitAppPcmBuf(void)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      *pMagic     = PNULL;

    if (pVeMemMgr->bAppPcmBufInit)
        return;

    pMagic  = (UINT32 *)halAppPcmBuf;
    *pMagic = HAL_VE_UINT32_MAGIC_WORD;

    pMagic  = (UINT32 *)((UINT8 *)halAppPcmBuf + 4 + (HAL_PCM_FRAME_SIZE*HAL_APP_PCM_FRAME_FIFO_SIZE));
    *pMagic = HAL_VE_UINT32_MAGIC_WORD;

    pVeMemMgr->bAppPcmBufInit  = TRUE;
    pVeMemMgr->playPcmOwner     = HAL_VE_MEM_APP_CALLER;

    return;
}
#endif
/**
  \fn           void halVEInitPlayPcmBuf(void)
  \brief        Init Tx/Play PCM FIFO buffer
  \param[in]    void
  \returns      void
*/
static void halVEInitPlayPcmBuf(void)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      *pMagic     = PNULL;

    if (pVeMemMgr->bPlayPcmBufInit)
        return;

    pMagic  = (UINT32 *)halPlayPcmBuf;
    *pMagic = HAL_VE_UINT32_MAGIC_WORD;

    pMagic  = (UINT32 *)((UINT8 *)halPlayPcmBuf + 4 + (HAL_PCM_FRAME_SIZE*HAL_DL_PLAY_PCM_FRAME_FIFO_SIZE));
    *pMagic = HAL_VE_UINT32_MAGIC_WORD;

    pVeMemMgr->bPlayPcmBufInit  = TRUE;
    pVeMemMgr->playPcmOwner     = HAL_VE_MEM_FREE_CALLER;

    return;
}

/**
  \fn           void halVEInitPlayPcmBuf(void)
  \brief        Init Tx/Play PCM FIFO buffer
  \param[in]    void
  \returns      void
*/
static void halVEInitRtpAmrPool(void)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    HalVMemPool *pCurMem = PNULL, *pPreMem = PNULL;
    UINT32      etyIdx = 0;

    if (pVeMemMgr->bRtpAmrPoolInit)
        return;

    for (etyIdx = 0; etyIdx < HAL_UL_RTP_AMR_FRAME_FIFO_SIZE; etyIdx++)
    {
        pCurMem    = (HalVMemPool *)(halRtpAmrFrameBuf + (HAL_VE_MEM_POOL_HDR_SIZE+HAL_RTP_ONE_AMR_FRAME_MAX_SIZE)*etyIdx);
        pCurMem->poolHdr.magicWord  = HAL_VE_UINT16_MAGIC_WORD;
        pCurMem->poolHdr.callerId   = HAL_VE_MEM_FREE_CALLER;
        pCurMem->pNext  = PNULL;

        if (pPreMem == PNULL)
        {
            pPreMem = pCurMem;
            pVeMemMgr->pRtpAmrFreeHead  = pCurMem;
        }
        else
        {
            pPreMem->pNext = pCurMem;
            pPreMem = pCurMem;
        }
    }

    pVeMemMgr->pRtpAmrFreeTail  = pCurMem;
    pVeMemMgr->bRtpAmrPoolInit  = TRUE;

    return;
}


/******************************************************************************
 ******************************************************************************
 * EXTERNAL FUNCTION
 ******************************************************************************
******************************************************************************/
#define __DEFINE_GLOBAL_FUNCTION__ //just for easy to find this position in SS
#if FEATURE_SUPPORT_APP_PCM_MEM_POOL
/**
  \fn           void halVEAllocAppPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc PCM App buffer, this buffer user App is writable
  \param[in]    pcmType         PCM type
  \param[in]    caller          who alloc this PCM buffer
  \param[out]   pRetBuf         return PCM start address
  \param[out]   pRetSize        alloc PCM buffer size
  \returns      void
*/
void halVEAllocAppPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      mask        = 0;
    //void        *pPcmBuf    = PNULL;

    if (pRetBuf == PNULL || pRetSize == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocAppPcmBuf_e_1, P_ERROR,
                      "HAL, alloc app PCM buffer, invalid input: 0x%lx/0x%lx, fail", pRetBuf, pRetSize);
        return;
    }

    *pRetBuf    = PNULL;
    *pRetSize   = 0;

    if (pcmType != HAL_VE_8KHZ_16B_PCM || pcmType != HAL_VE_16KHZ_16B_PCM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocAppPcmBuf_w_1, P_WARNING,
                      "HAL, alloc app PCM buffer, PCM type: %d, seems not support", pcmType);
    }

    /* check if init */
    if (pVeMemMgr->bAppPcmBufInit == FALSE)
    {
        /* disable ISR */
        mask = SaveAndSetIRQMask();
        halVEInitAppPcmBuf();
        RestoreIRQMask(mask);
    }

    *pRetBuf    = (void *)((UINT8 *)halAppPcmBuf + 4);
    *pRetSize   = (HAL_PCM_FRAME_SIZE*HAL_APP_PCM_FRAME_FIFO_SIZE);

    if (pVeMemMgr->playPcmOwner != HAL_VE_MEM_FREE_CALLER)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocAppPcmBuf_w_2, P_WARNING,
                      "HAL, app PCM buffer already alloc to caller: %d, here need re-alloc to caller: %d",
                      pVeMemMgr->playPcmOwner, caller);
    }

    pVeMemMgr->playPcmOwner = caller;

    return;
}

/**
  \fn           void halVEFreeAppPcmBuf(void **pFreeBuf)
  \brief        Free PCM play buffer
  \param[in/out]pFreeBuf        PCM buffer, set to PNULL when return
  \returns      void
*/
void halVEFreeAppPcmBuf(void **pFreeBuf)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      *pMagic     = PNULL;

    if (pFreeBuf == PNULL || *pFreeBuf == PNULL)
        return;

    if ((UINT32)(*pFreeBuf) != (UINT32)((UINT8 *)halAppPcmBuf + 4))
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEFreeAppPcmBuf_1, P_ERROR,
                      "HAL, free app PCM buff: 0x%x, buff not right, should: 0x%x",
                      (*pFreeBuf), ((UINT8 *)halAppPcmBuf + 4));
        return;
    }

    if (pVeMemMgr->bAppPcmBufInit == FALSE)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEFreeAppPcmBuf_2, P_ERROR,
                      "HAL, free app PCM buff: 0x%x, owner: %d, but app PCM buff not init, abnormal",
                      (*pFreeBuf), pVeMemMgr->playPcmOwner);
        return;
    }

    /* check magic words */
    pMagic  = (UINT32 *)halAppPcmBuf;
    OsaCheck(*pMagic == HAL_VE_UINT32_MAGIC_WORD, *pMagic, HAL_VE_UINT32_MAGIC_WORD, 0x186);

    pMagic  = (UINT32 *)((UINT8 *)halAppPcmBuf + 4 + (HAL_PCM_FRAME_SIZE*HAL_APP_PCM_FRAME_FIFO_SIZE));

    OsaCheck(*pMagic == HAL_VE_UINT32_MAGIC_WORD, *pMagic, HAL_VE_UINT32_MAGIC_WORD, 0x188);

    pVeMemMgr->playPcmOwner = HAL_VE_MEM_FREE_CALLER;

    *pFreeBuf   = PNULL;
    return;
}
#endif

/**
  \fn           void halVEAllocPlayPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc PCM play buffer, this buffer voice engine is writable
  \param[in]    pcmType         PCM type
  \param[in]    caller          who alloc this PCM buffer
  \param[out]   pRetBuf         return PCM start address
  \param[out]   pRetSize        alloc PCM buffer size
  \returns      void
*/
void halVEAllocPlayPcmBuf(UINT8 pcmType, UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      mask        = 0;
    //void        *pPcmBuf    = PNULL;

    if (pRetBuf == PNULL || pRetSize == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocPlayPcmBuf_e_1, P_ERROR,
                      "HAL, alloc play PCM buffer, invalid input: 0x%lx/0x%lx, fail", pRetBuf, pRetSize);
        return;
    }

    *pRetBuf    = PNULL;
    *pRetSize   = 0;

    if (pcmType != HAL_VE_8KHZ_16B_PCM && pcmType != HAL_VE_16KHZ_16B_PCM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocPlayPcmBuf_w_1, P_WARNING,
                      "HAL, alloc play PCM buffer, PCM type: %d, seems not support", pcmType);
    }

    /* check if init */
    if (pVeMemMgr->bPlayPcmBufInit == FALSE)
    {
        /* disable ISR */
        mask = SaveAndSetIRQMask();
        halVEInitPlayPcmBuf();
        RestoreIRQMask(mask);
    }

    *pRetBuf    = (void *)((UINT8 *)halPlayPcmBuf + 4);
    *pRetSize   = (HAL_PCM_FRAME_SIZE*HAL_DL_PLAY_PCM_FRAME_FIFO_SIZE);

    if (pVeMemMgr->playPcmOwner != HAL_VE_MEM_FREE_CALLER)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocPlayPcmBuf_w_2, P_WARNING,
                      "HAL, play PCM buffer already alloc to caller: %d, here need re-alloc to caller: %d",
                      pVeMemMgr->playPcmOwner, caller);
    }

    pVeMemMgr->playPcmOwner = caller;

    return;
}

/**
  \fn           void halVEFreePlayPcmBuf(void **pFreeBuf)
  \brief        Free PCM play buffer
  \param[in/out]pFreeBuf        PCM buffer, set to PNULL when return
  \returns      void
*/
void halVEFreePlayPcmBuf(void **pFreeBuf)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      *pMagic     = PNULL;

    if (pFreeBuf == PNULL || *pFreeBuf == PNULL)
        return;

    if ((UINT32)(*pFreeBuf) != (UINT32)((UINT8 *)halPlayPcmBuf + 4))
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEFreePlayPcmBuf_1, P_ERROR,
                      "HAL, free plat PCM buff: 0x%x, buff not right, should: 0x%x",
                      (*pFreeBuf), ((UINT8 *)halPlayPcmBuf + 4));
        return;
    }

    if (pVeMemMgr->bPlayPcmBufInit == FALSE)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEFreePlayPcmBuf_2, P_ERROR,
                      "HAL, free plat PCM buff: 0x%x, owner: %d, but play PCM buff not init, abnormal",
                      (*pFreeBuf), pVeMemMgr->playPcmOwner);
        return;
    }

    /* check magic words */
    pMagic  = (UINT32 *)halPlayPcmBuf;
    OsaCheck(*pMagic == HAL_VE_UINT32_MAGIC_WORD, *pMagic, HAL_VE_UINT32_MAGIC_WORD, 0x186);

    pMagic  = (UINT32 *)((UINT8 *)halPlayPcmBuf + 4 + (HAL_PCM_FRAME_SIZE*HAL_DL_PLAY_PCM_FRAME_FIFO_SIZE));

    OsaCheck(*pMagic == HAL_VE_UINT32_MAGIC_WORD, *pMagic, HAL_VE_UINT32_MAGIC_WORD, 0x188);

    pVeMemMgr->playPcmOwner = HAL_VE_MEM_FREE_CALLER;

    *pFreeBuf   = PNULL;
    return;
}

/**
  \fn           void halVEAllocEncodeAmrFrameBuf(void **pRetBuf, UINT16 *pRetSize)
  \brief        Alloc AMR frame buffer, which could used by voice engine AMR encoder
  \param[in]    caller          who alloc this AMR frame buf
  \param[out]   pRetBuf         return AMR frame start address
  \param[out]   pRetSize        now fix to: 76
  \returns      void
*/
void halVEAllocAmrEnFrameBuf(UINT8 caller, void **pRetBuf, UINT16 *pRetSize)
{
    HalVMemMgr  *pVeMemMgr  = &gHalVMemMgr;
    UINT32      mask        = 0;
    HalVMemPool *pRetPool   = PNULL;

    if (pRetBuf == PNULL || pRetSize == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocEncodeAmrFrameBuf_1, P_ERROR,
                      "HAL, alloc AMR frame buf, invalid input: 0x%lx/0x%lx, fail", pRetBuf, pRetSize);
        return;
    }

    *pRetBuf    = PNULL;
    *pRetSize   = 0;

    if (pVeMemMgr->bRtpAmrPoolInit == FALSE)
    {
        /* disable ISR */
        mask = SaveAndSetIRQMask();
        halVEInitRtpAmrPool();
        RestoreIRQMask(mask);
    }

    /* disable ISR */
    mask = SaveAndSetIRQMask();
    if (pVeMemMgr->pRtpAmrFreeHead != PNULL)
    {
        pRetPool = pVeMemMgr->pRtpAmrFreeHead;
        pVeMemMgr->pRtpAmrFreeHead  = pVeMemMgr->pRtpAmrFreeHead->pNext;

        if (pVeMemMgr->pRtpAmrFreeHead == PNULL)
        {
            pVeMemMgr->pRtpAmrFreeTail  = PNULL;
        }
    }
    /* enable ISR */
    RestoreIRQMask(mask);

    if (pRetPool == PNULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_HAL, halVEAllocEncodeAmrFrameBuf_w_1, P_WARNING,
                      "HAL, alloc AMR frame buf, no left, fail, callerId: %d", caller);

        return;
    }

    OsaCheck(pRetPool->poolHdr.magicWord == HAL_VE_UINT16_MAGIC_WORD &&
             pRetPool->poolHdr.callerId == HAL_VE_MEM_FREE_CALLER,
             pRetPool->poolHdr.magicWord, pRetPool->poolHdr.callerId, 0x288);

    pRetPool->poolHdr.callerId  = caller;

    *pRetBuf    = (void *)&(pRetPool->pNext);
    *pRetSize   = HAL_RTP_ONE_AMR_FRAME_MAX_SIZE;

    /**
     * Set magic words
     * 14 bytes reserved for RTP here, can't write by VE, here set a magic word for checking
     * +--------+------------------------------+
     * |  14    |           62                 |
     * +------+-+------------------------------+
     * ^      ^ *pMagicUINT16
     * |*pMagicUINT32
    */
    {
        UINT32  *pMagicUINT32   = (UINT32 *)(*pRetBuf);
        UINT16  *pMagicUINT16   = (UINT16 *)((UINT8 *)(*pRetBuf) + HAL_RTP_HEADER_RSVD_SIZE - 2);
        *pMagicUINT32   =   HAL_VE_UINT32_MAGIC_WORD;
        *pMagicUINT16   =   HAL_VE_UINT16_MAGIC_WORD;
    }

    return;
}

/**
  \fn           void halVEFreeAmrEnFrameBuf(void **pFreeBuf)
  \brief        Free AMR frame buffer, called after AMR frame send
  \param[in]    pFreeBuf        AMR frame buff need to free
  \returns      void
*/
void halVEFreeAmrEnFrameBuf(void **pFreeBuf)
{
    HalVMemMgr      *pVeMemMgr  = &gHalVMemMgr;
    HalVMemPool     *pFreePool  = (HalVMemPool *)((UINT32)(*pFreeBuf) - HAL_VE_MEM_POOL_HDR_SIZE);
    UINT32          mask        = 0;

    if (pFreeBuf == PNULL || *pFreeBuf == PNULL)
        return;

    OsaCheck(pFreePool->poolHdr.magicWord == HAL_VE_UINT16_MAGIC_WORD &&
             pFreePool->poolHdr.callerId != HAL_VE_MEM_FREE_CALLER,
             pFreePool->poolHdr.magicWord, pFreePool->poolHdr.callerId, 0x314);

    pFreePool->poolHdr.callerId = HAL_VE_MEM_FREE_CALLER;
    pFreePool->pNext            = PNULL;

    /* disable ISR */
    mask = SaveAndSetIRQMask();

    /* Free, insert into the tailer */
    if (pVeMemMgr->pRtpAmrFreeTail == PNULL)
    {
        pVeMemMgr->pRtpAmrFreeHead  = pVeMemMgr->pRtpAmrFreeTail = pFreePool;
    }
    else
    {
        pVeMemMgr->pRtpAmrFreeTail->pNext   = pFreePool;
        pVeMemMgr->pRtpAmrFreeTail          = pFreePool;
    }

    /* enable ISR */
    RestoreIRQMask(mask);

    *pFreeBuf   = PNULL;

    return;
}


/**
  \fn           void halVECheckAmrEnFrameBuf(void **pFreeBuf)
  \brief        Check the AMR encodec buffer be overflow by VE
  \param[in]    pOrigBuf        AMR frame buff
  \returns      void
*/
void halVECheckAmrEnFrameBuf(const void *pOrigBuf)
{
    /**
     * Set magic words
     * 14 bytes reserved for RTP here, can't write by VE, here set a magic word for checking
     * +--------+------------------------------+
     * |  14    |           62                 |
     * +------+-+------------------------------+
     * ^      ^ *pMagicUINT16
     * |*pMagicUINT32
    */
    UINT32  *pMagicUINT32   = (UINT32 *)(pOrigBuf);
    UINT16  *pMagicUINT16   = (UINT16 *)((UINT8 *)(pOrigBuf) + HAL_RTP_HEADER_RSVD_SIZE - 2);


    OsaCheck(*pMagicUINT32 == HAL_VE_UINT32_MAGIC_WORD, *pMagicUINT32, pOrigBuf, HAL_VE_UINT32_MAGIC_WORD);
    OsaCheck(*pMagicUINT16 == HAL_VE_UINT16_MAGIC_WORD, *pMagicUINT16, pOrigBuf, HAL_VE_UINT16_MAGIC_WORD);

    return;
}


