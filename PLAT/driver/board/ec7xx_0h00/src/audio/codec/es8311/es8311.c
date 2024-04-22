/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    es8311.c
 * Description:  EC7xx es8311 file
 * History:      Rev1.0   2021-9-18
 *
 ****************************************************************************/

#include "es8311.h"
#include "mw_nvm_audio.h"


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define IS_DMIC             0
#define USE_NV_VOLUME       1
#define MCLK_DIV_FRE        256

#define ES8311_DEFAULT_CONFIG()                         \
{                                                       \
        .adcInput  = CODEC_ADC_INPUT_LINE1,             \
        .dacOutput = CODEC_DAC_OUTPUT_ALL,              \
        .codecMode = CODEC_MODE_BOTH,                   \
        .codecIface = {                                 \
            .mode    = CODEC_MODE_SLAVE,                \
            .fmt     = CODEC_I2S_MODE,                  \
            .samples = CODEC_16K_SAMPLES,               \
            .bits    = CODEC_BIT_LENGTH_16BITS,         \
            .channel = CODEC_MONO,                      \
            .polarity = 1,                              \
        },                                              \
};


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/

// Clock coefficient structer
struct _coeffDiv 
{
    uint32_t mclk;          // mclk frequency
    uint32_t rate;          // sample rate
    uint8_t preDiv;         // the pre divider with range from 1 to 8
    uint8_t preMulti;       // the pre multiplier with x1, x2, x4 and x8 selection
    uint8_t adcDiv;         // adcclk divider
    uint8_t dacDiv;         // dacclk divider
    uint8_t fsMode;         // double speed or single speed, =0, ss, =1, ds
    uint8_t lrckH;          // adclrck divider and daclrck divider
    uint8_t lrckL;
    uint8_t bclkDiv;        // sclk divider
    uint8_t adcOsr;         // adc osr
    uint8_t dacOsr;         // dac osr
};

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

// 8311 func list
HalCodecFuncList_t es8311DefaultHandle = 
{
    .codecType                  = ES8311,
    .halCodecInitFunc           = es8311Init,
    .halCodecDeinitFunc         = es8311DeInit,
    .halCodecCtrlStateFunc      = es8311StartStop,
    .halCodecCfgIfaceFunc       = es8311Config,
    .halCodecSetMuteFunc        = es8311SetMute,
    .halCodecSetVolumeFunc      = es8311SetVolume,
    .halCodecGetVolumeFunc      = es8311GetVolume,
    //.halCodecEnablePAFunc           = es8311EnablePA,
    .halCodecSetMicVolumeFunc   = es8311SetMicVolume,
    .halCodecLock               = NULL,
    .handle                     = NULL,
    .halCodecGetDefaultCfg      = es8311GetDefaultCfg,
};



/*----------------------------------------------------------------------------*
 *                      PRIVATE VARIABLES                                     *
 *----------------------------------------------------------------------------*/

