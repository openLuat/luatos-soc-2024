/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:exception_dump_defs.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/

#ifndef _EXCEPTION_DUMP_DEFS_H
#define _EXCEPTION_DUMP_DEFS_H
#include "commontypedef.h"
#include "exception_dump.h"
#include "sys_record.h"

typedef struct _EPAT_ec_m3_exception_regs
{
    struct
    {
        uint32_t r0;
        uint32_t r1;
        uint32_t r2;
        uint32_t r3;
        uint32_t r4;
        uint32_t r5;
        uint32_t r6;
        uint32_t r7;
        uint32_t r8;
        uint32_t r9;
        uint32_t r10;
        uint32_t r11;
        uint32_t r12;
        uint32_t sp;
        uint32_t lr;
        uint32_t pc;
        union
        {
            uint32_t value;
            struct
            {
                uint32_t IPSR : 8;
                uint32_t EPSR : 8;
                uint32_t APSR : 8;
            }bits;
        }psr;
        uint32_t exc_return;
        uint32_t msp;
        uint32_t psp;
        uint32_t CONTROL;
        uint32_t BASEPRI;
        uint32_t PRIMASK;
        uint32_t FAULTMASK;
    }stack_frame;

    union
    {
        uint32_t value;
        struct
        {
            uint32_t MEM_FAULT_ACT      : 1;
            uint32_t BUS_FAULT_ACT      : 1;
            uint32_t UNUSED_BITS1       : 1;
            uint32_t USAGE_FAULT_ACT    : 1;
            uint32_t UNUSED_BITS2       : 3;
            uint32_t SVCALLACT          : 1;
            uint32_t MONITORACT         : 1;
            uint32_t UNUSED_BITS3       : 1;
            uint32_t PENDSVACT          : 1;
            uint32_t SYSTICKACT         : 1;
            uint32_t USAGEFAULTPENDED   : 1;
            uint32_t MEMFAULTPENDED     : 1;
            uint32_t BUSFAULTPENDED     : 1;
            uint32_t SVCALLPENDED       : 1;
            uint32_t MEMFAULT_EN        : 1;
            uint32_t BUSFAULT_EN        : 1;
            uint32_t USAGEFAULT_EN      : 1;
        }bits;
    }sys_ctrl_stat;

    union
    {
        uint8_t value;
        struct
        {
            uint8_t IACCVIOL    : 1;
            uint8_t DACCVIOL    : 1;
            uint8_t UNUSED_BIT  : 1;
            uint8_t MUNSTKERR   : 1;
            uint8_t MSTKERR     : 1;
            uint8_t MLSPERR     : 1;
            uint8_t UNUSED_BIT2 : 1;
            uint8_t MMARVALID   : 1;
        }bits;
    }mfsr;

    union
    {
        uint8_t value;
        struct
        {
            uint8_t IBUSERR     : 1;
            uint8_t PRECISEER   : 1;
            uint8_t IMPREISEER  : 1;
            uint8_t UNSTKERR    : 1;
            uint8_t STKERR      : 1;
            uint8_t LSPERR      : 1;
            uint8_t UNUSED_BIT  : 1;
            uint8_t BFARVALID   : 1;
        }bits;
    }bfsr;

    union
    {
        unsigned short value;
        struct
        {
            unsigned short UNDEFINSTR   : 1;
            unsigned short INVSTATE     : 1;
            unsigned short INVPC        : 1;
            unsigned short NOCP         : 1;
            unsigned short UNUSED_BITS  : 4;
            unsigned short UNALIGNED    : 1;
            unsigned short DIVBYZERO    : 1;
        }bits;
    }ufsr;

    union
    {
        uint32_t value;
        struct
        {
            uint32_t UNUSED_BIT1    : 1;
            uint32_t VECTBL         : 1;
            uint32_t UNUSED_BIT2    : 28;
            uint32_t FORCED         : 1;
            uint32_t DEBUGEVT       : 1;
        }bits;
    }hfsr;

    union
    {
        uint32_t value;
        struct
        {
            uint32_t HALTED     : 1;
            uint32_t BKPT       : 1;
            uint32_t DWTTRAP    : 1;
            uint32_t VCATCH     : 1;
            uint32_t EXTERNAL   : 1;
        }bits;
    }dfsr;

    uint32_t mmfar;
    uint32_t bfar;
    uint32_t afar;
}ecM3ExceptionRegs_s;

typedef struct _EPAT_PLAT_ExcepKeyInfoStore
{
	uint32_t uiExcepOccured;
	uint16_t usExceptChipType;
	uint8_t ucExcepCPUType;
	uint8_t ucIsUnilogDump;
	uint32_t uiExcepFlag;
	uint32_t uiUlgDumpOft;
	uint16_t usUlgStrOftAddr;
	uint16_t usUlgStrSpace;
	uint16_t usPlatOffset;
	uint16_t usPlatSpace;
	uint16_t usPHYOffset;
	uint16_t usPHYSpace;
	uint16_t usPSOffset;
	uint16_t usPSSpace;
	uint16_t usUnilogOffset;
	uint16_t usUnilogSpace;
	uint16_t usCustOffset;
	uint16_t usCustSpace;
	ecM3ExceptionRegs_s excep_regs;
	uint8_t ucCurrTaskName[EC_EXCEP_TASK_NAME_LEN];
    uint8_t ucEecepAssertBuff[EC_EXCEP_ASSERT_BUFF_LEN];

	struct
	{
		uint32_t uiTotalHeapSize;
		uint32_t uiFreeSize;
		uint32_t uiMaxFreeBlockSize;
		uint32_t uiMinEverFreeHeapSize;
	}heap_info_s;
}PLAT_EcExcepKeyInfoStore;

typedef union _EPAT_PLAT_ecInterInfo
{
    UINT32 ecNVICAndResv;                    // NVIC(0-14)---U16            resv(0xFFFF)---U16
    UINT32 ecXicModuleAndIdx;                // xic module(0-2)---U16       idx(0-31)---U16
    CHAR   ecRecordName[RECORD_LIST_NAME_LEN]; //RECORD_LIST_NAME_LEN
}ec_ecInterInfo;

typedef struct _EPAT_PLAT_stack
{
    UINT32 ecStack[32];
}PLAT_stack;

typedef struct _EPAT_PLAT_recordNode_s
{
    ec_ecInterInfo ecInterInfos;    
    UINT32 ecInTime;
}PLAT_ecRecordNode_s;

typedef struct _EPAT_PLAT_recordNodeList
{
    PLAT_ecRecordNode_s RecordNodeList[RECORD_LIST_LEN];
}PLAT_ecRecordNodeList;

typedef struct _EPAT_PS_Imsi
{
    UINT8               length;
    UINT8               mncType;    //PlmnMncDigitType
    UINT16              accessTech;
    UINT8               contents[8];
}PS_Imsi;

typedef struct _EPAT_PS_Plmn
{
    UINT16  mcc;
    UINT16  mncWithAddInfo; // if 2-digit MNC type, the 4 MSB bits should set to 'F',
}PS_Plmn;

typedef struct _EPAT_PS_Tai
{
    PS_Plmn plmn;
    UINT16  tac;
    UINT16  reserved;
}PS_Tai;

typedef enum _EPAT_PS_TaiListType
{
   PS_TAI_LIST_TYPE_0 = 0,
   PS_TAI_LIST_TYPE_1 = 1,
   PS_TAI_LIST_TYPE_2 = 2
}PS_TaiListType;

typedef struct _EPAT_PS_PartialTaiList_Type_0
{
    PS_Plmn plmn;
    UINT16  tac[16];    //MAX_TAC_LIST_SIZE
}PS_PartialTaiList_Type_0;

typedef PS_PartialTaiList_Type_0   PS_PartialTaiList_Type_1;

typedef struct _EPAT_PS_PartialTaiList_Type_2
{
    PS_Tai    tai[16];  //MAX_TAI_LIST_SIZE
}PS_PartialTaiList_Type_2;

typedef union _EPAT_PS_TaiListUnion
{
    PS_PartialTaiList_Type_0        taiListType_0;
    PS_PartialTaiList_Type_1        taiListType_1;
    PS_PartialTaiList_Type_2        taiListType_2;
}PS_TaiListUnion;

typedef struct _EPAT_PS_PartialTaiList
{
    PS_TaiListType      taiListType;
    UINT8               numOfElements;
    PS_TaiListUnion     taiListUnion;
}PS_PartialTaiList;

typedef enum _EPAT_PS_EpsUpdateStatus
{
    PS_USIM_EUS_UPDATED             = 0,
    PS_USIM_EUS_NOT_UPDATED         = 1,
    PS_USIM_EUS_ROAMING_NOT_ALLOWED = 2,
    PS_USIM_EUS_RESERVED            = 7
}PS_EpsUpdateStatus;

typedef struct _EPAT_PS_Guti
{
    UINT8   gutiContents[10];   //MAX_GUTI_SIZE
}PS_Guti;

typedef struct _EPAT_PS_TrackingInformation
{
    PS_Guti                 guti;
    PS_Tai                  tai;
    UINT8                   numOfTaiList;
    PS_PartialTaiList       *pTaiList;
    PS_EpsUpdateStatus      epsUpdateStatus;
}PS_TrackingInformation;

typedef struct _EPAT_PS_CkIkContent
{
    UINT8           ksi;  //NasKsi
    UINT8           ikLen;
    UINT8           ckLen;
    UINT8           reserved1;
    UINT8           ik[16]; //IK_SIZE
    UINT8           ck[16]; //CK_SIZE
}PS_CkIkContent;

typedef struct _EPAT_PS_AuthRes
{
    UINT8   length;
    UINT8   reserved0;
    UINT16  reserved1;
    UINT8   data[16];   //RES_SIZE
}PS_AuthRes;

typedef struct _EPAT_PS_Auts
{
    UINT8   length;
    UINT8   reserved0;
    UINT16  reserved1;
    UINT8   data[16];
}PS_Auts;

typedef enum _EPAT_PS_EmmCause
{
    PS_EMM_CAUSE_IMSI_UNKNOWN_IN_HSS                                  = 0x02,
    PS_EMM_CAUSE_ILLEGAL_UE                                           = 0x03,
    PS_EMM_CAUSE_IMEI_NOT_ACCEPTED                                    = 0x05,
    PS_EMM_CAUSE_ILLEGAL_ME                                           = 0x06,
    PS_EMM_CAUSE_EPS_SERVICES_NOT_ALLOWED                             = 0x07,
    PS_EMM_CAUSE_EPS_AND_NON_EPS_SERVICES_NOT_ALLOWED                 = 0x08,
    PS_EMM_CAUSE_UE_ID_CAN_NOT_BE_DERIVED_IN_NETWORK                  = 0x09,
    PS_EMM_CAUSE_IMPLICITLY_DETACHED                                  = 0x0A,
    PS_EMM_CAUSE_PLMN_NOT_ALLOWED                                     = 0X0B,
    PS_EMM_CAUSE_TRACKING_AREA_NOT_ALLOWED                            = 0x0C,
    PS_EMM_CAUSE_ROAMING_NOT_ALLOWED_IN_THIS_TRACKING_AREA            = 0X0D,
    PS_EMM_CAUSE_EPS_SERVICE_NOT_ALLOWED_IN_THIS_PLMN                 = 0x0E,
    PS_EMM_CAUSE_NO_SUITABLE_CELLS_IN_TRACKING_AREA                   = 0x0F,
    PS_EMM_CAUSE_MSC_TEMPORARILY_NOT_REACHABLE                        = 0x10,
    PS_EMM_CAUSE_NETWORK_FAILURE                                      = 0x11,
    PS_EMM_CAUSE_CS_DOMAIN_NOT_AVAILABLE                              = 0x12,
    PS_EMM_CAUSE_ESM_FAILURE                                          = 0x13,
    PS_EMM_CAUSE_MAC_FAILURE                                          = 0X14,
    PS_EMM_CAUSE_SYNCH_FAILURE                                        = 0X15,
    PS_EMM_CAUSE_CONGESTION                                           = 0X16,
    PS_EMM_CAUSE_UE_SECURITY_CAPAILITIES_MISMATCH                     = 0x17,
    PS_EMM_CAUSE_SECURITY_MODE_REJECTED_UNSPECIFIED                   = 0x18,
    PS_EMM_CAUSE_NOT_AUTHORIZED_FOR_THIS_CSG                          = 0x19,
    PS_EMM_CAUSE_NON_EPS_AUTHENTICATION_UNACCEPTABLE                  = 0x1A,
    PS_EMM_CAUSE_REQUESTED_SERVICE_OPTION_NOT_AUTHORIZED_IN_THIS_PLMN = 0X23,
    PS_EMM_CAUSE_CS_SERVICE_TEMPORARILY_NOT_AVAILABLE                 = 0x27,
    PS_EMM_CAUSE_NO_EPS_BEARER_CONTEXT_ACTIVATED                      = 0x28,
    PS_EMM_CAUSE_SERVERE_NETWORK_FAILURE                              = 0x2A,
    PS_EMM_CAUSE_SYMANTICALLY_INCORRECT_MESSAGE                       = 0X5F,
    PS_EMM_CAUSE_INVALID_MANDATORY_INFORMATION                        = 0X60,
    PS_EMM_CAUSE_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED         = 0X61,
    PS_EMM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE  = 0X62,
    PS_EMM_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED  = 0X63,
    PS_EMM_CAUSE_CONDITIONAL_IE_ERROR                                 = 0X64,
    PS_EMM_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE       = 0X65,
    PS_EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED                           = 0X6F,
    /* internal cause */
    PS_EMM_CAUSE_AUTHENTICATION_REJECT                                = 0x101,
    PS_EMM_CAUSE_NO_CAUSE                                             = 0XFFFF
}PS_EmmCause;

typedef struct _EPAT_PS_AuthenticateResult
{
    UINT8           sqnXorAK[6];
    UINT8           storedRand[16]; //RAND_SIZE
    PS_AuthRes      storedRes;
    PS_Auts         auts;
    PS_EmmCause     result;
    UINT8           failTimes;
}PS_AuthenticateResult;

typedef struct _EPAT_PS_NasConfig
{
    BOOL    signallingPriorityPresent;
    UINT8   nasSignallingPriority;      /* NasSignallingPriority */
    BOOL    minSearchTimerPresent;
    UINT8   minPeriodicSearchTimer;     /* value in minutes */

    BOOL    eabPresent;
    BOOL    extendedAccessBarring;
    BOOL    t3245BehaviourPresent;
    BOOL    t3245Behaviour;

    BOOL    overrideNslpPresent;
    BOOL    overrideNasSignallingLowPriority;
    BOOL    overideEabPresent;
    BOOL    overrideExtendedAccessBarring;

    BOOL    fastFirstSearchPresent;
    BOOL    fastFirstHigherPriorityPlmnSearch;
    BOOL    smRetryTimePresent;
    UINT8   smRetryWaitTime;

    BOOL    exceptDataReportPresent;
    BOOL    exceptionDataReportingAllowed;
    BOOL    attachWithImsiPresent;
    BOOL    attachWithImsi;

    BOOL    dcnIdPresent;
    UINT8   rsv0;
    UINT16  defaultDcnId;
}PS_NasConfig;

typedef enum _EPAT_PS_EPSFilePresentType
{
    PS_EPS_FILE_NOT_PRESENT = 0,
    PS_EPS_LOCI_PRESENT     = 1,
    PS_EPS_NSC_PRESENT      = 2,
    PS_LOCI_PRESENT         = 4
}PS_EPSFilePresentType;

typedef struct _EPAT_PS_CemmEsmSimAclConfigInd
{
    BOOL                bSimAclEnable; /* whether the ACL (APN Control List) service is enable or not */
    UINT8               resv0;
    UINT16              aclRawDataLen; /* length of valid ACL raw data */

    /*
    * ACL (APN Control List) raw data
    * number of APNs (1 byte) + APN TLVs (tag '0xDD' + length + APN content/...)
    */
    UINT8               *pAclRawData; /* EF_ACL, referred to TS31.102 4.2.48 */
}PS_CemmEsmSimAclConfigInd;

typedef struct _EPAT_PS_SimInformation
{
    BOOL                        simPresent;
    BOOL                        simInvalidForEps;
    BOOL                        simInvalidForNonEps;
    BOOL                        simInvalidForSms;
    PS_Imsi                     imsi;
    UINT16                      accessClass;
    PS_TrackingInformation      epsLocationInfo;
    UINT8                       writeSimFlag;
    PS_CkIkContent              keys;
    PS_AuthenticateResult       authticateResult;
    UINT8                       algorithmId;        /* The identifiers of selected NAS integrity and encryption algorithms */
    PS_NasConfig                nasConfig;
    PS_EPSFilePresentType       epsFilePresentType;
    PS_CemmEsmSimAclConfigInd   simAclConfig;       /* sim acl configuration */
}PS_SimInformation;

typedef struct _EPAT_PS_SysInformation
{
    PS_Tai      tai;
    BOOL        bTaiSame;
    BOOL        bPlmnSame;
    BOOL        attachWithoutPdn;
    UINT32      cellIdentity;
    UINT32      carrierFreq;
    UINT16      phyCellId;
    UINT8       freqBandIndicator;
    BOOL        imsEmergencySupport;
    BOOL        bSharedNetwork;
}PS_SysInformation;

