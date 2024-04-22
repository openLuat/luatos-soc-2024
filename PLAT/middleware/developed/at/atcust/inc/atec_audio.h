/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: atec_fwupd.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/
#ifndef __ATEC_AUDIO_H__
#define __ATEC_AUDIO_H__

#include "at_util.h"

#define ATC_ECADCFG_RSP_BUFF_MIN                   250

#define ATC_ECADCFG_SPH_EQ_LEN_MAX                 210
#define ATC_ECADCFG_SPH_EQ_CONTINUE                     0x88


#define ATC_ECADCFG_DIRECT_VAL_TX                   0
#define ATC_ECADCFG_DIRECT_VAL_RX                   1

#define ATC_ECADCFG_0_MODE_VAL_MIN                   0
#define ATC_ECADCFG_0_MODE_VAL_MAX                   1
#define ATC_ECADCFG_0_MODE_VAL_DEF                   0

#define ATC_ECADCFG_1_DIR_VAL_MIN                   0
#define ATC_ECADCFG_1_DIR_VAL_MAX                   1
#define ATC_ECADCFG_1_DIR_VAL_DEF                   0

#define ATC_ECADCFG_2_DEVICE_VAL_MIN                   0
#define ATC_ECADCFG_2_DEVICE_VAL_MAX                   3
#define ATC_ECADCFG_2_DEVICE_VAL_DEF                   0

#define ATC_ECADCFG_3_MODULE_VAL_MIN                   0
#define ATC_ECADCFG_3_MODULE_VAL_MAX                   6
#define ATC_ECADCFG_3_MODULE_VAL_DEF                   0

#define ATC_ECADCFG_4_PARA_STR_LEN                   (210*2)
#define ATC_ECADCFG_4_PARA_STR_DEF                   NULL

#define ATC_ECADCFG_4_LOG_CTRL_STR_LEN                   (16)
#define ATC_ECADCFG_4_LOG_CTRL_STR_DEF                   NULL

#define ATC_ECADCFG_5_VAL_MIN                   0
#define ATC_ECADCFG_5_VAL_MAX                   1
#define ATC_ECADCFG_5_VAL_DEF                   0


#define ATC_ECADSPHCFG_RSP_BUFF_MIN                   128

#define ATC_ECADSPHCFG_0_PARA_STR_LEN                   256
#define ATC_ECADSPHCFG_0_PARA_STR_DEF                   NULL

#define ATC_ECADSPHCFG_1_VAL_MIN                   0
#define ATC_ECADSPHCFG_1_VAL_MAX                   1
#define ATC_ECADSPHCFG_1_VAL_DEF                   0

#define ATC_ECADSPHCFG_2_VAL_MIN                   (-100)
#define ATC_ECADSPHCFG_2_VAL_MAX                   1
#define ATC_ECADSPHCFG_2_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_VAL_MIN                   0
#define ATC_ECADSPHCFG_3_VAL_MAX                   1
#define ATC_ECADSPHCFG_3_VAL_DEF                   0

#define ATC_ECADSPHCFG_4_VAL_MIN                   0
#define ATC_ECADSPHCFG_4_VAL_MAX                   1
#define ATC_ECADSPHCFG_4_VAL_DEF                   0

#define ATC_ECADSPHCFG_5_VAL_MIN                   0
#define ATC_ECADSPHCFG_5_VAL_MAX                   1
#define ATC_ECADSPHCFG_5_VAL_DEF                   0

#define ATC_ECADSPHCFG_6_VAL_MIN                   0
#define ATC_ECADSPHCFG_6_VAL_MAX                   1
#define ATC_ECADSPHCFG_6_VAL_DEF                   0

#define ATC_ECADSPHCFG_7_VAL_MIN                   0
#define ATC_ECADSPHCFG_7_VAL_MAX                   1
#define ATC_ECADSPHCFG_7_VAL_DEF                   0

#define ATC_ECADSPHCFG_8_VAL_MIN                   0
#define ATC_ECADSPHCFG_8_VAL_MAX                   1
#define ATC_ECADSPHCFG_8_VAL_DEF                   0

#define ATC_ECADSPHCFG_9_VAL_MIN                   0
#define ATC_ECADSPHCFG_9_VAL_MAX                   1
#define ATC_ECADSPHCFG_9_VAL_DEF                   0

#define ATC_ECAD_CODEC_1_IS_DMIC_VAL_MIN                   0
#define ATC_ECAD_CODEC_1_IS_DMIC_VAL_MAX                   1
#define ATC_ECAD_CODEC_1_IS_DMIC_VAL_DEF                   0

#define ATC_ECAD_CODEC_2_IS_EX_PA_VAL_MIN                   0
#define ATC_ECAD_CODEC_2_IS_EX_PA_VAL_MAX                   1
#define ATC_ECAD_CODEC_2_IS_EX_PA_VAL_DEF                   1