// codec hifi mclk clock divider coefficients
static const struct _coeffDiv coeffDiv[] = 
{
    //mclk     rate   preDiv  mult  adcDiv dacDiv fsMode lrch  lrcl  bckdiv adcOsr dacOsr
    // 8k
    {12288000, 8000 , 0x06,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {18432000, 8000 , 0x03,    0x02, 0x03,   0x03,   0x00,   0x05, 0xff, 0x18,  0x10,   0x20},
    {16384000, 8000 , 0x08,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {8192000 , 8000 , 0x04,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {6144000 , 8000 , 0x03,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {4096000 , 8000 , 0x02,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {3072000 , 8000 , 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {2048000 , 8000 , 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {1536000 , 8000 , 0x03,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},
    {1024000 , 8000 , 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04,  0x10,   0x20},

    // 16k
    {12288000, 16000, 0x03,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {18432000, 16000, 0x03,    0x02, 0x03,   0x03,   0x00,   0x02, 0xff, 0x0c, 0x10,    0x20},
    {16384000, 16000, 0x04,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {8192000 , 16000, 0x02,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {6144000 , 16000, 0x03,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {4096000 , 16000, 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {3072000 , 16000, 0x03,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {2048000 , 16000, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {1536000 , 16000, 0x03,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},
    {1024000 , 16000, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x20},

    // 22.05k
    {11289600, 22050, 0x02,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {5644800 , 22050, 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {2822400 , 22050, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {1411200 , 22050, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},

    // 32k
    {12288000, 32000, 0x03,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {18432000, 32000, 0x03,    0x04, 0x03,   0x03,   0x00,   0x02, 0xff, 0x0c, 0x10,    0x10},
    {16384000, 32000, 0x02,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {8192000 , 32000, 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {6144000 , 32000, 0x03,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {4096000 , 32000, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {3072000 , 32000, 0x03,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {2048000 , 32000, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {1536000 , 32000, 0x03,    0x08, 0x01,   0x01,   0x01,   0x00, 0x7f, 0x02, 0x10,    0x10},
    {1024000 , 32000, 0x01,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},

    // 44.1k
    {11289600, 44100, 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {5644800 , 44100, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {2822400 , 44100, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {1411200 , 44100, 0x01,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},

    // 48k
    {12288000, 48000, 0x01,    0x01, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {18432000, 48000, 0x03,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {6144000 , 48000, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {3072000 , 48000, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {1536000 , 48000, 0x01,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},

    // 96k
    {12288000, 96000, 0x01,    0x02, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {18432000, 96000, 0x03,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {6144000 , 96000, 0x01,    0x04, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {3072000 , 96000, 0x01,    0x08, 0x01,   0x01,   0x00,   0x00, 0xff, 0x04, 0x10,    0x10},
    {1536000 , 96000, 0x01,    0x08, 0x01,   0x01,   0x01,   0x00, 0x7f, 0x02, 0x10,    0x10},
};

static uint8_t dacVolBak, adcVolBak;
static bool isHasPA;
//static int slope, offset;
//static int gain0 = -955, gain100 = 320; // mul or divide 10 to a integer

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCS                                      *
 *----------------------------------------------------------------------------*/

static int32_t es8311WriteReg(uint8_t regAddr, uint16_t data)
{
    int32_t ret = 0;
    uint8_t rxNack = 0;
    uint8_t cmd[2] = {0};
    cmd[0]  = regAddr;
    cmd[1]  = data & 0xff;
    
    ret = halI2cWrite(ES8311_IICADDR, cmd, 2, &rxNack, true);

    if (rxNack == 1)
    {
        // if fail , write again
        ret = halI2cWrite(ES8311_IICADDR, cmd, 2, &rxNack, true);
    }
#if 0
#ifdef FEATURE_OS_ENABLE        
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311WriteReg_1, P_DEBUG, "reg write: reg=%x, data=%x", regAddr, data);
#else      
    printf("reg write: reg=%02x, data=%02x\n", regAddr, data);
#endif
#endif

    return ret;
}

static int32_t es8311ReadReg(uint8_t regAddr, uint8_t *retData)
{   
    return halI2cRead(ES8311_IICADDR, regAddr, retData, true);
}

static ARM_I2C_STATUS es8311GetStats()
{
    ARM_I2C_STATUS stats = {0};

    halI2cGetStats(&stats, true);
    return stats;
}

static int getCoeff(uint32_t mclk, uint32_t rate)
{
    for (int i = 0; i < (sizeof(coeffDiv) / sizeof(coeffDiv[0])); i++) {
        if (coeffDiv[i].rate == rate && coeffDiv[i].mclk == mclk)
            return i;
    }
    return -1;
}

// set es8311 dac mute or not. mute = 0, dac un-mute; mute = 1, dac mute
static void es8311Mute(HalCodecCfg_t* codecHalCfg, int mute)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311setmute_0, P_DEBUG, "Enter into es8311Mute(), mute = %d\n", mute);
    es8311SetVolume(codecHalCfg, 0);
}

// set es8311 into suspend mode
static void es8311Standby(uint8_t* dacVolB, uint8_t* adcVolB)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311suspend_0, P_DEBUG, "Enter into es8311 suspend");
    es8311ReadReg(ES8311_DAC_REG32, dacVolB);
    es8311ReadReg(ES8311_ADC_REG17, adcVolB);
    es8311WriteReg(ES8311_DAC_REG32,    0x00);
    es8311WriteReg(ES8311_ADC_REG17,    0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0E, 0xFF);
    es8311WriteReg(ES8311_SYSTEM_REG12, 0x02);
    es8311WriteReg(ES8311_SYSTEM_REG14, 0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0D, 0xFA);
    es8311WriteReg(ES8311_ADC_REG15,    0x00);
    es8311WriteReg(ES8311_DAC_REG37,    0x08);
    es8311WriteReg(ES8311_GP_REG45,     0x01);
}

// set es8311 ADC into suspend mode
static void es8311AdcStandby(uint8_t* adcVolB)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311suspend_1, P_DEBUG, "Enter into es8311 adc suspend");
    es8311ReadReg(ES8311_ADC_REG17, adcVolB);
    es8311WriteReg(ES8311_ADC_REG17,            0x00);
    es8311WriteReg(ES8311_SDPOUT_REG0A,         0x40);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x7f);
    es8311WriteReg(ES8311_SYSTEM_REG14,         0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x31);
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_RESET_REG00,          0x82);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x35);
}

// set es8311 DAC into suspend mode
static void es8311DacStandby(uint8_t* dacVolB)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311suspend_2, P_DEBUG, "Enter into es8311 dac suspend");
    es8311ReadReg(ES8311_DAC_REG32, dacVolB);
    es8311WriteReg(ES8311_DAC_REG32,            0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x0F);
    es8311WriteReg(ES8311_SYSTEM_REG12,         0x02);
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x09);
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_RESET_REG00,          0x81);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x3a);
}

// set es8311 into resume mode
static HalCodecSts_e es8311AllResume()
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311Resume_0, P_DEBUG, "Enter into es8311 resume");
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_GP_REG45,             0x00);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x3F);
    es8311WriteReg(ES8311_RESET_REG00,          0x80);
#ifdef FEATURE_OS_ENABLE
    osDelay(1);
#else    
    delay_us(1000);
#endif    
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_CLK_MANAGER_REG02,    0x00);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_ADC_REG15,            0x40);
    es8311WriteReg(ES8311_SYSTEM_REG14,         0x18);
    es8311WriteReg(ES8311_SYSTEM_REG12,         0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x00);
    es8311WriteReg(ES8311_DAC_REG32,            dacVolBak);
    es8311WriteReg(ES8311_ADC_REG17,            adcVolBak);
    
    return CODEC_EOK;
}

// set es8311 adc into resume mode
static HalCodecSts_e es8311AdcResume()
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311Resume_1, P_DEBUG, "Enter into es8311 adc resume");
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x3F);
    es8311WriteReg(ES8311_RESET_REG00,          0x80);
#ifdef FEATURE_OS_ENABLE
    osDelay(1);
#else    
    delay_us(1000);
#endif    
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_SYSTEM_REG14,         0x18);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x02);
    es8311WriteReg(ES8311_ADC_REG17,            adcVolBak);
#ifdef FEATURE_OS_ENABLE
    osDelay(50);
#else    
    delay_us(50*1000);
#endif    
    es8311WriteReg(ES8311_SDPOUT_REG0A,         0x00);
    return CODEC_EOK;
}

// set es8311 dac into resume mode
static HalCodecSts_e es8311DacResume()
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311Resume_2, P_DEBUG, "Enter into es8311 dac resume");
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x3F);
    es8311WriteReg(ES8311_RESET_REG00,          0x80);
#ifdef FEATURE_OS_ENABLE
    osDelay(1);
#else    
    delay_us(1000);
#endif    
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_SYSTEM_REG12,         0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x02);
    es8311WriteReg(ES8311_DAC_REG32,            dacVolBak);
    
    return CODEC_EOK;
}


// set es8311 into powerdown mode
static HalCodecSts_e es8311PwrDown()
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311pwrDown_0, P_DEBUG, "Enter into es8311 powerdown");
    es8311WriteReg(ES8311_DAC_REG32,            0x00);
    es8311WriteReg(ES8311_ADC_REG17,            0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0xff);
    es8311WriteReg(ES8311_SYSTEM_REG12,         0x02);
    es8311WriteReg(ES8311_SYSTEM_REG14,         0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0xf9);
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_DAC_REG37,            0x08);
    es8311WriteReg(ES8311_CLK_MANAGER_REG02,    0x10);
    es8311WriteReg(ES8311_RESET_REG00,          0x00);
#ifdef FEATURE_OS_ENABLE
    osDelay(1);
#else    
    delay_us(1000);
#endif 
    es8311WriteReg(ES8311_RESET_REG00,          0x1f);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x30);    
#ifdef FEATURE_OS_ENABLE
    osDelay(1);
#else    
    delay_us(1000);
#endif 
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x00);
    es8311WriteReg(ES8311_GP_REG45,             0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0xfc);
    es8311WriteReg(ES8311_CLK_MANAGER_REG02,    0x00);
    return CODEC_EOK;
}


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
 
