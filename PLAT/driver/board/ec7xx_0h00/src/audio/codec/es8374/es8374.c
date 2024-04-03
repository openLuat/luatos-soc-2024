/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    es8374.c
 * Description:  EC7xx es8374 file
 * History:      Rev1.0   2021-9-18
 *
 ****************************************************************************/

#include "es8374.h"


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

//adcInput  单声道ADC输入通道选择是CH1(MIC1P/1N)还是CH2(MIC2P/2N)
//dacOutput 单声道DAC输出通道选择:默认选择0:L声道,1:R声道
//mode      产品主从模式选择:默认选择0为SlaveMode,打开为1选择MasterMode
//fmt       数据格式选择,需要和实际时序匹配
//bits      数据长度选择,需要和实际时序匹配
#define ES8374_DEFAULT_CONFIG()                         \
{                                                       \
        .adcInput  = CODEC_ADC_INPUT_LINE2,             \
        .dacOutput = CODEC_DAC_OUTPUT_LINE1,            \
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

#define VDDSPK_5V0              0x2B
#define VDDSPK_4V2              0x2A
#define VDDSPK_3V3              0x29

#define RATIO                   256                 //实际Ratio=MCLK/LRCK比率，需要和实际时钟比例匹配
#define SCLK_DIV                4                   //SCLK分频选择:(选择范围1~18),SCLK=MCLK/SCLK_DIV，超过后非等比增加具体对应关系见相应DS说明
#define SCLK_INV                0                   //默认对齐方式为下降沿,1为上升沿对齐,需要和实际时序匹配
#define VDDSPK_VOLTAGE          VDDSPK_4V2          //SPK拟电压选择为4V2还是5V0,需要和实际硬件匹配
#define ADC_PGA_DF2SE_15DB      1                   //ADC模拟固定15dB增益:默认选择关闭0,打开为1
#define ADC_PGA_GAIN            3                   //ADC模拟增益:(选择范围0~7),具体对应关系见相应DS说明
#define ADC_VOLUME_MAX          0                   //ADC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define ADC_VOLUME_MIN          50                  //ADC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define ADC_VOLUME_MUTE         192                 //ADC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define ADC_VOLUME_DEFAULT      ADC_VOLUME_MAX      //ADC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define DAC_VOLUME_MAX          0                   //DAC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define DAC_VOLUME_MIN          50                  //DAC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define DAC_VOLUME_MUTE         192                 //DAC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define DAC_VOLUME_DEFAULT      DAC_VOLUME_MAX      //DAC数字增益:(选择范围0~192),0:0DB,-0.5dB/Step
#define DMIC_SELON              0                   //DMIC选择:默认选择关闭0,立体声打开为1,单声道需要选择2为H,3为L
#define DMIC_GAIN               0                   //DMIC增益:(选择范围0~1),6dB/Step
#define MICBIASOFF              0                   //内部MICBIAS偏置:默认选择关闭1,开启配置为0
#define PA_VOLUME_DEFAULT       7                   //(0~7),(0db,1.5db,3db,4db,5db,6db,6.75db,7.5db)
//16K,8K采样率下配置PLL_SET_12288 = 1 可以改善音质，目前只支持256Ratio,其他待更新
#define PLL_SET_12288           1                   //PLL选择输入频率,单位为KHZ:外部输入XXXXKHZ内部PLL锁成12M288

/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

// 8374 func list
HalCodecFuncList_t es8374DefaultHandle = 
{
    .codecType                  = ES8374,
    .halCodecInitFunc           = es8374Init,
    .halCodecDeinitFunc         = es8374DeInit,
    .halCodecCtrlStateFunc      = es8374StartStop,
    .halCodecCfgIfaceFunc       = es8374Config,
    .halCodecSetMuteFunc        = es8374SetMute,
    .halCodecSetVolumeFunc      = es8374SetVolume,
    .halCodecGetVolumeFunc      = es8374GetVolume,
    //.halCodecEnablePAFunc       = es8374EnablePA,
    .halCodecSetMicVolumeFunc   = es8374SetMicVolume,
    .halCodecLock               = NULL,
    .handle                     = NULL,
    .halCodecGetDefaultCfg      = es8374GetDefaultCfg,
};



/*----------------------------------------------------------------------------*
 *                      PRIVATE VARIABLES                                     *
 *----------------------------------------------------------------------------*/

static uint8_t dacVolBak, adcVolBak;
static bool isHasPA;
static uint8_t adcInput  = 2;
static uint8_t dacOutput = 0;

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCS                                      *
 *----------------------------------------------------------------------------*/

static int32_t es8374WriteReg(uint8_t regAddr, uint16_t data)
{
    int32_t ret = 0;
    uint8_t rxNack = 0;
    
    uint8_t cmd[2] = {0};
    cmd[0]  = regAddr;
    cmd[1]  = data & 0xff;
    
    ret = halI2cWrite(ES8374_IICADDR, cmd, 2, &rxNack, false);

#if 0
#ifdef FEATURE_OS_ENABLE
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374WriteReg, P_DEBUG, "[ES8374] [%x] %x", regAddr, data);
#else      
    printf("[ES8374] [%02X] %02X\r\n", regAddr, data);
#endif
#endif

    return ret;
}

