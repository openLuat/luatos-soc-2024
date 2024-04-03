#ifndef __IMI_PRO_H__
#define __IMI_PRO_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imipro.h
 Description:    - IMS module/device interface
 History:        - 2023/04/10,   Original created
******************************************************************************/
#include "imicomm.h"

/*********************************************************************************
*******************************************************************************
* Macros
*******************************************************************************
*********************************************************************************/
#define IMI_PRO_MAX_FILE_NUM 10
#define IMI_PRO_FILENAME_MAX_LEN 31
#define IMI_PRO_JSON_CFG_MAX_LEN 2047

#define IMI_PRO_PATH_MAX_LEN  127
#define IMI_PRO_VALUE_MAX_LEN 63

#define IMI_IP_MAX_STR_LEN          40
#define IMI_APN_MAX_STR_LEN         100
#define IMI_PRODUCT_NAME_MAX_LEN    32      /*user-agent header, eg: "SIP/2.0 (AirM2M Ltd)" */

/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_PRO_PRIM_ID_TAG
{
    IMI_PRO_PRIM_BASE = 0,

    /* follow by IMI_REG_PRIM_ID */
    IMI_PRO_SET_CFG_REQ = 0x0100, //ImiProSetCfgReq
    IMI_PRO_SET_CFG_CNF, //ImiProSetCfgCnf

    IMI_PRO_GET_CFG_FILE_LIST_REQ, //ImiProGetCfgFileListReq
    IMI_PRO_GET_CFG_FILE_LIST_CNF, //ImiProGetCfgFileListCnf

    IMI_PRO_READ_USAGE_SETTING_REQ, //ImiProReadUsageSettingReq
    IMI_PRO_READ_USAGE_SETTING_CNF, //ImiProReadUsageSettingCnf

    IMI_PRO_PRIM_END    = 0x0fff
}IMI_PRO_PRIM_ID;


typedef enum ImiProOperType_Tag
{
    IMI_PRO_OPER_TYPE_DELETE_CFG    = 0,
    IMI_PRO_OPER_TYPE_SET_CFG,
    IMI_PRO_OPER_TYPE_GET_CFG
}
ImiProOperType;

/******************************************************************************
 *****************************************************************************
 * IMI STRUCT
 *****************************************************************************
******************************************************************************/
typedef struct ImiProCfgFile_Tag
{
    CHAR            fileName[IMI_PRO_FILENAME_MAX_LEN + 1];//+1 for '\0'
}
ImiProCfgFile;

/**
 * ICM config struct
*/
typedef struct ImiIcmCfg_Tag
{
    UINT32      imsOLte         : 1;    /*whether IMS over LTE is enable*/
    UINT32      imsOIp          : 1;    /*whether IMS over IP (VoIP), only used for test case, and IPPcscf must configed */
    UINT32      autoReg         : 1;
    UINT32                      : 5;
    UINT32      ipType          : 3;    /*ImsIpType*/
    UINT32      quickReReg      : 1;
    UINT32                      :20;

    UINT8       ipPcscf[IMI_IP_MAX_STR_LEN];    /*whether using IP string? cost to much memory - TBD*/
    CHAR        imsApn[IMI_APN_MAX_STR_LEN];
}ImiIcmCfg;     /* already 144 bytes */

/**
 * UA config struct
*/
typedef struct ImiUaRegCfg_Tag
{
    UINT32      bIpSec        : 1;
    UINT32      bUnSubBfDereg : 1;
    UINT32                    :30;

    UINT16      svcInfoBp;      /* bitmap of: ICfgRegSvcInfoBP */
    UINT16      TregSec;        /* Treg in unit of second, if not cfg, set to 0 */
    UINT16      ueCliPort;      /* UE client port */
    UINT16      pcscfSrvPort;

    UINT8       authType;       /* ICfgRegAuthType */
    UINT8       ipsecEncAlgoBp; /* bitmap of: ICfgRegIpsecEncAlgoBP */
    UINT8       ipsecIntAlgoBp; /* bitmap of: ICfgRegIpsecIntAlgoBP */
    UINT8       rsvd;
}ImiUaRegCfg;   /* 16 bytes */