typedef struct _EPAT_PS_CauseInformation
{
    PS_EmmCause    rejectCause;
}PS_CauseInformation;

typedef enum _EPAT_PS_UeService
{
    PS_EPS_ONLY        = 0,
    PS_NON_EPS         = 1,
    PS_EPS_AND_NON_EPS = 2,
    PS_EMC_SERVICE     = 3,    /* request for emergency attach */
    PS_NON_SERVICE
}PS_UeService;

typedef enum _EPAT_PS_NwMode
{
    PS_NB_IOT = 0,
    PS_CAT1,
    PS_EMTC_IOT
}PS_NwMode;

typedef enum _EPAT_PS_CemmPlmnSelectTypeEnum
{
    PS_CEMM_DEFAULT_PLMN_REG = 0,  // CEMM PLMN module decide by self
    PS_CEMM_AUTO_PLMN_REG,
    PS_CEMM_MANUAL_PLMN_REG,
    PS_CEMM_MANUAL_THEN_AUTO_PLMN_REG, //if manual selection fails, automatic mode
}PS_CemmPlmnSelectTypeEnum;

typedef enum _EPAT_PS_MsgTransResult
{
    PS_NOT_REQUESTED            = 0,
    PS_REQUESTD_TRANSMITTING    = 1,
    PS_REQUESTD_TRANSMIT_SUCC   = 2
}PS_MsgTransResult;

typedef struct _EPAT_PS_AttachInformation
{
    UINT8                       attachPdnType;          /* PdnType */
    UINT8                       attachCounter;
    BOOL                        needComplete;
    BOOL                        waitEsmMsgContainer;    /* This flag means EMM is waiting ESM message container to complete ATTACH REQUEST message */
    PS_UeService                requestService;         /* User requested ATTACH or DETACH service type(default setting or AT command setting) */
    PS_UeService                emmService;             /* EMM maintained registered service type,may different with requestService */
    PS_NwMode                   nwMode;
    PS_CemmPlmnSelectTypeEnum   plmnSelectType;
    PS_Plmn                     requestedPlmn;
    PS_MsgTransResult           attachCompleteTransResult;    /* Attach Comeplete transmit Result */
    BOOL                        userReselection;
}PS_AttachInformation;

typedef struct _EPAT_PS_TauBoolInformation
{
    UINT16              tauRequired : 1;
    UINT16              periodicTauNeed : 1;
    UINT16              loadBalancingTau : 1;
    UINT16              tauResponseToPaging : 1;
    UINT16              tauForSmsOnly : 1;
    UINT16              activeFlag : 1;                 /* Request to set active flag */
    UINT16              nwEpsBearerStatusPresent : 1;
    UINT16              pendingTau : 1;
    UINT16              updateCs : 1;
    UINT16              reserved : 7;
}PS_TauBoolInformation;

typedef struct _EPAT_PS_TauInformation
{
    UINT8                   tauCounter;
    BOOL                    waitEsmBearerContext;       /* This flag means EMM is waiting EPS bearer context status to complete TAU REQUEST message */
    PS_TauBoolInformation   tauBoolInfo;
    UINT16                  epsBearerContextStatus;
    PS_MsgTransResult       tauCompleteTransResult;     /* Tau Comeplete trans Result */
}PS_TauInformation;

typedef enum PS_SrReason
{
    PS_SR_ESM_SIGNALLING,
    PS_SR_ESM_DATA,
    PS_SR_SMS_SIGNALLING,
    PS_SR_PAGING_RESPONSE,
    PS_SR_DATA,
    PS_SR_SS_SIGNALLING,
    PS_SR_NONE
}PS_SrReason;

typedef enum _EPAT_PS_CemmDrReestRet
{
    PS_CEMM_DR_RE_EST_SUCC = 0,                        /* SR succ, and DRB should established */
    PS_CEMM_DR_RE_EST_FAIL_RETRY_LATER = 1,            /* DR could retry, after a timer */
    PS_CEMM_DR_RE_EST_FAIL_RETRY_AFTER_RESUME = 2,     /* DR could retry, after recv resume request from EMM */
}PS_CemmDrReestRet;

typedef enum _EPAT_PS_CeDrEstConnSrvType
{
    PS_CEDR_EST_CONN_COMMON_SERVICE,
    PS_CEDR_EST_CONN_MMTEL_VOICE,
    PS_CEDR_EST_CONN_MMTEL_VIDEO,  /*define here, not support now*/
    PS_CEDR_EST_CONN_SMSOIP
}PS_CeDrEstConnSrvType;

typedef struct _EPAT_PS_ServiceInformation
{
    PS_SrReason             srReason;               /* Service request triggered reason */
    UINT16                  pendingRequest;         /* Pending upper layer service request */
    BOOL                    bSrTiggeredInIdle;      /* Service request is triggered in IDLE */
    BOOL                    activeFlag;             /* Set active flag in Control Plane Service Request */
    BOOL                    waitEsmBearerContext;   /* This flag means EMM is waiting EPS bearer context status to complete CONTROL SERVICE REQUEST message */
    BOOL                    esmForEmcBr;            /* The service request procedure is initiated to establish a PDN connection for emergency bearer */
    UINT8                   sigRetransmitCounter;   /* While RACH failed with CESTABLISH_FAIL_T300_EXPIRY,retry MAX 3 times */
    UINT8                   srCounter;              /* Service Request Attempt Counter */
    UINT8                   srRetryCounter;         /* Service Request Retry Counter */
    PS_CemmDrReestRet       drReestRslt;            /* DR re-establish result */
    PS_CeDrEstConnSrvType   drSrvType;              /* DR requested service type */
    BOOL                    drForEmcBr;             /* DR requested user plane radio resources for emergency bearer services */
}PS_ServiceInformation;

typedef struct _EPAT_PS_DetachBoolInformation
{
    UINT8           bPowerOff : 1;              /* Power off detach */
    UINT8           bSimRefresh : 1;            /* SIM refresh triggered detach */
    UINT8           bSimRemoveDetach : 1;       /* SIM remove triggered detach */
    UINT8           bWaitDetachCnf : 1;         /* Sending detach confirm flag */
    UINT8           bPsModeChangeDetach : 1;    /* AT+CEMODE changing UE mode of operation,trigger NON-EPS detach */
    UINT8           reserved : 3;
}PS_DetachBoolInformation;

typedef enum _EPAT_PS_CemmPlmnDeactCause
{
    PS_CEMM_PLMN_DEACT_DETACH,
    PS_CEMM_PLMN_DEACT_POWER_OFF,
    PS_CEMM_PLMN_DEACT_ACT_PSM,
    PS_CEMM_PLMN_DEACT_CRITICAL_REJECT,
    PS_CEMM_PLMN_DEACT_BUTT
}PS_CemmPlmnDeactCause;

typedef struct _EPAT_PS_DetachInformation
{
    PS_DetachBoolInformation    detachBoolInfo;
    UINT8                       detachCounter;      /* Detach attempt counter */
    PS_UeService                detachService;      /* Service type to be detached */
    PS_CemmPlmnDeactCause       deactiveReason;     /* Deactivate reason */
}PS_DetachInformation;

typedef struct _EPAT_PS_BlockedPlmnInformation
{
    PS_Plmn     sms;                /* SMS services not available in this PLMN */
    PS_Plmn     severeNwFailure;    /* #42 (Severe network failure) */
}PS_BlockedPlmnInformation;

typedef enum _EPAT_PS_CemmRegStatusEnum
{
    PS_CEMM_NOT_REG = 0,       //not registered, UE is not searching an operater to register to

    /* REG STATUS */
    PS_CEMM_REG_HOME = 1,      //registered, home network
    PS_CEMM_REG_ROAMING = 2,   //registered, roaming
    PS_CEMM_REG_EMC_BEARER,    //attached for emergency bearer services only

    /* DEREG STATUS */
    PS_CEMM_MO_DETACH,         //mo detach, user triggered
    PS_CEMM_MT_DETACH,         //mt detach, network triggered

    PS_CEMM_ATTACH_REJECT,     //attach rejected by network
    PS_CEMM_TAU_REJECT,        //tau rejected by network
    PS_CEMM_AUTH_FAIL,         //ue authentication network failed
    PS_CEMM_AUTH_NW_REJECT,    //network authentication ue failed
    PS_CEMM_SR_REJECT,         //service rejected by network

    PS_CEMM_IMSI_PAGING,       //imsi paging, network triggered

    PS_CEMM_REG_DENIED,        //attach abnormal,attempt counter is equal to 5, start t3402

    PS_CEMM_REG_TRYING,        //tau abnormal,attempt counter is less than 5, try to register, report cereg:2

    PS_CEMM_REG_MAX = 0xFF
}PS_CemmRegStatusEnum;

typedef enum _EPAT_PS_EmmSubState
{
    /* Main states */
    PS_EMM_NULL                                = 0,
    PS_EMM_REGISTERED_INITIATED                = 1,
    PS_EMM_DEREGISTERED_INITIATED              = 2,
    PS_EMM_TRACKING_AREA_UPDATING_INITIATED    = 3,
    PS_EMM_SERVICE_REQUEST_INITIATED           = 4,
    /* Substates of state EMM-DEREGISTERED */
    PS_EMM_DEREGISTERED_NORMAL_SERVICE         = 5,
    PS_EMM_DEREGISTERED_LIMITED_SERVICE        = 6,
    PS_EMM_DEREGISTERED_ATTEMPTING_TO_ATTACH   = 7,
    PS_EMM_DEREGISTERED_PLMN_SEARCH            = 8,
    PS_EMM_DEREGISTERED_NO_IMSI                = 9,
    PS_EMM_DEREGISTERED_ATTACH_NEEDED          = 10,
    PS_EMM_DEREGISTERED_NO_CELL_AVAILABLE      = 11,
    PS_EMM_DEREGISTERED_ECALL_INACTIVE         = 12,
    /* Substates of state EMM-REGISTERED */
    PS_EMM_REGISTERED_NORMAL_SERVICE           = 13,
    PS_EMM_REGISTERED_ATTEMPTING_TO_UPDATE     = 14,
    PS_EMM_REGISTERED_LIMITED_SERVICE          = 15,
    PS_EMM_REGISTERED_PLMN_SEARCH              = 16,
    PS_EMM_REGISTERED_UPDATE_NEEDED            = 17,
    PS_EMM_REGISTERED_NO_CELL_AVAILABLE        = 18,
    PS_EMM_REGISTERED_ATTEMPTING_TO_UPDATE_MM  = 19,
    PS_EMM_REGISTERED_IMSI_DETACH_INITIATED    = 20
}PS_EmmSubState;

typedef enum _EPAT_PS_EmmConnectionState
{
    PS_EMM_CONN_STATE_IDLE,
    PS_EMM_CONN_STATE_CONNECTED,
    PS_EMM_CONN_STATE_CONNECTING,
    PS_EMM_CONN_STATE_NAS_RELEASE
}PS_EmmConnectionState;

typedef enum _EPAT_PS_EmmSpecificProcedures
{
    PS_PROCEDURE_NONE = 0,
    PS_PROCEDURE_EPS_ATTACH,
    PS_PROCEDURE_EPS_AND_NON_EPS_ATTACH,
    PS_PROCEDURE_EMERGENCY_ATTACH,
    PS_PROCEDURE_PERIODIC_TAU,
    PS_PROCEDURE_NORMAL_TAU,
    PS_PROCEDURE_COMBINED_TAU,
    PS_PROCEDURE_COMBINED_TAU_WITH_IMSI_ATTACH,
    PS_PROCEDURE_CONTROL_PLANE_SERVICE,
    PS_PROCEDURE_EXTENDED_SERVICE,
    PS_PROCEDURE_EPS_DETACH,
    PS_PROCEDURE_IMSI_DETACH,
    PS_PROCEDURE_COMBINED_DETACH
}PS_EmmSpecificProcedures;

typedef struct _EPAT_PS_EmmProcedures
{
    BOOL                        bProcedureOngoing;
    PS_EmmSpecificProcedures    emmSpecificProcedure;
}PS_EmmProcedures;

typedef struct _EPAT_PS_EmmBoolFlags
{
    UINT32          bEpsAttached : 1;           /* EPS attached */
    UINT32          bAccessBarred : 1;          /* Access barred */
    UINT32          bEncrypted : 1;             /* ciphering of NAS messages is activated */
    UINT32          bIntegrityProtected : 1;    /* integrity protection of NAS messages is activated */
    UINT32          bSecureExchangeSet : 1;     /* secure exchange of NAS message is established */
    UINT32          bIntegrityProtectedMsg : 1; /* Message is integrity protected */
    UINT32          bRbSetUp : 1;               /* User plane radio bearers have been set up */
    UINT32          bStartT3440 : 1;            /* Need start T3440 */
    UINT32          bImeisvRequest : 1;         /* NW requested IMEISV during authentication */
    UINT32          bSuspendSignalling : 1;     /* NAS signalling connection suspend */
    UINT32          bCmMsg : 1;                 /* Cm layer air messages */
    UINT32          bSearchingPlmn : 1;         /* PLMN searching is ongoing */
    UINT32          bPendingPlmnSearch : 1;     /* Any PLMN searching is pending to be executed due to UE is in CONNECTED state */
    UINT32          bActivePsm : 1;             /* Pending Active PSM mode */
    UINT32          bT3324Valid : 1;            /* T3324 is valid means T3324 value is neither 0 nor TIMEOUT_PERIOD_TIMER_DEACTIVATED */
    UINT32          bIdleStateReq : 1;          /* Set to TRUE if received idle state req from PLMN module */
    UINT32          bSuspendedEsm : 1;          /* Has already suspended ESM */
    UINT32          bSuspendedSms : 1;          /* Has already suspended SMS */
    UINT32          bSuspendedSs : 1;           /* Has already suspended SS */
    UINT32          bExceptionData : 1;         /* An exceptional is event triggered */
    UINT32          bT3346ForException : 1;     /* T3346 was started when connection was established with cause set to  "MO exception data" */
    UINT32          bWaitCampInd : 1;           /* Received RRC connection release indication from AS,and waiting AS cell camping indication */
    UINT32          bTestLoopModeH : 1;
    UINT32          bChangeLowPriority : 1;
    UINT32          bDifferentHashValue :1;     /* HASHMME in the SECURITY MODE COMMAND message is different from the hash value locally calculated at the UE */
    UINT32          bLastSignallingPriority : 1;/* Last signalling priority in Attach/Tau/Control Ser Req */
    UINT32          bInLppMeasurement : 1;
    UINT32          bNwRel : 1;                 /* Newwork initiated Release Ind */
    UINT32          bUlMsgSuspend : 1;          /* Uplink NAS message transmission is suspended due to Control Plane RRCConnection re-establishment */
    UINT32          estFailReselection : 1;     /* Last time RRC connection setup fail with cause CESTABLISH_FAIL_RESELECTION */
    UINT32          waitActCnf : 1;             /* Whether is waiting SIG_CERRC_ACT_CNF */
    UINT32          waitDeactCnf : 1;           /* Whether is waiting SIG_CERRC_DEACT_CNF */
    /* 4 bytes above */

    UINT32          updateEpslociToNvm : 1;     /* Pending update EPS location information to NVM */
    UINT32          updateEpsnscToNvm : 1;      /* Pending uupdate EPS NAS security context to NVM */
    UINT32          bReEstSucc : 1;             /* Is Re-establishment Succeed */
    UINT32          erasedFlg : 1;              /* Is EMM event NVM memory has been cleared */
    UINT32          eventChangedFlg : 1;        /* Is EMM event statis has increased */
    UINT32          authPerformed : 1;          /* Is AUTHENTICATION procedure just performed */
    UINT32          abortPlmn : 1;              /* Is need abort current ongoing PLMN search */
    UINT32          bBlockUlEsmDataTrans : 1;   /* Uplink ESM_DATA_TRANSPORT will be blocked if T3440 is runing */
    UINT32          bEmerCampByNas : 1;         /* CEPLMN/CEMM trigger emergency camp on request */
    UINT32          bPendingEmergencyReq : 1;   /* Pending emergency camp on request when UE in connected state */
    UINT32          bAddT3402BlockPlmn : 1;     /* Is need to add T3402 block PLMN/TAC list */
    UINT32          estFailOtherOrT300Expiry : 1; /* Last time RRC connection setup fail with cause CESTABLISH_FAIL_OTHERS or CESTABLISH_FAIL_T300_EXPIRY */
    UINT32          bExitPsmPending : 1;        /* Exit PSM event report is pending */
    UINT32          rsvd : 19;
}PS_EmmBoolFlags;

typedef enum _EPAT_PS_ActStatus
{
    /* ERRC->EMM for normal service state, CerrcActReq(requestedPlmnValid=TRUE)
      Fill CerrcActCnf/CerrcActInd->actStatus with NO_SERVICE or SUCCESS*/
    /* NOTE: Upon ERRC received CerrcPlmnSearchReq(searchType=Normal), ERRC always send a CerrcActInd(ACT_NO_SERVICE) to EMM */
    PS_ACT_NO_SERVICE                          = 0,
    PS_ACT_SUCCESS                             = 1,

    /* ERRC->EMM for limited service state, CerrcActReq(requestedPlmnValid=FALSE)
      Fill CerrcActCnf/CerrcActInd->actStatus with EMERGENCY or EMERGENCY_NO_SERVICE */
    PS_ACT_EMERGENCY                           = 2,
    PS_ACT_EMERGENCY_NO_SERVICE                = 3,

    /* Only used in EMM */
    PS_ACT_FORBIDDEN_TA_FOR_REGIONAL_SERVICE   = 4,
    PS_ACT_FORBIDDEN_TA_FOR_ROAMING            = 5,
}PS_ActStatus;

