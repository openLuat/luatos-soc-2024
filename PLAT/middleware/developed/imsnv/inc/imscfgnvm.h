#ifndef __IMS_CFG_NVM_H__
#define __IMS_CFG_NVM_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imscfgnvm.h
 Description:    - IMS config nvm file/struct operation
 History:        - 2023/04/10, Original created
******************************************************************************/
#include "imscommtype.h"
#include "osasys.h"

/******************************************************************************
 *****************************************************************************
 * JSON file MARCO/Enum/struct
 *****************************************************************************
******************************************************************************/

/***
 * !!! default IMS config file: imscfg.json, an examplem refer: eg_imscfg.json !!!
**/
#define     IMS_CFG_FILE_NAME       "imscfg.json"

/**
 * JSON KEY STRING define, should not modify
*/
/*****************************************************************************
 * e.g.: "Icm" : {},            value type: OBJ
*****************************************************************************/
#define ICFG_JSON_ICM_KEY_OBJ   "Icm"
/**
 * follow member under: "Icm" - start
*/
    /* e.g.: "ImsOLte":true                 value type: bool, whether IMS is enabled over LTE */
    #define ICFG_JSON_IMSOLTE_KEY_BOOL      "ImsOLte"
    /* e.g.: "ImsOIp":true                  value type: bool, whether IMS is over IP, only used for test */
    #define ICFG_JSON_IMSOIP_KEY_BOOL       "ImsOIp"
    /* e.g.: "IpPcscf":"10.15.100.10"       value type: string, pcscf ip addr, only if "ImsOIp" is set TRUE */
    #define ICFG_JSON_IP_PCSCF_KEY_STR      "IpPcscf"
    /* e.g.: "ImsApn":"ims"                 value type: string                  */
    #define ICFG_JSON_IMSAPN_KEY_STR        "ImsApn"
    /* e.g.: "IpType":2                     value type: number, enum: ImsIpType     */
    #define ICFG_JSON_IPTYPE_KEY_UINT8      "IpType"
    /* e.g.: "AutoReg":true                 value type: bool                    */
    #define ICFG_JSON_AUTOREG_KEY_BOOL      "AutoReg"
    /* e.g.: "QuickReReg":true                 value type: bool                    */
    #define ICFG_JSON_QUICKREREG_KEY_BOOL      "QuickReReg"
    /* e.g.: "CallWaiting":true                 value type: bool                    */
    #define ICFG_JSON_CALLWAITING_KEY_BOOL      "CallWaiting"
/**
 * member under: "Icm" - end
*/
/**
 * ICM config struct
*/
typedef struct _EPAT_ImsIcmCfg_Tag
{
    UINT32      imsOLte         : 1;    /*whether IMS over LTE is enable*/
    UINT32      imsOIp          : 1;    /*whether IMS over IP (VoIP), only used for test case, and IPPcscf must configed */
    UINT32      autoReg         : 1;
    UINT32                      : 5;
    UINT32      ipType          : 3;    /*ImsIpType*/
    UINT32      quickReReg      : 1;
    UINT32      callWaiting     : 1;    /* whether call waiting is enable */
    UINT32                      :19;

    UINT8       ipPcscf[IMS_IP_MAX_STR_LEN];    /*whether using IP string? cost to much memory - TBD*/
    CHAR        imsApn[IMS_APN_MAX_STR_LEN];
}ImsIcmCfg;     /* already 144 bytes */


/**
 * ICM config struct
*/
typedef struct _EPAT_ImsSupSrvCfg_Tag
{
    UINT32      imsUtEnable :1;        /*ut switch, influence on all UT*/
    UINT32      cwTermEnable:1;        /*Call waiting, terminated based*/
    UINT32      cOipTermEnable:1;      /*COIP, terminated based*/
    UINT32      cOirTermEnable:1;      /*COIR, terminated based*/
    UINT32      clipTermEnable:1;      /*ClIP, terminated based*/
    UINT32      clirTermEnable:1;      /*ClIR, terminated based*/
    UINT32      resv0: 26;

    CHAR        xcapRootUri[IMS_USER_NAME_MAX_LEN];
    CHAR        bsfRootUri[IMS_USER_NAME_MAX_LEN];

    UINT16      xcapPort;
    UINT16      bsfPort;

}ImsSupSrvCfg;     /* already 144 bytes */

