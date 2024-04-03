/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    ps_lib_api.h
 * Description:  EC618 opencpu pslibapi header file
 * History:      Rev1.0   2018-12-10
 *
 ****************************************************************************/

#ifndef __PS_LIB_API_H__
#define __PS_LIB_API_H__
#include "cmsis_os2.h"
#include "cms_util.h"
#include "networkmgr.h"
#include "cmidev.h"
#include "cmips.h"
#include "cmisim.h"
#ifdef  FEATURE_IMS_ENABLE
#include "imicc.h"
#include "imireg.h"
#endif
#include "ps_sms_if.h"

/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/
#define EC_NV_DATA_LEN                  96       /* not suitable to put here, -TBD */
#define EC_NV_DATA_IMEI_LEN             32
#define EC_NV_DATA_SN_LEN               32
#define EC_NV_DATA_IMEI_LOCK_LEN        16
#define EC_NV_DATA_SN_LOCK_LEN          16

#define EC_NV_DATA_IMEI_OFFSET          0
#define EC_NV_DATA_SN_OFFSET            32
#define EC_NV_DATA_IMEI_LOCK_OFFSET     64
#define EC_NV_DATA_SN_LOCK_OFFSET       80


#define PS_APN_MAX_SIZE             (CMI_PS_MAX_APN_LEN+1)

#define SMS_MAX_ADDR_LEN                20

/*
 * 1> DCS 7bit, max size is 160, IRA
 * 2> DCS 8bit, max size is 140, HEX string type
 * 3> DCS UCS2, max size is 280, HEX string type
*/
#define SMS_MAX_MSG_LEN                 280

/******************************************************************************
 *****************************************************************************
 * ENUM
 *****************************************************************************
******************************************************************************/



/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

/*
 * APP request PS service
*/
typedef struct AppPsCmiReqData_Tag
{
    /* request input */
    UINT8   sgId;       //PS service group ID: CacSgIdEnum
    BOOL    bImiReq;    //indicated whether is IMI request for IMS

    UINT16  reqPrimId;  //request prim ID.
    UINT16  cnfPrimId;

    UINT16  reqParamLen;
    void    *pReqParam;

    /* confirm output */
    UINT16  cnfRc;      //confirm return code: MtErrorResultCode/SmsErrorResultCode
    UINT16  cnfBufLen;
    void    *pCnfBuf;   //filled in callback API
}AppPsCmiReqData;   //20 bytes



typedef struct CeregGetStateParams_Tag
{
    UINT8   state;          //CmiCeregStateEnum
    BOOL    bRegOngoing;    //if not registered, whether low layer is trying to search/register to a PLMN
    UINT16  rsvd0;

    BOOL    locPresent;
    UINT8   act;            //CmiCeregActEnum
    UINT16  tac;
    UINT32  celId;

    BOOL    rejCausePresent;
    UINT8   causeType;      // only support 0 by now
    UINT16  rejCause;

    BOOL    activeTimePresent;
    UINT8   activeTime;     //T3324, valid when "activeTimePresent" == TRUE

    BOOL    extTauTimePresent;
    UINT8   extPeriodicTau; //extT3412, valid when "extTauTimePresent" == TRUE

    /*
     * Active Time value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  activeTimeS;
    /*
     * extended periodic TAU value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  extPeriodicTauS;

    /*
     * 1> if NW not config the extT3412(extPeriodicTau), should return this T3412 (periodicTauS) value in seconds;
     * 2> else (if NW config extT3412), don't need to return this T3412 (periodicTauS)
    */
    BOOL    tauTimerPresent;
    UINT8   rsvd1;
    UINT16  rsvd2;

    UINT32  periodicTauS;
}CeregGetStateParams;


typedef struct EdrxPtwSetParamsReq_Tag
{
    UINT8   edrxMode; //CmiMmEdrxModeEnum
    UINT8   actType;  //CmiMmEdrxActTypeEnum 0 / 5 only

    UINT8   ptwValuePresent;
    UINT8   edrxValuePresent;

    /*
        WB-S1 mode
        The field contains the PTW value in seconds for WB-S1 mode.The PTW value is used
        as specified in 3GPP TS 23.682 [133a].The PTW value is derived as follows:
        bit
        8 7 6 5  Paging Time Window length
        0 0 0 0  1,28 seconds
        0 0 0 1  2,56 seconds
        0 0 1 0  3,84 seconds
        0 0 1 1  5,12 seconds
        0 1 0 0  6,4 seconds
        0 1 0 1  7,68 seconds
        0 1 1 0  8,96 seconds
        0 1 1 1  10,24 seconds
        1 0 0 0  11,52 seconds
        1 0 0 1  12,8 seconds
        1 0 1 0  14,08 seconds
        1 0 1 1  15,36 seconds
        1 1 0 0  16,64 seconds
        1 1 0 1  17,92 seconds
        1 1 1 0  19,20 seconds
        1 1 1 1  20,48 seconds

     */

    /*
     * <Requested_eDRX_value>: string type; half a byte in a 4 bit format.
     *  The eDRX value refers to bit 4 to 1 of octet 3 of the Extended DRX parameters information element
     *  for S1 mode, available value: 0x01 - 0x0F
     * bit
     * 4 3 2 1  E-UTRAN eDRX cycle length duration
     *
     * 0 0 0 0  5,12 seconds
     * 0 0 0 1  10,24 seconds
     * 0 0 1 0  20,48 seconds
     * 0 0 1 1  40,96 seconds
     * 0 1 0 0  61,44 seconds
     * 0 1 0 1  81,92 seconds
     * 0 1 1 0  102,4 seconds
     * 0 1 1 1  122,88 seconds
     * 1 0 0 0  143,36 seconds
     * 1 0 0 1  163,84 seconds
     * 1 0 1 0  327,68 seconds
     * 1 0 1 1  655,36 seconds
     * 1 1 0 0  1310,72 seconds
     * 1 1 0 1  2621,44 seconds
     * 1 1 1 0  5242,88 seconds
     * 1 1 1 1  10485,76 seconds
    */
    UINT8   reqPtwValue;
    UINT8   reqEdrxValue;
    UINT16  reserved1;

}EdrxPtwSetParamsReq;


typedef struct EdrxPtwGetSettingParams_Tag
{
    UINT8   edrxCfg;    //CmiMmEdrxCfgEnum
    UINT8   actType;    //CmiMmEdrxActTypeEnum 0 - 5 only

    /* if eDRX is disabled, reqEdrxValue should be set to 0x00, and actType should set to 0 */
    UINT8   reqPtwValue;
    UINT8   reqEdrxValue;

    /*
     * for easy reading, also return the "Request EDRX value" in milli-second;
    */
    UINT32  reqEdrxValueMs;
}EdrxPtwGetSettingParams;