typedef enum _EPAT_PS_CerrcCellChangeCause
{
    PS_CELL_CHANGE_NONE         = 0,
    PS_CELL_CHANGE_HANDOVER     = 1,
    PS_CELL_CHANGE_RESELECTION  = 2,
}PS_CerrcCellChangeCause;

typedef enum _EPAT_PS_EmmMessageType
{
    PS_ATTACH_REQUEST                 = 0x41,
    PS_ATTACH_ACCEPT                  = 0x42,
    PS_ATTACH_COMPLETE                = 0x43,
    PS_ATTACH_REJECT                  = 0x44,
    PS_DETACH_REQUEST                 = 0x45,
    PS_DETACH_ACCEPT                  = 0x46,
    PS_TA_UPDATE_REQUEST              = 0x48,
    PS_TA_UPDATE_ACCEPT               = 0x49,
    PS_TA_UPDATE_COMPLETE             = 0x4a,
    PS_TA_UPDATE_REJECT               = 0x4b,
    PS_EXTENDED_SERVICE_REQUEST       = 0x4c,
    PS_CONTROL_PLANE_SERVICE_REQUEST  = 0x4D,
    PS_SERVICE_REJECT                 = 0x4e,
    PS_SERVICE_ACCEPT                 = 0x4F,
    PS_GUTI_REALLOC_COMMAND           = 0x50,
    PS_GUTI_REALLOC_COMPLETE          = 0x51,
    PS_AUTHENTICATION_REQUEST         = 0x52,
    PS_AUTHENTICATION_RESPONSE        = 0x53,
    PS_AUTHENTICATION_REJECT          = 0x54,
    PS_AUTHENTICATION_FAILURE         = 0x5c,
    PS_IDENTITY_REQUEST               = 0x55,
    PS_IDENTITY_RESPONSE              = 0x56,
    PS_SECURITY_MODE_COMMAND          = 0x5d,
    PS_SECURITY_MODE_COMPLETE         = 0x5e,
    PS_SECURITY_MODE_REJECT           = 0x5f,
    PS_EMM_STATUS                     = 0x60,
    PS_EMM_INFORMATION                = 0x61,
    PS_DOWNLINK_NAS_TRANSPORT         = 0x62,
    PS_UPLINK_NAS_TRANSPORT           = 0x63,
    PS_DOWNLINK_GENERIC_NAS_TRANSPORT = 0x68,
    PS_UPLINK_GENERIC_NAS_TRANSPORT   = 0x69,
    PS_SERVICE_REQUEST                = 0x70,
    PS_CM_LAYER_MESSAGE               = 0x99,
    PS_MESSAGE_TYPE_RESERVED
}PS_EmmMessageType;

typedef enum _EPAT_PS_AdditionalUpdateResult
{
    PS_NO_ADDITIONAL_INFORMATION = 0,
    PS_CS_FALLBACK_NOT_PREFERRED = 1,
    PS_SMS_ONLY                  = 2,
    PS_RESERVED_VALUE            = 3
}PS_AdditionalUpdateResult;

typedef enum _EPAT_PS_DataRelAssistIndEnum
{
    PS_DATA_RAI_NO_INFO           = 0,
    PS_DATA_RAI_NO_UL_DL_FOLLOWED = 1,
    PS_DATA_RAI_ONLY_DL_FOLLOWED  = 2,
    PS_DATA_RAI_RESERVED          = 3
}PS_DataRelAssistIndEnum;

typedef struct _EPAT_PS_EmmFlags
{
    PS_EmmBoolFlags             emmBoolFlags;           /* EMM BOOL flalgs */
    PS_ActStatus                cellStatus;             /* active cell status */
    PS_CerrcCellChangeCause     cellChangeCause;        /* indicate if the cell is changed due to handover or cell reselecction */
    PS_EmmCause                 smcRejectCause;         /* Security mode reject cause */
    PS_EmmCause                 emmStatusCause;         /* EMM STATUS message cause */
    PS_EmmMessageType           resumeMsgType;          /* Message type which triggered the resume of NAS signalling */
    UINT8                       updateAsFlag;           /* Flag for sending SIG_CERRC_NAS_INFO_UPDATE_REQ */
    PS_AdditionalUpdateResult   additionalUpdateResult; /* Additional update result */
    PS_DataRelAssistIndEnum     esmRelInd;              /* ESM Release assistance indication */
}PS_EmmFlags;

typedef struct _EPAT_PS_MmGlobals
{
    PS_SimInformation               simInfo;                /* contents from sim card */
    PS_SysInformation               sysInfo;                /* values from system information */
    PS_CauseInformation             causeInfo;              /* Reject cause */
    PS_AttachInformation            attachInfo;             /* ATTACH procedure related information */
    PS_TauInformation               tauInfo;                /* TAU procedure related information */
    PS_ServiceInformation           srInfo;                 /* SERVICE REQUEST procedure related information */
    PS_DetachInformation            detachInfo;             /* DETACH procedure related information */
    PS_BlockedPlmnInformation       blockedPlmnInfo;        /* Blocked plmn information */
    PS_CemmRegStatusEnum            regStatus;              /* Register status to upper layer */
    PS_EmmSubState                  emmState;               /* EMM sublayer states */
    PS_EmmConnectionState           emmConnState;           /* EMM connection state */
    PS_EmmProcedures                procedureInfo;          /* EMM specific procedures information */
    PS_EmmFlags                     flags;                  /* EMM flags */
}PS_MmGlobals;

typedef struct _EPAT_PS_CesmSimConfig
{
    UINT8               signallingPriorityPresent   : 1;
    UINT8               overrideNslpPresent         : 1;
    UINT8               exceptDataReportPresent     : 1;
    UINT8               rsvd                        : 5;

    UINT8               nasSignallingPriority;          /* NasSignallingPriority,  Ref 31.102. 4.2.94. NAS signalling priority */
    BOOL                overrideNasSignallingPriority;  /* Ref 31.102, 4.2.94, As described in TS 24.368 [65],
                                                         * used to determine whether the NAS signalling priority included in NAS messages
                                                         * can be overriden.
                                                        */
    BOOL                exceptionDataReportingAllowed;
}PS_CesmSimConfig;

typedef struct _EPAT_PS_CesmCommInfo
{    /*
     * SIM info, used in CESM side, set by EMM, when:
     * 1> SIM ready IND; 2> power on recovery
    */
    PS_CesmSimConfig   simconfig;

    UINT32      esmInSuspending     :1;     /* suspend by CEMM */
    UINT32      sendUlDataBusy      :1;     /* an "ESM DATA TRANSPORT" is sent, and wait confirm for EMM,
                                             * then could send next "ESM DATA TRANSPORT"
                                            */
    UINT32      mtCpDataReportFlag  :1;     /* whether Reporting of terminating data via the control plane +CRTDCP
                                             * In fact, need to save into tiny context at least, - TBD
                                            */
    UINT32      nwEpcoSupportFlag   :1;     /* whether NW support EPCO, carried in: "EPS network feature support".
                                             * 1> if NW not support, can't send EPCO in next ESM signalling.
                                             * 2> this flag save in EMM tiny context, and pass to ESM when wakeup recovry
                                            */
    UINT32      nwAttWithoutPdnFlag :1;     /* whether NW support EMM-REGISTERED without PDN, carried in: "EPS network feature support".
                                             * 1> this flag save in EMM tiny context, and pass to ESM when wakeup recovry
                                            */
    UINT32      autoReplyFlag       :1;     /* For MT procedure: "Activate dedicated EPS bearer context request"/"Modify EPS bearer context request"
                                             * whether auto accept/reject the MT requstion.
                                            */
    UINT32      nwCpIotSupportFlag  :2;     /* CemmCiotOptTypeEnum, NW support CIOT opt info: NO OPT/CP/UP/CP+UP,
                                            */
    /* one bytes above */

    UINT32      uePreferOptFlag     :2;     /* CemmCiotOptTypeEnum, UE perfer CIOT opt setting: NO OPT/CP/UP
                                            */
    UINT32      ueSuptOptTypeFlag   :2;     /* CemmCiotOptTypeEnum, UE supported CIOT OPT type
                                            */
    UINT32      dupDlEsmMsg         :1;     /* whether the DL ESM message/signalling is duplicated. 0 - not, 1 - yes */
    UINT32      rsvd0               :3;
    /* one bytes above */

    UINT32      rsvd1               :8;

    UINT32      lastDlEsmMsg        :8;     /* EsmMessageType, last recv DL ESM message/signalling */
}PS_CesmCommInfo;

typedef struct _EPAT_PS_CfgCesmUserDefinedT3482
{
    BOOL            present;    /* whether user define/cfg the T3482 parameter. if not, using 3GPP timer */

    UINT8           tryCount;   /* "PDN CONNECTIVITY REQUEST" try count, 3GPP value: 5 */
    UINT16          tValueS;    /* Timer value in seconds */
}PS_CfgCesmUserDefinedT3482;

typedef struct _EPAT_PS_CfgCesmUserDefinedT3492
{
    BOOL            present;    /* whether user define/cfg the T3492 parameter. if not, using 3GPP timer */
    UINT8           tryCount;   /* "PDN DISCONNECT REQUEST" try count, 3GPP value: 5 */
    UINT16          tValueS;    /* Timer value in seconds */
}PS_CfgCesmUserDefinedT3492;

typedef struct _EPAT_PS_CesmTinyCtx
{
    PS_CfgCesmUserDefinedT3482  esm3482Timer;    /* ESM 3482 Timer */
    PS_CfgCesmUserDefinedT3492  esm3492Timer;    /* ESM 3492 Timer */

    UINT32                  attachEpsCid: 8;    /* Attach bearer CID */
    UINT32                  bEpco       : 1;    /* whether using PCO, or EPCO by default */
    UINT32                  bAclEnable  : 1;
    UINT32                  bPdpRemap   : 2;    /* whether PDP remap is enabled?
                                                 * 0 - disabled,
                                                 * 1 - remap once for attached bearer,
                                                 * 2 - always remap if APN/IP type is same, - TBD
                                                */
    UINT32                  bPdpReact   : 1;    /* whether default bearer/PDP is need to re-activate,
                                                 * if APN/IP type changes, when CGACT
                                                */
    UINT32                  esmRsvd     : 19;

}PS_CesmTinyCtx;

typedef struct _EPAT_PS_ctxInfo
{
	UINT8	bValid		: 1;
	UINT8	bDedicated	: 1;
	UINT8	rsvd		: 2;
	UINT8	cid 		: 4;	/* 0 - 15 */
}PS_ctxInfo;

typedef struct _EPAT_PS_CesmBrTinyCtx
{
	PS_ctxInfo ctxInfo[3];
}PS_CesmBrTinyCtx;

typedef struct _EPAT_PS_EpsQualityOfService
{
    UINT8           qci;                   /*Quality of Service Class Identifier*/
    BOOL            mbrGbrPresent;         /* indicate MBR/GBR present */
    UINT16          rsvd;

    UINT32          maxBitRateOfUplink;
    UINT32          maxBitRateOfDownlink;
    UINT32          guaranteedBitRateOfUplink;
    UINT32          guaranteedBitRateOfDownlink;
}PS_EpsQualityOfService;

typedef struct _EPAT_PS_PdnIpV4Addr
{
    UINT8            addrData[4];
}PS_PdnIpV4Addr;

typedef struct _EPAT_PS_PdnIpV6Addr
{
    UINT8            addrData[16];
}PS_PdnIpV6Addr;

typedef struct _EPAT_PS_APNRateControl
{
   UINT32  present     : 1; //whether APN RATE CTRL configured
   UINT32  reserved    : 3;
   UINT32  aer         : 1; //Additional exception reports
   UINT32  timeUnit    : 3;   //CeDrApnRateCtrlUlTimeUnit
   UINT32  maxUlRate   : 24;
}PS_APNRateControl;

typedef struct _EPAT_PS_AdditionalAPNRateControl
{
   UINT32  present     : 1; //whether APN RATE CTRL configured
   UINT32  reserved0   : 4;
   UINT32  timeUnit    : 3;   //CeDrApnRateCtrlUlTimeUnit
   UINT32  reserved1   : 8;
   UINT32  maxUlRate   : 16;
}PS_AdditionalAPNRateControl;

typedef struct _EPAT_PS_CesmDlPcoBasic
{
    UINT16          ipv4DnsNum      : 2;
    UINT16          ipv6DnsNum      : 2;
    UINT16          psDataOffInd    : 1;    /* Not support now */
    UINT16          rdsInd          : 1;    /* Not support now */
    UINT16          imCnSigFlagInd  : 1;    /* IM_CN_Signalling_Flag get from NW */

    UINT16          rsvd0 : 1;
    UINT16          rsvd1 : 8;

    UINT16          mtuSize : 16;       /* if set to 0, not configed, else
                                         * 1> if PDN is IPV4(IPV4V6) type, just the ipv4 MTU size
                                         * 1> if PDN is non-IP type, just the non-IP MTU size
                                        */

    PS_PdnIpV4Addr   ipv4DnsAddr[2];
    PS_PdnIpV6Addr   ipv6DnsAddr[2];

    PS_PdnIpV4Addr   gwIpv4Addr;         /* if gwIpv4Addr.addrData[0] == 0, not exit */

    PS_APNRateControl  apnRateCtrl;
    PS_AdditionalAPNRateControl    addApnRateCtrl;

    /*
     * others are not support now, don't need to save in local
    */
}PS_CesmDlPcoBasic;

typedef union _EPAT_PS_PacketFilterAddrInfo
{
    struct {
        UINT8   addr[4];
        UINT8   mask[4];
    } ipv4;
    struct {
        UINT8   addr[16];
        UINT8   mask[16];
    } ipv6;
    struct {
        UINT8   addr[16];
        UINT8   prefixLen;
    } ipv6Prefix;
}PS_PacketFilterAddrInfo;

typedef struct _EPAT_PS_PacketFilterPortInfo
{
    UINT16  min;
    UINT16  max;
}PS_PacketFilterPortInfo;

typedef struct _EPAT_PS_PacketFilterList
{
    UINT32      pfId        : 4;    //0 - 15
    UINT32      pfDir       : 2;    //TftPacketFilterDir
    UINT32      reserved0   : 2;
    UINT32      pfEpId      : 8;    //Packet filter evaluation precedence, 0 - 255

    UINT32      remoteAddrType  : 2;    //IpAddrTypeInTft
    UINT32      localAddrType   : 2;    //IpAddrTypeInTft
    UINT32      tosTcPresent    : 1;
    UINT32      protIdPresent   : 1;
    UINT32      ipSecSpiPresent : 1;
    UINT32      ipv6FLPresent   : 1;

    UINT32      localPortRangeType  : 2;    //PortTypeInTft
    UINT32      remotePortRangeType : 2;    //PortTypeInTft
    UINT32      cid                 : 4;    //current PacketFilter belong to which CID


    /*
     * 0 1 1 1 0 0 0 0: Type of service/Traffic class type
    */
    UINT8       tosTc;        //Type-of-Service/Traffic Class
    UINT8       tosTcMask;    //Type-of-Service/Traffic Class mask

    /*
     * 0 0 1 1 0 0 0 0: Protocol identifier/Next header type
    */
    UINT8       protId;

    UINT8       reserved2;

    /*
     * 0 0 0 1 0 0 0 0: IPv4 remote address type
     * 0 0 1 0 0 0 0 0: IPv6 remote address type
     * 0 0 1 0 0 0 0 1: IPv6 remote address/prefix length type
     * Can't coexist, only one shall be present in one packet filter
    */
    PS_PacketFilterAddrInfo    remoteAddr;

    /*
     * 0 0 0 1 0 0 0 1: IPv4 local address type
     * 0 0 1 0 0 0 1 1: IPv6 local address/prefix length type
    */
    PS_PacketFilterAddrInfo    localAddr;

    /*
     * 0 1 0 0 0 0 0 0: Single local port type
     * 0 1 0 0 0 0 0 1: Local port range type
    */
    PS_PacketFilterPortInfo    localPort;

    /*
     * 0 1 0 1 0 0 0 0: Single remote port type
     * 0 1 0 1 0 0 0 1: Remote port range type
    */
    PS_PacketFilterPortInfo    remotePort;

    /*
     * 0 1 1 0 0 0 0 0: Security parameter index type
    */
    UINT32      ipSecSpi;

    /*
     * 1 0 0 0 0 0 0 0: Flow label type
    */
    UINT32      ipv6FL;

    /*
     * next PF
    */
    struct _EPAT_PS_PacketFilterList  *pNext;
}PS_PacketFilterList;

//#include "cenascomm.h"
//
//#include "pscommtype.h"

