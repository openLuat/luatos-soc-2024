/****************************************************************************
 *
 * Copy right:   2023-, Copyrigths of AirM2M Ltd.
 * File name:    tpComm.c
 * Description:  ec7xx tpComm driver source file
 * History:      Rev1.0   2023-11-13
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
#include "tpComm.h"
#include "sctdef.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
#include "api_comm.h"
AP_PLAT_COMMON_BSS uint32_t tp_irq_padId = 0;
AP_PLAT_COMMON_BSS uint32_t tp_irq_PinId = 0;
AP_PLAT_COMMON_BSS uint32_t tp_i2c_usrId = 0;
#endif
extern ARM_DRIVER_I2C 	Driver_I2C0;
AP_PLAT_COMMON_DATA ARM_DRIVER_I2C	*i2cMasterDrv = &CREATE_SYMBOL(Driver_I2C, 0);
/**
  \fn          
  \brief    
  \return
*/
#ifndef FEATURE_SUBSYS_OPENHAL_ENABLE
AP_PLAT_COMMON_BSS static tpIsrFunc tpIsrCb = NULL;
void tpGpioIsr(uint32_t index)
{
    uint16_t portIrqMask = GPIO_saveAndSetIrqMask(TP_IRQ_GPIO_INSTANCE);
    if (GPIO_getInterruptFlags(TP_IRQ_GPIO_INSTANCE) & (1 << TP_IRQ_GPIO_PIN)){
        GPIO_clearInterruptFlags(TP_IRQ_GPIO_INSTANCE, 1 << TP_IRQ_GPIO_PIN);
        if(tpIsrCb != NULL){
            tpIsrCb(index);
        }
    }
    GPIO_restoreIrqMask(TP_IRQ_GPIO_INSTANCE, portIrqMask);
}
#endif
/**
  \fn          
  \brief     
  \return
*/
void tpIsrInit(void *cb)
{
    PadConfig_t padConfig = {0};
    PAD_getDefaultConfig(&padConfig);
    padConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
    padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    padConfig.mux = TP_IRQ_PAD_ALT_FUNC;
    #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    tp_irq_padId = api_pad_create(TP_IRQ_PAD_INDEX, &padConfig);
    api_pad_open(tp_irq_padId,NULL,0);
    #else
    PAD_setPinConfig(TP_IRQ_PAD_INDEX, &padConfig);
    #endif
    GpioPinConfig_t pinConfig = {0};
    pinConfig.pinDirection = GPIO_DIRECTION_INPUT;
    pinConfig.misc.interruptConfig = GPIO_INTERRUPT_RISING_EDGE;
    #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    tp_irq_PinId = api_gpio_create((TP_IRQ_GPIO_INSTANCE*16+TP_IRQ_GPIO_PIN), &pinConfig);
    api_gpio_open(tp_irq_PinId,NULL,0);
    api_gpio_ioctl(tp_irq_PinId,OPEN_GPIO_IOCTL_ISR_CB,cb);
    #else
    tpIsrCb = cb;
    GPIO_pinConfig(TP_IRQ_GPIO_INSTANCE, TP_IRQ_GPIO_PIN, &pinConfig);
    XIC_SetVector(PXIC1_GPIO_IRQn, tpGpioIsr);
    XIC_EnableIRQ(PXIC1_GPIO_IRQn);
    #endif
    // SYSLOG_PRINT(SL_INFO, "tpirq[%x],[%x]\r\n",tp_irq_PinId,tp_i2c_usrId);
}
/**
  \fn          
  \brief     
  \return
*/
void tpRstInit(void)
{
    #ifdef TP_RST_GPIO_PIN
    PadConfig_t padConfig = {0};
    PAD_getDefaultConfig(&padConfig);
    padConfig.pullUpEnable = PAD_PULL_UP_DISABLE;
    padConfig.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    padConfig.mux = TP_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(TP_RST_PAD_INDEX, &padConfig);

    GpioPinConfig_t gpioCfg = {0};
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(TP_RST_GPIO_INSTANCE, TP_RST_GPIO_PIN, &gpioCfg);
    #endif
}
/**
  \fn          
  \brief    
  \return
*/
void tpBusInit(void)
{
    #ifdef FEATURE_SUBSYS_OPENHAL_ENABLE
    tp_i2c_usrId = api_i2c_create(0,NULL);
    api_i2c_open(tp_i2c_usrId,NULL,0);
    // SYSLOG_PRINT(SL_INFO, "0x%X\r\n",tp_i2c_usrId);
    #else
    i2cMasterDrv->Uninitialize();
    i2cMasterDrv->Initialize(NULL);
    i2cMasterDrv->PowerControl(ARM_POWER_FULL);
    i2cMasterDrv->Control(ARM_I2C_BUS_SPEED, ARM_I2C_BUS_SPEED_FAST);
    i2cMasterDrv->Control(ARM_I2C_BUS_CLEAR, 0);
    #endif
    // TP_TRACE(tpBusInit, 1, "I2C_IO_MODE:%d",I2C_IO_MODE);
}
/**
  \fn          
  \brief    
  \return
*/
void tpBusDeInit(void)
{
    i2cMasterDrv->Uninitialize();
}
/**
  \fn          
  \brief    
  \return
*/
uint8_t tp_i2c_read(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data) 
{
    i2cMasterDrv->MasterTransmit(dev_id, &reg_addr, 1, true);   
    i2cMasterDrv->MasterReceive(dev_id, data, len, false);
    return 0;
}
/**
  \fn          
  \brief    
  \return
*/
uint8_t tp_i2c_send(uint8_t dev_id, uint8_t reg_addr, uint16_t len, uint8_t *data)
{
    uint8_t * tempBuffer = calloc(len+1,sizeof(char));
    memcpy(tempBuffer+1,data,len);
    tempBuffer[0] = reg_addr;
    i2cMasterDrv->MasterTransmit(dev_id, tempBuffer, len+1, false);
    free(tempBuffer);
    return 0;
}

#endif