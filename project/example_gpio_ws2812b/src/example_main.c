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
#include "luat_debug.h"
#include "luat_gpio.h"

// ws2812b测试 所用的gpio编号
#define TEST_PIN 	HAL_GPIO_8

static void task_test_ws2812(void *param)
{
	/* 
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
	uint8_t cnt;
	uint8_t color[3 * 64];
	luat_rtos_task_sleep(1);
	cnt = 0;
    while(1)
	{
    	memset(color, 0, sizeof(color));
    	for(int i = 0; i < 64; i++)
    	{
    		color[i * 3 + cnt] = 0xff;
    	}
		/* 
		 * 最后四个参数是调整NOP 实现的延时效果，具体数值是需要对照着示波器或者逻辑分析仪调整
		 * bit0的高电平额外延迟，默认写10，如果高电平时间不足酌情增加
		 * bit0的低电平额外延迟，默认写0
		 * bit1的高电平额外延迟，默认写10，如果高电平时间不足酌情增加
		 * bit1的低电平额外延迟，默认写0
        */
    	luat_gpio_driver_ws2812b(TEST_PIN, color, sizeof(color), 0, 10, 0, 10, 0); //如果不稳定，就把frame_cnt改成0，但是会关闭中断时间较长
    	luat_rtos_task_sleep(1000);
    	cnt = (cnt + 1) % 3;
	} 
    
}

static void task_demo_ws2812b(void)
{
	luat_rtos_task_handle task_handle;
	luat_gpio_cfg_t gpio_cfg = {0};
	gpio_cfg.pin = TEST_PIN;
	gpio_cfg.output_level = 0;
	luat_gpio_open(&gpio_cfg);
    luat_rtos_task_create(&task_handle, 2048, 20, "ws2812b", task_test_ws2812, NULL, NULL);
}

INIT_TASK_EXPORT(task_demo_ws2812b,"1");



