/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    hal_i2c.c
 * Description:  EC7XX i2c hal 
 * History:      Rev1.0   2021-9-18
 *
 ****************************************************************************/

#include "hal_i2c.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/
#define TRY_AGAIN_GET_LOCK_NUM    2

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/
extern ARM_DRIVER_I2C Driver_I2C0;
extern ARM_DRIVER_I2C Driver_I2C1;

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/
HalI2cPamram_t halI2cParam = {0};


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/
#ifdef FEATURE_OS_ENABLE

int halI2cLock(void* arg)
{
    osSemaphoreId_t i2cLock = (osSemaphoreId_t)arg;

    if (osSemaphoreAcquire(i2cLock, 1000) != osOK)
    {
        return osError;
    }
    return  osOK;
}

int halI2cUnlock(void* arg)
{
    osSemaphoreId_t i2cLock = (osSemaphoreId_t)arg;

    if (osSemaphoreRelease(i2cLock) != osOK)
    {
        return osError;
    }
    return  osOK;
}

int halI2cLockDestroy(void *arg)
{
    osSemaphoreId_t i2cLock = (osSemaphoreId_t)arg;

    if (osSemaphoreDelete(i2cLock) != osOK)
    {
        return osError;
    }
    return  osOK;
}
#endif


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
int32_t halI2cInit(bool needLock)
{
    int32_t ret = 0;
#ifdef FEATURE_OS_ENABLE
    uint8_t num = TRY_AGAIN_GET_LOCK_NUM;
#endif

    if (halI2cParam.initCnt++ == 0)
    {
        #if (RTE_I2C0)
        halI2cParam.i2cDrv = &CREATE_SYMBOL(Driver_I2C, 0); // Choose i2c0
        #elif (RTE_I2C1)
        halI2cParam.i2cDrv = &CREATE_SYMBOL(Driver_I2C, 1); // Choose i2c1
        #endif

#ifdef FEATURE_OS_ENABLE
        if (needLock)
        {        
            if (halI2cParam.i2cSemId == PNULL)
            {
                halI2cParam.i2cSemId = osSemaphoreNew(1, 1, NULL);
            
                if (halI2cParam.i2cSemId == PNULL)
                {
                    ECPLAT_PRINTF_OPT(UNILOG_PLA_DRIVER, halI2cInit_0, P_WARNING, "i2c hal can't create semaphore");
                    return osError;
                }
            }

            ret = halI2cLock(halI2cParam.i2cSemId);
            if (ret == osError)
            {
                do 
                {
                    osDelay(1);
                    ret = halI2cLock(halI2cParam.i2cSemId);
                } while ((ret == osError) && (--num > 0));

                if (ret == osError)
                {
                    ECPLAT_PRINTF_OPT(UNILOG_PLA_DRIVER, halI2cInit_1, P_WARNING, "i2c hal semaphore lock fail");
                    return ret;
                }
            }
        }
#endif

        halI2cParam.i2cDrv->Initialize(NULL);
        halI2cParam.i2cDrv->PowerControl(ARM_POWER_FULL);
        halI2cParam.i2cDrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_STANDARD);
        halI2cParam.i2cDrv->Control(ARM_I2C_BUS_CLEAR, 0);
#ifdef FEATURE_OS_ENABLE        
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
#endif        
    }

    return ret;
}

int32_t halI2cDeInit(bool needLock)
{    
    int32_t ret = 0;
#ifdef FEATURE_OS_ENABLE        
    uint8_t num = TRY_AGAIN_GET_LOCK_NUM;

    if (needLock) 
    {
        ret = halI2cLock(halI2cParam.i2cSemId);
        if (ret == osError)
        {
            do 
            {
                osDelay(1);
                ret = halI2cLock(halI2cParam.i2cSemId);
            } while ((ret == osError) && (--num > 0));
    
            if (ret == osError)
            {
                ECPLAT_PRINTF_OPT(UNILOG_PLA_DRIVER, halI2cDeInit_0, P_WARNING, "i2c hal semaphore lock fail");
                return ret;
            }
        }
    }

    if (halI2cParam.initCnt == 0)
    {
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
        return osErrorParameter;
    }
#endif

    if (--halI2cParam.initCnt == 0)
    {
        halI2cParam.i2cDrv->Uninitialize();
#ifdef FEATURE_OS_ENABLE
        if (needLock) 
        {
            halI2cUnlock(halI2cParam.i2cSemId);
            halI2cLockDestroy(halI2cParam.i2cSemId);
        }
#endif        
        memset(&halI2cParam, 0, sizeof(HalI2cPamram_t));
    }
    else
    {
#ifdef FEATURE_OS_ENABLE    
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
#endif        
    }

    return ret;
}