typedef struct _EPAT_PS_CesmEpsDefBrBasic
{
    UINT32      ebi             : 4;
    UINT32      pdnType         : 4;    /* PdnType, IP type */

    UINT32      epsQosPresent   : 1;
    UINT32      tftPresent      : 1;
    UINT32      cpOnly          : 1;    /* PDP is for CP only, must not valid for cat1 */
    UINT32      bAttachBr       : 1;    /* whether this bearer is activated during attach procedure */
    UINT32      bRemapped       : 1;    /* Whether this bearer already remapped */
    UINT32      bEmergency      : 1;    /* Whether this bearer is an emergency BR:
                                         * a) for attached bearer, during emergency attach proc, EMM will notify ESM current bearer type,
                                         *   maybe (mostly) this type not same as defined by CGDCONT in "CesmPdpDefinition". And as store here,
                                         *   for EMC bearer, "CesmEpsDefBrBasic" should allocated in "CemmEsmProcMsgContainerIndMsg()"
                                         * b) for additional bearer, this flag is set by CGDCONT in CesmPdpDefinition
                                        */

    UINT32      rsvd            : 18;


    UINT8       apnLen;
    UINT8       esmCause;           /* 24.301 9.9.4.4: ESM cause, 0 - no cause */

    /*
     * 24.301, 9.9.4.28
     * 1: The purpose of the Serving PLMN rate control information element is to indicate the maximum number of uplink ESM
     *     DATA TRANSPORT messages including User data container IEs the UE is allowed to send via a PDN connection per
     *     6 minute interval
     * 2: The Serving PLMN rate control value is an integer equal to or higher than 10;
     * 3: FFFFH indicates that the maximum number send per 6 minute interval is not restricted;
     * 4: if set to 0/0xFFFF, means no rate control
     * 5: Seems useless fro CAT1
    */
    UINT16      splmnRateCtrl;

    /* default bearer context */
    UINT8       pdnAddr[12];    /* 12 bytes CESM_PDN_ADDR_SIZE
                                                 * 1> IPv4, the length is set to 4;
                                                 * 2> IPv6, the length is set to 8, 8 bytes IPv6 interface identifier;
                                                 * 3> IPv4v6, the length is set to 12:
                                                 *    0 - 7: IPv6 interface identifier;
                                                 *    8 - 11: IPv4 address
                                                */

    UINT8                   apn[100];    /* PS_APN_MAX_LEN 100 bytes, encoded APN, NW assigned */
    PS_EpsQualityOfService  epsQos;     /* 20 bytes */
    PS_CesmDlPcoBasic       dlPcoInfo;  /* 56 bytes */

    PS_PacketFilterList     tftHdr;     /* 92 bytes, store one TFT, if more TFT configed, "tftHdr.pNext" should not PNULL,
                                     * and allocated in sleep2 memory  */
    /* APN-AMBR, -TBD */
}PS_CesmEpsDefBrBasic;     /* 288 bytes, must limited in 300 bytes */

/*
 * dedicated bearer basic info
*/
typedef struct _EPAT_PS_CesmEpsDedBrBasic
{
    UINT32      pCid          : 4;
    UINT32      ebi           : 4;
    UINT32      bActed        : 1;    /* whether this bearer is activated */
    UINT32      epsQosPresent : 1;
    UINT32      tft1Present   : 1;    /* whether "tft1st" present/valid */
    UINT32      tft2Present   : 1;    /* whether "tft2nd" present/valid */
    UINT32      imCnSigFlagPresent : 1;  /*whether imCnSigFlagReq present */
    UINT32      imCnSigFlagReq   : 1;    /* IM_CN_Signalling_Flag_Ind set by UE
                                           * 0 : UE indicates that the PDP context is not for IM CN subsystem-related siganlling only
                                           * 1 : UE indicates that the PDP context is for IM CN subsystem-related siganlling only
                                          */
    UINT32      imCnSigFlagInd    : 1;    /* IM_CN_Signalling_Flag get from NW
                                           * 0 : PDP context is not for IM CN subsystem-related siganlling only
                                           * 1 : PDP context is for IM CN subsystem-related siganlling only
                                          */
    UINT32      rsvd          : 17;

    PS_EpsQualityOfService epsQos;     /* 20 bytes */

    /*
     * Seems no valid DL PCO info need to save/maintain - TBD
    */

    PS_PacketFilterList    tft1st;     /* 92 bytes, first TFT, and "tft1st.pNext == PNULL"  */
    PS_PacketFilterList    tft2nd;     /* 92 bytes, second TFT, and if more TFT configed, "tft2nd.pNext" should not PNULL,
                                     * and allocated in sleep2 memory  */
}PS_CesmEpsDedBrBasic;

typedef struct  _EPAT_PS_CesmEpsBrBasic
{
	union
	{
		PS_CesmEpsDefBrBasic       defBrBasic;     //288 bytes
		PS_CesmEpsDedBrBasic       dedBrBasic;     //208 bytes
	}CesmEpsBrBasic;
}PS_CesmEpsBrBasic;

typedef struct _EPAT_PS_EpsNetworkFeatureSupport
{
    UINT16      imsVoiceSupported : 1;              /* IMS voice over PS session indicator (IMS VoPS) */
    UINT16      emerBearerServicesSupported : 1;    /* Emergency bearer services indicator (EMC BS) */
    UINT16      esrpsSupported : 1;                 /* Support of EXTENDED SERVICE REQUEST for packet services (ESRPS) */
    UINT16      withoutPdnSupported : 1;            /* EMM REGISTERED without PDN connectivity (ERw/oPDN) */
    UINT16      cpCiotSupported : 1;                /* Control plane CIoT EPS optimization (CP CIoT) */
    UINT16      upCiotSupported : 1;                /* User plane CIoT EPS optimization (UP CIoT) */
    UINT16      s1UdataSupported : 1;               /* This bit indicates the capability for S1-u data transfer */
    UINT16      hcCpCiotSupported : 1;              /* Header compression for control plane CIoT EPS optimization (HC-CP CIoT) */
    UINT16      ePcoSupported : 1;                  /* Extended protocol configuration options (ePCO) */
    UINT16      restrictEc : 1;                     /* Restriction on enhanced coverage (RestrictEC) */
}PS_EpsNetworkFeatureSupport;



/******************************************************************************
*******************************************************************************
*   RRC dump structure definition
*******************************************************************************
******************************************************************************/
typedef struct _EPAT_PS_PART8_CeRrcBootContext
{
    //CerrcPagingUeIdentity
    struct
    {
        UINT8             bIMSIPresent : 1;
        UINT8             sTMSIPresent : 1;
        UINT8             reserved : 6;
        UINT8             lengthOfImsi;   //range:6~21
        UINT8             imsi[21];
        UINT8             MMEC;
        UINT32            mTMSI;
    }pagingUeId;
    INT16                           nRsrpThreshold;
    INT16                           nRsrqThreshold;
    INT16                           rmRsrpThreshold;
    UINT8                           sSearchDeltaP;
}PS_PART8_CeRrcBootContext;


//CerrcState
typedef enum _EPAT_PS_CERRC_CerrcStateTag
{
    PS_CERRC_DEACTIVATED = 0,
    PS_CERRC_IDLE = 1,
    PS_CERRC_CONNECTED = 2,
    PS_CERRC_INVALID_STATE = 0x7FFFFFFF,
}PS_CERRC_CerrcState;

//CerrcCsrState
typedef enum _EPAT_PS_CERRC_CsrStateTag
{
    PS_CSR_IDLE = 0,
    PS_CSR_PLMN_SEARCH = 1,
    PS_CSR_CELL_SELECTION = 2,
    PS_CSR_CELL_SELECTION_CAMPING = 3,
    PS_CSR_CELL_RESELECTION_CAMPING = 4,
    PS_CSR_INVALID = 0x7FFFFFFF,
} PS_CERRC_CsrState;

//CerrcRccState
typedef enum _EPAT_PS_CERRC_RccStateTag
{
    PS_RCC_IDLE = 0,
    PS_RCC_CONN_EST = 1,
    PS_RCC_CONN_REL = 2,
    PS_RCC_CONN_RE_EST = 3,
    PS_RCC_CONN_NORMAL = 4,
    PS_RCC_WAIT_REL_ACK_CNF = 5,
    PS_RCC_HANDOVER = 6,
    PS_RCC_INVALID = 0x7FFFFFFF,
} PS_CERRC_RccState;

typedef struct _EPAT_PS_PART9_CeRrcState_UeId
{
    PS_CERRC_CerrcState             rrcState;
    PS_CERRC_CsrState               csrState;
    PS_CERRC_RccState               rccState;

    //4 CerrcUeIdentity
    struct
    {
        UINT16                                  cRnti;     //this info no need in tiny RAM
        UINT16                                  MMEGI;
        PS_Tai                                  registedTai;
        BOOL                                    regesteredMMEPresent;
        UINT8                                   gummeiType; //0: native; 1: mapped
        UINT16                                  accessClass;     //this info no need in tiny RAM
    }cerrcUeIdentity;

    //4 CerrcExtendDrxInfo
    struct
    {
        BOOL                                    eDrxCyclePresent;
        UINT8                                   eDrxCycle;
        UINT8                                   ptwLength;
        BOOL                                    reserved0;
        BOOL                                    eDRXAllowed;
        BOOL                                    specificDrxCyclePresent;
        UINT8                                   specificDrxCycle;
        BOOL                                    reserved1;
    }cerrcExtendDrxInfo;
}
PS_PART9_CeRrcState_UeId;

//PlmnIdentityInfo
typedef struct _EPAT_PS_CERRC_PlmnIdentityInfoTag
{
    PS_Plmn                                 plmnIdentity;
    BOOL                                    cellReserved4OperatorUse;
    BOOL                                    attachWithoutPDNConnectivity; //4 only apply for NB-IoT
}PS_CERRC_PlmnIdentityInfo;

//CellAccessRelatedInfo
typedef struct _EPAT_PS_CERRC_CellAccessRelatedInfoTag
{
    BOOL                                    csgIndication;
    UINT8                                   numOfPlmnIdentityInfo;
    PS_CERRC_PlmnIdentityInfo               plmnIdentityInfoList[6];
    UINT32                                  cellIdentity;
    UINT16                                  trackingAreaCode;
    BOOL                                    cellBarred;
    BOOL                                    intraFreqReselAllowed;
}PS_CERRC_CellAccessRelatedInfo;

//CerrcCellSelectionInfo
typedef struct _EPAT_PS_CERRC_CellSelectionInfoTag
{
    INT8                                    qRxLevMin;
    BOOL                                    qQualMinPresent;
    INT8                                    qQualMin;
}PS_CERRC_CellSelectionInfo;

//4 CerrcStoredMibInfo
typedef struct _EPAT_PS_CERRC_MibInfoTag
{
    BOOL                                    abEnabled;
    UINT8                                   sysInfoValueTag;
    UINT8                                   dlBandwidth;
    UINT8                                   reserved;
}PS_CERRC_MibInfo;

typedef struct _EPAT_PS_PART10_ServingCellInfo
{
    //4 OFFSETOF(ServingCellInfo, systemInfo)
    UINT32                                  carrierFreq;
    UINT16                                  phyCellId;
    UINT16                                  reserved;
    //CerrcSysInfoCollection
    struct
    {
        UINT32                                  siUpdateTime;
        UINT16                                  siRcvingBitmap;
        UINT16                                  siRequiredBitmap;
        UINT16                                  siValidBitmap;
        UINT16                                  siBackupBitmap;
        UINT8                                   waitSiUpdate;
        UINT8                                   siUpdatePending;
        UINT8                                   sib1UpdatePending;
        UINT8                                   reserved0;
        void                                    *pMib;
        void                                    *pSib1;
        void                                    *pSib2;
        UINT32                                  sib2CarrierFreq;
        UINT16                                  sib2PhyCellId;
        UINT16                                  sib2SiDataLen;
        UINT8                                   *pSib2SiData;
    }siCollection;
    //CerrcServCellCampPara
    struct
    {
        UINT32                                  campOnTime;
        UINT32                                  criterionSTime;
        UINT32                                  nservTimeLength;
        UINT32                                  escapeTimeLength;
        UINT32                                  unsuitableStartTime;
        BOOL                                    unsuitableTimeRunning;
        UINT8                                   numOfRrcReject;
        UINT8                                   numOfT300Expired;
        UINT8                                   cellChangeCause;    //CerrcCellChangeCause
    }campPara;
    //CerrcServCellMeasPara
    struct
    {
        UINT8                                   measAction;
        BOOL                                    unreliableFlag;
        UINT8                                   rsvd[2];
        //CerrcServCellMeasInfo
        struct
        {
            INT16                                   rsrp;
            INT16                                   rsrq;
            INT16                                   s;
            INT16                                   sQual;
            INT16                                   rank;
            BOOL                                    measValid;
            UINT8                                   ptwInd;
            UINT8                                   measIndCnt;
            BOOL                                    snrPresent;
            INT8                                    snr;    //useless for ERRC
            UINT8                                   riskPercent;
            UINT16                                  deltaRsrp;
            UINT16                                  deltaRsrq;
        }servCellMeasInfo;
        //CerrcRmMeasParas
        struct
        {
            //CerrcSleep2RmParas
            struct
            {
                INT16                                   srxlevRef;
                BOOL                                    bIntraMeasRmEnable : 4;
                BOOL                                    bInterMeasRmEnable : 4;
                BOOL                                    bRMCriterionFulfilled;
                UINT32                                  rMCriterionSuccTime;
                UINT32                                  rMCriterionFailTime;
            }sleep2RmParas;
            UINT16                                  tSearchDeltaP;
            UINT32                                  intraMeasStartTime;
            UINT32                                  interMeasStartTime;
            UINT32                                  intraMeasStopTime;
            UINT32                                  interMeasStopTime;
        }rmMeasPara;
        INT16                                   sIntraSearchP;
        INT16                                   sNonIntraSearchP;
        INT16                                   sIntraSearchQ;
        INT16                                   sNonIntraSearchQ;
        UINT8                                   higherPriorityBitmap;
        UINT8                                   lowerPriorityBitmap;
        BOOL                                    bLowerPriorityNeeded;
        BOOL                                    bLowerPriorityOngoing;
    }measPara;

    //4 CerrcStoredMibInfo
    PS_CERRC_MibInfo                            mibInfo;

    //4 OFFSETOF(CerrcStoredSib1Info, accessInfoList)
    struct
    {
        PS_CERRC_CellAccessRelatedInfo          accessInfo;
        UINT8                                   selectedPlmnIdx;
        PS_CERRC_CellSelectionInfo              cellSelectionInfo;
        UINT8  /* 1 to 8 */                     qRxLevMinOffset;
        UINT8  /* 1 to 8 */                     qQualMinOffset;
        UINT8                                   freqBandIndicator;
        INT8                                    pMax; //Need OP
        UINT8                                   pCom;
        UINT8                                   ceAuthorisationOffset;
        UINT8                                   siSchdNumInfo;
        UINT16                                  sysInfoValueTagSIList;
        UINT16                                  sibTypeMapBitmap[8];
        UINT8                                   numOfMultiBands;
        UINT8                                   multiBands[3];
        UINT8                                   selectedBandIndicator;
        UINT8                                   sib14SiIdx;
        BOOL                                    siValidityTimePresent;
        BOOL                                    imsEmergencySupport;
        BOOL                                    eCallOverIMSSupport;
        BOOL                                    eDRXAllowed;
        UINT8                                   numOfAccessInfo;
    }sibInfo;
}
PS_PART10_ServingCellInfo;

//PlmnIdentityInfo
typedef struct _EPAT_PS_RRC_CerrcBarredCellInfoTag
{
    UINT32                                  carrierFreq;
    UINT16                                  phyCellId;
    UINT16                                  barTimeLength;
    UINT32                                  barStartTime;
    BOOL                                    isBarTimeRun;
    UINT8                                   barCause;
}PS_RRC_CerrcBarredCellInfo;

typedef struct _EPAT_PS_PART11_CerrcCellLockContext
{
    //3 CerrcCellLockContext
    struct
    {
        BOOL                                    isEnable;
        UINT8                                   mode;
        UINT16                                  phyCellId;//0~503, 0xFFFF means invalid
        UINT32                                  carrierFreq;
    }cellLockContext;

    //3 numberOfBarCell
    UINT8                                   numberOfBarCell; //indicate the number of barred cell
    UINT8                                   numberOfQoffsetTemp; //indicate the number of valid QoffsetTemp
    UINT8                                   numberOfRiskCell; //indicate the number of valid riskCellList
    UINT8                                   reserved;

    //3 CerrcBarredCellInfo
    PS_RRC_CerrcBarredCellInfo              barCellList[4]; //save the barred cell list
}
PS_PART11_CerrcCellLockContext;

//CerrcSiSchdInfo
typedef struct _EPAT_PS_CERRC_CerrcSiSchdInfoTag
{
    UINT8                                   siPeriodicity;
    UINT16                                  sibTypeMapBitmap;
    UINT8  /* 0 to 31 */                    oriSiIndex;
}PS_CERRC_CerrcSiSchdInfo;

