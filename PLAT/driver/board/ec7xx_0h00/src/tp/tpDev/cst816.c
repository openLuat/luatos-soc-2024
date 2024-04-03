/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    CST816.c
 * Description:  EC7xx touchpanel driver source file
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/
// #ifdef FEATURE_TP_CST816_ENABLE
#include <stdio.h>
#include <string.h>
#include "ec7xx.h"
#include "bsp.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "tpDrv.h"
#include "tpComm.h"
#include "cst816.h"
/**
  \fn          
  \brief    
  \return
*/
int CST816_Write(uint8_t regAddr, uint8_t* buf,uint8_t len)
{
    return tp_i2c_send(CST816_ADDR, regAddr, len, buf);  
}
/**
  \fn          
  \brief    
  \return
*/
int CST816_Read(uint8_t regAddr, uint8_t* buf,uint8_t len)
{
    return tp_i2c_read(CST816_ADDR, regAddr, len, buf);   
}
/**
  \fn          
  \brief    
  \return
*/
void tp_cst816_init(void)
{

}
/**
  \fn          
  \brief    
  \return
*/
static uint16_t last_x=0,last_y=0;
uint8_t tp_cst816_scan(int16_t *pos)
{
    uint8_t temp[6]={0}; 
    // uint16_t x=0,y=0;
    // printf("gpioInterruptCount : %0x\r\n",gpioInterruptCount);
    CST816_Read(CST816_GET_GESTUREID,temp,6);
    // printf("CST816_GET_GESTUREID:%02x\r\n",temp[0]);
    // printf("CST816_GET_FINGERNUM:%02x\r\n",temp[1]);
    // printf("CST816_GET_LOC0:%02x,%02x,%02x,%02x\r\n",temp[2],temp[3],temp[4],temp[5]);
    
    pos[0]=((uint16_t)(temp[2]&0x0F)<<8)+temp[3];
    pos[1]=(((uint16_t)(temp[4]&0x0F)<<8)+temp[5]);
    if(last_x!=pos[0] || last_y!=pos[1]){
        SYSLOG_PRINT(SL_INFO,"%d,%d,x:%d->%d,y:%d->%d\r\n",temp[0],temp[1],last_x,pos[0],last_y,pos[1]);
        last_x = pos[0];
        last_y = pos[1];
    }
    return temp[1];
}

/**
  \fn          
  \brief    
  \return
*/
tpDrvFunc_t cst816Drv = 
{
    .chipID         = 0x816,
    // .init           = tp_cst816_init,
    // .send           = tp_cst816_send,
    // .read           = tp_cst816_Read,
    // .scan           = tp_cst816_Scan,
};

/**
  \fn          
  \brief    
  \return
*/
tpDrvPra_t cst816Pra = 
{
    .id     = 0x816,
    .width  = 240,
    .height = 280,
};

// #endif
