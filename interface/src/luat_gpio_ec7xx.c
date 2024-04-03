/*
 * Copyright (c) 2022                                                                                                   Luat & AirM2M
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
#include "common_api.h"
#include "luat_gpio.h"
#include "luat_rtos.h"
#include "driver_gpio.h"
#include "slpman.h"
#include "FreeRTOS.h"
#include "pwrkey.h"

#define GPIO_ALT_MAX (4)

__attribute__((weak)) int luat_gpio_irq_default(int pin, void* args)
{
	return 0;
}

static int luat_gpio_irq_callback(void *ptr, void *pParam)
{
    int pin = (int)ptr;
    luat_gpio_irq_default(pin, (void*)luat_gpio_get(pin));
    return 0;
}

void luat_gpio_set_default_cfg(luat_gpio_cfg_t* gpio)
{
	memset(gpio, 0, sizeof(luat_gpio_cfg_t));
}

int luat_gpio_open(luat_gpio_cfg_t* gpio)
{

	if (((uint32_t)(gpio->pin)) >= HAL_GPIO_QTY) return -1;

	GPIO_GlobalInit(NULL);

	if (gpio->pin >= HAL_GPIO_MAX)
	{
		uint8_t IsRiseHigh = 0;
		uint8_t IsFallLow = 0;
		uint8_t Pullup = 0;
		uint8_t Pulldown = 0;
		switch (gpio->pull)
		{
		case LUAT_GPIO_PULLUP:
			Pullup = 1;
			break;
		case LUAT_GPIO_PULLDOWN:
			Pulldown = 1;
			break;
		default:
			break;
		}
		switch (gpio->irq_type)
		{
		case LUAT_GPIO_RISING_IRQ:
			IsRiseHigh = 1;
			break;
		case LUAT_GPIO_FALLING_IRQ:
			IsFallLow = 1;
			break;
		case LUAT_GPIO_BOTH_IRQ:
			IsRiseHigh = 1;
			IsFallLow = 1;
			break;
		default:
			break;
		}

		if (LUAT_GPIO_IRQ == gpio->mode)
		{
			if (gpio->irq_cb)
			{
				GPIO_ExtiSetCB(gpio->pin, gpio->irq_cb, gpio->irq_args);
			}
			else
			{
				GPIO_ExtiSetCB(gpio->pin, luat_gpio_irq_callback, gpio->irq_args);
			}

		}
		else
		{
			GPIO_ExtiSetCB(gpio->pin, NULL, NULL);
		}
		GPIO_WakeupPadConfig(gpio->pin, (LUAT_GPIO_OUTPUT == gpio->mode)?false:true, IsRiseHigh, IsFallLow, Pullup, Pulldown);

		if (HAL_WAKEUP_PWRKEY == gpio->pin)
		{
			pwrKeySetSWOn();
		}
		return 0;
	}

	uint8_t is_pull;
	uint8_t is_pullup;
	uint8_t is_input = (LUAT_GPIO_OUTPUT == gpio->mode)?0:1;
    switch (gpio->pull)
    {
	case LUAT_GPIO_PULLUP:
		is_pull = 1;
		is_pullup = 1;
		break;
	case LUAT_GPIO_PULLDOWN:
		is_pull = 1;
		is_pullup = 0;
		break;
	default:
		is_pull = 0;
		is_pullup = 0;
		break;
    }
    GPIO_Config(gpio->pin, is_input, gpio->output_level);
    GPIO_PullConfig(GPIO_ToPadEC7XX(gpio->pin, gpio->alt_fun), is_pull, is_pullup);

    if (LUAT_GPIO_IRQ == gpio->mode)
    {
        if (gpio->irq_cb) {
        	GPIO_ExtiSetCB(gpio->pin, gpio->irq_cb, gpio->irq_args);
        }
        else
        {
        	GPIO_ExtiSetCB(gpio->pin, luat_gpio_irq_callback, gpio->irq_args);
        }
        switch (gpio->irq_type)
        {
		case LUAT_GPIO_RISING_IRQ:
			GPIO_ExtiConfig(gpio->pin, 0,1,0);
			break;
		case LUAT_GPIO_FALLING_IRQ:
			GPIO_ExtiConfig(gpio->pin, 0,0,1);
			break;
		case LUAT_GPIO_BOTH_IRQ:
			GPIO_ExtiConfig(gpio->pin, 0,1,1);
			break;
		case LUAT_GPIO_HIGH_IRQ:
			GPIO_ExtiConfig(gpio->pin, 1,1,0);
			break;
		case LUAT_GPIO_LOW_IRQ:
			GPIO_ExtiConfig(gpio->pin, 1,0,1);
			break;
		default:
			GPIO_ExtiConfig(gpio->pin, 0,0,0);
			break;
        }

    }
    else
    {
    	GPIO_ExtiConfig(gpio->pin, 0,0,0);
    	GPIO_ExtiSetCB(gpio->pin, NULL, NULL);
    }

    GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(gpio->pin, gpio->alt_fun), gpio->alt_fun, 0, 0);
    return 0;
}

typedef struct
{
	uint8_t pin;
	uint8_t alt;
}luat_gpio_alt_table_t;

static luat_gpio_alt_table_t gpio_alt[GPIO_ALT_MAX] = {
	{HAL_GPIO_16, 4},
	{HAL_GPIO_17, 4},
	{HAL_GPIO_18, 0},
	{HAL_GPIO_19, 0},
};

__USER_FUNC_IN_RAM__ uint8_t luat_gpio_get_alt(uint8_t GPIO) {return gpio_alt[GPIO - HAL_GPIO_16].alt;}

int luat_gpio_setup(luat_gpio_t *gpio){
	luat_gpio_cfg_t cfg = {0};
	cfg.pin = gpio->pin;
	cfg.alt_fun = gpio->alt_func;
	cfg.irq_cb = gpio->irq_cb;
	cfg.irq_args = gpio->irq_args;
	cfg.irq_type = gpio->irq;
	cfg.mode = gpio->mode;
	cfg.pull = gpio->pull;
	if (LUAT_GPIO_PULLUP == cfg.pull)
		cfg.output_level = 1;
	if (gpio->alt_func == -1) {
		cfg.alt_fun = 0;
		for (uint8_t i = 0; i < GPIO_ALT_MAX; i++)
		{
			if (gpio->pin == gpio_alt[i].pin)
			{
				cfg.alt_fun = gpio_alt[i].alt;
				break;
			}
		}
	}
	else
	{
		for (uint8_t i = 0; i < GPIO_ALT_MAX; i++)
		{
			if (gpio->pin == gpio_alt[i].pin)
			{
				gpio_alt[i].alt = cfg.alt_fun;
				break;
			}
		}
	}
	return luat_gpio_open(&cfg);
}

int luat_gpio_set(int pin, int level){
    if (((uint32_t)(pin)) >= HAL_GPIO_MAX) return -1;
    GPIO_Output(pin, level);
    return 0;
}

int luat_gpio_get(int pin){
	if (((uint32_t)(pin)) >= HAL_GPIO_QTY) return 0;
    uint8_t re;
    if (HAL_WAKEUP_PWRKEY == pin)
    {
    	return pwrKeyGetPinLevel();
    }
    if (HAL_WAKEUP_CHARGE == pin)
    {
    	return chargeGetCurStatus();
    }
    if (pin >= HAL_GPIO_MAX)
    {
    	pin -= HAL_WAKEUP_0;
    	re = slpManGetWakeupPinValue() & (1 << pin);
    }
    else
    {
    	re = GPIO_Input(pin);
    }
    return re?1:0;
}

void luat_gpio_close(int pin){

	if (pin >= HAL_GPIO_QTY) return ;
	GPIO_ExtiSetCB(pin, NULL, 0);
    if (pin >= HAL_GPIO_MAX)
    {
    	GPIO_WakeupPadConfig(pin, 0, 0, 0, 0, 0);
    	return;
    }
    else
    {
    	GPIO_ExtiConfig(pin, 0,0,0);
    }
    uint8_t alt_fun = 0;
	for (uint8_t i = 0; i < GPIO_ALT_MAX; i++)
	{
		if (pin == gpio_alt[i].pin)
		{
			alt_fun = gpio_alt[i].alt;
			break;
		}
	}
    PAD_setInputOutputDisable(GPIO_ToPadEC7XX(pin, alt_fun));
    return ;
}
int luat_gpio_set_irq_cb(int pin, luat_gpio_irq_cb cb, void* args)
{

	if (pin >= HAL_GPIO_QTY) return -1;
	GPIO_ExtiSetCB(pin, cb, args);
	return 0;
}

void luat_gpio_pulse(int pin, uint8_t *level, uint16_t len, uint16_t delay_ns)
{
	GPIO_OutPulse(pin, level, len, delay_ns);
}

int luat_gpio_ctrl(int pin, LUAT_GPIO_CTRL_CMD_E cmd, int param)
{
	if (pin >= (HAL_GPIO_MAX)) return -1;
	uint8_t alt_fun = (param >> 28);
	switch(cmd)
	{
	case LUAT_GPIO_CMD_SET_PULL_MODE:
		switch(param)
		{
		case LUAT_GPIO_PULLUP:
			GPIO_Config(GPIO_ToPadEC7XX(pin, alt_fun), 1, 1);
			break;
		case LUAT_GPIO_PULLDOWN:
			GPIO_Config(GPIO_ToPadEC7XX(pin, alt_fun), 1, 0);
			break;
		default:
			GPIO_Config(GPIO_ToPadEC7XX(pin, alt_fun), 0, 0);
			break;
		}
		break;
	case LUAT_GPIO_CMD_SET_IRQ_MODE:
		switch(param)
        {
		case LUAT_GPIO_RISING_IRQ:
			GPIO_ExtiConfig(pin, 0,1,0);
			break;
		case LUAT_GPIO_FALLING_IRQ:
			GPIO_ExtiConfig(pin, 0,0,1);
			break;
		case LUAT_GPIO_BOTH_IRQ:
			GPIO_ExtiConfig(pin, 0,1,1);
			break;
		case LUAT_GPIO_HIGH_IRQ:
			GPIO_ExtiConfig(pin, 1,1,0);
			break;
		case LUAT_GPIO_LOW_IRQ:
			GPIO_ExtiConfig(pin, 1,0,1);
			break;
		default:
			GPIO_ExtiConfig(pin, 0,0,0);
			break;
        }
		break;
	default:
		return -1;
	}
	return 0;
}

#ifndef __LUATOS__
void luat_gpio_mode(int pin, int mode, int pull, int initOutput) {
    if (pin == 255) return;
    luat_gpio_t conf = {0};
    conf.pin = pin;
    conf.mode = mode == Luat_GPIO_INPUT ? Luat_GPIO_INPUT : Luat_GPIO_OUTPUT; // 只能是输入/输出, 不能是中断.
    conf.pull = pull;
    conf.irq = initOutput;
    conf.lua_ref = 0;
    conf.irq_cb = 0;
    luat_gpio_setup(&conf);
    if (conf.mode == Luat_GPIO_OUTPUT)
        luat_gpio_set(pin, initOutput);
}
#endif


int luat_gpio_driver_ws2812b(int pin, uint8_t *data, uint32_t len, uint32_t frame_cnt, uint8_t bit0h, uint8_t bit0l, uint8_t bit1h, uint8_t bit1l)
{
	if (pin >= HAL_GPIO_MAX) return -1;
	uint32_t frame_byte = frame_cnt * 3;
	if (!frame_cnt)
	{
		frame_byte = len;
	}
	uint32_t done_len, dummy_len, cr;
	done_len = 0;
	while(done_len < len)
	{
		dummy_len = ((len - done_len) > frame_byte)?frame_byte:(len - done_len);
		cr = luat_rtos_entry_critical();
		GPIO_DriverWS2812B(pin, &data[done_len], dummy_len, bit0h, bit0l, bit1h, bit1l);
		luat_rtos_exit_critical(cr);
		done_len += dummy_len;
	}
	return 0;
}
