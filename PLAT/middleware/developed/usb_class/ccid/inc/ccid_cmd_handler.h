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
#ifndef __CCID_CMD_HANDLER_H
#define __CCID_CMD_HANDLER_H


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


/*support T0/1 is detemined by user, maybe after PPS*/
#define SUPPORT_PROTOCOL_T1

#define CCID_SET_SLOT_STATUS(cmd_status,icc_status)  gCcidCtrlBlk.gCcidBulkinData.bStatus=(cmd_status|icc_status)


//ccid spec 6.2.6  slot err register table 6.2-2

/*only defined when bmCommandStatus = 1 for bmCommandStatus = 0(no error)
  slot error register may have some user defined value,
  this value should be defiend per specific usage fully detemined by user*/

#define   SLOT_NO_ERROR         0x81
#define   SLOT_NO_ERROR2        0x00

/* part 1: ccid spec 6.2.6 table 6.2-2 when bmCommandStatus = 1*/
#define   SLOTERROR_CMD_ABORTED                    0xFF
#define   SLOTERROR_ICC_MUTE                       0xFE
#define   SLOTERROR_XFR_PARITY_ERROR               0xFD
#define   SLOTERROR_XFR_OVERRUN                    0xFC
#define   SLOTERROR_HW_ERROR                       0xFB
#define   SLOTERROR_BAD_ATR_TS                     0xF8
#define   SLOTERROR_BAD_ATR_TCK                    0xF7
#define   SLOTERROR_ICC_PROTOCOL_NOT_SUPPORTED     0xF6
#define   SLOTERROR_ICC_CLASS_NOT_SUPPORTED        0xF5
#define   SLOTERROR_PROCEDURE_BYTE_CONFLICT        0xF4
#define   SLOTERROR_DEACTIVATED_PROTOCOL           0xF3
#define   SLOTERROR_BUSY_WITH_AUTO_SEQUENCE        0xF2
#define   SLOTERROR_PIN_TIMEOUT                    0xF0
#define   SLOTERROR_PIN_CANCELLED                  0xEF
#define   SLOTERROR_CMD_SLOT_BUSY                  0xE0

/* part 2: C0h----81h :   User Defined,  add by user*/

/* part 3: 80h and those filling the gaps reserved for future use*/

/* part 4: 7Fh to 01h: Index of not supported / incorrect message parameter

Failure of a command
If CCID cannot parse one parameter or the ICC is not supporting one parameter.
Then the Slot Error register contains the index of the first bad parameter as a
positive number (1-127). e.g. if the CCID receives an ICC command to
an unimplemented slot, then the Slot Error register shall be set to 5 (index of bSlot field).

*/

#define   SLOTERROR_BAD_LENTGH                    0x01
#define   SLOTERROR_BAD_SLOT                      0x05
#define   SLOTERROR_BAD_POWERSELECT               0x07
#define   SLOTERROR_BAD_PROTOCOLNUM               0x07
#define   SLOTERROR_BAD_CLOCKCOMMAND              0x07
#define   SLOTERROR_BAD_ABRFU_3B                  0x07
#define   SLOTERROR_BAD_BMCHANGES                 0x07
#define   SLOTERROR_BAD_BFUNCTION_MECHANICAL      0x07
#define   SLOTERROR_BAD_ABRFU_2B                  0x08
#define   SLOTERROR_BAD_LEVELPARAMETER            0x08
#define   SLOTERROR_BAD_FIDI                      0x0A
#define   SLOTERROR_BAD_T01CONVCHECKSUM           0x0B
#define   SLOTERROR_BAD_GUARDTIME                 0x0C
#define   SLOTERROR_BAD_WAITINGINTEGER            0x0D
#define   SLOTERROR_BAD_CLOCKSTOP                 0x0E
#define   SLOTERROR_BAD_IFSC                      0x0F
#define   SLOTERROR_BAD_NAD                       0x10
#define   SLOTERROR_BAD_DWLENGTH                  0x08


