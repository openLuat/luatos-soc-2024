/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: es8311.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: es8311 interface
*
******************************************************************************/


#ifndef _CODEC_ES8311_H
#define _CODEC_ES8311_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "hal_codec.h"

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/



// ES8311_REGISTER NAME_REG_REGISTER ADDRESS
#define ES8311_RESET_REG00              0x00  /*reset digital,csm,clock manager etc.*/

// Clock Scheme Register definition
#define ES8311_CLK_MANAGER_REG01        0x01 // select clk src for mclk, enable clock for codec 
#define ES8311_CLK_MANAGER_REG02        0x02 // clk divider and clk multiplier 
#define ES8311_CLK_MANAGER_REG03        0x03 // adc fsmode and osr  
#define ES8311_CLK_MANAGER_REG04        0x04 // dac osr 
#define ES8311_CLK_MANAGER_REG05        0x05 // clk divier for adc and dac 
#define ES8311_CLK_MANAGER_REG06        0x06 // bclk inverter and divider 
#define ES8311_CLK_MANAGER_REG07        0x07 // tri-state, lrck divider 
#define ES8311_CLK_MANAGER_REG08        0x08 // lrck divider 

// SDP
#define ES8311_SDPIN_REG09              0x09 // dac serial digital port */
#define ES8311_SDPOUT_REG0A             0x0A // adc serial digital port */

// SYSTEM
#define ES8311_SYSTEM_REG0B             0x0B // system 
#define ES8311_SYSTEM_REG0C             0x0C // system 
#define ES8311_SYSTEM_REG0D             0x0D // system, power up/down 
#define ES8311_SYSTEM_REG0E             0x0E // system, power up/down 
#define ES8311_SYSTEM_REG0F             0x0F // system, low power 
#define ES8311_SYSTEM_REG10             0x10 // system 
#define ES8311_SYSTEM_REG11             0x11 // system 
#define ES8311_SYSTEM_REG12             0x12 // system, Enable DAC 
#define ES8311_SYSTEM_REG13             0x13 // system 
#define ES8311_SYSTEM_REG14             0x14 // system, select DMIC, select analog pga gain 

// ADC
#define ES8311_ADC_REG15                0x15 // ADC, adc ramp rate, dmic sense 
#define ES8311_ADC_REG16                0x16 // ADC 
#define ES8311_ADC_REG17                0x17 // ADC, volume 
#define ES8311_ADC_REG18                0x18 // ADC, alc enable and winsize 
#define ES8311_ADC_REG19                0x19 // ADC, alc maxlevel 
#define ES8311_ADC_REG1A                0x1A // ADC, alc automute 
#define ES8311_ADC_REG1B                0x1B // ADC, alc automute, adc hpf s1 
#define ES8311_ADC_REG1C                0x1C // ADC, equalizer, hpf s2 
#define ES8311_ADC_REG1D                0x1D // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG1E                0x1E // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG1F                0x1F // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG20                0x20 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG21                0x21 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG22                0x22 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG23                0x23 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG24                0x24 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG25                0x25 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG26                0x26 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG27                0x27 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG28                0x28 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG29                0x29 // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2A                0x2A // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2B                0x2B // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2C                0x2C // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2D                0x2D // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2E                0x2E // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG2F                0x2F // ADC, 30-bit B0 coefficient for ADCEQ
#define ES8311_ADC_REG30                0x30 // ADC, 30-bit B0 coefficient for ADCEQ

// DAC
#define ES8311_DAC_REG31                0x31 // DAC, mute 
#define ES8311_DAC_REG32                0x32 // DAC, volume 
#define ES8311_DAC_REG33                0x33 // DAC, offset 
#define ES8311_DAC_REG34                0x34 // DAC, drc enable, drc winsize 
#define ES8311_DAC_REG35                0x35 // DAC, drc maxlevel, minilevel 
#define ES8311_DAC_REG37                0x37 // DAC, ramprate 

// GPIO
#define ES8311_GPIO_REG44               0x44 // GPIO, dac2adc for test 
#define ES8311_GP_REG45                 0x45 // GP CONTROL 

