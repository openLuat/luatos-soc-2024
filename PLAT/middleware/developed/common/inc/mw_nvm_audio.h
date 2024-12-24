#ifndef __MW_NVM_AUDIO_H__
#define __MW_NVM_AUDIO_H__
/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    mw_nvm_sms.h
 * Description:  middleware NVM SMS header file
 * History:      2021/05/24, Originated by glwu
 ****************************************************************************/
//#include "osasys.h"
//#include "mw_common.h"
//#include "cmisms.h"
#include "audioCfg.h"
//#include "hal_codec.h"

/*
 * Differences between these MW config/AON files:
 * 1> mw_nvm_config.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is still VALID, after FOTA (SW upgrading) if not erase the NVM in flash.
 *   c) if not configed in NVM/flash, use the default value.
 * 2> mw_nvm_info.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is not VALID (reset to default value), after FOTA (SW upgrading), if:
 *       i> NVM in flash is erased, or
 *       ii> NVM info structure size is changed, or
 *       ii> NVM file version is changed.
 *   c) if not configed in NVM/flash, use the default value.
 * 3> mw_aon_info.h
 *   a) parameter value is still VALID, after wakeup from deep sleep
 *   b) parameter value is not VALID, after reboot
 * 4> mw_common.h
 *   a) middleware common header file, which included by "mw_nvm_config.h"&"mw_nvm_info.h"&"mw_aon_info.h"
 *   b) As customers maybe have different requirements about how to maintain the config,
 *      here could set the common structure in this file
 * 2> mw_nvm_sms.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is not VALID (reset to default value), after FOTA (SW upgrading), if:
 *       i> NVM in flash is erased, or
 *       ii> NVM info structure size is changed, or
 *       ii> NVM file version is changed.
*/


/******************************************************************************
 *****************************************************************************
 * MARCO/MARCO
 *****************************************************************************
******************************************************************************/
/*
 *
*/
#define MID_WARE_NVM_AUDIO_FILE_NAME     "ecAudioTlvCfg.nvm"

#define MID_WARE_NVM_AUDIO_CUR_VER       0x0


typedef enum
{
    AUDIO_CFG_MODE_WB            = 0,
    AUDIO_CFG_MODE_NB            = 1,
    AUDIO_CFG_MODE_VOIP          = 2,
    AUDIO_CFG_MODE_MAX           = 3,
}AudioParaCfgModeType_e;

typedef enum
{
    AUDIO_CFG_DEVICE_HAND_SET             = 0,
    AUDIO_CFG_DEVICE_HEAD_SET_3_POLE      = 1,
    AUDIO_CFG_DEVICE_HEAD_SET_4_POLE      = 2,
    AUDIO_CFG_DEVICE_HAND_FREE            = 3,
    AUDIO_CFG_DEVICE_MAX                  = 4,
}AudioParaCfgDeviceType_e;

#define EC_ADCFG_SPEECH_TX_NUMB                           (AUDIO_CFG_MODE_MAX*AUDIO_CFG_DEVICE_MAX)
#define EC_ADCFG_SPEECH_RX_NUMB                           (AUDIO_CFG_MODE_MAX*AUDIO_CFG_DEVICE_MAX)

#define EC_ADCFG_TLV_VALUE_CONVERT(value)       ((((value)>>8)&0xff)|(((value)<<8)&0xff00)) 
#define EC_ADCFG_TLV_LEN_MAX                              32

#define EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB                  32
#define EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB                   10

/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/
#pragma pack(1)

typedef struct AudioParaCfgHeader_Tag
{
    UINT32                  headerFlag;          /*  */
    UINT32                  headerLen;           /*  */
    UINT32                  totalLen;            /*  */
    UINT32                  version;                 /*  */ 
    //UINT32                  crc;                 /* the file sys has crc check already */ 
}AudioParaCfgHeader_t;

typedef struct AudioParaCfgCommon_Tag
{
    UINT8  mode;          /*  */
    UINT8  direct;           /*  */
    UINT8  device;              /*  */
}AudioParaCfgCommon_t;

typedef struct _EPAT_AudioParaCfgAmr_Tag
{
    UINT8  mode;  
    UINT8  bypassEncode; 
    UINT8  bypassDecode; 
    #ifdef NVM_AUDIO_Test
    UINT8  test;
    #endif
}AudioParaCfgAmr_t;

typedef struct _EPAT_AudioParaCfgCodec_Tag
{
    UINT16  isDmic;                
    UINT16  isExPa;              
    INT16   exPaGain;
    INT16   txDigGain;   
    INT16   txAnaGain;    
    
    INT16   rxDigGain0;
    INT16   rxAnaGain0;
    INT16   rxDigGain50; 
    INT16   rxAnaGain50; 
    INT16   rxDigGain100;
    INT16   rxAnaGain100;
}AudioParaCfgCodec_t;



typedef struct AudioParaCfgCodecDev_Tag
{
    UINT8  isPaExist;
    UINT8  paGain;
    UINT8  isDmic;                
    UINT8  micVolume;              
    UINT8  volume; 
    UINT8  spkMicVolume;              
    UINT8  spkVolume;             
}AudioParaCfgCodecDev_t;

typedef struct AudioParaSphTxAEC_Tag
{
    UINT8   bypass;
    UINT16   delay;
    UINT8   cngMode;
    UINT8   echoMode;
    UINT8   nlpFlag;
}AudioParaSphTxAEC_t;

