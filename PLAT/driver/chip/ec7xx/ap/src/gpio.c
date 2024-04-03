 /****************************************************************************
  *
  * Copy right:   2017-, Copyrigths of AirM2M Ltd.
  * File name:    gpio.c
  * Description:  EC718 gpio driver source file
  * History:
  *
  ****************************************************************************/

#include "gpio.h"
#include "clock.h"
#include "slpman.h"
#ifdef PM_FEATURE_ENABLE
#include "apmu_external.h"
#endif
#define EIGEN_GPIO(n)             ((GPIO_TypeDef *) (RMI_GPIO_BASE_ADDR + 0x1000*n))

/**
  \brief GPIO stutas flag
 */
static uint32_t gGpioStatus = 0;

#ifdef PM_FEATURE_ENABLE
/**
  \fn        void GPIO_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Backup gpio configurations before sleep.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
void GPIO_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    return;
}

/**
 \fn        void GPIO_exitLowPowerStateRestore(void* pdata, slpManLpState state)
 \brief     Restore gpio configurations after exit from sleep.
 \param[in] pdata pointer to user data, not used now
 \param[in] state low power state
 */
void GPIO_exitLowPowerStateRestore(void* pdata, slpManLpState state)
{

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            if(gGpioStatus & 0x1U)
            {
                GPR_gpioClkForceOnControl(true);
                GPR_clockEnable(GPIO_RMI_HCLK);
            }

            break;

        default:
            break;
    }

}
#endif

void GPIO_driverInit(void)
{
    if((gGpioStatus & 0x1U) == 0)
    {

        GPR_gpioClkForceOnControl(true); // force on clock in case of sleep

#ifdef PM_FEATURE_ENABLE
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_GPIO_MODULE, GPIO_enterLowPowerStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_GPIO_MODULE, GPIO_exitLowPowerStateRestore, NULL);
#endif
        CLOCK_clockEnable(GPIO_RMI_HCLK);

        gGpioStatus = 0x1U;

    }

}

void GPIO_driverDeInit(void)
{
    gGpioStatus = 0;

    // disable clock
    CLOCK_clockDisable(GPIO_RMI_HCLK);

#ifdef PM_FEATURE_ENABLE
    apmuVoteToDozeState(PMU_DOZE_GPIO_MOD, true);

    slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_GPIO_MODULE);
    slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_GPIO_MODULE);
#endif
}

#ifdef PM_FEATURE_ENABLE
bool GPIO_hasPinWaitInterrupt(void)
{
    GPIO_TypeDef *base;
    uint8_t i = 0;
    for(i=0; i<GPIO_INSTANCE_NUM; i++)
    {
        base = EIGEN_GPIO(i);
        if(base->INTENSET != 0)
            return true;
    }
    return false;
}
#endif

void GPIO_pinConfig(uint32_t port, uint16_t pin, const GpioPinConfig_t *config)
{
    ASSERT(port < GPIO_INSTANCE_NUM);

    uint16_t pinMask = 0x1U << pin;
    GPIO_TypeDef *base = EIGEN_GPIO(port);

    GPIO_driverInit();

    switch(config->pinDirection)
    {
        case GPIO_DIRECTION_INPUT:

            base->OUTENCLR = pinMask;

            GPIO_interruptConfig(port, pin, config->misc.interruptConfig);

            break;
        case GPIO_DIRECTION_OUTPUT:

            GPIO_pinWrite(port, pinMask, ((uint16_t)config->misc.initOutput) << pin);

            base->OUTENSET = pinMask;

            break;
        default:
            break;
    }

}

void GPIO_interruptConfig(uint32_t port, uint16_t pin, GpioInterruptConfig_e config)
{
    uint16_t pinMask = 0x1U << pin;
    GPIO_TypeDef *base = EIGEN_GPIO(port);

    switch(config)
    {
        case GPIO_INTERRUPT_DISABLED:

            base->INTENCLR = pinMask;

            break;
        case GPIO_INTERRUPT_LOW_LEVEL:

            base->INTPOLCLR = pinMask;
            base->INTTYPECLR = pinMask;
            base->INTENSET = pinMask;

            break;
        case GPIO_INTERRUPT_HIGH_LEVEL:

            base->INTPOLSET = pinMask;
            base->INTTYPECLR = pinMask;
            base->INTENSET = pinMask;

            break;
        case GPIO_INTERRUPT_FALLING_EDGE:

            base->INTPOLCLR = pinMask;
            base->INTTYPESET = pinMask;
            base->INTEDGECLR = pinMask;
            base->INTENSET = pinMask;

            break;
        case GPIO_INTERRUPT_RISING_EDGE:

            base->INTPOLSET = pinMask;
            base->INTTYPESET = pinMask;
            base->INTEDGECLR = pinMask;
            base->INTENSET = pinMask;

            break;

        case GPIO_INTERRUPT_BOTH_EDGE:

            base->INTTYPESET = pinMask;
            base->INTEDGESET= pinMask;
            base->INTENSET = pinMask;

            break;
        default :
            break;
    }
    #ifdef PM_FEATURE_ENABLE
    if(GPIO_hasPinWaitInterrupt())
        apmuVoteToDozeState(PMU_DOZE_GPIO_MOD, false);
    else
        apmuVoteToDozeState(PMU_DOZE_GPIO_MOD, true);
    #endif
}

void GPIO_pinWrite(uint32_t port, uint16_t pinMask, uint16_t output)
{
#if 0
    EIGEN_GPIO(port)->MASKLOWBYTE[pinMask & 0xFFU] = output;
    EIGEN_GPIO(port)->MASKHIGHBYTE[pinMask >> 8U] = output;
#else
    uint32_t dataOut = ((~pinMask) << 16) | output;
    EIGEN_GPIO(port)->DATAOUT = dataOut;
#endif
}

uint32_t GPIO_pinRead(uint32_t port, uint16_t pin)
{
    return (((EIGEN_GPIO(port)->DATA) >> pin) & 0x01U);
}

uint16_t GPIO_getInterruptFlags(uint32_t port)
{
    return EIGEN_GPIO(port)->INTSTATUS;
}

void GPIO_clearInterruptFlags(uint32_t port, uint16_t mask)
{
    EIGEN_GPIO(port)->INTSTATUS = mask;
}

uint16_t GPIO_saveAndSetIrqMask(uint32_t port)
{
    uint16_t mask = EIGEN_GPIO(port)->INTENSET;
    EIGEN_GPIO(port)->INTENCLR = 0xFFFFU;
    return mask;
}

void GPIO_restoreIrqMask(uint32_t port, uint16_t mask)
{
    EIGEN_GPIO(port)->INTENSET = mask;
}


void GPIO_pinSetDirectionOnly(uint32_t port, uint16_t pin, GpioPinDirection_e dir)
{
    ASSERT(port < GPIO_INSTANCE_NUM);
    uint16_t pinMask = 0x1U << pin;
    GPIO_TypeDef *base = EIGEN_GPIO(port);
    GPIO_driverInit();
    switch(dir)
    {
        case GPIO_DIRECTION_INPUT:
            base->OUTENCLR = pinMask;
            break;
        case GPIO_DIRECTION_OUTPUT:
            base->OUTENSET = pinMask;
            break;
    }
}
