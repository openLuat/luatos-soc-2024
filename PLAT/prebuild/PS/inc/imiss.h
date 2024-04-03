/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imiss.h
 Description:    - Declare  the  supplementary service API for IMI
 History:        - 2023/04/10, Original created
******************************************************************************/
#ifndef IMI_SS_H
#define IMI_SS_H

/*********************************************************************************
*****************************************************************************
* Includes
*****************************************************************************
*********************************************************************************/
#include "imicomm.h"

/*********************************************************************************
*****************************************************************************
* Macros
*****************************************************************************
*********************************************************************************/

#define  IMI_STR_NUMBER_MAX_LEN    (128)

#define  IMI_UT_CALL_FWD_NODE_MAX  (6)

#define  IMI_UT_CALL_BARR_NODE_MAX  (8)


/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_SS_PRIM_ID_TAG
{
    IMI_SS_PRIM_BASE = 0,

    /* follow by IMI_CC_PRIM_ID */
    IMI_SS_SET_CALL_HOLD_SUPP_REQ = 0x0200, /* ImiSsSetCallHoldSuppReq, AT+CHLD */
    IMI_SS_SET_CALL_HOLD_SUPP_CNF,          /* ImiSsSetCallHoldSuppCnf */
    IMI_SS_SET_CALL_WAITING_REQ,            /* ImiSsSetCallWaitingReq, AT+CCWA */
    IMI_SS_SET_CALL_WAITING_CNF,            /* ImiSsSetCallWaitingCnf */

    IMI_SS_SET_CLIP_SUPP_REQ,              /*AT+CLIP SET REQ*/
    IMI_SS_SET_CLIP_SUPP_CNF,              /*AT+CLIP SET CNF*/
    IMI_SS_GET_CLIP_SUPP_REQ,              /*AT+CLIP GET REQ*/
    IMI_SS_GET_CLIP_SUPP_CNF,              /*AT+CLIP GET CNF*/

    IMI_SS_SET_CLIR_SUPP_REQ,              /*AT+CLIR SET REQ*/
    IMI_SS_SET_CLIR_SUPP_CNF,              /*AT+CLIR SET CNF*/
    IMI_SS_GET_CLIR_SUPP_REQ,              /*AT+CLIR GET REQ*/
    IMI_SS_GET_CLIR_SUPP_CNF,              /*AT+CLIR GET CNF*/

    IMI_SS_SET_CLCK_SUPP_REQ,              /*AT+CLCK SET REQ*/
    IMI_SS_SET_CLCK_SUPP_CNF,              /*AT+CLCK SET CNF*/
    IMI_SS_GET_CLCK_SUPP_REQ,              /*AT+CLCK GET REQ*/
    IMI_SS_GET_CLCK_SUPP_CNF,              /*AT+CLCK GET CNF*/

    /*  -------SS UT--------*/
    IMI_SS_SET_CALL_FORWARD_REQ,         /*AT+CCFC SET REQ*/
    IMI_SS_SET_CALL_FORWARD_CNF,         /*AT+CCFC SET CNF*/
    IMI_SS_GET_CALL_FORWARD_REQ,         /*AT+CCFC GET REQ*/
    IMI_SS_GET_CALL_FORWARD_CNF,         /*AT+CCFC GET CNF*/


    /*unsolicited indication*/


    IMI_SS_PRIM_END    = 0x0fff
}IMI_SS_PRIM_ID;


typedef enum ImiSsCallHoldSuppModeTag
{
    IMI_SS_REL_ALL_HELD_WAITING_CALL            = 0, //<n>:0, release all held calls or waiting call
    IMI_SS_REL_ACTIVE_ACCEPT_HELD_WAITING_CALL  = 1, //<n>:1, release all active calls and accepts the other (held or waiting) call
    IMI_SS_REL_SPECIFIC_ACTIVE_CALL_X           = 2, //<n>:1X, release a specific active call X, X indicated in call Id
    IMI_SS_HLD_ACTIVE_ACCEPT_HELD_WAITING_CALL  = 3, //<n>:2, hold all active calls and accepts the other (held or waiting) call
    IMI_SS_HLD_ALL_ACTIVE_EXCEPT_CALL_X         = 4  //<n>:2X, hold all active calls except call X
}
ImiSsCallHoldSuppMode;