/* part 5: only one  */

#define   SLOTERROR_CMD_NOT_SUPPORTED              0x00


//default para value for T0/1
#define   DEFAULT_FIDI              0x11
#define   DEFAULT_T01CONVCHECKSUM   0x10
#define   DEFAULT_EXTRA_GUARDTIME   0x00
#define   DEFAULT_WAITINGINTEGER    0x41
#define   DEFAULT_CLOCKSTOP         0x00
#define   DEFAULT_IFSC              0x20
#define   DEFAULT_NAD               0x00



//ccid spec 6.2.6  slot status register table 6.2-3

/*
bit[1:0] bmICCStatus 2 bit  0, 1, 2
    0 - An ICC is present and active (power is on and stable, RST is inactive)
    1 - An ICC is present and inactive (not activated or shut down by hardware error)
    2 - No ICC is present
    3 - RFU
bit[5:2] bmRFU 4 bits 0 RFU
bit[7:6] bmCommandStatus 2 bits 0, 1, 2
    0 - Processed without error
    1 - Failed (error code provided by the error register)
    2 - Time Extension is requested
    3 - RFU
*/

#define BM_ICC_PRESENT_ACTIVE        0x00
#define BM_ICC_PRESENT_INACTIVE      0x01
#define BM_ICC_NO_ICC_PRESENT        0x02

#define BM_COMMAND_STATUS_POS        0x06
#define BM_COMMAND_STATUS_NO_ERROR   (0x00 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_FAILED     (0x01 << BM_COMMAND_STATUS_POS)
#define BM_COMMAND_STATUS_TIME_EXTN  (0x02 << BM_COMMAND_STATUS_POS)

/* defines for the CCID_CMD Layers */

#define LEN_RDR_TO_PC_SLOTSTATUS 10
#define LEN_PROTOCOL_STRUCT_T0   5
#define LEN_PROTOCOL_STRUCT_T1   7


#define BPROTOCOL_NUM_T0  0
#define BPROTOCOL_NUM_T1  1


//wLevelParameter when exchange level is extended apdu
#define WLVL_PARA_BEG_AND_END_CMD                         0x0000
#define WLVL_PARA_BEG_AND_CONT_CMD                        0x0001
#define WLVL_PARA_ABDATA_CONT_AND_END_CMD                 0x0002
#define WLVL_PARA_ABDATA_CONT_AND_ANOTBLKFL_CMD           0x0003
#define WLVL_PARA_EMPTY                                   0x0010



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef enum
{
  CHK_PARAM_SLOT = 1,
  CHK_PARAM_DWLENGTH = (1<<1),
  CHK_PARAM_abRFU2 = (1<<2),
  CHK_PARAM_abRFU3 = (1<<3),
} CmdParamCheck_e;



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/
uint8_t  pcToRDRIccPowerOn(void);
uint8_t  pcToRDRIccPowerOff(void);
uint8_t  pcToRDRGetSlotStatus(void);
uint8_t  pcToRDRXfrBlock(void);
uint8_t  pcToRDRGetParameters(void);
uint8_t  pcToRDRResetParameters(void);
uint8_t  pcToRDRSetParameters(void);
uint8_t  pcToRDREscape(void);
uint8_t  pcToRDRIccClock(void);
uint8_t  pcToRDRAbort(void);
uint8_t  pcToRDRT0Apdu(void);
uint8_t  pcToRDRMechanical(void);
uint8_t  pcToRDRSetDataRateAndClockFrequency(void);
uint8_t  pcToRDRSecure(void);

void rdrToPcDataBlock(unsigned char );
void rdrToPcNotifySlotChange(void);
void rdrToPcSlotStatus(unsigned char );
void rdrToPcParameters(unsigned char );
void rdrToPcEscape(unsigned char );
void rdrToPcDataRateAndClockFrequency(uint8_t  errorCode);
void rdrToPcWaitTimeExtend(uint8_t multi);




#ifdef __cplusplus
}
#endif

#endif