static int32_t es8374ReadReg(uint8_t regAddr, uint8_t* retData)
{
    return halI2cRead(ES8374_IICADDR, regAddr, retData, false);
}

// enable pa power
void es8374EnablePA(bool enable)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374EnablePA, P_DEBUG, "[ES8374] es8374EnablePA: enable=%d", enable);

#if 0
	if (enable)
	{
		es8374WriteReg(ES8374_REG_1E, 0x20);	// spk on
		delay_us(1000);
		es8374WriteReg(ES8374_REG_1E, 0xa0);	// spk on
	}
	else
	{
		es8374WriteReg(ES8374_REG_1E, 0x20);
		delay_us(1000);
		es8374WriteReg(ES8374_REG_1C, 0x10);
		es8374WriteReg(ES8374_REG_1D, 0x10);
		es8374WriteReg(ES8374_REG_1E, 0x40);	// spk off
	}
#endif
}


// set es8374 into suspend mode
 void es8374Standby(uint8_t* dacVolB, uint8_t* adcVolB)//待机配置--搭配es8374AllResume(void)//恢复配置
{
    es8374ReadReg(ES8374_REG_38, dacVolB);
    es8374ReadReg(ES8374_REG_25, adcVolB);

    es8374WriteReg(ES8374_REG_38, 0xC0);
    es8374WriteReg(ES8374_REG_25, 0xC0);
    es8374WriteReg(ES8374_REG_28, 0x1C);
    es8374WriteReg(ES8374_REG_36, 0x20);
    es8374WriteReg(ES8374_REG_37, 0x20);
    es8374WriteReg(ES8374_REG_6D, 0x00);
    es8374WriteReg(ES8374_REG_09, 0x80);
    es8374WriteReg(ES8374_REG_1A, 0x08);
    es8374WriteReg(ES8374_REG_1E, 0x20);
    delay_us(1000);
    es8374WriteReg(ES8374_REG_1C, 0x10);
    es8374WriteReg(ES8374_REG_1D, 0x10);
    es8374WriteReg(ES8374_REG_1E, 0x40);
    es8374WriteReg(ES8374_REG_24, 0x20);
    es8374WriteReg(ES8374_REG_22, 0x00);
    es8374WriteReg(ES8374_REG_21, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0xFF);
    es8374WriteReg(ES8374_REG_14, 0x82);
    es8374WriteReg(ES8374_REG_01, 0x03);

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Standby, P_DEBUG, "[ES8374] es8374Standby: dacVolB=%d, adcVolB=%d", *dacVolB, *adcVolB);
}

#if 1
// set es8374 ADC into suspend mode
 void es8374AdcStandby(uint8_t* adcVolB)//ADC待机配置--搭配es8374AdcResume
{
    es8374ReadReg(ES8374_REG_25, adcVolB);

    es8374WriteReg(ES8374_REG_25, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_28, 0x1C);
    es8374WriteReg(ES8374_REG_09, 0x80);
    delay_us(5000);
    es8374WriteReg(ES8374_REG_24, 0x20);
    es8374WriteReg(ES8374_REG_21, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0x0C);
    es8374WriteReg(ES8374_REG_01, 0x75);

}

