/*
 * Copyright (c) 2022 OpenLuat & AirM2M
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
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"
#include "luat_gpio.h"
#include "luat_pm.h"
#include "luat_mobile.h"
#include "clock.h"
#include "timer.h"
#include "driver_gpio.h"
#include "luat_mcu.h"
luat_rtos_task_handle task1_handle;
luat_rtos_task_handle task2_handle;
#define TRIGGER_PIN HAL_GPIO_18
#define ECHO_PIN HAL_GPIO_19
#define TIMEOUT_TIMES (40)

typedef struct
{
	uint8_t is_finish;
	uint64_t start_tick;
	uint64_t end_tick;
} hcsr04_t;
static hcsr04_t g_s_hcsr04;
static int gpio_isr(int pin, void *args)
{
	if (pin == ECHO_PIN)
	{
		if (luat_gpio_get(pin))
		{
			g_s_hcsr04.start_tick = luat_mcu_tick64();
		}
		else
		{
			g_s_hcsr04.end_tick = luat_mcu_tick64();
			g_s_hcsr04.is_finish = 1;
		}
	}
	return 0;
}
extern void delay_us(uint32_t us);
static void task1(void *param)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = TRIGGER_PIN;
	gpio_cfg.mode = LUAT_GPIO_OUTPUT;
	gpio_cfg.output_level = LUAT_GPIO_LOW;
	luat_gpio_open(&gpio_cfg);
	while (1)
	{
		luat_gpio_set(TRIGGER_PIN, LUAT_GPIO_HIGH);
		delay_us(10);
		luat_gpio_set(TRIGGER_PIN, LUAT_GPIO_LOW);
		gpio_cfg.pin = ECHO_PIN;
		gpio_cfg.mode = LUAT_GPIO_IRQ;
		gpio_cfg.irq_type = LUAT_GPIO_BOTH_IRQ;
		gpio_cfg.pull = LUAT_GPIO_PULLDOWN;
		gpio_cfg.irq_cb = gpio_isr;
		luat_gpio_open(&gpio_cfg);
		g_s_hcsr04.is_finish = 0;
		uint8_t delay = 0;
		while (!g_s_hcsr04.is_finish && delay < TIMEOUT_TIMES)
		{
			luat_rtos_task_sleep(1);
			delay++;
		}
		luat_gpio_close(ECHO_PIN);
		if (g_s_hcsr04.is_finish && delay < TIMEOUT_TIMES)
		{
			float distance = ((g_s_hcsr04.end_tick - g_s_hcsr04.start_tick) / luat_mcu_us_period()) * 340 / 10000 / 2 ;
			LUAT_DEBUG_PRINT("距离: %.f cm", distance);
		}
		else
		{
			LUAT_DEBUG_PRINT("hcsr04 超时");
		}
		luat_rtos_task_sleep(2000);
	}
}


static void task_demoE_init(void)
{
	luat_rtos_task_create(&task1_handle, 2 * 1024, 50, "task1", task1, NULL, 0);
}
INIT_TASK_EXPORT(task_demoE_init, "1");