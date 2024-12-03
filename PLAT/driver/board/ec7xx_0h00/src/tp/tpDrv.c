/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    tpDrv.c
 * Description:  ec7xx tpDrv driver source file
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/
#ifdef FEATURE_DRIVER_TP_ENABLE
#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "bsp_custom.h"
#include "osasys.h"
#include "ostask.h"
#include "slpman.h"
#include "tpDrv.h"

#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

#ifdef FEATURE_TP_FT6336_ENABLE
#include "ft6336.h"
extern tpDrvFunc_t  ft6336Drv;
extern tpDrvPra_t   ft6336Pra;
#endif 
#ifdef FEATURE_TP_CST816_ENABLE
#include "cst816.h"
extern tpDrvFunc_t  cst816Drv;
extern tpDrvPra_t   cst816Pra;
#endif 
#ifdef FEATURE_TP_GT911_ENABLE
#include "gt911.h"
extern tpDrvFunc_t  gt911Drv;
extern tpDrvPra_t   gt911Pra;
#endif 
#include "sctdef.h"

AP_PLAT_COMMON_BSS static tpDev_t tpDevList[SUPPORT_TP_NUM] ;
AP_PLAT_COMMON_DATA static tpObj_t tpObjList[SUPPORT_TP_NUM] = 
{
    {"ft6336",  0x6336},
    {"cst816",  0x816},
    {"gt911",   0x911},
};

AP_PLAT_COMMON_BSS int16_t xy_pos[4]={0};
AP_PLAT_COMMON_BSS int16_t last_xy_pos[4]={0};
AP_PLAT_COMMON_BSS static bool is_pressed = false;
#ifndef FEATURE_SUBSYS_INPUT_ENABLE
AP_PLAT_COMMON_BSS static osSemaphoreId_t    tpSemaphore	= NULL;
#endif

#define TP_TRACE(subId, argLen, format,  ...)  \
    ECOMM_TRACE(UNILOG_TP, subId, P_VALUE, argLen, format,  ##__VA_ARGS__) 
/**
  \fn          
  \brief    
  \return
*/
void tpIsrCallback(uint32_t data)
{
    #ifdef FEATURE_SUBSYS_INPUT_ENABLE
    inputNotify();
    #else
    if (tpSemaphore != NULL)
    {
        osSemaphoreRelease(tpSemaphore);
    }
    #endif
}
/**
  \fn          
  \brief    
  \return
*/
bool tpPressed(void)
{
    // SYSLOG_PRINT(SL_INFO,"get\r\n");
	return is_pressed;
}
/**
  \fn          
  \brief     
  \return
*/
uint8_t tpScan(tpDev_t* tpDev)
{
    uint8_t fingers = 0;
    if (tpDev!=NULL) fingers = tpDev->drv->scan(xy_pos);
    if(fingers)
    {
        if(xy_pos[0]!=last_xy_pos[0] || xy_pos[1]!=last_xy_pos[1]) 
        {
            is_pressed = true;
            TP_TRACE(tpScan, 5, "%d,%d->%d,%d->%d",fingers,last_xy_pos[0],xy_pos[0],last_xy_pos[1],xy_pos[1]);
            SYSLOG_PRINT(SL_INFO,"[%d]%d->%d,%d->%d\r\n",fingers,last_xy_pos[0],xy_pos[0],last_xy_pos[1],xy_pos[1]);
            memcpy(last_xy_pos,xy_pos,sizeof(xy_pos));
        }
	}
    return fingers;
}
/**
  \fn          
  \brief     
  \return
*/
void tpData(int16_t* x,int16_t* y)
{
    if(is_pressed)
	{
		*x=xy_pos[0];
		*y=xy_pos[1];
		is_pressed = false;
        // SYSLOG_PRINT(SL_INFO,"clean\r\n");
	}
}


/**
  \fn          
  \brief     
  \return
*/
void tpLoop(tpDev_t* tpDev,uint32_t timeout)
{
    #ifndef FEATURE_SUBSYS_INPUT_ENABLE
    osSemaphoreAcquire(tpSemaphore, timeout);
    #else
    osDelay(timeout);
    #endif
    tpScan(tpDev);
}
/**
  \fn          
  \brief     
  \return
*/
static uint8_t tpFind(tpDev_t *tpList)
{
    uint8_t cnt = 0; 
    if (tpList == NULL) return SUPPORT_TP_NUM;
    for (uint8_t i = 0; i < SUPPORT_TP_NUM; i++)
    {
        tpDev_t *pdev = NULL; 
        memset(tpList + cnt, 0, sizeof(tpDev_t));
        #ifdef FEATURE_TP_FT6336_ENABLE
        if (tpObjList[i].id == ft6336Pra.id)
        {
            pdev = tpList + cnt;
            cnt++; 
            pdev->pra = &ft6336Pra;
            pdev->drv = &ft6336Drv;
            pdev->obj = &tpObjList[i];
        }
        #endif
        #ifdef FEATURE_TP_CST816_ENABLE
        if (tpObjList[i].id == cst816Pra.id)
        {
            pdev = tpList + cnt;
            cnt++; 
            pdev->pra = &cst816Pra;
            pdev->drv = &cst816Drv;
            pdev->obj = &tpObjList[i];
        }
        #endif
        #ifdef FEATURE_TP_GT911_ENABLE
        if (tpObjList[i].id == gt911Pra.id)
        {
            pdev = tpList + cnt;
            cnt++; 
            pdev->pra = &gt911Pra;
            pdev->drv = &gt911Drv;
            pdev->obj = &tpObjList[i];
        }
        #endif
    }
    // Return the count of populated LCD devices
    return cnt;
}
/**
  \fn          
  \brief     
  \return
*/
int tpInit(void* tp_cb)
{
    uint8_t num = tpFind(tpDevList);
    if (num > INSTALL_TP_NUM) num = INSTALL_TP_NUM;
    if(num){
        #ifndef FEATURE_SUBSYS_INPUT_ENABLE
        if(tpSemaphore != NULL){
            osSemaphoreDelete(tpSemaphore);
        } 
        tpSemaphore = osSemaphoreNew(1, 1, NULL);
        #endif
        tpBusInit();
        tpRstInit();
    }
    for (uint8_t i = 0; i < num; i++) 
    {
        if (tpDevList[i].obj->id) 
        {
            tpDev_t *pdev = &tpDevList[i]; 
            pdev->drv->init(tp_cb);           
        }
    }
    if(num) tpIsrInit(tpIsrCallback);
    return num;
}
/**
  \fn          
  \brief        
  \return
*/
tpDev_t* tpOpen(char* name)
{
    if(name == NULL) return NULL;
    for (uint8_t i = 0; i < INSTALL_TP_NUM; i++)
    {
        tpDev_t *pdev = &tpDevList[i];
        // SYSLOG_PRINT(SL_INFO,"0x%X,%s,%s\r\n",pdev->obj->id,pdev->obj->name,name);
        if (pdev->obj->id != 0 && strcasecmp(name, pdev->obj->name) == 0)
        {
            SYSLOG_PRINT(SL_INFO,"%d:0x%X,%s\r\n",i,pdev->obj->id,pdev->obj->name);
            return pdev;
        }
    }
    return NULL;
}
#endif