typedef struct AudioParaSphTxANS_Tag
{
    UINT8   bypass;
    UINT8   mode;
    INT16   eqBypass;
    UINT16  eqBand[EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB];
}AudioParaSphTxANS_t;

typedef struct AudioParaSphTxDRC_Tag
{
    UINT8   bypass;
    INT16    compThreshold;
    INT16    compRatio;
    INT16    expandThreshold;
    INT16    expandRatio;
    UINT16   attackTime;
    UINT16   releaseTime;
    UINT16   makeupGain;
}AudioParaSphTxDRC_t;

typedef struct AudioParaSphTxAGC_Tag
{
    UINT8   bypass;
    UINT8   targetLevel;
    UINT8   compressionGain;
    UINT8   limiterEnable;
}AudioParaSphTxAGC_t;

typedef struct AudioParaSphEQBiquard_Tag
{
    UINT8    biquardType;
    union
    {
        struct
        {
            INT16 a1;
            INT16 a2;
            INT16 b0;
            INT16 b1;
            INT16 b2;
        }raw;
        struct
        {
            INT16 f0;
            INT16 gain;
            INT16 q;
        }design;
    }filt;
}AudioParaSphEQBiquard_t;

typedef struct AudioParaSphTxEQ_Tag
{
    UINT32   bypass;
    UINT32   gain;
    UINT32   num;
    AudioParaSphEQBiquard_t    biquardParam[EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB];
}AudioParaSphTxEQ_t;

typedef struct AudioParaSphRxANS_Tag
{
    UINT8   bypass;
    UINT8   mode;
    INT16  eqBypass;
    UINT16  eqBand[EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB];
}AudioParaSphRxANS_t;

typedef struct AudioParaSphRxDRC_Tag
{
    UINT8    bypass;
    INT16    compThreshold;
    INT16    compRatio;
    INT16    expandThreshold;
    INT16    expandRatio;
    UINT16   attackTime;
    UINT16   releaseTime;
    UINT16   makeupGain;
}AudioParaSphRxDRC_t;

typedef struct AudioParaSphRxAGC_Tag
{
    UINT8   bypass;
    UINT8   targetLevel;
    UINT8   compressionGain;
    UINT8   limiterEnable;
}AudioParaSphRxAGC_t;

typedef struct AudioParaSphRxEQ_Tag
{
    UINT32   bypass;
    UINT32   gain;
    UINT32   num;
    AudioParaSphEQBiquard_t    biquardParam[EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB];
}AudioParaSphRxEQ_t;

typedef struct AudioParaCfgLogControl_Tag
{
    UINT8   TxBeforeCcio;
    UINT8   TxAfterCcio;
    UINT8   TxBeforeVem; 
    UINT8   TxAfterVem;
    UINT8   TxAfterEncoder;
    
    UINT8   RxBeforeDecoder;
    UINT8   RxBeforeVem;
    UINT8   RxAfterVem;
    UINT8   RxBeforeCcio;
    UINT8   RxAfterCcio;
}AudioParaCfgLogControl_t;

typedef struct AudioParaCfgSphTx_Tag
{
    AudioParaSphTxAEC_t       audioSphTxAEC;
    AudioParaSphTxANS_t       audioSphTxANS;
    AudioParaSphTxDRC_t       audioSphTxDRC;
    AudioParaSphTxAGC_t       audioSphTxAGC;
    AudioParaSphTxEQ_t        audioSphTxEQ;
}AudioParaCfgSphTx_t;

typedef struct AudioParaCfgSphRx_Tag
{
    AudioParaSphRxANS_t       audioSphRxANS;
    AudioParaSphRxDRC_t       audioSphRxDRC;
    AudioParaSphRxAGC_t       audioSphRxAGC;
    AudioParaSphRxEQ_t        audioSphRxEQ;
}AudioParaCfgSphRx_t;

typedef struct MWNvmAudioCfgStore_Tag
{
    AudioParaCfgHeader_t      audiosCfgHeader;
    //AudioParaCfgCommon_t      audioCfgCommon;
    AudioParaCfgAmr_t         audioCfgAmr;
    
    AudioParaCfgCodec_t    audioCfgCodec0;
    AudioParaCfgCodec_t    audioCfgCodec1;
    AudioParaCfgCodec_t    audioCfgCodec2;
    AudioParaCfgCodec_t    audioCfgCodec3;

    AudioParaCfgSphTx_t       audioCfgSphTx0;  //mode==0, direct==0 device==0 
    AudioParaCfgSphTx_t       audioCfgSphTx1;  //mode==0, direct==0 device==1 
    AudioParaCfgSphTx_t       audioCfgSphTx2;  //mode==0, direct==0 device==2 
    AudioParaCfgSphTx_t       audioCfgSphTx3;  //mode==1, direct==0 device==3 
    AudioParaCfgSphTx_t       audioCfgSphTx4;  //mode==1, direct==0 device==0 
    AudioParaCfgSphTx_t       audioCfgSphTx5;  //mode==1, direct==0 device==1 
    AudioParaCfgSphTx_t       audioCfgSphTx6;  //mode==1, direct==0 device==0 
    AudioParaCfgSphTx_t       audioCfgSphTx7;  //mode==1, direct==0 device==1 

    AudioParaCfgSphRx_t       audioCfgSphRx0;  //mode==0, direct==1 device==0     
    AudioParaCfgSphRx_t       audioCfgSphRx1;  //mode==0, direct==1 device==1 
    AudioParaCfgSphRx_t       audioCfgSphRx2;  //mode==0, direct==1 device==2 
    AudioParaCfgSphRx_t       audioCfgSphRx3;  //mode==1, direct==1 device==3 
    AudioParaCfgSphRx_t       audioCfgSphRx4;  //mode==1, direct==1 device==0 
    AudioParaCfgSphRx_t       audioCfgSphRx5;  //mode==1, direct==1 device==1 
    AudioParaCfgSphRx_t       audioCfgSphRx6;  //mode==1, direct==1 device==2 
    AudioParaCfgSphRx_t       audioCfgSphRx7;  //mode==1, direct==1 device==3 
    
}MWNvmAudioCfgStore;

