/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_dm.h
*
*  Description: Device manager
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_DM_H__
#define __ATEC_DM_H__

#include "at_util.h"

#define AUTOREGCFG_STR_LEN              200
#define DMCONFIG_CET_RESP_STR_LEN       64
/* AT+AUTOREGCFG */
#define AUTOREGCFG_0_STR_LEN             32
#define AUTOREGCFG_0_STR_DEF             NULL
#define AUTOREGCFG_1_STR_LEN             66
#define AUTOREGCFG_1_STR_DEF             NULL
#define AUTOREGCFG_2_MIN               0
#define AUTOREGCFG_2_MAX               0x7fffffff
#define AUTOREGCFG_2_DEF               0
#define AUTOREGCFG_2_LIFETIME_MIN        1
#define AUTOREGCFG_2_LIFETIME_MAX        0xffff
#define AUTOREGCFG_2_LIFETIME_DEF        0
#define AUTOREGCFG_2_TEST_MIN          0
#define AUTOREGCFG_2_TEST_MAX          1
#define AUTOREGCFG_2_TEST_DEF          0
#define AUTOREGCFG_2_APPKEY_STR_LEN      11
#define AUTOREGCFG_2_APPKEY_STR_DEF      NULL
#define AUTOREGCFG_2_SECRET_STR_LEN    33
#define AUTOREGCFG_2_SECRET_STR_DEF    NULL

/* AT+DMCONFIG */
#define DMCONFIG_0_MIN                 0
#define DMCONFIG_0_MAX                 1
#define DMCONFIG_0_DEF                 0
#define DMCONFIG_1_MIN               0
#define DMCONFIG_1_MAX               0xFFFF//large than 24x60 minutes
#define DMCONFIG_1_DEF               0
#define DMCONFIG_2_STR_LEN             12
#define DMCONFIG_2_STR_DEF             NULL
#define DMCONFIG_3_STR_LEN           33
#define DMCONFIG_3_STR_DEF             NULL
#define DMCONFIG_4_MIN               0
#define DMCONFIG_4_MAX               1
#define DMCONFIG_4_DEF               0


#define UNIKEYINFOM_PARA_STR_LEN             48
#define UNIKEYINFOM_RSP_STR_LEN              256

/* AT+UNIKEYINFOM */
#define UNIKEYINFOM_0_MPK_STR_LEN             17
#define UNIKEYINFOM_0_MPK_STR_DEF             NULL
#define UNIKEYINFOM_1_MPS_STR_LEN                 33
#define UNIKEYINFOM_1_MPS_STR_DEF                 NULL
#define UNIKEYINFOM_2_MDK_STR_LEN             33        //device imei
#define UNIKEYINFOM_2_MDK_STR_DEF             NULL
#define UNIKEYINFOM_3_MDS_STR_LEN                 33
#define UNIKEYINFOM_3_MDS_STR_DEF                 NULL

/* AT+UNIDELKEYINFOM */
#define UNIDELKEYINFOM_0_MIN               0   //0-clean pk/ps/dk/ds     1-clean ds     2-clean token
#define UNIDELKEYINFOM_0_MAX               2
#define UNIDELKEYINFOM_0_DEF               0


/* AT+UNIKEYINFO */
#define UNIKEYINFO_0_PK_STR_LEN             17
#define UNIKEYINFO_0_PK_STR_DEF             NULL
#define UNIKEYINFO_1_PS_STR_LEN                 33
#define UNIKEYINFO_1_PS_STR_DEF                 NULL
#define UNIKEYINFO_2_DK_STR_LEN             33        //device imei
#define UNIKEYINFO_2_DK_STR_DEF             NULL
#define UNIKEYINFO_3_DS_STR_LEN                 33
#define UNIKEYINFO_3_DS_STR_DEF                 NULL

/* AT+UNIDELKEYINFO */
#define UNIDELKEYINFO_0_MIN               0   //0-clean pk/ps/dk/ds     1-clean ds     2-clean token
#define UNIDELKEYINFO_1_MAX               2
#define UNIDELKEYINFO_2_DEF               0

/* AT+UNIAUTOREGCFG */
#define UNIAUTOREGCFG_0_MANUFAC_STR_LEN             32
#define UNIAUTOREGCFG_0_MANUFAC_STR_DEF             NULL
#define UNIAUTOREGCFG_1_MODULE_TPYE_STR_LEN           32
#define UNIAUTOREGCFG_1_MODULE_TPYE_STR_DEF           NULL
#define UNIAUTOREGCFG_2_MODULE_SW_STR_LEN           32
#define UNIAUTOREGCFG_2_MODULE_SW_STR_DEF           NULL
#define UNIAUTOREGCFG_3_MODULE_HW_STR_LEN             32
#define UNIAUTOREGCFG_3_MODULE_HW_STR_DEF             NULL
#define UNIAUTOREGCFG_4_OEM_STR_LEN                 32
#define UNIAUTOREGCFG_4_OEM_STR_DEF                 NULL
#define UNIAUTOREGCFG_5_OEM_TPYE_STR_LEN              32
#define UNIAUTOREGCFG_5_OEM_TPYE_STR_DEF              NULL
#define UNIAUTOREGCFG_MAX_STR_LEN                   48

/* AT+UNIAUTOREG */
#define UNIAUTOREG_0_MIN               0   //0-clean pk/ps/dk/ds     1-clean ds     2-clean token
#define UNIAUTOREG_0_MAX               1
#define UNIAUTOREG_0_DEF               0

CmsRetId dmAUTOREGCFG(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniKeyInfom(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniKeyInfo(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniDelKeyInfom(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniDelKeyInfo(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniAutoRegCfg(const AtCmdInputContext *pAtCmdReq);
CmsRetId dmCuccUniAutoReg(const AtCmdInputContext *pAtCmdReq);

#endif

/* END OF FILE */
