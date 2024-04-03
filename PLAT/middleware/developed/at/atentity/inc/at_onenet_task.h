/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: at_onenet_task.h
*
*  Description: Process onenet related AT commands
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef _AT_ONENET_TASK_H
#define _AT_ONENET_TASK_H

#include "at_util.h"
#include "commontypedef.h"
#include "cis_def.h"
#include "cms_util.h"
#include "cis_api.h"
#include "cis_if_sys.h"
//#include "ps_event_callback.h"

#define CIS_NVM_FILE_NAME      "cisconfig.nvm"

#define CIS_ERRID_PARAMETER_ERROR         (601)
#define CIS_ERRID_STATUS_ERROR            (602)
#define CIS_ERRID_UNKNOWN_ERROR           (100)

#define CIS_ERRID_OK                      (650)

#define CIS_ERRID_NOT_SUPPORT             (CIS_ERRID_OK + 1)
#define CIS_ERRID_SDK_ERROR               (CIS_ERRID_OK + 2)
#define CIS_ERRID_NO_INSTANCE             (CIS_ERRID_OK + 3)
#define CIS_ERRID_MALLOC_FAIL             (CIS_ERRID_OK + 4)
#define CIS_ERRID_NO_NETWORK              (CIS_ERRID_OK + 5)
#define CIS_ERRID_CONFIG_FAIL             (CIS_ERRID_OK + 6)
#define CIS_ERRID_CONFLICT_FAIL           (CIS_ERRID_OK + 7)// add for SIM OTA
#define CIS_ERRID_TASK_ERR                (CIS_ERRID_OK + 8)// add for AT handle
#define CIS_ERRID_NO_OBJMEM               (CIS_ERRID_OK + 9)// no memory to store new object 

#define ONENET_ATTR_NUM         (100)
#define ONENET_AT_RESPONSE_DATA_LEN     (100)

#define ONENET_CMD_READRESP           (8)
#define ONENET_CMD_WRITERESP          (9)
#define ONENET_CMD_EXECUTERESP        (10)
#define ONENET_CMD_OBSERVERESP        (11)
#define ONENET_CMD_DISCOVERRESP       (12)
#define ONENET_CMD_PARAMRESP          (13)

#define MTU_SIZE   (1280)

#define ONENET_INSTANCE_NUM          (1)
#define ONENET_CONFIG_MAX_BUFFER_LEN (105)
#define ONENET_CONFIG_BUFFER_DTLS         "130053F10003F20045050011C00005434D494F540000000000123138332E3233302E34302E33393A35363834001D41757468436F64653A45433631363B50534B3A454947454E434F4D4D3BF30008E400C80000" //with DTLS userdata(AuthCode:EC616;PSK:AirM2M;) 83
#define ONENET_CONFIG_BUFFER         "130045F10003F20037050011000005434D494F540000000000123138332E3233302E34302E34303A35363833000F41757468436F64653A3B50534B3A3BF30008E400C80000"
#define ONENET_CONFIG_BUFFER_BS      "130045F10003F20037050011800005434D494F540000000000123138332E3233302E34302E33393A35363833000F41757468436F64653A3B50534B3A3BF30008E400C80000"
#define ONENET_MAX_OBJECT_COUNT      (10)
#define ONENET_MAX_OBJECT_INSTANCE_COUNT (8)
#define ONENET_HOST_NAME_LEN         (40)
#define ONENET_PSK_LEN               (32)
#define ONENET_PSKID_LEN             (32)


#define ONENET_FLAG_LAST             (0)

#ifdef WITH_TINYDTLS
#define ONENET_MAIN_TASK_STACK_SIZE   3584
#else
#define ONENET_MAIN_TASK_STACK_SIZE   1024*2+512
#endif

//AT HANDLE 
#define ONENET_ATHDL_TASK_STACK_SIZE   1024
#define ONENET_MSG_TIMEOUT 500

#define ONENET_AUTO_UPDATE_ENABLE 0

#if ONENET_AUTO_UPDATE_ENABLE
#define ONENET_AUTO_UPDATE_THD (4*3600)
#define ONENET_AUTO_UPDATE_IN_ADVANCE (ONENET_AUTO_UPDATE_THD*1/10)
#define CIS_LIFETIMEOUT_ID  DEEPSLP_TIMER_ID6
#endif

