/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: scom_event.h
*
*  Description:
*
*  History: 2025/1/19 created by xuwang
*
*  Notes: simulated COM events and its event group
*
******************************************************************************/
#ifndef SCOM_EVENT_H
#define SCOM_EVENT_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdint.h>
#include <string.h>

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
/* errno! */
#define SCOM_EOK    0
#define SCOM_ERR   -1

#define SCOM_EVENT_IDX_UNDEF  0xff


#define SCOM_EVENT_SET(evid, set)  \
            do{\
                if((evid) < SCOM_EVID_MAXID)\
                {\
                    (set) |= (1 << (evid));\
                }\
            }while(0)
#define SCOM_EVENT_CLR(evid, set)  \
            do{\
                if((evid) < SCOM_EVID_MAXID)\
                {\
                    (set) &= ~(1 << (evid));\
                }\
            }while(0)
#define SCOM_EVENT_ZERO(set)  \
            do{\
                (set) &= 0xFF000000;\
            }while(0)
#define SCOM_EVENT_ISSET(evid, set)  (((set) & (1 << (evid))) ? 1 : 0)


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
typedef uint32_t  EventSet_bm;

typedef enum
{
    SCOM_EVID_RX_BASE = 0,
    SCOM_EVID_RX_0 = SCOM_EVID_RX_BASE,
    //SCOM_EVID_RX_1,
    SCOM_EVID_RX_MAXNUM,

    SCOM_EVID_CTRL_BASE = 16,
    SCOM_EVID_WAKEUP = SCOM_EVID_CTRL_BASE,

    SCOM_EVID_MAXID = 24
}ScomEventId_e;

typedef void (*onEventFunc)(void *args);

typedef struct
{
    uint8_t      idx;
    uint8_t      persist; /* 1:true, 0:false */
    uint8_t      rsvd[2];
    uint32_t     evId;    /* ScomEventId_e */
    onEventFunc  onEventFn;
    void        *args;    /* args for onEventFunc */
}ScomEvent_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
void scomSetEvent(ScomEvent_t *ev, uint32_t evId, uint8_t isPersist, onEventFunc onEventFn, void *args);
void scomAddEvent(ScomEvent_t *ev);
void scomDelEvent(ScomEvent_t *ev);

uint16_t scomQueryEventCnt(void);
int32_t  scomQueryEventSet(EventSet_bm *evSet);
int32_t  scomHandleEventSet(EventSet_bm evSet);


#ifdef __cplusplus
}
#endif
#endif

