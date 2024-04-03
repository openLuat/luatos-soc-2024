/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    CST816.h
 * Description:  CST816 driver file
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/
#ifndef _TP_CST816_H
#define _TP_CST816_H
 #ifdef __cplusplus
 extern "C" {
#endif

#include "ec7xx.h"
#include "Driver_Common.h"


#define CST816_ADDR 		      0X15 

#define CST816_GET_GESTUREID  0x01
#define CST816_GET_FINGERNUM  0x02
#define CST816_GET_LOC0       0x03
#define CST816_GET_CHIPID     0xA7

/*******************************************************************************
 * API
 ******************************************************************************/
uint8_t tp_cst816_scan(int16_t *pos);
void tp_cst816_init(void);


#ifdef __cplusplus
}
#endif
#endif
