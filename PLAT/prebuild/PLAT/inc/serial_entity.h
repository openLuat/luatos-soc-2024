/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: serial_entity.h
*
*  Description:
*
*  History: 2021/1/19 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef SERIAL_ENTITY_H
#define SERIAL_ENTITY_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "cmsis_os2.h"
#include "lwip/netif.h"
#include "ccio_pub.h"



#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define atosEntityOutput(atosEnt, list, extras)         csioOutputEntity((SerialEntity_t*)((AtosEntity_t*)(atosEnt))->baseEnt, list, extras)
#define opaqosEntityOutput(opaqosEnt, list, extras)     csioOutputEntity((SerialEntity_t*)((OpaqosEntity_t*)(opaqosEnt))->baseEnt, list, extras)
#define audioosEntityOutput(audioosEnt, list, extras)   csioOutputEntity((SerialEntity_t*)((AudioosEntity_t*)(audioosEnt))->baseEnt, list, extras)


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
/* for compability */
#define ATOS_ENTITY_FLAGS_UNDEF   CCIO_ENTITY_FLAGS_UNDEF
#define ATOS_ENTITY_FLAGS_INIT    CCIO_ENTITY_FLAGS_INIT
#define ATOS_ENTITY_FLAGS_ACT     CCIO_ENTITY_FLAGS_ACT
#define ATOS_ENTITY_FLAGS_MUX     CCIO_ENTITY_FLAGS_MUX
typedef CcioEntityFlags_bm AtosEntFlags_bm;

typedef enum
{
    SERIAL_EF_BASEONLY = 0,
    SERIAL_EF_ATOS,   /* including pppos */
    SERIAL_EF_OPAQOS,
    SERIAL_EF_AUDIOOS,
    SERIAL_EF_ETHOS,
    SERIAL_EF_MUXOS,

    SERIAL_EF_MAXNUM
}SerialEntityFlags_e;

typedef enum
{
    ATOS_CHM_CMD_OFFLINE = 0,
    ATOS_CHM_CMD_ONLINE,
    ATOS_CHM_DATA_ONLINE,

    ATOS_CHM_MAXNUM
}AtosChanMode_e;

/* for compability */
#define SERIAL_CHM_CMD_OFFLINE  ATOS_CHM_CMD_OFFLINE
#define SERIAL_CHM_CMD_ONLINE   ATOS_CHM_CMD_ONLINE
#define SERIAL_CHM_DATA_ONLINE  ATOS_CHM_DATA_ONLINE
#define SERIAL_CHM_MAXNUM       ATOS_CHM_MAXNUM
typedef AtosChanMode_e SerialChanMode_e;

typedef struct SubxosEntity
{
    struct SubxosEntity *next;

    void    *baseEnt;    /* for fast access */
    void    *dlcCtx;     /* dlc context */
    uint8_t  flags;      /* CcioEntityFlags_bm */
    uint8_t  rsvd[3];
}SubxosEntity_t;

typedef struct PpposEntity
{
    struct PpposEntity *next;

    uint8_t       netCid;       /* netif context Id between ps, use CcioLanMediaType_e instead for now */
    uint8_t       lanState;     /* refer to 'CcioLanMediaState_e', via monitor/user */
    uint8_t       pdp4Cid;      /* ipv4 pdp context Id, via monitor */
    uint8_t       pdp6Cid;      /* ipv6 pdp context Id, via monitor */

    uint8_t       needAct;      /* 0/1: need to activate pdp context or not? */
    uint8_t       rsvd[3];
    struct netif  pppIf;

    void         *extras;       /* for user context. */

    //struct stats_proto link;  /* statistic, via user */
}PpposEntity_t;

typedef struct AtosEntity
{
    struct AtosEntity *next;

    void    *baseEnt;      /* for fast access */
    void    *dlcCtx;       /* dlc context */
    uint8_t  flags;        /* AtosEntFlags_bm */
    uint8_t  atCid;        /* at context Id between ps  */
    uint8_t  outFmt;       /* CcioOutXferFormat_e */
    uint8_t  chanMode  :4; /* refer to 'AtosChanMode_e' */
    uint8_t  isChgMode :1; /* is channel mode changing or not? */
    uint8_t  rsvdBits  :3;
    uint32_t memThres;     /* dlfc memory threshold for ppp service */
    osSemaphoreId_t semaId;

    PpposEntity_t  *ppp;   /* at enhanced service */
}AtosEntity_t;

