/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_misc.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes: adaptor layer of reference data type(struct/macro/...)
*
******************************************************************************/
#ifndef CCIO_MISC_H
#define CCIO_MISC_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "uldp.h"
#include "pspdu.h"
#include "osaulfcmem.h"
#include "osadlfcmem.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
//#define CCIO_ULPDU_FAST_FREE_ENABLE
#define CCIO_NETDATA_LIST_INPUT_ENABLE

/* the overhead of spi frame */
#define SPI_FRAME_HDR_SIZE       4

/* ip net MTU of data */
#define CCIO_IPNET_DATA_MTU      1500

/* min size of recv window for each channel */
#define CCIO_RX_WIN_MINSIZE      512

/* NOTE: must be LE than sizeof(NmAtiGetNetInfoRet) */
#define CCIO_WAN_CONF_MAXSIZE    128

/* max rndis packets in one transfer */
#define CCIO_RNDIS_MAX_PKT_PER_XFER  10 //RNDIS_MAX_PACK_PER_XFER

/* max size of rndis packet */
#define CCIO_RNDIS_XFER_MAXSIZE  3584 //ALIGNUP(RNDIS_RX_BUFFER_SIZE)

/* max size of ecm packet
 * --------------------------------------------------------------------
 * ATTENTION:
 * there will be no ZLP in some vendors' Windows driver,
 * and the xfer maxsize should be set to 512 @cost of uplink data rate!
 * --------------------------------------------------------------------
 */
#define CCIO_ECM_XFER_MAXSIZE    1536 //ALIGNUP(eth mtu 1514)

/* max size of ppp packet over serial */
#define CCIO_PPP_XFER_MAXSIZE    1536 //ALIGNUP(ppp mtu 1508)

/* max size of normal packet over serial, e.g. AT, OPAQ, PPP
 * [caution]: as being restricted by usb driver,
 * some data of particular size(e.g 512/1024) can't be transfered @maxsize = 1536!
 */
#define CCIO_NORM_XFER_MAXSIZE   512

/* max size of frame over spi */
#define CCIO_SPI_XFER_MAXSIZE    (CCIO_RBUF_XTRA_SIZE + SPI_FRAME_HDR_SIZE + CCIO_NORM_XFER_MAXSIZE * 3/*ETH_FRAME_HDR_SIZE + CCIO_IPNET_DATA_MTU*/)

/**
 * extra size for managing ulpdu link node
 * when data was written in two contiguous ram blocks.
 * and it often occurs when the size between write_idx
 * and the buffer end is smaller than data length and
 * it has to split it and write the remaining data to
 * the ring buffer head again.
 */
#define CCIO_RBUF_XTRA_SIZE      CCIO_RBUF_NPT_HDR_SIZE

/**
 * fixed size for uldp control block
 * and it equals to sizeof(UldpHdrInfo_t) + sizeof(UlPduBlock_t).
 */
#define CCIO_RBUF_NPT_HDR_SIZE   (sizeof(UldpHdrInfo_t) + sizeof(UlPduBlock_t))

/* dlpdu maxCnt threshold */
#define CCIO_DLPDU_PEND_MAXNUM   64

/* supported cmux dlc count for now */
#define CCIO_CMUX_DLC_REAL_CNT   CCIO_CHAN_VIRT_MAXNUM

#define CCIO_ALIGN_UP(x,sz)      (((size_t)(x) + ((sz) - 1)) & (~((sz) - 1)))
#define CCIO_ALIGN_DOWN(x,sz)    ((size_t)(x) & (~((sz) - 1)))

#define CCIO_CASE_UPPER(c)       ((((c) >= 'a') && ((c) <= 'z')) ? ((c) - ('a' - 'A')) : (c))
#define CCIO_CASE_LOWER(c)       ((((c) >= 'A') && ((c) <= 'Z')) ? ((c) + ('a' - 'A')) : (c))

#define CCIO_MAX(x,y)            (((x) > (y)) ? (x) : (y))
#define CCIO_MIN(x,y)            (((x) < (y)) ? (x) : (y))
#define CCIO_ELEMNUM(x)          (sizeof(x)/sizeof(x[0]))

#if 1
#define CCIO_HTONS(x)            ((((x) & 0x00ffUL) << 8) | (((x) & 0xff00UL) >> 8))
#define CCIO_HTONL(x)            ((((x) & 0x000000ffUL) << 24) | \
                                  (((x) & 0x0000ff00UL) <<  8) | \
                                  (((x) & 0x00ff0000UL) >>  8) | \
                                  (((x) & 0xff000000UL) >> 24))