// enable pa power
void es8311EnablePA(bool enable)
{
    GPIO_pinWrite(CODEC_PA_GPIO_INSTANCE, 1 << CODEC_PA_GPIO_PIN, enable ? (1 << CODEC_PA_GPIO_PIN) : 0);
}

HalCodecSts_e es8311Init(HalCodecCfg_t *codecCfg)
{
    uint8_t datmp, regv;
    int coeff;
    HalCodecSts_e ret = CODEC_EOK;
    getCoeff(0,0);
    
    halI2cInit(true);

#ifdef FEATURE_OS_ENABLE
#if (USE_NV_VOLUME)
    AudioParaCfgCommon_t mAudioCfgCommon;
    AudioParaCfgCodec_t  mwNvmAudioCodec;
    ecAudioCfgTlvStore *pMwNvmAudioCfg = NULL;

    // get default gain value
    memset((char *)&mAudioCfgCommon, 0x0, sizeof(mAudioCfgCommon));
    mAudioCfgCommon.device = codecCfg->codecDeviceType;
    mAudioCfgCommon.direct = DIRECTION_RX;

    pMwNvmAudioCfg = (ecAudioCfgTlvStore *)OsaAllocZeroMemory(sizeof(ecAudioCfgTlvStore)+ sizeof(AudioParaSphEQBiquard_t)*EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB*EC_ADCFG_SPEECH_TX_NUMB
                    + sizeof(AudioParaSphEQBiquard_t)*EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB*EC_ADCFG_SPEECH_RX_NUMB + sizeof(UINT16)*EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB*EC_ADCFG_SPEECH_RX_NUMB
                     + sizeof(UINT16)*EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB*EC_ADCFG_SPEECH_TX_NUMB);
    if (FALSE == mwNvmAudioCfgRead(pMwNvmAudioCfg))
    {
        DEBUG_PRINT(UNILOG_PLA_MIDWARE, ec8311DefaultCfg_1, P_WARNING, "MW NVM Audio, Read Audio Storage memory failure, try again !");
        if (FALSE == mwNvmAudioCfgRead(pMwNvmAudioCfg))
        {
            DEBUG_PRINT(UNILOG_PLA_MIDWARE, ec8311DefaultCfg_2, P_WARNING, "MW NVM Audio, Read Audio Storage memory failure !");
            OsaFreeMemory(&pMwNvmAudioCfg);
            return FALSE;
        }
    }
    
    mwNvmAudioCfgCodecGet(&mAudioCfgCommon, &mwNvmAudioCodec, pMwNvmAudioCfg);

    codecCfg->codecVolParam.defaultVal.rxDigGain0   = -955;//mwNvmAudioCodec.rxDigGain0;
    codecCfg->codecVolParam.defaultVal.rxDigGain100 = 320;//mwNvmAudioCodec.rxDigGain100;
    
    codecCfg->codecVolParam.offset = codecCfg->codecVolParam.defaultVal.rxDigGain0 * 128;
    codecCfg->codecVolParam.slope  = (codecCfg->codecVolParam.defaultVal.rxDigGain100 - codecCfg->codecVolParam.defaultVal.rxDigGain0) * 1024/100;
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_7, P_DEBUG, "offset: %d, slope:%d", codecCfg->codecVolParam.offset, codecCfg->codecVolParam.slope);
    OsaFreeMemory(&pMwNvmAudioCfg);