//CerrcDetectedCellInfo
typedef struct _EPAT_PS_CERRC_CerrcDetectedCellInfoTag
{
    UINT32                                  carrierFreq;
    UINT16                                  phyCellId;
    INT16                                   nRsrp;
    INT16                                   nRsrq;
    INT8                                    snr;
    INT16                                   s;
    INT16                                   sQual;

    //CerrcStoredMibInfo
    PS_CERRC_MibInfo                            mibInfo;

    //CerrcSib1Info
    struct
    {
        PS_CERRC_CellAccessRelatedInfo          cellAccessRelatedInfo;
        UINT8                                   selectedPlmnIdx;
        PS_CERRC_CellSelectionInfo              cellSelectionInfo;
        UINT8                                   freqBandIndicator;
        UINT8                                   numOfMultiBands;
        UINT8                                   multiBands[3];
        BOOL                                    freqBandIndicatorPriority;
        UINT8                                   selectedBandIndicator;
        UINT8  /* 0 to 31 */                    sysInfoValueTag;
        UINT8                                   siWindowLength;
        UINT8                                   sib14SiIdx;
        UINT8                                   numOfSchedulingInfo;
        PS_CERRC_CerrcSiSchdInfo                siSchdInfoList[8];
        BOOL                                    pMaxPresent;
        INT8                                    pMax; //Need OP
        UINT8                                   pCom;
        UINT8                                   ceAuthorisationOffset;
        BOOL                                    imsEmergencySupport;
        BOOL                                    eCallOverIMSSupport;
        BOOL                                    eDRXAllowed;
        BOOL                                    tddConfigPresent;
        UINT8                                   tddSfAssignment;
        UINT8                                   tddSsp;
        UINT8                                   numOfCellAccessRelatedInfoR14;
        PS_CERRC_CellAccessRelatedInfo          cellAccessRelatedInfoList[5];
        UINT8                                   selectedAccessInfoIdx;
        INT8                                    qQualMinWB;
        INT8                                    qQualMinRSRQOnAllSymbols;
        UINT8  /* 1 to 8 */                     qRxLevMinOffset;
        UINT8  /* 1 to 8 */                     qQualMinOffset;
        BOOL                                    hyperSFNPresent;
        UINT16    /* 0 to 1023 */               hyperSFN;
        BOOL                                    siValidityTimePresent;
        BOOL                                    sysInfoValueTagSIPresent;
        UINT16                                  sysInfoValueTagSIList;
    }sib1Info;
}
PS_CERRC_CerrcDetectedCellInfo;


typedef struct _EPAT_PS_PART12_CerrcPlmnSearchContext
{
    //3 OFFSETOF(CerrcPlmnSearchContext, pList)
    UINT8                                   plmnSearchState;
    BOOL                                    isHighLevelDone;
    UINT8                                   plmnBitmap;
    BOOL                                    requestedPlmnValid;
    PS_Plmn                                 requestedPlmn;
    PS_CERRC_CerrcDetectedCellInfo          currDetectedCell;
}PS_PART12_CerrcPlmnSearchContext;


typedef struct _EPAT_PS_PART12_CerrcCellSearchContext
{
    //3 OFFSETOF(CerrcCellSearchContext, numOfInterFreq)
    PS_CERRC_CerrcDetectedCellInfo          currDetectedCell;
    void                                    *pendingSignal;
}PS_PART12_CerrcCellSearchContext;


//CerrcCellStatus
typedef enum _EPAT_PS_CERRC_CellStatusTag
{
    PS_CELL_STATUS_SIB1_INVALID = 0, // 0: sib1 CRC NOK or ASN.1 decode error
    PS_CELL_STATUS_SUITABLE = 1, // 1: cell is suitable
    PS_CELL_STATUS_BARRED = 2, // 2: cell is indicated with "cellBarred"
    PS_CELL_STATUS_PLMN_FORBIDDEN = 3, // 3: cell is not barred, plmn is forbidden
    PS_CELL_STATUS_TA_FORBIDDEN = 4, // 4: cell is not barred and plmn is not forbidden, but TA is forbidden
} PS_CERRC_CellStatus;

typedef struct _EPAT_PS_PART12_CerrcCellReselTargetCellInfo
{
    //3 OFFSETOF(CerrcCellReselTargetCellInfo, sib2Info)

    PS_CERRC_CellStatus                     cellStatus;

    //4 CerrcDetectedCellInfo
    PS_CERRC_CerrcDetectedCellInfo          cellBaseInfo;
}PS_PART12_CerrcCellReselTargetCellInfo;


//CerrcReestablishmentCause
typedef enum _EPAT_PS_CERRC_ReestablishmentCauseTag
{
    PS_REEST_RECONFIGURATION_FAILURE = 0,
    PS_REEST_HANDOVER_FAILURE = 1,
    PS_REEST_RADIO_LINK_FAILURE = 2,
    PS_REEST_INTEGRITY_CHECK_FAILURE = 3,
} PS_CERRC_ReestablishmentCause;

typedef struct _EPAT_PS_PART12_CerrcConnReEstContext
{
    //3 CerrcConnReEstContext
    UINT16                                  phyCellId;    // source PCI
    UINT16                                  cRntiSource;    // source CRNTI
    PS_CERRC_ReestablishmentCause           reestblishementCause;
    UINT16                                  xDlNasMac;
    BOOL                                    bRaSucc;
    UINT8                                   reserved;
    UINT8                                   bandSearchRound;
}PS_PART12_CerrcConnReEstContext;

typedef struct _EPAT_PS_CERRC_RlcAmConfigTag
{
    UINT8           pollPDU;        //PollPduAm
    UINT8           pollByte;       //PollByteAm
    UINT8           pollPDUv1310;   //PollPduAm
    UINT8           pollByter14;     //PollByteAm
    UINT16          tPollRetransmit : 6;
    UINT16          maxRetxThreshold : 4;
    UINT16          ulExtLI : 1;
    UINT16          dlExtLI : 1;
    UINT16          ulExtSn : 1;
    UINT16          dlExtSn : 1;
    UINT16          enableStatusReportSNGap : 1;
    UINT16          rsvd : 1;
    UINT8           tReordering;        //Reordering
    UINT8           tStatusProhibit;        //StatusProhibit
}PS_CERRC_RlcAmConfig;

typedef struct _EPAT_PS_CERRC_RlcUmBiDirConfigTag
{
    UINT8           ulSnFieldLen;
    UINT8           dlSnFieldLen;
    UINT8           tReordering;   //Reordering
    UINT8           rsvd;
}PS_CERRC_RlcUmBiDirConfig;

typedef struct _EPAT_PS_CERRC_RlcUmUniDirUlConfigTag
{
    UINT8           ulSnFieldLen;
    UINT8           rsvd0;
    UINT16          rsvd1;
}PS_CERRC_RlcUmUniDirUlConfig;

typedef struct _EPAT_PS_CERRC_RlcUmUniDirDlConfigTag
{
    UINT8           dlSnFieldLen;
    UINT8           tReordering;  //Reordering
    UINT16          rsvd;
}PS_CERRC_RlcUmUniDirDlConfig;

typedef union _EPAT_PS_CERRC_RlcConfigTag
{
    PS_CERRC_RlcAmConfig            amCfg;
    PS_CERRC_RlcUmBiDirConfig       umCfg;
    PS_CERRC_RlcUmUniDirUlConfig    umUlCfg;
    PS_CERRC_RlcUmUniDirDlConfig    umDlCfg;
}PS_CERRC_RlcConfig;

typedef struct _EPAT_PS_CERRC_LogicalCHConfigTag
{
    BOOL    priorityPresent;
    UINT8   priority;           /*1..16*/
    BOOL    logicalChannelSRProhibit;
    UINT8   reserved0;
    UINT8   prioritisedBitRate;
    UINT8   bucketSizeDuration;
    UINT8   logicalChannelGroupType;    // UpConfigType
    UINT8   logicalChannelGroup;        /* 0..3 */
    BOOL    logicalChannelSR_Mask;      // TRUE is Setup
    UINT8   bitRateQueryProhibitTimerType;//UpConfigType need OR
    UINT8   bitRateQueryProhibitTimer;      //BitRateQueryProhibitTimer
    UINT8   reserved1;
}PS_CERRC_LogicalCHConfig;   //12 bytes

typedef struct _EPAT_PS_CERRC_SrbInfoTag
{
    UINT8       type;           //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8       logicIdentity;  //SRB1: 1, SRB2: 2, SRB1bis: 3
    UINT8       rlcCfgMode;     //RlcConfigModeType
    UINT8       rsvd;
    PS_CERRC_RlcConfig          rlcConfig;
    PS_CERRC_LogicalCHConfig    logicConfig;
}PS_CERRC_SrbInfo;

typedef struct _EPAT_PS_CERRC_Key128Tag
{
    UINT8    content[16];    //128 bit Key
}
PS_CERRC_Key128;

typedef struct _EPAT_PS_CERRC_Key256Tag
{
    UINT8    content[32];    //256 bit Key
}
PS_CERRC_Key256;

//CerrcSecurityInfo
typedef struct _EPAT_PS_CERRC_SecurityInfoTag
{
    PS_CERRC_Key128                         KrrcInt128; //store the KrrcInt for generating short-MAC-I in re-estRequest/resumeRequest
    PS_CERRC_Key256                         KeNB;     //the latest derived KeNB, during SMC, re-est, resume or HO procedure
    PS_CERRC_Key256                         KeNBInitial;//the initial KeNB derived during the security activation procedure (SMC)
    PS_CERRC_Key256                         NH;       //the fresh sync NH
    BOOL                                    bInitialNH; //indicate if it is the initial NH derivation, set to TRUE during the security activation procedure (SMC)
    UINT8                                   currentHopChainingCount; //the previous nextHopChainingCount from NW, set to 0 in SMC procedure
    UINT8                                   cipAlgo;    //CipheringAlgorithm
    UINT8                                   intAlgo;    //IntegrityAlgorithm
}PS_CERRC_SecurityInfo;

typedef struct _EPAT_PS_PART12_CerrcHandoverContext
{
    //3 CerrcHandoverContext
    UINT32                                  targetPCellCarrierFrequency;
    UINT16                                  targetPCellPhysicalCellId;
    UINT16                                  targetPCellCrnti;
    UINT8                                   makeBeforeBreak;
    UINT8                                   t304;
    //SrbInfo
    PS_CERRC_SrbInfo                        srbInfo[2];
    void                                    *pDrbInfo[11];
    //CerrcSecurityInfo
    PS_CERRC_SecurityInfo                   securityInfo;
    UINT8                                   mibPresent : 1;
    UINT8                                   sib1Present : 1;
    UINT8                                   rachSkipPresent: 1;
    UINT8                                   rachConfigPresent : 1;
    UINT8                                   rsvd : 4;
    //CerrcMibInfo
    PS_CERRC_MibInfo                        mibInfo;
    //store SIB1 information of the target cell
    void                                    *pSib1Info;
    //RACHConfigDedicated
    struct
    {
        UINT8                           raPreambleIndex;/* 0 to 63 */
        UINT8                           raPRACHMaskIndex;/* 0 to 15 */
        UINT8                           rsvd[2];
    }rachConfig;
    //RachSkipConfig
    struct
    {
        UINT8       ta0R14;
        UINT8       mcgPtagR14;
        UINT8       scgPtagR14;
        UINT8       mcgStagR14;
        UINT8       scgStagR14;
        UINT8       reserved[2];
        BOOL        rachSkipUlConfigPresent;
        //RachSkipUlConfigInfoR14
        struct
        {
            UINT8       numOfConfUlProcessesR14;      // INTEGER (1..8)
            UINT8       ulStartSubframeR14;           // INTEGER (0..9)
            UINT16      ulGrantR14;                   // BIT STRING (SIZE(16))
            UINT8       ulSchedIntervalR14;           // RachSkipUlSchedIntervalR14
            UINT8       reserved[3];
        }rachSkipUlConfig;
    }rachSkipConfig;
    void                                    *pRrcConnRfg;
    void                                    *hoIeMemInfo;
    UINT8                                   rlfReportLength;
    UINT8                                   rlfReport[100];
}PS_PART12_CerrcHandoverContext;


typedef struct _EPAT_PS_PART12_CerrcConnReleaseContext
{
    //3 CerrcConnReleaseContext
    UINT8                                   leavConnCause; //LeaveConnectedCause
    BOOL                                    relCnfNeeded;
    BOOL                                    nwRelPresent;
    BOOL                                    redirectedCarrierFreqPresent;
    UINT32                                  redirectedCarrierFreq;
    BOOL                                    extendedWaitTimePresent;
    UINT16                                  extendedWaitTime; /*1 to 1800, Value in seconds*/
    BOOL                                    extendedWaitTimeCpDataPresent; //be TRUE only if the extendedWaitTimePresent is FALSE; otherwise is FALSE.
    UINT16                                  extendedWaitTimeCpData;  /*1 to 1800, Value in seconds*/
    UINT8                                   t320;
    UINT8                                   reserved;
}PS_PART12_CerrcConnReleaseContext;


typedef struct _EPAT_PS_PART12_CerrcConnMeasContext
{
    //3 CerrcConnMeasContext
    UINT8          measObjectChange : 1;//record if measObject has changed, inform L1 if changed
    UINT8          measIdChange : 1;//record if measId has changed, reconstruct self defined measIdListDefined, restart intra/intrFreq measurement
    UINT8          siRequestForHo : 1;
    UINT8          onlyScellEnableFlag : 1; //only configure event A1, A2
    UINT8          intraFreqEnableFlag : 1;
    UINT8          interFreqEnableFlag : 1;
    UINT8          intraFreqBlackCellFlag : 1; //add or remove black cell in measObjectEUTRA
    UINT8          validMeasGapFlag : 1; //phy has valid measGap or not
    UINT16         T321Length;//T321 timer length, unit:ms
    UINT8          startT321MeasId;//record the measId which start T321 timer

    UINT8          interFreqIndexBitmap;//used for restart measure
    UINT32         interFreqCarrier[8];//used for reconfig measure Object
    void           *varMeasReportList;
}PS_PART12_CerrcConnMeasContext;


//CerrcRceType
typedef enum _EPAT_PS_CERRC_RceTypeTag
{
    PS_RCE_ESTABLISH = 0,
    PS_RCE_RESUME = 1,
    PS_RCE_RESUME_TO_ESTABLISH = 2,
} PS_CERRC_RceType;

//CerrcRceFailReason
typedef enum _EPAT_PS_CERRC_RceFailReasonTag
{
    PS_RCE_FAIL_REASON_T300_EXPIRY = 1,
    PS_RCE_FAIL_REASON_NO_RAR = 2,
    PS_RCE_FAIL_REASON_NW_REJECT = 3,
    PS_RCE_FAIL_REASON_MM_RELEASE = 4,
    PS_RCE_FAIL_REASON_CELL_RESELECT = 5,
    PS_RCE_FAIL_REASON_PHY_RESET = 6,
} PS_CERRC_RceFailReason;

typedef struct _EPAT_PS_PART13_CerrcConnEstContext
{
    //3 CerrcConnEstContext
    PS_CERRC_RceType                        rceType;
    PS_CERRC_RceFailReason                  rceFailReason;
    UINT16                                  nasMessageLength;
    UINT8                                   *pNasMessage;   //point to NAS message received from CEMM
    UINT8                                   nasMessageIndex; //RRC PDU ID, range(1..5, 17..255)
    BOOL                                    bAttachWithoutPDNConnectivity;
    BOOL                                    bUpCIoTOptimisation;
    UINT8                                   raFailCounter;
    BOOL                                    extendedWaitTimePresent;
    UINT16                                  extendedWaitTime;
    BOOL                                    rrcSuspendIndication;
    UINT8                                   dcnIdPresent;
    UINT16                                  dcnId;
    UINT8                                   dataRaiInfo;
    UINT8                                   establishmentCause; //CEstablishmentCause
    void                                    *pendingSignal;
    UINT8                                   rrcTransactionIdentifier;//used for resume procedure
    BOOL                                    bSib14Updated;
    UINT8                                   sib14SiBitmap;
    BOOL                                    bMibUpdated;
    BOOL                                    bT300IsRunning;
    BOOL                                    bRaSucc;
    UINT8                                   callType; //CallType
    BOOL                                    bCpCIoTOptimisation;
    BOOL                                    subjectToEab;
    BOOL                                    subjectToAcdc;
    UINT8                                   acdcCategory;
    UINT8                                   t302;
    UINT8                                   t325;
    UINT8                                   numberOfPreamblesSent;
    BOOL                                    contentionDetected;
    BOOL                                    maxTxPowerReached;
}PS_PART13_CerrcConnEstContext;