#if 0
typedef struct EcCfgSetParamsReq_Tag
{
    /* CCM */
    BOOL    psSoftResetPresent;
    BOOL    bEnablePsSoftReset;

    BOOL    rohcPresent;
    BOOL    bRohc;                  //whether ROHC enabled

    BOOL    ipv6RsForTestSimPresent;
    BOOL    bIpv6RsForTestSim;      //whether enable IPv6 RS procedure for TEST SIM card

    BOOL    tcpTptOptPresent;
    BOOL    bTcpTptOpt;             //whether enable TCP throughput optimization

    BOOL    dataCounterPresent;
    BOOL    bEnableDataCounter;       //whether enable data counter
    UINT16  rsvd1;

    BOOL    ipv6GetPrefixTimePresent;
    BOOL    rsvd2;
    UINT16  ipv6GetPrefixTime;      //the maximum time of getting IPv6 prefix

    /* EMM */
    BOOL    powerLevelPresent;
    UINT8   plmnSearchPowerLevel;   /* range [0..3] */

    BOOL    epcoPresent;
    BOOL    enableEpco;

    /*  useless
    BOOL    dnsIpAddrReadCfgPresent;
    UINT8   dnsIpType;
    UINT8   enableDnsIpAddrRead;
    */

    BOOL    barValuePresent;
    UINT8   t3324MaxValuePresent;
    UINT16  barValueS;              /* User defined SIB14 bar timer value in seconds */

    UINT32  t3324MaxValueS;

    BOOL    attachWithImsiCtrlPresent;
    UINT8   attachWithImsiCtrl;     /* attach with imsi control */
    BOOL    pwrAttachWoEiaPresent;
    BOOL    pwrAttachWoEia;         /* whether or not attach without integrity protected while power on */

    /* ERRC */
    BOOL    dataInactTimerPresent;
    UINT8   dataInactTimerS;        /* DataInactivityTimer-r14, used in CERRC; should > 40s; 0 - just means not use DataInactivityTimer feature */

    BOOL    relaxMonitorPresent;
    UINT8   relaxMonitorDeltaP;     /* range [0..15], value in dB, 0 means relaxed monitoring(36.304, 5.2.4.12) in is not used, used in CERRC */

    BOOL    relVersionPresent;
    UINT8   relVersion;             /* release version, 13-14 */

    BOOL    ueCategoryPresent;
    UINT8   ueCategory;             /* Configure the UE category. */

    BOOL    enableEabPresent;
    BOOL    enableEab;

    BOOL    attachCidPresent;
    UINT8   attachCid;
}EcCfgSetParamsReq;


typedef struct EcCfgGetParamsReq_Tag
{
    /* CCM */
    BOOL    bEnablePsSoftReset;
    BOOL    bRohc;

    BOOL    bIpv6RsForTestSim;
    BOOL    bTcpTptOpt;

    BOOL    bEnableDataCounter;
    UINT8   rsvd1;

    UINT16  ipv6GetPrefixTime;

    /* EMM */
    UINT8   plmnSearchPowerLevel;
    BOOL    enableEpco;
    UINT16  rsvd3;
    //UINT8   dnsIpv4AddrReadCfg;   /* Useless remove */
    //UINT8   dnsIpv6AddrReadCfg;

    UINT32  t3324MaxValue;

    UINT16  barValue;

    BOOL    enableEab;

    UINT8   attachEpsCid;

    UINT8   attachWithImsiCtrl;
    BOOL    pwrAttachWoEia;
    UINT16  rsvd2;

    /* ERRC */
    UINT8   ueCfgDataInactTimer;        /* ERRC data inactivity timer, in seconds */

    UINT8   ueCfgRelaxMonitorDeltaP;    /* relaxed monitor parameter, in DB, 0 - 17 */
    UINT8   relVersion;                 /* release version, 13 - 14 */

    UINT8   ueCategory;
}EcCfgGetParamsReq;
#endif
/******************************************************************************
 * Set ECCFG params struct
******************************************************************************/
typedef CmiDevSetExtCfgReq       EcCfgSetParamsReq;

/******************************************************************************
 *Get ECCFG params struct
******************************************************************************/
typedef  CmiDevGetExtCfgCnf      EcCfgGetParamsReq;


#if 0
typedef struct SrvCellBasicInfoTag{
    UINT16          mcc;
    UINT16          mncWithAddInfo; // if 2-digit MNC type, the 4 MSB bits should set to 'F',
    /* Example:
    *  46000; mcc = 0x0460, mnc = 0xf000
    *  00101; mcc = 0x0001, mnc = 0xf001
    *  46012; mcc = 0x0460, mnc = 0xf012
    *  460123; mcc = 0x0460, mnc = 0x0123
    */

    //DL earfcn (anchor earfcn), range 0 - 262143
    UINT32          earfcn;
    //the 28 bits Cell-Identity in SIB1, range 0 - 268435455
    UINT32          cellId;

    //physical cell ID, range 0 - 503
    UINT16          phyCellId;
    // value in dB, value range: -30 ~ 30
    BOOL            snrPresent;
    INT8            snr;

    //value in units of dBm, value range: -156 ~ -44
    INT16           rsrp;
    //value in units of dB, value range: -34 ~ 25
    INT16           rsrq;
}SrvCellBasicInfo;

typedef struct NCellBasicInfoTag{
    UINT32          earfcn;     //DL earfcn (anchor earfcn), range 0 - 262143

    UINT16          phyCellId;
    UINT16          revd0;

    //value in units of dBm, value range: -156 ~ -44
    INT16           rsrp;
    //value in units of dB, value range: -34 ~ 25
    INT16           rsrq;
}NCellBasicInfo;

#define NCELL_INFO_CELL_NUM     6

typedef struct BasicCellListInfoTag
{
    BOOL                sCellPresent;
    UINT8               nCellNum;

    SrvCellBasicInfo    sCellInfo;
    NCellBasicInfo      nCellList[NCELL_INFO_CELL_NUM];
}BasicCellListInfo;

struct ecbcInfoApiMsg
{
    BasicCellListInfo   *bcInfo;
    osSemaphoreId_t     *sem;
    UINT16              result;
};
#endif

typedef CmiDevGetBasicCellListInfoCnf   BasicCellListInfo;

#define SUPPORT_MAX_FREQ_NUM    8 //== CMI_DEV_SUPPORT_MAX_FREQ_NUM

typedef struct CiotSetFreqParams_Tag
{
    UINT8   mode;       // CmiDevSetFreqModeEnum
    UINT8   cellPresent;// indicate whether phyCellId present
    UINT16  phyCellId;  // phyCell ID, 0 - 503

    UINT8   arfcnNum;   // 0 is not allowed for mode is CMI_DEV_SET_PREFER_FREQ (1);
                        // max number is CMI_DEV_SUPPORT_MAX_FREQ_NUM
    UINT8   reserved0;
    UINT16  reserved1;
    UINT32  lockedArfcn;//locked EARFCN
    UINT32  arfcnList[SUPPORT_MAX_FREQ_NUM];
}CiotSetFreqParams; // total 44 bytes

typedef struct CiotGetFreqParams_Tag
{
    UINT8   mode;       // CmiDevGetFreqModeEnum  3:means UE has set preferred EARFCN list and has locked EARFCN
    UINT8   cellPresent;// indicate whether phyCellId present
    UINT16  phyCellId;  // phyCell ID, 0 - 503

    UINT8   arfcnNum;   // 0 is not allowed for mode is CMI_DEV_SET_PREFER_FREQ (1);
                        // max number is CMI_DEV_SUPPORT_MAX_FREQ_NUM
    UINT8   reserved0;
    UINT16  reserved1;
    UINT32  lockedArfcn;//locked EARFCN
    UINT32  arfcnList[SUPPORT_MAX_FREQ_NUM];
}CiotGetFreqParams; // total 44 bytes


/******************************************************************************
 * Get PS extended status info
******************************************************************************/
typedef enum UeExtStatusType_TAG
{
    UE_EXT_STATUS_ALL   = CMI_DEV_GET_ECSTATUS,
    UE_EXT_STATUS_PHY   = CMI_DEV_GET_ECSTATUS_PHY,
    UE_EXT_STATUS_L2    = CMI_DEV_GET_ECSTATUS_L2,
    UE_EXT_STATUS_ERRC  = CMI_DEV_GET_ECSTATUS_RRC,
    UE_EXT_STATUS_EMM   = CMI_DEV_GET_ECSTATUS_EMM,
    UE_EXT_STATUS_PLMN  = CMI_DEV_GET_ECSTATUS_PLMN,
    UE_EXT_STATUS_ESM   = CMI_DEV_GET_ECSTATUS_ESM,
    UE_EXT_STATUS_CCM   = CMI_DEV_GET_ECSTATUS_CCM,
    UE_EXT_QENG_SCELL   = CMI_DEV_GET_QENG_SCELL,
    UE_EXT_QENG_NCELL   = CMI_DEV_GET_QENG_NCELL
}UeExtStatusType;

typedef CmiDevGetExtStatusCnf   UeExtStatusInfo;