// CHIP
#define ES8311_CHD1_REGFD               0xFD // CHIP ID1
#define ES8311_CHD2_REGFE               0xFE // CHIP ID2
#define ES8311_CHVER_REGFF              0xFF // VERSION 
#define ES8311_CHD1_REGFD               0xFD // CHIP ID1 

#define ES8311_MAX_REGISTER             0xFF


/*----------------------------------------------------------------------------*
*                   DATA TYPE DEFINITION                                     *
*----------------------------------------------------------------------------*/

extern HalCodecFuncList_t es8311DefaultHandle;


/*----------------------------------------------------------------------------*
*                    GLOBAL FUNCTIONS DECLEARATION                           *
*----------------------------------------------------------------------------*/

 /**
  \brief    Enables or disables PA
  \param[in] enable     true/false
  \return    NULL
  \note
 */
void es8311EnablePA(bool enable);

/**
  \brief    Initialize ES8311 codec chip
  \param[in] codec_cfg  configuration of ES8311
  \return    -CODEC_EOK   -CODEC_INIT_ERR
  \note
 */ 
HalCodecSts_e es8311Init(HalCodecCfg_t *codecCfg);

 /**
  \brief    Deinitialize ES8311 codec chip
  \param[in] NULL
  \return    NULL
  \note
 */
void es8311DeInit(void);

/**
  \brief    start/stop ES8311 codec chip
  \param[in] mode codec mode
  \param[in] ctrlState start or stop decode or encode progress
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState);

/**
  \brief    Configure ES8311 codec mode and I2S interface
  \param[in] mode codec mode
  \param[in] iface I2S config
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311Config(HalCodecMode_e mode, HalCodecIface_t *iface);

/**
  \brief    Configure ES8311 DAC mute or not. Basically you can use this function to mute the output or unmute
  \param[in] enable enable(1) or disable(0)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311SetMute(HalCodecCfg_t* codecHalCfg, bool enable);

/**
  \brief    Set voice volume
  \param[in] volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311SetVolume(HalCodecCfg_t* codecHalCfg, int volume);

/**
  \brief    Get voice volume
  \param[out] *volume:  voice volume (0~100)
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311GetVolume(HalCodecCfg_t* codecHalCfg, int *volume);

/**
  \brief    Configure ES8311 I2S format
  \param[in] mod:  set ADC or DAC or both
  \param[in] cfg:   ES8388 I2S format
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311ConfigFmt(HalCodecIfaceFormat_e fmt);

/**
  \brief    Configure ES8311 data sample bits
  \param[in] mode:  set ADC or DAC or both
  \param[in] bits:  bit number of per sample
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311SetBitsPerSample(HalCodecIfaceBits_e bits);

/**
  \brief    Start ES8311 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311Start(HalCodecMode_e mode);

/**
  \brief    Stop ES8311 codec chip
  \param[in] mode:  set ADC or DAC or both
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311Stop(HalCodecMode_e mode);

/**
  \brief    Get ES8311 DAC mute status
  \param[out] mute  get mute
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311GetVoiceMute(int *mute);

/**
  \brief    Set ES8311 mic gain and volume
  \param[in] micGain db of mic gain, varies from 0~10, default is 8
  \param[in] micVolume micVolume, varies from 0~100, default is 75
  \return    -CODEC_EOK   -CODEC_ERR
  \note
 */ 
HalCodecSts_e es8311SetMicVolume(uint8_t micGain, int micVolume);

/**
 * @brief Print all ES8311 registers
 *
 * @return
 *     - void
 */
/**
  \brief    Print all ES8311 registers
  \param[in] NULL
  \return    NULL
  \note
 */ 
void es8311ReadAll();

/**
  \brief    get es8311 default config
  \param[in] NULL
  \return    HalCodecCfg_t codecCfg
  \note
 */
HalCodecCfg_t es8311GetDefaultCfg();

HalCodecSts_e es8311Resume(HalCodecMode_e mode);



#ifdef __cplusplus
}
#endif

#endif /* _CODEC_ES8311_H */

