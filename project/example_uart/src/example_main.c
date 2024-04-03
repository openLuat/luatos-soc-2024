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
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_mobile.h"
#include "luat_pm.h"
#include "luat_uart.h"
#include "plat_config.h"
/*
 一.上报接收数据中断的逻辑：
    1.串口初始化时，新建一个缓冲区
    2.可以考虑多为用户申请几百字节的缓冲长度，用户处理时防止丢包
    3.每次串口收到数据时，先存入缓冲区，记录长度
    4.遇到以下情况时，再调用串口中断
        a)缓冲区满（帮用户多申请的的情况）/缓冲区只剩几百字节（按实际长度申请缓冲区的情况）
        b)收到fifo接收超时中断（此时串口数据应该是没有继续收了）
    5.触发收到数据中断时，返回的数据应是缓冲区的数据
    6.关闭串口时，释放缓冲区资源
 二.uart0使用情况
    因为uart0被sdk内部占用为底层日志口，用来输出bootloader运行过程中的日志，此功能无法关闭；
    除此之外，还可以用来作为系统的日志口，usb抓取日志受限于usb枚举时间，会丢失一部分日志，而uart0则不会丢失这部分日志
    如果uart0被配置成应用使用，在开机过程中出现的异常，会因为无法抓取底层日志而无法分析定位问题
 三.串口复用问题
    1.串口复用，默认复用引脚见luat_uart_ec7xx.c, 若需要复用至其他引脚, 参考luat_mcu_iomux_ctrl接口使用
*/
#define UART_ID 1
//#define LPUART_TEST
static luat_rtos_task_handle uart_task_handle;

void luat_uart_recv_cb(int uart_id, uint32_t data_len){
    char* data_buff = malloc(data_len+1);
    memset(data_buff,0,data_len+1);
    luat_uart_read(uart_id, data_buff, data_len);
    LUAT_DEBUG_PRINT("uart_id:%d data_len:%d",uart_id,data_buff,data_len);
    luat_uart_write(uart_id, data_buff, data_len);
    free(data_buff);
}

static void task_test_uart(void *param)
{
    // 除非你已经非常清楚uart0作为普通串口给用户使用所带来的的后果，否则不要打开以下注释掉的代码
    // BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_LOG_PORT_SEL,PLAT_CFG_ULG_PORT_USB);
    char send_buff[] = "hello LUAT!!!\n";
#ifdef LPUART_TEST
    luat_uart_t uart = {
        .id = UART_ID,
        .baud_rate = 9600,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
#else
    luat_uart_t uart = {
        .id = UART_ID,
        .baud_rate = 115200,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
#endif
    LUAT_DEBUG_PRINT("setup result %d", luat_uart_setup(&uart));
    LUAT_DEBUG_PRINT("ctrl result %d", luat_uart_ctrl(UART_ID, LUAT_UART_SET_RECV_CALLBACK, luat_uart_recv_cb));
#ifdef LPUART_TEST
    luat_mobile_set_flymode(0, 1);
    luat_pm_power_ctrl(LUAT_PM_POWER_USB, 0);
    luat_pm_request(LUAT_PM_SLEEP_MODE_LIGHT);
#endif
    while (1)
    {
#ifdef LPUART_TEST
        luat_rtos_task_sleep(30000);
        LUAT_DEBUG_PRINT("send result %d", luat_uart_write(UART_ID, send_buff, strlen(send_buff)));
#else
        luat_rtos_task_sleep(1000);
        LUAT_DEBUG_PRINT("send result %d", luat_uart_write(UART_ID, send_buff, strlen(send_buff)));
#endif
    }
    luat_rtos_task_delete(uart_task_handle);
}

static void task_demo_uart(void)
{
    luat_rtos_task_create(&uart_task_handle, 2048, 20, "uart", task_test_uart, NULL, 0);
}

// 除非你已经非常清楚uart0作为普通串口给用户使用所带来的的后果，否则不要打开以下注释掉的代码
// extern void soc_uart0_set_log_off(uint8_t is_off);
// static void uart0_init(void)
// {
//     soc_uart0_set_log_off(1);
// }
// INIT_TASK_EXPORT(uart0_init,"1");

INIT_TASK_EXPORT(task_demo_uart,"1");



