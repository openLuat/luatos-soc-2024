/*
 * Copyright (c) 2025 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#ifndef __CORE_CAN_H__
#define __CORE_CAN_H__
#include "bsp_common.h"
enum
{
	CAN_NEW_MSG,
	CAN_TX_OK,
	CAN_TX_FAILED,
	CAN_ERROR_REPORT,
	CAN_STATE_CHANGE,


	CAN_STATE_POWER_OFF = 0,
	CAN_STATE_INITED,
	CAN_STATE_IN_RESET_MODE,
	CAN_STATE_NODE_ACTIVE_ERROR,
	CAN_STATE_NODE_PASSIVE_ERROR,
	CAN_STATE_NODE_BUS_OFF,
	CAN_STATE_ONLY_LISTEN,
	CAN_STATE_SELF_TEST,
	CAN_STATE_SLEEP,

	CAN_WORK_MODE_NORMAL = 0,
	CAN_WORK_MODE_ONLY_LISTEN,
	CAN_WORK_MODE_SELF_TEST,
	CAN_WORK_MODE_SLEEP,

	CAN_ERROR_TYPE_BIT = 0,
	CAN_ERROR_TYPE_FORM,
	CAN_ERROR_TYPE_STUFF,
	CAN_ERROR_TYPE_OTHER,

	CAN_ERROR_LOC_SOF = 0x03,		/* start of frame */
	CAN_ERROR_LOC_ID28_21  = 0x02,	/* extended: ID bits 28 - 21, standard:  10 - 3 */
	CAN_ERROR_LOC_ID20_18 = 0x06,	/* extended: ID bits 20 - 18, standard:  2 - 0 */
	CAN_ERROR_LOC_SRTR = 0x04,		/* extended: substitute RTR, standard: RTR */
	CAN_ERROR_LOC_IDE = 0x05,		/* identifier extension */
	CAN_ERROR_LOC_ID17_13 = 0x07,		/* extended: ID bits 17 - 13 */
	CAN_ERROR_LOC_ID12_5  = 0x0f,	/* extended: ID bits 12 - 5 */
	CAN_ERROR_LOC_ID4_0 = 0x0e,		/* extended: ID bits 4 - 0 */
	CAN_ERROR_LOC_RTR = 0x0C,		/* RTR */
	CAN_ERROR_LOC_RES1 = 0x0D,		/* reserved bit 1 */
	CAN_ERROR_LOC_RES0  = 0x09,		/* reserved bit 0 */
	CAN_ERROR_LOC_DLC  = 0x0b,	/* data length code */
	CAN_ERROR_LOC_DATA = 0x0a,		/* data section */
	CAN_ERROR_LOC_CRC_SEQ = 0x08,		/* CRC sequence */
	CAN_ERROR_LOC_CRC_DEL = 0x18,		/* CRC delimiter */
	CAN_ERROR_LOC_ACK  = 0x19,		/* ACK slot */
	CAN_ERROR_LOC_ACK_DEL   = 0x1b,	/* ACK delimiter */
	CAN_ERROR_LOC_EOF = 0x1a,		/* end of frame */
	CAN_ERROR_LOC_INTERM = 0x12,		/* intermission */
	CAN_ERROR_LOC_UNSPEC = 0x00,		/* unspecified */
};

typedef struct
{
	uint32_t ID;
	uint8_t RTR:1;
	uint8_t ExtendID:1;
	uint8_t OneShot:1;
	uint8_t Pad:1;
	uint8_t Len:4;
	uint8_t Data[8];
}CAN_MsgStruct;

typedef struct
{
	uint8_t ErrorType;
	uint8_t ErrorDir;
	uint8_t ErrorLOC;
	uint8_t ALCode;
}CAN_ErrorMsg;

int CAN_BaseInit(uint32_t RxMsgCacheNums, CBFuncEx_t CB);
int CAN_SetBitRate(uint32_t BitRate, uint8_t PTS, uint8_t PBS1, uint8_t PBS2, uint8_t SJW);
int CAN_SetFilter(uint8_t IsDual, uint8_t ACR[4], uint8_t AMR[4]);
int CAN_SetWorkMode(uint8_t mode);
uint8_t CAN_GetState(void);
void CAN_DeInit(void);

int CAN_TxMsg(CAN_MsgStruct *Msg);
void CAN_TxStop(void);
int CAN_RxMsg(CAN_MsgStruct *Msg);

int CAN_EnterResetMode(uint32_t timeout);
int CAN_ExitResetMode(uint32_t timeout);

void CAN_PrintReg(void);
void CAN_SetDebug(uint8_t OnOff);
#endif
