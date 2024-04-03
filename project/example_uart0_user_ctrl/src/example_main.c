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

/**
 * 简单演示一下用户通过往UART0发送命令来停止UART0的log输出，从而用户可以使用收发功能，在用户模式下收到命令来切换到log输出
 * log默认波特率是6M，注意手上的串口工具是否支持这个波特率。
 */
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"

#include "luat_uart.h"
#include "plat_config.h"
#include "debug_trace.h"
#define UART_ID 0
static uint8_t log_off_mode;
static luat_rtos_task_handle uart_task_handle;

void soc_log_rx(uint8_t *data, uint32_t len)
{
	if (log_off_mode)
	{

		if (!memcmp(data, "LOG ON", 6))
		{
			log_off_mode = 0;
			uniLogStop();
			uniLogInitStart(UART_0_FOR_UNILOG);
		}
		else
		{
			luat_uart_write(UART_ID, data, len);
		}
	}
	else
	{
		if (!memcmp(data, "LOG OFF", 7))
		{
			log_off_mode = 1;
			uniLogStop();
			uniLogInitStart(USB_FOR_UNILOG);
		}
	}

}

static void task_test_uart(void *param)
{
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	const char *send_buff = "this user mode!\r\n";
    while (1)
    {
        luat_rtos_task_sleep(1000);
        if (log_off_mode)
        {
        	DBG("%x", luat_uart_write(UART_ID, (void *)send_buff, strlen(send_buff)));
        }
    }
    luat_rtos_task_delete(uart_task_handle);
}

static void task_demo_uart(void)
{
    luat_rtos_task_create(&uart_task_handle, 2048, 20, "uart", task_test_uart, NULL, 0);
}

INIT_TASK_EXPORT(task_demo_uart,"1");