typedef struct ImiUaCallCfg_Tag
{
    UINT32      localNumberType     : 1;    /* local number type: home-local or geo-local */
    UINT32      bReliable18x        : 1;    /* whether support 18x reliable transmission */
    UINT32      bPrecondition       : 1;    /* whether support precondition */
    UINT32      bAmrNb              : 1;    /* whether support AMR-NB */
    UINT32      bAmrWb              : 1;    /* whether support AMR-WB */
    UINT32      bTelephoneEvent     : 1;    /* whether support Telephone-Event */
    UINT32      sessionRefresher    : 2;    /* session refresher: specified by server, UAC or UAS */

    UINT32      bBandWidthEfficient : 1;    /* whether support BandWidth-efficient */
    UINT32      bOctetAllign        : 1;    /* whether support Octet-allign */
    UINT32      modeChangePeriod    : 2;    /* 1 or 2(default value) */
    UINT32      modeChangeCapa      : 2;    /* 1 or 2(default value) */
    UINT32      bPortMut            : 1;    /* whether support rtp port multiplier */
    UINT32      bSupportPrakNegoSdp : 1;    /* whether support nego SDP(include precondition status) within PRACK request */

    UINT32      bEarlyMediaSupport  : 1;    /* whether support early media */
    UINT32                          :15;

    UINT8       amrNbModesBp;               /* ICfgCallAmrNbModeBP */
    UINT8       rsvd;
    UINT16      amrWbModesBp;               /* ICfgCallAmrWbModeBP */

    UINT8       amrNbPTNum;                 /* AMR-NB payload type number, 102(default value) */
    UINT8       amrWbPTNum;                 /* AMR-WB payload type number, 104(default value) */
    UINT8       dtmf1PTNum;                 /* payload type number of dtmf sample rate 8000, 105(default value) */
    UINT8       dtmf2PTNum;                 /* payload type number of dtmf sample rate 16000, 100(default value) */

    UINT8       pTimeMs;                    /* ptime. default value:20 */
    UINT8       maxPTimeMs;                 /* maxptime. default value:240 */
    UINT16      maxRedunancyTimeMs;         /* maxRed. default value:120[0-65535] */

    UINT8       tqos;                       /* tqos time length in unit of second:5 [0-20] */
    UINT8       tCalls;                     /* tcall time length in unit of second:5[0-255] */
    UINT16      sessionTimer;               /* session time length in unit of second:1800[1800-7200] */

    UINT16      iWaitTermTimer;             /*session term timer*/
    UINT16      iWaitAcceptTimer;           /*mo call session accept timer*/

    UINT16      iWaitAnswerTimer;           /*mt call session answer timer*/
    UINT8       iWaitMoRingTimer;           /*call session mo ring timer*/
    UINT8       iWaitMtRingTimer;           /*call session mt ring timer*/
}ImiUaCallCfg;  /* 28 bytes */

typedef struct ImiUaSmsCfg_Tag
{
    UINT32      rsvd;
}ImiUaSmsCfg;

/*
* sip cfg struct
*/
typedef struct ImiSipCfg_Tag
{
    UINT32      bMergeSameHeader    : 1;  /* whether merge multiLine header to one line when decode*/
    UINT32                          : 31;

    UINT16      T1ms;
    UINT16      T2ms;
    UINT16      T4ms;
    UINT16      Tcsec;

    UINT16      Tdsec;
    UINT16      rsvd0;

    UINT8       tpType;         /* ImsTpType */
    UINT8       tcpConnTryCout; /* must >= 1 */
    UINT16      tcpConnTms;

    CHAR        prodName[IMI_PRODUCT_NAME_MAX_LEN];     /* 32 bytes, end with '\0' */
}ImiSipCfg;   /* 48 bytes */

/*
* transport cfg struct
*/
typedef struct ImiTpCfg_Tag
{
    UINT32      rsvd;
}ImiTpCfg;

/******************************************************************************
 * IMI_PRO_SET_CFG_REQ
 * AT+ECIMSJSON=<filename>,<oper>,<path>,<value>
******************************************************************************/
typedef struct ImiProSetCfgReq_Tag
{
    ImiProCfgFile               jsonFile;

    UINT8                       oper; //ImiProOperType
    UINT8                       rsv[3];

    CHAR                        path[IMI_PRO_PATH_MAX_LEN + 1];//+1 for '\0'
    CHAR                        value[IMI_PRO_VALUE_MAX_LEN + 1];//+1 for '\0'
}
ImiProSetCfgReq;

//IMI_PRO_SET_CFG_CNF
typedef struct ImiProSetCfgCnf_Tag
{
    UINT8                       oper; //ImiProOperType
    UINT8                       resvd;
    UINT16                      cnfLen;//Json cfg length
    CHAR                        *pJsonCfg;//the pointer to cfg params in Json file, this memory is allocated by IMA,
                                          //shall be free by CMS/AT CNF
}
ImiProSetCfgCnf;

/******************************************************************************
 * IMI_PRO_GET_CFG_FILE_LIST_REQ
 * AT+ECIMSJSON=?
******************************************************************************/
typedef ImiEmptySig ImiProGetCfgFileListReq;

//IMI_PRO_GET_CFG_FILE_LIST_CNF
typedef struct ImiProGetCfgFileListCnf_Tag
{
    UINT8                       fileNum; //file number
    UINT8                       rsv[3];
    ImiProCfgFile               jsonFile[IMI_PRO_MAX_FILE_NUM];
}
ImiProGetCfgFileListCnf;

/******************************************************************************
 * IMI_PRO_READ_USAGE_SETTING_REQ
 * AT+ECIMSRUS
******************************************************************************/
typedef ImiEmptySig ImiProReadUsageSettingReq;

//IMI_PRO_READ_USAGE_SETTING_CNF
typedef struct ImiProReadUsageSettingCnf_Tag
{
    ImiIcmCfg                   icmCfg;
    ImiUaRegCfg                 uaRegCfg;
    ImiUaCallCfg                uaCallCfg;
    ImiUaSmsCfg                 uaSmsCfg;
    ImiSipCfg                   sipCfg;
    ImiTpCfg                    tpCfg;
}
ImiProReadUsageSettingCnf;

#endif