/******************************************************************************
 * Set attach bearer parameter
******************************************************************************/
typedef CmiPsSetAttachedBearerCtxReq    SetAttachBearerParams;

/******************************************************************************
 * Get attach bearer setting parameter
******************************************************************************/
typedef CmiPsGetAttachedBearerCtxCnf    GetAttachBearerSetting;

/******************************************************************************
 * Set UE extended configuration
******************************************************************************/
typedef CmiDevSetExtCfgReq   SetExtCfgParams;

/******************************************************************************
 * Get UE extended configuration
******************************************************************************/
typedef CmiDevGetExtCfgCnf   GetExtCfgSetting;

/******************************************************************************
 * Set Eps Bearer information struct
******************************************************************************/
typedef CmiPsDefineBearerCtxReq       SetPsBearerParams;

/******************************************************************************
 *Get Eps Bearer information struct
******************************************************************************/
typedef  CmiPsGetDefinedBearerCtxCnf  GetPsBearerParams;

/******************************************************************************
 *Act/Deactivate Eps Bearer struct
******************************************************************************/
typedef  CmiPsSetBearerActStateReq   SetEpsBearerStateParams;

/******************************************************************************
 *Set Auth params struct
******************************************************************************/
typedef CmiPsSetDefineAuthCtxReq    SetPsAuthCtxParams;

/******************************************************************************
 *Get Auth params struct
******************************************************************************/
typedef CmiPsGetDefineAuthCtxCnf    GetPsAuthCtxParams;


/******************************************************************************


 *Set WifiScan execute params struct
******************************************************************************/
/* WIFISCAN Params Range */
#define WIFISCAN_0_TIME_VAL_MIN             4000
#define WIFISCAN_0_TIME_VAL_MAX             255000

#define WIFISCAN_1_ROUND_VAL_MIN            1
#define WIFISCAN_1_ROUND_VAL_MAX            3

#define WIFISCAN_2_MAXBSSIDNUM_VAL_MIN      4
#define WIFISCAN_2_MAXBSSIDNUM_VAL_MAX      40

#define WIFISCAN_3_SCANTIMEOUT_VAL_MIN      1
#define WIFISCAN_3_SCANTIMEOUT_VAL_MAX      255

#define WIFISCAN_4_PRIORITY_VAL_MIN         0   //data preferred
#define WIFISCAN_4_PRIORITY_VAL_MAX         1   //wifiscan preferred

#define WIFISCAN_5_CHANNELTIMEOUT_VAL_MIN   100
#define WIFISCAN_5_CHANNELTIMEOUT_VAL_MAX   280

#define WIFISCAN_6_CHANNELCOUNT_VAL_MIN     1
#define WIFISCAN_6_CHANNELCOUNT_VAL_MAX     14

#define WIFISCAN_7_CHANNELID_VAL_MIN        1
#define WIFISCAN_7_CHANNELID_VAL_MAX        14

#define WIFISCAN_MAX_CHANNELID_NUM          14

typedef  CmiDevSetWifiSacnReq  SetWifiScanParams;

/******************************************************************************
 *Get WifiScan result struct
******************************************************************************/
typedef  CmiDevSetWifiScanCnf  GetWifiScanInfo;

/******************************************************************************
 * Set ECSIMCFG params struct
******************************************************************************/
typedef CmiSimSetExtCfgReq       EcSimCfgSetParams;

/******************************************************************************
 *Get ECSIMCFG params struct
******************************************************************************/
typedef  CmiSimGetExtCfgCnf      EcSimCfgGetParams;

/******************************************************************************
 *Set PIN operation request params struct
******************************************************************************/
#define PIN_STR_SIZE (CMI_SIM_MAX_PIN_CODE_LENGTH +1) /*+ 1 for '\0'*/
typedef struct SetPinOperReqParams_Tag
{
    CmiSimPinOper    operMode; //only support verify/unblock/change/enable/disable PIN, don't support query
    UINT8            rsvd1;
    UINT16           rsvd2;
    CHAR             pinStr[PIN_STR_SIZE];/* string type, e.g. "1234",
                                           * Verify PIN, PIN string, number of digits: 4-8
                                           * Change PIN, old PIN string, number of digits: 4-8
                                           * Unblock PIN, PUK string, number of digits: 8
                                           */
    CHAR             newPinStr[PIN_STR_SIZE];/* string type, e.g. "1234"
                                              * Change PIN, new PIN string, number of digits: 4-8
                                              * Unblock PIN, new PIN string, number of digits: 4-8
                                              */
}
SetPinOperReqParams;

/******************************************************************************
 * Get PIN state params
******************************************************************************/
typedef enum GetPinStateType_Tag
{
    QUERY_CPIN, //query cpin as "AT+CPIN?"
    QUERY_SIM_LOCK_STATUS //query SIM lock status referred to "AT+CLCK="SC",2",  that also means query PIN status is enable or not
}
GetPinStateType;

//+CPIN: <code>
typedef enum CpinCode_Tag
{
    CPIN_SIM_NOT_READY, /* NOT READY, maybe the SIM is not inserted or failure */
    CPIN_READY,   /* READY, MT is not pending for any password, SIM is ready */
    CPIN_SIM_PIN, /* SIM PIN, MT is waiting for SIM PIN to be given */
    CPIN_SIM_PUK, /* SIM PUK, MT is waiting for SIM PUK to be given */
    CPIN_SIM_UNKNOWN /* unknown CPIN code */
}
CpinCode;

//+CLCK: <status>
typedef enum FacSimLockStatus_Tag
{
    SIM_LOCK_STATUS_NOT_ACTIVE, //SIM lock is not active, that means PIN is disable
    SIM_LOCK_STATUS_ACTIVE, //SIM lock is active, that means PIN is enable
    SIM_LOCK_STATUS_UNKNOWN
}
FacSimLockStatus;

typedef struct GetPinStateCnfParams_Tag
{
    CpinCode            cpinCode; //+CPIN: <code>
    FacSimLockStatus    simLockStatus; //+CLCK: <status>
    UINT16              rsvd;
}
GetPinStateCnfParams;

/******************************************************************************
 *Manual PLMN Search result struct
******************************************************************************/
#define PLMN_STR_MAX_LENGTH 7

typedef struct PlmnSearchInfo_Tag
{
    UINT8  plmn[PLMN_STR_MAX_LENGTH];
    UINT8  plmnState;   //CmiMmPlmnStateEnum, cops <stat>
    UINT8  longPlmn[CMI_MM_STR_PLMN_MAX_LENGTH]; // end with '\0'
    UINT8  shortPlmn[CMI_MM_SHORT_STR_PLMN_MAX_LENGTH]; // end with '\0'
    UINT8  act; //CmiCregActEnum, cops <act>
}PlmnSearchInfo;   //size = (7+1+32+8) = 48

typedef struct ManualPlmnSearchInfo_Tag
{
    UINT8           plmnNum;
    PlmnSearchInfo  plmnList[CMI_MM_PLMN_SEARCH_NUM];
}ManualPlmnSearchInfo;

/******************************************************************************
 *Current Operator Info struct
******************************************************************************/
typedef struct GetCurrentOperatorInfo_Tag
{
    UINT8  plmn[PLMN_STR_MAX_LENGTH]; // end with '\0'
    UINT8  longPlmn[CMI_MM_STR_PLMN_MAX_LENGTH]; // end with '\0'
    UINT8  shortPlmn[CMI_MM_SHORT_STR_PLMN_MAX_LENGTH]; // end with '\0'
}GetCurrentOperatorInfo;


typedef CmiPsSetTrafficIdleMonitorReq   EcSclkExSetParamsReq;

typedef CmiPsGetTrafficIdleMonitorCnf   EcSclkExGetParamsReq;

typedef CmiSimImsAuthReq    EcSimAuthReqParams;

typedef CmiSimImsAuthCnf    EcSimAuthRspParams;