#pragma pack()

#define MID_WARE_NVM_AUDIO_CFG_HEADER_FLAG      0xECADECAD

#define MID_WARE_NVM_AUDIO_CFG_INIT_FLAG            0xECAD0101
#define MID_WARE_NVM_AUDIO_CFG_NOT_INIT_FLAG        0xECAD0000

#define NVM_AUDIO_CFG_TLV_TPYE_MASK        0x00FFFFFF
#define NVM_AUDIO_CFG_TLV_LEN_MASK         0x000000FF
#define NVM_AUDIO_CFG_TLV_LEN_SHIFT        24
#define NVM_AUDIO_CFG_TLV_ERR_MASK         0xFFFFFFFF
#define NVM_AUDIO_CFG_TLV_VALUE_ERR        0xFFFF
#define NVM_AUDIO_CFG_TLV_VALUE_OK         0x0

#define NVM_AUDIO_CFG_TLV_LEN_MAX          (255)
#define NVM_AUDIO_CFG_TLV_LEN_MIN          3

#define NVM_AUDIO_CFG_CODEC_NUMB_MAX       (AUDIO_CFG_MODE_MAX*AUDIO_CFG_DEVICE_MAX)

#define NVM_AUDIO_CFG_TX_RX_NUMB_MAX            8
#define NVM_AUDIO_CFG_TX_RX_MODULE_SHIFT        16
#define NVM_AUDIO_CFG_CODEC_SHIFT               16

#define NVM_AUDIO_CFG_INDEX_MASK           0x000000FF

#define NVM_AUDIO_CFG_TX_RX_MODULE_MULTIPLE     3

#define EC_ADCFG_COMMON_MODE_VAL_DEF                     0
#define EC_ADCFG_COMMON_DIR_VAL_DEF                      0
#define EC_ADCFG_COMMON_DEVICE_VAL_DEF                   0

#define EC_ADCFG_CODEC_DEV_IS_PA_EXIST_VAL_DEF           1
#define EC_ADCFG_CODEC_DEV_PA_GAIN_VAL_DEF               2
#define EC_ADCFG_CODEC_DEV_IS_DMIC_VAL_DEF               3
#define EC_ADCFG_CODEC_DEV_MIC_VOLUME_VAL_DEF            4
#define EC_ADCFG_CODEC_DEV_VOLUME_VAL_DEF                5
#define EC_ADCFG_CODEC_DEV_SPK_MIC_VOLUME_VAL_DEF        6
#define EC_ADCFG_CODEC_DEV_SPK_VOLUME_VAL_DEF            7

#define EC_ADCFG_AMR_MODE_VAL_DEF                               1
#define EC_ADCFG_AMR_BYPASS_ENCODE_VAL_DEF                      0
#define EC_ADCFG_AMR_BYPASS_DECODE_VAL_DEF                      0

#define EC_ADCFG_SPEECH_TX_AEC_BYPASS_VAL_DEF                      5
#define EC_ADCFG_SPEECH_TX_AEC_DELAY_VAL_DEF                       4
#define EC_ADCFG_SPEECH_TX_AEC_CNGMODE_VAL_DEF                     3
#define EC_ADCFG_SPEECH_TX_AEC_ECHO_MODE_VAL_DEF                   2
#define EC_ADCFG_SPEECH_TX_AEC_NLP_FLAG_VAL_DEF                    1

#define EC_ADCFG_SPEECH_TX_ANS_BYPASS_VAL_DEF                   0
#define EC_ADCFG_SPEECH_TX_ANS_MODE_VAL_DEF                     0

#define EC_ADCFG_SPEECH_TX_DRC_BYPASS_VAL_DEF                         0
#define EC_ADCFG_SPEECH_TX_DRC_COMP_THRESHOLD_VAL_DEF                 (-960)
#define EC_ADCFG_SPEECH_TX_DRC_COMP_RATIO_VAL_DEF                     (32)
#define EC_ADCFG_SPEECH_TX_DRC_EXPAND_THRESHOLD_VAL_DEF               (-2880)
#define EC_ADCFG_SPEECH_TX_DRC_EXPAND_RATIO_VAL_DEF                   (-51)
#define EC_ADCFG_SPEECH_TX_DRC_ATTACK_TIME_VAL_DEF                    14459
#define EC_ADCFG_SPEECH_TX_DRC_RELEASE_TIME_VAL_DEF                   16046
#define EC_ADCFG_SPEECH_TX_DRC_MAKEUP_GAIN_VAL_DEF                    1

#define EC_ADCFG_SPEECH_TX_AGC_BYPASS_VAL_DEF                             0
#define EC_ADCFG_SPEECH_TX_AGC_TARGET_LEVEL_VAL_DEF                       0
#define EC_ADCFG_SPEECH_TX_AGC_COMPRESSION_GAIN_VAL_DEF                   6
#define EC_ADCFG_SPEECH_TX_AGC_LIMITER_ENABLE_VAL_DEF                     1

