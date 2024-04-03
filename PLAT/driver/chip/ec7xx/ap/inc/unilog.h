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

#ifndef _UNILOG_API_H
#define _UNILOG_API_H
/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdbool.h>

#ifdef __cplusplus
 extern "C" {
#endif

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define UNILOG_TRACE_MAX_MODULE_VALUE         (1024)

#define UNILOG_DMA_REQ_MODE_USB               (0x3)
#define UNILOG_DMA_REQ_MODE_UART              (0x0)

#define UNILOG_ID_CONSTRUCT(ownerId, moduleId, subId)    ((ownerId << 28) | (moduleId << 21) | (subId << 11))
#define UNILOG_GET_OWNERID(id)                ((id) >> 28)

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef enum
{
    UART_0_FOR_UNILOG = 0,
    UART_1_FOR_UNILOG = 1,
    UART_2_FOR_UNILOG = 2,
    UART_3_FOR_UNILOG = 3,
//    SPI_0_FOR_UNILOG  = 4,
//    SPI_1_FOR_UNILOG  = 5,
    USB_FOR_UNILOG    = 6,
    RAM_FOR_UNILOG    = 7
} UnilogPeripheralType_e;

typedef struct
{
    uint32_t addr;
    uint32_t len;
} UnilogRamLogBuffer_t;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
void uniLogModuleAllowTraces(uint16_t moduleID);
void uniLogModuleDisableTraces(uint16_t moduleID);
void uniLogModuleAllowAllTraces(void);
void uniLogModuleDisableAllTraces(void);
bool uniLogTraceAllowCheck(uint32_t swLogID, uint8_t debugLevel);
void uniLogFlushOut(void);
void uniLogForceOut(bool waitFifoOut);
void uniLogForceOutWFI(bool waitFifoOut);
void uniLogStop(void);
void uniLogStartHwLog(void);
void uniLogStopHwLog(void);
bool uniLogIsInitialized(void);
void uniLogGetSettingFromFlash(void);
void swLogExcep(uint32_t swLogID, uint8_t debugLevel, ...);
void swLogPrintf(uint32_t swLogID, uint8_t debugLevel, ...);
void swLogDump(uint32_t swLogID, uint8_t debugLevel, uint32_t dumpLen, const uint8_t*pDump);
void swLogPrintfPolling(uint32_t swLogID, uint8_t debugLevel, ...);
void swLogDumpPolling(uint32_t swLogID, uint8_t debugLevel, uint32_t dumpLen, const uint8_t*pDump);
void uniLogSetPherType(UnilogPeripheralType_e periphType);
UnilogPeripheralType_e uniLogGetPherType(void);
bool unilogSwitch2UartLog(void);
bool unilogSwitch2UsbLog(void);
bool uniLogGetDumpRdyFlag(void);
bool unilogPollDMADeactive(void);
void unilogSetDMAReactive(void);
void uniLogSetBusy(bool isBusy);
uint32_t uniLogReadWaterMark(void);

/*
RamLogBuffer layout:
    +-----------+-------------------+-------------------+---------------+-------------------+-------------------------------------+
    |  metaDada |  DMA desc 0(16B)  |  DMA desc 1(16B)  |      ...      |  DMA desc N(16B)  |                 Log                 |
    +-----------+-------------------+-------------------+---------------+-------------------+-------------------------------------+
    ^           ^                                                                           ^                                     ^
    |           |                                                                           |                                     |
    |     aligned start(16B)                                                    ram log ring buffer start                         |
    |<--------------------------------------------------------  bufferLen  ------------------------------------------------------>|
bufferStart
*/
int32_t unilogSetRamLog(void);
void unilogSetRamLogAppendFlag(void);
uint32_t unilogGetRamLogStartAddr(void);
uint32_t unilogGetRamLogStopAddr(void);

void swLogInternalPrintf(uint32_t swLogID, uint8_t debugLevel, ...);
void swLogInternalDump(uint32_t swLogID, uint8_t debugLevel, uint32_t dumpLen, const uint8_t*pDump);
void swLogInternalDumpPolling(uint32_t swLogID, uint8_t debugLevel, uint32_t dumpLen, const uint8_t*pDump);

#ifdef __cplusplus
}
#endif

#endif//_UNILOG_API_H