// set es8374 DAC into suspend mode
 void es8374DacStandby(uint8_t* dacVolB)//DAC待机配置--搭配es8374DacResume
{
    es8374ReadReg(ES8374_REG_38, dacVolB);

    es8374WriteReg(ES8374_REG_38, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_36, 0x20);
    es8374WriteReg(ES8374_REG_37, 0x20);
    es8374WriteReg(ES8374_REG_6D, 0x00);
    es8374WriteReg(ES8374_REG_09, 0x80);
    es8374WriteReg(ES8374_REG_1A, 0x08);
	es8374WriteReg(ES8374_REG_1E, 0x20);
	delay_us(1000);
	es8374WriteReg(ES8374_REG_1C, 0x10);
	es8374WriteReg(ES8374_REG_1D, 0x10);
	//es8374WriteReg(ES8374_REG_1E, 0x40);	// spk off
    es8374WriteReg(ES8374_REG_15, 0x62);
    es8374WriteReg(ES8374_REG_01, 0x7A);

}
#endif
#if 1
// set es8374 adc into resume mode
static HalCodecSts_e es8374AdcResume(void)//ADC恢复配置--搭配es8374AdcStandby
{

    es8374WriteReg(ES8374_REG_01, 0x7F);
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_21, (adcInput << 4) + (ADC_PGA_DF2SE_15DB << 2));
    es8374WriteReg(ES8374_REG_24, 0x08 + (DMIC_GAIN << 7) + DMIC_SELON);
    es8374WriteReg(ES8374_REG_09, 0x41);
    es8374WriteReg(ES8374_REG_28, 0x00);
    es8374WriteReg(ES8374_REG_15, 0x40);
    es8374WriteReg(ES8374_REG_25, adcVolBak);

    return CODEC_EOK;
}

// set es8374 dac into resume mode
static HalCodecSts_e es8374DacResume(void)//DAC恢复配置--搭配es8374DacStandby
{

    es8374WriteReg(ES8374_REG_01, 0x7F);
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_21, 0x50);
	es8374WriteReg(ES8374_REG_1E, 0x20);	// spk on
	delay_us(1000);
	es8374WriteReg(ES8374_REG_1E, 0xa0);	// spk on
    es8374WriteReg(ES8374_REG_1D, VDDSPK_VOLTAGE);
    es8374WriteReg(ES8374_REG_1C, 0x90);
    es8374WriteReg(ES8374_REG_1A, 0xA0);
    es8374WriteReg(ES8374_REG_09, 0x41);
    es8374WriteReg(ES8374_REG_6D, 0x60);
    es8374WriteReg(ES8374_REG_37, 0x00);
    es8374WriteReg(ES8374_REG_36, (dacOutput << 6));
    es8374WriteReg(ES8374_REG_15, 0x40);
    es8374WriteReg(ES8374_REG_38, dacVolBak);

    return CODEC_EOK;
}
#endif


// set es8374 into resume mode
static HalCodecSts_e es8374AllResume(void)//恢复配置(未下电)--搭配es8374Standby(void)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374AllResume, P_DEBUG, "[ES8374] es8374AllResume: dacVolBak=%d, adcVolBak=%d", dacVolBak, adcVolBak);

    es8374WriteReg(ES8374_REG_01, 0x7F);
    es8374WriteReg(ES8374_REG_14, 0x8A + (MICBIASOFF << 4));
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_21, 0x50);
    es8374WriteReg(ES8374_REG_22, ADC_PGA_GAIN + (ADC_PGA_GAIN << 4));
    es8374WriteReg(ES8374_REG_21, (adcInput << 4) + (ADC_PGA_DF2SE_15DB << 2));
    es8374WriteReg(ES8374_REG_24, 0x08 + (DMIC_GAIN << 7) + DMIC_SELON);
	es8374WriteReg(ES8374_REG_1E, 0x20);	// spk on
	delay_us(1000);
	es8374WriteReg(ES8374_REG_1E, 0xa0);	// spk on
    es8374WriteReg(ES8374_REG_1D, VDDSPK_VOLTAGE);
    es8374WriteReg(ES8374_REG_1C, 0x90);
    es8374WriteReg(ES8374_REG_1A, 0xA0);
    es8374WriteReg(ES8374_REG_09, 0x41);
    es8374WriteReg(ES8374_REG_6D, 0x60);
    es8374WriteReg(ES8374_REG_37, 0x00);
    es8374WriteReg(ES8374_REG_36, (dacOutput << 6));
    es8374WriteReg(ES8374_REG_28, 0x00);
    es8374WriteReg(ES8374_REG_15, 0x40);
    es8374WriteReg(ES8374_REG_38, dacVolBak);
    es8374WriteReg(ES8374_REG_25, adcVolBak);

    return CODEC_EOK;
}



