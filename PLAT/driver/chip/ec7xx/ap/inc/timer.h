/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    timer.h
 * Description:  EC718 timer driver header file
 * History:
 *
 ****************************************************************************/

#ifndef _TIMER_EC7XX_H
#define _TIMER_EC7XX_H

#include "ec7xx.h"
#include "Driver_Common.h"

/**
  \addtogroup timer_interface_gr
  \{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** \brief List of TIMER clock source */
typedef enum
{
    TIMER_INTERNAL_CLOCK  = 0U,  /**< Internal clock */
    TIMER_EXTERNAL_CLOCK  = 1U,  /**< External clock */
} TimerClockSource_e;

/** \brief List of TIMER match value select */
typedef enum
{
    TIMER_MATCH0_SELECT    = 0U,  /**< Select Match0 */
    TIMER_MATCH1_SELECT    = 1U,  /**< Select Match1 */
    TIMER_MATCH2_SELECT    = 2U,  /**< Select Match2 */
} TimerMatchSelect_e;

/** \brief List of TIMER reload option, counter will be reloaded to init value upon reach it */
typedef enum
{
    TIMER_RELOAD_DISABLE      = 0U,  /**< Counter will run freely */
    TIMER_RELOAD_ON_MATCH0    = 1U,  /**< Counter will be reloaded on reaching match0 value */
    TIMER_RELOAD_ON_MATCH1    = 2U,  /**< Counter will be reloaded on reaching match1 value */
    TIMER_RELOAD_ON_MATCH2    = 3U,  /**< Counter will be reloaded on reaching match1 value */
} TimerReloadOption_e;

/** \brief List of Capture mode control */
typedef enum
{
    TIMER_CAPTURE_MODE_DISABLE      = 0U,  /**< Disable caputure mode */
    TIMER_CAPTURE_MODE_ENABLE       = 1U,  /**< Enable capture mode */
} TimerCaptureModeControl_e;

/** \brief List of Capture edge option */
typedef enum
{
    TIMER_CAPTURE_RISING_EDGE      = 0U,  /**< Caputure Rising edge */
    TIMER_CAPTURE_FALLING_EDGE     = 1U,  /**< Caputure falling edge */
    TIMER_CAPTURE_BOTH_EDGE        = 2U,  /**< Caputure both edges */
} TimerCaptureEdge_e;

/** \brief List of PWM stop option, controls pwm output level(high,low or holds current level)when stopped */
typedef enum
{
    TIMER_PWM_STOP_LOW      = 0U,  /**< Push pwm output low when stopped */
    TIMER_PWM_STOP_HIGH     = 1U,  /**< Push pwm output high when stopped */
    TIMER_PWM_STOP_HOLD     = 2U,  /**< Hold current output level when stopped */
} TimerPwmStopOption_e;

/** \brief PWM configuration structure */
typedef struct
{
    uint32_t pwmFreq_HZ;             /**< PWM signal frequency in HZ */
    uint32_t srcClock_HZ;            /**< TIMER counter clock in HZ */
    uint32_t dutyCyclePercent;       /**< PWM pulse width, the valid range is 0 to 100 */
    TimerPwmStopOption_e stopOption; /**< PWM stop option, controls pwm output level(high,low or holds current level) when stopped */
} TimerPwmConfig_t;

/** \brief TIMER configuration structure */
typedef struct
{
    TimerClockSource_e           clockSource;          /**< Clock source */
    TimerReloadOption_e          reloadOption;         /**< Reload option */
    TimerCaptureModeControl_e    captureMode;          /**< Capture mode enable or not */
    TimerCaptureEdge_e           captureEdge;          /**< Capture edge option, valid only when capture mode is enabled */
    uint32_t                     initValue;            /**< Counter init value */
    uint32_t                     match0;               /**< Match0 value */
    uint32_t                     match1;               /**< Match1 value */
    uint32_t                     match2;               /**< Match2 value */
} TimerConfig_t;

/** \brief TIMER capture result structure */
typedef struct
{
    uint32_t              counterValue;               /**< counter value when captured */
    TimerCaptureEdge_e    capturedEdge;               /**< Edge type that triggers this capture */
}TimerCaptureResult_t;

/** \brief TIMER interrupt configuration */
typedef enum
{
    TIMER_INTERRUPT_DISABLE     = 0U,  /**< Disable interrupt */
    TIMER_INTERRUPT_LEVEL       = 1U,  /**< Level interrupt, a high level interrupt signal is generated */
    TIMER_INTERRUPT_PULSE       = 2U,  /**< Pulse interrupt, don't need to clear interrupt flag for this kind of interrupt type,
	                                        so you can't be able to get the interrupt sources when multiply interrupts are enabled */
} TimerInterruptConfig_e;

