/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_xota_task.h
*
*  Description:FW upgrade over serial port via AT command
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __AT_XOTA_TASK_H__
#define __AT_XOTA_TASK_H__

#include "at_util.h"

#define XOTA_MQUE_MSG_MAXNUM        (2 * XOTA_NVM_DEV_MAXNUM)
#define XOTA_MQUE_SEND_TIMEOUT       500
#define XOTA_TASK_STACK_SIZE         2048

#define XOTA_RESP_BUF_MAXLEN         128
#define XOTA_RDBK_BUF_MAXLEN        (XOTA_DATA_HEXSTR_MAXLEN + 100) /* rsvd 100 bytes for other fields*/

#define XOTA_VERI_HASH_MAXLEN        32

#define XOTA_FW_VER_MAXLEN           64
#define XOTA_FW_NAME_MAXLEN          64
#define XOTA_FW_PSN_MINNUM           0
#define XOTA_FW_PSN_MAXNUM           65535
#define XOTA_DATA_BYTES_MAXNUM       1024
#define XOTA_DATA_HEXSTR_MAXLEN      XOTA_HEXSTR_LEN(XOTA_DATA_BYTES_MAXNUM)

#define XOTA_HEXBYTE_NUM(strlen)       ((strlen) >> 1)
#define XOTA_HEXSTR_LEN(nbytes)        ((nbytes) << 1)
#define XOTA_HEXCHAR_TO_INTEGER(hex)   ((hex >= '0' && hex <= '9') ? (hex - '0') : \
                                         ((hex >= 'A' && hex <= 'F') ? (hex - 'A' + 10) : \
                                         ((hex >= 'a' && hex <= 'f') ? (hex - 'a' + 10) : 0)))

#define XOTA_REQMSG_INIT(msg)    \
        do{\
            memset(&msg, 0, sizeof(XotaReqMsg_t));\
            msg.cmdCode  = XOTA_CMD_CODE_MAXNUM;\
        }while(0)

#define XOTA_CNFMSG_INIT(msg)    \
        do{\
            memset(&msg, 0, sizeof(XotaCnfMsg_t));\
            msg.errCode  = XOTA_EC_UNDEF_ERROR;\
        }while(0)

typedef enum
{
    APPL_XOTA_PRIM_ID_BASE = 0,

    APPL_XOTA_REQ,
    APPL_XOTA_CNF,
    APPL_XOTA_RDBK,

    APPL_XOTA_PRIM_ID_END = 0xFF
}ApplXotaPrimId;

typedef enum
{
    XOTA_CMD_VER_1 = 1, /* AT+NFWUPD */
    XOTA_CMD_VER_2      /* AT+ECOTA */
}XotaCmdVer_e;

typedef enum
{
    FWUPD_CMD_CODE_BEGIN = 0,
    FWUPD_CMD_CLEAR_FLASH = FWUPD_CMD_CODE_BEGIN,
    FWUPD_CMD_DOWNLOAD_FW,
    FWUPD_CMD_VERIFY_FW,
    FWUPD_CMD_QUERY_FWNAME,
    FWUPD_CMD_QUERY_FWVER,
    FWUPD_CMD_UPGRADE_FW,
    FWUPD_CMD_DOWNLOAD_OVER,
    FWUPD_CMD_READ_FLASH,
    FWUPD_CMD_DDLVR_FW,
    FWUPD_CMD_DFU_STATUS,
    FWUPD_CMD_CODE_END = FWUPD_CMD_DFU_STATUS,

    FWUPD_CMD_CODE_MAXNUM
}FwupdCmdCode_e;

typedef enum
{
    XOTA_CMD_CODE_BEGIN = 0,
    XOTA_CMD_PRESET_MZONE = XOTA_CMD_CODE_BEGIN,
    XOTA_CMD_CLEAR_MZONE,
    XOTA_CMD_READ_MZONE,
    XOTA_CMD_WRITE_PKG,
    XOTA_CMD_DDLVR_PKG,  /* direct delivery */
    XOTA_CMD_WRITE_OVER,
    XOTA_CMD_VERIFY_PKG,
    XOTA_CMD_UPGRADE,
    XOTA_CMD_QUERY_FWNAME,
    XOTA_CMD_QUERY_FWVER,
    XOTA_CMD_QUERY_STATUS,
    XOTA_CMD_CODE_END = XOTA_CMD_QUERY_STATUS,

    XOTA_CMD_CODE_MAXNUM
}XotaCmdCode_e;

