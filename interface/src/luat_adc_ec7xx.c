/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

/*
 * ADC操作
 * 
 */
#include "common_api.h"
#include "FreeRTOS.h"
#include "task.h"

#include "luat_base.h"
#ifdef __LUATOS__
#include "luat_malloc.h"
#include "luat_msgbus.h"
#include "luat_timer.h"
#endif
#include "luat_adc.h"

#include "adc.h"
#include "hal_adc.h"
#include "ic.h"
#include "hal_trim.h"


/*
注意, 按硬件应用手册的描述, ADC0 对应的是 AIO3, ADC1 对应的 AIO4

而 VBAT和TEMP 则使用LuatOS定义的, 与硬件无关.

*/

// 老的ID, 兼容一下
#define ADC_CH_CPU_OLD 10
#define ADC_CH_VBAT_OLD 11

typedef struct
{
    LUAT_ADC_RANGE_E range;
    AdcAioResDiv_e resdiv;
    float ratio;
}adc_range_resdiv_map_item_t;

extern void delay_us(uint32_t us);

static uint8_t adc_state[6] = {0};
static LUAT_ADC_RANGE_E adc_range[6] = {LUAT_ADC_AIO_RANGE_4_0, LUAT_ADC_AIO_RANGE_4_0, LUAT_ADC_AIO_RANGE_4_0, LUAT_ADC_AIO_RANGE_4_0, LUAT_ADC_VBAT_RANGE_5_3_RATIO, LUAT_ADC_VBAT_RANGE_5_3_RATIO}; 
static volatile uint32_t adc0ChannelResult = 0;
static volatile uint32_t adc1ChannelResult = 0;
static volatile uint32_t adc2ChannelResult = 0;
static volatile uint32_t adc3ChannelResult = 0;
static volatile uint32_t vbatChannelResult = 0;
static volatile uint32_t thermalChannelResult = 0;

static int adc_exist(int id) {
#ifndef TYPE_EC716S
    if (id >= 0 && id < 5)
        return 1;
#else
    if (id >= 0 && id < 2)
        return 1;
#endif
    if (id == LUAT_ADC_CH_CPU || id == LUAT_ADC_CH_VBAT)
        return 1;
    if (id == ADC_CH_CPU_OLD || id == ADC_CH_VBAT_OLD)
        return 1;
    return 0;
}

static int adc_range_to_resdiv(int id, int range, AdcAioResDiv_e *resdiv, float *ratio)
{
    adc_range_resdiv_map_item_t map[]=
    {
        {LUAT_ADC_AIO_RANGE_1_2, ADC_AIO_RESDIV_BYPASS, (float)1},
#if defined TYPE_EC716S
        {LUAT_ADC_AIO_RANGE_1_4, ADC_AIO_RESDIV_RATIO_28OVER32, (float)32/28},
        {LUAT_ADC_AIO_RANGE_1_6, ADC_AIO_RESDIV_RATIO_24OVER32, (float)32/24},
#else
        {LUAT_ADC_AIO_RANGE_1_4, ADC_AIO_RESDIV_BYPASS, (float)1},
        {LUAT_ADC_AIO_RANGE_1_6, ADC_AIO_RESDIV_BYPASS, (float)1},
#endif
        {LUAT_ADC_AIO_RANGE_1_9, ADC_AIO_RESDIV_RATIO_16OVER32, (float)32/16},
        {LUAT_ADC_AIO_RANGE_2_6, ADC_AIO_RESDIV_RATIO_12OVER32, (float)32/12},
        {LUAT_ADC_AIO_RANGE_4_0, ADC_AIO_RESDIV_RATIO_8OVER32, (float)32/8},
        {LUAT_ADC_AIO_RANGE_MAX, ADC_AIO_RESDIV_RATIO_8OVER32, (float)32/8},
        
        {LUAT_ADC_VBAT_RANGE_2_0_RATIO, ADC_VBAT_RESDIV_RATIO_8OVER32, (float)32/8},
        {LUAT_ADC_VBAT_RANGE_2_2_RATIO, ADC_VBAT_RESDIV_RATIO_7OVER32, (float)32/7},
        {LUAT_ADC_VBAT_RANGE_2_6_RATIO, ADC_VBAT_RESDIV_RATIO_6OVER32, (float)32/6},
        {LUAT_ADC_VBAT_RANGE_3_2_RATIO, ADC_VBAT_RESDIV_RATIO_5OVER32, (float)32/5},
        {LUAT_ADC_VBAT_RANGE_4_0_RATIO, ADC_VBAT_RESDIV_RATIO_4OVER32, (float)32/4},
        {LUAT_ADC_VBAT_RANGE_5_3_RATIO, ADC_VBAT_RESDIV_RATIO_3OVER32, (float)32/3},
        {LUAT_ADC_VBAT_RANGE_8_0_RATIO, ADC_VBAT_RESDIV_RATIO_2OVER32, (float)32/2},
        {LUAT_ADC_VBAT_RANGE_16_0_RATIO, ADC_VBAT_RESDIV_RATIO_1OVER32, (float)32/1},
    };

    size_t i = 0;

    for (i = 0; i < sizeof(map)/sizeof(map[0]); i++)
    {
        if(map[i].range == range)
        {
            if (resdiv)
            {
                *resdiv = map[i].resdiv;
            }
            
            if (ratio)
            {
                *ratio = map[i].ratio;
            }            

            return 0;
        }
    }
    
    return 1;
}