#define EC_ADCFG_SPEECH_TX_EQ_BYPASS_VAL_DEF                           0
#define EC_ADCFG_SPEECH_TX_EQ_GAIN_VAL_DEF                             0
#define EC_ADCFG_SPEECH_TX_EQ_NUM_VAL_DEF                              1
#define EC_ADCFG_SPEECH_TX_EQ_TYPE_VAL_DEF                             0
#define EC_ADCFG_SPEECH_TX_TX_EQ_FILT_VALUE1_VAL_DEF                   500
#define EC_ADCFG_SPEECH_TX_TX_EQ_FILT_VALUE2_VAL_DEF                   0
#define EC_ADCFG_SPEECH_TX_TX_EQ_FILT_VALUE3_VAL_DEF                   5790

#define EC_ADCFG_SPEECH_RX_ANS_BYPASS_VAL_DEF                   0
#define EC_ADCFG_SPEECH_RX_ANS_MODE_VAL_DEF                     0

#define EC_ADCFG_SPEECH_RX_DRC_BYPASS_VAL_DEF                         0
#define EC_ADCFG_SPEECH_RX_DRC_COMP_THRESHOLD_VAL_DEF                 (-960)
#define EC_ADCFG_SPEECH_RX_DRC_COMP_RATIO_VAL_DEF                     (32)
#define EC_ADCFG_SPEECH_RX_DRC_EXPAND_THRESHOLD_VAL_DEF               (-2880)
#define EC_ADCFG_SPEECH_RX_DRC_EXPAND_RATIO_VAL_DEF                   (-51)
#define EC_ADCFG_SPEECH_RX_DRC_ATTACK_TIME_VAL_DEF                    14459
#define EC_ADCFG_SPEECH_RX_DRC_RELEASE_TIME_VAL_DEF                   16046
#define EC_ADCFG_SPEECH_RX_DRC_MAKEUP_GAIN_VAL_DEF                    1

#define EC_ADCFG_SPEECH_RX_AGC_BYPASS_VAL_DEF                             0
#define EC_ADCFG_SPEECH_RX_AGC_TARGET_LEVEL_VAL_DEF                       0
#define EC_ADCFG_SPEECH_RX_AGC_COMPRESSION_GAIN_VAL_DEF                   6
#define EC_ADCFG_SPEECH_RX_AGC_LIMITER_ENABLE_VAL_DEF                     1

#define EC_ADCFG_SPEECH_RX_EQ_BYPASS_VAL_DEF                           0
#define EC_ADCFG_SPEECH_RX_EQ_GAIN_VAL_DEF                             0
#define EC_ADCFG_SPEECH_RX_EQ_NUM_VAL_DEF                              1
#define EC_ADCFG_SPEECH_RX_EQ_TYPE_VAL_DEF                             0
#define EC_ADCFG_SPEECH_RX_TX_EQ_FILT_VALUE1_VAL_DEF                   500
#define EC_ADCFG_SPEECH_RX_TX_EQ_FILT_VALUE2_VAL_DEF                   0
#define EC_ADCFG_SPEECH_RX_TX_EQ_FILT_VALUE3_VAL_DEF                   5790

#define EC_ADCFG_LOG_CTRL_TX_BEFORE_CCIO_VAL_DEF                   0
#define EC_ADCFG_LOG_CTRL_TX_AFTER_CCIO_VAL_DEF                    0
#define EC_ADCFG_LOG_CTRL_TX_BEFORE_VEM_VAL_DEF                    0
#define EC_ADCFG_LOG_CTRL_TX_AFTER_VEM_VAL_DEF                     0
#define EC_ADCFG_LOG_CTRL_TX_AFTER_ENCODER_VAL_DEF                 0
#define EC_ADCFG_LOG_CTRL_RX_BEFORE_DECODER_VAL_DEF                0
#define EC_ADCFG_LOG_CTRL_RX_BEFORE_VEM_VAL_DEF                    0
#define EC_ADCFG_LOG_CTRL_RX_AFTER_VEM_VAL_DEF                     0
#define EC_ADCFG_LOG_CTRL_RX_BEFORE_CCIO_VAL_DEF                   0
#define EC_ADCFG_LOG_CTRL_RX_AFTER_CCIO_VAL_DEF                    0

#define AUDIO_CFG_TLV_TYPE_SHIFT                   8