#endif    
#endif
    #if 0
    es8311WriteReg(ES8311_GP_REG45,             0x00);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x30);
    es8311WriteReg(ES8311_CLK_MANAGER_REG02,    0x10);

    // 256
    es8311WriteReg(ES8311_CLK_MANAGER_REG02,    0x00);
    es8311WriteReg(ES8311_CLK_MANAGER_REG03,    0x10);
    es8311WriteReg(ES8311_ADC_REG16,            0x21);
    es8311WriteReg(ES8311_CLK_MANAGER_REG04,    0x19);
    es8311WriteReg(ES8311_CLK_MANAGER_REG05,    0x00);

    // clk
    es8311WriteReg(ES8311_CLK_MANAGER_REG06,    (0<<5) + 4 -1);
    es8311WriteReg(ES8311_CLK_MANAGER_REG07,    0x00);
    es8311WriteReg(ES8311_CLK_MANAGER_REG08,    0xff);

    // sdp
    es8311WriteReg(ES8311_SDPIN_REG09,          (0<<7) + 0x00 + (0x00<<2));
    es8311WriteReg(ES8311_SDPOUT_REG0A,         0x00 + (0x00<<2));

    // system
    es8311WriteReg(ES8311_SYSTEM_REG0B,         0x00);
    es8311WriteReg(ES8311_SYSTEM_REG0C,         0x00);

    es8311WriteReg(ES8311_SYSTEM_REG10,         (0x1C*0) + (0x60*0x00) + 0x03);
    es8311WriteReg(ES8311_SYSTEM_REG11,         0x7F);
    es8311WriteReg(ES8311_RESET_REG00,          0x80);
    es8311WriteReg(ES8311_SYSTEM_REG0D,         0x01);
    es8311WriteReg(ES8311_CLK_MANAGER_REG01,    0x3F + (0x00<<7));
    es8311WriteReg(ES8311_SYSTEM_REG14,         0x18);
    es8311WriteReg(ES8311_SYSTEM_REG12,         0x28);
    es8311WriteReg(ES8311_SYSTEM_REG13,         0x00 + (0<<4));
    es8311WriteReg(ES8311_SYSTEM_REG0E,         0x02);
    es8311WriteReg(ES8311_SYSTEM_REG0F,         0x44);

    // adc
    es8311WriteReg(ES8311_ADC_REG15,            0x00);
    es8311WriteReg(ES8311_ADC_REG1B,            0x0a);
    es8311WriteReg(ES8311_ADC_REG1C,            0x6a);

    // dac
    es8311WriteReg(ES8311_DAC_REG37,            0x48);

    // gpio
    es8311WriteReg(ES8311_GPIO_REG44,           (0 <<7));

    
    es8311WriteReg(ES8311_ADC_REG17,            0x88);
    es8311WriteReg(ES8311_DAC_REG32,            0xf6);
    
   #else
   HalCodecIface_t *i2sCfg = &(codecCfg->codecIface);
    ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG01, 0x3F);

    // Select clock source for internal mclk
    es8311ReadReg(ES8311_CLK_MANAGER_REG01, &regv);

    // es8311 voltage check
    ARM_I2C_STATUS i2sStats = {0};
    uint8_t i2cCheckCnt = 3;
    do 
    {
        i2sStats.rx_nack = 0;
        i2sStats = es8311GetStats();
    } while ((i2sStats.rx_nack == 1) && i2cCheckCnt--);

    if (i2sStats.rx_nack == 1)
    {
        DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_0, P_DEBUG, "I2C not impassable, check pull-up register or voltage!!!");
        return CODEC_START_I2C_ERR;
    }
    ///////////////////////////////////////////////
    
    regv &= 0x7F;
    ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG01, regv);

    // Set clock parammeters
    int sampleFre = 0;
    int mclkFre = 0;
    switch (i2sCfg->samples) 
    {
        case CODEC_08K_SAMPLES:
            sampleFre = 8000;
            break;
        case CODEC_16K_SAMPLES:
            sampleFre = 16000;
            break;
        case CODEC_22K_SAMPLES:
            sampleFre = 22050;
            break;
        case CODEC_32K_SAMPLES:
            sampleFre = 32000;
            break;
        case CODEC_44K_SAMPLES:
            sampleFre = 44100;
            break;
        case CODEC_48K_SAMPLES:
            sampleFre = 48000;
            break;
        default:           
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_1, P_DEBUG, "Unable to configure sample rate %dHz", sampleFre);
            break;
    }
    mclkFre = sampleFre * MCLK_DIV_FRE;
    coeff = getCoeff(mclkFre, sampleFre);
    if (coeff < 0) 
    {
        DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_2, P_DEBUG, "Unable to configure sample rate %dHz with %dHz MCLK", sampleFre, mclkFre);
        return CODEC_INIT_ERR;
    }
    
    if (coeff >= 0) 
    {
        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG02) & 0x07;
        es8311ReadReg(ES8311_CLK_MANAGER_REG02, &regv);
        regv = regv & 0x07;
        regv |= (coeffDiv[coeff].preDiv - 1) << 5;
        datmp = 0;
        switch (coeffDiv[coeff].preMulti) 
        {
            case 1:
                datmp = 0;
                break;
            case 2:
                datmp = 1;
                break;
            case 4:
                datmp = 2;
                break;
            case 8:
                datmp = 3;
                break;
            default:
                break;
        }

        regv |= (datmp) << 3;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG02, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG05) & 0x00;
        es8311ReadReg(ES8311_CLK_MANAGER_REG05, &regv);
        regv = regv & 0x00;
        regv |= (coeffDiv[coeff].adcDiv - 1) << 4;
        regv |= (coeffDiv[coeff].dacDiv - 1) << 0;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG05, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG03) & 0x80;
        es8311ReadReg(ES8311_CLK_MANAGER_REG03, &regv);
        regv = regv & 0x80;
        regv |= coeffDiv[coeff].fsMode << 6;
        regv |= coeffDiv[coeff].adcOsr << 0;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG03, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG04) & 0x80;
        es8311ReadReg(ES8311_CLK_MANAGER_REG04, &regv);
        regv = regv & 0x80;
        regv |= coeffDiv[coeff].dacOsr << 0;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG04, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG07) & 0xC0;
        es8311ReadReg(ES8311_CLK_MANAGER_REG07, &regv);
        regv = regv & 0xc0;
        regv |= coeffDiv[coeff].lrckH << 0;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG07, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG08) & 0x00;
        es8311ReadReg(ES8311_CLK_MANAGER_REG08, &regv);
        regv = regv & 0x00;
        regv |= coeffDiv[coeff].lrckL << 0;
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG08, regv);

        //regv = es8311ReadReg(ES8311_CLK_MANAGER_REG06) & 0xE0;
        es8311ReadReg(ES8311_CLK_MANAGER_REG06, &regv);
        regv = regv & 0xe0;
        if (coeffDiv[coeff].bclkDiv < 19) 
        {
            regv |= (coeffDiv[coeff].bclkDiv - 1) << 0;
        } 
        else 
        {
            regv |= (coeffDiv[coeff].bclkDiv) << 0;
        }
        ret |= es8311WriteReg(ES8311_CLK_MANAGER_REG06, regv);
    }

    ret |= es8311WriteReg(ES8311_RESET_REG00,       0x80);
    
    // Set Codec into Master or Slave mode
    es8311ReadReg(ES8311_RESET_REG00, &regv);
    
    // Set master/slave audio interface
    switch (i2sCfg->mode) 
    {
        case CODEC_MODE_MASTER:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_3, P_DEBUG, "es8311 in master mode");
            regv |= 0x40;
            break;
        case CODEC_MODE_SLAVE:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_4, P_DEBUG, "es8311 in slave mode");
            regv &= 0xBF;
            break;
        default:
            regv &= 0xBF;
    }
    ret |= es8311WriteReg(ES8311_RESET_REG00, regv);
   
    ret |= es8311WriteReg(ES8311_SYSTEM_REG13, 0x00/*0x10*/);
    ret |= es8311WriteReg(ES8311_ADC_REG1B, 0x0A);
    ret |= es8311WriteReg(ES8311_ADC_REG1C, 0x6A);
    if (ret != CODEC_EOK)
    {
        DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311init_5, P_DEBUG, "es8311 initialize failed");
    }
    #endif

    if (codecCfg->hasPA)
    {       
        isHasPA = codecCfg->hasPA;
        
        // GPIO function select
        PadConfig_t     padConfig = {0};
        GpioPinConfig_t pinConfig = {0};
        
        PAD_getDefaultConfig(&padConfig);
        padConfig.mux = PAD_MUX_ALT0;
        PAD_setPinConfig(CODEC_PA_PAD_INDEX, &padConfig);

        
        // CODEC_PA pin config
        pinConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
        pinConfig.misc.initOutput = 0;  // when codec has been init, PA should open   
        GPIO_pinConfig(CODEC_PA_GPIO_INSTANCE, CODEC_PA_GPIO_PIN, &pinConfig);
        
        // enable pa power
        //es8311EnablePA(true);
    }
    
    return CODEC_EOK;
}