// set es8374 into powerdown mode
static HalCodecSts_e es8374PwrDown(HalCodecModule_e mode)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374PwrDown, P_DEBUG, "[ES8374] es8374PwrDown: mode=%d", mode);

    es8374WriteReg(ES8374_REG_38, 0xC0);
    es8374WriteReg(ES8374_REG_25, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0x00);
    es8374WriteReg(ES8374_REG_28, 0x1C);
    es8374WriteReg(ES8374_REG_36, 0x20);
    es8374WriteReg(ES8374_REG_37, 0x20);
    es8374WriteReg(ES8374_REG_6D, 0x00);
    es8374WriteReg(ES8374_REG_09, 0x80);
    es8374WriteReg(ES8374_REG_1A, 0x08);
    es8374WriteReg(ES8374_REG_1E, 0x20);
    delay_us(1000);
    es8374WriteReg(ES8374_REG_1C, 0x10);
    es8374WriteReg(ES8374_REG_1D, 0x10);
    es8374WriteReg(ES8374_REG_1E, 0x40);
    es8374WriteReg(ES8374_REG_24, 0x20);
    es8374WriteReg(ES8374_REG_22, 0x00);
    es8374WriteReg(ES8374_REG_21, 0xC0);
    es8374WriteReg(ES8374_REG_15, 0xFF);
    es8374WriteReg(ES8374_REG_14, 0x16);
    es8374WriteReg(ES8374_REG_01, 0x03);

    return CODEC_EOK;
}


/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
 