/******************************************************************************
 *Current Tx Power setting struct
******************************************************************************/
#define TXPOWER_MAX_POWER_VAL_MIN           -45
#define TXPOWER_MAX_POWER_VAL_MAX           23

#define TXPOWER_MIN_POWER_VAL_MIN           -45
#define TXPOWER_MIN_POWER_VAL_MAX           23

#define TXPOWER_FIXED_POWER_VAL_MIN         -45
#define TXPOWER_FIXED_POWER_VAL_MAX         23

typedef CmiDevSetTxPowerReq   TxPowerSettingReq;

/******************************************************************************
 *Set bar cell Params
******************************************************************************/
#define ECBARCELL_0_EARFCN_VAL_MIN          (1)
#define ECBARCELL_0_EARFCN_VAL_MAX          (262143)    //max support maxEarfcn2

#define ECBARCELL_1_PCI_VAL_MIN             (0)
#define ECBARCELL_1_PCI_VAL_MAX             (503)       //physcellid (0..503)

#define ECBARCELL_2_BAR_TIME_VAL_MIN        (0)         //unbar cell
#define ECBARCELL_2_BAR_TIME_VAL_MAX        (65535)     //bar cell inifinity

typedef CmiDevSetBarCellReq   SetBarCellParamsReq;

#ifdef  FEATURE_IMS_ENABLE
/******************************************************************************
 *Set CIREG URC reporting mode
******************************************************************************/
typedef enum CiregRptModeTag
{
    CIREG_RPT_DISABLE           = 0,//disable reporting
    CIREG_RPT_ENABLE            = 1,//enable reporting (parameter <reg_info>)
    CIREG_RPT_ENABLE_EXTENDED   = 2,//enable extended reporting (parameters <reg_info> and <ext_info>)
    CIREG_RPT_UNKNOWN
}
CiregRptMode;

typedef struct CiregExtInfoTag
{
    UINT32  rtpVoice    : 1;    /* 0 - voice over PS not avaiable, 1 - voice over PS avaiable */
    UINT32  rtpText     : 1;    /* not support */
    UINT32  smsIms      : 1;    /* 0 - SMS can't use/over IMS, 1 - SMS can use/over IMS */
    UINT32  rtpVideo    : 1;    /* not support */
    UINT32              :16;    /* rsvd for 3GPP */
    UINT32              :12;    /* rsvd for user */
}
CiregExtInfo;

typedef ImiCiregRegInfo CiregRegInfo;

typedef ImiCcListCurrCallCnf ListCurrCallInfo;


#endif

#ifdef FEATURE_SMS_API_ENABLE
typedef enum SmsFormatModeTag
{
    SMS_FORMAT_PDU_MODE = 0,
    SMS_FORMAT_TXT_MODE = 1
}SmsFormatMode;

typedef enum SmsMessageTypeTag
{
    SMS_TYPE_DELIVER = 0,       /* SMS-Deliver PDU */
    SMS_TYPE_DELIVER_REPORT,    /* SMS-Deliver Report PDU */
    SMS_TYPE_STATUS_REPORT,     /* SMS-Status Report PDU */
    SMS_TYPE_CB_ETWS_CMAS,      /* CB SMS */
    SMS_TYPE_SUBMIT,            /* SMS-SUBMIT */
    SMS_TYPE_COMMAND,           /* SMS-COMMAND */
    SMS_TYPE_RESERVE
}SmsMessageType;

typedef enum SmsMsgCodingTypeTag
{
    SMS_MSG_CODING_DEFAULT_7BIT = 0x00,
    SMS_MSG_CODING_8BIT         = 0x01,
    SMS_MSG_CODING_UCS2         = 0x02,
}SmsMsgCodingType;

typedef struct SmsInfoTag
{
    UINT8 smsType;                          /* enum SmsMessageType */
    UINT8 addrStr[SMS_MAX_ADDR_LEN + 1];    /* originating or destination address string */
    UINT8 msgCoding;                        /* enum SmsMsgCodingType */
    UINT8 msgStr[SMS_MAX_MSG_LEN + 1];      /* message body buffer text/pdu string */
}SmsInfo;

typedef enum SmsTypeOfAddressTag
{
    SMS_TOA_NUMBER_RESTRICTED       = 0x80,   /* 128, Unknown type, unknown number format */
    SMS_TOA_NUMBER_UNKNOWN          = 0x81,   /* 129, Unknown type, IDSN format number */
    SMS_TOA_NUMBER_INTERNATIONAL    = 0x91,   /* 145, International number type, ISDN format */
    SMS_TOA_NUMBER_NATIONAL         = 0xA1,   /* 161, National number type, IDSN format */
    SMS_TOA_NUMBER_NETWORK_SPECIFIC = 0xB1,   /* 177, Network specific number, ISDN format */
    SMS_TOA_NUMBER_DEDICATED        = 0xC1,   /* 193, Subscriber number, ISDN format */
    SMS_TOA_NUMBER_EXTENSION        = 0xF1,   /* 241, extension, ISDN format */
    SMS_TOA_NUMBER_INVALID
}SmsTypeOfAddress;

typedef enum SmsStoreMemTypeTag
{
    SMS_STOR_MEM_TYPE_ME   = 1, /* ME message storage */
    SMS_STOR_MEM_TYPE_SM   = 2, /* (U)SIM message storage */
    SMS_STOR_MEM_TYPE_BM   = 3, /* broadcast message storage  */
    SMS_STOR_MEM_TYPE_MT   = 4, /* Any of the storage associated whit ME */
    SMS_STOR_MEM_TYPE_TA   = 5, /* TA message storage */
    SMS_STOR_MEM_TYPE_SR   = 6, /* status report storage */
    SMS_STOR_MEM_TYPE_END
}SmsStoreMemType;

typedef enum SmsRecStorStatusTag
{
    SMS_STOR_STATUS_REC_UNREAD = 0, /* Received unread message, i.e new message */
    SMS_STOR_STATUS_REC_READ   = 1, /* Received read message */
    SMS_STOR_STATUS_STO_UNSENT = 2, /* Stored unsent message only applicable to SMs */
    SMS_STOR_STATUS_STO_SENT   = 3, /* Stored sent message only applicable to SMs */
    SMS_STOR_STATUS_ALL        = 4, /* All message, only applicable to +CGML command */
    SMS_STOR_STATUS_END
}SmsRecStorStatus;

typedef enum SmsDelFlagTag
{
    SMS_DEL_FLAG_INDEX    = 0,  /* delete the message specified in index*/
    SMS_DEL_FLAG_STATUS_1 = 1,  /* delete all Read message from the message storage */
    SMS_DEL_FLAG_STATUS_2 = 2,  /* delete all Read message and sent mobile originated message */
    SMS_DEL_FLAG_STATUS_3 = 3,  /* delete all read message, sent MO message and unsent mobile originated message */
    SMS_DEL_FLAG_ALL      = 4,  /* delete all message */
    SMS_DEL_FLAG_UNKNOWN
}SmsDelFlag;

#endif

/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

//void appCheckTcpipReady(void);
//void psSyncProcCmiCnf(const SignalBuf *cnfSignalPtr);

//CmsRetId initPsCmiReqMapList(void);
CmsRetId appGetNetInfoSync(UINT32 cid, NmAtiNetifInfo *result );
CmsRetId appGetImsiNumSync(CHAR *imsi);
CmsRetId appGetIccidNumSync(CHAR *iccid);
CmsRetId appGetImeiNumSync(CHAR *imei);
CmsRetId appGetCeregStateSync(CeregGetStateParams *pCeregGetStateParams);
CmsRetId appSetEdrxPtwSettingSync(EdrxPtwSetParamsReq *pEdrxPtwSetParams);
CmsRetId appGetEdrxPtwSettingSync(EdrxPtwGetSettingParams *pEdrxPtwGetSettingParams);
CmsRetId appSetEcCfgSettingSync(EcCfgSetParamsReq *pEcCfgSetParams);
CmsRetId appGetEcCfgSettingSync(EcCfgGetParamsReq *pEcCfgGetParams);
CmsRetId appGetAPNSettingSync(UINT8 cid, UINT8 *pApn);
CmsRetId appCheckSystemTimeSync(void);
CmsRetId appGetSystemTimeSecsSync(time_t *time);
CmsRetId appGetSystemTimeNitzSecsSync(time_t *time);
CmsRetId appGetSystemTimeUtcSync(utc_timer_value_t *time);
CmsRetId appSetSystemTimeUtcSync(UINT32 time1, UINT32 time2, UINT32 time3);
//CmsRetId appGetActedCidSync(UINT8 *cid, UINT8 *num);