typedef struct _EPAT_PS_PART13_CerrcConnContext
{
    //3 CerrcConnContext
    UINT8                                   numOfOutSync;
    UINT8                                   numOfInsync;
    BOOL                                    bShortenT310;
    BOOL                                    inSyncOngoing;
    //CerrcRlfTimersAndConstants
    struct
    {
        BOOL                                    bFromDedicatedCfg;
        UINT8                                   t301;
        UINT8                                   t310;
        UINT8                                   t311;
        UINT8                                   n310;
        UINT8                                   n311;
        UINT8                                   t301_r9;
        UINT8                                   t310_r9;
    }rlfTimersAndConstants;
    //SrbInfo
    PS_CERRC_SrbInfo                        srbInfo[2];
    void                                    *pDrbInfo[11];
    //LogicalChannelSrProhibitTimer
    struct
    {
        UINT8       type; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
        UINT8       value;  // index of MAC_MainConfig_NB_r13_logicalChannelSR_ProhibitTimer_r13
    }logicChProTimer;  // from MAC_MainConfig_NB_r13
    BOOL                                    raiActivation;    // from MAC_MainConfig_NB_r13, Need OR
    BOOL                                    nwConfigtwoHarq;   // from physicalConfigDedicated_r13, Need OR
    BOOL                                    nwConfigRaiActivation;   // from MAC_MainConfig_NB_r13, Need OR
    UINT8                                   dataInactivityTimer;
    BOOL                                    resumeCnfNeed;
    BOOL                                    nasUlMsgResumeIndNeed;
    BOOL                                    bNasMsgInd;
    UINT32                                  ulInfoTransBitmap[8];
    UINT16                                  dlBler;
    void                                    *pendingSignal;
    UINT8                                   drxConfigTypeRecord;
    UINT8                                   onDurationTimer;
    UINT8                                   drxInactivityTimer;
    UINT8                                   drxRetransmissionTimer;
    UINT8                                   longDrxCycle;
    UINT16                                  drxStartOffset;
    UINT8                                   shortDrxConfigType;
    UINT8                                   shortDrxCycle;
    UINT8                                   shortCycleTimer;
    BOOL                                    perCCGapIndicationRequest;
    UINT16                                  longCdrxCycle;       //ms
    UINT16                                  shortCdrxCycle;      //ms
    void                                    *tempIeMemInfo;
    BOOL                                    bSaveDecMem;
    UINT8                                   numOfIeMemInfoForPhy;
    void                                    *ieMemInfoForPhy[8];
    UINT8                                   countCheckTransactionId;
    UINT8                                   phrConfigType; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8                                   extendedBSRSizesType; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8                                   extendedPHRType; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8                                   extendedPHR2Type; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8                                   srCfgType;  //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    UINT8                                   rachSkipType; //UpConfigType, 0~6 indicate: not change, setup, setup_resume, modify, releasse, release_suspend, invalid
    BOOL                                    fullConfigAdditPhyDedicated; //fullconfig and phyDedicatedConfig in a rrcRecfg
    UINT8                                   numberOfPreamblesSent;
    BOOL                                    contentionDetected;
    BOOL                                    maxTxPowerReached;
}PS_PART13_CerrcConnContext;


//CerrcActStatus
typedef enum _EPAT_PS_CERRC_ActStatusTag
{
    PS_CERRC_ACT_NO_SERVICE = 0,
    PS_CERRC_ACT_SUCCESS = 1,
    PS_CERRC_ACT_INVALID = 0x7FFFFFFF,
} PS_CERRC_ActStatus;

typedef struct _EPAT_PS_PART14_CerrcActStatus
{
    //3 CerrcActStatus
    PS_CERRC_ActStatus                          actStatus;

    //3 CerrcStoredPlmnBandInfo
    struct
    {
        BOOL                                    requestedPlmnValid;
        BOOL                                    bImsEmergency;
        UINT8                                   numOfBand;
        UINT8                                   reserved;
        PS_Plmn                                 requestedPlmn;
        UINT8                                   bandIndList[32];
    }plmnBandInfo;

    //3 LastCampCellInfo
    struct
    {
        UINT32                                  carrierFreq;
        UINT16                                  phyCellId;
        UINT16                                  siValidBitmap;//pending SI updating REQ during cell reselection
        UINT8                                   sysInfoValueTag;
        UINT8                                   siSchdNumInfo;
        UINT16                                  sysInfoValueTagSIList;
        UINT32                                  siUpdateTime;
    }lastCampCell;
}PS_PART14_CerrcActStatus;


typedef struct _EPAT_PS_PART15_CerrcSecurityInfo
{
    //3 CerrcSecurityInfo
    PS_CERRC_SecurityInfo                   securityInfo;
}PS_PART15_CerrcSecurityInfo;


// UE capability info.
typedef struct _EPAT_PhyUeCapabilityTag
{
    // 3GPP release version of AS
    UINT8                               asRelease;
    // UE category
    UINT8                               category;
    // used to indicate if delta RSRP/RSRQ was taken effect
    // when reporting cephySysInfoInd(SIB1) and cephyCellMeasInd of serving cell
    BOOL                                weakCellCampOptEn;
    // static config, following optimization under staticConfig:
    //  1 strengthen filter of measurement result from 0.25 to 0.0625, both for connect and idle
    //  2 statis the dec failure of paging
    BOOL                                bStaticConfig;

    // get UE quality level from RRC configuration
    // PHY enum value is aligned with range of AT+ECCFG = 'qualityFirst', qualityLevel
    UINT8                               qualityLevel;
    UINT8                               rsvd[3];
}PhyUeCapability_s;


// time offset typedef
typedef struct _EPAT_T_TIME_OFST
{
    UINT32  hfnsfn;       // hfn(10)|sfn(10); for BaseTime(AP used), 8 bits for OFN at bit[27:20]
    UINT32  sbnspn;       // sbn(4)|spn(15)
} T_TIME_OFST_s;


// cell base info.
typedef struct _EPAT_PhyCellBaseInfoTag
{
    // DL EU-ARFCN
    UINT32                              dlEuArfcn;

    // DL center frequency, [100Hz], dlCentFreq = carrierCentFreq - PHY_RF_RX_LOWIF_FREQ
    UINT32                              dlCentFreq;

    // Physical cell ID
    UINT16                              phyCellId;
    // high or low band indicator of center frequency
    UINT8                               highLowBandInd;
    // indicate downlink RFPLL LO divider.
    UINT8                               dlRfPllLoDiv;

    // RF PLL feedback divider ratio for DL = LoDiv*dlCenterFreq*2^23/26e6, where
    // [31:24] integer, [23:0] fractional
    // NOTE:  baseInfo.dlRfPllDivRatio do not including freqError, so should calc rfPara->pllDivRatio to config RF(rsx_div_ratio)
    //        rfPara->pllDivRatio = baseInfo.dlRfPllDivRatio + rfPara->freqErrIdx;
    UINT32                              dlRfPllDivRatio;

    // time offset of the cell, should be stored in Flash!
    // NOTE: this is RT_OFST_SW, not RT_OFST_HW configured in TMU BC;
    //       there are 3 timeofst, RT_OFST_SW, RT_OFST_HW, BT_OFST_HW, xxx_HW are value configured in TMU; 
    //             RT_OFST_SW = RT_OFST_HW - BT_OFST_HW
    //       when power on, all these 3 timeOfst are 0;
    //       RT_OFST_SW and RT_OFST_HW are adjust simultaneously during cell search(FRS, SSS, MIB, SIB1);
    //       then when UE camped on a serving cell, RT_OFST_SW should never changed, 
    //       but RT_OFST_HW and BT_OFST_HW should adjust simultaneously
    //       when AP&CP sleep, AP will config BcLdVal to remove BT_OFST_HW(set to 0)
    //       when wakeup from sleep1/2/hib, CP should config timeOfstHw as timeOfstSw(restore from AON mem); BT_OFST_HW = 0 by default value after wakeup
    T_TIME_OFST_s                        timeOfstSw;

    // this is RT_OFST_HW configured in TMU BC, no need to write Flash when this value changed
    // store this value for HW task controlling judgement, especially, switch HW task of NCELL to that of SCELL
    // for SCELL, timeOfstHw = timeOfstSw + BT_OFST_HW
    // otherwise, timeOfstHw = timeOfstSw
    // cleared in PhyCellBaseInfoIcsAdd;  
    // changed in:
    //            PhyTimeOfstUpdtAftFrs/PhyTimeOfstUpdtAftSss:                                sbnspn only
    //            PhyTimeOfstUpdtAftMib/PhyTimeOfstUpdtAftSib1/PhyPresyncTimingAdjForMib:     hfnSfn only
    //            PhyTimeOfstUpdtAftAtc/PhyPresyncTimingAdjToLast/PhyPresyncTimingAdjXrs:     all
    T_TIME_OFST_s                        timeOfstHw;

    // RSRP and RSSI, [dBm, FWL = 2], need write to AON register when go to hibernate
    // remove FWL part when storing to AON REG
    INT16                               rsrp;
    INT16                               rssi;

    // SNR, [dB, FWL = 2], only be used by PhyMeasCellInfo
    // 0x7FFF means this value is invalid
    INT16                               snr;
    INT16                               rsvd;

    // frequency error, should be stored in AON register
    // unit is ppm, format is (iwl,fwl)=(7,10)
    // format is changed to (iwl,fwl)=(7,9) when being stored into AON
    INT32                               freqErrPPM;

    // remainder freqErr of this module, only updated before ICS FRS/RSC/FRS4RSC, for TCXO, it is a RF cali data; for DCXO, it is calc according current Tempr and DCXO Cx
    // move to gPhyIcsSchdInfo
    //INT32                               freqErrInit;

    // CP type: 0-ECP; 1-NCP
    UINT8                               cpType;
    // duplex mode: 0-FDD; 1-TDD
    UINT8                               duplexMode;
    // blocker signal eixsted or not, set by CE and clear by RxDFE
    UINT8                               blkrExist;
    // derived from dlEuArfcn according to Table 5.5-1 of TS36.101
    // set to freqBandIndicator after getting SIB1
    // used to configure RF
    UINT8                               bandNum;
}PhyCellBaseInfo_s;


typedef struct _EPAT_PhyPreSCellBaseInfoTag
{
    UINT16                              prePhyCellId;
    UINT16                              rsvd;
    UINT32                              preDlEuArfcn;
    T_TIME_OFST_s                       preTimeOfstHw;
}PhyPreSCellBaseInfo_s;

// cell MIB info.
typedef struct _EPAT_PhyCellMibInfoTag
{
    // transmission bandwidth, PRB number in downlink
    // need set to "BandWidth_n6"(non-RSC) or "BandWidth_n100"(RSC) before starting INIT/BGPLMN ICS
    // need set to PHY_MIN_DL_BW_IND_5MHZ(can NOT be used for NCMIB in CONN)
    // or PHY_MIN_DL_BW_IND_1_4MHZ after SSS detection
    // because this value will be used in the following MIB receiving
    UINT8                               dlBandwidth;
    // PHICH duration, see Table 6.9.3-1 in TS36.211
    UINT8                               phichDuration;
    // PHICH resource, Ng value used in section 6.9 of TS36.211
    UINT8                               phichResource; 
    // based on Table 5.3.1.1-1 in TS36.212, get antenna port number from PBCH CRC mask
    // need set to 4-port as default after SSS detection
    // because this value will be used in the following MIB receiving
    UINT8                               antPortNum;
}PhyCellMibInfo_s;


// cell SIBx scheduling info.
typedef struct _EPAT_PhySibxParaTag
{
    // SI window length, value range:{1,2,5,10,15,20,40}, unit: ms
    UINT8                               siWinLen;
    // number of SI window
    // not all SI-win configured in SIB1 by NW, because RRC removed useless SI
    UINT8                               siNum;
    // siMinPrdMsk = (1<<minSiPeriod)-1
    //UINT8                               siMinPrdMsk;
    UINT8                               rsvd[2];

    // (1<<siPeriod[n]) - 1
    UINT8                               siPrdMsk[8];

    // original SI index configured in SIB1 by NW
    UINT8                               siOrgnlIdx[8];
}PhySibxPara_s;


// cell SIB1 info.
typedef struct _EPAT_PhyCellSib1InfoTag
{
    // Pmax value, used to restrict max transmission power of uplink
    INT8                                pMax;
    // band indicator, see TS36.101 section 5.5
    // value range: 1~256, now support to 255
    UINT8                               freqBandIndicator;
    // UL-DL configurations for TDD, see Table 4.2-2 in TS36.211
    // default value should be set as PHY_TDD_UL_DL_CFG_0
    // FDD: set to PHY_TDD_UL_DL_CFG_MAX_NUM @ updating SIB1
    UINT8                               ulDlConfig;
    // configuration of special subframe for TDD, see Table 4.2-1 in TS36.211
    // default value should be set as PHY_TDD_SS_CFG_0
    // FDD: set to PHY_TDD_SS_CFG_MAX_NUM @ updating SIB1
    UINT8                               specialSbfrmPatterns;

    // derived from FALSH data PhyTddUlDlConfigTab based on ulDlConfig
    // Bit19~0 represents subframe type of SBN0~9, 2-bit for each subframe
    /*
        *    19      18      17      16               3       2       1       0
        * +-------+-------+-------+-------+-------+-------+-------+-------+-------+
        * |     SBN#0     |     SBN#1     | ..... |     SBN#8     |     SBN#9     |
        * +-------+-------+-------+-------+-------+-------+-------+-------+-------+
    */
    UINT32                              tddSbfrmType;

    // used to record HFN value in PHY CONN state
    // PHY CONN state: set to new HFN value if HFN is changed via SIB1 CONFIG
    //                 and this new HFN value will take effect @ CONN2IDLE
    // otherwise: set to invaid value 0xFFFF
    UINT16                              newHfn;
    // TRUE means HFN is configured in SIB1, used to check if eDRX configuration is valid or not
    BOOL                                hfnPresent;
    // indicate if RSSI/RSRQ measurement is performed on all OFDM symbols or not
    // see 5.1.3 of TS36.214
    // if FALSE, UE shall RSSI/RSRQ is only measured from OS contained CRS of port 0
    BOOL                                rsrqOnAllSymbols;

    // qRxLevMin in SIB1. Value in dBm, range: -156 ~ -22
    // we always set this value stored in PHY as 0xFFFF if value sent by RRC is NOT in range -156 ~ -22
    // use to revise RSRP reported to RRC in PHY IDLE state
    // in case the signal power of the antenna was just a little bit lower than SrxLev
    // leads to UE enter OOS in weak signal area
    INT16                               qRxLevMin;
    // qQualMin in SIB1. Value in dB, range:-34~-3 and 0xFF
    // we always set this value stored in PHY as 0xFF if value sent by RRC is NOT in range -34~-3
    // 0xFF is used and the UE does not take qQualMin into account for criterion S check
    // same purpose as qRxLevMin
    INT8                                qQualMin;
    UINT8                               rsvd;

    // SIBx scheduling parameters
    PhySibxPara_s                       sibxPara;
}PhyCellSib1Info_s;


// cell common info. obtained from SIB2 or Handover
typedef struct _EPAT_PhyPrachParaTag
{
    // RACH_ROOT_SEQUENCE, section 5.7.1 of TS36.211, value range: 0~837
    UINT16                              rootSequenceIdx;
    // prach-ConfigurationIndex, section 5.7.1 of TS36.211, value range: 0~63
    UINT8                               prachConfigIdx;
    // High-speed-flag, section 5.7.2 of TS36.211
    // indicate if unrestricted set(FALSE) or restricted set type A(TRUE) shall be used for PRACH preamble Set 1
    BOOL                                highSpeedFlag;

    // index for looking for N_CS configuration, see TS 36.211 [5.7.2: table 5.7.2-2] for preamble format 0..3
    // and TS 36.211 [5.7.2: table 5.7.2-3] for preamble format 4
    // value range: 0~15
    UINT8                               zeroCorrZoneConfig;
    // prach-FrequencyOffset, see TS 36.211 [5.7.1]. For TDD the value range is dependent on the value of prach-ConfigIndex
    // value range: 0~94
    UINT8                               prachFreqOfst;
    // Number of non-dedicated random access preambles in TS 36.321
    // value range: {1~16}*4
    UINT8                               numOfRaPreambles;
    // Size of the random access preambles group A in TS 36.321
    // value range: {1~15}*4
    UINT8                               sizeOfRaPreamblesGroupA;

    // Threshold for preamble selection in TS 36.321
    // value range: {56,144,208,256}(bits)
    UINT16                              messageSizeGroupA;
    // Threshold for preamble selection in TS 36.321
    // value range: {0xFF,0,5,8,10,12,15,18}(dB), 0xFF means -infinity
    INT8                                messagePowerOffsetGroupB;
    // Power ramping factor in TS 36.321
    // value range: {0,2,4,6}(dB)
    UINT8                               powerRampingStep;

    // Initial preamble power in TS 36.321
    // value range: {-120,-118,-116,-114,-112,-110,-108,-106,-104,-102,-100,-98,-96,-94,-92,-90}(dBm)
    INT8                                preambleInitialReceivedTargetPower;
    // Maximum number of preamble transmission in TS 36.321
    // value range: {3,4,5,6,7,8,10,20,50,100,200}
    UINT8                               preambleTransMax;
    // Duration of the RA response window in TS 36.321
    // value range: {2,3,4,5,6,7,8,10}(ms)
    UINT8                               raResponseWindowSize;
    // Timer for contention resolution in TS 36.321
    // value range: (1~8) * 8ms
    UINT8                               macContentionResolutionTimer;

    // maximum number of Msg3 HARQ transmissions in TS 36.321, used for contention based random access.
    // value range: 1~8
    UINT8                               maxHarqMsg3Tx;
    // preamble format 0~4
    UINT8                               preambleFormat;
    // indiates whether the resource is reoccurring in all radio framees, in even frames, in odd frames
    UINT8                               sfnAttr;
    // length of preamble signal
    UINT8                               preambleLength;  

}PhyPrachPara_s;