HalCodecSts_e es8374Init(HalCodecCfg_t *codecCfg)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Init_1, P_DEBUG, "[ES8374] es8374Init_1: adcInput=%d, dacOutput=%d, codecMode=%d, hasPA=%d",
                codecCfg->adcInput, codecCfg->dacOutput, codecCfg->codecMode, codecCfg->hasPA);
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Init_2, P_DEBUG, "[ES8374] es8374Init_2: mode=%d, fmt=%d, samples=%d, bits=%d, channel=%d, polarity=%d",
                codecCfg->codecIface.mode, codecCfg->codecIface.fmt, codecCfg->codecIface.samples, codecCfg->codecIface.bits, codecCfg->codecIface.channel, codecCfg->codecIface.polarity);

    //codecI2cInit();
    halI2cInit(false);

    switch (codecCfg->adcInput)
    {
        case CODEC_ADC_INPUT_LINE1:
            adcInput = 1;
            break;

        case CODEC_ADC_INPUT_LINE2:
            adcInput = 2;
            break;

        case CODEC_ADC_INPUT_ALL:
            adcInput = 3;
            break;

        case CODEC_ADC_INPUT_DIFFERENCE:
            break;

        default:
            break;
    }

    switch (codecCfg->dacOutput)
    {
        case CODEC_DAC_OUTPUT_LINE1:
            dacOutput = 0;
            break;

        case CODEC_DAC_OUTPUT_LINE2:
            dacOutput = 1;
            break;

        case CODEC_DAC_OUTPUT_ALL:
            break;

        default:
            break;
    }

    es8374WriteReg(ES8374_REG_00, 0x3F);            //IC Rst start
    es8374WriteReg(ES8374_REG_00, 0x03);            //IC Rst stop
    es8374WriteReg(ES8374_REG_01, 0x7F);            //IC clk on
    es8374WriteReg(ES8374_REG_02, 0x00);
    es8374WriteReg(ES8374_REG_03, 0x20);
    es8374WriteReg(ES8374_REG_04, 0x00);
    es8374WriteReg(ES8374_REG_05, 0x11);            //clk div set
    if(PLL_SET_12288 == 1)
    {
        es8374WriteReg(ES8374_REG_6F, 0xA0);        //pll set:mode enable
        es8374WriteReg(ES8374_REG_72, 0x41);        //pll set:mode set
        es8374WriteReg(ES8374_REG_09, 0x01);        //pll set:reset on ,set start
        es8374WriteReg(ES8374_REG_0A, 0x8A);
        es8374WriteReg(ES8374_REG_0B, 0x0C);
        es8374WriteReg(ES8374_REG_0C, 0x00);
        es8374WriteReg(ES8374_REG_0D, 0x00);
        es8374WriteReg(ES8374_REG_0E, 0x00);
        es8374WriteReg(ES8374_REG_09, 0x41);        //pll set:reset off ,set stop    
        es8374WriteReg(ES8374_REG_02, 0x08);        //pll set:use pll
        es8374WriteReg(ES8374_REG_03, 0x30);        //384 OSR
        es8374WriteReg(ES8374_REG_04, 0x20);        //384 OSR
    }
    es8374WriteReg(ES8374_REG_06, (RATIO >> 8));    //LRCK DIV
    es8374WriteReg(ES8374_REG_07, (RATIO & 0xFF));  //LRCK DIV
    es8374SetBitsPerSample(codecCfg->codecIface.bits);
    es8374ConfigFmt(codecCfg->codecIface.fmt);
    es8374WriteReg(ES8374_REG_0F, (((codecCfg->codecIface.mode == CODEC_MODE_SLAVE) ? 0 : 1) << 7) + (SCLK_INV<<5) + SCLK_DIV);
    es8374WriteReg(ES8374_REG_24, 0x08 + (DMIC_GAIN << 7) + DMIC_SELON);    //adc set
    es8374WriteReg(ES8374_REG_36, (dacOutput << 6));                        //dac set
    es8374WriteReg(ES8374_REG_12, 0x30);                                    //timming set
    es8374WriteReg(ES8374_REG_13, 0x20);                                    //timming set
    es8374WriteReg(ES8374_REG_18, 0xFF);
    es8374WriteReg(ES8374_REG_21, 0x50);
    es8374WriteReg(ES8374_REG_22, ADC_PGA_GAIN + (ADC_PGA_GAIN << 4));      //adc set
    es8374WriteReg(ES8374_REG_21, (adcInput << 4) + (ADC_PGA_DF2SE_15DB << 2));
    es8374WriteReg(ES8374_REG_00, 0x80);                // IC START
    delay_us(10000);
    es8374WriteReg(ES8374_REG_14, 0x8A + (MICBIASOFF << 4));
    es8374WriteReg(ES8374_REG_15, 0x40);
    es8374WriteReg(ES8374_REG_1A, 0xA0);                // monoout set
    es8374WriteReg(ES8374_REG_1B, 0x19);                //5V 0DB
    es8374WriteReg(ES8374_REG_1C, 0x90);
    es8374WriteReg(ES8374_REG_1D, VDDSPK_VOLTAGE);      //5V 0DB 8R=>1W
    es8374WriteReg(ES8374_REG_1F, 0x00);                // spk set
    es8374WriteReg(ES8374_REG_20, 0x00);                // spk set
	es8374WriteReg(ES8374_REG_1E, 0x20);	// spk on
	delay_us(1000);
	es8374WriteReg(ES8374_REG_1E, 0xa0);	// spk on
    es8374WriteReg(ES8374_REG_28, 0x00);                // alc set
    es8374WriteReg(ES8374_REG_25, ADC_VOLUME_DEFAULT);  // ADCVOLUME  on
    es8374WriteReg(ES8374_REG_38, DAC_VOLUME_DEFAULT);  // DACVOLUMEL on
    es8374WriteReg(ES8374_REG_37, 0x00);                // dac set
    es8374WriteReg(ES8374_REG_6D, 0x60);                // SEL:GPIO1=DMIC CLK OUT+SEL:GPIO2=PLL CLK OUT

    isHasPA = codecCfg->hasPA;

    return CODEC_EOK;
}

void es8374DeInit()
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374DeInit, P_DEBUG, "[ES8374] es8374DeInit");
    
    halI2cDeInit(false);
}

HalCodecSts_e es8374ConfigFmt(HalCodecIfaceFormat_e fmt)
{
    HalCodecSts_e ret = CODEC_EOK;
    uint8_t       adc = 0, dac = 0; 
    es8374ReadReg(ES8374_REG_10, &adc);
    es8374ReadReg(ES8374_REG_11, &dac);
    adc = adc & 0xdc;
    dac = dac & 0xdc;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374ConfigFmt, P_DEBUG, "[ES8374] es8374ConfigFmt: fmt=%d", fmt);

    switch (fmt) 
    {
        case CODEC_MSB_MODE:
        case CODEC_LSB_MODE:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374fmt_1, P_DEBUG, "[ES8374] left/right Format");
            adc |= 0x01;
            dac |= 0x01;
            break;

        case CODEC_I2S_MODE:          
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374fmt_2, P_DEBUG, "[ES8374] I2S Format");
            adc |= 0x00;
            dac |= 0x00;
            break;

        case CODEC_PCM_MODE:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374fmt_3, P_DEBUG, "[ES8374] pcm Format");
            adc |= 0x03;
            dac |= 0x03;
            break;

        default:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374fmt_5, P_DEBUG, "[ES8374] default Format (I2S)");
            adc |= 0x00;
            dac |= 0x00;
            break;
    }

    ret |= es8374WriteReg(ES8374_REG_10, adc);
    ret |= es8374WriteReg(ES8374_REG_11, dac);

    return ret;
}