/**
  \fn          appSetSimLogicalChannelOpenSync
  \brief       Send cmi request to open SIM logical channel
  \param[out]  *sessionID: Pointer to a new logical channel number returned by SIM
  \param[in]   *dfName: Pointer to DFname selected on the new logical channel
  \returns     CMS_RET_SUCC: success
  \            CMS_SIM_NOT_INSERT: SIM not inserted
  \            CMS_OPER_NOT_SUPPROT: operation not supported
*/
//CmsRetId appSetSimLogicalChannelOpenSync(UINT8 *sessionID, UINT8 *dfName);

/**
  \fn          appSetSimLogicalChannelCloseSync
  \brief       Send cmi request to close SIM logical channel
  \param[in]   sessionID: the logical channel number to be closed
  \returns     CMS_RET_SUCC---success
  \            CMS_SIM_NOT_INSERT: SIM not inserted
  \            CMS_INVALID_PARAM: input invalid parameters
*/
//CmsRetId appSetSimLogicalChannelCloseSync(UINT8 sessionID);

/**
  \fn          appSetSimGenLogicalChannelAccessSync
  \brief       Send cmi request to get generic SIM logical channel access
  \param[in]   sessionID: the logical channel number
  \param[in]   *command: Pointer to command apdu, HEX string
  \param[in]   cmdLen: the length of command apdu, max value is CMI_SIM_MAX_CMD_APDU_LEN * 2 (522)
  \param[out]  *response: Pointer to response apdu, HEX string
  \param[out]  respLen: the length of command apdu, max value is 4KB
  \returns     CmsRetId
*/
//CmsRetId appSetSimGenLogicalChannelAccessSync(UINT8 sessionID, UINT8 *command, UINT16 cmdLen,
//                                                UINT8 *response, UINT16 *respLen);

/**
  \fn          appSetRestrictedSimAccessSync
  \brief       Send cmi request to get generic SIM access
  \param[in]   *pCmdParam: Pointer to command parameters
  \param[out]  *pRspParam: Pointer to response parameters
  \returns     CMS_RET_SUCC---success
  \            CMS_SIM_NOT_INSERT: SIM not inserted
  \            CMS_INVALID_PARAM: input invalid parameters
*/
//CmsRetId appSetRestrictedSimAccessSync(CrsmCmdParam *pCmdParam, CrsmRspParam *pRspParam);


CmsRetId appSetCFUN(UINT8 fun);
CmsRetId appGetCFUN(UINT8 *pOutCfun);

//CmsRetId appSetBootCFUNMode(UINT8 mode);
//UINT8 appGetBootCFUNMode(void);
//CmsRetId appGetSignalInfoSync(UINT8 *csq, INT8 *snr, INT8 *rsrp);
CHAR* appGetUeVersionInfo(void);
//void drvSetPSToWakeup(void);
BOOL appSetImeiNumSync(CHAR* imei);
BOOL appGetSNNumSync(CHAR* sn);
BOOL appSetSNNumSync(CHAR* sn, UINT8 len);

BOOL appGetImeiLockSync(CHAR* imeiLock);
BOOL appSetImeiLockSync(CHAR* imeiLock);
BOOL appGetSNLockSync(CHAR* snLock);
BOOL appSetSNLockSync(CHAR* snLock);
BOOL appSetEcNVDataLockCleanSync(void);
BOOL appSetSnImeiLockCleanSync(void);

CmsRetId appSetBandModeSync(UINT8 bandNum,  UINT8 *orderBand);
CmsRetId appGetBandModeSync(UINT8 *bandNum,  UINT8 *orderBand);
CmsRetId appGetSupportedBandModeSync(UINT8 *bandNum,  UINT8 *orderBand);

CmsRetId appGetECBCInfoSync(BasicCellListInfo *bcListInfo);

CmsRetId appSetCiotFreqSync(CiotSetFreqParams *pCiotFreqParams);
CmsRetId appGetCiotFreqSync(CiotGetFreqParams *pCiotFreqParams);

CmsRetId appGetPSMModeSync(UINT8 *pMode);

CmsRetId appGetUeExtStatusInfoSync(UeExtStatusType statusType, UeExtStatusInfo *pStatusInfo);


CmsRetId appSetAttachBearerSync(SetAttachBearerParams *pAttachBearerParams);
CmsRetId appGetAttachBearerSettingSync(GetAttachBearerSetting *pAttachBearerSettingParams);

CmsRetId appGetCsconStateSync(UINT8 *pCsconState);

CmsRetId appGetWifiScanInfo(SetWifiScanParams *pWifiScanParams, GetWifiScanInfo *pWifiScanInfo);


/**
  \fn          CmsRetId appSetSIMHotSwapNotify(BOOL bSimPlugIn)
  \brief       Send fast signal to CMS task, then trigger cmi request to notify PS the SIM card plugged in or out
  \param[in]   BOOL bSimPlugIn, indicated whether the SIM card plugged in (TRUE) or out (FALSE)
  \returns     void
  \NOTE:       this API called by app tasks or ISR (interrrupt service routine)
*/
void appSetSIMHotSwapNotify(BOOL bSimPlugIn);

CmsRetId appTriggerTau(UINT8 epsUpdateType);

/**
  \fn          CmsRetId appTriggerRel()
  \brief       Send cmi request to trigger RRC local Release
  \returns     CmsRetId CMS_RET_SUCC: RRC connection local release is triggered
               CMS_FAIL: RRC connection local release is not triggered
*/
CmsRetId appTriggerRel();

/**
  \fn          appSetECSIMCFGSync
  \brief       Send cmi request to set ECSIMCFG
  \param[in]   EcSimCfgSetParams *pEcSimCfgSetParams, the pointer to EcSimCfgSetParams
  \returns     CmsRetId
  \NOTE:
*/
CmsRetId appSetECSIMCFGSync(EcSimCfgSetParams *pEcSimCfgSetParams);
/**
  \fn          CmsRetId appGetECSIMCFGSync
  \brief       Send cmi request to get ECSIMCFG information
  \param[out]  EcSimCfgGetParams *pEcSimCfgGetParams, the pointer to the EcSimCfgGetParams
  \returns     CmsRetId
*/
CmsRetId appGetECSIMCFGSync(EcSimCfgGetParams *pEcSimCfgGetParams);

/**
  \fn          appSetPinOperationSync
  \brief       Send cmi request to operate SIM PIN.
  \param[in]   SetPinOperReqParams *pPinOperReqParams, the pointer to SetPinOperReqParams
  \returns     CmsRetId
  \NOTE:       This API implements PIN operation feature:
               1 Enable & disable PIN //AT+CLCK=<fac>,<mode>[,<passwd>]
               2 Verify & unblock PIN //AT+CPIN=<pin>[,<new pin>]
               3 Change PIN //AT+CPWD=<fac>,<oldpwd>,<newpwd>
*/
CmsRetId appSetPinOperationSync(SetPinOperReqParams *pPinOperReqParams);