typedef struct OpaqosEntity
{
    struct OpaqosEntity *next;

    void    *baseEnt;    /* for fast access */
    void    *dlcCtx;     /* dlc context */
    uint8_t  flags;      /* CcioEntityFlags_bm */
    uint8_t  opaqCid;    /* opaq context Id between a certain app. */
    uint8_t  rsvd[2];
}OpaqosEntity_t;

typedef struct AudioosEntity
{
    struct AudioosEntity *next;

    void    *baseEnt;    /* for fast access */
    void    *dlcCtx;     /* dlc context */
    uint8_t  flags;      /* CcioEntityFlags_bm */
    uint8_t  audioCid;   /* audio context Id between a certain app. */
    uint8_t  rsvd[2];
}AudioosEntity_t;

typedef EnetEntity_t EthosEntity_t;

typedef struct SerialEntity
{
    /* inherited field & MUST be placed on the top! */
    CcioEntity_t  base;

    uint8_t  entFlags :4;   /* refer to 'SerialEntityFlags_e' */
    uint8_t  isBufGot :1;   /* is rbuf ready or not? */
    uint8_t  rsvdBits :3;
    uint8_t  refCnt;        /* the referred count of this serial entity */
    uint8_t  rsvd[2];

    UlPduBlockList_t inList;    /* for 1)raw ppp via sct, 2) mux data if needed */
    DlPduBlockList_t outList;   /* for 1)raw ppp via sw esc, fifo order */

    AtosEntity_t     at;    /* atEnt list @either SERIAL_EF_ATOS or SERIAL_EF_MUXOS */
    PpposEntity_t   *ppp;   /* for fast access, only one ppp entity is supported for now! @either SERIAL_EF_ATOS or SERIAL_EF_MUXOS */
    OpaqosEntity_t   opaq;  /* opaqEnt list @either SERIAL_EF_OPAQOS or SERIAL_EF_MUXOS */
    AudioosEntity_t  audio; /* audioEnt list @either SERIAL_EF_AUDIOOS or SERIAL_EF_MUXOS */
    EthosEntity_t    eth;   /* netEnt list @either SERIAL_EF_ETHOS or SERIAL_EF_MUXOS */
    void            *mux;   /* CmuxDlcCtxMan_t, valid @entFlags = SERIAL_EF_MUXOS */

    void            *extras;/* for user context. */
}SerialEntity_t;


typedef void*    (*dlSubxEntGetFunc)(uint8_t cid);
typedef uint32_t (*dlPendListGetFunc)(uint8_t cid, uint8_t flags, DlPduBlockList_t *list);

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
 * @brief csioInitAtEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an AToS entity
 *
 * @param serlEnt   The entity to be created
 * @param isForPpp  The entity is for ppp service or not
 * @param status_cb The handler of entity status
 * @param extras    The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitAtEntity(SerialEntity_t *serlEnt,
                         uint8_t isForPpp,
                         chentStatusCallback statusCb,
                         void *extras);