/*****************************************************************************
 *  e.g.: "Icm" : {},   - end
*****************************************************************************/


/*****************************************************************************
 * e.g.: "UA" : {},             value type: OBJ, - start
*****************************************************************************/
#define ICFG_JSON_UA_KEY_OBJ    "UA"
    /**
     * follow member under: "UA" - start
    */
    /* e.g.: "Reg": {}                  value type: OBJ  --- start */
    #define ICFG_JSON_REG_KEY_OBJ       "Reg"
        /* follow member under: "REG" - start */

        /* e.g.: "UeCliCPort":5060                  value type: number, UINT16          */
        #define ICFG_JSON_UE_CPORT_KEY_UINT16       "UeCliPort"

        /* e.g.: "PcscfSrvPort":5060                value type: number, UINT16          */
        #define ICFG_JSON_PCSCF_SRV_PORT_KEY_UINT16 "PcscfSrvPort"

        /* e.g.: "IpSec":true                       value type: bool                    */
        #define ICFG_JSON_IPSEC_KEY_BOOL            "IpSec"

        /* e.g.: "UnsubBfDereg":false               value type: bool                    */
        #define ICFG_JSON_UNSUB_BF_DEREG_KEY_BOOL   "UnsubBfDereg"

        /* e.g.: "ServiceInfo":3                    value type: number, bitmap, ICfgRegSvcInfoBP */
        #define ICFG_JSON_SVC_INFO_KEY_UINT16       "ServiceInfo"

        /* e.g.: "AuthType":1                       value type: number, enum, ICfgRegAuthType */
        #define ICFG_JSON_AUTH_TYPE_KEY_UINT8       "AuthType"

        /* e.g.: "IpsecEncAlgo":3                   value type: number, bitmap, ICfgRegIpsecEncAlgoBP */
        #define ICFG_JSON_IPSEC_ENC_KEY_UINT8       "IpsecEncAlgo"

        /* e.g.: "IpsecIntAlgo":3                   value type: number, bitmap, ICfgRegIpsecIntAlgoBP */
        #define ICFG_JSON_IPSEC_INT_KEY_UINT8       "IpsecIntAlgo"

        /* e.g.: "TregSec":0                        value type: number, UINT16          */
        #define ICFG_JSON_T_REG_KEY_UINT16          "TregSec"
    typedef struct _EPAT_ImsUaRegCfg_Tag
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
    }ImsUaRegCfg;   /* 16 bytes */
    /* e.g.: "Reg": {}                  value type: OBJ  --- end */


    /* e.g.: "Call": {}                 value type: OBJ  --- start */
    #define ICFG_JSON_CALL_KEY_OBJ      "Call"
        /* follow member under: "Call" - start */

        /* e.g.: "LocalNumType":0                        value type: enum, ICfgCallLocalNumType      */
        #define ICFG_JSON_LOCAL_NUM_TYPE_KEY_UINT8       "LocalNumType"

        /* e.g.: "Reliable18x":true                      value type: bool,       */
        #define ICFG_JSON_RELIABLE_18X_KEY_UINT8         "Reliable18x"

        /* e.g.: "Precondition":true                      value type: bool,       */
        #define ICFG_JSON_PRECONDITION_KEY_UINT8         "Precondition"

        /* e.g.: "SessRefresher":0                       value type: enum, ICfgCallSessRefresher      */
        #define ICFG_JSON_SESS_REFRESHER_TYPE_KEY_UINT8  "SessRefresher"

        /* e.g.: "AmrNb":true                            value type: bool,       */
        #define ICFG_JSON_AMR_NB_KEY_UINT8               "AmrNb"

        /* e.g.: "AmrWb":true                            value type: bool,       */
        #define ICFG_JSON_AMR_WB_KEY_UINT8               "AmrWb"

        /* e.g.: "TelephoneEvnet":true                   value type: bool,       */
        #define ICFG_JSON_TELEPHONE_EVENT_KEY_UINT8      "TelephoneEvent"

        /* e.g.: "AmrNbModes":63                          value type: number, UINT8          */
        #define ICFG_JSON_AMR_NB_MODES_KEY_UINT8         "AmrNbModes"

        /* e.g.: "AmrWbModes":255                        value type: number, UINT16          */
        #define ICFG_JSON_AMR_WB_MODES_KEY_UINT16        "AmrWbModes"

        /* e.g.: "BandwidthEff":true                     value type: bool,       */
        #define ICFG_JSON_BANDWIDTH_EFF_KEY_UINT8        "BandwidthEff"

        /* e.g.: "OctetAllign":true                      value type: bool,       */
        #define ICFG_JSON_OCTET_ALLIGN_KEY_UINT8         "OctetAllign"

        /* e.g.: "ModeChgPeriod":2                       value type: UINT8,       */
        #define ICFG_JSON_MCP_KEY_UINT8                  "ModeChgPeriod"

        /* e.g.: "ModeChgCapa":2                         value type: UINT8,       */
        #define ICFG_JSON_MCC_KEY_UINT8                  "ModeChgCapa"

        /* e.g.: "PortMut":true                          value type: bool,       */
        #define ICFG_JSON_PORT_MUT_KEY_UINT8             "PortMut"

        /* e.g.: "AmrNbPTNum":102                         value type: UINT8,       */
        #define ICFG_JSON_AMR_NB_PT_NUM_KEY_UINT8        "AmrNbPTNum"

        /* e.g.: "AmrWbPTNum":104                         value type: UINT8,       */
        #define ICFG_JSON_AMR_WB_PT_NUM_KEY_UINT8        "AmrWbPTNum"

        /* e.g.: "Dtmf1PTNum":100                         value type: UINT8,       */
        #define ICFG_JSON_DTMF_F1_PT_NUM_KEY_UINT8       "Dtmf1PTNum"

        /* e.g.: "Dtmf2PTNum":105                         value type: UINT8,       */
        #define ICFG_JSON_DTMF_F2_PT_NUM_KEY_UINT8       "Dtmf2PTNum"

        /* e.g.: "Ptime":20                              value type: UINT8,       */
        #define ICFG_JSON_PTIME_KEY_UINT8                "Ptime"

        /* e.g.: "Maxptime":240                          value type: UINT8,       */
        #define ICFG_JSON_MAXPTIME_KEY_UINT8             "Maxptime"

        /* e.g.: "MaxRed":120                            value type: UINT16,       */
        #define ICFG_JSON_MAXRED_KEY_UINT16              "MaxRed"

        /* e.g.: "SessTimer":1800                        value type: number, UINT16          */
        #define ICFG_JSON_SESS_TIMER_KEY_UINT16          "SessTimer"

        /* e.g.: "TCall":5                               value type: UINT8,       */
        #define ICFG_JSON_TCALL_KEY_UINT8                "TCall"

        /* e.g.: "TQos":6                                value type: UINT8,       */
        #define ICFG_JSON_TQOS_KEY_UINT8                 "TQos"

        /* e.g.: "WaitTermTimer":5                       value type: number, UINT16          */
        #define ICFG_JSON_WAITTERM_TIMER_KEY_UINT16      "WaitTermTimer"

        /* e.g.: "WaitAcceptTimer":200                   value type: number, UINT16          */
        #define ICFG_JSON_WAITACCEPT_TIMER_KEY_UINT16    "WaitAcceptTimer"

        /* e.g.: "WaitAnswertTimer":200                    value type: number, UINT16          */
        #define ICFG_JSON_WAITANSWER_TIMER_KEY_UINT16    "WaitAnswertTimer"

        /* e.g.: "WaitRingMoTimer":120                    value type: number, UINT16          */
        #define ICFG_JSON_WAITMORING_TIMER_KEY_UINT16    "WaitMoRingTimer"

        /* e.g.: "WaitRingMtTimer":120                    value type: number, UINT16          */
        #define ICFG_JSON_WAITMTRING_TIMER_KEY_UINT16    "WaitMtRingTimer"

        /* e.g.: "NegoSdpInPrack":false                  value type: bool,       */
        #define ICFG_JSON_NEGO_SDP_IN_PRACK_KEY_UINT8    "NegoSdpInPrack"

        /* e.g.: "EarlyMedia":false                      value type: bool,       */
        #define ICFG_JSON_EARLY_MEDIA_UINT8              "EarlyMedia"

    typedef struct _EPAT_ImsUaCallCfg_Tag
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
    }ImsUaCallCfg;  /* 28 bytes */
    /* e.g.: "Call": {}                 value type: OBJ  --- end */

    /* e.g.: "SMS": {}                  value type: OBJ  --- start */
    #define ICFG_JSON_SMS_KEY_OBJ       "SMS"
        /* e.g.: SMS config param*/

    typedef struct ImsUaSmsCfg_Tag
    {
        UINT32      rsvd;
    }ImsUaSmsCfg;
    /* e.g.: "SMS": {}                  value type: OBJ  --- end */