typedef enum 
{
    MSG_ONENET_NOTIFY,
    MSG_ONENET_OPEN,
    MSG_ONENET_CLOSE, 
    MSG_ONENET_UPDATE, 
}ONENET_MSG_CMD;

enum ONENET_TASK_STATUS
{
    ONENET_TASK_STAT_NONE, 
    ONENET_TASK_STAT_CREATE
};
//AT HANDLE

/*
 * APPL SGID: APPL_Onenet, related PRIM ID
*/
typedef enum applOnenetPrimId_Enum
{
    APPL_ONENET_PRIM_ID_BASE = 0,
    APPL_ONENET_OPEN_CNF,
    APPL_ONENET_NOTIFY_CNF,
    APPL_ONENET_CLOSE_CNF,
    APPL_ONENET_UPDATE_CNF,
    APPL_ONENET_IND,
    APPL_ONENET_PRIM_ID_END = 0xFF
}applOnenetPrimId;

typedef enum {
    ONENET_OP_RESULT_205_CONTENT = 1,
    ONENET_OP_RESULT_204_CHANGED = 2,
    ONENET_OP_RESULT_400_BAD_REQUEST = 11,
    ONENET_OP_RESULT_401_UNAUTHORIZED = 12,
    ONENET_OP_RESULT_404_NOT_FOUND = 13,
    ONENET_OP_RESULT_405_METHOD_NOT_ALLOWED = 14,
    ONENET_OP_RESULT_406_NOT_ACCEPTABLE = 15
} onenet_at_result_t;

typedef enum {
    TASK_STATUS_INIT = 0,
    TASK_STATUS_OPEN,
    TASK_STATUS_CLOSE
} onenetTaskStatus;

typedef enum
{
    SYSTEM_FREE,
    SYSTEM_BUSY,
    SYSTEM_FREE_ONE
}onenet_sleep_status_e;

//AT HANDLE
typedef struct
{
    UINT32 msgid;
    UINT32 objectid;
    UINT32 instanceid;
    UINT32 resourceid;
    UINT32 valuetype;
    UINT32 len;
    UINT8 *value;
    UINT32 ackid;
    cis_coapret_t result;
    UINT8 raiflag;
}onenetResuourcCmd;

typedef struct
{
    UINT32 msgid;
    cis_coapret_t result;
    UINT8 raiflag;
}onenetRspCmd;

typedef struct
{
    UINT32 lifetime;
    BOOL withObjectFlag;
    UINT8 raiflag;
}onenetUpdateCmd;

typedef struct
{
    INT32 cmd_type;
    UINT16 reqhandle;
    UINT32 onenetId;
    INT32 lifetime;
    INT32 timeout;
    UINT8 oflag;
    UINT8 raiflag;
    void* cmd;
}ONENET_ATCMD_Q_MSG;

typedef struct
{
    int ret;
}onenet_cnf_msg;
//AT HANDLE

typedef struct {
    BOOL bUsed;
    cis_oid_t objectId;
    cis_instcount_t instCount;
    UINT8 instBitmap[(ONENET_MAX_OBJECT_INSTANCE_COUNT + 7) / 8];
    cis_attrcount_t attrCount;
    cis_actcount_t actCount;
} onenetObjectInfo;

typedef struct OnenetSIMOTAContext_Tag
{
    BOOL                bSimOtaStart;
    UINT8               otaFinishState;
    UINT8               otaAtReturnCode;
    UINT32              timeoutValue;
    UINT16              configLen;
    UINT8               configBuf[ONENET_CONFIG_MAX_BUFFER_LEN];
    UINT32              reqhandle;
}OnenetSIMOTAContext;

typedef struct cisNvmHeader_Tag
{
    UINT16 fileBodySize; //file body size, not include size of header;
    UINT8  version;
    UINT8  checkSum;
}cisNvmHeader;




typedef enum
{
    CIS_DL_BUFCFG_BUF_NONE = 0,
    CIS_DL_BUFCFG_BUF_WRITE,
    CIS_DL_BUFCFG_BUF_EXECUTE,
    CIS_DL_BUFCFG_BUF_WRITE_AND_EXCUTE,
}cisDLBufCfg;

