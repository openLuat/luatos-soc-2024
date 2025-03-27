/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: device_utils.h
*
*  Description:
*
*  History: 2024/1/31 created by KevinLiu
*
*  Notes:
*
******************************************************************************/
#ifndef __DEVICE_UTILS_H__
#define __DEVICE_UTILS_H__
#include "timer.h"
#include "bsp_custom.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C" {
#endif
/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define UART_DEV_PAD_PAD_INDEX(id,tag)      (UART##id##_##tag##_PAD_INDEX)
#define UART_DEV_PAD_GPIO_INSTANCE(id,tag)  (UART##id##_##tag##_GPIO_INSTANCE)
#define UART_DEV_PAD_GPIO_PIN(id,tag)       (UART##id##_##tag##_GPIO_PIN)
#define UART_DEV_PAD_PWM_INSTANCE(id,tag)   (UART##id##_##tag##_PWM_INSTANCE)
#define UART_DEV_PAD_PWM_CLK_ID(id,tag)     (UART##id##_##tag##_PWM_CLK_ID)
#define UART_DEV_PAD_PWM_CLK_SEL(id,tag)    (UART##id##_##tag##_PWM_CLK_SEL)

#define UART_DEV_DTR_PAD_INDEX(id)          UART_DEV_PAD_PAD_INDEX(id,DTR)
#define UART_DEV_DTR_GPIO_INSTANCE(id)      UART_DEV_PAD_GPIO_INSTANCE(id,DTR)
#define UART_DEV_DTR_GPIO_PIN(id)           UART_DEV_PAD_GPIO_PIN(id,DTR)

#define UART_DEV_DCD_PAD_INDEX(id)          UART_DEV_PAD_PAD_INDEX(id,DCD)
#define UART_DEV_DCD_GPIO_INSTANCE(id)      UART_DEV_PAD_GPIO_INSTANCE(id,DCD)
#define UART_DEV_DCD_GPIO_PIN(id)           UART_DEV_PAD_GPIO_PIN(id,DCD)

#define UART_DEV_RI_PAD_INDEX(id)           UART_DEV_PAD_PAD_INDEX(id,RI)
#define UART_DEV_RI_GPIO_INSTANCE(id)       UART_DEV_PAD_GPIO_INSTANCE(id,RI)
#define UART_DEV_RI_GPIO_PIN(id)            UART_DEV_PAD_GPIO_PIN(id,RI)
#define UART_DEV_RI_PWM_INSTANCE(id)        UART_DEV_PAD_PWM_INSTANCE(id,RI)
#define UART_DEV_RI_PWM_CLK_ID(id)          UART_DEV_PAD_PWM_CLK_ID(id,RI)
#define UART_DEV_RI_PWM_CLK_SEL(id)         UART_DEV_PAD_PWM_CLK_SEL(id,RI)

#define UART_DEV_REAL_MAXNUM            PORT_USART_MAX

/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/
typedef enum
{
    UARTD_PIS_IDLE = 0,
    UARTD_PIS_BUSY
}UartDevPadInstState_e;

typedef struct
{
    uint8_t   hasInit:4;
    uint8_t   pwmFlag:4;
    uint8_t   pinId;    /* pin@gpio */
    uint16_t  instId;
    uint32_t  data1;    /* initVal@gpio or dutyCyclePct@pwm */
    uint32_t  data2;    /* handleCb@gpio, cycleTimeMs@pwm */
}UartDevPadInst_t;

typedef struct
{
    uint8_t  hasUsed;
    uint8_t  altFunc;  /* PadMux_e */
    uint8_t  state;    /* UartDevPadInstState_e */
    uint8_t  rsvd;
    UartDevPadInst_t inst[2]; /* 0-gpio, 1-pwm */
}UartDevPadInstSel_t;

typedef struct
{
    UartDevPadInst_t     dtr;    /* in */
    UartDevPadInst_t     dcd;    /* out */
    UartDevPadInstSel_t  riSel;  /* out */
}UartDevPinMux_t;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
void uartDevGpioInit(uint8_t isAon, uint16_t padIdx, uint32_t port, uint16_t pin, uint16_t pinDir, uint16_t initVal);
int32_t uartDevRiPwmEnable(uint8_t uartIdx, uint16_t dutyCyclePct, uint32_t cycleTimeMs);
int32_t uartDevRiPwmDisable(uint8_t uartIdx);
int32_t uartDevRiGpioSet(uint8_t uartIdx, uint16_t outVal);

#ifdef __cplusplus
}
#endif
#endif