typedef enum _EPAT_AtcAudioTypeMapping_Tag
{
//  
    EC_AUDIO_CFG_TLV_ITEM = 0,
    AUDIO_CFG_TLV_HEADER_FLAG        = 0x100000,
    AUDIO_CFG_TLV_HEADER_LEN         = 0x100001,
    AUDIO_CFG_TLV_HEADER_TOTAL_LEN   = 0x100002,
    AUDIO_CFG_TLV_HEADER_CRC         = 0x100003,

    AUDIO_CFG_TLV_AMR_MODE             = 0x100101,
    AUDIO_CFG_TLV_AMR_BYPASSENCODE     = 0x100102,
    AUDIO_CFG_TLV_AMR_BYPASSDECODE     = 0x100103,
    
    AUDIO_CFG_TLV_CODEC_ISDMIC          = 0x100200,
    AUDIO_CFG_TLV_CODEC_ISEXPA          = 0x100201,
    AUDIO_CFG_TLV_CODEC_EXPAGAIN        = 0x100202,
    AUDIO_CFG_TLV_CODEC_TXDIGGAIN       = 0x100203,
    AUDIO_CFG_TLV_CODEC_TXANAGAIN       = 0x100204,
    AUDIO_CFG_TLV_CODEC_RXDIGGAIN0      = 0x100205,
    AUDIO_CFG_TLV_CODEC_RXANAGAIN0      = 0x100206,
    AUDIO_CFG_TLV_CODEC_RXDIGGAIN50     = 0x100207,
    AUDIO_CFG_TLV_CODEC_RXANAGAIN50     = 0x100208,
    AUDIO_CFG_TLV_CODEC_RXDIGGAIN100    = 0x100209,
    AUDIO_CFG_TLV_CODEC_RXANAGAIN100    = 0x10020A,
    
    AUDIO_CFG_TLV_SPEECH_TX_AEC_BYPASS    = 0x100401,
    AUDIO_CFG_TLV_SPEECH_TX_AEC_DELAY     = 0x100402,
    AUDIO_CFG_TLV_SPEECH_TX_AEC_CNGMODE   = 0x100403,
    AUDIO_CFG_TLV_SPEECH_TX_AEC_ECHOMODE  = 0x100404,
    AUDIO_CFG_TLV_SPEECH_TX_AEC_NLPFLAG   = 0x100405,
    
    AUDIO_CFG_TLV_SPEECH_TX_ANS_BYPASS     = 0x100501,
    AUDIO_CFG_TLV_SPEECH_TX_ANS_MODE       = 0x100502,
    AUDIO_CFG_TLV_SPEECH_TX_ANS_EQBAND     = 0x100503,
    AUDIO_CFG_TLV_SPEECH_TX_ANS_EQBYPASS   = 0x100504,
    
    AUDIO_CFG_TLV_SPEECH_TX_DRC_BYPASS           = 0x100601,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_COMPTHRESHOLD    = 0x100602,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_COMPRATIO        = 0x100603,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_EXPANDTHRESHOLD  = 0x100604,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_EXPANDRATIO      = 0x100605,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_ATTACKTIME       = 0x100606,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_RELEASETIME      = 0x100607,
    AUDIO_CFG_TLV_SPEECH_TX_DRC_MAKEUPGAIN       = 0x100608,
    
    AUDIO_CFG_TLV_SPEECH_TX_AGC_BYPASS           = 0x100701,
    AUDIO_CFG_TLV_SPEECH_TX_AGC_TARGETLEVEL      = 0x100702,
    AUDIO_CFG_TLV_SPEECH_TX_AGC_COMPRESSIONGAIN  = 0x100703,
    AUDIO_CFG_TLV_SPEECH_TX_AGC_LIMITERENABLE    = 0x100704,
    
    AUDIO_CFG_TLV_SPEECH_TX_EQ_BYPASS      = 0x100801,
    AUDIO_CFG_TLV_SPEECH_TX_EQ_GAIN        = 0x100802,
    AUDIO_CFG_TLV_SPEECH_TX_EQ_NUM         = 0x100803,
    AUDIO_CFG_TLV_SPEECH_TX_EQ_PARAMS      = 0x100808,

    AUDIO_CFG_TLV_SPEECH_RX_ANS_BYPASS     = 0x100901,
    AUDIO_CFG_TLV_SPEECH_RX_ANS_MODE       = 0x100902,
    AUDIO_CFG_TLV_SPEECH_RX_ANS_EQBAND     = 0x100903,
    AUDIO_CFG_TLV_SPEECH_RX_ANS_EQBYPASS   = 0x100904,
    
    AUDIO_CFG_TLV_SPEECH_RX_DRC_BYPASS           = 0x100A01,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_COMPTHRESHOLD    = 0x100A02,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_COMPRATIO        = 0x100A03,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_EXPANDTHRESHOLD  = 0x100A04,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_EXPANDRATIO      = 0x100A05,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_ATTACKTIME       = 0x100A06,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_RELEASETIME      = 0x100A07,
    AUDIO_CFG_TLV_SPEECH_RX_DRC_MAKEUPGAIN       = 0x100A08,
    
    AUDIO_CFG_TLV_SPEECH_RX_AGC_BYPASS           = 0x100B01,
    AUDIO_CFG_TLV_SPEECH_RX_AGC_TARGETLEVEL     = 0x100B02,
    AUDIO_CFG_TLV_SPEECH_RX_AGC_COMPRESSIONGAIN = 0x100B03,
    AUDIO_CFG_TLV_SPEECH_RX_AGC_LIMITERENABLE   = 0x100B04,
    
    AUDIO_CFG_TLV_SPEECH_RX_EQ_BYPASS      = 0x100C01,
    AUDIO_CFG_TLV_SPEECH_RX_EQ_GAIN        = 0x100C02,
    AUDIO_CFG_TLV_SPEECH_RX_EQ_NUM         = 0x100C03,
    AUDIO_CFG_TLV_SPEECH_RX_EQ_PARAMS      = 0x100C08,

    AUDIO_CFG_TLV_LOG_CTRL_TXBEFORECCIO        = 0x100D00,
    AUDIO_CFG_TLV_LOG_CTRL_TXAFTERCCIO         = 0x100D01,
    AUDIO_CFG_TLV_LOG_CTRL_TXBEFOREVEM         = 0x100D02,
    AUDIO_CFG_TLV_LOG_CTRL_TXAFTERVEM          = 0x100D03,
    AUDIO_CFG_TLV_LOG_CTRL_TXAFTERENCODER      = 0x100D04,
    AUDIO_CFG_TLV_LOG_CTRL_RXBEFOREDECODER     = 0x100D05,
    AUDIO_CFG_TLV_LOG_CTRL_RXBEFOREVEM         = 0x100D06,
    AUDIO_CFG_TLV_LOG_CTRL_RXAFTERVEM          = 0x100D07,
    AUDIO_CFG_TLV_LOG_CTRL_RXBEFORECCIO        = 0x100D08,
    AUDIO_CFG_TLV_LOG_CTRL_RXAFTERCCIO         = 0x100D09,

}AtcAudioTypeMap_e;

