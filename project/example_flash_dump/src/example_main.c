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
//air780ef不支持flashdump功能
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"
#include "luat_uart.h"
#include "FreeRTOS.h"
#include "task.h"
#include "mem_map.h"
extern bool ecFlashDumpOccuredCheck(void);
extern void soc_get_flash_dump(uint8_t *buf);
extern void soc_flash_dump_clear(void);
static void task1(void *param)
{
    luat_uart_t uart = {
        .id = LUAT_VUART_ID_0,
        .baud_rate = 921600,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
    LUAT_DEBUG_PRINT("test start");
    luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_SAVE_RESET);//死机时重要信息写入flash，然后重启
    luat_uart_setup(&uart);
    luat_rtos_task_sleep(10000);//如果有luatools，会自动提取flashdump
    if (ecFlashDumpOccuredCheck())
    {
    	uint8_t *data = luat_heap_malloc(0x4000);
    	soc_get_flash_dump(data);
    	luat_uart_write(uart.id, data, 0x4000);
    	luat_heap_free(data);
    	soc_flash_dump_clear();
    	luat_rtos_task_sleep(10000);//串口工具接收并且能处理完
    }
    else
    {
    	luat_rtos_task_sleep(10000);
    }
    LUAT_DEBUG_ASSERT(0, "");//故意死机
	while(1)
	{
		luat_rtos_task_sleep(1000);
		LUAT_DEBUG_PRINT("task1 loop");
	}
}

static void task_demo_init(void)
{
	luat_rtos_task_handle task1_handle;
	luat_rtos_task_create(&task1_handle, 2*1024, 50, "task1", task1, NULL, 0);
}
#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
INIT_TASK_EXPORT(task_demo_init, "1");
#endif

