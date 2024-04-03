/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    hal_adc.c
 * Description:  EC618 adc hal driver source file
 * History:      Rev1.0   2019-12-12
 *               Rev1.1   2020-02-29  Add api to get internal thermal temperature
 *
 ****************************************************************************/

#include "adc.h"
#include "slpman.h"
#include "hal_trim.h"
#include "cmsis_os2.h"
#include "sctdef.h"



#define BSP_HALADC_TEXT_SECTION     SECTION_DEF_IMPL(.sect_hal_adc_text)
#define BSP_HALADC_RODATA_SECTION   SECTION_DEF_IMPL(.sect_hal_adc_rodata)
#define BSP_HALADC_DATA_SECTION     SECTION_DEF_IMPL(.sect_hal_adc_data)
#define BSP_HALADC_BSS_SECTION      SECTION_DEF_IMPL(.sect_hal_adc_bss)


typedef struct _adc_conversion_result
{
    volatile uint32_t rawCode;
    osSemaphoreId_t sem;
} adc_conversion_result_t;

BSP_HALADC_BSS_SECTION static adc_conversion_result_t g_adcVbatResult = {0};
BSP_HALADC_BSS_SECTION static adc_conversion_result_t g_adcThermalResult = {0};

/**
  \fn          void ADC_VbatCallback(uint32_t result)
  \brief       Vbat channel callback
  \return
*/
static void ADC_VbatCallback(uint32_t result)
{
    g_adcVbatResult.rawCode = result;

    if(g_adcVbatResult.sem)
    {
        osSemaphoreRelease(g_adcVbatResult.sem);
    }
}

/**
  \fn          void ADC_ThermalCallback(uint32_t result)
  \brief       Thermal channel callback
  \return
*/
static void ADC_ThermalCallback(uint32_t result)
{
    g_adcThermalResult.rawCode = result;

    if(g_adcThermalResult.sem)
    {
        osSemaphoreRelease(g_adcThermalResult.sem);
    }
}

/**
  \breif Calibarte ADC raw sample code
  \param[in] input     ADC conversion raw register value
  \return              calibrated voltage in unit of uV
  \details

  The voltage is calculated from formula:
      gain * input + offset                    where input is actual conversion data from ADC and gain and offset is read from EFUSE

 */
uint32_t HAL_ADC_CalibrateRawCode(uint32_t input)
{
    int32_t temp = 0;
    uint32_t result = 0;

    AdcEfuseCalCode_t * efuseCalcodePtr = trimAdcGetCalCode();

    // Resulotion is 12-bits
    input &= 0xFFFU;

    // convert to complement code
    if(efuseCalcodePtr->offset & 0x800)
    {
        temp = ~(efuseCalcodePtr->offset & 0x7FF) + 1;
    }
    else
    {
        temp = efuseCalcodePtr->offset;
    }

    temp = efuseCalcodePtr->gain * input + temp * 256;

    if (temp > 0)
    {
#if 0
        // original formula, yet it may overflow
        result = ((((uint32_t)temp) * 1000) + 2048)/ 4096;
#else
        result = ((((uint32_t)temp) * 125) + 256)/ 512;
#endif
    }

    return result;

}

/**
  \breif Convert ADC thermal channel raw sample to temperature in unit of centidegree
  \param[in] input     ADC thermal channel register value
  \return              temperature in unit of mili centidegree
  \details

  The empirical equation between temperature and ADC thermal ram sample is:

  T = k * Input + T0                                          (a)

  where k is slope and its value is approximately -0.16, T0 is bias and its value ranges from 400 to 500,
        Input is actual conversion data from ADC

  The actural k is deduced from EFUSE values with equation:

  k = -0.968 / 1.83 * (gain / 4096)                 (b)

  where gain is ADC calibration data stored in EFUSE, refer to previous comment in \ref HAL_ADC_CalibrateRawCode

  T0 = Toffset - k * Tcode                                    (c)

  where Toffset and Tcode are Thermal calibartion data also stored in EFUSE and Toffset is a number with accuracy of 2 decimal digits

  To get rid of float number calculation and balance accuracy loss, we here amplify the equation (a) by 4096*1000 = 4096000,

  that's T = (4096000 * k * Input + 4096000 * T0) / (4096000)            (d)

  Combine equation (c), we get

  T = (4096000 * k * (Input - Tcode) + 4096000 * Toffset) / 4096000    (e)

  Put equation (b) into (e) and note that Toffset has accuracy of 2 decimal digits and final result is in mili centidegree unit, perform some simplifying we get

  T = (-529 * gain * (Input - Tcode) + Toffset * 1024000) / 4096  (f)

  Also, round the result in the way: round(a/b) = (a + b/2) / b if a > 0 and round(a/b) = (a - b/2) / b if a < 0

 */
