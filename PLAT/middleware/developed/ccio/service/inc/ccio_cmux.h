/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: ccio_cmux.h
*
*  Description:
*
*  History: 2023/1/31 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CCIO_CMUX_H
#define CCIO_CMUX_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
// the errno of the frames
typedef enum _EPAT_CmuxFrameErrNo
{
    CMUX_FEN_EOK    = 0x0,     /* no error */
    CMUX_FEN_ERR    = 0x1,     /* general error */
    CMUX_FEN_EFCS   = 0x2,     /* bad fcs */
    CMUX_FEN_EEF    = 0x3,     /* incorrect end flag */

    CMUX_FEN_MAXNO  = 0xFF
}CmuxFrameErrNo_e;

#define CMUX_FRAME_HDR_MINSIZE  4
#define CMUX_FRAME_HDR_MAXSIZE  5
#define CMUX_FRAME_TAIL_SIZE    2

#define CMUX_INIT_FCS     0xff  /* Initial FCS value */
#define CMUX_GOOD_FCS     0xcf  /* Good final FCS value */

#define CMUX_CR   2
#define CMUX_EA   1

// basic mode flag for frame start and end
#define CMUX_BASIC_FLAG   0xF9

// bits: Poll/final, Command/Response, Extension
#define CMUX_CONTROL_PF   16
#define CMUX_ADDRESS_CR   CMUX_CR
#define CMUX_ADDRESS_EA   CMUX_EA

// the types of the frames
typedef enum _EPAT_CmuxFrameType
{
    CMUX_FRAME_SABM = 0x2F,
    CMUX_FRAME_UA   = 0x63,
    CMUX_FRAME_DM   = 0xF,
    CMUX_FRAME_DISC = 0x43,
    CMUX_FRAME_UIH  = 0xEF,
    CMUX_FRAME_UI   = 0x3
}CmuxFrameType_e;

#define CMUX_DLCI_MASK       63         /* DLCI number is port number, 63 is the mask of DLCI; C/R bit is 1 when we send data */
#define CMUX_LO_LEN_MASK     127        /* when data length is out of 127( 0111 1111 ), we must use two bytes to describe data length in the cmux frame */
#define CMUX_HI_LEN_MASK     32640      /* 32640 (‭ 0111 1111 1000 0000 ‬), the mask of high data bits */

#define CMUX_CMD_IS(cmd, type)       ((type & ~CMUX_ADDRESS_CR) == cmd)
#define CMUX_FRAME_IS(type, frame)   ((frame->control & ~CMUX_CONTROL_PF) == type)
#define CMUX_PF_ISSET(frame)         ((frame->control & CMUX_CONTROL_PF) == CMUX_CONTROL_PF)

//====================================================================================================
// Cmux Control Channel(dlc0) Message
//====================================================================================================
#define CMUX_CCC_MSG_MAXSIZE  CCIO_BUF_SIZE_64

// bits: Command/Response, Extension in type/length field
#define CMUX_CCC_EA      CMUX_EA   /* type or length field */
#define CMUX_CCC_CR      CMUX_CR   /* type field */

// the types of the Control Channel Commands
typedef enum _EPAT_CmuxCtrlChanCmd
{
    CMUX_CCC_NSC   = 0x11,      // non supported command response
    CMUX_CCC_TEST  = 0x21,      // test
    CMUX_CCC_PSC   = 0x41,      // power saving control
    CMUX_CCC_RLS   = 0x51,      // remote line status command
    CMUX_CCC_FCOFF = 0x61,      // flow control off command
    CMUX_CCC_FCON  = 0xA1,      // flow control on command
    CMUX_CCC_PN    = 0x81,      // DLC parameter negotiation
    CMUX_CCC_RPN   = 0x91,      // remote port negocition command
    CMUX_CCC_CLD   = 0xC1,      // multiplexer close down
    CMUX_CCC_SNC   = 0xD1,      // service negociation command
    CMUX_CCC_MSC   = 0xE1,      // modem status command
}CmuxCtrlChanCmd_e;

/* PN message */
#define CMUX_CCC_PN_MSG_SIZE        8

typedef enum _EPAT_CmuxCccPnFrameType
{
    CMUX_PN_FRAME_UIH = 0,
    CMUX_PN_FRAME_UI  = 1,
    CMUX_PN_FRAME_I   = 2,
}CmuxCccPnFrameType_e;