#else /* Big Endian */
#define CCIO_HTONS(x)
#define CCIO_HTONL(x)
#endif
#define CCIO_NTOHS(x)            CCIO_HTONS(x)
#define CCIO_NTOHL(x)            CCIO_HTONL(x)

#define CCIO_RBUF_ULDP_HDR_BUILD(uldph, len, isCmplt, crcRes)   \
             do\
             {\
                 (uldph)->magicNum     = ULDP_HDR_MAGIC;\
                 (uldph)->msgLen       = (len);\
                 (uldph)->isRdsPppCmpl = (isCmplt);\
                 (uldph)->crcResult    = (crcRes);\
             }while(0)

#define CCIO_RBUF_ULPDU_BUILD(ulpdu, chno, buf, len, bsame, bcont, next)  \
                              UL_PDU_BLOCK_SET_BASICS(ulpdu, chno, UL_RBUF_MEM, bsame, buf, len, 0, bcont, next)

#define CCIO_HEAP_DLPDU_BUILD(dlpdu, buf, len)   \
                              DL_PDU_BLOCK_SET_BASICS(dlpdu, DL_HEAP_MEM, buf, len, 0, NULL)
#define CCIO_DLFC_DLPDU_BUILD(dlpdu, buf, len)   \
                              DL_PDU_BLOCK_SET_BASICS(dlpdu, DL_AFC_MEM, buf, len, 0, NULL)

#define CCIO_ULPDU_LIST_APPEND(list, ulpdu)   \
             do\
             {\
                 if(((UlPduBlockList_t*)(list))->tail)\
                 {\
                     ((UlPduBlockList_t*)(list))->tail->pNext = (ulpdu);\
                 }\
                 else\
                 {\
                     ((UlPduBlockList_t*)(list))->head = ((UlPduBlockList_t*)(list))->tail = (ulpdu);\
                 }\
                 while(((UlPduBlockList_t*)(list))->tail->pNext)\
                 {\
                     ((UlPduBlockList_t*)(list))->tail = ((UlPduBlockList_t*)(list))->tail->pNext;\
                 }\
             }while(0)

#define CCIO_ULPDU_LIST_APPEND_LIST(list, hd, tl)   \
             do\
             {\
                 if(((UlPduBlockList_t*)(list))->tail)\
                 {\
                     ((UlPduBlockList_t*)(list))->tail->pNext = (hd);\
                     ((UlPduBlockList_t*)(list))->tail = (tl);\
                 }\
                 else\
                 {\
                     ((UlPduBlockList_t*)(list))->head = (hd);\
                     ((UlPduBlockList_t*)(list))->tail = (tl);\
                 }\
             }while(0)

#define CCIO_DLPDU_LIST_APPEND(list, dlpdu)   \
             do\
             {\
                 if(((DlPduBlockList_t*)(list))->tail)\
                 {\
                     ((DlPduBlockList_t*)(list))->tail->pNext = (dlpdu);\
                 }\
                 else\
                 {\
                     ((DlPduBlockList_t*)(list))->head = ((DlPduBlockList_t*)(list))->tail = (dlpdu);\
                 }\
                 while(((DlPduBlockList_t*)(list))->tail->pNext)\
                 {\
                     ((DlPduBlockList_t*)(list))->tail = ((DlPduBlockList_t*)(list))->tail->pNext;\
                 }\
             }while(0)

#define CCIO_DLPDU_LIST_APPEND_LIST(list, hd, tl)   \
             do\
             {\
                 if(((DlPduBlockList_t*)(list))->tail)\
                 {\
                     ((DlPduBlockList_t*)(list))->tail->pNext = (hd);\
                     ((DlPduBlockList_t*)(list))->tail = (tl);\
                 }\
                 else\
                 {\
                     ((DlPduBlockList_t*)(list))->head = (hd);\
                     ((DlPduBlockList_t*)(list))->tail = (tl);\
                 }\
             }while(0)

#define CCIO_CONVTO_LANTYPE(lmt)  (((lmt) == CCIO_LAN_MEDIA_RNDIS) ? NM_LAN_TYPE_RNDIS \
                                  : ((((lmt) == CCIO_LAN_MEDIA_ECM) || ((lmt) == CCIO_LAN_MEDIA_EOS)) ? NM_LAN_TYPE_ECM \
                                  : (((lmt) == CCIO_LAN_MEDIA_PPP) ? NM_LAN_TYPE_PPP \
                                                                   : NM_LAN_TYPE_INVALID)))
