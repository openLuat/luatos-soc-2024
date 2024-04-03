/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_onenet.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _ATEC_ONENET_H
#define _ATEC_ONENET_H

#include "at_util.h"

/* AT+MIPLCREATE */
#define ATC_MIPLCREATE_0_VAL_MIN                   0
#define ATC_MIPLCREATE_0_VAL_MAX                   2
#define ATC_MIPLCREATE_0_VAL_DEFAULT               0

/* AT+MIPLDELETE */
#define ATC_MIPLDELETE_0_REF_VAL_MIN                   0
#define ATC_MIPLDELETE_0_REF_VAL_MAX                   2
#define ATC_MIPLDELETE_0_REF_VAL_DEFAULT               0

/* AT+MIPLOPEN */
#define ATC_MIPLOPEN_0_REF_VAL_MIN                   0
#define ATC_MIPLOPEN_0_REF_VAL_MAX                   2
#define ATC_MIPLOPEN_0_REF_VAL_DEFAULT               0
#define ATC_MIPLOPEN_1_LIFETIME_VAL_MIN                   0
#define ATC_MIPLOPEN_1_LIFETIME_VAL_MAX                   2
#define ATC_MIPLOPEN_1_LIFETIME_VAL_DEFAULT               0
#define ATC_MIPLOPEN_2_TIMEOUT_VAL_MIN                   0
#define ATC_MIPLOPEN_2_TIMEOUT_VAL_MAX                   2
#define ATC_MIPLOPEN_2_TIMEOUT_VAL_DEFAULT               0

/* AT+MIPCLOSE */
#define ATC_MIPCLOSE_0_REF_VAL_MIN                   0
#define ATC_MIPCLOSE_0_REF_VAL_MAX                   2
#define ATC_MIPCLOSE_0_REF_VAL_DEFAULT               0

/* AT+MIPLADDOBJ */
#define ATC_MIPLADDOBJ_0_REF_VAL_MIN                   0
#define ATC_MIPLADDOBJ_0_REF_VAL_MAX                   2
#define ATC_MIPLADDOBJ_0_REF_VAL_DEFAULT               0
#define ATC_MIPLADDOBJ_1_OBJID_VAL_MIN                   0
#define ATC_MIPLADDOBJ_1_OBJID_VAL_MAX                   2
#define ATC_MIPLADDOBJ_1_OBJID_VAL_DEFAULT               0
#define ATC_MIPLADDOBJ_2_INSTCOUNT_VAL_MIN                   0
#define ATC_MIPLADDOBJ_2_INSTCOUNT_VAL_MAX                   2
#define ATC_MIPLADDOBJ_2_INSTCOUNT_VAL_DEFAULT               0
#define ATC_MIPLADDOBJ_3_INSTBITMAP_VAL_MIN                   0
#define ATC_MIPLADDOBJ_3_INSTBITMAP_VAL_MAX                   2
#define ATC_MIPLADDOBJ_3_INSTBITMAP_VAL_DEFAULT               0
#define ATC_MIPLADDOBJ_4_ATTR_VAL_MIN                   0
#define ATC_MIPLADDOBJ_4_ATTR_VAL_MAX                   2
#define ATC_MIPLADDOBJ_4_ATTR_VAL_DEFAULT               0
#define ATC_MIPLADDOBJ_5_ACT_VAL_MIN                   0
#define ATC_MIPLADDOBJ_5_ACT_VAL_MAX                   2
#define ATC_MIPLADDOBJ_5_ACT_VAL_DEFAULT               0

/* AT+MIPLDELOBJ */
#define ATC_MIPLDELOBJ_0_REF_VAL_MIN                   0
#define ATC_MIPLDELOBJ_0_REF_VAL_MAX                   2
#define ATC_MIPLDELOBJ_0_REF_VAL_DEFAULT               0

