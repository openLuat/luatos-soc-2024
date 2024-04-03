/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: cmux_entity.h
*
*  Description:
*
*  History: 2023/11/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef CMUX_ENTITY_H
#define CMUX_ENTITY_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define CMUX_DLC_ULTI_MAXNUM       64

// parameter negociation
//-----------------------

// time out value (ms)
#define CMUX_DLC_T1   100
#define CMUX_DLC_T2   300
#define CMUX_DLC_T3   10000

#define CMUX_DLC_N2   3
#define CMUX_DLC_K    2


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
/* Dlc Entity Ctrl Map, maxDlci = 63 */
#define CMUX_DECM_NBYTES      (CMUX_DLC_ULTI_MAXNUM / 8)
typedef uint8_t CmuxDecm_bm[CMUX_DECM_NBYTES];

typedef enum
{
    CMUX_DLC_PARAM_MODE = 0,
    CMUX_DLC_PARAM_SUBSET,
    CMUX_DLC_PARAM_SPEED,
    CMUX_DLC_PARAM_N1,
    CMUX_DLC_PARAM_T1,
    CMUX_DLC_PARAM_N2,
    CMUX_DLC_PARAM_T2,
    CMUX_DLC_PARAM_T3,
    CMUX_DLC_PARAM_K
}CmuxDlcParamId_e;

typedef enum
{
    CMUX_MODE_BASIC = 0,
    CMUX_MODE_ADVANCED
}CmuxWorkMode_e;

typedef enum
{
    CMUX_FRAME_UIH_ONLY = 0,
    CMUX_FRAME_UI_ONLY,
    CMUX_FRAME_I_ONLY
}CmuxFrameSubset_e;

typedef enum
{
    CMUX_DLC_DISCONNECTED = 0,
    CMUX_DLC_CONNECTING,
    CMUX_DLC_CONNECTED,
    CMUX_DLC_DISCONNECTING,
    CMUX_DLC_FLOW_STOPPED
}CmuxDlcFsm_e;

typedef enum
{
    CMUX_DLC_FC_OFF = 0,
    CMUX_DLC_FC_ON
}CmuxDlcFcState_e;

typedef enum _EPAT_CmuxDlcXctlFlags
{
    CMUX_DXF_CTRL_BEGIN = 0xDF0000,
    CMUX_DXF_CTRL_END = 0xDF0FFF,

    CMUX_DXF_DATA_BEGIN = 0xDF1000,
    CMUX_DXF_OPEN_DLC,
    CMUX_DXF_CLOSE_DLC,
    CMUX_DXF_RECV_UA,
    CMUX_DXF_RECV_DM,
    CMUX_DXF_DATA_END = 0xDF1FFF,

    CMUX_DXF_UNDEF = 0xDFFFFF,
    CMUX_DXF_END = CMUX_DXF_UNDEF
}CmuxDlcXctlFlags_e;

/* CMUX_DXF_OPEN_DLC */
typedef struct
{
    uint8_t  rsvd[4];
}CmuxDxaOpenDlc_t;

/* CMUX_DXF_CLOSE_DLC */
typedef struct
{
    uint8_t  rsvd[4];
}CmuxDxaCloseDlc_t;

/* CMUX_DXF_RECV_UA */
typedef struct
{
    uint8_t  rsvd[4];
}CmuxDxaRecvUa_t;

/* CMUX_DXF_RECV_DM */
typedef struct
{
    uint8_t  rsvd[4];
}CmuxDxaRecvDm_t;

/* prototype definition */
struct CmuxDlcEntity;