/** \brief List of TIMER interrupts */
typedef enum
{
    TIMER_MATCH0_INTERRUPT  = 0,       /**< Match0 interrupt */
    TIMER_MATCH1_INTERRUPT  = 1,       /**< Match1 interrupt */
    TIMER_MATCH2_INTERRUPT  = 2,       /**< Match2 interrupt */
    TIMER_CAPTURE_INTERRUPT = 3,       /**< Capture interrupt */
} TimerInterruptSource_e;

/** \brief List of TIMER interrupt flags */
typedef enum
{
    TIMER_MATCH0_INTERRUPT_FLAG  = TIMER_TSR_ICLR_0_Msk,  /**< Match0 interrupt flag */
    TIMER_MATCH1_INTERRUPT_FLAG  = TIMER_TSR_ICLR_1_Msk,  /**< Match1 interrupt flag */
    TIMER_MATCH2_INTERRUPT_FLAG  = TIMER_TSR_ICLR_2_Msk,  /**< Match2 interrupt flag */
    TIMER_CAPTURE_INTERRUPT_FLAG = TIMER_TSR_CAPTURE_Msk, /**< Capture interrupt flag */
} TimerInterruptFlags_e;


/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/** \name TIMER Driver Initialization */
/** \{ */

/**
  \fn    void TIMER_driverInit(void);
  \brief Intialize TIMER driver internal data, must be called before any other APIs
 */
void TIMER_driverInit(void);

/** \} */

/** \name TIMER Configuration */
/** \{ */

/**
  \fn    void TIMER_getDefaultConfig(TimerConfig_t *config)
  \brief Gets the TIMER default configuartion.
         This function sets the configuration structure to default values as below:
  \code
         config->clockSource = TIMER_INTERNAL_CLOCK;
         config->reloadOption = TIMER_RELOAD_ON_MATCH1;
         config->captureMode = TIMER_CAPTURE_MODE_DISABLE;
         config->captureEdge = TIMER_CAPTURE_BOTH_EDGE;
         config->initValue = 0;
         config->match0 = 0x10000 >> 1U;
         config->match1 = 0x10000;
         config->match2 = 0xFFFFFFFFU;
  \endcode

  \param[in] config      Pointer to TIMER configuration structure
 */
void TIMER_getDefaultConfig(TimerConfig_t *config);

/**
  \fn    void TIMER_init(uint32_t instance, const TimerConfig_t *config)
  \brief Intialize TIMER
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] config      Pointer to TIMER configuration
  \note      PWM out is disabled after this function's call, use \ref TIMER_setupPwm function to eable PWM
 */
void TIMER_init(uint32_t instance, const TimerConfig_t *config);

/**
  \fn    void TIMER_deInit(uint32_t instance)
  \brief Deintialize TIMER
  \param[in] instance    TIMER instance number (0, 1, ...)
 */
void TIMER_deInit(uint32_t instance);

/**
  \fn    void TIMER_setMatch(uint32_t instance, TimerMatchSelect_e matchNum, uint32_t matchValue)
  \brief Sets one of TIMER match values
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] matchNum    TIMER match select
  \param[in] matchValue  TIMER match value
 */
void TIMER_setMatch(uint32_t instance, TimerMatchSelect_e matchNum, uint32_t matchValue);

/**
  \fn    void TIMER_setInitValue(uint32_t instance, uint32_t initValue)
  \brief Sets TIMER counter initial value
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] initValue   TIMER initial value
 */
void TIMER_setInitValue(uint32_t instance, uint32_t initValue);

/**
  \fn    void TIMER_setReloadOption(uint32_t instance, TimerReloadOption_e option)
  \brief Sets TIMER counter reload option
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] option      TIMER counter reload option
 */
void TIMER_setReloadOption(uint32_t instance, TimerReloadOption_e option);

/**
  \fn    void TIMER_setExternalEdgeCountType(uint32_t instance, TimerCaptureEdge_e type)
  \brief Sets edge type of external signal to be counted, that's, counting the number of TIMER_EXTERNAL_CLOCK edges(rising/falling or both)
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] type        Edge type to be set
 */
void TIMER_setExternalEdgeCountType(uint32_t instance, TimerCaptureEdge_e type);

/**
  \fn    void TIMER_setExternalClockInput(uint32_t instance, uint32_t gpioPort, uint32_t gpioPin, bool onOff)
  \brief Sets external clock source input pin
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] gpioPort    GPIO instance
  \param[in] gpioPin     GPIO pin index
  \param[in] onOff       true for enabling external clock input, false for disablling external clock input
 */