void es8311DeInit()
{
    //es8311PwrDown(0);
    halI2cDeInit(true);
}

HalCodecSts_e es8311ConfigFmt(HalCodecIfaceFormat_e fmt)
{
    HalCodecSts_e ret = CODEC_EOK;
    uint8_t adcIface = 0, dacIface = 0;
    es8311ReadReg(ES8311_SDPIN_REG09, &dacIface);
    es8311ReadReg(ES8311_SDPOUT_REG0A, &adcIface);
    switch (fmt) 
    {
        case CODEC_MSB_MODE:
        case CODEC_LSB_MODE:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311fmt_2, P_DEBUG, "ES8311 in left/right Format");
            adcIface &= 0xFC;
            dacIface &= 0xFC;
            adcIface |= 0x01;
            dacIface |= 0x01;
            break;

        case CODEC_I2S_MODE:          
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311fmt_1, P_DEBUG, "ES8311 in I2S Format");
            dacIface &= 0xFC;
            adcIface &= 0xFC;
            break;
            
        case CODEC_PCM_MODE:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311fmt_3, P_DEBUG, "ES8311 in pcm Format");
            adcIface &= 0xDC;
            dacIface &= 0xDC;
            adcIface |= 0x03;
            dacIface |= 0x03;
            break;
        default:
            dacIface &= 0xFC;
            adcIface &= 0xFC;
            break;
    }
    ret |= es8311WriteReg(ES8311_SDPIN_REG09, dacIface);
    ret |= es8311WriteReg(ES8311_SDPOUT_REG0A, adcIface);

    return ret;
}