/* AT+MIPLNOTIFY */
#define ATC_MIPLNOTIFY_0_REF_VAL_MIN                   0
#define ATC_MIPLNOTIFY_0_REF_VAL_MAX                   2
#define ATC_MIPLNOTIFY_0_REF_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_1_MSGID_VAL_MIN                   0
#define ATC_MIPLNOTIFY_1_MSGID_VAL_MAX                   2
#define ATC_MIPLNOTIFY_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_2_OBJID_VAL_MIN                   0
#define ATC_MIPLNOTIFY_2_OBJID_VAL_MAX                   2
#define ATC_MIPLNOTIFY_2_OBJID_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_3_INSTID_VAL_MIN                   0
#define ATC_MIPLNOTIFY_3_INSTID_VAL_MAX                   2
#define ATC_MIPLNOTIFY_3_INSTID_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_4_RESOURCEID_VAL_MIN                   0
#define ATC_MIPLNOTIFY_4_RESOURCEID_VAL_MAX                   2
#define ATC_MIPLNOTIFY_4_RESOURCEID_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_5_VALTYPE_VAL_MIN                   0
#define ATC_MIPLNOTIFY_5_VALTYPE_VAL_MAX                   2
#define ATC_MIPLNOTIFY_5_VALTYPE_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_6_VALLEN_VAL_MIN                   0
#define ATC_MIPLNOTIFY_6_VALLEN_VAL_MAX                   2
#define ATC_MIPLNOTIFY_6_VALLEN_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_7_VALUE_STR_MAX_LEN              6
#define ATC_MIPLNOTIFY_7_VALUE_STR_DEFAULT              NULL
#define ATC_MIPLNOTIFY_8_INDEX_VAL_MIN                   0
#define ATC_MIPLNOTIFY_8_INDEX_VAL_MAX                   2
#define ATC_MIPLNOTIFY_8_INDEX_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_9_FLAG_VAL_MIN                   0
#define ATC_MIPLNOTIFY_9_FLAG_VAL_MAX                   2
#define ATC_MIPLNOTIFY_9_FLAG_VAL_DEFAULT               0
#define ATC_MIPLNOTIFY_10_ACKID_VAL_MIN                   0
#define ATC_MIPLNOTIFY_10_ACKID_VAL_MAX                   2
#define ATC_MIPLNOTIFY_10_ACKID_VAL_DEFAULT               0

/* AT+MIPLREADRSP */
#define ATC_MIPLREADRSP_0_REF_VAL_MIN                   0
#define ATC_MIPLREADRSP_0_REF_VAL_MAX                   2
#define ATC_MIPLREADRSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLREADRSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLREADRSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_2_RET_VAL_MIN                   0
#define ATC_MIPLREADRSP_2_RET_VAL_MAX                   2
#define ATC_MIPLREADRSP_2_RET_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_3_OBJID_VAL_MIN                   0
#define ATC_MIPLREADRSP_3_OBJID_VAL_MAX                   2
#define ATC_MIPLREADRSP_3_OBJID_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_4_INSTID_VAL_MIN                   0
#define ATC_MIPLREADRSP_4_INSTID_VAL_MAX                   2
#define ATC_MIPLREADRSP_4_INSTID_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_5_RESOURCEID_VAL_MIN                   0
#define ATC_MIPLREADRSP_5_RESOURCEID_VAL_MAX                   2
#define ATC_MIPLREADRSP_5_RESOURCEID_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_6_VALTYPE_VAL_MIN                   0
#define ATC_MIPLREADRSP_6_VALTYPE_VAL_MAX                   2
#define ATC_MIPLREADRSP_6_VALTYPE_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_7_VALLEN_VAL_MIN                   0
#define ATC_MIPLREADRSP_7_VALLEN_VAL_MAX                   2
#define ATC_MIPLREADRSP_7_VALLEN_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_8_VALUE_STR_MAX_LEN              6
#define ATC_MIPLREADRSP_8_VALUE_STR_DEFAULT              NULL
#define ATC_MIPLREADRSP_9_INDEX_VAL_MIN                   0
#define ATC_MIPLREADRSP_9_INDEX_VAL_MAX                   2
#define ATC_MIPLREADRSP_9_INDEX_VAL_DEFAULT               0
#define ATC_MIPLREADRSP_10_FLAG_VAL_MIN                   0
#define ATC_MIPLREADRSP_10_FLAG_VAL_MAX                   2
#define ATC_MIPLREADRSP_10_FLAG_VAL_DEFAULT               0

/* AT+MIPLWRITERSP */
#define ATC_MIPLWRITERSP_0_REF_VAL_MIN                   0
#define ATC_MIPLWRITERSP_0_REF_VAL_MAX                   2
#define ATC_MIPLWRITERSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLWRITERSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLWRITERSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLWRITERSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLWRITERSP_2_RET_VAL_MIN                   0
#define ATC_MIPLWRITERSP_2_RET_VAL_MAX                   2
#define ATC_MIPLWRITERSP_2_RET_VAL_DEFAULT               0

/* AT+MIPLEXECUTERSP */
#define ATC_MIPLEXECUTERSP_0_REF_VAL_MIN                   0
#define ATC_MIPLEXECUTERSP_0_REF_VAL_MAX                   2
#define ATC_MIPLEXECUTERSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLEXECUTERSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLEXECUTERSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLEXECUTERSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLEXECUTERSP_2_RET_VAL_MIN                   0
#define ATC_MIPLEXECUTERSP_2_RET_VAL_MAX                   2
#define ATC_MIPLEXECUTERSP_2_RET_VAL_DEFAULT               0