/**
  \fn          CmsRetId appGetPINStateSync
  \brief       Send cmi request to get CPIN state or SIM lock status depend on type input
  \param[in]   GetPinStateType type, get PIN state type referred to "AT+CPIN?" or "AT+CLCK="SC",2"
  \param[out]  GetPinStateCnfParams *pGetPinStateCnfParams, the pointer to the GetPinStateCnfParams
  \returns     CmsRetId
*/
CmsRetId appGetPINStateSync(GetPinStateType type, GetPinStateCnfParams *pGetPinStateCnfParams);

/**
  \fn          appSetSimCCHOSync
  \brief       Send cmi request to open SIM logical channel
  \param[in]   *pDfName: Pointer to DFname selected on the new logical channel, hex string, max length is CMI_SIM_MAX_AID_LEN(16 bytes)
  \param[out]  *pSessionId: Pointer to store the result of a new logical channel number returned by SIM
  \returns     CmsRetId
  \NOTE:       refer to AT+CCHO
*/
CmsRetId appSetSimCCHOSync(UINT8 *pDfName, UINT8 *pSessionId);

/**
  \fn          appSetSimCCHCSync
  \brief       Send cmi request to close SIM logical channel
  \param[in]   sessionId: the logical channel number to be closed
  \returns     CmsRetId
  \NOTE:       refer to AT+CCHC
*/
CmsRetId appSetSimCCHCSync(UINT8 sessionId);

/**
  \fn          appSetSimCGLASync
  \brief       Send cmi request to get generic SIM logical channel access
  \param[in]   sessionID: the logical channel number
  \param[in]   cmdApduStrLen: the length of cmdApduStr, max value is CMI_SIM_MAX_CMD_APDU_LEN * 2 (522)
  \param[in]   *cmdApduStr: Pointer to command apdu, HEX string
  \param[in]   UINT16 rspApduBufSize, the buffer size for response apdu string, shall evaluate the max legnth of response APDU hex string
  \param[out]  *rspApduStrLen: Pointer to store the result of the length of rspApduStr, max value allowed is 4K, it's depend on the special card application
  \param[out]  *rspApduStr: Pointer to store the result of response apdu, HEX string
  \returns     CmsRetId
  \NOTE:       refer to AT+CGLA
*/
CmsRetId appSetSimCGLASync(UINT8 sessionId,
                                  UINT16 cmdApduStrLen,
                                  UINT8 *cmdApduStr,
                                  UINT16 rspApduBufSize,
                                  UINT16 *rspApduStrLen,
                                  UINT8 *rspApduStr);

/**
  \fn          appSetEcSIMSleepSync
  \brief       set UE to allow SIM card sleep (power off SIM) or not (power on SIM) for appSetCSIMSync(AT+CSIM) and appSetRestrictedSimAccessSync(AT+CRSM)
  \param[in]   BOOL bAllowSleep, indicated whether allow SIM sleep or not
  \returns     CmsRetId
  \NOTE:       refer: AT+ECSIMSLEEP,
  \            shall set SIM sleep not allowed (power on SIM) before use appSetCSIMSync(AT+CSIM) or appSetRestrictedSimAccessSync(AT+CRSM),
  \            then set SIM sleep allowed (power off SIM) to save power after finish if required.
*/
CmsRetId appSetEcSIMSleepSync(BOOL bAllowSleep);

/**
  \fn          appSetCSIMSync
  \brief       Send cmi request to transmit APDU command to the SIM and get APDU response from the SIM
  \param[in]   UINT16 cmdApduStrLen, the length of command APDU string
  \param[in]   UINT8 *cmdApduStr, the pointor to the command APDU, hex string
  \param[in]   UINT16 rspApduBufSize, the buffer size for response APDU, shall evaluate the max legnth of response APDU hex string
  \param[out]  UINT16 *rspApduStrLen, the pointor to the length of response APDU string, shall less than rspApduBufSize
  \param[out]  UINT8 *rspApduStr, the pointor to the response APDU, hex string
  \returns     CmsRetId
  \NOTE:       refer: AT+CSIM
  \            shall set SIM sleep not allowed (power on SIM) by appSetEcSIMSleepSync first if enable SIM power save,
  \            then set SIM sleep allowed (power off SIM) to save power after finish if required.
*/
CmsRetId appSetCSIMSync(UINT16 cmdApduStrLen,
                                UINT8 *cmdApduStr,
                                UINT16 rspApduBufSize,
                                UINT16 *rspApduStrLen,
                                UINT8 *rspApduStr);

/**
  \fn          CmsRetId appManualPlmnSearch
  \brief       Send cmi request to perform manual PLMN search
  \param[in]   uint32 gardTimer (second)
  \param[out]  ManualPlmnSearchInfo *pManualPlmnSearchInfo
  \returns     CmsRetId
*/
CmsRetId appManualPlmnSearch(UINT32 gardTimer, ManualPlmnSearchInfo *pManualPlmnSearchInfo);

/**
  \fn          CmsRetId appManualPlmnSelect
  \brief       Send cmi request to set Manual PLMN register
  \param[in]   UINT8 mode, CHAR *pPlmnStr
  \returns     CmsRetId
*/
CmsRetId appManualPlmnSelect(UINT8 mode, CHAR *pPlmnStr);

/**
  \fn          CmsRetId appAutoPlmnSelect
  \brief       Send cmi request to perform auto PLMN search
  \returns     CmsRetId
*/
CmsRetId appAutoPlmnSelect();

/**
  \fn          CmsRetId appGetCurrentOperatorInfo
  \brief       Send cmi request to get the current operator info
  \param[out]  GetCurrentOperatorInfo *pOperInfo
  \returns     CmsRetId
*/
CmsRetId appGetCurrentOperatorInfo(GetCurrentOperatorInfo *pOperInfo);

/**
  \fn          CmsRetId appSetPSMSetting(UINT8 psmMode, UINT32 tauTime, UINT32 activeTime)
  \brief       Send cmi request to set PSM setting information
  \param[out]  psmMode     Value to input psm mode--psmmode with the Enum "CmiMmPsmReqModeEnum"
               \CMI_MM_DISABLE_PSM(0)/CMI_MM_ENABLE_PSM(1)/CMI_MM_DISCARD_PSM(2)
  \param[out]  tauTimeS     Value to input TAU time(unit: S)---related to T3412
  \param[out]  activeTimeS  Value to input active time(unit: S)---related to T3324
  \returns     CmsRetId
*/
CmsRetId appSetPSMSetting(UINT8 psmMode, UINT32 tauTimeS, UINT32 activeTimeS);

/**
  \fn          CmsRetId appGetPSMSetting(UINT8 *psmmode, UINT32 *tauTime, UINT32 *activeTime)
  \brief       Send cmi request to get PSM setting information
  \param[out]  *psmmode     Pointer to store the result mode--psmmode with the Enum "CmiMmPsmReqModeEnum"
               \CMI_MM_DISABLE_PSM(0)/CMI_MM_ENABLE_PSM(1)/CMI_MM_DISCARD_PSM(2)
  \param[out]  *tauTimeS     Pointer to store the result TAU time(unit:S)---related to T3412
  \param[out]  *activeTimeS  Pointer to store the result active time(unit:S)---related to T3324
  \returns     CmsRetId
*/
CmsRetId appGetPSMSetting(UINT8 *psmMode, UINT32 *tauTimeS, UINT32 *activeTimeS);


/**
  \fn          CmsRetId appSetECSCLKEXSync(EcSclkExSetParamsReq *pEcSclkExSetParamsInfo)
  \brief       Send cmi request to set ECSCLKEX setting information
  \param[in]   EcSclkExSetParamsReq *pEcSclkExSetParamsInfo, the pointer to the EcSclkExSetParamsReq
  \returns     CmsRetId
*/
CmsRetId appSetECSCLKEXSync(EcSclkExSetParamsReq *pEcSclkExSetParamsInfo);


/**
  \fn           CmsRetId appGetECSCLKEXSync(TrafficIdleMonitorInfo *pTrafficIdleMonitorInfo)
  \brief        Request PS to get traffic idle monitor Info of AT+ECSCLKEX?
  \param[out]   pTrafficIdleMonitorInfo     Pointer to store the result of traffic idle monitor info
  \returns      CmsRetId
*/
CmsRetId appGetECSCLKEXSync(EcSclkExGetParamsReq *pEcSclkExGetParamsInfo);