/*****************************************************************************
 * e.g.: "UA" : {},             value type: OBJ, -- end
*****************************************************************************/


/*****************************************************************************
 * e.g.: "SIP" : {},                value type: OBJ, - start
*****************************************************************************/
#define ICFG_JSON_SIP_KEY_OBJ   "SIP"
/* follow member under: "SIP"       ---- start */
    /* e.g.: "ProductName":"SIP/1.0 (AirM2M Ltd)"    value type: string, max 31 bytes */
    #define ICFG_JSON_PROD_NAME_KEY_STR                 "ProductName"
    /* e.g.: "T1ms":500                                 value type: number, UINT16, T1 in unit of min-second */
    #define ICFG_JSON_T1_MS_KEY_UINT16                  "T1ms"
    /* e.g.: "T2ms":4000                                value type: number, UINT16, T2 in unit of min-second */
    #define ICFG_JSON_T2_MS_KEY_UINT16                  "T2ms"
    /* e.g.: "T4ms":5000                                value type: number, UINT16, T4 in unit of min-second    */
    #define ICFG_JSON_T4_MS_KEY_UINT16                  "T4ms"
    /* e.g.: "TCsec":180                                value type: number, UINT16, Tc in unit of second    */
    #define ICFG_JSON_TC_SEC_KEY_UINT16                 "TCsec"
    /* e.g.: "TDsec":320                                value type: number, UINT16, Td in unit of second    */
    #define ICFG_JSON_TD_SEC_KEY_UINT16                 "TDsec"
    /* e.g.: "TcpConnTms":2000                          value type: number, UINT16, tcp connect timeout val in ms */
    #define ICFG_JSON_TCP_CONN_T_MS_KEY_UINT16          "TcpConnTms"
    /* e.g.: "TcpConnTryCout":1                         value type: number, UINT8, tcp connect try counter */
    #define ICFG_JSON_TCP_CONN_TRY_COUT_KEY_UINT8       "TcpConnTryCout"
    /* e.g.: "TpType":2                                 value type: number, enum, ImsTpType */
    #define ICFG_JSON_TP_TYPE_KEY_UINT8                 "TpType"
    /* e.g.: "MergeSameHeader":false                    value type: bool,       */
    #define ICFG_JSON_MERGE_SAME_HEADER_KEY_UINT8       "MergeSameHeader"

