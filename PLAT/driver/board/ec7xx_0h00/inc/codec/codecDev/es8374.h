/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: es8374.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: es8374 interface
*
******************************************************************************/


#ifndef _CODEC_ES8374_H
#define _CODEC_ES8374_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "codecDrv.h"

#ifdef __cplusplus
extern "C" {
#endif

extern HalCodecFuncList_t es8374DefaultHandle;


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/



// ES8374_REG_ISTER NAME_REG_REGISTER ADDRESS
#define ES8374_REG_00               0x00
#define ES8374_REG_01               0x01
#define ES8374_REG_02               0x02
#define ES8374_REG_03               0x03
#define ES8374_REG_04               0x04
#define ES8374_REG_05               0x05
#define ES8374_REG_06               0x06
#define ES8374_REG_07               0x07
#define ES8374_REG_08               0x08
#define ES8374_REG_09               0x09
#define ES8374_REG_0A               0x0A
#define ES8374_REG_0B               0x0B
#define ES8374_REG_0C               0x0C
#define ES8374_REG_0D               0x0D 
#define ES8374_REG_0E               0x0E 
#define ES8374_REG_0F               0x0F
#define ES8374_REG_10               0x10
#define ES8374_REG_11               0x11
#define ES8374_REG_12               0x12
#define ES8374_REG_13               0x13
#define ES8374_REG_14               0x14
#define ES8374_REG_15               0x15
#define ES8374_REG_16               0x16
#define ES8374_REG_17               0x17
#define ES8374_REG_18               0x18
#define ES8374_REG_19               0x19
#define ES8374_REG_1A               0x1A
#define ES8374_REG_1B               0x1B
#define ES8374_REG_1C               0x1C
#define ES8374_REG_1D               0x1D
#define ES8374_REG_1E               0x1E
#define ES8374_REG_1F               0x1F
#define ES8374_REG_20               0x20
#define ES8374_REG_21               0x21
#define ES8374_REG_22               0x22
#define ES8374_REG_23               0x23
#define ES8374_REG_24               0x24
#define ES8374_REG_25               0x25
#define ES8374_REG_26               0x26
#define ES8374_REG_27               0x27
#define ES8374_REG_28               0x28
#define ES8374_REG_29               0x29
#define ES8374_REG_2A               0x2A
#define ES8374_REG_2B               0x2B
#define ES8374_REG_2C               0x2C
#define ES8374_REG_2D               0x2D
#define ES8374_REG_2E               0x2E
#define ES8374_REG_2F               0x2F
#define ES8374_REG_30               0x30
#define ES8374_REG_31               0x31
#define ES8374_REG_32               0x32
#define ES8374_REG_33               0x33
#define ES8374_REG_34               0x34
#define ES8374_REG_35               0x35
#define ES8374_REG_36               0x36
#define ES8374_REG_37               0x37
#define ES8374_REG_38               0x38
#define ES8374_REG_6D               0x6D
#define ES8374_REG_6F               0x6F
#define ES8374_REG_72               0x72


/*----------------------------------------------------------------------------*
*                   DATA TYPE DEFINITION                                     *
*----------------------------------------------------------------------------*/

extern HalCodecFuncList_t es8374DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void es8374EnablePA(bool enable);

/**
  \brief    Initialize ES8374 codec chip
  \param[in] codec_cfg  configuration of ES8374
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e es8374Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize ES8374 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void es8374DeInit(void);

/**
  \brief    start/stop ES8374 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure ES8374 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure ES8374 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374SetMute(HalCodecCfg_t* codecHalCfg, bool enable);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure ES8374 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure ES8374 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start ES8374 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374Start(HalCodecModule_e mode);

/**
  \brief    Stop ES8374 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374Stop(HalCodecModule_e mode);

/**
  \brief    Get ES8374 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374GetVoiceMute(uint8_t *mute);

/**
  \brief    Set ES8374 mic gain and volume
  \param[in] not used
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8374SetMicVolume(HalCodecCfg_t* codecHalCfg, uint8_t micGain, int micVolume);

/**
 * @brief Print all ES8374 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all ES8374 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void es8374ReadAll();

/**
  \brief    get es8374 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t es8374GetDefaultCfg();

HalCodecSts_e es8374Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_ES8374_H */

