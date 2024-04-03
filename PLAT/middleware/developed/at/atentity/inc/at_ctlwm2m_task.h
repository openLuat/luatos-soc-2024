/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_lwm2m_task.h
*
*  Description: Process LWM2M client related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_CTLWM2M_TASK_H
#define _AT_CTLWM2M_TASK_H

#include "cms_api.h"
//#include "ec_ctlwm2m_api.h"
//for sm9
//#include "liblwm2m.h"

#ifdef WITH_MBEDTLS
#define CTLWM2M_TASK_STACK_SIZE   1536
#else
#define CTLWM2M_TASK_STACK_SIZE   1024
#endif

#define CTLWM2M_MSG_TIMEOUT 500

#define CT_SM9_TASK_STACK_SIZE   3584

#define CT_SM9_GETKEY_TASK_STACK_SIZE   3072

/*
 * APPL SGID: APPL_CTLWM2M, related PRIM ID
*/
typedef enum applCtPrimId_Enum
{
    APPL_CT_PRIM_ID_BASE = 0,
    APPL_CT_REG_CNF,
    APPL_CT_DEREG_CNF,
    APPL_CT_SEND_CNF,
    APPL_CT_UPDATE_CNF,
    APPL_CT_SM9_INIT_CNF,
    APPL_CT_SM9_ENC_CNF,
    APPL_CT_SM9_DEC_CNF,
    APPL_CT_SM9_SIGN_CNF,
    APPL_CT_SM9_VERI_CNF,
    APPL_CT_SM9_ONLINEKEY_CNF,
    APPL_CT_IND,
    APPL_CT_PRIM_ID_END = 0xFF
}applCtPrimId;

enum CTLWM2M_RC
{
    CTLWM2M_OK = 0,
    CTLWM2M_TASK_ERR
};

enum CTLWM2M_MSG_CMD
{
    MSG_CTLWM2M_DEREG_CLIENT, 
    MSG_CTLWM2M_SEND, 
    MSG_CTLWM2M_UPDATE, 
    MSG_CTLWM2M_REG, 
    MSG_CTLWM2M_DEREG_DONE, 
    MSG_CTLWM2M_DEREG_NON_CLIENT, 
    MSG_CTSM9_INIT,
    MSG_CTSM9_ENC,
    MSG_CTSM9_DEC,
    MSG_CTSM9_SIGN,
    MSG_CTSM9_VERIFY,
    MSG_CTSM9_ONLINEKEY
};

enum CTLWM2M_TASK_STATUS
{
    CTLWM2M_TASK_STAT_NONE, 
    CTLWM2M_TASK_STAT_CREATE
};

typedef struct
{
	UINT8 mode;
    UINT8 seqNum;
    CHAR *data;
}CTLWM2M_SEND_DATA_STRUCT;

typedef struct
{
    BOOL  update;
	UINT8 bindingMode;
}CTLWM2M_UPDATE_STRUCT;

typedef struct
{
    uint8_t cmd_type;
    UINT16 reqhandle;
    CTLWM2M_SEND_DATA_STRUCT send_data;
}CTLWM2M_ATCMD_Q_MSG;

#if 0
typedef struct
{
    uint8_t cmd_type;
    UINT16 reqhandle;
    uint8_t index;
    UINT16 length;
    UINT16 length2;
    uint8_t* param1;
    uint8_t* param2;
    uint8_t* param3;
    uint8_t type;
}CT_SM9_ATCMD_Q_MSG;

typedef struct
{
	uint8_t ret;
	uint16_t len;
    char* rsp;
}ct_sm9_cnf_msg;

typedef struct {
    UINT16 reqHandle;
    uint8_t index;
    uint8_t type;
    uint8_t* pubKey;
    uint8_t* priKey;
    lwm2m_context_t* context;
    ctiot_funcv1_client_data_t* clientData;
} sm9Lwm2mlientContext;
#endif

CmsRetId ctlwm2m_client_reg(UINT32 reqHandle);
CmsRetId ctlwm2m_client_update(UINT32 reqHandle);
CmsRetId ctlwm2m_client_send(UINT32 reqHandle, char* data, UINT8 mode, UINT8 seqNum);
CmsRetId ctlwm2m_client_dereg(UINT32 reqHandle, BOOL bNonType);
CmsRetId ctlwm2m_client_dereg_done(UINT32 reqHandle);

int ctlwm2m_client_create(void);

void  ctiotEngineInit(void);

#endif
