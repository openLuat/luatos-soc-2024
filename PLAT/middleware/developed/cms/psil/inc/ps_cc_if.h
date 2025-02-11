/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    PS_cc_if.h
 * Description:  API interface implementation header file for IMS CC service
 * History:      Rev1.0
 *
 ****************************************************************************/
#ifndef __PS_CC_IF_H__
#define __PS_CC_IF_H__

#ifdef FEATURE_IMS_ENABLE

#include "cms_util.h"
#include "imicc.h"
#include "imimedia.h"
#include "imiss.h"

CmsRetId ccSetATD(UINT32 atHandle, UINT8 *dialNumStr, UINT16 dialNumStrLen, UINT8 clir);
CmsRetId ccSetATA(UINT32 atHandle);
CmsRetId ccSetHangupCall(UINT32 atHandle);
CmsRetId ccSetCSTA(UINT32 atHandle, UINT16 type);
CmsRetId ccGetCSTA(UINT32 atHandle);
CmsRetId ccSetCVHU(UINT32 atHandle, UINT8 mode);
CmsRetId ccGetCVHU(UINT32 atHandle);
CmsRetId ccSetCLCC(UINT32 atHandle);
CmsRetId ccSetCRC(UINT32 atHandle, UINT8 mode);
CmsRetId ccGetCRC(UINT32 atHandle);
CmsRetId ccSetVTS(UINT32 atHandle, UINT8 *dtmfStr, UINT16 dtmfStrLen, UINT16 duration);
CmsRetId ccSetVTD(UINT32 atHandle, UINT16 duration);
CmsRetId ccGetVTD(UINT32 atHandle);
CmsRetId ccSetCCWA(UINT32 atHandle, UINT8 mode, BOOL classPresent, UINT8 class);
CmsRetId ssSetCHLD(UINT32 atHandle, UINT8 n);
CmsRetId ccGetECEMGNUM(UINT32 atHandle);
CmsRetId ccSetECEMGNUM(UINT32 atHandle, UINT8 mode, UINT8 type, UINT8 category, UINT8 emgNumCount,
                            CHAR emgNum[IMI_CC_MAX_EMG_NUMS][IMI_CC_MAX_SINGLE_EMG_NUM_STR_LEN + 1]);


CmsRetId ssGetCCFC(UINT8     operType, UINT32 atHandle);
CmsRetId ssSetCCFC
          (UINT8    opType,
           UINT8    mode,
           UINT8    tarNumType,
           UINT8    tarNumLen,
           UINT8   *pTarNum,
           UINT8    noRepTime,
           UINT32   atHandle);

CmsRetId ssCLCK(UINT32   atHandle, UINT8 mode, UINT8 *pFac,UINT8 *pPwd,UINT8 class);


CmsRetId ccGetCLIP(UINT32 atHandle);
CmsRetId ccSetCLIP(UINT8 mode, UINT32 atHandle);
CmsRetId ccSetCLIR(UINT8 mode, UINT32 atHandle);
CmsRetId ccGetCLIR(UINT32 atHandle);
CmsRetId ccSetCOLP(UINT8 mode, UINT32 atHandle);
CmsRetId ccGetCOLP(UINT32 atHandle);
CmsRetId ccSetCOLR(UINT8 mode, UINT32 atHandle);
CmsRetId ccGetCOLR(UINT32 atHandle);
#endif
#endif