/**
  \fn          appSetEpsBeaerStateSync
  \brief       activate or deactivate eps bearer, such as  AT command AT+CGACT
  \param[in]   pSetEpsStateParams:
                            cid   : which cid to be activate or deactivate
                            state : 1--activate; 0 - deactivate

  \returns     CmsRetId
  \NOTE:       1.this api  can activate/deactivate the eps bearer  as the same effect with AT +CGACT
               2.this is block api and max guard time is 42 seconds,the app layer task should estimate whether can cover it
*/
CmsRetId appSetEpsBeaerStateSync(SetEpsBearerStateParams *pSetEpsStateParams);

/**
  \fn           CmsRetId appGetSignalQualitySync(UINT8 *csq, INT8 *snr, INT8 *rsrp, UINT *rsrq)
  \brief        Get signal information
  \param[out]   *csq Pointer to signal info csq
                * CSQ mapping with RSSI
                *<rssi>: integer type
                * 0        -113 dBm or less
                * 1        -111 dBm
                * 2...30   -109... -53 dBm
                * 31       -51 dBm or greater
                * 99       not known or not detectable
  \param[out]   *snr Pointer to signal info snr(value in dB, value range: -20 ~ 40);
  \param[out]   *rsrp Pointer to signal info rsrp(value range: -17 ~ 97, 127);
                * 1> AS extended the RSRP value in: TS 36.133-v14.5.0, Table 9.1.4-1
                *   -17 rsrp < -156 dBm
                *   -16 -156 dBm <= rsrp < -155 dBm
                *    ...
                *   -3 -143 dBm <= rsrp < -142 dBm
                *   -2 -142 dBm <= rsrp < -141 dBm
                *   -1 -141 dBm <= rsrp < -140 dBm
                *    0 rsrp < -140 dBm
                *    1 -140 dBm <= rsrp < -139 dBm
                *    2 -139 dBm <= rsrp < -138 dBm
                *    ...
                *    95 -46 dBm <= rsrp < -45 dBm
                *    96 -45 dBm <= rsrp < -44 dBm
                *    97 -44 dBm <= rsrp
                * 2> If not valid, set to 127
\param[out]   *rsrq Pointer to signal info rsrq(value range: -30 ~ 46, 127);
                * 1> AS extended the RSRQ value in TS 36.133-v14.5.0, Table 9.1.7-1/Table 9.1.24-1
                *   -30 rsrq < -34 dB
                *   -29 -34 dB <= rsrq < -33.5 dB
                *   ...
                *   -2 -20.5 dB <= rsrq < -20 dB
                *   -1 -20 dB <= rsrq < -19.5 dB
                *   0 rsrq < -19.5 dB
                *   1 -19.5 dB <= rsrq < -19 dB
                *   2 -19 dB <= rsrq < -18.5 dB
                *   ...
                *   32 -4 dB <= rsrq < -3.5 dB
                *   33 -3.5 dB <= rsrq < -3 dB
                *   34 -3 dB <= rsrq
                *   35 -3 dB <= rsrq < -2.5 dB
                *   36 -2.5 dB <= rsrq < -2
                *   ...
                *   45 2 dB <= rsrq < 2.5 dB
                *   46 2.5 dB <= rsrq
                * 2> If not valid, set to 127
  \returns     CmsRetId
*/
CmsRetId appGetSignalQualitySync(UINT8 *csq, INT8 *snr, INT8 *rsrp, INT8 *rsrq);

/**
  \fn          appSIMAuthSync
  \brief       Send cmi request to execute authentication on SIM
  \param[in]   EcSimAuthReqParams *pEcSimAuthReqParams, the pointer to EcSimAuthReqParams
  \param[out]  EcSimAuthRspParams *pEcSimAuthRspParams, the pointer to EcSimAuthRspParams
  \returns     CmsRetId
  \NOTE:
*/
CmsRetId appSIMAuthSync(EcSimAuthReqParams *pEcSimAuthReqParams, EcSimAuthRspParams *pEcSimAuthRspParams);

/**
  \fn          appSetAuthParamSync
  \brief       Send define auth ctx req to PS to set auth parameters
  \param[in]   SetPsAuthCtxParams *pSetAuthCtxParams
  \returns     CmsRetId
  \NOTE:       this api can settting the auth parameters as the same effect with AT+CGAUTH;
            1> this api only can get one Eps beaer information every function callback, and the cid must a valid cid
            2>: The BR correspond with cid must be defined and not dedicated BR;
            3>: If delAuthInfo is 1, means need to delete auth parameters;
*/
CmsRetId appSetAuthParamSync(SetPsAuthCtxParams *pSetAuthCtxParams);

/**
  \fn          appGetAuthParamSync
  \brief       get authentication parameter information, such as authProtocol , user name, password
  \param[in]   const UINT8 cid the eps bearer cid
  \param[in\out]   GetPsAuthCtxParams *pGetAuthParams the auth parameters
  \returns     CmsRetId
  \NOTE:       this api  can query the eps bearer parameters as the same effect with AT +QICSGP
  *Note:
       1> this api only can get one Eps beaer information every function callback, and the cid must a valid cid
       2> Only support return usename and password of NULL/PAP/CHAP/CHAP_PAP auth protocol type
*/
CmsRetId appGetAuthParamSync(const UINT8  cid, GetPsAuthCtxParams *pGetAuthParams);


/**
  \fn          CmsRetId appSetTxPowerSetting
  \brief       Send cmi request to set Tx Power while UE in connnection state
  \param[in]   TxPowerSettingReq *pTxPowerSettingReq
  \param[out]
  \returns     CmsRetId
*/
CmsRetId appSetTxPowerSetting(TxPowerSettingReq *pTxPowerSettingReq);

CmsRetId appSetECBarCell(SetBarCellParamsReq *pSetBarCellReq);

#ifdef  FEATURE_IMS_ENABLE
/**
  \fn          appSetECIMSREGSync
  \brief       Send set ims register req to PS/ims
  \param[in]   UINT8 regAct, IMI_REG_IMS_DEREG(0)/IMI_REG_IMS_REG(1)
  \returns     CmsRetId
  \NOTE:       this api can set as the same as AT+ECIMSREG=<n>
*/
CmsRetId appSetECIMSREGSync(UINT8 regAct);
/**
  \fn          appGetECIMSREGSync
  \brief       Send get ims register state req to PS/ims
  \param[in]   UINT8 *pImsRegState, IMI_REG_IMS_DEREG(0)/IMI_REG_IMS_REG(1)
  \returns     CmsRetId
  \NOTE:       this api can set as the same as AT+ECIMSREG?
*/
CmsRetId appGetECIMSREGSync(UINT8 *pImsRegState);
/**
  \fn          appGetCIREGSync
  \brief       Get IMS register info
  \param[out]   CiregRegInfo *pRegInfo
  \param[out]   CtregExtInfo *pExtInfo
  \returns     CmsRetId
  \NOTE:       this api can set as the same as AT+CIREG?
*/
CmsRetId appGetCIREGSync(CiregRegInfo *pRegInfo, CiregExtInfo *pExtInfo);

/**
  \fn          appSetUpCallSync
  \brief       Send set up call req to PS/ims
  \param[in]   CHAR *pDialNumStr, the pointer to dial number string
  \returns     CmsRetId
  \NOTE:       this api can set as the same as ATD
*/
CmsRetId appSetUpCallSync(CHAR *pDialNumStr);

/**
  \fn          appAnswerCallSync
  \brief       Send answer call req to PS/ims
  \param[in]   null
  \returns     CmsRetId
  \NOTE:       this api can set as the same as ATA
*/
CmsRetId appAnswerCallSync(void);

