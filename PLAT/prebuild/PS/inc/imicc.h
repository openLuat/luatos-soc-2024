#ifndef __IMI_CC_H__
#define __IMI_CC_H__

/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imicc.h
 Description:    - IMS CC interface
 History:        - 2023/04/10, Original created
******************************************************************************/

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
#define IMI_CC_MAX_DIAL_NUMBER_STR_LEN    80//TS24.008 10.5.4.7 called party BCD number digit max 40 octets
#define IMI_CC_MAX_SUB_ADDR_LEN    20//TS24.008 10.5.4.8
#define IMI_CC_MAX_ALPHA_ID_STR_LEN     32
#define IMI_CC_MAX_CURRENT_CALLS    6
/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_CC_PRIM_ID_TAG
{
    IMI_CC_PRIM_BASE = 0,

    IMI_CC_MAKE_CALL_REQ,               /* ImiCcMakeCallReq, ATD<number>[;] */
    IMI_CC_MAKE_CALL_CNF,               /* ImiCcMakeCallCnf */
    IMI_CC_ANSWER_CALL_REQ,             /* ImiCcAnswerCallReq, ATA */
    IMI_CC_ANSWER_CALL_CNF,             /* ImiCcAnswerCallCnf */
    IMI_CC_HANGUP_CALL_REQ,             /* ImiCcHangupCallReq, ATH/AT+CHUP */
    IMI_CC_HANGUP_CALL_CNF,             /* ImiCcHangupCallCnf */
    IMI_CC_LIST_CURRENT_CALL_REQ,       /* ImiCcListCurrentCallReq, AT+CLCC */
    IMI_CC_LIST_CURRENT_CALL_CNF,       /* ImiCcListCurrentCallCnf */

    IMI_CC_SEND_DTMF_STR_REQ,           /* ImiCcSendDtmfStrReq, AT+VTS=<DTMF>[,<duration>] */
    IMI_CC_SEND_DTMF_STR_CNF,           /* ImiCcSendDtmfStrCnf */
    IMI_CC_SET_DTMF_DURATION_REQ,       /* ImiCcSetDtmfDurationReq, AT+VTD=<n> */
    IMI_CC_SET_DTMF_DURATION_CNF,       /* ImiCcSetDtmfDurationCnf */
    IMI_CC_GET_DTMF_DURATION_REQ,       /* ImiCcGetDtmfDurationReq, AT+VTD? */
    IMI_CC_GET_DTMF_DURATION_CNF,       /* ImiCcGetDtmfDurationCnf */


    /*unsolicited indication*/
    IMI_CC_INCOMING_CALL_IND,           /*ImiCcIncomingCallInd, RING/+CRING*/
    IMI_CC_CONN_LINE_ID_PRE_IND,        /*ImiCcConnLineIdPreInd, +COLP, MO call display remote number*/
    IMI_CC_CALL_LINE_ID_PRE_IND,        /*ImiCcCallLineIdPreInd, +CLIP, MT call display remote number*/
    IMI_CC_CALL_CONNECT_IND,
    IMI_CC_CALL_DISCONNECT_IND,         /*ImiCcCallDisconnectInd, NO CARRIER */
    IMI_CC_CALL_WAITING_IND,            /*ImiCcCallWaitingInd, +CCWA*/

    /*internal Ind*/
    IMI_CC_CALL_STATE_IND,

    IMI_CC_PRIM_END    = 0x01ff
}IMI_CC_PRIM_ID;

typedef enum ImiCcResultCodeTag
{
    IMI_CC_SUCC                  = 0, //
    IMI_CC_NO_CARRIER,
    IMI_CC_BUSY
    //IMI_CC_NO_ANSWER ?
}
ImiCcResultCode;

typedef enum ImiCcCallDirectionTag
{
    MO_CALL     = 0,
    MT_CALL     = 1
}
ImiCcCallDirection;

typedef enum ImiCcCallStateTag
{
    IMI_CC_CALL_ACTIVE          = 0,
    IMI_CC_CALL_HELD            = 1,
    IMI_CC_CALL_DIALING_MO      = 2,
    IMI_CC_CALL_ALERTING_MO     = 3,
    IMI_CC_CALL_INCOMING        = 4,
    IMI_CC_CALL_WAITING         = 5,
    IMI_CC_CALL_INVALID         = 0xFF
}
ImiCcCallState;