#define ATC_ECAD_CODEC_3_EX_PA_GAIN_VAL_MIN                   0
#define ATC_ECAD_CODEC_3_EX_PA_GAIN_VAL_MAX                   100
#define ATC_ECAD_CODEC_3_EX_PA_GAIN_VAL_DEF                   10

#define ATC_ECAD_CODEC_4_TX_DIG_GAIN_VAL_MIN                   0
#define ATC_ECAD_CODEC_4_TX_DIG_GAIN_VAL_MAX                   100
#define ATC_ECAD_CODEC_4_TX_DIG_GAIN_VAL_DEF                   1

#define ATC_ECAD_CODEC_5_TX_ANA_GAIN_VAL_MIN                   0
#define ATC_ECAD_CODEC_5_TX_ANA_GAIN_VAL_MAX                   100
#define ATC_ECAD_CODEC_5_TX_ANA_GAIN_VAL_DEF                   1

#define ATC_ECAD_CODEC_6_RX_DIG_GAIN0_VAL_MIN                   0
#define ATC_ECAD_CODEC_6_RX_DIG_GAIN0_VAL_MAX                   100
#define ATC_ECAD_CODEC_6_RX_DIG_GAIN0_VAL_DEF                   0

#define ATC_ECAD_CODEC_7_RX_ANA_GAIN0_VAL_MIN                   0
#define ATC_ECAD_CODEC_7_RX_ANA_GAIN0_VAL_MAX                   100
#define ATC_ECAD_CODEC_7_RX_ANA_GAIN0_VAL_DEF                   0

#define ATC_ECAD_CODEC_8_RX_DIG_GAIN50_VAL_MIN                   0
#define ATC_ECAD_CODEC_8_RX_DIG_GAIN50_VAL_MAX                   100
#define ATC_ECAD_CODEC_8_RX_DIG_GAIN50_VAL_DEF                   0

#define ATC_ECAD_CODEC_9_RX_ANA_GAIN50_VAL_MIN                   0
#define ATC_ECAD_CODEC_9_RX_ANA_GAIN50_VAL_MAX                   100
#define ATC_ECAD_CODEC_9_RX_ANA_GAIN50_VAL_DEF                   0

#define ATC_ECAD_CODEC_10_RX_DIG_GAIN100_VAL_MIN                   0
#define ATC_ECAD_CODEC_10_RX_DIG_GAIN100_VAL_MAX                   100
#define ATC_ECAD_CODEC_10_RX_DIG_GAIN100_VAL_DEF                   0


#define ATC_ECAD_CODEC_11_RX_ANA_GAIN100_VAL_MIN                   0
#define ATC_ECAD_CODEC_11_RX_ANA_GAIN100_VAL_MAX                   100
#define ATC_ECAD_CODEC_11_RX_ANA_GAIN100_VAL_DEF                   0

#define ATC_ECAD_AMR_1_MODE_VAL_MIN                   0
#define ATC_ECAD_AMR_1_MODE_VAL_MAX                   10
#define ATC_ECAD_AMR_1_MODE_VAL_DEF                   0

#define ATC_ECAD_AMR_2_DEVICE_VAL_MIN                   0
#define ATC_ECAD_AMR_2_DEVICE_VAL_MAX                   100
#define ATC_ECAD_AMR_2_DEVICE_VAL_DEF                   40

#define ATC_ECADSPHCFG_1_BYPASS_VAL_MIN                   0
#define ATC_ECADSPHCFG_1_BYPASS_VAL_MAX                   1
#define ATC_ECADSPHCFG_1_BYPASS_VAL_DEF                   0

#define ATC_ECADSPHCFG_2_DELAY_VAL_MIN                   0
#define ATC_ECADSPHCFG_2_DELAY_VAL_MAX                   30
#define ATC_ECADSPHCFG_2_DELAY_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_CNGMODE_VAL_MIN                   0
#define ATC_ECADSPHCFG_3_CNGMODE_VAL_MAX                   3
#define ATC_ECADSPHCFG_3_CNGMODE_VAL_DEF                   0

#define ATC_ECADSPHCFG_4_ECHOMODE_VAL_MIN                   0
#define ATC_ECADSPHCFG_4_ECHOMODE_VAL_MAX                   6
#define ATC_ECADSPHCFG_4_ECHOMODE_VAL_DEF                   3

#define ATC_ECADSPHCFG_5_NLPFLAG_VAL_MIN                   0
#define ATC_ECADSPHCFG_5_NLPFLAG_VAL_MAX                   1
#define ATC_ECADSPHCFG_5_NLPFLAG_VAL_DEF                   0

#define ATC_ECADSPHCFG_2_MODE_VAL_MIN                   0
#define ATC_ECADSPHCFG_2_MODE_VAL_MAX                   3
#define ATC_ECADSPHCFG_2_MODE_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_EQBYPASS_VAL_MIN                   0
#define ATC_ECADSPHCFG_3_EQBYPASS_VAL_MAX                   1
#define ATC_ECADSPHCFG_3_EQBYPASS_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_PARA_STR_LEN                   300
#define ATC_ECADSPHCFG_3_PARA_STR_DEF                   NULL

