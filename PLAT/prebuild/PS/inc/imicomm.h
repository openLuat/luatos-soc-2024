#ifndef __IMI_COMM_H__
#define __IMI_COMM_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imicomm.c
 Description:    - IMS Interface common header file
 History:        - 2023/04/10, Original created
******************************************************************************/
#ifdef WIN32
#include "imisys.h"
#else
#include "osasys.h"
#endif
#include "cmicomm.h"    /* here we suggest/better to unify the CME ERR code, for CMS/AT layer easy */

/******************************************************************************
 ******************************************************************************
 * ENUM/MARCO definition
 ******************************************************************************
******************************************************************************/

/*
 * Same defination in: cms_def.h
*/
#define IMI_BROADCAST_IND_HANDLER       0x0001

/*
 * used in IMA, this reserved for IMA SYNC API
 * a) Same defination in: cms_def.h: CAM_CMI_SYNC_REQ_START_HANDLER
 * b) Use the same value as in CAM task
*/
#define IMI_SYNC_REQ_NUM                16
#define IMI_SYNC_REQ_START_HANDLER      0x0030
#define IMI_SYNC_REQ_END_HANDLER        (IMI_SYNC_REQ_START_HANDLER + IMI_SYNC_REQ_NUM - 1)

/**
 * [0x0050~0x005F] for ECIMS
*/
#define IMS_MOD_HANDLER_NUM             16
#define IMS_MOD_START_HANDLER           0x0050
#define IMS_MOD_END_HANDLER             0x005F


/**
 * IMI CNF result code
*/
#if 0
typedef enum ImiRc_Tag
{
    IME_RC_SUCC = 0,
    IME_RC_FAIL,
    IME_UE_BUSY,
    IME_PARAM_ERROR,
    IME_SYS_ERROR,
    IME_MEM_ERROR,
    IME_OPERATION_NOT_ALLOW,
    IME_OPERATION_NOT_SUPPORT,


    IME_MAX_ERROR = 0xFF,
}ImiRc;
#endif
typedef MtErrorResultCode   _ImiRcCode;
typedef CmiRcCode           ImiRcCode;


/*IMA SUB module defined*/
typedef enum ImaSgIdEnum_Tag
{
    IMA_BASE    = 1,
    IMA_MOD     = 2,
    IMA_REG     = 3,
    IMA_CC      = 4,
    IMA_SMS     = 5,
    IMA_MED     = 6,
    IMA_SS      = 7,
    IMA_PRO     = 8,

    IMA_MAX     = 15
}ImaSgIdEnum;


/**
 * IMA IMI signal ID, IMS service provide to uplayer
*/
typedef enum _SIG_EPAT_IMAIMISIGID_enum
{
    /*
     * [0x2D00, 0x2D0F]
    */
    SIG_IMA_IMI_BASE = 0x2D00,
    SIG_IMA_IMI_REQ,        /* ImaImiReq */
    SIG_IMA_IMI_CNF,        /* ImaImiCnf */

    SIG_IMA_IMI_IND,        /* ImaImiInd */
    SIG_IMA_IMI_RSP,        /* not used now */

    SIG_IMA_IMI_SYNC_REQ,   /* ImaImiSyncReq */

    SIG_IMA_IMI_END = 0x2D0F,
}IMAIMISIGID;



#define IMA_GET_IMICNF_FROM_BODY(pImiBody) (ImaImiCnf *)(((UINT8 *)(pImiBody)) - (UINT32)(OFFSETOF(ImaImiCnf, body)))
#define IMA_GET_SIGNAL_FROM_BODY(pSigBody) (SignalBuf *)(((UINT8 *)(pSigBody)) - OFFSETOF(SignalBuf, sigBody))
#define IMA_GET_IMIIND_FROM_BODY(pImiBody) (ImaImiInd *)(((UINT8 *)(pImiBody)) - (UINT32)(OFFSETOF(ImaImiInd, body)))

//4 bits IMA ID | 12 bits PRIM ID
#define IMA_GET_IMA_ID(IMID)    (((IMID)>>12)&0x000f)
#define IMA_GET_PRIM_ID(IMID)   ((IMID)&0x0fff)
#define IMA_SET_IM_ID(IMAID, PRIMID)    ((UINT16)(((IMAID)<<12)|((PRIMID)&0x0fff)))



/******************************************************************************
 ******************************************************************************
 * STRUCT definition
 ******************************************************************************
******************************************************************************/
typedef UINT32      ImiEmptySig;



/******************************************************************************
 * ImaImiReq - SIG_IMA_IMI_REQ
 * IMI interface request, all other module (AT CMD server) use this signal to ICM task
 * Int16 reqId; 4 bits IMA ID | 12 bits PRIM ID
 * Int16 srcHandler; ID use to identify which module send this request
 * Int8 body[0]; request PRIM entity
******************************************************************************/
typedef struct ImaImiReq_Tag
{
    struct {
        UINT16  imId;           //4 bits IMA ID | 12 bits PRIM ID
        UINT16  srcHandler;
    }header;

    UINT8 body[];
}ImaImiReq;


/******************************************************************************
 * ImaImiCnf - SIG_IMA_IMI_CNF
******************************************************************************/
typedef struct ImaImiCnf_Tag
{
    struct {
        UINT16      imId;       //4 bits IMA ID | 12 bits PRIM ID
        UINT16      srcHandler;
        UINT16      rc;
        UINT16      reserved;
    }header;

    UINT8 body[];
}ImaImiCnf;

/******************************************************************************
 * ImaImiInd - SIG_IMA_IMI_IND
******************************************************************************/
typedef struct ImaImiInd_Tag
{
    struct {
        UINT16      imId;       //4 bits IMA ID | 12 bits PRIM ID
        UINT16      reqHandler;
    }header;

    UINT8 body[];
}ImaImiInd;



/******************************************************************************
 * ImaImiReq - SIG_IMA_IMI_SYNC_REQ
 * IMA interface request, all other module (AT CMD server) use this signal to
 *  access the IMS modem, and syn confirm
******************************************************************************/
typedef struct ImaImiSyncReqTag
{
    osSemaphoreId_t     sem;

    /* output info */
    UINT32              *pImiRc;            /* ImiResultCode */
    UINT16              cnfBufSize;
    UINT16              rsvd0;
    void                *pOutImiCnf;        /* IMI confirm struct, IMI CNF, copy to here */

    /* input info */
    ImaImiReq           imiReq;
}ImaImiSyncReq;



/******************************************************************************
 ******************************************************************************
 * FUNC definition
 ******************************************************************************
******************************************************************************/

/**
 * call by CMS/other APP, to send SIG_IMA_IMI_REQ to IMS module
*/
void ImsSendImiReqSig(UINT16 srcHdr, UINT8 imaId, UINT16 primId, UINT16 primSize, void *primBody);

/**
 * call by CMS/other APP, to get/set the request from/to IMS module
 * Note: this API is blocked the Caller task
*/
ImiRcCode ImsSendImiSynReq(UINT8 imaId, UINT16 primId, UINT16 primSize, const void *primBody, UINT16 outCnfSize, void *pOutCnfBody);


#endif

