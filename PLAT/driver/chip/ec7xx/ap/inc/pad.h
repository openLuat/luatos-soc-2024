/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    pad.h
 * Description:  EC718 pad driver header file
 * History:
 *
 ****************************************************************************/

#ifndef _PAD_EC7XX_H_
#define _PAD_EC7XX_H_

#include "ec7xx.h"
#include "Driver_Common.h"


/**
  \addtogroup pad_interface_gr
  \{
 */

/*******************************************************************************
 * Definitions
 ******************************************************************************/

/** \brief PAD pin mux selection */
typedef enum
{
    PAD_MUX_ALT0 = 0U,             /**< Chip-specific */
    PAD_MUX_ALT1 = 1U,             /**< Chip-specific */
    PAD_MUX_ALT2 = 2U,             /**< Chip-specific */
    PAD_MUX_ALT3 = 3U,             /**< Chip-specific */
    PAD_MUX_ALT4 = 4U,             /**< Chip-specific */
    PAD_MUX_ALT5 = 5U,             /**< Chip-specific */
    PAD_MUX_ALT6 = 6U,             /**< Chip-specific */
    PAD_MUX_ALT7 = 7U,             /**< Chip-specific */
} PadMux_e;

/** \brief Internal pull-up resistor configuration */
typedef enum
{
    PAD_PULL_UP_DISABLE = 0U,      /**< Internal pull-up resistor is disabled */
    PAD_PULL_UP_ENABLE  = 1U,      /**< Internal pull-up resistor is enabled */
} PadPullUp_e;

/** \brief Internal pull-down resistor configuration */
typedef enum
{
    PAD_PULL_DOWN_DISABLE = 0U,      /**< Internal pull-down resistor is disabled */
    PAD_PULL_DOWN_ENABLE  = 1U,      /**< Internal pull-down resistor is enabled */
} PadPullDown_e;

/** \brief Pull feature selection */
typedef enum
{
    PAD_PULL_AUTO      = 0U,      /**< Pull up/down is controlled by muxed alt function signal, i.e. I2C SCL */
    PAD_PULL_INTERNAL  = 1U,      /**< Use internal pull-up/down resistor */
} PadPullSel_e;

/** \brief Input buffer enable/disable */
typedef enum
{
    PAD_INPUT_CONTROL_AUTO   = 0U,     /**< Input is controlled by muxed alt function signal, i.e. I2C SCL */
    PAD_INPUT_CONTROL_SW_ON  = 1U,     /**< Input is forced enable */
} PadInputControl_e;

/** \brief Output enable/disable */
typedef enum
{
    PAD_OUTPUT_CONTROL_AUTO   = 0U,     /**< Output is controlled by muxed alt function signal, i.e. I2C SCL */
    PAD_OUTPUT_CONTROL_SW_ON  = 1U,     /**< Output is forced enable */
} PadOutputControl_e;

/** \brief Configures pull feature */
typedef enum
{
    PAD_INTERNAL_PULL_UP   = 0U,  /**< select internal pull up */
    PAD_INTERNAL_PULL_DOWN = 1U,  /**< select internal pull down */
    PAD_AUTO_PULL          = 2U,  /**< Pull up/down is controlled by muxed alt function signal, i.e. I2C SCL */
} PadPullConfig_e;

#if defined CHIP_EC718
/** \brief Configures slew rate feature */
typedef enum
{
    PAD_SLEW_RATE_LOW   = 0U,  /**< Slew rate is set to low */
    PAD_SLEW_RATE_HIGH  = 1U,  /**< Slew rate is set to high */
} PadSlewRate_e;

/** \brief Drive strength configuration */
typedef enum
{
    PAD_DRIVE_STRENGTH_LOW  = 0U,      /**< Drive strength is low */
    PAD_DRIVE_STRENGTH_HIGH = 1U,      /**< Drive strength is high */
} PadDriveStrength_e;

/** \brief PAD configuration structure */
typedef struct
{
    uint32_t                                    : 4;
    uint32_t         mux                        : 3;   /**< Pad mux configuration */
    uint32_t                                    : 1;
    uint32_t         pullUpEnable               : 1;   /**< Enable pull-up */
    uint32_t         pullDownEnable             : 1;   /**< Enable pull-down */
    uint32_t         pullSelect                 : 1;   /**< Pull select, external or internal control */
    uint32_t         inputForceDisable          : 1;   /**< Force to disable input or not, if true, input path is cut off, otherwise, it's controlled by inputControl bit */
    uint32_t                                    : 1;
    uint32_t         inputControl               : 1;   /**< Input enable, 'force' on or controlled by muxed alt function signal, i.e. I2C SCL */
    uint32_t         outputControl              : 1;   /**< Output enable, 'force' on or controlled by muxed alt function signal, i.e. I2C SCL */
    uint32_t         outputForceDisable         : 1;   /**< Force to disable output or not, if true, output path is cut off, otherwise, it's controlled by outputControl bit */
    uint32_t                                    : 1;
    uint32_t         driveStrength              : 1;
    uint32_t                                    : 12;
    uint32_t         swOutputValue              : 1;
    uint32_t         swOutputEnable             : 1;  /** Control to overwrite function signal's output or not, if set to ture, the pad's output is determined by swOutputValue bit */
} PadConfig_t;
#endif