HalCodecSts_e es8311SetBitsPerSample(HalCodecIfaceBits_e bits)
{
    HalCodecSts_e ret = CODEC_EOK;
    uint8_t adc_iface = 0, dac_iface = 0;
    es8311ReadReg(ES8311_SDPIN_REG09, &dac_iface);
    es8311ReadReg(ES8311_SDPOUT_REG0A, &adc_iface);
    switch (bits) 
    {
        case CODEC_BIT_LENGTH_16BITS:
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;
        case CODEC_BIT_LENGTH_24BITS:
            dac_iface &= ~0x1c;
            adc_iface &= ~0x1c;
            break;
        case CODEC_BIT_LENGTH_32BITS:
            dac_iface |= 0x10;
            adc_iface |= 0x10;
            break;
        default:
            dac_iface |= 0x0c;
            adc_iface |= 0x0c;
            break;

    }
    ret |= es8311WriteReg(ES8311_SDPIN_REG09, dac_iface);
    ret |= es8311WriteReg(ES8311_SDPOUT_REG0A, adc_iface);

    return ret;
}

HalCodecSts_e es8311Config(HalCodecMode_e mode, HalCodecIface_t *iface)
{
    int ret = CODEC_EOK;
    ret |= es8311SetBitsPerSample(iface->bits);
    ret |= es8311ConfigFmt(iface->fmt);
    return ret;
}

