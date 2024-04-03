#ifndef PSIF_HDR_API_H
#define PSIF_HDR_API_H

/******************************************************************************
 ******************************************************************************
 Copyright:      - 2018- Copyrights of AirM2M Ltd.
 File name:      - psifapi.h
 Description:    - ps netif UL/DL function header
 History:        - 01/02/2018, Originated by xwang
 ******************************************************************************
******************************************************************************/
#include "pspdu.h"
#include "lwip/ip_addr.h"

#define PS_DLPDUBLOCK_SIZE sizeof(DlPduBlock)

#define PS_UL_RAM_ALLOC_MAX_LEN 4100

typedef enum PsIfRetEnum_Tag
{
    PSIF_SUCC = 0,
    PSIF_ERROR
}PsIfRetEnum;

typedef struct PsifUlRawData_Tag
{
    UINT16 len;
    UINT16 rsvd;
    struct PsifUlRawData_Tag *pNext;
    UINT8 pData[]; //the ul raw data buffer
}PsifUlRawData; //allocate by PsifAllocRawUlBlockMem();and it will be free with PsifFreeRawUlBlockMem() api or PsifFreeRawUlBlockMemList()

typedef struct PsifRawUlPkgPendingList_Tag
{
    UINT16          pkgNum; // the total pkg num of the pending list
    UINT16          rsvd;

    PsifUlRawData      *pHead;
    PsifUlRawData      *pTail;
}PsifRawUlPkgPendingList;
typedef struct PsifDlDataHeader_Tag
{
    UINT16 magic; //PS_UL_MAGIC
    UINT16 dlLen; // UL packet length
}PsifDlDataHeader;

/*
 * PS IF control header allocated before UL IP PKG send to PS
 *
 * +---------------------------+----------------------------------------------------+
 * | PsifUlDataHeader(4 bytes) |                  UL IP PKG                         |
 * +---------------------------+----------------------------------------------------+
*/
typedef struct PsifUlDataHeader_Tag
{
    UINT8 magic; //PS_UL_MAGIC
    UINT8 refCnt; // reference count
    UINT16 ulLen; // UL packet length
}PsifUlDataHeader;

typedef struct PsifUlPkgInfo_Tag
{
    UINT8 ipVer; //ip version
    UINT8 tos;
    UINT8 nextProto;
    UINT8 rsvd;
    UINT32 ipv6Fl; //if ipVer is ipv6, th traffic flow label
    UINT8 *srcIp;
    UINT8 *dstIp;
    UINT8  spi; //if nextProto is esp; the spi value within esp hdr
    UINT16 srcPort;
    UINT16 dstPort;//if nextProto is udp/tcp; the source port and destination port info within udp/tcp hdr
}PsifUlPkgInfo;


#define PBUF_DLRAM_HLEN  (sizeof(PsifDlDataHeader))
#define PBUF_ULRAM_HLEN  (sizeof(PsifUlDataHeader))


typedef UINT8   PsIfRet;

/******************************************************************************
 * PsifRawUlPkgListInput
 * Description: Raw ul pkg input
 * input: UINT8 cid, UINT16 pkgNum, PsifUlRawData *pHead, PsifUlRawData *pTail
 * output:
 * Comment:
 *
******************************************************************************/
void PsifRawUlPkgListInput(UINT8 cid, UINT16 pkgNum, PsifUlRawData *pHead, PsifUlRawData *pTail);


/******************************************************************************
 * PsifAllocRawUlBlockMem
 * Description: allocate ps ul raw data
 * input: UINT16 length(data buffer len)
 * output:PsifUlRawData *
 * Comment:
******************************************************************************/
PsifUlRawData *PsifAllocRawUlBlockMem(UINT16 length);

/******************************************************************************
 * PsifFreeRawUlBlockMem
 * Description: free ps ul raw data
 * input: PsifUlRawData *pUlRaw
 * output:void
 * Comment:
******************************************************************************/
void PsifFreeRawUlBlockMem(PsifUlRawData *pUlRaw);

/******************************************************************************
 * PsifFreeRawUlBlockMem
 * Description: free ps ul raw data list
 * input: PsifUlRawData *pUlRaw
 * output:void
 * Comment:
******************************************************************************/
void PsifFreeRawUlBlockMemList(PsifUlRawData *pUlRaw);

/*
 *
 * process the DL IP PKG,run within lwip task
 *you can define your dl ip pkg process function by set the external global var.  NetifRecvDlIpPkg gPsDlIpPkgProcFunc;(deault value is PNULL)
 *and you must free the dl pdu block by PsifFreeDlIpPkgBlockList
*/
typedef void ( *NetifRecvDlIpPkg)(UINT8 cid, DlPduBlock *pPduHdr);


/******************************************************************************
 *                           DL, NW -> UE, PS -> LWIP                        *
******************************************************************************/