typedef struct _EPAT_PhyPuxchCommParaTag
{
    // Parameter N_PUCCH-(1) used in section 10.1 of TS36.213, value range: 0~2047
    UINT16                              n1PucchAn;
    // Parameter delta_PUCCH-shift used in section 5.4.1 of TS36.211, value range: {1,2,3}
    UINT8                               deltaPucchShift;
    // Parameter N_RB-(2) used in section 5.4 of TS36.211, value range: 0~98
    UINT8                               nRbCqi;

    // Parameter N_CS-(1) used in section 5.4 of TS36.211, value range: 0~7
    UINT8                               nCsAn;
    // Parameter Nsb in TS 36.211 [5.3.4], value range: 1~4
    UINT8                               nSb;
    // PUSCH hopping mode used in section 5.3.4 of TS36.211
    UINT8                               puschHoppingMode;
    // Parameter N_RB_HO used in section 5.3.4 of TS36.211, value range: 0~98
    UINT8                               puschHoppingOffset;

    // Parameter Group-hoppinp-enabled used in section 5.5.1.3 of TS36.211
    BOOL                                groupHoppingEnable;
    // Parameter delta_SS used in section 5.5.1.3 of TS36.211, value range: 0~29
    UINT8                               groupAssignmentPusch;
    // Parameter Sequence-hoppinp-enabled used in section 5.5.1.4 of TS36.211
    BOOL                                sequenceHoppingEnable;
    // Parameter cyclicShift used in Table 5.5.2.1.1-2 of TS36.211, value range: [0 2 3 4 6 8 9 10]
    UINT8                               cyclicShift;
}PhyPuxchCommPara_s;


typedef struct _EPAT_PhySrsCommParaTag
{
    // indicate SRS configuration is setup or release
    BOOL                                srsPresent;
    UINT8                               rsvd0;
    UINT8                               rsvd1;
    // Parameter: SRS Bandwidth Configuration. See TS 36.211, [table 5.5.3.2-1, 5.5.3.2-2, 5.5.3.2-3 and 5.5.3.2-4].
    // Actual configuration depends on UL bandwidth.
    UINT8                               srsBwConfig;

    // flag of CELL specified subframe reserved for SRS, low 10 bits is used (low first), which every i-th bit indicates whether sounding signal is in this subframe.
    // UL subframe & srsSbfrmIndicator
    UINT16                              srsSbfrmIndicator;
    // Parameter: Simultaneous-AN-and-SRS, see TS 36.213 [8.2].
    // indicate if ACK/NACK on PUCCH can be sent with SRS simultaneously
    BOOL                                ackNackSrsSimultaneousTransmission;
    // Parameter: srsMaxUpPts, see TS 36.211 [5.5.3.2]. optional present for TDD and absent for FDD
    // If this field is present, reconfiguration of m_SRS,0_max, applies for UpPts, otherwise reconfiguration does not apply.
    BOOL                                srsMaxUpPtsExist;
}PhySrsCommPara_s;


typedef struct _EPAT_PhyUlPowerCtrlCommParaTag
{
    // Parameter P_0_NOMINAL_PUSCH(1) used in section 5.1.1.1 of TS36.213
    // this field is applicable for non-SPS only
    // value range: -126~24, unit is dBm
    INT8                                p0NominalPusch;
    // Parameter P_0_NOMINAL_PUCCH used in section 5.1.2.1 of TS36.213
    // value range: -127~-96, unit is dBm
    INT8                                p0NominalPucch;
    // Parameter delta_PREAMBLE_Msg3 used in section 5.1.1.1 of TS36.213
    // value range: (-1~6)*2dB
    INT8                                deltaPreambleMsg3;
    // Parameter alpha used in section 5.1.1.1 of TS36.213, value range:{0,0.4,0.5,0.6,0.7,0.8,0.9,1}*10
    UINT8                               alpha;

    // Parameter delta_F_PUCCH(F) for PUCCH format 1, see section 5.1.2 of TS36.213
    // value range:{-2,0,2}(dB)
    INT8                                deltaFPucchF1;
    // Parameter delta_F_PUCCH(F) for PUCCH format 1b, see section 5.1.2 of TS36.213
    // value range:{1,3,5}(dB)
    INT8                                deltaFPucchF1b;
    // Parameter delta_F_PUCCH(F) for PUCCH format 2, see section 5.1.2 of TS36.213
    // value range:{-2,0,1,2}(dB)
    INT8                                deltaFPucchF2;
    // Parameter delta_F_PUCCH(F) for PUCCH format 2a, see section 5.1.2 of TS36.213
    // value range:{-2,0,2}(dB)
    INT8                                deltaFPucchF2a;

    // Parameter delta_F_PUCCH(F) for PUCCH format 2b, see section 5.1.2 of TS36.213
    // value range:{-2,0,2}(dB)
    INT8                                deltaFPucchF2b;
    // Parameter delta_F_PUCCH(F) for PUCCH format 3, see section 5.1.2 of TS36.213
    // value range:{-1,0,1,2,3,4,5,6}(dB)
    INT8                                deltaFPucchF3;
    // Parameter delta_F_PUCCH(F) for PUCCH format 1b with channel selection, see section 5.1.2 of TS36.213
    // value range:{1,2}(dB)
    INT8                                deltaFPucchF1bCS;
    UINT8                               rsvd;
}PhyUlPowerCtrlCommPara_s;


typedef struct _EPAT_PhyUlCommonInfoTag
{
    // UL EU-ARFCN
    UINT32                              ulEuArfcn;

    // UL center frequency, [100Hz], ulCentFreq = carrierCentFreq - PHY_RF_RX_LOWIF_FREQ
    UINT32                              ulCentFreq;

    // RF PLL feedback divider ratio for UL = LoDiv*ulCenterFreq*2^23/26e6, where
    // [31:24] integer, [23:0] fractional
    UINT32                              ulRfPllDivRatio;

    // indicate uplink RFPLL LO divider.
    UINT8                               ulRfPllLoDiv;
    // UL Bandwidth
    UINT8                               ulBandwidth;
    // timeAlignmentTimerCommon
    UINT8                               timeAlignmentTimer;
    // uplink cyclic prefix length, section 5.2.1 of TS 36.211
    // len1 corresponds to normal cyclic prefix and len2 corresponds to extended cyclic prefix
    UINT8                               ulCyclicPrefixLen;

    //for UEs neither in CE nor BL UEs (CAT1), ref to TS 36.101 [table 6.2.4-1]  and
    UINT8                               additionalSpectrumEmission;
    UINT8                               txDacClkSel;
    UINT16                              reserved_16;

    // PRACH parameters
    PhyPrachPara_s                      prachPara;

    // PUSCH and PUCCH parameters
    PhyPuxchCommPara_s                  puxchPara;

    // SRS parameters
    PhySrsCommPara_s                    srsPara;

    // UL power control parameters
    PhyUlPowerCtrlCommPara_s            powerCtrlPara;
}PhyUlCommonInfo_s;


// cell paging parameters
typedef struct _EPAT_PhyPagingParaTag
{
    // DRX paging occasion, (PF << 4)|PO
    UINT16                              PFPO;
    // default paging cycle, used to: 1> derive 'T' in TS 36.304; 2> calculate BCCH modification boudary
    // value range: 0/1/2/3 represents (32/64/128/256)*10ms
    UINT8                               defaultPagingCycle;
    // UE-specific paging cycle, used to control PCH receiving
    // value range: 0/1/2/3/4 represents (32/64/128/256/512) * 10ms
    // according to description in section 7.1 of TS36.304
    // if eDRX is configured with 5.12s cycle period, set to _RF512
    // else if specificDrxCyclePresent is TRUE, set to MIN(specificDrxCycle,defaultPagingCycle)
    // else, set to defaultPagingCycle
    UINT8                               ueSpecificPagingCycle;

    // Present flag of eDRX configuration, TURE means eDRX is configured
    BOOL                                eDrxPresent;
    // eDRX cycle is configured with unit of seconds
    // value range:(1/2/4/6/8/10/12/14/16/32/64/128/256)*10.24s
    // eDRX is NOT used if eDRX cycle is configured with 5.12s
    UINT8                               eDrxCycle;
    // Paging hyper-frame, see TS36.304 section 7.3, value range:0~1023
    // this PH value is the first position. L1 need get other PH values with PH+eDrxCycle
    // PH must be less than HFN cycle(according to eDrxCycle), so if eDrxCycle <= 10.24s, PH always be 0
    UINT16                              PH;

    // eDRX PTW start, {HFN,SFN,SBN}
    // ptwStartSfn <<= 8; ptwStartSfn += (PH << 10)
    // ptwStart = (ptwStartSfn + (PF-ptwStartSfn)&drxMask) & 0xFFFFF
    // ptwStart >>= 5;
    // Note: ptwStart<<(5+4) | PFPO is the actual start
    UINT16                              ptwStart;
    // eDRX PTW end, {HFN,SFN,SBN}
    // ptwEndSfn = ptwStartSfn + ((ptwLength+1)<<7) - 1
    // ptwEnd = (ptwEndSfn - (ptwEndSfn-PF)&drxMask) & 0xFFFFF
    // ptwEnd >>= 5
    // ptwEnd<<(5+4) | PFPO is the actual end
    UINT16                              ptwEnd;

    // number of UE specific paging cycle in one PTW
    UINT8                               ptwLen;
    // present flag of user setting DRX cycle, TRUE means valid userDrxCycle is informed by RRC
    BOOL                                userDrxPresent;
    // user setting DRX cycle(configured via ATCMD)
    // value range: 0/1/2/3 represents (32/64/128/256) * 10ms
    // PHY will monitor P_RNTI userDrxCycle instead of ueSpecificPagingCycle in PHY IDLE state
    UINT8                               userDrxCycle;
    UINT8                               rsvd;

    // starting radio frame of paging occasion in one period
    // now this value is always "0"
    //UINT16                              pagingOccasionStart;
    // end radio frame of paging occasion in one period
    //UINT16                              pagingOccasionEnd;
    // paging occasion is always set within the first radio frame of one period
    // period of paging occasion with unit of 10ms
    UINT16                              occasionPeriod;
    // PO bitmap in one radio frame within paging occasion
    // bit9~0 represents subframe#9~0
    UINT16                              occasionBitmap;
}PhyPagingPara_s;

typedef struct _EPAT_PhyDlCommonInfoTag
{
    // Actual modification period, expressed in number of radio frames= modificationPeriodCoeff * defaultPagingCycle
    // value range:{2,4,8,16,64}
    UINT8                               modificationPrdCoef;
    // provides the downlink CRS EPRE, value range: -60~50, unit: dBm
    INT8                                referenceSignalPower;
    // used to determine cell-specific ratio rouB/rouA based on Table 5.2-1 in TS36.213
    UINT8                               pb;
    // high speed enhanced measurement, it will affect period of intra-frequency measurement
    // see TS36.133 with key word "highSpeedEnhancedMeasFlag"
    BOOL                                highSpeedMeasEnable;

    // paging parameters, get from SIB2 or handover
    PhyPagingPara_s                     pagingPara;

    // indicate status of MBSFN-subframeConfig
    // for serving cell Rx: 
    // 1> set to "EXIST" or "NOT_EXIST" based on configuration in cephyCommonConfigReq
    // 2> set to "UNKONWN" after SSS detection of ICS
    // 3> set to "UNKONWN" after handling cephyCellReselectReq
    // 4> how to do for HO????
    // for neighbor cell Rx(NC SI):
    // always consider as "UNKNOWN"
    UINT8                               mbsfnCfgType;
    // backup previous mbsfnCfgType when setting it to unknown after ICS
    // also align it to mbsfnCfgType when parsing cephyCommonConfigReq
    // to cover that SIB2 receiving might not be scheduled when UE recover from PSM
    UINT8                               prevMbsfnType;
    UINT8                               rsvd1[2];

    // MBSFN subframe config
    // covert to format of SEQ HW REGs, including the configuration of R13 & R14
    /*
    R13: store MBSFN bitmap of 32 radio frames(SFN[4:0] = 0~31)
    *     7       6       5       4       3       2       1       0
    * +-------+-------+-------+-------+-------+-------+-------+-------+
    * | SBN#1 | SBN#2 | SBN#3 | SBN#4 | SBN#6 | SBN#7 | SBN#8 | SBN#9 |
    * +-------+-------+-------+-------+-------+-------+-------+-------+
    */
    // array index #0~#31 represents SFN#0~#31
    UINT8                               mbsfnBitmap[32];
}PhyDlCommonInfo_s;

// serving cell info.
typedef struct _EPAT_PhyScellInfoTag
{
    // PHY state, need write LSB[1:0] to AON register when go to hibernate
    UINT8                               phyState;
    // used to indicate if BT need be adjusted in post process of ATC
    // clear to FALSE when starting init. CS
    // set to TRUE when getting sib1ConfigReq from RRC
    BOOL                                btAdjEn;
    UINT16                              rntiList[9];

    // UE capability info.
    // configured by RRC when power on via CephyInitConfigReq
    PhyUeCapability_s                   ueCapability;

    // cell base info., get from ICS or handover
    PhyCellBaseInfo_s                   baseInfo;

    PhyPreSCellBaseInfo_s               preBaseInfo;

    // MIB info., get from MIB or handover
    PhyCellMibInfo_s                    mibInfo;

    // SIB1 info., get from SIB1 or handover
    PhyCellSib1Info_s                   sib1Info;

    // uplink common info.
    PhyUlCommonInfo_s                   ulCommInfo;

    // downlink common info.
    PhyDlCommonInfo_s                   dlCommInfo;
}PHY_PhyScellInfo;

// set txpower by PS
// Should be send in CONNECTED mode(after RandomAccessRequest, before MacReset(go IDLE))
// PHY will release this value when leave CONNECT mode
// Range: -45~24 stands for 0~24dB; other values are invalid
typedef struct _EPAT_PhyTxPowerReqByPsTag
{
    //powerSetType == 0, mean no set by PS;
    //powerSetType == 1, mean set targetPower by PS
    //powerSetType == 2, mean set minPower and maxPower by PS
    UINT8  powerSetType;
    INT8   minPower;
    INT8   maxPower;
    INT8   targetPower;

}PhyTxPowerReqByPs_s;

typedef struct _EPAT_PhySpecialCaseInfoTag
{
    // bitmap of special GCF case No.
    UINT16                          specialCaseBmp;
    UINT16                          rsvd;
}PhySpecialCaseInfo_s;

typedef struct _EPAT_PhyRxPdBackupRegTag
{
    UINT32* pRfPdReg;
    UINT32* pSqPdReg;
    UINT32* pCePdReg;
    UINT32* pCsiPdReg;

    UINT32  rxPdRegStaBuf[1529]; //200 + 1329
} PhyRxPdBackupReg_s;

typedef struct _EPAT_PhyConnRaStatisInfoTag
{
    // total number of received SIG_CEPHY_RANDOM_ACCESS_REQ
    UINT16                      accessRequestNum;
    // total number of transmitted preamble
    UINT16                      preambleTxNum;
    // total number of received RAR(RAPID matched)
    UINT16                      rarReceivedNum;
    // total number of transmitted msg3
    UINT16                      msg3TxNum;
    // total number of MSG4 received
    UINT16                      msg4ReceivedNum;
    // total number of RAR timer expired
    UINT16                      rarTimerExpiredNum;
    // total number of CR timer expired
    UINT16                      crTimerExpiredNum;
    // total number of RA success
    UINT16                      raSuccNum;
}PhyConnRaStatisInfo_s;

typedef struct _EPAT_PhyConnRxStatisInfoTag
{
    /* statis value during period, should cleared every period */
    // new grant and CRC NOK;
    UINT16      crcNokNewGrantNum;
    // new grant and CRC OK;
    UINT16      crcOkNewGrantNum;
    // Grant Number with CRC OK, including new trans/re-trans CRC OK
    UINT16      crcOkGrantNum;
    // DL Grant received, including new and retrans grant
    UINT16      dlGrantNum;

    // PDSCH decode right but ACK fails
    UINT16      Ack2NackNum;
    UINT16      reserved;

    // mcsAcc for new grant and CRC NOK;
    UINT32      crcNokNGMcsAcc;
    // mcsAcc for new grant and CRC OK;
    UINT32      crcOkNGMcsAcc;
    // mcsAcc for all grant;
    UINT32      dlGrantMcsAcc;

    // mcsAvg for new grant and CRC NOK;  the real value is this value/2
    UINT8       crcNokNGMcsAvg;
    // mcsAvg for new grant and CRC OK;  the real value is this value/2
    UINT8       crcOkNGMcsAvg;
    // mcsAvg for all grant;  the real value is this value/2
    UINT8       dlGrantMcsAvg;

    //Average PRB for all Grant, real value is this value/2
    UINT8       prbAvg;

    //Accumulated PRB for all grant
    UINT32      prbAcc;

    //codeRateAcc for new grant and CRC NOK;
    //the real value is this value/10000
    UINT32      crcNokNGCodeRateAcc;
    //codeRateAcc for new grant and CRC OK;
    UINT32      crcOkNGCodeRateAcc;

    //codeRateAvg for new grant and CRC NOK;
    //the real value is this value/100
    UINT8       crcNokNGCodeRateAvg;
    //codeRateAvg for new grant and CRC OK;
    //the real value is this value/100
    UINT8       crcOkNGCodeRateAvg;

    // wideband CQI report number
    UINT16      wbCqiRptNum;

    // Accumulated wideband CQI
    UINT32      wbCqiAcc;

    //tbsize for all grant(CRC OK), unit: byte
    UINT32      tbsAcc;

    // Average TBS(in byte) for al grant
    UINT16      tbsAvg;

     // averaged wideband CQI = wbCqiAcc/wbCqiRptNum; the real value is this value/2
    UINT16      wbCqiAvg;

    // new grant bler: crcNokNewGrantNum/(crcNokNewGrantNum+crcOkNewGrantNum)     
    // all bler: (totalGrantNum-crcOkGrantNum)/totalGrantNum
    // the real value is newGrantBler/10000
    UINT16      newGrantBler;
    UINT16      dlBler;

    /* statis value during connected mode, should not cleared every period */
    // not cleared during connected mode, for AT+ECSTATUS?
    UINT16      totalBler;
    UINT16      rsvd;
    UINT32      totalGrants;
    UINT32      totalBytes;
    UINT32      totalCrcOkGrants;
}PhyConnRxStatisInfo_s;