typedef enum ImiSsCallWaitingModeTag
{
    IMI_SS_CALL_WAITING_DISABLE             = 0,
    IMI_SS_CALL_WAITING_ENABLE,
    IMI_SS_CALL_WAITING_QUERY_STATUS,
    IMI_SS_CALL_WAITING_UNKNOWN
}
ImiSsCallWaitingMode;

typedef enum ImiSsUtReqMethodType_ENUM
{
    IMI_UT_REQ_INVALID = 0,
    IMI_UT_REQ_METHOD_GET,
    IMI_UT_REQ_METHOD_PUT,
    IMI_UT_REQ_METHOD_POST,
    IMI_UT_REQ_METHOD_DEL
}ImiSsUtReqMethodType;



typedef enum ImiSsUtReqModType_ENUM
{
    IMI_UT_MOD_INVALID = 0,
    IMI_UT_MOD_OIP,
    IMI_UT_MOD_TIP,
    IMI_UT_MOD_OIR,
    IMI_UT_MOD_TIR,
    IMI_UT_MOD_CW,

    IMI_UT_MOD_CDIV,
    IMI_UT_MOD_ICB,
    IMI_UT_MOD_OCB,
    IMI_UT_MOD_MAX = 16,
}ImiSsUtReqModType;


typedef enum ImiUtCcfcStatus_ENUM
{
    IMI_UT_CCFC_DEACTIVED = 0,
    IMI_UT_CCFC_ACTIVED

}ImiUtCcfcStatus;


typedef enum ImiUtCcfcReqOperType_ENUM
{
    IMI_UT_CCFC_REQ_UNCONDITIONAL = 0,
    IMI_UT_CCFC_REQ_BUSY,
    IMI_UT_CCFC_REQ_NOREPLY,
    IMI_UT_CCFC_REQ_NOREACH,
    IMI_UT_CCFC_REQ_ALL_CDIV,       /*ALL call forwarding*/
    IMI_UT_CCFC_REQ_ALL_COND_CDIV,   /*ALL conditional call forwarding */
    IMI_UT_CCFC_REQ_NO_REPLY_TIME,
}ImiUtCcfcReqOperType;


typedef enum ImiUtCcfcReqModeType_ENUM
{
    IMI_UT_CCFC_MODE_DISABLE = 0,
    IMI_UT_CCFC_MODE_ENABLE,
    IMI_UT_CCFC_MODE_QUERY,
    IMI_UT_CCFC_MODE_REG,
    IMI_UT_CCFC_MODE_ERASURE
}ImiUtCcfcReqModeType;


typedef enum ImiUtCcfcTagNumType_ENUM
{
    IMI_TAGNUM_TYPE_DEF   =0,
    IMI_TAGNUM_LOCAL_TYPE =  IMI_TAGNUM_TYPE_DEF,
    IMI_TAGNUM_GEO_TYPE,
}ImiUtCcfcTagNumType;



/******************************************************************************
 *****************************************************************************
 * IMI STRUCT
 *****************************************************************************
******************************************************************************/


/******************************************************************************
 * IMI_SS_SET_CALL_HOLD_SUPP_REQ
 * AT+CHLD
******************************************************************************/
typedef struct ImiSsSetCallHoldSuppReqTag
{
    UINT8               mode; //ImiSsCallHoldSuppMode
    UINT8               callId; //1-6, 0-invalid;
                                //present if mode is IMI_SS_REL_SPECIFIC_ACTIVE_CALL_X or IMI_SS_HLD_ALL_ACTIVE_EXCEPT_CALL_X
    UINT16              resvd;
}
ImiSsSetCallHoldSuppReq;

//IMI_SS_SET_CALL_HOLD_SUPP_CNF
typedef ImiEmptySig ImiSsSetCallHoldSuppCnf;