#if defined CHIP_EC716
/** \brief Configures slew rate feature */
typedef enum
{
    PAD_SLEW_RATE_LOW   = 0U,  /**< Slew rate is set to low */
    PAD_SLEW_RATE_HIGH  = 1U,  /**< Slew rate is set to high */
} PadSlewRate_e;

/** \brief Drive strength configuration */
typedef enum
{
    PAD_DRIVE_STRENGTH_0  = 0U,      /**< Drive strength level 0 */
    PAD_DRIVE_STRENGTH_1  = 1U,      /**< Drive strength level 1 */
    PAD_DRIVE_STRENGTH_2  = 2U,      /**< Drive strength level 2 */
    PAD_DRIVE_STRENGTH_3  = 3U,      /**< Drive strength level 3 */
    PAD_DRIVE_STRENGTH_4  = 4U,      /**< Drive strength level 4 */
    PAD_DRIVE_STRENGTH_5  = 5U,      /**< Drive strength level 5 */
    PAD_DRIVE_STRENGTH_6  = 6U,      /**< Drive strength level 6 */
    PAD_DRIVE_STRENGTH_7  = 7U,      /**< Drive strength level 7 */
} PadDriveStrength_e;

/** \brief PAD configuration structure */
typedef struct
{
    uint32_t                                    : 4;
    uint32_t         mux                        : 3;   /**< Pad mux configuration */
    uint32_t                                    : 1;
    uint32_t         pullUpEnable               : 1;   /**< Enable pull-up */
    uint32_t         pullDownEnable             : 1;   /**< Enable pull-down */
    uint32_t         pullSelect                 : 1;   /**< Pull select, external or internal control */
    uint32_t         inputForceDisable          : 1;   /**< Force to disable input or not, if true, input path is cut off, otherwise, it's controlled by inputControl bit */
    uint32_t         schmittTriggerEnable       : 1;   /**< Schmitt trigger enable or not, if true, enable this feature */
    uint32_t         inputControl               : 1;   /**< Input enable, 'force' on or controlled by muxed alt function signal, i.e. I2C SCL */
    uint32_t         outputControl              : 1;   /**< Output enable, 'force' on or controlled by muxed alt function signal, i.e. I2C SCL */
    uint32_t         outputForceDisable         : 1;   /**< Force to disable output or not, if true, output path is cut off, otherwise, it's controlled by outputControl bit */
    uint32_t         slewRate                   : 1;   /**< Slew rate setting */
    uint32_t         driveStrength              : 3;   /**< Driver strength setting */
    uint32_t                                    : 10;
    uint32_t         swOutputValue              : 1;
    uint32_t         swOutputEnable             : 1;  /** Control to overwrite function signal's output or not, if set to ture, the pad's output is determined by swOutputValue bit */
} PadConfig_t;
#endif

/*******************************************************************************
 * API
 ******************************************************************************/

#if defined(__cplusplus)
extern "C" {
#endif

/** \name PAD Driver Initialization */
/** \{ */

/**
  \fn    void PAD_driverInit(void);
  \brief Intialize PAD driver internal data, must be called before any other APIs
 */
void PAD_driverInit(void);

/**
  \fn    void PAD_driverInit(void);
  \brief De-Intialize PAD driver, disable PAD clock and perform some clearups
 */
void PAD_driverDeInit(void);

/** \} */

/** \name Configuration */
/* \{ */

/**
  \fn    void PAD_getDefaultConfig(PadConfig_t *config)
  \brief Gets the PAD default configuartion
         This function sets the configuration structure to default values as below:
  \code
         config->mux = PAD_MUX_ALT0;
         config->inputControl = PAD_INPUT_CONTROL_AUTO;
         config->inputForceDisable = 0;
         config->outputForceDisable = 0;
         config->outputControl = PAD_OUTPUT_CONTROL_AUTO;
         config->swOutputEnable = 0;
         config->swOutputValue = 0;
         config->pullSelect = PAD_PULL_AUTO;
         config->pullUpEnable = PAD_PULL_UP_DISABLE;
         config->pullDownEnable = PAD_PULL_DOWN_DISABLE;
         config->driveStrength = PAD_DRIVE_STRENGTH_HIGH;
  \endcode
  \param config Pointer to PAD configuration structure
 */
void PAD_getDefaultConfig(PadConfig_t *config);

/**
  \fn    void PAD_setPinConfig(uint32_t pin, const PadConfig_t *config)
  \brief Sets the pad PCR register
  \param pin       PAD pin number
  \param config    Pointer to PAD configuration structure
 */
void PAD_setPinConfig(uint32_t paddr, const PadConfig_t *config);

/**
  \fn    void PAD_setPinMux(uint32_t pin, PadMux_e mux)
  \brief Configures pin mux
  \param pin       PAD pin number
  \param mux       pin signal source selection
 */
void PAD_setPinMux(uint32_t paddr, PadMux_e mux);

/**
  \fn    void PAD_setPinPullConfig(uint32_t pin, PadPullConfig_e config)
  \brief Configures pin's pull feature
  \param pin       PAD pin number
  \param config    PAD pin pull configuration
 */
void PAD_setPinPullConfig(uint32_t paddr, PadPullConfig_e config);
/**
  \fn    void PAD_setInputOutputDisable(uint32_t paddr);
  
  \brief set specific pad as no input and no output
  \param pin       PAD pin number
 */
void PAD_setInputOutputDisable(uint32_t paddr);

/** \} */

/** \}*/

#if defined(__cplusplus)
}
#endif

#endif /* _PAD_EC718_H_ */
