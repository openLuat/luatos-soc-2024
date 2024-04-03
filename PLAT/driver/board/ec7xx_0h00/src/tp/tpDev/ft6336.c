/****************************************************************************
 *
 * Copy right:   2024-, Copyrigths of AirM2M Ltd.
 * File name:    ft6336.c
 * Description:  ec7xx ft6336 driver source file
 * History:      Rev1.1   2024-03-08
 *
 ****************************************************************************/
#ifdef FEATURE_TP_FT6336_ENABLE
#include <stdio.h>
#include <string.h>
#include "tpDrv.h"
#include "tpComm.h"
#include "ft6336.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
/**
  \fn          
  \brief    
  \return
*/
int tp_ft6336_send(uint8_t regAddr, uint8_t* buf,uint8_t len)
{
    return tp_i2c_send(FT6336_ADDR, regAddr, len, buf);   
}
/**
  \fn          
  \brief    
  \return
*/
int tp_ft6336_read(uint8_t regAddr, uint8_t* buf,uint8_t len)
{
    return tp_i2c_read(FT6336_ADDR, regAddr, len, buf);   
}

/**
  \fn          
  \brief    
  \return
*/
uint8_t tp_ft6336_scan(int16_t *pos)
{
    uint8_t temp[8]={0}; 
    uint8_t fingerNum = 0;
    tp_ft6336_read(FT6336_GET_FINGERNUM,&fingerNum,1);
    if(fingerNum)
    {
        tp_ft6336_read(FT6336_GET_LOC0,temp,4);
        pos[0]=((uint16_t)(temp[0]&0x0F)<<8)+temp[1];
        pos[1]=(((uint16_t)(temp[2]&0x0F)<<8)+temp[3]);
        // printf("x1:%d,y1:%d\r\n",pos[0], pos[1]);
        if(fingerNum>1)
        {
            tp_ft6336_read(FT6336_GET_LOC1,&temp[4],4);
            pos[2]=((uint16_t)(temp[4]&0x0F)<<8)+temp[5];
            pos[3]=(((uint16_t)(temp[6]&0x0F)<<8)+temp[7]);
        }
    }
	return fingerNum;
}
/**
  \fn          
  \brief    
  \return
*/

static int tp_ft6336_init(void* cb)
{   
    SYSLOG_PRINT(SL_INFO,"ID\r\n");
    return 0;
}
/**
  \fn          
  \brief    
  \return
*/
tpDrvFunc_t ft6336Drv = 
{
    .init           = tp_ft6336_init,
    .send           = tp_ft6336_send,
    .read           = tp_ft6336_read,
    .scan           = tp_ft6336_scan,
};
/**
  \fn          
  \brief    
  \return
*/
tpDrvPra_t ft6336Pra = 
{
    .id     = 0x6336,
    .width  = 240,
    .height = 320,
};
#endif