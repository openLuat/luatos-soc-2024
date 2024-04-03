#ifndef __SCT_USB_TX_API_H__
#define __SCT_USB_TX_API_H__

/******************************************************************************
 ******************************************************************************
  Copy right:   2017-, Copyrigths of AirM2M Ltd.
  File name:    sct_usb_tx_api.h
  Description:  SCT PPP and CRC API, external provide to app
  History:      2020/12/02    Originated by Jason
 ******************************************************************************
******************************************************************************/
#include "pspdu.h"


#if defined CHIP_EC618
#undef  ENABLE_UTFW
#else
#define ENABLE_UTFW
#endif

//#define ENABLE_UTFW
//#undef  ENABLE_UTFW


#ifndef ENABLE_UTFW
/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/
#define SCT_USB_TX_EP_PARALLEL


/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

typedef enum
{
    SCT_USB_TX_RET_OK   = 0,
    SCT_USB_TX_ERR      = -1,
    SCT_USB_TX_BUSY     = -2
}SctUsbTxRetEnum;

typedef INT32   SctUsbTxRet;


/*
 * USB Tx Block
*/
typedef DlPduBlock      SctUsbTxBlock;


/*
 * SCT USB ISR event
*/
typedef enum
{
    SCT_USB_DEINT   = 0,    /* when USB plug out, or suspended */
    SCT_USB_INIT    = 1,    /* USB is OK (EP enumed), and SCT could sent data to USBC TxFIFO, when USB plug in, or resumed */
    SCT_USB_RESET   = 2     /* USB is reseted, SCT USB channel should be reset, and wait USB init(OK) again. */
}SctUsbStateEvent;


/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/


/******************************************************************************
 * SctUsbTxRawDataNew
 * Description: SCT USB Tx raw data, without cipher/CRC/PPP operation.
 * input:   UINT32 epId     //USB EP ID < 16
 *          SctUsbTxBlock *pTxHead      //input, Tx block header
 *          SctUsbTxBlock *pTxTail      //input, Tx block header
 *          BOOL          allowConcatenated     //input, whether allow serveral PDU Tx in one USB transfer
 * output:  SctUsbTxRet
 * Comment:
 * 1> As the MAX USB Tx transfer size is limited, Tx block maybe can't transmit one time
 *    then, "pRetHead" will return the next not transmited block header
 * 2> This API return OK, just means block inserted into SCT FIFO, not means already
 *    transmited to USB host, so the caller: !!! can't free the block memory !!!
 *    let the SCT responsible for freeing it after SCT done
 * 3> This API could only be called in USB TX task
 * 4> The caller don't need any handling after called this API
******************************************************************************/
SctUsbTxRet SctUsbTxRawDataNew(UINT32 epId, SctUsbTxBlock *pTxHead, SctUsbTxBlock *pTxTail, BOOL allowConcatenated);


/******************************************************************************
 * SctUsbTxPppDataNew
 * Description: SCT USB Tx raw data, data need PPP escaption.
 * input:   UINT32 epId     //USB EP ID < 16
 *          SctUsbTxBlock *pTxHead      //input, Tx block header
 *          SctUsbTxBlock *pTxTail      //input, Tx block header
 * output:  SctUsbTxRet
 * Comment:
 * 1> As the MAX USB Tx transfer size is limited, Tx block maybe can't transmit one time
 *    then, "pRetHead" will return the next not transmited block header
 * 2> This API return OK, just means block inserted into SCT FIFO, not means already
 *    transmited to USB host, so the caller: !!! can't free the block memory !!!
 *    let the SCT responsible for freeing it after SCT done
 * 3> This API could only be called in USB TX task
 * 4> The caller don't need any handling after called this API
******************************************************************************/
SctUsbTxRet SctUsbTxPppDataNew(UINT32 epId, SctUsbTxBlock *pTxHead, SctUsbTxBlock *pTxTail);

/******************************************************************************
 * SctUsbProcDoneChainInfo
 * Description: SCT USB process chain done info
 * input:   BOOL *pSctAvailable     //
 * output:  void
 * Comment:
 * 1> Called when USB Tx task scheduled
 * 2> if "pSctAvailable" return TRUE, just means SCT USB channel is available again
 *    and the caller could Tx data again
******************************************************************************/
void SctUsbProcDoneChainInfo(BOOL *pSctAvailable);

/******************************************************************************
 * SctUsbProcUsbEvent
 * Description: SCT USB process USB event notification
 * input:   SctUsbStateEvent usbState
 * output:  void
 * Comment:
 * 1> Called when USB Tx task when USB event signal/message received
******************************************************************************/
void SctUsbProcUsbEvent(SctUsbStateEvent usbEvent);