#define ATC_ECADSPHCFG_2_COMP_THRESHOLD_VAL_MIN                   (-3840)
#define ATC_ECADSPHCFG_2_COMP_THRESHOLD_VAL_MAX                   (-320)
#define ATC_ECADSPHCFG_2_COMP_THRESHOLD_VAL_DEF                   (-960)

#define ATC_ECADSPHCFG_3_COMP_RATIO_VAL_MIN                   (-500)
#define ATC_ECADSPHCFG_3_COMP_RATIO_VAL_MAX                   (50)
#define ATC_ECADSPHCFG_3_COMP_RATIO_VAL_DEF                   (32)

#define ATC_ECADSPHCFG_4_EXPAND_THRESHOLD_VAL_MIN                   (-3840)
#define ATC_ECADSPHCFG_4_EXPAND_THRESHOLD_VAL_MAX                   (-320)
#define ATC_ECADSPHCFG_4_EXPAND_THRESHOLD_VAL_DEF                   (-2880)

#define ATC_ECADSPHCFG_5_EXPAND_RATIO_VAL_MIN                   (-500)
#define ATC_ECADSPHCFG_5_EXPAND_RATIO_VAL_MAX                   (50)
#define ATC_ECADSPHCFG_5_EXPAND_RATIO_VAL_DEF                   (-51)

#define ATC_ECADSPHCFG_6_ATTACK_TIME_VAL_MIN                   10000
#define ATC_ECADSPHCFG_6_ATTACK_TIME_VAL_MAX                   20000
#define ATC_ECADSPHCFG_6_ATTACK_TIME_VAL_DEF                   14459

#define ATC_ECADSPHCFG_7_REL_TIME_VAL_MIN                   10000
#define ATC_ECADSPHCFG_7_REL_TIME_VAL_MAX                   20000
#define ATC_ECADSPHCFG_7_REL_TIME_VAL_DEF                   16046

#define ATC_ECADSPHCFG_8_MAKEUP_GAIN_VAL_MIN                   0
#define ATC_ECADSPHCFG_8_MAKEUP_GAIN_VAL_MAX                   10
#define ATC_ECADSPHCFG_8_MAKEUP_GAIN_VAL_DEF                   1

#define ATC_ECADSPHCFG_2_TARGET_LEVEL_VAL_MIN                   0
#define ATC_ECADSPHCFG_2_TARGET_LEVEL_VAL_MAX                   30
#define ATC_ECADSPHCFG_2_TARGET_LEVEL_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_COMP_GAIN_VAL_MIN                   3
#define ATC_ECADSPHCFG_3_COMP_GAIN_VAL_MAX                   100
#define ATC_ECADSPHCFG_3_COMP_GAIN_VAL_DEF                   6

#define ATC_ECADSPHCFG_4_LIMITER_EN_VAL_MIN                   1
#define ATC_ECADSPHCFG_4_LIMITER_EN_VAL_MAX                   9
#define ATC_ECADSPHCFG_4_LIMITER_EN_VAL_DEF                   1

#define ATC_ECADSPHCFG_2_GAIN_VAL_MIN                   0
#define ATC_ECADSPHCFG_2_GAIN_VAL_MAX                   63
#define ATC_ECADSPHCFG_2_GAIN_VAL_DEF                   0

#define ATC_ECADSPHCFG_3_NUM_VAL_MIN                   1
#define ATC_ECADSPHCFG_3_NUM_VAL_MAX                   9
#define ATC_ECADSPHCFG_3_NUM_VAL_DEF                   1







typedef enum AtcAudioSphTypeMapping_Tag
{
    TYPE_IIR_BIQUARD_LPF = 0,
    TYPE_IIR_BIQUARD_HPF,
    TYPE_IIR_BIQUARD_BPF0,
    TYPE_IIR_BIQUARD_BPF1,
    TYPE_IIR_BIQUARD_NOTCH,
    TYPE_IIR_BIQUARD_APF,
    TYPE_IIR_BIQUARD_PEAKING,
    TYPE_IIR_BIQUARD_LOWSHELF,
    TYPE_IIR_BIQUARD_HIGHSHELF,

}AtcAudioSphTypeMap;


typedef struct AtcAudioSphStrMapping_Tag
{
    UINT32  type;
    const CHAR *pStr;
}AtcAudioSphStrMap;

CmsRetId ecAudioCfg(const AtCmdInputContext *pAtCmdReq);
CmsRetId ecAudioSpeechCfg(const AtCmdInputContext *pAtCmdReq);
CmsRetId ecAudioCfgEqCancel(void);
CmsRetId  ecAudioCfgEqInputData(UINT8 chanId, UINT8 *pData, INT16 dataLength);
CmsRetId ecAudioFixedSrcCfg(const AtCmdInputContext *pAtCmdReq);
CmsRetId ecAudioSetVolume(const AtCmdInputContext *pAtCmdReq);


#endif

/* END OF FILE */

