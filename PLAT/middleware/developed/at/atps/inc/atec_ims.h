/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_ims.h
*
*  Description: Macro definition for IMS service related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef  __AT_EC_IMS_H__
#define  __AT_EC_IMS_H__

#include "at_util.h"

#define ATC_ECREG_0_VAL_MIN                   0
#define ATC_ECREG_0_VAL_MAX                   1
#define ATC_ECREG_0_VAL_DEF                   0

/* AT+CIREG */
#define ATC_CIREG_0_VAL_MIN                       0
#define ATC_CIREG_0_VAL_MAX                       2
#define ATC_CIREG_0_VAL_DEFAULT                   2

/* AT+ECIMSJSON */
#define ATC_ECIMSJSON_0_FILE_STR_MAX_LEN             31
#define ATC_ECIMSJSON_0_FILE_STR_DEF                 NULL
#define ATC_ECIMSJSON_1_OPER_VAL_MIN                  0
#define ATC_ECIMSJSON_1_OPER_VAL_MAX                  2
#define ATC_ECIMSJSON_1_OPER_VAL_DEF                  0
#define ATC_ECIMSJSON_2_PATH_STR_MAX_LEN             127
#define ATC_ECIMSJSON_2_PATH_STR_DEF                 NULL
#define ATC_ECIMSJSON_3_VALUE_STR_MAX_LEN            63
#define ATC_ECIMSJSON_3_VALUE_STR_DEF                NULL


CmsRetId  imsECIMSREG(const AtCmdInputContext *pAtCmdReq);

CmsRetId  imsECIMSJSON(const AtCmdInputContext *pAtCmdReq);

/*AT+CIREG*/
CmsRetId  imsCIREG(const AtCmdInputContext *pAtCmdReq);

CmsRetId  imsECIMSRUS(const AtCmdInputContext *pAtCmdReq);

#endif

