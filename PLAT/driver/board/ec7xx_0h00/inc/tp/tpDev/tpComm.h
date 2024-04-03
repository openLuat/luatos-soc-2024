/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    tpComm.h
 * Description:  ec7xx tpComm.h 
 * History:      Rev1.0   2023-11-13
 *
 ****************************************************************************/
#ifndef _TP_COMM_H
#define _TP_COMM_H
#ifdef __cplusplus
extern "C" {
#endif

#include "ec7xx.h"
#include "Driver_Common.h"
#include "bsp.h"

typedef void (*tpIsrFunc)(uint32_t);
#define I2C_IO_MODE 	RTE_I2C0_IO_MODE

extern ARM_DRIVER_I2C *i2cMasterDrv;

void tpBusInit(void);
void tpRstInit(void);
void tpIsrInit(void *cb);
uint8_t tp_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
uint8_t tp_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data);
#ifdef __cplusplus
}
#endif
#endif