/******************************************************************************
 * SctUsbProcTimerExpiry
 * Description: SCT USB retry timer expiry handling
 * input:   UINT32 tid
 * output:  void
 * Comment:  Called when CCIO Tx task
******************************************************************************/
void SctUsbProcTimerExpiry(UINT32 tid);

/*
 * SCT USB proc EP unblock signal, EP unblock now
*/
void SctUsbProcEpUnblock(UINT32 epId);


/******************************************************************************
 * SctUsbCtsBeAllowSend
 * Description: whether CTS(Clear to Send) signal (trigger in MCU side) allow UE
 *              to send data
 * input:   UINT32  epId
 *          BOOL    bAllow  //TRUE - allow to send, FALSE - not allow
 * output:  void
 * Comment:  Called in Tx task
 * 1> if virtual CTS signal is not allow to send data via current USB EP,
 *    current USB EP will be blocked 10 seconds waiting for CTS allow signal,
 *    data will be pended during this time
 * 2> if exceed 10 seconds, the pending data and following data are all discarded.
******************************************************************************/
void SctUsbCtsBeAllowSend(UINT32 epId, BOOL bAllow);



#else   //ENABLE UTFW


/******************************************************************************
 *****************************************************************************
 * MARCO
 *****************************************************************************
******************************************************************************/


/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

typedef enum
{
    SCT_USB_TX_RET_OK   = 0,
    SCT_USB_TX_ERR      = -1,
    SCT_USB_TX_BUSY     = -2,

    UTFW_TX_RET_OK      = SCT_USB_TX_RET_OK,
    UTFW_TX_ERR         = SCT_USB_TX_ERR,
    UTFW_TX_BUSY        = SCT_USB_TX_BUSY
}SctUsbTxRetEnum;

typedef INT32   SctUsbTxRet;
typedef INT32   UtfwTxRet;


/*
 * USB Tx Block
*/
typedef DlPduBlock      SctUsbTxBlock;

typedef SctUsbTxBlock   UtfwTxBlock;


/*
 * SCT USB ISR event
*/
typedef enum
{
    SCT_USB_DEINT   = 0,    /* when USB plug out, or suspended */
    SCT_USB_INIT    = 1,    /* USB is OK (EP enumed), and SCT could sent data to USBC TxFIFO, when USB plug in, or resumed */
    SCT_USB_RESET   = 2,    /* USB is reseted, SCT USB channel should be reset, and wait USB init(OK) again. */

    UTFW_USB_DEINT  = SCT_USB_DEINT,    /* when USB plug out, or suspended */
    UTFW_USB_INIT   = SCT_USB_INIT,     /* USB is OK (EP enumed), and SCT could sent data to USBC TxFIFO, when USB plug in, or resumed */
    UTFW_USB_RESET  = SCT_USB_RESET     /* USB is reseted, SCT USB channel should be reset, and wait USB init(OK) again. */
}SctUsbStateEvent;

typedef SctUsbStateEvent    UtfwUsbStateEvent;

/******************************************************************************
 *****************************************************************************
 * Functions
 *****************************************************************************
******************************************************************************/

/******************************************************************************
 * SctUsbTxRawDataNew
 * Description: SCT USB Tx raw data, without cipher/CRC/PPP operation.
 * input:   UINT32 epId     //USB EP ID < 16
 *          SctUsbTxBlock *pTxHead      //input, Tx block header
 *          SctUsbTxBlock *pTxTail      //input, Tx block header
 *          BOOL          allowConcatenated     //input, whether allow serveral PDU Tx in one USB transfer
 * output:  SctUsbTxRet
 * Comment:
 * 1> As the MAX USB Tx transfer size is limited, Tx block maybe can't transmit one time
 *    then, "pRetHead" will return the next not transmited block header
 * 2> This API return OK, just means block inserted into SCT FIFO, not means already
 *    transmited to USB host, so the caller: !!! can't free the block memory !!!
 *    let the SCT responsible for freeing it after SCT done
 * 3> This API could only be called in USB TX task
 * 4> The caller don't need any handling after called this API
******************************************************************************/
UtfwTxRet   UtfwTxRawData(UINT32 epId, UtfwTxBlock *pTxHead, UtfwTxBlock *pTxTail, BOOL allowConcatenated);
#define SctUsbTxRawDataNew      UtfwTxRawData