/**
 * @brief csioDeinitAtEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an AToS entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitAtEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioInitPppEntity(AtosEntity_t *atosEnt, void *extras)
 * @details create an PPPoS entity
 *
 * @param atosEnt   The entity to be created
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitPppEntity(AtosEntity_t *atosEnt, void *extras);

/**
 * @brief csioDeinitPppEntity(AtosEntity_t *atosEnt)
 * @details delete/reset an PPPoS entity
 *
 * @param atosEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitPppEntity(AtosEntity_t *atosEnt);

/**
 * @brief csioInitDiagEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an DIAGoS entity
 *
 * @param serlEnt The entity to be created
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitDiagEntity(SerialEntity_t *serlEnt,
                           chentStatusCallback statusCb,
                           void *extras);

/**
 * @brief csioDeinitDiagEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an DIAGoS entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitDiagEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioInitOpaqEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an opaqos entity
 *
 * @param serlEnt The entity to be created
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitOpaqEntity(SerialEntity_t *serlEnt,
                           chentStatusCallback statusCb,
                           void *extras);

/**
 * @brief csioDeinitOpaqEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an opaqoS entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitOpaqEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioInitAudioEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an audio entity
 *
 * @param serlEnt The entity to be created
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitAudioEntity(SerialEntity_t *serlEnt,
                            chentStatusCallback statusCb,
                            void *extras);

/**
 * @brief csioDeinitAudioEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an audio entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitAudioEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioInitEthEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an eth entity
 *
 * @param serlEnt The entity to be created
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitEthEntity(SerialEntity_t *serlEnt,
                          chentStatusCallback statusCb,
                          void *extras);

/**
 * @brief csioDeinitEthEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an eth entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitEthEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioInitMuxEntity(SerialEntity_t *serlEnt, chentStatusCallback statusCb, void *extras)
 * @details create an mux entity
 *
 * @param serlEnt The entity to be created
 * @param status_cb The handler of entity status
 * @param extras The user's extra info/useful context
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioInitMuxEntity(SerialEntity_t *serlEnt,
                          chentStatusCallback statusCb,
                          void *extras);

/**
 * @brief csioDeinitMuxEntity(SerialEntity_t *serlEnt)
 * @details delete/reset an mux entity
 *
 * @param serlEnt The entity to be deleted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioDeinitMuxEntity(SerialEntity_t *serlEnt);

/**
 * @brief csioSetUpChannel(SerialEntity_t *serlEnt)
 * @details establish a serial(AT or PPP) channel
 *
 * @param serlEnt The entity to be established the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioSetUpChannel(SerialEntity_t *serlEnt);

/**
 * @brief csioPullDownChannel(SerialEntity_t *serlEnt)
 * @details destroy a serial(AT or PPP) channel
 *
 * @param serlEnt The entity to be destroied the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioPullDownChannel(SerialEntity_t *serlEnt);

/**
 * @brief csioSetChanMode(AtosEntity_t *atosEnt, AtosChanMode_e chanMode)
 * @details set/change the serial channel mode
 *
 * @param atosEnt  The serial entity whose chanMode is changed
 * @param chanMode The value of new chan mode
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioSetChanMode(AtosEntity_t *atosEnt, AtosChanMode_e chanMode);

/**
 * @brief csioHoldOnPppSessn(AtosEntity_t *atosEnt)
 * @details switch to online command mode and hold on ppp sessn
 *
 * @param serlEnt  The serial entity whose ppp sessn will be hold on
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioHoldOnPppSessn(AtosEntity_t *atosEnt);

/**
 * @brief csioAltChanType(SerialEntity_t *serlEnt, CsioDevType_e newType)
 * @details the serial channel will be altered to a new service
 *
 * @param serlEnt  The serial entity whose chanType is altered
 * @param newType  The new service type of the channel
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioAltChanType(SerialEntity_t *serlEnt, CsioDevType_e newType);

/**
 * @brief csioTryAdjustDlfcMemThres(AtosEntity_t *atosEnt)
 * @details try to adjust dlfc memory threshold according the serial baudrate
 *
 * @param atosEnt  The serial entity whose dlfc memory threshold will be adjusted
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioTryAdjustDlfcMemThres(AtosEntity_t *atosEnt);

/**
 * @brief csioTryRestoreDlfcMemThres(AtosEntity_t *atosEnt)
 * @details try to restore dlfc memory threshold if adjusted
 *
 * @param atosEnt  The atos entity whose dlfc memory threshold will be restored
 * @return 0 succ; < 0 failure with errno.
 */
int32_t csioTryRestoreDlfcMemThres(AtosEntity_t *atosEnt);

int32_t csioInputAtDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputPppDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputDiagDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputOpaqDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputAudioDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputEthDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);
int32_t csioInputMuxDevice(CcioDevice_t *chdev, uint32_t xferCnt, void *extras);

int32_t csioInputAtEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputPppEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputDiagEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputOpaqEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputAudioEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputEthEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);
int32_t csioInputMuxEntity(SerialEntity_t *serlEnt, UlPduBlock_t *ulpdu, void *extras);

int32_t csioOutputEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);

int32_t csioOutputAtEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputPppEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputDiagEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputOpaqEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputAudioEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputEthEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);
int32_t csioOutputMuxEntity(SerialEntity_t *serlEnt, DlPduBlockList_t *list, void *extras);

#ifdef __cplusplus
}
#endif
#endif