#pragma pack(1)
typedef struct EcAudioCfgTlvInt8Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    INT8     value;
    
}EcAudioCfgTlvInt8Item_t;

typedef struct EcAudioCfgTlvUint8Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    UINT8    value;
    
}EcAudioCfgTlvUint8Item_t;

typedef struct EcAudioCfgTlvInt16Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    INT16    value;
    
}EcAudioCfgTlvInt16Item_t;

typedef struct EcAudioCfgTlvUint16Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    UINT16   value;
    
}EcAudioCfgTlvUint16Item_t;


typedef struct EcAudioCfgTlvInt32Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    INT32    value;
    
}EcAudioCfgTlvInt32Item_t;

typedef struct EcAudioCfgTlvUint32Item_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    UINT32   value;
    
}EcAudioCfgTlvUint32Item_t;

typedef struct EcAudioCfgTlvVariItem_Tag
{
    UINT32   type  : 24;
    UINT32   len   : 8;
    UINT8    value[];    //variable length value
    
}EcAudioCfgTlvVariItem_t;

typedef struct EcAudioCfgTlvHeader_Tag
{
    UINT32                  headerFlag;          /*  */
    UINT32                  headerLen;           /*  */
    UINT32                  totalLen;            /*  */
    UINT32                  version;             /*  */
    //UINT32                  crc;                 /* the file sys has crc check already */  
}EcAudioCfgTlvHeader_t;

typedef struct EcAudioCfgTlvCommon_Tag
{
    EcAudioCfgTlvUint8Item_t  mode;                /*  */
    EcAudioCfgTlvUint8Item_t  direct;              /*  */
    EcAudioCfgTlvUint8Item_t  device;              /*  */
}EcAudioCfgTlvCommon_t;

typedef struct EcAudioCfgTlvAmr_Tag
{
    EcAudioCfgTlvUint8Item_t  mode;              
    EcAudioCfgTlvUint8Item_t  bypassEncode; 
    EcAudioCfgTlvUint8Item_t  bypassDecode;
    
    #ifdef NVM_AUDIO_Test
    EcAudioCfgTlvUint8Item_t  test; 
    #endif
}EcAudioCfgTlvAmr_t;

typedef struct EcAudioCfgTlvCodec_Tag
{
    EcAudioCfgTlvUint16Item_t  isDmic;                
    EcAudioCfgTlvUint16Item_t  isExPa;              
    EcAudioCfgTlvUint16Item_t  exPaGain; 
    EcAudioCfgTlvUint16Item_t  txDigGain;     
    EcAudioCfgTlvUint16Item_t  txAnaGain; 

    EcAudioCfgTlvUint16Item_t  rxDigGain0;     
    EcAudioCfgTlvUint16Item_t  rxAnaGain0; 
    EcAudioCfgTlvUint16Item_t  rxDigGain50;     
    EcAudioCfgTlvUint16Item_t  rxAnaGain50; 
    EcAudioCfgTlvUint16Item_t  rxDigGain100;     
    EcAudioCfgTlvUint16Item_t  rxAnaGain100; 
}EcAudioCfgTlvCodec_t;

typedef struct EcAudioCfgTlvSphTxAEC_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvUint16Item_t   delay;
    EcAudioCfgTlvUint8Item_t   cngMode;
    EcAudioCfgTlvUint8Item_t   echoMode;
    EcAudioCfgTlvUint8Item_t   nlpFlag;
}EcAudioCfgTlvSphTxAEC_t;

typedef struct EcAudioCfgTlvSphTxANS_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvUint8Item_t   mode;
    EcAudioCfgTlvInt16Item_t   eqBypass;
    EcAudioCfgTlvVariItem_t    eqBand;    
}EcAudioCfgTlvSphTxANS_t;

typedef struct EcAudioCfgTlvSphTxDRC_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvInt16Item_t    compThreshold;
    EcAudioCfgTlvInt16Item_t    compRatio;
    EcAudioCfgTlvInt16Item_t    expandThreshold;
    EcAudioCfgTlvInt16Item_t    expandRatio;
    EcAudioCfgTlvUint16Item_t   attackTime;
    EcAudioCfgTlvUint16Item_t   releaseTime;
    EcAudioCfgTlvUint16Item_t   makeupGain;
}EcAudioCfgTlvSphTxDRC_t;

typedef struct EcAudioCfgTlvSphTxAGC_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvUint8Item_t   targetLevel;
    EcAudioCfgTlvUint8Item_t   compressionGain;
    EcAudioCfgTlvUint8Item_t   limiterEnable;
}EcAudioCfgTlvSphTxAGC_t;

typedef struct EcAudioCfgTlvSphTxEQ_Tag
{
    EcAudioCfgTlvUint32Item_t   bypass;
    EcAudioCfgTlvUint32Item_t   gain;
    EcAudioCfgTlvUint32Item_t   num;
    EcAudioCfgTlvVariItem_t    biquardParam;
}EcAudioCfgTlvSphTxEQ_t;