typedef enum
{
    XOTA_NVM_DEV_BEGIN = 0,
    XOTA_NVM_DEV_IFLASH = XOTA_NVM_DEV_BEGIN,  /* internal system flash */
    XOTA_NVM_DEV_EFLASH,                       /* external extended flash */
    XOTA_NVM_DEV_SD,
    XOTA_NVM_DEV_END = XOTA_NVM_DEV_SD,

    XOTA_NVM_DEV_MAXNUM
}XotaNvmDev_e;

typedef struct
{
    uint32_t addr;
    uint32_t size;
}XotaNvmZone_t;

typedef enum
{
    XOTA_DF_DLVR_START = 0,
    XOTA_DF_DLVR_PKG
    //XOTA_DF_DLVR_DONE
}XotaDlvrFlags_e;

typedef struct
{
    uint16_t  pkgSn;
    uint8_t   flags;   /* XotaPkgFlags_e */
    uint8_t   crc8;
    uint32_t  dataLen;  /* StrLen or octetCnt */
    uint8_t  *data;     /* hexStr or octets */
}XotaPkgDesc_t;

typedef enum
{
    XOTA_CA_SHA256 = 0,

    XOTA_CA_ALGO_MAXNUM
}XotaChksumAlgo_e;

typedef struct
{
    uint8_t   hasHash;
    uint8_t   hashType;  /* XotaChksumAlgo_e */
    uint16_t  hashLen;
    uint8_t   hash[XOTA_VERI_HASH_MAXLEN];
}XotaVeriDesc_t;

typedef struct
{
    uint32_t atHandle;
    uint8_t  cmdCode;  /* XotaCmdCode_e */
    uint8_t  nvmDev;   /* XotaNvmDev_e */
    uint8_t  rsvd[2];
    union
    {
        XotaNvmZone_t   nvm;
        XotaPkgDesc_t   pkg;
        XotaVeriDesc_t  veri;
    }u;
}XotaReqMsg_t;

typedef struct
{
    uint8_t  errCode;  /* AtXotaErrCode_e */
    uint8_t  rsvd;
    uint16_t strLen;
    uint8_t  respStr[XOTA_RESP_BUF_MAXLEN];
}XotaCnfMsg_t;

typedef struct
{
    void  *dlpdu;
}XotaRdBkMsg_t;

typedef struct
{
    uint8_t   appId;    /* CmsAppId */
    uint8_t   nvmDev;   /* XotaNvmDev_e */
    uint8_t   cmdVer;   /* XotaCmdVer_e*/
    uint8_t   rsvd;
    uint8_t  *cmdName;
}XotaAppConf_t;

typedef struct
{
    uint8_t   isInit;
    uint8_t   appId;    /* CmsAppId */
    uint8_t   nvmDev;   /* XotaNvmDev_e */
    uint8_t   chanMode;
    uint32_t  atHandle;
    uint32_t  recvLen;
    uint32_t  totalLen;
    uint32_t  timeOutMs;
}XotaPkgDlvrPreMan_t;


int32_t  XOTA_initTask(XotaAppConf_t *conf);
int32_t  XOTA_deinitTask(void);
int32_t  XOTA_sendMsg(XotaReqMsg_t *msg);

uint8_t  XOTA_chkAppId(XotaNvmDev_e nvmDev, CmsAppId appId);
uint32_t XOTA_getNvmSize(XotaNvmDev_e nvmDev);

void XOTA_enableUlfc(uint8_t chanId);
void XOTA_disableUlfc(UINT8 chanId);

XotaCmdCode_e  XOTA_ConvToXotaCmd(FwupdCmdCode_e fwupdCmd);
FwupdCmdCode_e XOTA_ConvToFwupdCmd(XotaCmdCode_e xotaCmd);

XotaPkgDlvrPreMan_t* XOTA_getDlvrPreCtx(uint16_t chanId);
int32_t XOTA_ungetDlvrPreCtx(XotaPkgDlvrPreMan_t *dlvrCtx);

#endif

/* END OF FILE */