/* sip cfg struct */
typedef struct _EPAT_ImsSipCfg_Tag
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

    CHAR        prodName[IMS_PRODUCT_NAME_MAX_LEN];     /* 32 bytes, end with '\0' */
}ImsSipCfg;   /* 48 bytes */
/*****************************************************************************
 * e.g.: "SIP" : {},                value type: OBJ, -- end
*****************************************************************************/


/*****************************************************************************
 * e.g.: "Transport" : {},          value type: OBJ, - start
*****************************************************************************/
#define ICFG_JSON_TRANSPROT_KEY_OBJ     "Transport"
/* follow member under: "Transport"       ---- start */


/* sip cfg struct */
typedef struct ImsTpCfg_Tag
{
    UINT32      rsvd;
}ImsTpCfg;
/*****************************************************************************
 * e.g.: "Transport" : {},          value type: OBJ, -- end
*****************************************************************************/

/*****************************************************************************
 * e.g.: "OperCfgFile" : {},        value type: OBJ, - start
*****************************************************************************/
#define ICFG_JSON_OPER_CFG_FILE_KEY_OBJ     "OperCfgFile"

/*
 * e.g: this KEY only exist/parse in "imscfg.json" file
 * "OperCfgFile":
 * {
 *   "46000":"imscfg_cmcc.json", //use this to find operator config file name, max len: 64
 *   "46005":"imscfg_ctcc.json",
 *
 *  }
*/
/*****************************************************************************
 * e.g.: "OperCfgFile" : {},        value type: OBJ, - end
*****************************************************************************/

/******************************************************************************
 *****************************************************************************
 * MARCO/Enum
 *****************************************************************************
******************************************************************************/

