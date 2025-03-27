/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: scom_inst.h
*
*  Description:
*
*  History: 2025/1/19 created by xuwang
*
*  Notes: simulated COM port for I/O
*
******************************************************************************/
#ifndef SCOM_INST_H
#define SCOM_INST_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stddef.h>
#include "scom_event.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define SCOM_INST_MAXNUM            SCOM_EVID_RX_MAXNUM

// flags definition for struct 'ScomInst_t'
#define SCOM_INST_FLAG_INITIALIZED  (1U << 0)     // initialized
#define SCOM_INST_FLAG_TX_ENABLED   (1U << 1)     // TX enabled
#define SCOM_INST_FLAG_RX_ENABLED   (1U << 2)     // RX enabled


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef enum
{
    SCOM_IO_RX = 0,
    SCOM_IO_TX,

    SCOM_IO_MAXNUM
}ScomIo_e;

typedef enum
{
    SCOM_ROLE_SLAVE = 0,
    SCOM_ROLE_MASTER,

    SCOM_ROLE_MAXNUM
}ScomRole_e;

typedef struct
{
    uint32_t  isInit   :1;     /* rbuf is initialized or not */
    uint32_t  rsvdBits :31;
    uint32_t  xferCnt;
    uint32_t  readIdx;
    uint32_t  writeIdx;
    uint32_t  bufLen;
    uint8_t  *buf;
}ScomXfer_t;

typedef int32_t (*xferInCallback)(uint32_t scomIdx, uint32_t xferCnt, void *extras);

typedef struct
{
    uint32_t        flags;
    uint8_t         role;   /* ScomRole_e */
    uint8_t         rsvd[3];
    ScomXfer_t      xferIn;
    ScomXfer_t     *xferOut;
    xferInCallback  xferInCb;

    /* more privacy info */
    void           *extras;
}ScomInst_t;

/* flags definition for funciton 'int32_t  (*ctrl) (uint32_t scomIdx, uint32_t flags, void *args)' */
typedef enum _EPAT_ScomInstCtrlFlagsTag
{
    SCOM_ICF_BEGIN = 0x11CF0000,
    SCOM_ICF_INIT_RX_CONF,
    SCOM_ICF_GET_RX_XFER,
    SCOM_ICF_UPD_RX_XFER,

    SCOM_ICF_END
}ScomInstCtrlFlags_e;

/* SCOM_ICF_INIT_RX_CONF */
typedef struct
{
    uint8_t  *buf;
    uint32_t  bufLen;
}ScomIcaInitRxConf_t;

/* SCOM_ICF_GET_RX_XFER */
typedef struct
{
    uint32_t  readIdx;
    uint32_t  xferCnt;
}ScomIcaGetRxXfer_t;

/* SCOM_ICF_UPD_RX_XFER */
typedef struct
{
    uint32_t  readIdx;
}ScomIcaUpdRxXfer_t;


/**
\brief Access structure of the SCOM Driver.
*/
typedef struct
{
    /* sizeof(ScomInstDrv_t) */
    uint32_t   size;

    int32_t  (*init)(uint32_t scomIdx, xferInCallback inputCb, void *extras);
    int32_t  (*exit)(uint32_t scomIdx);
    int32_t  (*send)(uint32_t scomIdx, const uint8_t *data, uint32_t nbytes);
    int32_t  (*ctrl)(uint32_t scomIdx, uint32_t flags, void *args);
}ScomInstDrv_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
const ScomInstDrv_t* scomGetInstDrv(ScomRole_e role);

#ifdef __cplusplus
}
#endif
#endif