/******************************************************************************
 * SctUsbTxPppDataNew
 * Description: SCT USB Tx raw data, data need PPP escaption.
 * input:   UINT32 epId     //USB EP ID < 16
 *          SctUsbTxBlock *pTxHead      //input, Tx block header
 *          SctUsbTxBlock *pTxTail      //input, Tx block header
 * output:  SctUsbTxRet
 * Comment:
 * 1> As the MAX USB Tx transfer size is limited, Tx block maybe can't transmit one time
 *    then, "pRetHead" will return the next not transmited block header
 * 2> This API return OK, just means block inserted into SCT FIFO, not means already
 *    transmited to USB host, so the caller: !!! can't free the block memory !!!
 *    let the SCT responsible for freeing it after SCT done
 * 3> This API could only be called in USB TX task
 * 4> The caller don't need any handling after called this API
******************************************************************************/
UtfwTxRet UtfwTxPppData(UINT32 epId, UtfwTxBlock *pTxHead, UtfwTxBlock *pTxTail);
#define SctUsbTxPppDataNew      UtfwTxPppData


/******************************************************************************
 * SctUsbProcDoneChainInfo
 * Description: SCT USB process chain done info
 * input:   BOOL *pSctAvailable     //
 * output:  void
 * Comment:
 * 1> Called when USB Tx task scheduled
 * 2> if "pSctAvailable" return TRUE, just means SCT USB channel is available again
 *    and the caller could Tx data again
******************************************************************************/
void UtfwProcDoneChainInfo(BOOL *pSctAvailable);
#define SctUsbProcDoneChainInfo UtfwProcDoneChainInfo


/******************************************************************************
 * SctUsbProcUsbEvent
 * Description: SCT USB process USB event notification
 * input:   SctUsbStateEvent usbState
 * output:  void
 * Comment:
 * 1> Called when USB Tx task when USB event signal/message received
******************************************************************************/
void UtfwProcUsbEvent(UtfwUsbStateEvent usbEvent);
#define SctUsbProcUsbEvent      UtfwProcUsbEvent



/******************************************************************************
 * UtfwEpCheckAndUnblock
 * Description: If "txEpId" is blocked, send signal to Tx task to unblock this EP
 * input:   UINT32 txEpId
 * output:  void
 * Comment:  Called in Rx task, when an EP recv data from host, just means EP is OK for Tx
******************************************************************************/
void UtfwEpCheckAndUnblock(UINT32 txEpId);
#define SctUsbEpCheckAndUnblock     UtfwEpCheckAndUnblock

/******************************************************************************
 * UtfwProcEpUnblock
 * Description: UTFW USB proc EP unblock signal
 * input:   UINT32 epId
 * output:  void
 * Comment:  Called in CCIO Tx task
******************************************************************************/
void UtfwProcEpUnblock(UINT32 epId);
#define SctUsbProcEpUnblock         UtfwProcEpUnblock

/******************************************************************************
 * UtfwCtsBeAllowSend
 * Description: whether CTS(Clear to Send) signal (trigger in MCU side) allow UE
 *              to send data
 * input:   UINT32  epId
 *          BOOL    bAllow  //TRUE - allow to send, FALSE - not allow
 * output:  void
 * Comment:  Called in Tx task
 * 1> if virtual CTS signal is not allow to send data via current USB EP,
 *    current USB EP will be blocked 10 seconds waiting for CTS allow signal,
 *    data will be pended during this time
 * 2> if exceed 10 seconds, the pending data and following data are all discarded.
******************************************************************************/
void UtfwCtsBeAllowSend(UINT32 epId, BOOL bAllow);
#define SctUsbCtsBeAllowSend    UtfwCtsBeAllowSend


/******************************************************************************
 * UtfwProcTimerExpiry
 * Description: UTFW USB retry timer expiry handling
 * input:   UINT32 tid
 * output:  void
 * Comment:  Called in CCIO Tx task
******************************************************************************/
void UtfwProcTimerExpiry(UINT32 tid);
#define SctUsbProcTimerExpiry   UtfwProcTimerExpiry


/******************************************************************************
 * UtfwStateEventInIsr
 * Description: USB state, called in ISR
 * input:   UINT32 usbState
 * output:  void
 * Comment:
******************************************************************************/
void UtfwUsbStateEventInIsr(UtfwUsbStateEvent usbState);
#define SctUsbStateEventInIsr   UtfwUsbStateEventInIsr

/******************************************************************************
 * UtfwBeDuringPppTx
 * Description: whether UTFW PPP Tx is ongoing
 * input:   void
 * output:  void
******************************************************************************/
BOOL UtfwBeDuringPppTx(void);
#define SctUsbBeDuringPppTx     UtfwBeDuringPppTx

#endif

#endif