int32_t halI2cWrite(uint8_t slaveAddr, uint8_t* cmd, uint32_t cmdNum, uint8_t *rxNack, bool needLock)
{
    ARM_I2C_STATUS status = {0};
    int32_t ret = 0;
#ifdef FEATURE_OS_ENABLE    
    uint8_t num = TRY_AGAIN_GET_LOCK_NUM;

    if (needLock) 
    {
        ret = halI2cLock(halI2cParam.i2cSemId);
        if (ret == osError)
        {
            do
            {
                osDelay(1);
                ret = halI2cLock(halI2cParam.i2cSemId);
            } while ((ret == osError) && (--num > 0));

            if (ret == osError)
            {
                ECPLAT_PRINTF_OPT(UNILOG_PLA_DRIVER, halI2cWrite_0, P_WARNING, "i2c hal semaphore lock fail");
                return ret;
            }
        }
    }

    if (halI2cParam.initCnt == 0)
    {
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
        return osErrorParameter;
    }
#endif    
    
    halI2cParam.i2cDrv->MasterTransmit(slaveAddr, cmd, cmdNum, false);
    status = halI2cParam.i2cDrv->GetStatus();
    *rxNack = status.rx_nack;

#ifdef FEATURE_OS_ENABLE    
    if (needLock) 
    {
        ret = halI2cUnlock(halI2cParam.i2cSemId);
    }
#endif    

    return ret;
}

int32_t halI2cRead(uint8_t slaveAddr, uint8_t regAddr, uint8_t *retData, bool needLock)
{
    uint8_t a = regAddr;
    int32_t ret = 0;
#ifdef FEATURE_OS_ENABLE
    uint8_t num = TRY_AGAIN_GET_LOCK_NUM;

    if (needLock) 
    {
        ret = halI2cLock(halI2cParam.i2cSemId);
        if (ret == osError)
        {
            do 
            {
                osDelay(1);
                ret = halI2cLock(halI2cParam.i2cSemId);
            } while ((ret == osError) && (--num > 0));

            if (ret == osError)
            {
                ECPLAT_PRINTF_OPT(UNILOG_PLA_DRIVER, halI2cRead_0, P_WARNING, "i2c hal semaphore lock fail");
                return ret;
            }
        }
    }

    if (halI2cParam.initCnt == 0)
    {
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
        return osErrorParameter;
    }
#endif

    halI2cParam.i2cDrv->MasterTransmit(slaveAddr, (uint8_t *)&a, 1, true);   
    halI2cParam.i2cDrv->MasterReceive(slaveAddr, retData, 1, false);
#ifdef FEATURE_OS_ENABLE    
    if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
#endif    

    return ret;
}

int32_t halI2cGetStats(ARM_I2C_STATUS *retData, bool needLock)
{
    int32_t ret = 0;
#ifdef FEATURE_OS_ENABLE
    uint8_t num = TRY_AGAIN_GET_LOCK_NUM;

    if (needLock) 
    {
        ret = halI2cLock(halI2cParam.i2cSemId);
        if (ret == osError)
        {
            do 
            {
                osDelay(1);
                ret = halI2cLock(halI2cParam.i2cSemId);
            } while ((ret == osError) && (--num > 0));

            if (ret == osError)
            {
                return ret;
            }
        }
    }
    
    if (halI2cParam.initCnt == 0)
    {
        if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
        return osErrorParameter;
    }
#endif

    *retData = halI2cParam.i2cDrv->GetStatus();
#ifdef FEATURE_OS_ENABLE    
    if (needLock) halI2cUnlock(halI2cParam.i2cSemId);
#endif
    return ret;
}
