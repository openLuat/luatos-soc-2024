/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: hal_i2c.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: Hal I2C interface
*
******************************************************************************/


#ifndef _HAL_I2C_H
#define _HAL_I2C_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "stdio.h"
#include "stdlib.h"
#include "ec7xx.h"
#include "string.h"
#include "Driver_Common.h"
#include "bsp.h"
#ifdef FEATURE_OS_ENABLE
#include "osasys.h"
#include "cmsis_os2.h"
#endif
#include "slpman.h"
#include "exception_process.h"
#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
*                   DATA TYPE DEFINITION                                     *
*----------------------------------------------------------------------------*/
typedef struct
{
    ARM_DRIVER_I2C   *i2cDrv;
    void             *i2cSemId;
    uint8_t           initCnt;
}HalI2cPamram_t;



/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

int32_t halI2cInit(bool needLock);
int32_t halI2cDeInit(bool needLock);
int32_t halI2cWrite(uint8_t slaveAddr, uint8_t* cmd, uint32_t cmdNum, uint8_t* rxNack, bool needLock);
int32_t halI2cRead(uint8_t slaveAddr, uint8_t regAddr, uint8_t *retData, bool needLock);
int32_t halI2cGetStats(ARM_I2C_STATUS *retData, bool needLock);



#ifdef __cplusplus
}
#endif

#endif /* _HAL_I2C_H */
