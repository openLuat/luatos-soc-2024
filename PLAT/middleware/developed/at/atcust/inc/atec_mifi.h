#ifndef __ATEC_MIFI_H__
#define __ATEC_MIFI_H__
#include "at_util.h"

#ifdef FEATURE_MIFI_AT_ENABLE
/******************************************************************************
 *****************************************************************************
 * AT MIFI COMMON MARCO
 *****************************************************************************
******************************************************************************/
#define EC_MWIFICFG_CNF_STR_LEN         (512)
#define EC_MWIFIVERSION_CNF_STR_LEN     (128)

#define EC_MWIFIMAC_CNF_STR_LEN         (32)

#define ATEC_MWIFI_STA_NUM_MIN          (0)
#define ATEC_MWIFI_STA_NUM_MAX          (3)
#define ATEC_MWIFI_STA_NUM_DEF          ATEC_MWIFI_STA_NUM_MIN

#ifdef FEATURE_WIFI_SWITCH_ENABLE
#define  EC_WIFICFG_NETTYPE_MIN             1
#define  EC_WIFICFG_NETTYPE_MAX             3
#define  EC_WIFICFG_NETTYPE_DEF             1

#define  EC_WIFICFG_NETTYPE_IPV4            1
#define  EC_WIFICFG_NETTYPE_IPV6            2
#define  EC_WIFICFG_NETTYPE_IPV4V6          3

#define  EC_WIFICFG_MTU_VALUE_MIN           0
#define  EC_WIFICFG_MTU_VALUE_MAX           1500

#define  EC_WIFICFG_MTU_VALUE_DEF           EC_WIFICFG_MTU_VALUE_MAX

#define  EC_WIFICFG_IP_BUF_SIZEMAX          63

#define  EC_WIFICFG_STR_DEFAULT             NULL

#define  EC_WIFICFG_CNF_STR_LEN             256

#define EC_WTOC_CID_VAL_MIN                 1
#define EC_WTOC_CID_VAL_MAX                 15
#define EC_WTOC_CID_VAL_DEF                 1

#endif
/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

CmsRetId mwifiCFG(const AtCmdInputContext *pAtCmdReq);

CmsRetId mwifiMAC(const AtCmdInputContext *pAtCmdReq);

CmsRetId mwifiSTA(const AtCmdInputContext *pAtCmdReq);

CmsRetId mwifiPWMODE(const AtCmdInputContext *pAtCmdReq);

CmsRetId mwifiVERSION(const AtCmdInputContext *pAtCmdReq);

CmsRetId mwifiSLEEP(const AtCmdInputContext *pAtCmdReq);
#ifdef FEATURE_WIFI_SWITCH_ENABLE
CmsRetId  ecWIFISWITCH(const AtCmdInputContext *pAtCmdReq);

CmsRetId  ecCATWANSWITCH(const AtCmdInputContext *pAtCmdReq);
#endif

#endif/*FEATURE_MIFI_AT_ENABLE*/

#endif/*__ATEC_MIFI_H__*/