/**
  \fn          appHangupCallSync
  \brief       Send hangup call req to PS/ims
  \param[in]   null
  \returns     CmsRetId
  \NOTE:       this api can set as the same as ATH/AT+CHUP to hangup current voice call
*/
CmsRetId appHangupCallSync(void);

/**
  \fn          appListCurrentCallSync
  \brief       Get current calls info
  \param[out]  EcListCurrCallInfo *pListCurrCallInfo, pointer to the List of current calls info
  \returns     CmsRetId
  \NOTE:       this api can set as the same as AT+CLCC
*/
CmsRetId appListCurrentCallSync(ListCurrCallInfo *pListCurrCallInfo);

/**
  \fn          appSetVTSSync
  \brief       Send DTMF str req to PS/ims
  \param[in]   CHAR *dtmfStr, DTMF string, support ''0-9,#,*,A-D''
  \param[in]   UINT16 duration, ms, range: 0, 100 -1000. 0 means use default value or set by appSetVTDSync
  \returns     CmsRetId
  \NOTE:       refer to AT+VTS
*/
CmsRetId appSetVTSSync(CHAR *dtmfStr, UINT16 duration);

/**
  \fn          appSetVTDSync
  \brief       Set DTMF duration req to PS/ims
  \param[in]   UINT16 duration, ms, range: 100 -1000
  \returns     CmsRetId
  \NOTE:       refer to AT+VTD=
*/
CmsRetId appSetVTDSync(UINT16 duration);

/**
  \fn          appGetVTDSync
  \brief       Get current calls info
  \param[out]  UINT16 *pDuration, pointer to the duration
  \returns     CmsRetId
  \NOTE:       this api can set as the same as AT+VTD?
*/
CmsRetId appGetVTDSync(UINT16 *pDuration);

#endif

#ifdef FEATURE_SMS_API_ENABLE
/**
  \fn          CmsRetId appSendSms
  \brief       Send cmi request to send SMS
  \param[in]   UINT8 smsFormat, Text/PDU Mode, see SmsFormatMode
  \param[in]   CHAR *da, see 27.005 <da>, Text Mode only
  \param[in]   UINT8 toda, see 27.005 <toda>, Text Mode only
  \param[in]   CHAR *inputStr,
  \                Text Mode: text string if dcs is GSM 7bit; hex string if dcs is 8-bit or UCS2
  \                PDU Mode: SC address followed by TPDU in hex format
  \param[in]   UINT32 guardTimerSec, guard timer for sending SMS in second
  \param[out]  UINT8 *pMsgRef, TP-Message-Reference, see 27.005 <mr>
  \returns     CmsRetId
*/
CmsRetId appSendSmsSync(BOOL bTextMode, CHAR *da, UINT8 toda,
                        CHAR *inputStr, UINT32 guardTimerSec, UINT8 *pMsgRef);

/**
  \fn          CmsRetId appSendMtSmsRsp
  \brief       Send ack after receiving MT SMS
  \param[in]   UINT8 smsId, smsId is received in PS_URC_ID_NEW_SMS
  \param[in]   bRcvOk, TRUE: MT SMS handling is OK, response RP-ACK to NW;
  \                    FALSE: MT SMS handling is fail, response RP-ERROR to NW
  \returns     CmsRetId
*/
void appSendMtSmsRsp(UINT8 smsId, BOOL bRcvOk);

/**
  \fn          CmsRetId appSmsSetTextModeParaSync
  \brief       Set SMS parameters used in Text mode
  \param[in]   UINT8 cfgMask, bit mask, set to 1 if the config is present,
  \                           bit 0: fo; bit 1: vp; bit 2: pid; bit 3: dcs;
  \param[in]   UINT8 fo, see 27.005 <fo>
  \param[in]   UINT8 vp, when vpf is RELATIVE format, see 27.005 <vp>
  \param[in]   CHAR *vpStr, when vpf is not RELATIVE format, see 27.005 <vp>
  \param[in]   UINT8 pid, see 27.005 <pid>
  \param[in]   UINT8 dcs, see 27.005 <dcs>
  \returns     CmsRetId
*/
CmsRetId appSmsSetTextModeParaSync(UINT8 cfgMask, UINT8 fo, UINT8 vp, CHAR *vpStr, UINT8 pid, UINT8 dcs);

/**
  \fn          CmsRetId appSmsWriteTextSmsToStorageSync
  \brief       Save SMS to NVM or SIM, Text Mode
  \param[in]   CHAR *da, see 27.005 <da>, Text Mode only
  \param[in]   UINT8 toda, see 27.005 <toda>, Text Mode only, Enum SmsTypeOfAddress
  \param[in]   CHAR *inputStr, text string if dcs is GSM 7bit; hex string if dcs is 8-bit or UCS2
  \param[in]   UINT8 smsMsgType, SMS message type, Enum SmsMessageType
  \param[in]   UINT8 smsMemType, see 27.005 <mem1>/<mem2>/<mem3>, Enum SmsStoreMemType
  \param[in]   UINT8 smsRecStatus, see 27.005 <stat>, Enum SmsRecStorStatus
  \param[out]  UINT8 *pMemIndex, index of the Memory
  \returns     CmsRetId
*/
CmsRetId appSmsWriteTextSmsToStorageSync(CHAR *da, UINT8 toda, CHAR *inputStr, UINT8 smsMsgType,
                                         UINT8 smsMemType, UINT8 smsRecStatus, UINT8 *pMemIndex);

/**
  \fn          CmsRetId appSmsWritePduSmsToStorageSync
  \brief       Save SMS to NVM or SIM, PDU Mode
  \param[in]   CHAR *inputStr, PDU hex string
  \param[in]   UINT8 smsMemType, see 27.005 <mem1>/<mem2>/<mem3>, Enum SmsStoreMemType
  \param[in]   UINT8 smsRecStatus, see 27.005 <stat>, Enum SmsRecStorStatus
  \param[out]  UINT8 *pMemIndex, index of the Memory
  \returns     CmsRetId
*/
CmsRetId appSmsWritePduSmsToStorageSync(CHAR *inputStr, UINT8 smsMemType, UINT8 smsRecStatus, UINT8 *pMemIndex);

/**
  \fn          CmsRetId appSmsReadTextSmsFromStorageSync
  \brief       Read SMS from NVM or SIM
  \param[in]   UINT8 smsMemType, see 27.005 <mem1>/<mem2>/<mem3>, Enum SmsStoreMemType
  \param[in]   UINT8 memIndex, index of the Memory
  \param[out]  SmsInfo *pSmsInfo, SMS info
  \returns     CmsRetId
*/
CmsRetId appSmsReadTextSmsFromStorageSync(UINT8 smsMemType, UINT8 memIndex, SmsInfo *pSmsInfo);

/**
  \fn          CmsRetId appSmsDeleteSmsFromStorageSync
  \brief       Save text SMS to NVM or SIM
  \param[in]   UINT8 smsMemType, SMS message preferred storage memory type PsilSmsStoreMemType
  \param[in]   UINT8 delFlag, see 27.005 <delflag>, Enum SmsDelFlag
  \param[out]  UINT8 delIndex, index of the Memory
  \returns     CmsRetId
*/
CmsRetId appSmsDeleteSmsFromStorageSync(UINT8 smsMemType, UINT8 delFlag, UINT8 delIndex);

/**
  \fn          CmsRetId appSmsGetMemStorageInfoSync
  \brief       get SMS preferred storage memory info from NVM or SIM
  \param[in]   UINT8 smsMemType, SMS message preferred storage memory type PsilSmsStoreMemType
  \param[out]  UINT8 *pTotalNum, total number of message locations in <smsMemType>
  \param[out]  UINT8 *pUsedNum, number of messages currently in <smsMemType>
  \returns     CmsRetId
*/
CmsRetId appSmsGetMemStorageInfoSync(UINT8 smsMemType, UINT8 *pTotalNum, UINT8 *pUsedNum);

#endif

#endif

