/******************************************************************************

*(C) Copyright 2018 AirM2M Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: psdial_sms.h
*
*  Description: SMS OPEN CPU implementation
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _PSDIAL_SMS_H
#define _PSDIAL_SMS_H

#include "commontypedef.h"
#include "ps_sms_if.h"

#if !defined FEATURE_AT_ENABLE && defined FEATURE_PS_SMS_PSDIAL_ENABLE

/******************************************************************************
 *****************************************************************************
 * SMS COMMON MARCO
 *****************************************************************************
******************************************************************************/

#define PSDIAL_SMS_RESP_1024_STR_LEN        1024        //must using heap for this size
#define PSDIAL_SMS_RESP_512_STR_LEN         512         //must using heap for this size
#define PSDIAL_SMS_RESP_256_STR_LEN         256
#define PSDIAL_SMS_RESP_128_STR_LEN         128
#define PSDIAL_SMS_RESP_64_STR_LEN          64
#define PSDIAL_SMS_RESP_48_STR_LEN          48
#define PSDIAL_SMS_RESP_32_STR_LEN          32
#define PSDIAL_SMS_IND_RESP_1024_STR_LEN    1024        //must using heap for this size
#define PSDIAL_SMS_IND_RESP_512_STR_LEN     512         //must using heap for this size
#define PSDIAL_SMS_IND_RESP_256_STR_LEN     256
#define PSDIAL_SMS_IND_RESP_128_STR_LEN     128
#define PSDIAL_SMS_IND_RESP_64_STR_LEN      64
#define PSDIAL_SMS_IND_RESP_48_STR_LEN      48
#define PSDIAL_SMS_IND_RESP_32_STR_LEN      32

#define PSDIAL_SMS_MT_SMS_TYPE_MASK         0x03       /* SMS MT Message-Type Indicator mask */
#define PSDIAL_SMS_M0_SMS_TYPE_MASK         0x03       /* SMS MO Message-Type Indicator mask */

#define PSDIAL_SMS_MT_DELIVER_SMS           0
#define PSDIAL_SMS_MT_SUBMIT_REPORT_SMS     1
#define PSDIAL_SMS_MT_STATUS_REPORT_SMS     2

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

/******************************************************************************
 * CmiCnfFuncMapList
 * "CamCmiCnf" handler table
******************************************************************************/
typedef CmsRetId (*CmiCnfHandler)(UINT16 reqHandle, UINT16 rc, void *paras);
typedef struct CmiCnfFuncMapList_Tag
{
    UINT16          primId;
    CmiCnfHandler   cmiCnfHdr;
}CmiCnfFuncMapList;

/******************************************************************************
 * CmiIndFuncMapList
 * "CamCmiInd" handler table
******************************************************************************/
typedef void (*CmiIndHandler)(void *paras);
typedef struct CmiIndFuncMapList_Tag
{
    UINT16          primId;
    CmiIndHandler   cmiIndHdr;
}CmiIndFuncMapList;

/******************************************************************************
 * ApplCnfFuncMapList
 * "applCmsCnf" handler table
******************************************************************************/
typedef CmsRetId (*ApplCnfHandler)(UINT16 reqHandle, UINT16 rc, void *paras);
typedef struct CmsCnfFuncMapList_Tag
{
    UINT16          primId;
    ApplCnfHandler  applCnfHdr;
}ApplCnfFuncMapList;

/******************************************************************************
 * ApplIndFuncMapList
 * "applCmsInd" handler table
******************************************************************************/
typedef CmsRetId (*ApplIndHandler)(UINT16 indHandle, void *paras);
typedef struct CmsIndFuncMapList_Tag
{
    UINT16          primId;
    ApplIndHandler  applIndHdr;
}ApplIndFuncMapList;



/******************************************************************************
 * ImiCnfFuncMapList
 * "ImiCnfHandler" handler table  to process ims Imi cnf/indication
******************************************************************************/
typedef CmsRetId (*ImiCnfHandler)(UINT16 reqHandle, UINT16 rc, void *paras);
typedef struct ImiCnfFuncMapList_Tag
{
    UINT16          primId;
    ImiCnfHandler   ImiCnfHdr;
}ImiCnfFuncMapList;


typedef struct PsDialSmsCMSSinfo_TAG
{
    UINT8                 sendIndex;
    UINT8                 mem2Type;
    PsilSmsStoreItemInfo  *pReadSmsInfo;
    BOOL                  isSmsSendPending;
    CmiSmsAddressInfo     *pDestAddrInfo;
}PsDialSmsCMSSinfo;

typedef struct PsDialSmsCPMSinfo_TAG
{
    BOOL                    bSuspendCPMS;       /* +CPMS? is suspended due to checking SMSFULL */
    BOOL                    bPendingCPMS;       /* +CPMS? is pending */
    UINT16                  pendingCPMSHdlr;
}PsDialSmsCPMSinfo;


/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/
CmiSmsAccMemRet smsStoreNewMsgToStorage(UINT32 atHandle, CmiSmsNewMsgInd *pCmiMsgInd, UINT8 saveMode, UINT8* pMemIndex);

CmsRetId psDialSmsProcCmiCnf(const SignalBuf *cnfSignalPtr);
void psDialSmsProcCmiInd(UINT16 primId, void *paras);

#endif //#if !defined FEATURE_AT_ENABLE && defined FEATURE_PS_SMS_PSDIAL_ENABLE
#endif


