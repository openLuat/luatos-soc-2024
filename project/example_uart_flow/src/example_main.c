/*
 * Copyright (c) 2024 OpenLuat & AirM2M
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
#include "luat_mobile.h"
#include "luat_pm.h"
#include "luat_uart.h"
#include "luat_gpio.h"
#include "plat_config.h"
#include "luat_mem.h"
/*
在example_uart基础上增加流控功能，为了兼容低功耗应用，直接使用AGPIO和wakeuppad
*/

static luat_rtos_task_handle uart_task_handle;

static void uart_cts_cb(int uart_id, uint32_t state){
	//回调在中断里，不可以用malloc
	LUAT_DEBUG_PRINT("UART%d CTS%d",uart_id, state);
}



static void task_test_uart(void *param)
{
    luat_uart_t uart = {
        .id = UART_ID1,
        .baud_rate = 2000000,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
    luat_uart_setup(&uart);
    luat_uart_setup_flow_ctrl(UART_ID1, uart_cts_cb);
    luat_rtos_task_sleep(2000);
    while (1)
    {
    	LUAT_DEBUG_PRINT("UART%d RTS%d",UART_ID1, 1);
    	luat_uart_ctrl(UART_ID1, LUAT_UART_SET_RTS_STATE, (void *)1);	//拉高RTS
    	luat_rtos_task_sleep(1000);
    	LUAT_DEBUG_PRINT("UART%d CTS%d",UART_ID1, luat_uart_ctrl(UART_ID1, LUAT_UART_GET_CTS_STATE, NULL));
    	luat_rtos_task_sleep(1000);
    	LUAT_DEBUG_PRINT("UART%d RTS%d",UART_ID1, 0);
    	luat_uart_ctrl(UART_ID1, LUAT_UART_SET_RTS_STATE, (void *)0);	//拉低RTS
    	luat_rtos_task_sleep(1000);
    	LUAT_DEBUG_PRINT("UART%d CTS%d",UART_ID1, luat_uart_ctrl(UART_ID1, LUAT_UART_GET_CTS_STATE, NULL));
    	luat_rtos_task_sleep(1000);
    }
}


static void task_demo_uart(void)
{

	luat_rtos_task_handle task_handle;
    luat_rtos_task_create(&uart_task_handle, 4*1024, 20, "uart", task_test_uart, NULL, 16);
}


INIT_TASK_EXPORT(task_demo_uart,"1");



