/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef CCID_INTERFACE_H
#define CCID_INTERFACE_H



/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif



/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define CCID_DEBUG_EN


/* for short APDU max is 261 bytes, for extended APDU max is 65544 bytes
   customer need to support extended APDU, but should not exceed 2048 bytes due to limited sram
*/
#define CCID_ABDATA_SIZE                  2048
#define CCID_MSG_HDR_SIZE                 10
#define CCID_RESP_HDR_SIZE                10

#define INTR_MAX_PACKET_SIZE              8

#define CCID_NUM_OF_SLOTS                 1

// all CCID message

//PC----->CCID
#define PC_TO_RDR_ICCPOWERON                        0x62
#define PC_TO_RDR_ICCPOWEROFF                       0x63
#define PC_TO_RDR_GETSLOTSTATUS                     0x65
#define PC_TO_RDR_XFRBLOCK                          0x6F
#define PC_TO_RDR_GETPARAMETERS                     0x6C
#define PC_TO_RDR_RESETPARAMETERS                   0x6D
#define PC_TO_RDR_SETPARAMETERS                     0x61
#define PC_TO_RDR_ESCAPE                            0x6B
#define PC_TO_RDR_ICCCLOCK                          0x6E
#define PC_TO_RDR_T0APDU                            0x6A
#define PC_TO_RDR_SECURE                            0x69
#define PC_TO_RDR_MECHANICAL                        0x71
#define PC_TO_RDR_ABORT                             0x72
#define PC_TO_RDR_SETDATARATEANDCLOCKFREQUENCY      0x73

//CCID----->PC
#define RDR_TO_PC_DATABLOCK                         0x80
#define RDR_TO_PC_SLOTSTATUS                        0x81
#define RDR_TO_PC_PARAMETERS                        0x82
#define RDR_TO_PC_ESCAPE                            0x83
#define RDR_TO_PC_DATARATEANDCLOCKFREQUENCY         0x84



//notify message to PC via INT EP
#define RDR_TO_PC_NOTIFYSLOTCHANGE                  0x50
#define RDR_TO_PC_HARDWAREERROR                     0x51




#define SLOT_CURSTATE_NO_ICC_PRESENT                0x0
#define SLOT_CURSTATE_ICC_PRESENT                   0x1

#define SLOT_CHGSTATUS_NO_CHANGE                    0x0
#define SLOT_CHGSTATUS_CHANGE                       0x2


#define HARDWARE_ERROR_CODE_OVERCURRENT             0x1



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

//define CCID BULK OUT message callback, register by user

typedef uint8_t (*ccidIccPowerOnCb)( void );
typedef uint8_t (*ccidIccPowerOffCb)( void );
typedef uint8_t (*ccidGetSlotStatusCb)( void );
typedef uint8_t (*ccidXfrBlockCb)(uint8_t* ptr, uint32_t len);
typedef uint8_t (*ccidGetParaOnCb)( void );
typedef uint8_t (*ccidResettParaOnCb)( void );
typedef uint8_t (*ccidSetParaOnCb)( void );
typedef uint8_t (*ccidEscapeCb)( void );
typedef uint8_t (*ccidIccClockCb)( void );
typedef uint8_t (*ccidT0ApduCb)( void );
typedef uint8_t (*ccidSecureCb)( void );
typedef uint8_t (*ccidMechanicalCb)( void );
typedef uint8_t (*ccidAbortCb)( void );
typedef uint8_t (*ccidSetDataRateAndClkFreqCb)( void );


typedef enum
{
    CCID_STATUS_INIT=0,//default
    CCID_STATUS_RECV_DATA,
    CCID_STATUS_WAIT_RESP,
    CCID_STATUS_MAX
 }CcidIfStatus_e;


#pragma pack(push, 1)

typedef struct
{
  uint8_t bmFindexDindex;
  uint8_t bmTCCKST0;
  uint8_t bGuardTimeT0;
  uint8_t bWaitingIntegerT0;
  uint8_t bClockStop;
} ProtocolT0_t;


