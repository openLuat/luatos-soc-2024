/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    tpDrv.h
 * Description:  ec7xx tpDrv 
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/
#ifndef _TP_DRV_H
#define _TP_DRV_H
#ifdef __cplusplus
extern "C" {
#endif

#include "tpComm.h"

#define SUPPORT_TP_NUM             (3)
#define INSTALL_TP_NUM             (1)

typedef struct 
{
    uint16_t chipID;
    int (*init)(void *cb);
    int (*send)(void *tp);
    int (*read)(void *tp);
    int (*scan)(void *tp);
}tpDrvFunc_t;

typedef struct
{
    uint16_t        id;
    uint32_t        width;
    uint32_t        height;
}tpDrvPra_t;

typedef struct
{
    char        *name;          ///< lcd's name used to configure its id, then use id to find its info, including driver function
    uint16_t    id;             ///< every lcd's id should be different, no matter lcd's type is the same or not
}tpObj_t;

typedef struct
{
    int             handle;      
    tpObj_t         *obj;  
    tpDrvPra_t      *pra;           
    tpDrvFunc_t     *drv;          
}tpDev_t;

int tpInit(void* tp_cb);
tpDev_t* tpOpen(char* name);
uint8_t tpScan(tpDev_t* tpDev);
void tpLoop(tpDev_t* tpDev,uint32_t timeout);
void tpData(int16_t* x,int16_t* y);
bool tpPressed(void);

#ifdef __cplusplus
}
#endif
#endif