static void adc0_cb(uint32_t result) {
    adc0ChannelResult = result;
    adc_state[0] = 1;
}

static void adc1_cb(uint32_t result) {
    adc1ChannelResult = result;
    adc_state[1] = 1;
}

static void adc2_cb(uint32_t result) {
    adc2ChannelResult = result;
    adc_state[2] = 1;
}

static void adc3_cb(uint32_t result) {
    adc3ChannelResult = result;
    adc_state[3] = 1;
}

static void adc_vbat_cb(uint32_t result) {
    vbatChannelResult = result;
    adc_state[4] = 1;
}

static void adc_temp_cb(uint32_t result) {
    thermalChannelResult = result;
    adc_state[5] = 1;
}

static volatile uint8_t adc_init = 1;

int luat_adc_open(int id, void* ptr) {
    AdcConfig_t adcConfig;
    if (!adc_exist(id))
        return -1;
    ADC_getDefaultConfig(&adcConfig);
    if (adc_init){
        trimAdcSetGolbalVar();
        adc_init = 0;
    }

    AdcAioResDiv_e resdiv;
    if (LUAT_ADC_CH_CPU == id || ADC_CH_CPU_OLD == id)
    {
       adc_range_to_resdiv(id, adc_range[4], &resdiv, NULL);
    }
    else if (LUAT_ADC_CH_VBAT == id || ADC_CH_VBAT_OLD == id)
    {
       adc_range_to_resdiv(id, adc_range[5], &resdiv, NULL);
    }
    else
    {
        adc_range_to_resdiv(id, adc_range[id], &resdiv, NULL);
    }    
    
    switch (id)
    {
    case 0:
        adcConfig.channelConfig.aioResDiv = resdiv;
#if defined TYPE_EC716S
        ADC_channelInit(ADC_CHANNEL_AIO1, ADC_USER_APP, &adcConfig, adc0_cb);
#else
        ADC_channelInit(ADC_CHANNEL_AIO3, ADC_USER_APP, &adcConfig, adc0_cb);
#endif
        adc_state[0] = 0;
        break;
    case 1:
        adcConfig.channelConfig.aioResDiv = resdiv;
#if defined TYPE_EC716S
        ADC_channelInit(ADC_CHANNEL_AIO2, ADC_USER_APP, &adcConfig, adc1_cb);
#else
        ADC_channelInit(ADC_CHANNEL_AIO4, ADC_USER_APP, &adcConfig, adc1_cb);
#endif
        adc_state[1] = 0;
        break;
    case 2:
        adcConfig.channelConfig.aioResDiv = resdiv;
        ADC_channelInit(ADC_CHANNEL_AIO1, ADC_USER_APP, &adcConfig, adc2_cb);
        adc_state[2] = 0;
        break;
    case 3:
        adcConfig.channelConfig.aioResDiv = resdiv;
        ADC_channelInit(ADC_CHANNEL_AIO2, ADC_USER_APP, &adcConfig, adc3_cb);
        adc_state[3] = 0;
        break;
    case LUAT_ADC_CH_VBAT: // vbat
    case ADC_CH_VBAT_OLD:
        adcConfig.channelConfig.vbatResDiv = resdiv;
        ADC_channelInit(ADC_CHANNEL_VBAT, ADC_USER_APP, &adcConfig, adc_vbat_cb);
        adc_state[4] = 0;
        break;
    case LUAT_ADC_CH_CPU: // temp
    case ADC_CH_CPU_OLD:
        adcConfig.channelConfig.vbatResDiv = resdiv;
        ADC_channelInit(ADC_CHANNEL_THERMAL, ADC_USER_APP, NULL, adc_temp_cb);
        adc_state[5] = 0;
        break;
    default:
        return -1;
    }
    
    return 0;
}