typedef struct
{
  uint8_t bmFindexDindex;
  uint8_t bmTCCKST1;
  uint8_t bGuardTimeT1;
  uint8_t bWaitingIntegerT1;
  uint8_t bClockStop;
  uint8_t bifsc;
  uint8_t bNadVal;
} ProtocolT1_t;


typedef struct
{
    uint8_t bMessageType;
    uint32_t dwLength;    /*4B , msg specific data len, not including the 10-byte header.*/
    uint8_t bSlot;
    uint8_t bSeq;
    uint8_t bMsgSpec0;
    uint8_t bMsgSpec1;
    uint8_t bMsgSpec2;
    uint8_t abData[CCID_ABDATA_SIZE]; /* max size differ per spec:
                                        TPDU T=0 block is 260 bytes
                                        (5 bytes command; 255 bytes data),
                                        TPDU T=1 block is 259 bytes,
                                        short APDU T=1 block is 261 bytes,
                                        an extended APDU T=1 block is 65544 bytes.*/
} CcidBbulkoutData_t;

typedef struct
{
    uint8_t bMessageType;
    uint32_t dwLength;    /*4B , msg specific data len, not including the 10-byte header.*/
    uint8_t bSlot;
    uint8_t bSeq;
    uint8_t bStatus;/*bit1:0 iccstatus bit7:6 command status*/
    uint8_t bError;
    uint8_t bMsgSpec0;
    uint8_t abData[CCID_ABDATA_SIZE];
    uint16_t u16SizeToSend;
} CcidBulkinData_t;




typedef struct
{
    ccidIccPowerOnCb usrCcidIccPowerOnCb;
    ccidIccPowerOffCb usrccidIccPowerOffCb;
    ccidGetSlotStatusCb usrccidGetSlotStatusCb;
    ccidXfrBlockCb usrccidXfrBlockCb;
    ccidGetParaOnCb  usrccidGetParaOnCb;
    ccidResettParaOnCb usrccidResettParaOnCb;
    ccidSetParaOnCb usrccidSetParaOnCb;
    ccidEscapeCb usrccidEscapeCb;
    ccidIccClockCb  usrccidIccClockCb;
    ccidT0ApduCb usrccidT0ApduCb;
    ccidSecureCb usrccidSecureCb;
    ccidMechanicalCb  usrccidMechanicalCb;
    ccidAbortCb usrccidAbortCb;
    ccidSetDataRateAndClkFreqCb usrccidSetDataRateAndClkFreqCb;
}UsrCcidCb_t;



typedef struct
{
    CcidIfStatus_e ccidIfState;                             /*ccidIf msg handle state*/
    uint16_t gApduProcess;                                  /*record wLevelParameter in extended apdu mode in XfeBlcok*/
    CcidBulkinData_t   gCcidBulkinData;                     /*ccid bulk in format data*/
    CcidBbulkoutData_t gCcidBulkouData;                     /*ccid bulk out format data*/
    ProtocolT0_t gPT0DataStructure;                         /*ccid protocol T0 structure in RDR_to_PC_Parameters*/
    ProtocolT1_t gPT1DataStructure;                         /*ccid protocol T1 structure in RDR_to_PC_Parameters*/
    UsrCcidCb_t  gUsrBulkOutMsgCb;                          /*user layer callback*/

    #ifdef CCID_SUPPORT_NOTIFY_MSG
    uint8_t gUsbIntMessageBuffer[INTR_MAX_PACKET_SIZE];     /* notify data buffer*/
    #endif
} UsbCcidCtrlBlk_t;

#pragma pack(pop)



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
void CCID_ReceiveCmdHeader(uint8_t* pDst, uint8_t u8length);
void CCID_CmdDecode( void );
void ccidIfInit ( void );
void ccidIfDeInit ( void );
void ccidSendDatatoHost(uint8_t* buf, uint16_t len);
void ccidOutDataHanlder (uint8_t *recvBuf, uint32_t len);

#ifdef CCID_SUPPORT_NOTIFY_MSG
void ccidSendSlotChgNotifyMessage( void );
void ccidSendHwErrNotifyMessage( uint8_t errorCode );
#endif




#ifdef __cplusplus
}
#endif

#endif