/**
 * IMS CFG file size limition
*/
#define IMS_CFG_FILE_WARN_SIZE          (4*1024)
#define IMS_CFG_FILE_ERROR_SIZE         (6*1024)       /* file size can't too large, as mem limition */

/**
 * IMS operator config file name len
*/
#define IMS_CFG_FILE_NAME_MAX_LEN       64


/**
 * operator ID, as some operator may have private requirement
*/
typedef enum
{
    ICFG_TEST_SIM_OPER,

    /*[1:9] rsvd for test */

    ICFG_CHA_CMCC_OPER  = 10,
    ICFG_CHA_CUCC_OPER  = 11,
    ICFG_CHA_CTCC_OPER  = 12,

    /**
     * Note, operator ID could only ADD here, can't modify !!
    */

    ICFG_OTHER_OPER     = 0xFFF,

    ICFG_MAX_OPER_ID    = 0xFFFF
}ICfgOperatorId;


/**
 * REG, ServiceInfo, bit0: voice, bit1: sms, 16 bits
*/
typedef enum
{
    ICFG_REG_SVC_INFO_VOICE_BP  = 0,        /* Bit prosition */
    ICFG_REG_SVC_INFO_VOICE     = 0x0001,   /* bit 0 */
    ICFG_REG_SVC_INFO_SMS_BP    = 1,
    ICFG_REG_SVC_INFO_SMS       = 0x0002,   /* bit 1 */

    ICFG_REG_SVC_INFO_MAX       = 0x8000
}ICfgRegSvcInfoBP;

/**
 * REG, AuthType,  bit 0: AKA with GIBA, bit 1: GIBA only, 8 bits
*/
typedef enum
{
    ICFG_REG_AUTH_AKA_GIBA      = 0,
    ICFG_REG_AUTH_GIBA          = 1,
    ICFG_REG_AUTH_TYPE_MAX      = 0xFF
}ICfgRegAuthType;

/**
 * REG, IpsecEncAlgo, bit0: Null, bit1: aes-cbc
*/
typedef enum
{
    ICFG_REG_IPSEC_ENC_NULL_BP  = 0,        /* Bit prosition */
    ICFG_REG_IPSEC_ENC_NULL     = 0x01,
    ICFG_REG_IPSEC_AES_CBC_BP   = 1,
    ICFG_REG_IPSEC_AES_CBC      = 0x02,

    ICFG_REG_IPSEC_ENC_TYPE_MAX = 0x80
}ICfgRegIpsecEncAlgoBP;


/**
 * REG, IpsecIntAlgo, bit0: hmac-md5-96, bit1: hmac-sha-1-96
*/
typedef enum
{
    ICFG_REG_IPSEC_MHAC_MD5_96_BP   = 0,        /* Bit prosition */
    ICFG_REG_IPSEC_MHAC_MD5_96      = 0x01,
    ICFG_REG_IPSEC_MHAC_SHA1_96_BP  = 1,
    ICFG_REG_IPSEC_MHAC_SHA1_96     = 0x02,

    ICFG_REG_IPSEC_INT_TYPE_MAX     = 0x80
}ICfgRegIpsecIntAlgoBP;

typedef enum
{
    ICFG_CALL_HOME_LOCAL_NUMBER      = 0,
    ICFG_CALL_GEO_LOCAL_NUMBER       = 1,
}ICfgCallLocalNumType;

typedef enum
{
    ICFG_CALL_SESSION_REFRESHER_SERVER_SPEC  = 0,
    ICFG_CALL_SESSION_REFRESHER_UAC          = 1,
    ICFG_CALL_SESSION_REFRESHER_UAS          = 2,
}ICfgCallSessRefresher;

typedef enum
{
    ICFG_CALL_MCP_0      = 0,
    ICFG_CALL_MCP_1      = 1,
    ICFG_CALL_MCP_2      = 2,
}ICfgCallModeChangePeriod;

typedef enum
{
    ICFG_CALL_MCC_0      = 0,
    ICFG_CALL_MCC_1      = 1,
    ICFG_CALL_MCC_2      = 2,
}ICfgCallModeChangeCapa;

