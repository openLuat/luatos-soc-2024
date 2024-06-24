/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_spic.h
*
*  Description:
*
*  History: 2023/1/31 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_SPIC_H
#define CCIO_SPIC_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define SPIC_MSG_MAGIC_WORD         0x43495053 /* 'C','I','P','S' */

#define SPIC_NOTIF_FLOWCTL_ON       0x70000001
#define SPIC_NOTIF_FLOWCTL_OFF      0x70000002
typedef uint32_t  SpicNotif_t;

/* return code for 'SPIC_GEN_STATUS_IND_MSG' */
#define SPIC_STATUS_SUCC            0x00000000
#define SPIC_STATUS_FAIL            0xe0000001
#define SPIC_STATUS_DATA_ERR        0xe0010002
#define SPIC_STATUS_UNSUPP_ERR      0xe0000003
#define SPIC_STATUS_FLOWCTL_ON      0xf0000001
#define SPIC_STATUS_FLOWCTL_OFF     0xf0000002
#define SPIC_STATUS_MEDIA_CONN      0xf0000003
#define SPIC_STATUS_MEDIA_DISCONN   0xf0000004
typedef uint32_t  SpicStatus_t;


/* generic message set for spi device */
#define SPIC_GEN_NOTIFICATION_MSG   0x0001
#define SPIC_GEN_STATUS_IND_MSG     0x8001

/* network message set for spi device */
#define SPIC_NET_RESET_MSG          0x0401
#define SPIC_NET_RESET_CMPLT        0x8401
#define SPIC_NET_INITIAL_MSG        0x0402
#define SPIC_NET_INITIAL_CMPLT      0x8402
#define SPIC_NET_QUERY_MSG          0x0403
#define SPIC_NET_QUERY_CMPLT        0x8403
#define SPIC_NET_SET_MSG            0x0404
#define SPIC_NET_SET_CMPLT          0x8404
#define SPIC_NET_KEEPALIVE_MSG      0x0405
#define SPIC_NET_KEEPALIVE_CMPLT    0x8405

typedef uint16_t  SpicMsgType_t;
typedef uint16_t  SpicMsgSize_t;
typedef uint32_t  SpicSeqId_t;
typedef uint32_t  SpicObjId_t;

#define SPIC_CHECK_MAGIC(ptr)      (((SpicMsgHdr_t*)(ptr))->magic == SPIC_MSG_MAGIC_WORD)
/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

/*+------------+------------------------+
 *|  FrameHdr  |        Obj Info        |
 *+------------+------------------------+
 */

typedef struct
{
    uint32_t  magic;     /* "SPIC" */
    SpicMsgType_t  type;
    SpicMsgSize_t  len;  /* including hdr size */
    uint32_t  rsvd;
    uint8_t   body[0];
}SpicMsgHdr_t;

typedef struct
{
    SpicStatus_t status;
    uint32_t  rsvd;
}SpicStatusInd_t;

typedef struct
{
    SpicNotif_t notif;
    uint32_t  rsvd;
}SpicGenNotif_t;

typedef struct
{
    uint32_t  rsvd;
}SpicNetResetMsg_t;

typedef struct
{
    SpicStatus_t status;
    uint32_t  rsvd;
}SpicNetResetCmplt_t;

typedef struct
{
    SpicSeqId_t  sid;
    uint16_t  majVer;
    uint16_t  minVer;
    uint32_t  maxXferSize;
}SpicNetInitialMsg_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicStatus_t status;
    uint16_t  majVer;
    uint16_t  minVer;
    uint32_t  maxXferSize;
    uint16_t  maxPktsPerXfer;
    uint16_t  pktAlignValue;
}SpicNetInitialCmplt_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicObjId_t  oid;
}SpicNetQueryMsg_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicStatus_t status;
    uint32_t  rsvd;
}SpicNetQueryCmplt_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicObjId_t  oid;
}SpicNetSetMsg_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicStatus_t status;
    uint32_t  rsvd;
}SpicNetSetCmplt_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicObjId_t  oid;
}SpicNetKeepAliveMsg_t;

typedef struct
{
    SpicSeqId_t  sid;
    SpicStatus_t status;
    uint32_t  rsvd;
}SpicNetKeepAliveCmplt_t;

typedef union
{
    SpicGenNotif_t          genNotif;
    SpicStatusInd_t         genStatusInd;

    SpicNetResetMsg_t       netResetMsg;
    SpicNetResetCmplt_t     netResetCmplt;
    SpicNetInitialMsg_t     netInitialMsg;
    SpicNetInitialCmplt_t   netInitialCmplt;
    SpicNetQueryMsg_t       netQueryMsg;
    SpicNetQueryCmplt_t     netQueryCmplt;
    SpicNetSetMsg_t         netSetMsg;
    SpicNetSetCmplt_t       netSetCmplt;
    SpicNetKeepAliveMsg_t   netKeepAliveMsg;
    SpicNetKeepAliveCmplt_t netKeepAliveCmplt;
}SpicMsgBody_u;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
int32_t  spiFrameHdrDecap(UlPduBlock_t **ulpdu, uint16_t *frameLen);

int32_t  spicMsgFilter(UlPduBlock_t **ulpdu, uint8_t isSubx, void *extras);
int32_t  spicMsgSend(CcioDevice_t *chdev, SpicMsgType_t type, SpicStatus_t status, void *args);

#ifdef __cplusplus
}
#endif
#endif