typedef int32_t (*cmuxDlcInitFunc)(struct CmuxDlcEntity *dlcEnt);
typedef int32_t (*cmuxDlcDeinitFunc)(struct CmuxDlcEntity *dlcEnt);
typedef int32_t (*cmuxDlcInputFunc)(struct CmuxDlcEntity *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
typedef int32_t (*cmuxDlcOutputFunc)(struct CmuxDlcEntity *dlcEnt, DlPduBlockList_t *list, void *extras);
typedef int32_t (*cmuxDlcCtlFunc)(struct CmuxDlcEntity *dlcEnt, uint32_t flags, void *args);

typedef struct
{
    cmuxDlcInitFunc    dlcInitFn;
    cmuxDlcDeinitFunc  dlcDeinitFn;
    cmuxDlcInputFunc   dlcInputFn;
    cmuxDlcOutputFunc  dlcOutputFn;
    cmuxDlcCtlFunc     dlcCtlFn;
}CmuxDlcFuncs_t;

typedef struct CmuxDlcEntity
{
    uint8_t   id;         /* dlci */
    uint8_t   serv;       /* 'CmuxDlcType_e' */
    uint8_t   prior;      /* priority for this dlci */
    uint8_t   fsm;        /* CmuxDlcFsm_e */

    uint16_t  n1;         /* mtu */
    uint8_t   fcState;    /* is flow ctrl enabled or not? refer to 'CmuxDlcFcState_e'*/
    uint8_t   rsvd;

    uint16_t  winSize;
    uint16_t  winThres;

    CmuxDlcFuncs_t  *dlcFns;

    void      *extras;    /* details about this entity, point to XxxosEntity_t */
}CmuxDlcEntity_t;

typedef struct CmuxDlcCtxMan
{
    uint8_t           initiator;   /* initiator:1, responser: 0 */
    uint8_t           dlcNum;
    uint8_t           rsvd[2];
    CmuxDlcEntity_t   dlcEnt[CCIO_CMUX_DLC_REAL_CNT];
    CmuxDecm_bm       bmOnline;    /* connected state */

    osTimerId_t       dlc0Timer;

    CmuxFrameDesc_t   frmDesc;
}CmuxDlcCtxMan_t;


typedef enum
{
    CMUX_DLC_ANY = 0,
    CMUX_DLC_CTRL = CMUX_DLC_ANY,
    CMUX_DLC_DATA = 1,
    CMUX_DLC_AT = CMUX_DLC_DATA,
    CMUX_DLC_PPP,
    CMUX_DLC_OPAQ,
    CMUX_DLC_ETH,
    CMUX_DLC_AUDIO,

    CMUX_DLC_MAXTYPE
}CmuxDlcType_e;

typedef struct
{
    uint8_t   mode;     /* refer to 'CmuxWorkMode_e' */
    uint8_t   subset;   /* refer to 'CmuxFrameSubset_e' */
    uint8_t   rsvd0[2];

    uint32_t  hasN1:1;
    uint32_t  rsvd :7;
    uint32_t  n1   :16; /* mtu (maximum frame size). */
    uint32_t  n2   :8;  /* max times of re-transmissions */

    uint16_t  t1;       /* ack timer in units of 1 msec */
    uint16_t  t2;       /* resp timer for the multiplexer ctrl channel in units of 1 msec, t2 must be longer than t1 */

    uint16_t  t3;       /* wake up response timer in units of 1 msec */
    uint16_t  winSize;  /* window size(k), for Advanced operation with Error Recovery options(not used yet) */
}CmuxDlcComnConf_t;

typedef struct
{
    uint8_t   enable :1;
    uint8_t   serv   :7;   /* 'CmuxDlcType_e' */
    uint8_t   prior;
    uint16_t  mtu;
    char     *alias;
}CmuxDlcCustConf_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
/**
 * @brief cmuxSetDlcComnConf(uint32_t flags, uint8_t subset, uint16_t n1, uint16_t t1, uint8_t n2, uint16_t t2, uint16_t t3, uint16_t k)
 * @details set cmux common config parameters
 *
 * @param flags   the present flag for the following parameters
 * @param subset  frame type to be used,refer to 'CmuxFrameSubset_e'
 * @param n1      maximum frame size(mtu)
 * @param t1      ack timer in units of 1 msec
 * @param n2      max times of re-transmissions
 * @param t2      resp timer for the multiplexer ctrl channel in units of 1 msec, t2 must be longer than t1
 * @param t3      wake up response timer in units of 1 msec
 * @param k       window size, for Advanced operation with Error Recovery options
 * @return 0 succ; < 0 failure with errno.
 */
void cmuxSetDlcComnConf(uint32_t flags, uint8_t subset, uint16_t n1, uint16_t t1, uint8_t n2, uint16_t t2, uint16_t t3, uint16_t k);

/**
 * @brief cmuxGetDlcComnConf(void)
 * @details get cmux common config parameters
 *
 * @return !NULL succ; NULL failure.
 */
CmuxDlcComnConf_t* cmuxGetDlcComnConf(void);

/**
 * @brief cmuxStartService(SerialEntity_t *serlEnt)
 * @details set serial port in cmux mode
 *
 * @param serlEnt   The entity will be in cmux mode
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cmuxStartService(SerialEntity_t *serlEnt);

/**
 * @brief cmuxStopService(SerialEntity_t *serlEnt)
 * @details unset serial port from cmux mode
 *
 * @param serlEnt   The entity will be out of cmux mode
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cmuxStopService(SerialEntity_t *serlEnt);

/**
 * @brief cmuxFsmInput(CmuxFrameDesc_t *desc, CmuxDlcCtxMan_t *cmuxCtx)
 * @details cmux frame input
 *
 * @param desc     The info of cmux frame
 * @param cmuxCtx  cmux dlc context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t cmuxFsmInput(CmuxFrameDesc_t *desc, CmuxDlcCtxMan_t *cmuxCtx);

int32_t cmuxFrameOutput(CcioDevice_t *chdev, DlPduBlock_t *dlpdu, void *extras);

int32_t cmuxDlc0Init(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlc0Deinit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlc0Input(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlc0Output(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlc0Ctl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);

int32_t cmuxDlcAtInit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcAtDeinit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcAtInput(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlcAtOutput(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlcAtCtl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);
/* atos enhanced service */
int32_t cmuxDlcPppInput(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlcPppOutput(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlcPppCtl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);

int32_t cmuxDlcOpaqInit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcOpaqDeinit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcOpaqInput(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlcOpaqOutput(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlcOpaqCtl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);

int32_t cmuxDlcEthInit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcEthDeinit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcEthInput(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlcEthOutput(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlcEthCtl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);

int32_t cmuxDlcAudioInit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcAudioDeinit(CmuxDlcEntity_t *dlcEnt);
int32_t cmuxDlcAudioInput(CmuxDlcEntity_t *dlcEnt, CmuxFrameDesc_t *desc, void *extras);
int32_t cmuxDlcAudioOutput(CmuxDlcEntity_t *dlcEnt, DlPduBlockList_t *list, void *extras);
int32_t cmuxDlcAudioCtl(CmuxDlcEntity_t *dlcEnt, uint32_t flags, void *args);

int32_t cmuxDlcSetAtIoc(CmuxDlcFuncs_t *dlcFns);
int32_t cmuxDlcSetPppIoc(CmuxDlcFuncs_t *dlcFns);

#ifdef __cplusplus
}
#endif
#endif