/**
 * Call, AMR-NB modes, bit0: mode0, bit1: mode1,..., bit7: mode7
*/
typedef enum
{
    ICFG_CALL_AMR_NB_MODE_0      = 0x01,
    ICFG_CALL_AMR_NB_MODE_1      = 0x02,
    ICFG_CALL_AMR_NB_MODE_2      = 0x04,
    ICFG_CALL_AMR_NB_MODE_3      = 0x08,
    ICFG_CALL_AMR_NB_MODE_4      = 0x10,
    ICFG_CALL_AMR_NB_MODE_5      = 0x20,
    ICFG_CALL_AMR_NB_MODE_6      = 0x40,
    ICFG_CALL_AMR_NB_MODE_7      = 0x80,

    ICFG_CALL_AMR_NB_MODE_MAX    = 0x80
}ICfgCallAmrNbModeBP;

/**
 * Call, AMR-WB modes, bit0: mode0, bit1: mode1,..., bit8: mode8
*/
typedef enum
{
    ICFG_CALL_AMR_WB_MODE_0      = 0x0001,
    ICFG_CALL_AMR_WB_MODE_1      = 0x0002,
    ICFG_CALL_AMR_WB_MODE_2      = 0x0004,
    ICFG_CALL_AMR_WB_MODE_3      = 0x0008,
    ICFG_CALL_AMR_WB_MODE_4      = 0x0010,
    ICFG_CALL_AMR_WB_MODE_5      = 0x0020,
    ICFG_CALL_AMR_WB_MODE_6      = 0x0040,
    ICFG_CALL_AMR_WB_MODE_7      = 0x0080,
    ICFG_CALL_AMR_WB_MODE_8      = 0x0100,

    ICFG_CALL_AMR_WB_MODE_MAX    = 0x0100
}ICfgCallAmrWbModeBP;

#define ICFG_CALL_RTP_PAYLOAD_MIN_NUM  96
#define ICFG_CALL_RTP_PAYLOAD_MAX_NUM  127

#define ICFG_CALL_MIN_PTIME  20
#define ICFG_CALL_MAX_PTIME  40

#define ICFG_CALL_MAX_PTIME_MIN_VALUE  240
#define ICFG_CALL_MAX_PTIME_MAX_VALUE  240

#define ICFG_CALL_MAX_RED_MIN_VALUE  0
#define ICFG_CALL_MAX_RED_MAX_VALUE  65535

#define ICFG_CALL_MIN_T_QOS  0
#define ICFG_CALL_MAX_T_QOS  20

#define ICFG_CALL_MIN_T_CALL  0
#define ICFG_CALL_MAX_T_CALL  255

#define ICFG_CALL_MIN_SESSION_TIME  1800
#define ICFG_CALL_MAX_SESSION_TIME  7200

#define ICFG_CALL_MIN_WAITTERM_TIME  2
#define ICFG_CALL_MAX_WAITTERM_TIME  10

#define ICFG_CALL_MIN_WAITACCEPT_TIME  200
#define ICFG_CALL_MAX_WAITACCEPT_TIME  300

#define ICFG_CALL_MIN_WAITANSWER_TIME  200
#define ICFG_CALL_MAX_WAITANSWER_TIME  300

#define ICFG_CALL_MIN_WAITRING_TIME  60
#define ICFG_CALL_MAX_WAITRING_TIME  200


/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

/**
 * default config value
*/
void ImsCfgDefaultIcmCfg(ImsPlmn hplmn, ImsIcmCfg *pIcmCfg);
void ImsCfgDefaultUaRegCfg(ImsPlmn hplmn, ImsUaRegCfg *pUaRegCfg);
void ImsCfgDefaultUaCallCfg(ImsPlmn hplmn, ImsUaCallCfg *pUaCallCfg);
void ImsCfgDefaultSipCfg(ImsPlmn hplmn, ImsSipCfg *pSipCfg);
void ImsCfgDefaultTpCfg(ImsPlmn hplmn, ImsTpCfg *pTpCfg);
UINT16 ImsCfgDefaultOperId(ImsPlmn plmn);

/**
 * NVM read/write API
*/
INT32 ImsCfgReadFile(const CHAR *filename, void **pFileBuf);
void ImsCfgWriteFile(const CHAR *filename, void *pFile, INT32 fileSize);
void ImsCfgRemoveFile(const CHAR *filename);
void ImsCfgFreeFileMem(void **pFileBuf);
BOOL ImsCfgIsFileExist(const CHAR* fileName);
BOOL ImsCfgCreateFile(const CHAR* fileName);
INT32 ImsCfgGetFileSize(const CHAR *filename);

#endif