typedef enum
{
    CIS_DL_URC_MODE_OFF = 0,
    CIS_DL_URC_MODE_ON,
}cisDLBufURCMode;



typedef struct cisDLBufContext_Tag
{
    cisDLBufferInfo buf[CIS_MAX_DL_BUFFER_NUM];
    uint16_t total_len;
    uint8_t fifo_header;
    uint8_t fifo_ending;
    uint32_t recv_cnt;
    uint32_t drop_cnt;
    bool urc_direct_send;
}cisDLBufContext;

typedef struct cisMIPLCfg_v1_Tag
{
    // 0x0
    CHAR ip[40];
    // 0x28
    CHAR port[6];
    UINT8 rsp_timeout:5;
    UINT8 obs_autoack:1;
    UINT8 auth_enable:1;
    UINT8 dtls_enable:1;
    CHAR auth_code[17];
    // 0x40
    CHAR psk[17];
    UINT8 write_format:1;
    cisDLBufCfg buf_cfg:2;
    cisDLBufURCMode buf_urc_mode:1;
    UINT8 bsMode:1;
    UINT8 bConfiged:1;
    UINT8 reserved2;
    // 0x54
    UINT32 valid_magic;
}cisMIPLCfg_v1;

typedef struct {
    BOOL bUsed;
    BOOL bConnected;
    BOOL bRestoreFlag;
    BOOL FotaState;
    UINT32 onenetId;
    void *cis_context;
    cis_time_t lifeTime;
    cis_time_t timeout;
    UINT8 configBuf[ONENET_CONFIG_MAX_BUFFER_LEN];
    UINT16 configLen;
    onenetObjectInfo objectInfo[ONENET_MAX_OBJECT_COUNT];
    UINT8 host[ONENET_HOST_NAME_LEN];
    UINT32 reqhandle;
    BOOL isPsk;
    UINT8 identify[ONENET_PSKID_LEN];
    UINT8 secret[ONENET_PSK_LEN];
    UINT32 closeHandle;
    BOOL bPSKPortConfilict;
    BOOL bEngineInit;
} onenetContext;

#define CIS_NVM_FILE_VERSION_3   3
typedef struct {
    //offset 0
    UINT32 bUsed:1;
    UINT32 bConnected:1;
    UINT32 bRestoreFlag:1;
    UINT32 FotaState:1;
    //offset 0x4
    UINT32 lifeTime;
    //offset 0x8
    UINT32 otaFinishState:8;
    UINT32 onenetId:8;
    UINT32 reserved0:16;
    //offset 0xC
    UINT32 reqhandle;
    
    //offset 0x10
    UINT8 host[ONENET_HOST_NAME_LEN];

    //offset 0x38
    UINT32 reserved1;

    //offset 0x3C
    UINT32 isPsk:8;
    UINT32 observeObjNum:8;
    UINT32 reserved2:16;
    
    //offset 0x40
    UINT8 identify[32];
    UINT8 secret[32];

    //offset 0x80
    cisMIPLCfg_v1 mMiplcfg;

    //offset 0xD8
    onenetObjectInfo objectInfo[ONENET_MAX_OBJECT_COUNT];
    observed_backup_t gObservedBackup[MAX_OBSERVED_COUNT];
} onenetRetentionContext_v3;

typedef onenetRetentionContext_v3 onenetRetentionContext;



//AT HANDLE
CmsRetId onenet_client_open(UINT32 reqHandle, UINT32 id, INT32 lifetime, INT32 timeout);
CmsRetId onenet_client_close(UINT32 reqHandle, UINT32 id);
CmsRetId onenet_client_update(UINT32 reqHandle, UINT32 id, INT32 lifetime, UINT8 oflag, UINT8 raiflag);
CmsRetId onenet_client_notify(UINT32 reqHandle, UINT32 id, void* cmd);
CmsRetId onenet_client_update(UINT32 reqHandle, UINT32 id, INT32 lifetime, UINT8 oflag, UINT8 raiflag);
INT32 onenet_athandle_create(void);
//AT HANDLE

void onenetSaveFlash(void);
INT8 onenetRestoreContext(void);
void onenetClearFlash(bool ToClearCfg);

void onenetSleepVote(onenet_sleep_status_e bSleep);