int luat_adc_read(int id, int* val, int* val2) {
    if (!adc_exist(id))
        return -1;
    int t = 1000;
    switch (id)
    {
    case 0:
        adc_state[0] = 0;
#if defined TYPE_EC716S
        ADC_startConversion(ADC_CHANNEL_AIO1, ADC_USER_APP);
#else
        ADC_startConversion(ADC_CHANNEL_AIO3, ADC_USER_APP);
#endif
        while(adc_state[0] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[0] == 0) return -1;
        break;
    case 1:
        adc_state[1] = 0;
#if defined TYPE_EC716S
        ADC_startConversion(ADC_CHANNEL_AIO2, ADC_USER_APP);
#else
        ADC_startConversion(ADC_CHANNEL_AIO4, ADC_USER_APP);
 #endif
        while(adc_state[1] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[1] == 0) return -1;
        break;
    case 2:
        adc_state[2] = 0;
        ADC_startConversion(ADC_CHANNEL_AIO1, ADC_USER_APP);
        while(adc_state[2] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[2] == 0) return -1;
        break;
    case 3:
        adc_state[3] = 0;
        ADC_startConversion(ADC_CHANNEL_AIO2, ADC_USER_APP);
        while(adc_state[3] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[3] == 0) return -1;
        break;
    case LUAT_ADC_CH_VBAT:
    case ADC_CH_VBAT_OLD:
        adc_state[4] = 0;
        ADC_startConversion(ADC_CHANNEL_VBAT, ADC_USER_APP);
        while(adc_state[4] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[4] == 0) return -1;
        break;
    case LUAT_ADC_CH_CPU:
    case ADC_CH_CPU_OLD:
        adc_state[5] = 0;
        ADC_startConversion(ADC_CHANNEL_THERMAL, ADC_USER_APP);
        while(adc_state[5] == 0 && t > 0) {
            delay_us(10);
            t --;
        }; // 1w个循环,通常需要4000个循环
        if (adc_state[5] == 0) return -1;
        break;
    default:
        return -1;
    }

    float ratio;
    if (LUAT_ADC_CH_CPU == id || ADC_CH_CPU_OLD == id)
    {
       adc_range_to_resdiv(id, adc_range[4], NULL, &ratio);
    }
    else if (LUAT_ADC_CH_VBAT == id || ADC_CH_VBAT_OLD == id)
    {
       adc_range_to_resdiv(id, adc_range[5], NULL, &ratio);
    }
    else
    {
        adc_range_to_resdiv(id, adc_range[id], NULL, &ratio);
    } 
    
    switch (id)
    {
    case 0:
        *val = adc0ChannelResult;
#ifdef __LUATOS__
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc0ChannelResult) * ratio /1000;
#else
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc0ChannelResult) * ratio ;
#endif
        break;
    case 1:
        *val = adc1ChannelResult;