/******************************************************************************
 * IMI_SS_SET_CALL_WAITING_REQ
 * AT+CCWA
******************************************************************************/
typedef struct ImiSsSetCallWaitingReqTag
{
    UINT8               mode; //ImiSsCallWaitingMode
    BOOL                classPresent;//shall be TRUE if mode is not query status
    UINT8               class;
    UINT8               resvd;
}
ImiSsSetCallWaitingReq;

//IMI_SS_SET_CALL_WAITING_CNF
typedef struct ImiSsSetCallWaitingCnfTag
{
    UINT8               mode; //ImiSsCallWaitingMode
    /*
    * status returned for query status mode
    */
    UINT8               voice  : 1;
    UINT8               data   : 1;
    UINT8               fax    : 1;
    UINT8               sms    : 1;
    UINT8               resvd  : 4;

    UINT16              resvd2;
}
ImiSsSetCallWaitingCnf;


/******************************************************************************
 * IMI_SS_GET_CALL_FORWARD_REQ
 * AT+CCFC =  operType, modtype(which is equal to 2)
******************************************************************************/
typedef struct ImiSsGetCallFwdReqTag
{
    UINT8         method;        /*ImiSsUtReqMethodType*/
    UINT8         operType;      /*ImiUtCcfcReqOperType*/
    UINT8         resv[2];
}ImiSsGetCallFwdReq;



typedef struct ImiSsGetPeerCallFwdCnfTag
{
/*
    when <mode>=2 and command successful:
    +CCFC: <status>,<class1>[,<number>,<type>[,<subad
    dr>,<satype>[,<time>]]]
    [...]]

*/
    UINT8         result;
    UINT8         state;      /**/
    UINT8         serType;    /*ImiUtCcfcReqOperType*/

    UINT8         bNtfCaller            : 1;            /*notify-caller,  */
    UINT8         bRevealCaller         : 1;            /*reveal-identity-to-caller*/
    UINT8         bRvlSrvedUsrIdToCaller: 1;            /*reveal-served-user-identity-to-caller*/
    UINT8         bRvlIdToTarget        : 1;            /*reveal-identity-to-target*/
    UINT8         bNtfySrvedUsr         : 1 ;           /* notify-served-user, default is false */
    UINT8         resv0                  : 3;

    union {
        struct {
            UINT8  voice    : 1;
            UINT8  data     : 1;
            UINT8  fax      : 1;
            UINT8  sms      : 1;
            UINT8  resv1     : 4;
        }u;

        UINT8 value;
    }classInfo;

    UINT8  numberType;             /*ImiUtCcfcTagNumType*/
    UINT8  targetLen;              /*ImiUtCcfcTagNumType*/

    UINT8  resv2[2];

    UINT8  targetNumber[IMI_STR_NUMBER_MAX_LEN];

}ImiSsGetPeerCallFwdCnf;


/******************************************************************************
 * IMI_SS_GET_CALL_FORWARD_CNF
******************************************************************************/
typedef struct ImiSsGetCallFwdCnfTag
{
    UINT8  nodeNum;
    UINT8  operType;        /*ImiUtCcfcReqOperType*/
    UINT8  methodType;
    UINT8  resv;

    ImiSsGetPeerCallFwdCnf   nodeInfo[IMI_UT_CALL_FWD_NODE_MAX];
}ImiSsGetCallFwdCnf;

/******************************************************************************
 * IMI_SS_SET_CALL_FORWARD_REQ
 * AT+CCFC/CCFCU

******************************************************************************/
typedef struct ImiSsSetCallFwdReqTag
{
    /*
        +CCFC=<reason>,<mode>[,<number>[,<type>[,<class>[,<subaddr>[,<satype>[,<time>]]]]]]
    */
    UINT8         operType;      /*ImiUtCcfcReqOperType*/
    UINT8         mode;          /*ImiUtCcfcReqModeType*/
    UINT8         numberType;    /*ImiUtCcfcTagNumType*/
    UINT8         noRepTime;


    UINT8         targetNumber[IMI_STR_NUMBER_MAX_LEN];
}ImiSsSetCallFwdReq;