typedef enum ImiCcCallModeTag
{
    IMI_CC_MODE_VOICE           = 0,
    IMI_CC_MODE_DATA            = 1,
    IMI_CC_MODE_FAX             = 2
}
ImiCcCallMode;

typedef enum ImiCcRingTypeTag
{
    IMI_CC_RING_TYPE_ASYNC                = 0,
    IMI_CC_RING_TYPE_SYNC                 = 1,
    IMI_CC_RING_TYPE_REL_ASYNC            = 2,
    IMI_CC_RING_TYPE_REL_SYNC             = 3,
    IMI_CC_RING_TYPE_FAX                  = 4,
    IMI_CC_RING_TYPE_VOICE                = 5,
    IMI_CC_RING_TYPE_VOICE_VIDEO          = 6
}
ImiCcRingType;

/*
* call number type refer to TS24.008 10.5.4.7
*/
typedef enum ImiCcCallNumTypeTag
{
    IMI_CC_NUM_TYPE_UNKNOWN                 = 129,//unknown type
    IMI_CC_NUM_TYPE_INTER_NUMBER            = 145,//international number
    IMI_CC_NUM_TYPE_NATIONAL_NUMBER         = 161 //national number
}
ImiCcCallNumType;

typedef enum ImiCcCallHangupTypeTag
{
    IMI_CC_HANGUP_ALL               = 0,
    IMI_CC_HANGUP_ALL_HOLD_WAIT     = 1,
    IMI_CC_HANGUP_ONGOING           = 2
}
ImiCcCallHangupType;

typedef enum ImiCcCliValidityTag
{
    IMI_CC_CLI_VALID                = 0,
    IMI_CC_CLI_WITHHELD_BY_ORIG     = 1,//CLI has been withheld by the originator
    IMI_CC_CLI_INVALID_BY_NW        = 2,
    IMI_CC_CLI_INVALID_MO_PAYPHONE  = 3,
    IMI_CC_CLI_INVALID_OTHER        = 4
}
ImiCcCliValidity;

/******************************************************************************
 *****************************************************************************
 * IMI STRUCT
 *****************************************************************************
******************************************************************************/

typedef struct ImiCcCallInfoTag
{
    UINT8                   callId;//call Id number, 1-N, current 1-3?
    UINT8                   direction;//ImiCcCallDirection
    UINT8                   state;//ImiCcCallState
    UINT8                   mode;//ImiCcCallMode
    BOOL                    bMpty;// whether the call is one of multiparty (conference) call parties
    UINT8                   type;//ImiCcCallNumType
    UINT8                   resvd;
    UINT8                   dialNumStrLen;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
}
ImiCcCallInfo;

/******************************************************************************
 * IMI_CC_MAKE_CALL_REQ
 * ATD<number>
******************************************************************************/
typedef struct ImiCcMakeCallReqTag
{
    UINT8               dialNumStrLen;
    UINT8               resvd1;
    UINT16              resvd2;
    CHAR                dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
}
ImiCcMakeCallReq;

//IMI_CC_MAKE_CALL_CNF
typedef struct ImiCcMakeCallCnfTag
{
    UINT8                   ccRc;//ImiCcResultCode
    UINT8                   resvd1;
    UINT16                  resvd2;
}
ImiCcMakeCallCnf;

/******************************************************************************
 * IMI_CC_ANSWER_CALL_REQ
 * ATA
******************************************************************************/
typedef ImiEmptySig ImiCcAnswerCallReq;

//IMI_CC_ANSWER_CALL_CNF
typedef struct ImiCcAnswerCallCnfTag
{
    UINT8               ccRc;//ImiCcResultCode
    UINT8               resvd1;
    UINT16              resvd2;
}
ImiCcAnswerCallCnf;

/******************************************************************************
 * IMI_CC_HANGUP_CALL_REQ
 * ATH/AT+CHUP
******************************************************************************/
typedef struct ImiCcHangupCallReqTag
{
    UINT8               type;//
    UINT8               resvd1;
    UINT16              resvd2;
}
ImiCcHangupCallReq;

