/*
 * Copyright (c) 2025 OpenLuat & AirM2M
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
#include "csdk.h"
#include "driver_gpio.h"

#define UART_ID 2

static luat_rtos_task_handle uart_task_handle;

void luat_uart_recv_cb(int uart_id, uint32_t data_len){
    char* data_buff = luat_heap_malloc(1024);
    if (data_buff == NULL)
    {
        LUAT_DEBUG_PRINT("malloc buff for uart fail");
        return;
    }
    memset(data_buff,0, 1024);
    int len = 0;
    while (1) {
        len = luat_uart_read(uart_id, data_buff, 1024);
        if (len <= 0) {
            break;
        }
        LUAT_DEBUG_PRINT("uart_id:%d %p len:%d",uart_id,data_buff,len);
        luat_uart_write(uart_id, data_buff, len);
    }
    luat_heap_free(data_buff);
}

static void uart_iomux(void)
{
	luat_uart_pin_iomux_t uart_iomux = {0};
	luat_pin_function_description_t pin_decs;
	luat_pin_get_iomux_info(LUAT_MCU_PERIPHERAL_UART, 2, uart_iomux.pin_list); //读出当前的复用配置
	uart_iomux.pin_list[LUAT_PIN_UART_RX].altfun_id = 3;
	uart_iomux.pin_list[LUAT_PIN_UART_TX].altfun_id = 3;
	luat_pin_get_description_from_num(84, &pin_decs);	//模块84复用uart2rx
	uart_iomux.pin_list[LUAT_PIN_UART_RX].uid = pin_decs.uid;
	luat_pin_get_description_from_num(86, &pin_decs);	//模块86复用uart2tx
	uart_iomux.pin_list[LUAT_PIN_UART_TX].uid = pin_decs.uid;
	luat_pin_set_iomux_info(LUAT_MCU_PERIPHERAL_UART, 2, uart_iomux.pin_list);//写入修改之后的复用配置
}

static void task_test_uart(void *param)
{
    // 除非你已经非常清楚uart0作为普通串口给用户使用所带来的的后果，否则不要打开以下注释掉的代码
    // BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL,PLAT_CFG_ULG_PORT_USB);
    char send_buff[] = "hello LUAT!!!\n";

    luat_uart_t uart = {
        .id = UART_ID,
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
    //uart2的pin复用到GPIO10和GPIO11
    uart_iomux();
    LUAT_DEBUG_PRINT("setup result %d", luat_uart_setup(&uart));
    LUAT_DEBUG_PRINT("ctrl result %d", luat_uart_ctrl(UART_ID, LUAT_UART_SET_RECV_CALLBACK, luat_uart_recv_cb));

    while (1)
    {

        luat_rtos_task_sleep(1000);
        LUAT_DEBUG_PRINT("send result %d", luat_uart_write(UART_ID, send_buff, strlen(send_buff)));
    }
    luat_rtos_task_delete(uart_task_handle);
}

static void task_demo_uart(void)
{
    luat_rtos_task_create(&uart_task_handle, 8*1024, 20, "uart", task_test_uart, NULL, 0);
}
INIT_TASK_EXPORT(task_demo_uart,"1");