#ifdef __LUATOS__
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc1ChannelResult) * ratio /1000;
#else
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc1ChannelResult) * ratio ;
#endif
        break;
    case 2:
    	*val = adc2ChannelResult;
#ifdef __LUATOS__
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc2ChannelResult) * ratio /1000;
#else
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc2ChannelResult) * ratio ;
#endif
        break;
    case 3:
    	*val = adc3ChannelResult;
#ifdef __LUATOS__
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc3ChannelResult) * ratio /1000;
#else
        *val2 = (int)HAL_ADC_CalibrateRawCode(adc3ChannelResult) * ratio ;
#endif
        break;
    case LUAT_ADC_CH_VBAT:
    case ADC_CH_VBAT_OLD:
        *val = vbatChannelResult;
        *val2 =  (int)HAL_ADC_CalibrateRawCode(vbatChannelResult) * ratio / 1000;
        break;
    case LUAT_ADC_CH_CPU:
    case ADC_CH_CPU_OLD:
        *val = thermalChannelResult;
#ifdef __LUATOS__
        *val2 = (int)HAL_ADC_ConvertThermalRawCodeToTemperature(thermalChannelResult) * 1000;
#else
        *val2 = (int)HAL_ADC_ConvertThermalRawCodeToTemperature(thermalChannelResult);
#endif
        break;
    default:
        return -1;
    }
    if (*val == 0)
    {
    	*val2 = 0;
    }
    return 0;
}

int luat_adc_close(int id) {
    if (!adc_exist(id))
        return -1;
    switch (id)
    {
    case 0:
#if defined TYPE_EC716S
        ADC_channelDeInit(ADC_CHANNEL_AIO1, ADC_USER_APP);
#else
        ADC_channelDeInit(ADC_CHANNEL_AIO3, ADC_USER_APP);
#endif
        break;
    case 1:
#if defined TYPE_EC716S
        ADC_channelDeInit(ADC_CHANNEL_AIO2, ADC_USER_APP);
#else
        ADC_channelDeInit(ADC_CHANNEL_AIO4, ADC_USER_APP);
#endif
        break;
    case 2:
        ADC_channelDeInit(ADC_CHANNEL_AIO1, ADC_USER_APP);
        break;
    case 3:
        ADC_channelDeInit(ADC_CHANNEL_AIO2, ADC_USER_APP);
        break;
    case LUAT_ADC_CH_VBAT:
    case ADC_CH_VBAT_OLD:
        ADC_channelDeInit(ADC_CHANNEL_VBAT, ADC_USER_APP);
        break;
    case LUAT_ADC_CH_CPU:
    case ADC_CH_CPU_OLD:
        ADC_channelDeInit(ADC_CHANNEL_THERMAL, ADC_USER_APP);
        break;
    default:
        return -1;
    }
    return 0;
}


int luat_adc_global_config(int tp, int val) {
    switch(tp)
    {
    case ADC_SET_GLOBAL_RANGE:
    	adc_range[0] = val;
    	adc_range[1] = val;
#ifndef TYPE_EC716S
    	adc_range[2] = val;
    	adc_range[3] = val;
#endif
    	return 0;
    default:
    	return -1;
    }
}

int luat_adc_ctrl(int id, LUAT_ADC_CTRL_CMD_E cmd, luat_adc_ctrl_param_t param)
{
    if (!adc_exist(id))
    {
        return -1;
    }

    switch (cmd)
    {
    case LUAT_ADC_SET_GLOBAL_RANGE:
        
        if (LUAT_ADC_CH_CPU == id || ADC_CH_CPU_OLD == id)
        {
            adc_range[4] = param.range;
        }
        else if (LUAT_ADC_CH_VBAT == id || ADC_CH_VBAT_OLD == id)
        {
            adc_range[5] = param.range;
        }
        else
        {
            adc_range[id] = param.range;
        }
        break;
    
    default:
        return -1;
        break;
    }  

    return 0;      
}