typedef struct EcAudioCfgTlvSphRxANS_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvUint8Item_t   mode;
    EcAudioCfgTlvInt16Item_t   eqBypass;
    EcAudioCfgTlvVariItem_t    eqBand;
}EcAudioCfgTlvSphRxANS_t;

typedef struct EcAudioCfgTlvSphRxDRC_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvInt16Item_t    compThreshold;
    EcAudioCfgTlvInt16Item_t    compRatio;
    EcAudioCfgTlvInt16Item_t    expandThreshold;
    EcAudioCfgTlvInt16Item_t    expandRatio;
    EcAudioCfgTlvUint16Item_t   attackTime;
    EcAudioCfgTlvUint16Item_t   releaseTime;
    EcAudioCfgTlvUint16Item_t   makeupGain;
}EcAudioCfgTlvSphRxDRC_t;

typedef struct EcAudioCfgTlvSphRxAGC_Tag
{
    EcAudioCfgTlvUint8Item_t   bypass;
    EcAudioCfgTlvUint8Item_t   targetLevel;
    EcAudioCfgTlvUint8Item_t   compressionGain;
    EcAudioCfgTlvUint8Item_t   limiterEnable;
}EcAudioCfgTlvSphRxAGC_t;

typedef struct EcAudioCfgTlvSphRxEQ_Tag
{
    EcAudioCfgTlvUint32Item_t   bypass;
    EcAudioCfgTlvUint32Item_t   gain;
    EcAudioCfgTlvUint32Item_t   num;
    EcAudioCfgTlvVariItem_t     biquardParam;
}EcAudioCfgTlvSphRxEQ_t;

typedef struct EcAudioCfgTlvLogCtrl_Tag
{   
    EcAudioCfgTlvUint8Item_t   logCtrlTxBeforeCcio;
    EcAudioCfgTlvUint8Item_t   logCtrlTxAfterCcio;
    EcAudioCfgTlvUint8Item_t   logCtrlTxBeforeVem;
    EcAudioCfgTlvUint8Item_t   logCtrlTxAfterVem;
    EcAudioCfgTlvUint8Item_t   logCtrlTxAfterEncoder;

    EcAudioCfgTlvUint8Item_t   logCtrlRxBeforeDecoder;
    EcAudioCfgTlvUint8Item_t   logCtrlRxBeforeVem;
    EcAudioCfgTlvUint8Item_t   logCtrlRxAfterVem;
    EcAudioCfgTlvUint8Item_t   logCtrlRxBeforeCcio;
    EcAudioCfgTlvUint8Item_t   logCtrlRxAfterCcio;
}EcAudioCfgTlvLogCtrl_t;

typedef struct EcAudioCfgTlvSphTx_Tag
{
    EcAudioCfgTlvSphTxAEC_t       audioSphTxAEC;
    EcAudioCfgTlvSphTxANS_t       audioSphTxANS;
    EcAudioCfgTlvSphTxDRC_t       audioSphTxDRC;
    EcAudioCfgTlvSphTxAGC_t       audioSphTxAGC;
    EcAudioCfgTlvSphTxEQ_t        audioSphTxEQ;
}EcAudioCfgTlvSphTx_t;

typedef struct EcAudioCfgTlvSphRx_Tag
{
    EcAudioCfgTlvSphRxANS_t       audioSphRxANS;
    EcAudioCfgTlvSphRxDRC_t       audioSphRxDRC;
    EcAudioCfgTlvSphRxAGC_t       audioSphRxAGC;
    EcAudioCfgTlvSphRxEQ_t        audioSphRxEQ;
}EcAudioCfgTlvSphRx_t;


typedef struct MWNvmAudioCfgTlvStore_Tag
{
    EcAudioCfgTlvHeader_t      audioCfgTlvHeader;    /*  */
    
    //EcAudioCfgTlvCommon_t      audioCfgTlvCommon;
    EcAudioCfgTlvAmr_t         audioCfgTlvAmr;

    /*
    EcAudioCfgTlvCodec_t       audioCfgTlvCodec0;
    EcAudioCfgTlvCodec_t       audioCfgTlvCodec1;
    EcAudioCfgTlvCodec_t       audioCfgTlvCodec2;
    EcAudioCfgTlvCodec_t       audioCfgTlvCodec3;
    */
    EcAudioCfgTlvCodec_t       audioCfgTlvCodec[NVM_AUDIO_CFG_CODEC_NUMB_MAX];

    /*
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx0;  //mode==0, direct==0 device==0 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx1;  //mode==0, direct==0 device==1 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx2;  //mode==0, direct==0 device==2 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx3;  //mode==1, direct==0 device==3 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx4;  //mode==1, direct==0 device==0 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx5;  //mode==1, direct==0 device==1 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx6;  //mode==1, direct==0 device==2 
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx7;  //mode==1, direct==0 device==3 
    */
    EcAudioCfgTlvSphTx_t       audioCfgTlvSphTx[EC_ADCFG_SPEECH_TX_NUMB];  //EC_ADCFG_SPEECH_TX_NUMB==0 

    /*
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx0;  //mode==0, direct==1 device==0     
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx1;  //mode==0, direct==1 device==1 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx2;  //mode==0, direct==1 device==2 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx3;  //mode==1, direct==1 device==3 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx4;  //mode==1, direct==1 device==0 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx5;  //mode==1, direct==1 device==1 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx6;  //mode==1, direct==1 device==2 
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx7;  //mode==1, direct==1 device==3 
    */
    EcAudioCfgTlvSphRx_t       audioCfgTlvSphRx[EC_ADCFG_SPEECH_RX_NUMB];  //EC_ADCFG_SPEECH_RX_NUMB==8   
    EcAudioCfgTlvLogCtrl_t     audioLogControl;
}ecAudioCfgTlvStore;
#pragma pack()



