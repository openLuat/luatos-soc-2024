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

#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"
#include "luat_mobile.h"
#include "luat_gpio.h"
#define NET_LED_PIN	HAL_GPIO_14
extern void tgt_app_service_init(void);
static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{
			LUAT_DEBUG_PRINT("luatos_mobile_event_callback  link ...");
			luat_gpio_set(NET_LED_PIN, 1);
			// luat_socket_check_ready(index, NULL);
		}
        else if(LUAT_MOBILE_NETIF_LINK_OFF == status || LUAT_MOBILE_NETIF_LINK_OOS == status)
        {
            LUAT_DEBUG_PRINT("luatos_mobile_event_callback  error ...");
            luat_gpio_set(NET_LED_PIN, 0);
        }
	}
}

static int32_t luat_test_socket_callback(void *pdata, void *param)
{
	OS_EVENT *event = (OS_EVENT *)pdata;
	LUAT_DEBUG_PRINT("%x", event->ID);
	return 0;
}
static luat_rtos_task_handle g_s_task_handle;
static network_ctrl_t *g_s_network_ctrl;
static void test(void *param)
{

	//途鸽虚拟卡服务
	tgt_app_service_init();
	//系统切换到虚拟卡
	luat_mobile_set_flymode(0, 1);
	luat_mobile_softsim_onoff(1);
	luat_mobile_set_flymode(0, 0);
	/*
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_RESET);
	g_s_network_ctrl = network_alloc_ctrl(NW_ADAPTER_INDEX_LWIP_GPRS);
	network_init_ctrl(g_s_network_ctrl, g_s_task_handle, luat_test_socket_callback, NULL);
	network_set_base_mode(g_s_network_ctrl, 1, 15000, 1, 300, 5, 9);
	g_s_network_ctrl->is_debug = 1;	//下行测速时关闭debug，如果只是普通测试，打开debug

	const char remote_ip[] = "170.106.106.105";
	int port = 6608;
	const char hello[] = "hello, luatos!";
	uint8_t *tx_data = malloc(1024);
	uint8_t *rx_data = malloc(1024 * 8);
	uint32_t tx_len, rx_len, cnt;
	uint64_t uplink, downlink;
	size_t total, alloc, peak;
	int result;
	uint8_t is_break,is_timeout;
	cnt = 0;
	while(1)
	{
		//luat_meminfo_sys(&total, &alloc, &peak);
		//LUAT_DEBUG_PRINT("meminfo %d,%d,%d",total,alloc,peak);
		result = network_wait_link_up(g_s_network_ctrl, 60000);
		if (result)
		{
			continue;
		}

		result = network_connect(g_s_network_ctrl, remote_ip, sizeof(remote_ip) - 1, NULL, port, 30000);
		if (!result)
		{
			result = network_tx(g_s_network_ctrl, (const uint8_t*)hello, sizeof(hello) - 1, 0, NULL, 0, &tx_len, 15000);
			if (!result)
			{
				while(!result)
				{
					result = network_wait_rx(g_s_network_ctrl, 20000, &is_break, &is_timeout);
					if (!result)
					{
						if (!is_timeout && !is_break)
						{
							do
							{
								result = network_rx(g_s_network_ctrl, rx_data, 1024 * 8, 0, NULL, NULL, &rx_len);
								if (rx_len > 0)
								{
									LUAT_DEBUG_PRINT("rx %d", rx_len);
									LUAT_DEBUG_PRINT("rx data %s",rx_data);
								}
							}while(!result && rx_len > 0);
						}
						else if (is_timeout)
						{
							sprintf((char*)tx_data, "test %u cnt", cnt);
							result = network_tx(g_s_network_ctrl, tx_data, strlen((char*)tx_data), 0, NULL, 0, &tx_len, 15000);
							cnt++;
							if (!(cnt % 10))
							{
								luat_mobile_get_ip_data_traffic(&uplink, &downlink);
								LUAT_DEBUG_PRINT("%u,%u", (uint32_t)uplink, (uint32_t)downlink);
								luat_mobile_clear_ip_data_traffic(1, 1);
							}
							luat_meminfo_sys(&total, &alloc, &peak);
							LUAT_DEBUG_PRINT("meminfo %d,%d,%d",total,alloc,peak);
						}
					}

				}
			}
		}
		LUAT_DEBUG_PRINT("网络断开，15秒后重试");
		network_close(g_s_network_ctrl, 5000);
		luat_rtos_task_sleep(15000);
	}
}

static void luat_example_init(void)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);


	gpio_cfg.pin = NET_LED_PIN;
	gpio_cfg.alt_fun = 0;
	luat_gpio_open(&gpio_cfg);

	luat_mobile_event_register_handler(luatos_mobile_event_callback);
	luat_rtos_task_create(&g_s_task_handle, 4*1024, 50, "test", test, NULL, 0);
}

INIT_TASK_EXPORT(luat_example_init, "1");