/******************************************************************************
 * IMI_SS_SET_CALL_FORWARD_CNF
******************************************************************************/
typedef struct ImiSsSetCallFwdCnfTag
{
    UINT32  result;
}ImiSsSetCallFwdCnf;


typedef enum ImiSsClckFacType_Enum
{
    IMI_SS_CLCK_FAC_INVALID = 0,
    IMI_SS_CLCK_BAOC,             /*Barr All Outgoing Calls*/
    IMI_SS_CLCK_BOIC,             /*Barr Outgoing International Calls*/
    IMI_SS_CLCK_BOIC_exHC,        /*Barr Outgoing International Calls except to Home Country*/
    IMI_SS_CLCK_BAIC,             /*Barr All Incoming Calls*/
    IMI_SS_CLCK_BIC_Roam,         /*Barr Incoming Calls when Roaming outside the home country*/
    IMI_SS_CLCK_BIC_ACR,
}ImiSsClckFacType;


typedef enum ImiSsClckMode_Enum
{
    IMI_SS_CLCK_UNLOCK = 0,
    IMI_SS_CLCK_LOCK,
    IMI_SS_CLCK_QUERY
}ImiSsClckMode;


typedef enum ImiSsClckStatus_Enum
{
    IMI_SS_CLCK_NO_TACT = 0,
    IMI_SS_CLCK_ACT,
}ImiSsClckStatus;

typedef enum ImiSsClckClass_Enum
{
    IMI_SS_CLCK_CLASS_INVALID = 0,
    IMI_SS_CLCK_VOICE   = 1,
    IMI_SS_CLCK_DATA    = 2,
    IMI_SS_CLCK_FAX     = 4,
    IMI_SS_CLCK_SMS     = 8,
    IMI_SS_CLCK_MAX     = 255,
} ImiSsClckClass;

/******************************************************************************
 *IMI_SS_SET_CLCK_SUPP_REQ
******************************************************************************/
typedef struct  ImiSsClckSetReq_Tag
{
    UINT8   fac;    /*see ImiSsClckFacType*/
    UINT8   mode;   /*see ImiSsClckMode  */
    UINT8   class;  /*see ImiSsClckClass */
    UINT8   resv;

    CHAR    password[IMI_STR_NUMBER_MAX_LEN];
}ImiSsClckSetReq;

/******************************************************************************
 * IMI_SS_SET_CLCK_SUPP_CNF
******************************************************************************/
typedef ImiEmptySig ImiSsClckSetCnf;

/******************************************************************************
 *IMI_SS_GET_CLCK_SUPP_REQ
******************************************************************************/
typedef struct  ImiSsClckGetReq_Tag
{
    UINT8   fac;    /*see ImiSsClckFacType*/
    UINT8   resv[3];
}ImiSsClckGetReq;


/******************************************************************************
 *IMI_SS_GET_CLCK_SUPP_CNF
******************************************************************************/
typedef struct  ImiSsClckPerNodeInfo_Tag
{
    UINT8   fac;      /*see ImiSsClckFacType*/
    UINT8   status;   /*see ImiSsClckStatus  */
    UINT8   class;    /*see ImiSsClckClass*/
    UINT8   resv;
}ImiSsClckPerNodeInfo;

typedef struct  ImiSsClckGetCnf_Tag
{
    UINT8    num;
    UINT8    fac;    /* see ImiSsClckFacType*/
    UINT8    resv[2];

    ImiSsClckPerNodeInfo   perNodeInfo[IMI_UT_CALL_BARR_NODE_MAX];
}ImiSsClckGetCnf;


typedef enum ImiSsClipMode_Enum
{
    IMI_SS_CLIP_MODE_DISABLE            = 0,
    IMI_SS_CLIP_MODE_ENABLE,
}ImiSsClipMode;

typedef enum ImiSsClipProvType_Enum
{
    IMI_SS_CLIP_NOT_PROVISIONED         = 0,
    IMI_SS_CLIP_PROVISIONED,
    IMI_SS_CLIP_UNKNOWN
}ImiSsClipProvType;