HalCodecSts_e es8311StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState)
{
    HalCodecSts_e ret = CODEC_EOK;

    switch(ctrlState)
    {
        case CODEC_CTRL_START:
        {
            ret |= es8311Start(mode);
            if (isHasPA)
            {
                es8311EnablePA(true);
            }
        }
        break;

        case CODEC_CTRL_STOP:
            ret |= es8311Stop(mode);
            break;
        case CODEC_CTRL_RESUME:
            ret |= es8311Resume(mode);
            break;
        case CODEC_CTRL_POWERDONW:
            ret |= es8311PwrDown(mode);
            break;
    }
    

    return ret;
}

HalCodecSts_e es8311Start(HalCodecMode_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;
    uint8_t adcIface = 0, dacIface = 0;

    //dacIface = es8311ReadReg(ES8311_SDPIN_REG09) & 0xBF;
    //adcIface = es8311ReadReg(ES8311_SDPOUT_REG0A) & 0xBF;
    es8311ReadReg(ES8311_SDPIN_REG09, &dacIface);
    dacIface = dacIface & 0xBF;
    es8311ReadReg(ES8311_SDPOUT_REG0A, &adcIface);
    adcIface = adcIface & 0xBF;
    adcIface |= BIT(6);
    dacIface |= BIT(6);
    
    if (mode == CODEC_MODE_ENCODE || mode == CODEC_MODE_BOTH) 
    {
        adcIface &= ~(BIT(6));
    }
    
    if (mode == CODEC_MODE_DECODE || mode == CODEC_MODE_BOTH) 
    {
        dacIface &= ~(BIT(6));
    }

    ret |= es8311WriteReg(ES8311_SDPIN_REG09,   dacIface);
    ret |= es8311WriteReg(ES8311_SDPOUT_REG0A,  adcIface);

    ret |= es8311WriteReg(ES8311_ADC_REG17,     0x88/*0xBF*/);
    ret |= es8311WriteReg(ES8311_SYSTEM_REG0E,  0x02);
    ret |= es8311WriteReg(ES8311_SYSTEM_REG12,  0x28/*0x00*/);
    ret |= es8311WriteReg(ES8311_SYSTEM_REG14,  0x18/*0x1A*/);

    // pdm dmic enable or disable
    uint8_t regv = 0;
    if (IS_DMIC) 
    {
        es8311ReadReg(ES8311_SYSTEM_REG14, &regv);
        regv |= 0x40;
        ret |= es8311WriteReg(ES8311_SYSTEM_REG14, regv);
    } 
    else 
    {
        es8311ReadReg(ES8311_SYSTEM_REG14, &regv);
        regv &= ~(0x40);
        ret |= es8311WriteReg(ES8311_SYSTEM_REG14, regv);
    }
    es8311WriteReg(ES8311_SYSTEM_REG14, 0x18);  //add for debug

    ret |= es8311WriteReg(ES8311_SYSTEM_REG0D, 0x01);
    ret |= es8311WriteReg(ES8311_ADC_REG15, 0x00/*0x40*/);
    ret |= es8311WriteReg(ES8311_DAC_REG37, 0x48);
    ret |= es8311WriteReg(ES8311_GP_REG45, 0x00);

    // set internal reference signal (ADCL + DACR)
    ret |= es8311WriteReg(ES8311_GPIO_REG44, 0x00/*0x50*/);

    return ret;
}

HalCodecSts_e es8311Stop(HalCodecMode_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;
    switch(mode)
    {
        case CODEC_MODE_ENCODE:
        es8311AdcStandby(&adcVolBak);
        break;

        case CODEC_MODE_DECODE:
        {
            es8311DacStandby(&dacVolBak);
            
            // disable  PA
            if (isHasPA)
            {
                es8311EnablePA(false);
            }
        }
        break;

        case CODEC_MODE_BOTH:
        es8311Standby(&dacVolBak, &adcVolBak);
        break;
        
        default:
        break;
    }
    
    return ret;
}

HalCodecSts_e es8311Resume(HalCodecMode_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;
    switch(mode)
    {
        case CODEC_MODE_ENCODE:
        es8311AdcResume();
        break;

        case CODEC_MODE_DECODE:
        {
            es8311DacResume();
            
            // enable  PA
            if (isHasPA)
            {
                es8311EnablePA(true);
            }
        }
        break;

        case CODEC_MODE_BOTH:
        es8311AllResume();
        break;
        
        default:
        break;
    }
    
    return ret;

}