int32_t HAL_ADC_ConvertThermalRawCodeToTemperatureHighAccuracy(uint32_t input)
{
    int32_t gain = 0, temp = 0;

    AdcEfuseCalCode_t * efuseCalcodePtr = trimAdcGetCalCode();
    AdcEfuseT0Code_t * efuseT0CodePtr = trimAdcGetT0Code();

    // Resulotion is 12-bits
    input &= 0xFFFU;

#ifdef CHIP_EC718
    gain = -556 * efuseCalcodePtr->gain;
#elif defined CHIP_EC716
    gain = -602 * efuseCalcodePtr->gain;
#endif
    temp = gain * (int32_t)(input - efuseT0CodePtr->codet0) + (int32_t)(efuseT0CodePtr->t0 * 1024000);

    return (temp > 0) ? (temp + 2048) / 4096 : (temp - 2048) / 4096;

}


int32_t HAL_ADC_ConvertThermalRawCodeToTemperature(uint32_t input)
{
    return  HAL_ADC_ConvertThermalRawCodeToTemperatureHighAccuracy(input) / 1000;
}

int32_t HAL_ADC_SampleVbatVoltage(uint32_t timeout_ms)
{
    AdcConfig_t adcConfig;
    int ret = -1;

    g_adcVbatResult.sem = osSemaphoreNew(1U, 0, NULL);

    // semphore created succussful
    if(g_adcVbatResult.sem != NULL)
    {
        uint32_t adcReadyTimeout = 30; // at most 30ms

        while((slpManCheckADCReady() == false) && adcReadyTimeout)
        {
            osDelay(1);
            adcReadyTimeout--;
        }

        ADC_getDefaultConfig(&adcConfig);
#if defined CHIP_EC718
        adcConfig.channelConfig.vbatResDiv = ADC_VBAT_RESDIV_RATIO_8OVER32;
#elif defined CHIP_EC716
        adcConfig.channelConfig.vbatResDiv = ADC_VBAT_RESDIV_RATIO_6OVER32;
#endif
        ADC_channelInit(ADC_CHANNEL_VBAT, ADC_USER_APP, &adcConfig, ADC_VbatCallback);

        ADC_startConversion(ADC_CHANNEL_VBAT, ADC_USER_APP);

        ret = osSemaphoreAcquire(g_adcVbatResult.sem, timeout_ms);

        ADC_channelDeInit(ADC_CHANNEL_VBAT, ADC_USER_APP);

        if(ret == osOK)
        {
            ret = HAL_ADC_CalibrateRawCode(g_adcVbatResult.rawCode);
            // amplify the result by the reciprocal of div ratio
#if defined CHIP_EC718
            ret = ret * 4;
#elif defined CHIP_EC716
            ret = ret * 16 / 3;
#endif
            ret = (ret + 500) / 1000; // uV -> mV
        }

        osSemaphoreDelete(g_adcVbatResult.sem);
    }

    return ret;

}

int32_t HAL_ADC_GetThermalTemperature(uint32_t timeout_ms, int32_t* temperatruePtr)
{
    int ret = -1;

    if(temperatruePtr == NULL)
        return -1;

    g_adcThermalResult.sem = osSemaphoreNew(1U, 0, NULL);

    // semphore created succussful
    if(g_adcThermalResult.sem != NULL)
    {
        uint32_t adcReadyTimeout = 30; // at most 30ms

        while((slpManCheckADCReady() == false) && adcReadyTimeout)
        {
            osDelay(1);
            adcReadyTimeout--;
        }

        ADC_channelInit(ADC_CHANNEL_THERMAL, ADC_USER_APP, NULL, ADC_ThermalCallback);

        ADC_startConversion(ADC_CHANNEL_THERMAL, ADC_USER_APP);

        ret = osSemaphoreAcquire(g_adcThermalResult.sem, timeout_ms);

        ADC_channelDeInit(ADC_CHANNEL_THERMAL, ADC_USER_APP);

        if(ret == osOK)
        {
            *temperatruePtr = HAL_ADC_ConvertThermalRawCodeToTemperature(g_adcThermalResult.rawCode);
        }

        osSemaphoreDelete(g_adcThermalResult.sem);
    }

    return ret;

}