//IMI_CC_HANGUP_CALL_CNF
typedef ImiEmptySig ImiCcHangupCallCnf;

/******************************************************************************
 * IMI_CC_LIST_CURRENT_CALL_REQ
 * AT+CLCC
******************************************************************************/
typedef ImiEmptySig ImiCcListCurrCallReq;

//IMI_CC_LIST_CURRENT_CALL_CNF
typedef struct ImiCcListCurrCallCnfTag
{
    ImiCcCallInfo                   callInfoList[IMI_CC_MAX_CURRENT_CALLS];
}
ImiCcListCurrCallCnf;


/**
 * IMI_CC_SEND_DTMF_STR_REQ
*/
typedef struct ImiCcSendDtmfStrReqTag
{
    UINT16              duration;//ms
    BOOL                durationPresent;
    UINT8               dtmfStrLen;
    UINT8               dtmfStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
}
ImiCcSendDtmfStrReq;

/**
 * IMI_CC_SEND_DTMF_STR_CNF
*/
typedef  ImiEmptySig       ImiCcSendDtmfStrCnf;


/**
 * IMI_CC_SET_DTMF_DURATION_REQ
*/
typedef struct ImiCcSetDtmfDurationReqTag
{
    UINT16              duration;//ms
    UINT16              resvd;
}
ImiCcSetDtmfDurationReq;

/**
 * IMI_CC_SET_DTMF_DURATION_CNF
*/
typedef  ImiEmptySig       ImiCcSetDtmfDurationCnf;

/**
 * IMI_CC_GET_DTMF_DURATION_REQ
*/
typedef  ImiEmptySig       ImiCcGetDtmfDurationReq;

/**
 * IMI_CC_GET_DTMF_DURATION_CNF
*/
typedef struct ImiCcGetDtmfDurationCnfTag
{
    UINT16              duration;//ms
    UINT16              resvd;
}
ImiCcGetDtmfDurationCnf;




/**
 * IMI_CC_INCOMING_CALL_IND
*/
typedef struct ImiCcIncomingCallIndTag
{
    UINT8                   type;//ImiCcRingType
    UINT8                   rsvd1;
    UINT16                  rsvd2;
}
ImiCcIncomingCallInd;


/**
 * IMI_CC_CONN_LINE_ID_PRE_IND
 * +COLP
*/
typedef struct ImiCcConnLineIdPreIndTag
{
    UINT8                   type;//ImiCcCallNumType
    UINT8                   dialNumStrLen;
    UINT8                   saType;
    UINT8                   subAddrStrLen;
    UINT8                   alphaStrLen;
    UINT8                   rsvd1;
    UINT16                  rsvd2;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    subAddrStr[IMI_CC_MAX_SUB_ADDR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcConnLineIdPreInd;

/**
 * IMI_CC_CALL_LINE_ID_PRE_IND
 * +CLIP
*/
typedef struct ImiCcCallLineIdPreIndTag
{
    UINT8                   type;//ImiCcCallNumType
    UINT8                   dialNumStrLen;
    UINT8                   saType;
    UINT8                   subAddrStrLen;
    UINT8                   alphaStrLen;
    UINT8                   cliValidity;//ImiCcCliValidity
    UINT16                  rsvd;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    subAddrStr[IMI_CC_MAX_SUB_ADDR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcCallLineIdPreInd;

/**
 * IMI_CC_CALL_DISCONNECT_IND
*/
typedef ImiEmptySig ImiCcCallDisconnectInd;

/**
 * IMI_CC_CALL_WAITING_IND
 * +CCWA
*/
typedef struct ImiCcCallWaitingIndTag
{
    BOOL                    bFirstRpt;//whether the first report, call waiting urc is not repeat but RING repeat if call waiting URC disabled
    UINT8                   type;//ImiCcCallNumType
    UINT8                   class;
    UINT8                   resvd1;
    UINT8                   dialNumStrLen;
    UINT8                   alphaStrLen;
    UINT16                  resvd2;
    CHAR                    dialNumStr[IMI_CC_MAX_DIAL_NUMBER_STR_LEN];
    CHAR                    alphaStr[IMI_CC_MAX_ALPHA_ID_STR_LEN];
}
ImiCcCallWaitingInd;



#endif

