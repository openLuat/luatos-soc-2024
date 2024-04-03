/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imisms.h
 Description:    - Declare  the  SMS API for IMI
 History:        - 2023/04/10,   Original created
******************************************************************************/
#ifndef IMI_SMS_H
#define IMI_SMS_H

#include "imicomm.h"

/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_SMS_PRIM_ID_TAG
{
    IMI_SMS_PRIM_BASE = 0,

    IMI_SMS_SEND_MESSAGE_REQ,       /* ImiSmsSendMessageReq */
    IMI_SMS_SEND_MESSAGE_CNF,       /* ImiSmsSendMessageCnf */

    IMI_SMS_SEND_SMMA_REQ,          /* ImiSmsSendSmmaReq */
    IMI_SMS_SEND_SMMA_CNF,          /* ImiSmsSendSmmaCnf */

    IMI_SMS_SEND_MESSAGE_ABORT_REQ, /* ImiSmsSendMessageAbortReq */
    IMI_SMS_SEND_MESSAGE_ABORT_CNF, /* ImiSmsSendMessageAbortCnf */

    IMI_SMS_PRIM_END    = 0x0fff
}IMI_SMS_PRIM_ID;

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

/**
 * IMI_SMS_SEND_MESSAGE_REQ
*/
#define IMI_SMS_MAX_PDU_SIZE                180
#define IMI_SMS_MAX_LENGTH_OF_ADDRESS_VALUE 40

typedef struct ImiSmsSendMessageReqTag
{
    UINT8   smsId;                  /* SMS ID */
    UINT8   smscLength;             /* Destination Service Centre address */
    UINT8   smscTypeOfNumber;       /* TypeOfNumber */
    UINT8   smscNumberPlanId;       /* NumberingPlanId */

    UINT8   smscAddressValue[IMI_SMS_MAX_LENGTH_OF_ADDRESS_VALUE];

    UINT8   smsRef;
    UINT8   reserved0;
    UINT16  pduLength;

    UINT8   pduData[IMI_SMS_MAX_PDU_SIZE];
}ImiSmsSendMessageReq;

typedef struct ImiSmsRpCauseElement_Tag
{
    UINT8               cause;      /* RpCause */
    UINT8               diagnostic;
    UINT16              reserved0;
}ImiSmsRpCauseElement;

typedef struct ImiSmsRpUserData_Tag
{
    UINT16     length;
    UINT16     reserved0;
    UINT8     *rpUserData;
}ImiSmsRpUserData;

/**
 * IMI_SMS_SEND_MESSAGE_CNF
*/
#define IMI_SMS_RETRY_OVER_SGS                  0
#define IMI_SMS_RETRY_OVER_IP                   1
#define IMI_SMS_RETRY_SMMA_OVER_SGS             2
#define IMI_SMS_RETRY_SMMA_OVER_IP              3
#define IMI_SMS_NO_RETRY                        4

typedef struct ImiSmsSendMessageCnfTag
{
    UINT8                           shortMsgId;
    UINT8                           smsRef;             /* Message Reference */
    UINT8                           statusOfReport;     /* SmsStatusOfReport */
    UINT8                           tpCause;            /* TpCause */
    ImiSmsRpCauseElement            rpCause;            /* RpCause */
    ImiSmsRpUserData                rpUserData;
    UINT16                          tpduRawDatLen;
    UINT8                           retryType;
    UINT8                           reserved0;
    UINT8                           *tpduRawDat;
}ImiSmsSendMessageCnf;

/**
 * IMI_SMS_SEND_SMMA_REQ
*/
typedef struct ImiSmsSendSmmaReq_Tag
{
    UINT8   smsId;                  /* SMS ID */
    UINT8   smscLength;             /* Destination Service Centre address */
    UINT8   smscTypeOfNumber;       /* TypeOfNumber */
    UINT8   smscNumberPlanId;       /* NumberingPlanId */

    UINT8   smscAddressValue[IMI_SMS_MAX_LENGTH_OF_ADDRESS_VALUE];

    UINT8   smsRef;
    UINT8   reserved0[3];
}ImiSmsSendSmmaReq;

/**
 * IMI_SMS_SEND_SMMA_CNF
*/
typedef ImiSmsSendMessageCnf ImiSmsSendSmmaCnf;

/**
 * IMI_SMS_SEND_MESSAGE_ABORT_REQ
*/
typedef struct ImiSmsSendMessageAbortReqTag
{
    UINT8   smsId;                  /* SMS ID */
    UINT8   reserved0[3];
}ImiSmsSendMessageAbortReq;

/**
 * IMI_SMS_SEND_MESSAGE_ABORT_CNF
*/
typedef struct  ImiSmsSendMessageAbortCnfTag
{
    UINT8   smsId;                  /* SMS ID */
    UINT8   reserved0[3];
}ImiSmsSendMessageAbortCnf;

/******************************************************************************
 ******************************************************************************
 * extern function
 ******************************************************************************
******************************************************************************/

void ImaSmsImiSendMessageCnfFunc(ImiSmsSendMessageCnf *pSmsSendMessageCnf);
void ImaSmsImiSendMessageAbortCnfFunc(ImiSmsSendMessageAbortCnf *pSmsSendMessageAbortCnf);
void ImaSmsImiSendSmmaCnfFunc(ImiSmsSendSmmaCnf *pSmsSendSmmaCnf);

#endif