/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/
UINT32 mwNvmAudioCfgInit(void);
BOOL mwNvmAudioCfgRead(ecAudioCfgTlvStore  *pAudioCfg);
void mwNvmAudioCfgSave(ecAudioCfgTlvStore      *pWriteMwNvmAudioCfg);

UINT32 mwNvmAudioCfgCommonGet(AudioParaCfgCommon_t *pMwNvmAudioCfgCommon, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgCommonSet(AudioParaCfgCommon_t       *pMwNvmAudioCfgCommon, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgCodecGet(AudioParaCfgCommon_t *pMwNvmAudioCfgCommon, AudioParaCfgCodec_t *pMwNvmAudioCfgCodec, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgCodecSet(AudioParaCfgCommon_t *pMwNvmAudioCfgCommon, AudioParaCfgCodec_t *pMwNvmAudioCfgCodec, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgCodecDeviceGet(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AudioParaCfgCodecDev_t         *pMwNvmAudioCfgCodecDev, ecAudioCfgTlvStore *pMwNvmAudioCfg);
    BOOL mwNvmAudioCfgCodecDeviceSet(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AudioParaCfgCodecDev_t *pMwNvmAudioCfgCodecDev, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgAmrGet(AudioParaCfgAmr_t *pMwNvmAudioCfgAmr, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgAmrSet(AudioParaCfgAmr_t *pMwNvmAudioCfgAmr, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetTxAEC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AecConfig_t *pMwNvmAudioCfgTxAEC, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetTxAEC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AecConfig_t *pWriteMwNvmAudioCfgTxAEC, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetTxANS(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AnsConfig_t *pMwNvmAudioCfgTxANS, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetTxANS(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AnsConfig_t *pWriteMwNvmAudioCfgTxANS, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetTxDRC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, DrcConfig_t *pMwNvmAudioCfgTxDRC, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetTxDRC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, DrcConfig_t *pWriteMwNvmAudioCfgTxDRC, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetTxAGC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AgcConfig_t *pMwNvmAudioCfgTxAGC, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetTxAGC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AgcConfig_t *pWriteMwNvmAudioCfgTxAGC, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetTxEQ(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pMwNvmAudioCfgTxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetTxEQ(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pWriteMwNvmAudioCfgTxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);
UINT32 mwNvmAudioCfgSpeechGetTxEQForAt(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pMwNvmAudioCfgTxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetRxANS(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AnsConfig_t *pMwNvmAudioCfgRxANS, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetRxANS(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AnsConfig_t *pWriteMwNvmAudioCfgRxANS, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetRxDRC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, DrcConfig_t *pMwNvmAudioCfgRxDRC, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetRxDRC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, DrcConfig_t *pWriteMwNvmAudioCfgRxDRC, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetRxAGC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AgcConfig_t *pMwNvmAudioCfgRxAGC, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetRxAGC(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, AgcConfig_t *pWriteMwNvmAudioCfgRxAGC, ecAudioCfgTlvStore *pMwNvmAudioCfg);

UINT32 mwNvmAudioCfgSpeechGetRxEQ(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pMwNvmAudioCfgRxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgSpeechSetRxEQ(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pWriteMwNvmAudioCfgRxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);
UINT32 mwNvmAudioCfgSpeechGetRxEQForAt(AudioParaCfgCommon_t         *pMwNvmAudioCfgCommon, EqConfig_t *pMwNvmAudioCfgRxEQ, ecAudioCfgTlvStore *pMwNvmAudioCfg);
UINT32 mwNvmAudioCfgLogControlGet(/*AudioParaCfgCommon_t *pMwNvmAudioCfgCommon, */AudioParaCfgLogControl_t        *pMwNvmAudioCfgLogCtrl, ecAudioCfgTlvStore *pMwNvmAudioCfg);
BOOL mwNvmAudioCfgLogControlSet(/*AudioParaCfgCommon_t *pMwNvmAudioCfgCommon, */AudioParaCfgLogControl_t *pMwNvmAudioCfgLogCtrl, ecAudioCfgTlvStore *pMwNvmAudioCfg);

/**
  \fn           Bool mwNvmGetAudioCfgForCP(AudioConfig_t *readAudioCfgForCp, UINT8 device, UINT8 direct, UINT8 mode)
  \brief 
  \ useage:
  \ audioCfg = mallocEc(sizeof(audioCfg));
  \
  \ ret = mwNvmGetAudioCfgForCP(audioCfg, device, direct, mode);
  \
  \ if(ret == SUCCESS)
  \ AcVoiceCodecConfigReq.pAudioPara = audioCfg
  \ else
  \ AcVoiceCodecConfigReq.pAudioPara = NULL;
  \
  \ if you need to modify the audio param read this time, modify here
  \
  \ EX:
  \ audioCfg->speechCfgTx.CVT_AEC.cngMode = 2;
  \
  \ sendcfgreq IPC to CP and wait for CONFIRM 
  \
  \ free(audioCfg) in confirm
  \param[in]    audioCfgPtr 
  \param[out]   audioCfgPtr
  \returns      BOOL.
*/
BOOL mwNvmGetAudioCfgForCP(AudioConfig_t *readAudioCfgForCp, UINT8 device, UINT8 direct, UINT8 mode);

#endif /* __MW_NVM_AUDIO_H__ */