#define CCIO_CONVTO_LANNETTYPE(lmt)  ((((lmt) == CCIO_LAN_MEDIA_RNDIS) || ((lmt) == CCIO_LAN_MEDIA_ECM) || ((lmt) == CCIO_LAN_MEDIA_EOS)) ? NM_LAN_NET_TYPE_ETH \
                                  : (((lmt) == CCIO_LAN_MEDIA_PPP) ? NM_LAN_NET_TYPE_PPP \
                                                                   : NM_LAN_NET_TYPE_INVALID))
#define CCIO_CONVTO_LANSTATUS(lms)  (((lms) == CCIO_LMS_CONNECTED) ? NM_LAN_LINK_STATUS_CONNECTED \
                                                                   : NM_LAN_LINK_STATUS_DISCONNECTED)

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
/* singly linked list */
typedef struct CcioSlistNode
{
    struct CcioSlistNode  *next;
}CcioSlistNode_t;

typedef CcioSlistNode_t CcioSlist_t;

/* dual linked list */
typedef struct CcioDlistNode
{
    struct CcioDlistNode  *prev;
    struct CcioDlistNode  *next;
}CcioDlistNode_t;

typedef CcioDlistNode_t CcioDlist_t;


typedef UlPduBlock UlPduBlock_t;
typedef DlPduBlock DlPduBlock_t;

typedef struct
{
    UlPduBlock_t  *head;
    UlPduBlock_t  *tail;
}UlPduBlockList_t;

typedef struct
{
    DlPduBlock_t  *head;
    DlPduBlock_t  *tail;
}DlPduBlockList_t;

typedef struct CmuxFrameDesc
{
    //struct CmuxFrameDesc *next;

    uint8_t   address;    /* the frame channel */
    uint8_t   control;    /* the type of frame */
    uint8_t   inState;    /* refer to 'CmuxPdState_e' */
    uint8_t   inFcs;      /* fcs of input data */
    uint16_t  length;     /* remaining length of the frame */
    uint16_t  isCmplt  :1;
    uint16_t  rsvdBits :7;
    uint16_t  errno    :8;  /* CmuxFrameErrNo_e */
    UlPduBlockList_t frame; /* one complete frame */
}CmuxFrameDesc_t;

typedef struct
{
    OsaUlfcMemEvtFlags_e  flags;
    OsaUlfcMemEvtArgs_t  *args;
}CcioUlfcMemEvent_t;

typedef struct
{
    OsaDlfcMemEvtFlags_e  flags;
    OsaDlfcMemEvtArgs_t  *args;
}CcioDlfcMemEvent_t;

typedef enum
{
    CCIO_OXF_RAW_DATA,
    CCIO_OXF_CMD_LINE,

    CCIO_OXF_MAXNUM
}CcioOutXferFormat_e;

typedef enum
{
    CCIO_OXM_PEND_LIST,
    CCIO_OXM_SEND_PDU,

    CCIO_OXM_MAXNUM
}CcioOutXferMode_e;

typedef struct
{
    uint16_t         blockCnt[CCIO_OXF_MAXNUM];
    DlPduBlockList_t blockList[CCIO_OXF_MAXNUM];
}CcioDlPendListMan_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
int32_t  ccioSlistAppend(CcioSlist_t **list, CcioSlist_t *nodes);
int32_t  ccioSlistRemove(CcioSlist_t **list, CcioSlistNode_t *node);
int32_t  ccioSlistTraverse(CcioSlist_t *list, int32_t (*func)(void *node));

/**
 * @brief ccioGetOnePktFromUlList(UlPduBlock_t  *list,
                                  UlPduBlock_t **pktHead,
                                  UlPduBlock_t **pktTail,
                                  uint16_t      *pktLen)
 * @details get the first complete pkt from an ulpdu list
 *
 * @param list     The orignal list
 * @param pktHead  The head of the packet got from the list
 * @param pktTail  The tail of the packet got from the list
 * @param pktLen   The length of the packet got from the list
 * @return the remaining list.
 */
UlPduBlock_t* ccioGetOnePktFromUlList(UlPduBlock_t  *list,
                                      UlPduBlock_t **pktHead,
                                      UlPduBlock_t **pktTail,
                                      uint16_t      *pktLen);


#ifdef __cplusplus
}
#endif
#endif

