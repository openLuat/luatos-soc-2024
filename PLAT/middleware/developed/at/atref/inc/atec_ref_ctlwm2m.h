/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

*******************************************************************************
*  Filename: atec_ctlwm2m1_5.h
*
*  Description: at entry for ctlwm2m header file
*
*  History:
*
*  Notes: China Telecom LWM2M version 1.5
*
******************************************************************************/

#ifndef _ATEC_REF_CTLWM2M_H
#define _ATEC_REF_CTLWM2M_H

#include "at_util.h"

#define MAX_RSP_LEN           (100)

CmsRetId  ctm2mNCDP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWSREGIND(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWULDATA(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWULDATAEX(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWULDATASTATUS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWFOTAIND(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQSETPSK(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQREGSWT(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNMGS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNSMI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNNMI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNMGR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNMSTATUS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQRESETDTLS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQDTLSSTAT(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQLWSERVERIP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNQMGR(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mNQMGS(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQSECSWT(const AtCmdInputContext *pAtCmdReq);
CmsRetId  ctm2mQCRITICALDATA(const AtCmdInputContext *pAtCmdReq);

#endif