/******************************************************************************
 * IMI_SS_SET_CLIP_SUPP_REQ
******************************************************************************/
typedef struct  ImiSsClipSetReq_Tag
{
    UINT8   mode;    /*see ImiSsClipMode*/
    UINT8   resv[3];
}ImiSsClipSetReq;


/******************************************************************************
 * IMI_SS_SET_CLIP_SUPP_CNF
******************************************************************************/
typedef ImiEmptySig ImiSsClipSetCnf;


/******************************************************************************
 * IMI_SS_GET_CLIP_SUPP_REQ
******************************************************************************/
typedef ImiEmptySig ImiSsClipGetReq;


/******************************************************************************
 * IMI_SS_GET_CLIP_SUPP_CNF
******************************************************************************/
typedef struct ImiSsClipGetCnf_Tag
{
    UINT8   mode;    /*see IcmSsUtClipMode*/
    UINT8   type;      /*see IcmSsUtClipProvType*/
    UINT8   resv[2];
}ImiSsClipGetCnf;



typedef enum ImiSsClirMode_Enum
{
    IMI_SS_CLIR_SERVICE_BY_NW =0,
    IMI_SS_CLIR_INVOCATION,
    IMI_SS_CLIR_SUPPRESSION,
}ImiSsClirMode;

typedef enum ImiSsClirProvType_Enum
{
    IMI_SS_CLIR_NOT_PROVISION = 0,
    IMI_SS_CLIR_PROVISION_WITH_PERMODE,
    IMI_SS_CLIR_UNKNOWN,
    IMI_SS_CLIR_TEMP_RESTRICT,
    IMI_SS_CLIR_TEMP_ALLOWED
}ImiSsClirProvType;

/******************************************************************************
 *IMI_SS_SET_CLIR_SUPP_REQ
******************************************************************************/
typedef struct  ImiSsClirSetReq_Tag
{
    UINT8   mode;    /*see IcmSsUtClipMode*/
    UINT8   resv[3];
}ImiSsClirSetReq;

/******************************************************************************
 * IMI_SS_SET_CLIR_SUPP_CNF
******************************************************************************/
typedef ImiEmptySig ImiSsClirSetCnf;


/******************************************************************************
 * IMI_SS_GET_CLIR_SUPP_REQ
******************************************************************************/
typedef ImiEmptySig ImiSsClirGetReq;


/******************************************************************************
 * IMI_SS_GET_CLIR_SUPP_CNF
******************************************************************************/
typedef struct ImiSsClirGetCnf_Tag
{
    UINT8   mode;       /*see IcmSsUtClirMode*/
    UINT8   provType;   /*see IcmSsUtClirProvType*/
    UINT8   resv[2];
}ImiSsClirGetCnf;


typedef enum ImiSsColpMode_Enum
{
    IMI_SS_COLP_MODE_DISABLE            = 0,
    IMI_SS_COLP_MODE_ENABLE,
}ImiSsColpMode;

typedef enum ImiSsColpProvType_Enum
{
    IMI_SS_COLP_NOT_PROVISIONED         = 0,
    IMI_SS_COLP_PROVISIONED,
    IMI_SS_COLP_UNKNOWN
}ImiSsColpProvType;


/******************************************************************************
 *IMI_SS_SET_COLP_SUPP_REQ
******************************************************************************/
typedef struct  ImiSsColpSetReq_Tag
{
    UINT8   mode;    /*see ImiSsColpMode*/
    UINT8   resv[3];
}ImiSsColpSetReq;

/******************************************************************************
 * IMI_SS_SET_COLP_SUPP_CNF
******************************************************************************/
typedef ImiEmptySig ImiSsColpSetCnf;

/******************************************************************************
 * IMI_SS_GET_COLP_SUPP_REQ
******************************************************************************/
typedef ImiEmptySig ImiSsColpGetReq;


/******************************************************************************
 * IMI_SS_GET_COLP_SUPP_CNF
******************************************************************************/
typedef struct ImiSsColpGetCnf_Tag
{
    UINT8   mode;
    UINT8   provType;
    UINT8   resv[2];
}ImiSsColpGetCnf;

#endif