/*
 * PSIF API:
 * DL IP PKG memory allocation API
 * 1> Here, LWIP could reserve the "pbuf" memory before this return memory, and LWIP don't need to copy the PKG data again
 *    in function "ps_receive_data()";
 * 2> This API dhouls safe for several tasks access
 * 3> ip pkg len must not bigger than: (PBUF_POOL_BUFSIZE - sizeof(struct pbuf) - sizeof(DlPduBlock))
 * The memory allocated:
 * +-----------------+--------------------------+--------------------------+
 * | pbuf (16 bytes) |   DlPduBlock (12 bytes)  |      ip pkg Len          |
 * +-----------------+--------------------------+--------------------------+
 *                   ^
 *                   | return address
 *
 * Input: UINT16 ipPkgLen = ip pkg len
 *
 * Note: DlPduBlock not need to init in this API
*/
DlPduBlock *PsifAllocDlIpPkgBlockMem(UINT16 ipPkgLen);

/*
 * LWIP API
 * DL IP PKG memory free API, free all list memory
 * if PS found the IP PKG is not right, could free the memory
*/
void PsifFreeDlIpPkgBlockList(DlPduBlock *pPkgBlock);

/*
 * LWIP API
 * process the DL IP PKG
 * 1> This API maybe called in ISR, don't take too much operation;
 * 2> PS -> LWIP, could send several IP pkg at one time;
 * 3> LWIP response for the memory free;
 * 4> 0 success,others fail
*/
PsIfRet PsifRecvDlIpPkg(UINT8 cid, DlPduBlock *pPduHdr);

/*
 * LWIP API
*/
void PsifFreeUlIpPkgMem(UINT8 *pPkgData);


/*
 * LWIP API
 * Whether LWIP DL high water, if return not zero, don't call "PsifAllocDlIpPkgBlockMem()" to allocate the DL memory;
*/
UINT8 PsifIsDlHighWater(void);


/*
 * LWIP API
 * Called by PS, notify LWIP task whether PS is suspended or resumed;
 * Note:
 * 1> if need to some operation with NETIF, suggest handle it in LWIP task;
 * 2> "bSuspend" = TRUE, just means PS suspended, can't send any UL pkg;
 * 3> "bSuspend" = FALSE, just means PS resumed, then LWIP could send UL pkg (maybe pended UL pkg);
 * 4> In some case, maybe UL "exception" data could still allow to send if suspended, here, we don't support it now; - TBD
*/
void PsifPsResumeSuspendInd(BOOL bSuspend);

/*
 * LWIP API
 * Check whether any UL pending(suspended) pkg in LWIP task;
*/
BOOL PsifAnySuspendULPendingPkg(void);


/*
 * PSIF API
 * check whether reserved size (LWIP_PBUF_STRUCT_LEN) is enough for pbuf
*/
void PsifPbufSizeCheck(void);

/*
 * PSIF API
 * Set and init pbuf struct, which located before "DlPduBlock", called by CEDR, when alloc DL PKG memory
*/
void PsifInitPsDlPkgPbuf(DlPduBlock *pDlPduBlk);

/*
 * PSIF API
 * Free pbuf which type is: PBUF_PS_DL_PKG, called by: pbuf_free()
 * input: struct pbuf *p
*/
void PsifFreePsDlPkgPbuf(void *pbuf);

UINT16 PsifGetCurrentPacketDelay(void);


/*
 * Free all netif TFT packet filter list
*/
void PsifNetifTftFree(void *tft_list);

BOOL PsifDlRamCheckHighWater(UINT16 preAllocLength);

struct pbuf * PsifTcpipDlHighWaterChkInput(struct pbuf *pInput);

#if 0
UINT8 *PsifAllocUlRamWithLimited(UINT16 len, UINT32 totalLimited);
UINT8 *PsifAllocUlRam(UINT16 len);
UINT8 *PsifAllocImsUlRam(UINT16 len);
#endif
UINT8 *PsifAllocUlRam(UINT16 len, BOOL bImsData);

void PsifFreeUlRam(UINT8 *pUlRam);

struct pbuf *PsifAllocUlRamBlockMem(UINT16 length);

void PsifFreeUlRamBlockMem(struct pbuf *p);

void PsifGenTftUlPkgInfo(PsifUlPkgInfo *pUlPkgInfo, UINT8 nextPro, const ip_addr_t *src, const ip_addr_t *dst, UINT8 tos, UINT32 spi, UINT32 ipv6FL, UINT16 srcPort, UINT16 dstPort);


/******************************************************************************
 * PsifTftRouteUlPkg
 * Description: Route the UL PDU to matched bearer, and return the CID of matched bearer
 * input:   UINT8 defaultIpv4Cid    //default ipv4 bearer CID, if no ipv4 CID, input: 0xFF
 *          UINT8 defaultIpv6Cid    //default ipv6 bearer CID, if no ipv6 CID, input: 0xFF
 *          void *pfList    //packet filter list
 *          UlPduBlock *pPdu            //UL PDU which need to dispatch
 * output:  UINT8 cid   // matched bearer cid
 * Note:
 * 1> if no bearer matched, return 0xFF (LWIP_PS_INVALID_CID)
******************************************************************************/
UINT8 PsifTftRouteUlPkg(UINT8 defaultIpv4Cid, UINT8 defaultIpv6Cid, void *pfList, PsifUlPkgInfo *pUlPkgInfo);

#endif