/* AT+MIPLOBSERVERSP */
#define ATC_MIPLOBSERVERSP_0_REF_VAL_MIN                   0
#define ATC_MIPLOBSERVERSP_0_REF_VAL_MAX                   2
#define ATC_MIPLOBSERVERSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLOBSERVERSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLOBSERVERSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLOBSERVERSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLOBSERVERSP_2_RET_VAL_MIN                   0
#define ATC_MIPLOBSERVERSP_2_RET_VAL_MAX                   2
#define ATC_MIPLOBSERVERSP_2_RET_VAL_DEFAULT               0

/* AT+MIPLDISCOVERRSP */
#define ATC_MIPLDISCOVERRSP_0_REF_VAL_MIN                   0
#define ATC_MIPLDISCOVERRSP_0_REF_VAL_MAX                   2
#define ATC_MIPLDISCOVERRSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLDISCOVERRSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLDISCOVERRSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLDISCOVERRSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLDISCOVERRSP_2_RET_VAL_MIN                   0
#define ATC_MIPLDISCOVERRSP_2_RET_VAL_MAX                   2
#define ATC_MIPLDISCOVERRSP_2_RET_VAL_DEFAULT               0
#define ATC_MIPLDISCOVERRSP_3_LEN_VAL_MIN                   0
#define ATC_MIPLDISCOVERRSP_3_LEN_VAL_MAX                   2
#define ATC_MIPLDISCOVERRSP_3_LEN_VAL_DEFAULT               0

/* AT+MIPLPARAMETERRSP */
#define ATC_MIPLPARAMETERRSP_0_REF_VAL_MIN                   0
#define ATC_MIPLPARAMETERRSP_0_REF_VAL_MAX                   2
#define ATC_MIPLPARAMETERRSP_0_REF_VAL_DEFAULT               0
#define ATC_MIPLPARAMETERRSP_1_MSGID_VAL_MIN                   0
#define ATC_MIPLPARAMETERRSP_1_MSGID_VAL_MAX                   2
#define ATC_MIPLPARAMETERRSP_1_MSGID_VAL_DEFAULT               0
#define ATC_MIPLPARAMETERRSP_2_RET_VAL_MIN                   0
#define ATC_MIPLPARAMETERRSP_2_RET_VAL_MAX                   2
#define ATC_MIPLPARAMETERRSP_2_RET_VAL_DEFAULT               0

/* AT+MIPLUPDATE */
#define ATC_MIPLUPDATE_0_REF_VAL_MIN                   0
#define ATC_MIPLUPDATE_0_REF_VAL_MAX                   2
#define ATC_MIPLUPDATE_0_REF_VAL_DEFAULT               0
#define ATC_MIPLUPDATE_1_LIFETIME_VAL_MIN                   0
#define ATC_MIPLUPDATE_1_LIFETIME_VAL_MAX                   2
#define ATC_MIPLUPDATE_1_LIFETIME_VAL_DEFAULT               0
#define ATC_MIPLUPDATE_2_OFLAG_VAL_MIN                   0
#define ATC_MIPLUPDATE_2_OFLAG_VAL_MAX                   2
#define ATC_MIPLUPDATE_2_OFLAG_VAL_DEFAULT               0

/* AT+MIPLVER */
#define ATC_MIPLVER_VAL_MIN                   0
#define ATC_MIPLVER_VAL_MAX                   2
#define ATC_MIPLVER_VAL_DEFAULT               0

/* AT+SETIMEI */
#define ATC_SETIMEI_0_IMEI_VAL_MIN                   0
#define ATC_SETIMEI_0_IMEI_VAL_MAX                   2
#define ATC_SETIMEI_0_IMEI_VAL_DEFAULT               0

/* AT+SETIMSI */
#define ATC_SETIMSI_0_VALUE_STR_MAX_LEN              6
#define ATC_SETIMSI_0_VALUE_STR_DEFAULT              NULL

#define MAX_CONFIG_LEN   (256)

CmsRetId  onenetCONFIG(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetCREATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDELETE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOPEN(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetCLOSE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetADDOBJ(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDELOBJ(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetNOTIFY(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetREADRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetWRITERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetEXECUTERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOBSERVERSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetDISCOVERRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetPARAMETERRSP(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetUPDATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetVERSION(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetSETIMSI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetSETIMEI(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOTASTART(const AtCmdInputContext *pAtCmdReq);
CmsRetId  onenetOTASTATE(const AtCmdInputContext *pAtCmdReq);
CmsRetId onenetMIPLRD(const AtCmdInputContext *pAtCmdReq);

#endif