onenetContext *onenetCreateInstance(void);
onenetContext *onenetSearchInstance(UINT32 onenetId);
onenetContext *onenetSearchCisContext(void *cis_context);
onenetContext *onenetSearchCisInstance(UINT32 onenetId);
void onenetWaitEngineInit(void);

UINT32 onenetHexToBin(UINT8 *dest, UINT8 *source, UINT32 max_dest_len);
void onenetDeleteInstance(onenetContext *onenet);

INT32 onenetDelete(UINT32 onenetId);
INT32 onenetAddObj(UINT32 onenet_id, UINT32 objectid, UINT32 instancecount, UINT8 *instancebitmap, UINT32 attributecount, UINT32 actioncount);
INT32 onenetDelObj(UINT32 onenet_id, UINT32 objectid);
BOOL onenetResult2Coapret(UINT32 cmdid, onenet_at_result_t result, cis_coapret_t *coapret);
INT32 onenetResponse(UINT32 onenet_id, UINT32 msgid, const cis_uri_t *uri, const cis_data_t *cis_data, cis_coapret_t result, UINT8 raiflag);
UINT32 onenetParseAttr(const char *param_string, const char *field_buffer, const char *field_key[], char *field_list[], UINT32 field_max_num, const char *delim);
INT32 onenetReadResp(UINT32 onenet_id, UINT32 msgid, UINT32 objectid, UINT32 instanceid, UINT32 resourceid, UINT32 valuetype, UINT32 len, UINT8 *value, cis_coapret_t result, UINT8 raiflag);
UINT32 onenetBinToHex(char *dest, const UINT8 *source, UINT32 max_dest_len);
void onenetServerIPGet(UINT32 onenetId, BOOL *pBsMode, CHAR* ip, CHAR* port);
void onenetSimOtaTaskEntry(void *arg);
BOOL atCheckForNumericOnlyChars(const UINT8 *password);

INT32 onenetServerIPConfig(UINT32 onenetId, BOOL bsMode, CHAR* ip, CHAR* port);
bool onenetDLBufferIsEnable(UINT32 onenetId, cisDLBufType type);
void onenetDLPush2Buffer(cisDLBufferInfo bufInfo);
void onenetDLBufferInfoCheck(uint8_t *pBuffered, uint16_t *pBufferedSize, uint32_t *pRecvCnt, uint32_t *pDropCnt);
bool onenetDLPopout(void);

INT32 onenetRspTimeoutConfig(UINT32 onenetId, UINT8 rsp_timeout);
void onenetRspTimeoutGet(UINT32 onenetId, UINT8 *rsp_timeout);
INT32 onenetObsAutoackConfig(UINT32 onenetId, UINT8 obs_autoack);
void onenetObsAutoackGet(UINT32 onenetId, UINT8 *obs_autoack);
INT32 onenetAuthConfig(UINT32 onenetId, BOOL auth_enable, UINT8 *auth_code);
void onenetAuthGet(UINT32 onenetId, UINT8 *auth_enable, UINT8 *auth_code);
INT32 onenetDTLSConfig(UINT32 onenetId, BOOL dtls_enable, UINT8 *psk);
void onenetDTLSGet(UINT32 onenetId, UINT8 *dtls_enable, UINT8 *psk);
INT32 onenetWriteFormatConfig(UINT32 onenetId, UINT8 write_format);
void onenetWriteFormatGet(UINT32 onenetId, UINT8 *write_format);
INT32 onenetMIPLRDConfig(UINT32 onenetId, UINT8 buf_cfg, UINT8 buf_urc_mode);
void onenetMIPLRDCfgGet(UINT32 onenetId, UINT8 *buf_cfg, UINT8 *buf_urc_mode);
bool onenetURCSendMode(void);
INT16 createConfigBuf(UINT32 onenetId, UINT8* buf, BOOL* bPSKPortConfilict);

#if ONENET_AUTO_UPDATE_ENABLE
void cis_lifeTimeStart(uint32_t timeout_second);
void cis_lifeTimeStop(void);
bool cis_DeepSlpTimerHasExp(void);
void cis_lifeTimeExpCallback(uint8_t id);
void cis_deepSleepTimerCbRegister(void);
#endif


#endif