void TIMER_setExternalClockInput(uint32_t instance, uint32_t gpioPort, uint32_t gpioPin, bool onOff);

/** \} */

/** \name TIMER Counter */
/** \{ */

/**
  \fn    void TIMER_start(uint32_t instance)
  \brief Starts TIMER counter
  \param[in] instance    TIMER instance number (0, 1, ...)
 */
void TIMER_start(uint32_t instance);

/**
  \fn    void TIMER_stop(uint32_t instance)
  \brief Stops TIMER counter
  \param[in] instance    TIMER instance number (0, 1, ...)
 */
void TIMER_stop(uint32_t instance);

/**
  \fn    uint32_t TIMER_getCount(uint32_t instance)
  \brief Reads current TIMER counter value
  \param[in] instance    TIMER instance number (0, 1, ...)
  \return                current TIMER counter value
 */
uint32_t TIMER_getCount(uint32_t instance);

/**
  \fn    void TIMER_getCaptureResult(uint32_t instance, TimerCaptureResult_t* result)
  \brief Reads capture result
  \param[in]  instance    TIMER instance number (0, 1, ...)
  \parma[out] result      pointer to buffer for capture result
 */
void TIMER_getCaptureResult(uint32_t instance, TimerCaptureResult_t* result);

/** \} */

/** \name TIMER PWM */
/** \{ */

/**
  \fn    int32_t TIMER_setupPwm(uint32_t instance, const TimerPwmConfig_t *config)
  \brief Configures the PWM signals period, mode, etc.
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] config      Pointer to PWM parameter
  \return    ARM_DRIVER_OK if the PWM setup is successful
             ARM_DRIVER_ERROR_PARAMETER on parameter check failure
 */
int32_t TIMER_setupPwm(uint32_t instance, const TimerPwmConfig_t *config);

/**
  \fn    void TIMER_updatePwmDutyCycle(uint32_t instance, uint32_t dutyCyclePercent)
  \brief Updates the duty cycle of PWM signal
  \param[in] instance              TIMER instance number (0, 1, ...)
  \param[in] dutyCyclePercent      New PWM pulse width, value shall be between 0 to 100,
                                   if the value exceeds 100, dutyCyclePercent is set to 100.
 */
void TIMER_updatePwmDutyCycle(uint32_t instance, uint32_t dutyCyclePercent);

/** \} */

/** \name TIMER Interrupt */
/** \{ */

/**
  \fn    void TIMER_interruptConfig(uint32_t instance, TimerInterruptSource_e source, TimerInterruptConfig_e config)
  \brief Configures the selected TIMER interrupt
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] source      TIMER interrupt source
  \param[in] config      TIMER interrupt configuration
  */
void TIMER_interruptConfig(uint32_t instance, TimerInterruptSource_e source, TimerInterruptConfig_e config);

/**
  \fn    TimerInterruptConfig_e TIMER_getInterruptConfig(uint32_t instance, TimerInterruptSource_e source)
  \brief Gets current configuration of the selected TIMER interrupt
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] source      TIMER interrupt source
  \return                Current TIMER interrupt configuration
  */
TimerInterruptConfig_e TIMER_getInterruptConfig(uint32_t instance, TimerInterruptSource_e source);

/**
  \fn    uint32_t TIMER_getInterruptFlags(uint32_t instance)
  \brief Reads TIMER interrupt status flags
  \param[in] instance    TIMER instance number (0, 1, ...)
  \return    Interrupt flags. This is the logical OR of members of the
             enumeration \ref TimerInterruptFlags_e
 */
uint32_t TIMER_getInterruptFlags(uint32_t instance);

/**
  \fn    void TIMER_clearInterruptFlags(uint32_t instance, uint32_t mask)
  \brief Clears TIMER interrupt flags
  \param[in] instance    TIMER instance number (0, 1, ...)
  \param[in] mask        Interrupt flags to clear. This is a logic OR of members of the
                         enumeration \ref TimerInterruptFlags_e
 */
void TIMER_clearInterruptFlags(uint32_t instance, uint32_t mask);

/**
  \fn    void TIMER_netlightEnable(uint32_t instance)
  \brief Set Netlight Enable, called by user in bsp_custom.c to define specific timer instance for netlight
  \param[in] instance    TIMER instance number (0, 1, ...)
 */
void TIMER_netlightEnable(uint8_t instance);

/** \} */

/** \} */

#ifdef __cplusplus
}
#endif

#endif /* _TIMER_EC7XX_H */