typedef enum _EPAT_CmuxCccPnConvType
{
    CMUX_PN_CONV_TYPE_1 = 0,
    CMUX_PN_CONV_TYPE_2 = 1,
    CMUX_PN_CONV_TYPE_3 = 2,
    CMUX_PN_CONV_TYPE_4 = 3,
}CmuxCccPnConvType_e;

typedef struct
{
    uint8_t  dlci;
#if 1 /* little endian */
    uint8_t  cl :4;
    uint8_t  i  :4;
#else
    uint8_t  i  i4;
    uint8_t  cl :4;
#endif
    uint8_t  prior;
    uint8_t  t1;
    uint8_t  n1[2];
    uint8_t  n2;
    uint8_t  k;
}CmuxCccPnMsg_t;

/* SNC message */
#define CMUX_CCC_SNC_MSG_SIZE       3
#define CMUX_CCC_SNC_DATA           0x02
#define CMUX_CCC_SNC_VOICE          0x04

/* MSC message */
#define CMUX_CCC_MSC_MSG_SIZE       2
#define CMUX_CCC_MSC_BIT_FC         0x02
#define CMUX_CCC_MSC_BIT_DSR        0x04
#define CMUX_CCC_MSC_BIT_CTS        0x08
#define CMUX_CCC_MSC_BIT_RI         0x40
#define CMUX_CCC_MSC_BIT_DCD        0x80
#define CMUX_CCC_MSC_MS_STATUS     (CMUX_CCC_MSC_BIT_DSR | CMUX_CCC_MSC_BIT_CTS | CMUX_CCC_MSC_BIT_DCD)

#define CMUX_CCC_TYPE_IS(cmd, type)  ((type & ~CMUX_CCC_CR) == cmd)


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

/*
 * | ----------- | ------- | ------- | ---------- | ----------- | ------ | ----------- |
 * |    Flag     | Address | Control |   Length   | Information |  FCS   |    Flag     |
 * | ----------- | ------- | ------- | :--------: | ----------- | ------ | ----------- |
 * | 0xF9(basic) | 1 Byte  | 1 Bytes |  1/2 Bytes |  >= 1 Byte  | 1 Byte | 0xF9(basic) |
 * | ----------- | ------- | ------- | ---------- | ----------- | ------ | ----------- |
 */

typedef struct __PACKED
{
    uint8_t   flag;
    uint8_t   address;      /* the frame channel */
    uint8_t   control;      /* the type of frame */
    union
    {
        uint8_t   a;
        uint8_t   aa[2];
    }length;
}CmuxFrameHdr_t;

typedef struct __PACKED
{
    uint8_t   fcs;       /* the frame channel */
    uint8_t   flag;      /* the type of frame */
}CmuxFrameTail_t;

typedef struct __PACKED
{
    uint8_t   type;      /* the type of ctrl channel msg */
    uint8_t   length;    /* the length of ctrl channel msg */
    uint8_t   data[CMUX_CCC_MSG_MAXSIZE];
}CmuxDlc0Msg_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief cmuxFrameDecap(UlPduBlockList_t *list, CmuxFrameDesc_t *desc);
 * @details extract a complete cmux frame from ulpdu list every time and no frame format any more
 *
 * @param list    The received uplink octect stream
 * @param desc    description of the cmux frame
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cmuxFrameDecap(UlPduBlockList_t *list, CmuxFrameDesc_t *desc);

/**
 * @brief cmuxFrameEncap(DlPduBlock_t **dlpdu, uint8_t initiator, uint8_t dlci, uint8_t type);
 * @details set cmux frame format for output data
 *
 * @param dlpdu      the output data to be set format and the remaining data to be set next time for memory lack if *dlpdu != NULL
 * @param initiator  the device is an initiator side or not?
 * @param dlci       index of cmux dlc
 * @param type       frame type in frame format
 * @return !NULL the output data with cmux format; NULL failure with errno.
 */
DlPduBlock_t* cmuxFrameEncap(DlPduBlock_t **dlpdu, uint8_t initiator, uint8_t dlci, uint8_t type);


#ifdef __cplusplus
}
#endif
#endif