HalCodecSts_e es8374SetBitsPerSample(HalCodecIfaceBits_e bits)
{
    HalCodecSts_e ret = CODEC_EOK;
    uint8_t       adc = 0, dac = 0; 
    es8374ReadReg(ES8374_REG_10, &adc);
    es8374ReadReg(ES8374_REG_11, &dac);
    adc = adc & 0xe3;
    dac = dac & 0xe3;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetBitsPerSample, P_DEBUG, "[ES8374] es8374SetBitsPerSample: bits=%d", bits);

    switch (bits) 
    {
        case CODEC_BIT_LENGTH_16BITS:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetBitsPerSample_1, P_DEBUG, "[ES8374] 16 bits");
            adc |= 0x0C;
            dac |= 0x0C;
            break;

        case CODEC_BIT_LENGTH_24BITS:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetBitsPerSample_2, P_DEBUG, "[ES8374] 24 bits");
            adc |= 0x00;
            dac |= 0x00;
            break;

        case CODEC_BIT_LENGTH_32BITS:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetBitsPerSample_3, P_DEBUG, "[ES8374] 32 bits");
            adc |= 0x10;
            dac |= 0x10;
            break;

        default:
            DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetBitsPerSample_5, P_DEBUG, "[ES8374] default bit length (16 bits)");
            adc |= 0x0C;
            dac |= 0x0C;
            break;
    }

    ret |= es8374WriteReg(ES8374_REG_10, adc);
    ret |= es8374WriteReg(ES8374_REG_11, dac);

    return ret;
}

HalCodecSts_e es8374Config(HalCodecMode_e mode, HalCodecIface_t *iface)
{
    HalCodecSts_e ret = CODEC_EOK;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Config, P_DEBUG, "[ES8374] es8374Config: mode=%d, bits=%d, fmt=%d", mode, iface->bits, iface->fmt);

    ret |= es8374SetBitsPerSample(iface->bits);
    ret |= es8374ConfigFmt(iface->fmt);

    return ret;
}

HalCodecSts_e es8374StartStop(HalCodecMode_e mode, HalCodecCtrlState_e ctrlState)
{
    HalCodecSts_e ret = CODEC_EOK;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374StartStop, P_DEBUG, "[ES8374] es8374StartStop: mode=%d, ctrlState=%d", mode, ctrlState);
    switch(ctrlState)
    {
        case CODEC_CTRL_START:
            ret |= es8374Start(mode);
            if (isHasPA)
            {
                //es8374EnablePA(true);
            }
            break;
        case CODEC_CTRL_STOP:
            ret |= es8374Stop(mode);
            break;
        case CODEC_CTRL_RESUME:
            ret |= es8374Resume(mode);
            break;
        case CODEC_CTRL_POWERDONW:
            ret |= es8374PwrDown(mode);
            break;
    }
    

    return ret;
}

HalCodecSts_e es8374Start(HalCodecModule_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Start, P_DEBUG, "[ES8374] es8374Start: mode=%d", mode);

    return ret;
}

HalCodecSts_e es8374Stop(HalCodecModule_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Stop, P_DEBUG, "[ES8374] es8374Stop: mode=%d", mode);
    switch(mode)
    {
        case CODEC_MODE_ENCODE:
            es8374AdcStandby(&adcVolBak);
            break;

        case CODEC_MODE_DECODE:
            es8374DacStandby(&dacVolBak);
            if (isHasPA)
            {
                delay_us(2000);
                //es8374EnablePA(false);
            }
            break;

        case CODEC_MODE_BOTH:
            es8374Standby(&dacVolBak, &adcVolBak);
            break;

        default:
        break;
    }

    return ret;
}