HalCodecSts_e es8311SetVolume(HalCodecCfg_t* codecHalCfg, int volume)
{
    HalCodecSts_e res = CODEC_EOK;
#if (USE_NV_VOLUME)    
    int calGainDb = 0; // calculated db, should divide 10
    uint8_t regValWrite = 0;
#endif

    if (volume <= 0)
    {
        //return es8311SetMute(1);
        volume = 0;
    }

    if (volume >= 100)
    {
        volume = 100;
    }
    
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311SetVolume_1, P_DEBUG, "Es8311 Set volume:%d", volume);

#if (USE_NV_VOLUME)
    /* 1. get default tuning value

            2. Gain[db] = volume[%] * slope/1024[db] + offset/128[db]

                offset = Gain[0] * 128
                slope  = (Gain[100] - Gain[0]) * 1024 / 100

                offset and slope should stored into NV
        */

     // 1. cal needed db value
     calGainDb = (int)(volume * codecHalCfg->codecVolParam.slope/1024 + codecHalCfg->codecVolParam.offset/128);

     // 2. cal the writed reg val
     regValWrite = (calGainDb - codecHalCfg->codecVolParam.defaultVal.rxDigGain0)/5;

     // 3. write into reg
     res = es8311WriteReg(ES8311_DAC_REG32, regValWrite);
#else
    int vol = volume * 2550 / 1000; // volume * (rxDigGain100 - rxDigGain0) / 100
    res = es8311WriteReg(ES8311_DAC_REG32, vol);
#endif

    return res;
}

HalCodecSts_e es8311GetVolume(HalCodecCfg_t* codecHalCfg, int *volume)
{
    HalCodecSts_e res = CODEC_EOK;
    uint8_t regVal = 0;
#if (USE_NV_VOLUME)    
    int calGainDb = 0;
#endif

    es8311ReadReg(ES8311_DAC_REG32, &regVal);
    
#if (USE_NV_VOLUME)
    calGainDb = regVal*5 + codecHalCfg->codecVolParam.defaultVal.rxDigGain0;
    *volume = (calGainDb - codecHalCfg->codecVolParam.offset/128) * 1024 / codecHalCfg->codecVolParam.slope;
#else
    *volume = regVal * 1000 / 2550;
#endif

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311getVolume_1, P_DEBUG, "Get volume:%d reg_value:0x%x", *volume, regVal);
    return res;
}

HalCodecSts_e es8311SetMute(HalCodecCfg_t* codecHalCfg, bool enable)
{
#if 1
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311setMute_1, P_DEBUG, "Es8311SetVoiceMute:%d", enable);
    es8311Mute(codecHalCfg, enable);
#endif    
    return CODEC_EOK;
}

HalCodecSts_e es8311GetVoiceMute(int *mute)
{
    HalCodecSts_e res = CODEC_EOK;
    uint8_t reg = 0;
    res = es8311ReadReg(ES8311_DAC_REG31, &reg);
    if (res == CODEC_EOK) 
    {
        reg = (res & 0x20) >> 5;
    }
    *mute = reg;
    return res;
}

HalCodecSts_e es8311SetMicVolume(uint8_t micGain, int micVolume)
{
    HalCodecSts_e res = CODEC_EOK;

    if (micVolume <= 0)
    {
        //return es8311SetMute(1);
        micVolume = 0;
    }

    if (micVolume >= 100)
    {
        micVolume = 100;
    }

    int vol = micVolume * 2550 / 1000;
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311SetMicVolume_1, P_DEBUG, "Es8311 Set mic volume:%d", micVolume);
    
    res = es8311WriteReg(ES8311_SYSTEM_REG14, (1<<4 | micGain));
    res = es8311WriteReg(ES8311_ADC_REG17, vol);
    return res;
}

void es8311ReadAll()
{
    uint8_t reg = 0;

#ifdef FEATURE_OS_ENABLE        
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311ReadAll_1, P_DEBUG, "now read 8311 all");
#else      
    printf("now read 8311 all\n");
#endif

    for (int i = 0; i < 0x4A; i++) 
    {
        es8311ReadReg(i, &reg);
#ifdef FEATURE_OS_ENABLE        
        DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311ReadAll_2, P_DEBUG, "REG:%x, %x", reg, i);
#else      
        printf("reg = 0x%02x, val = 0x%02x\n", i, reg);
#endif
    }

#ifdef FEATURE_OS_ENABLE        
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8311ReadAll_3, P_DEBUG, "now read 8311 all end");
#else      
    printf("now read 8311 all end\n");
#endif
}

HalCodecCfg_t es8311GetDefaultCfg()
{
    HalCodecCfg_t codecCfg = ES8311_DEFAULT_CONFIG();

    return codecCfg;
}