typedef struct _EPAT_PhyConnTxStatisInfoTag
{
    /* statis value during period, should cleared every period */
    // number for UL new Grant
    UINT16                      ulNewGrantNum;

    // CRC OK number for UL new Grant
    UINT16                      ulNewGrantCrcOkNum;

    // crc ok grant num including New grant and Retransmission grant
    UINT16                      ulGrantCrcOkNum;

    // total UL grant number scheduled during one statistical period
    // max is 10240, if counter is over this max value, right shift 1 bit
    UINT16                      ulGrantNum;

    // Ack -> Nack (DCI0 indicates new transmission but phich set to nack or dtx)
    UINT16                      phichAck2DtxNum;
    //Nack -> Ack  (DCI0 indicates re-transmission but phich set to ACK)
    UINT16                      phichNack2AckNum;

    //Accumulated PRB for all grant
    UINT32                      prbAcc;

    //Accumulated MCS for all grant
    UINT32                      mcsAcc;

    //Accumulated TBSize for all grant
    UINT32                      tbsAcc;
    
    //Average PRB for all Grant, real value is this value/2
    UINT8                       prbAvg;

    //Average MCS for all Grant, real value is this value/2
    UINT8                       mcsAvg;

    //Average TBS(in byte) for all Grant
    UINT16                      tbsAvg;

    //ulBler of new grant, ulNewGrantBler = (ulGrantCnt-ulRetransCnt-ulNewGrantCrcOkCnt)/(ulGrantCnt-ulRetransCnt), the real value is ulNewGrantBler/10000
    UINT16                      ulNewGrantBler;

    //total ulBler = ulRetransCnt/ulGrantCnt, the real value is ulBler/10000
    UINT16                      ulBler;

    //If the txpower ENB expected is 30db higher than Max txpower(23db), then txPowerInsufficientNum++
    //May use this para to determine whether trigger escape procedure in the future
    UINT16                      txPowerInsufficientNum;

    /* statis value during connected mode, should not cleared every period */
    UINT16                      totalBler;
    UINT32                      totalGrants;
    UINT32                      totalBytes;
    UINT32                      totalCrcOkGrants;
}PhyConnTxStatisInfo_s;

typedef struct _EPAT_PhyConnStatisInfoTag
{
    PhyConnRaStatisInfo_s       raStatisInfo;
    PhyConnRxStatisInfo_s       rxStatisInfo;
    PhyConnTxStatisInfo_s       txStatisInfo;
}PhyConnStatisInfo_s;

typedef struct _EPAT_PhyAssertResetInfoTag
{
    UINT8                           assertRstBranch;
    // record assert type from AT debug cmd
    UINT8                           atDbgAssertType;
    // indicated PS reset is ongoing, then skip all PHY ASSERT_TRUE
    // set to TRUE when sending CEPHY_PS_RESET_IND to RRC
    // clear to FALSE when reporting CEPHY_MAC_RESET_CNF or CEPHY_DEACT_CNF to PS
    BOOL                            psRstOngoing;
    UINT8                           rsvd;

    // format is the same as "HwTaskRxBitmap"
    // used to backup and recovery RNTI task
    UINT32                          rxTaskBmp;
}PhyAssertResetInfo_s;

typedef struct _EPAT_PhyStaticCfgInfoTag
{
    // record if received paging DCI for last 32 paging cycles
    // 1 stands for dci received, 0 stands for no dci received
    // cycle used, left shift for each paging cycle
    UINT32                          pagingDciBmp;

    // record if received paging DEC CRC OK for last 32 paging cycles
    // 1 stands for DEC OK, 0 stands for no dec or dec NOK
    // cycle used, left shift for each paging cycle
    UINT32                          pagingDecBmp;
}PhyStaticCfgInfo_s;

typedef struct _EPAT_PhySCellInfoStatusTag
{
    // DL EU-ARFCN
    UINT32                          dlEuArfcn;

    // cell ID
    UINT16                          phyCellId;
    // BandWidth_t
    UINT8                           dlBandwidth;
    UINT8                           rsvd0;

    // RSRP and RSRQ, [dB]
    INT16                           rsrp;
    INT16                           rssi;

    // in dB, fwl = 6
    INT16                           snr;
    // used to indicate original SCellInfo should be retained
    // also used to indicate if voting of flash written is necessary when receiving sysinfo.
    // set to TRUE during AT+ECBCINFO=0(CephyBasicCellInfoReq) or AT+ECBCINFO=1(CephyCellSearchReq with value TRUE of retainSCellInfo) in PSM state
    // clear to FALSE when PhySchdInit(power on/SLEEP2/HIB/CPOFF_WKUP) or CephyDeactReq(Normal DEACT) or CephyCellSearchReq(retainSCellInfo is FALSE)
    BOOL                            retainSCellInfo;
    UINT8                           rsvd1;
}PhySCellInfoStatus_s;

typedef struct _EPAT_PhySchdCtrlInfoTag
{
    // bitmap for dynamic allocated memory
    UINT32                          dynMemBmp;

    // record HFN|SFN|SBN of last dataInd, used to calc ACK time when dediCfg received, dediCfg should pending to after ACK time
    UINT32                          dataIndTime;

    // record HFN(10)|SFN(10) where MIB/SIB1 with correct CRC was allocated
    // used to update timeOfst of baseInfo
    UINT32                          siRcvTime;

    // bitmap for post process of modem XIC INT scheduling
    // [MXIC_x][IDX]: for CCH/DEC, IDX = SBN & 0x1; otherwise, IDX = 0
    // [0][x]: bit sequence same as MXIC0 of IRQn_Type
    // left-->right: bit[31]-->bit[0], bit[0]~bit[31] represents MXIC0_Ovf_IRQn ~ MXIC0_Sipc_IRQn
    // [1][x]: bit sequence same as MXIC1 of IRQn_Type
    // left-->right: bit[31]-->bit[0], bit[0]~bit[31] represents MXIC1_Ovf_IRQn ~ MXIC1_AuxAdc_IRQn
    // set bitmap to "1" in INT ISR, check this bitmap when scheduling post process
    // if it is NOT "1", skip post process and return directly
    // clear all bitmap when doing HwHalt
    UINT32                          modemIntBmpVal[2][2];

    // callback func for each req, called in  PhyMeasBuffRel when a request is pended
    PhyCBFunc_T                     measBuffReqCBFunc[3];

    // record if meas buff is in use by which module; set to measBuffReqType in PhyMeasBuffReq when current is INVALID; set to INVALID in PhyMeasBuffRel
    UINT8                           measBuffInUse;
    // record pending request for meas buff; set bit in PhyMeasBuffReq when need to wait; clear bit in PhyMeasBuffRel when callback func called
    UINT8                           measBuffReqBmp;
    // record pending of PhyHwTaskAdd when parsing dediCfg, all these pending tasks should be added later
    UINT8                           dediPendTask;
    // Record RX PAUSE reason, set in hwTaskConfig for RXPAUSE_DEDICFG/RXPAUSE_MEASGAP/RXPAUSE_ECGIGAP, cleared when hwTaskConfig for RXRESUME
    UINT8                           rxPauseRsn;

    // record time when RX paused
    // [1~6] store for HWTASK_RXPAUSE_DEDICFG/HWTASK_RXPAUSE_MEASGAP/HWTASK_RXPAUSE_ECGIGAP/HWTASK_RXPAUSE_DATADUMP
    // [0] is reserved
    UINT32                          rxPauseTime[7];

    // record time of RX_RESUME
    // [1~6] store for HWTASK_RXPAUSE_DEDICFG/HWTASK_RXPAUSE_MEASGAP/HWTASK_RXPAUSE_ECGIGAP/HWTASK_RXPAUSE_DATADUMP
    // [0] is reserved
    UINT32                          rxResumeTime[7];

    // set to TRUE in mibInfoUpdt if bandwidth changed
    // clear to FALSE at the end of the 1st TASK_CFG after MIB
    // 0-SC; 1-NC
    BOOL                            bwChngFlag[2];
    UINT8                           dataDumpTtiNum;
    // record statis start command from AP, 
    // if TRUE, AP will periodically request CP to report statisInfo(all PHY state), so PHY no need to start PHY_STATIS_REPORT_TIMER
    // if FALSE, PHY should start PHY_STATIS_REPORT_TIMER and print statis on EPAT itself
    BOOL                            connStatisStart;

    // record temprature return from AP, fwl = 4
    INT16                           currTemprVal;
    // set to TRUE when received PhyHandleHandoverReq; 
    // set to FALSE when received commonConfig if HO success or when HO fail(in PhyHandoverCnfReport with CRC NOK), or when MacReset Received
    BOOL                            handoverFlag;
    // store duplexMode of original cell
    UINT8                           hoOrigCellDuplexMode;

    // set max txpower by PS, PHY should not transmit with power higher than maxPower;
    // Should be send in CONNECTED mode(after RandomAccessRequest, before MacReset(go IDLE))
    // PHY will release this value when leave CONNECT mode
    // Range: -45~24 stands for 0~24dB; other values are invalid
    PhyTxPowerReqByPs_s             psSetPower;
    UINT8                           utcReqId;
    //used to record called in paging update or hibTimer expire
    UINT8                           fakeEdrxUtcReqBranch;
    UINT8                           flshClkChgOnging;//flash frequncy change request ongoing

    //in paging image, spll can be off after CP initialization finished and taskconfig interrupt of pch/presync has not been coming
    //which cannot be judged in PhyPmuModeCtrl because IDLE length maybe very short, so set in PhySchdInit function
    BOOL                            phyInitFnsh;

    // special case info.
    PhySpecialCaseInfo_s            specialCaseInfo;

    // backup buffer for RX BB&SEQ when CP vote to sleep
    PhyRxPdBackupReg_s              *rxPdBackupReg;

    // used for store statistical info in connected mode; malloc when other states to CONN, free when CONN to other states
    PhyConnStatisInfo_s             *connStatisInfo;
    UINT32                          lastStatisReportTime;

    PhyAssertResetInfo_s            assertRstInfo;

    PhyStaticCfgInfo_s              staticCfgInfo;

    // when phyState is INIT
    // backup original SCellInfo when receiving CephyBasicCellInfoReq or CephyCellSearchReq(retainSCellInfo is TRUE)
    // recovery SCellInfo when receiving CephyDeactReq(Normal DEACT)
    // NOTE: SCellInfo is DL info. is used in CephyGetStatusCnf
    PhySCellInfoStatus_s            scellInfoStatus;
    //Total TX time since last reboot in millisecond
    UINT32                          txTime;
    //Total RX time since last reboot in millisecond,do not count ICS, Meas related receiving, only count DCI INT.
    UINT32                          rxTime;

}PHY_PhySchdCtrlInfo;

typedef struct _EPAT_PhyRfTxParaTag
{
    // RF center frequency, unit:100Hz
    UINT32                  centFreq;

    // only for debug print
    UINT32                  earfcn;

    // RF PLL feedback divider ratio
    UINT32                  pllDivRatio;

    // RF frequency error index
    INT32                   freqErrIdx;

    // indicate RFPLL Lo Divider.
    UINT8                   rfPllLoDiv;

    // curr bandNum.
    UINT8                   bandNum;

    // RF downlink bandwidth, 1.4/3/5/10/15/20MHz
    UINT8                   txBw;

    // 0-307.2MHz, 1-245.72MHz.
    UINT8                   dacClkSel;
    
} PhyRfTxPara_s;

typedef struct _EPAT_PhyCurrTxHwTaskTag
{
    // paramters for RF
    PhyRfTxPara_s                 rfTxPara;
}PhyCurrMiscTxHwTask_s;

typedef struct _EPAT_PhyCurrRxHwTaskTag
{
    // Updated by rxTaskAddBitmap/rxTaskDelBitmap in taskConfig
    UINT32                          rxTaskBitmap;

    // Transit to RUNNING after HwTaskConf, transit to INACTIVE in RX STOP and no other hwTask is running
    UINT8                           rxState;
    // get temprChng indication when entering HW_TASK_CFG
    // it is used by sub-module configuration of HW_TASK_CFG
    // not used now
    BOOL                            tmprChngInd;
    UINT8                           rsvd[2];

    // Rx configured parameters
    //PhyCurrRxPara               cnfgPara;
}PhyCurrRxHwTask_s;

typedef struct _EPAT_PhyHwTaskPendNodeTag
{
    // timeOfst should be stored in pendList, we need use the timeOfst to compare origTime, and to config Tpu L1 Timer;
    // Note: should no two nodes have the same startTime and different timeOfst; 
    //       no two noeds have diff startTime and diff timeOfst, but have the same origTime

    // startTime of modem, SW config time should be startTime - 1ms(maybe more for large TA)
    // format: HFN(10)|SFN(10)|SBN(4)
    UINT32                                startTime;

    // store timeOfstHw of hwTask, used to sort hwTask with same time base and config Tpu L1 Timer
    T_TIME_OFST_s                         timeOfst;

    // Recored all Misc&TX trigger event with same startTime
    UINT32                                miscTxTaskBitmap;
    // Recored all RX trigger Add event with same startTime
    UINT32                                rxTaskAddBitmap;
    // Recored all RX trigger Del event with same startTime
    UINT32                                rxTaskDelBitmap;

    struct _EPAT_PhyHwTaskPendNodeTag     *pNext;
}PhyHwTaskPendNode_s;


typedef struct _EPAT_PhyHwTaskCtrlInfoTag
{
    // Record current misc hwTask, for pending message, need to check misTaskBitmap and startTime to decied if need to ingore DCI for current subframe
    PhyCurrMiscTxHwTask_s           currMiscTxHwTask;

    // Current working RX hwTask
    // Updated in TaskConfig when RX related event included in the header of hwTaskPendList
    PhyCurrRxHwTask_s               currRxHwTask;

    // format: HFN(10)|SFN(10)|SBN(4)
    UINT32                          startTime;

    PhyHwTaskPendNode_s             taskList[5];
}PHY_PhyHwTaskCtrlInfo;

#define __PHY_EXCEP_INFO_PART_4__

typedef struct _EPAT_MacTimerElementTag
{
    UINT8   userID;
    UINT8   timerID;
    UINT16  rsvd;
    UINT32  length;

    // format: HFN[23:14]|SFN[13:4]|SBN[3:0]
    UINT32  expiredTime;

    struct _EPAT_MacTimerElementTag *pNext;
}MacTimerElement_s;

typedef struct _EPAT_MacTimerCtrlTag
{
    MacTimerElement_s    timerList[5];
}PHY_MacTimerCtr;

typedef struct _EPAT_PhyTxSchdTrigNodeTag
{
    // txHfnSfnSbn to be transmit, initilized according to the first Trig, after txSchedule for this txSfnSbn, clear readyFlag, and add 1 sfn to txSfnSbn
    UINT32                                txHfnSfnSbn;

    // 0: not ready for trig, 1: ready for trig
    // set when insert called for this txSfnSbn, clear when txSchedule called for this txSfnSbn
    UINT8                                 readyFlag;
    UINT8                                 rsvd0;
    UINT16                                rsvd1;
    struct _EPAT_PhyTxSchdTrigNodeTag     *pNext;
}PhyTxSchdTrigNode_s;

typedef struct _EPAT_PhyTxSchdTrigCtrlTag
{
    // should be cycle linked and increased in the trigList
    PhyTxSchdTrigNode_s    trigList[10];
    // next to be transmit tx time
    PhyTxSchdTrigNode_s   *pRunTrigHeader;
    // record node with readyFlag = 1, increased when insert called, decreased when txSchedule called
    UINT8                  runTrigNumber;
    // flag of Sbn, set to 1 when sbn = 0, clear when sbn = 9
    UINT8                  flagSbn0;
    // number of scheduled sbns
    UINT8                  schdSbnNum;
    // total scheduled sbns in one frame (10ms)
    UINT8                  schdSbnNumIn10ms;

    // used to initialize TxSchdTrigNode
    // set to 1:
    //          PhyTxFirstTaskCfg(for initPrach)
    //          PhyTxInitMsg3TaskCfg
    //          PhyTxPdRecovery(when wakeup in connected mode)
    //          PhyTxCancelAll(in TxStopPostIsr)
    //          PhyTxHoInit
    // Clear to 0 in PhyTxScheduleWrapper when initFlag = 1, will call PhyTxSchdTrigListInit before clear to 0
    UINT8                 initFlag;
    UINT8                 rsvd_08;
    UINT16                svd_16;
}PhyTxSchdTrigCtrl_s;

typedef struct _EPAT_PhyTxSchdInfo
{
    // Tx Schdeuler information
    UINT32               txSchdBmp;

    // Tx Wrapper Trig List
    PhyTxSchdTrigCtrl_s  txSchdTrigInfo;

    // Tx Dram Cfg
    UINT16               txDramCfg[10];
}PHY_PhyTxSchdInfo;
#endif