HalCodecSts_e es8374Resume(HalCodecMode_e mode)
{
    HalCodecSts_e ret = CODEC_EOK;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374Resume, P_DEBUG, "[ES8374] es8374Resume: mode=%d", mode);
    switch(mode)
    {
        case CODEC_MODE_ENCODE:
			//es8374AllResume();

            es8374AdcResume();
            break;

        case CODEC_MODE_DECODE:
			//es8374AllResume();

            es8374DacResume();
            if (isHasPA)
            {
                //es8374EnablePA(true);
            }
            break;

        case CODEC_MODE_BOTH:
            es8374AllResume();
            break;

        default:
            break;
    }

    return ret;
}

HalCodecSts_e es8374SetVolume(HalCodecCfg_t* codecHalCfg, int volume)
{
    uint8_t level = volume;
    uint8_t value = DAC_VOLUME_MUTE;

    if (level > 0)
    {
        level = (level > 100) ? 100 : level;
        value = DAC_VOLUME_MIN - (level * DAC_VOLUME_MIN / 100);
    }

    es8374WriteReg(ES8374_REG_38, value);
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetVolume, P_DEBUG, "[ES8374] es8374SetVolume: volume=%d, level=%d, value=%d", volume, level, value);

    return CODEC_EOK;
}

HalCodecSts_e es8374GetVolume(HalCodecCfg_t* codecHalCfg, int *volume)
{
    uint8_t level = 0;
    uint8_t value = 0;
    es8374ReadReg(ES8374_REG_38, &value);

    if (value <= DAC_VOLUME_MIN)
    {
        level = 100 - (value * 100 / DAC_VOLUME_MIN);
        level = (level == 0) ? 1 : level;
    }

    *volume = level;
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374GetVolume, P_DEBUG, "[ES8374] es8374GetVolume: value=%d, level=%d", value, level);

    return CODEC_EOK;
}

HalCodecSts_e es8374SetMute(HalCodecCfg_t* codecHalCfg, bool enable)
{
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetMute, P_DEBUG, "[ES8374] es8374SetMute: enable=%d", enable);

    if (enable == true)
    {
        es8374WriteReg(ES8374_REG_36, 0x20);
    }
    else
    {
        es8374WriteReg(ES8374_REG_36, (dacOutput << 6));
    }

    return CODEC_EOK;
}

HalCodecSts_e es8374GetVoiceMute(uint8_t *mute)
{
    //*mute = (es8374ReadReg(ES8374_REG_36) >> 5) & 0x01;
    es8374ReadReg(ES8374_REG_36, mute);
    *mute = (*mute >> 5) & 0x01;

    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374GetVoiceMute, P_DEBUG, "[ES8374] es8374GetVoiceMute: mute=%d", *mute);

    return CODEC_EOK;
}

HalCodecSts_e es8374SetMicVolume(uint8_t micGain, int micVolume)
{
    uint8_t level = micVolume;
    uint8_t value = ADC_VOLUME_MUTE;

    if (level > 0)
    {
        level = (level > 100) ? 100 : level;
        value = ADC_VOLUME_MIN - (level * ADC_VOLUME_MIN / 100);
    }

    es8374WriteReg(ES8374_REG_25, value);
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374SetMicVolume, P_DEBUG, "[ES8374] es8374SetMicVolume: micVolume=%d, level=%d, value=%d", micVolume, level, value);

    return CODEC_EOK;
}

void es8374ReadAll()
{
    uint8_t reg = 0;

#ifdef FEATURE_OS_ENABLE        
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374ReadAll_1, P_DEBUG, "[ES8374] es8374ReadAll Begin");
#else      
    printf("[ES8374] es8374ReadAll Begin\r\n");
#endif

    for (int i = 0; i < 0x6E; i++) 
    {
        es8374ReadReg(i, &reg);
#ifdef FEATURE_OS_ENABLE        
        DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374ReadAll_2, P_DEBUG, "[ES8374] [%x] %x", reg, i);
#else      
        printf("[ES8374] [%02X] %02X\r\n", i, reg);
#endif
    }

#ifdef FEATURE_OS_ENABLE        
    DEBUG_PRINT(UNILOG_PLA_DRIVER, es8374ReadAll_3, P_DEBUG, "[ES8374] es8374ReadAll End");
#else      
    printf("[ES8374] es8374ReadAll End\r\n");
#endif
}

HalCodecCfg_t es8374GetDefaultCfg()
{
    HalCodecCfg_t codecCfg = ES8374_DEFAULT_CONFIG();

    return codecCfg;
}
