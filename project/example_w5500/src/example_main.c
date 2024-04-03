#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_rtos.h"
#include "luat_mcu.h"
#include "net_lwip.h"
#include "luat_spi.h"
#include "w5500_def.h"


#define MQTT_DEMO_SSL 			1
#define MQTT_DEMO_AUTOCON 		1


static luat_rtos_task_handle g_s_task_handle;
static network_ctrl_t *g_s_network_ctrl;
static int32_t luat_test_socket_callback(void *pdata, void *param)
{
	OS_EVENT *event = (OS_EVENT *)pdata;
	LUAT_DEBUG_PRINT("%x", event->ID);
	return 0;
}
void w5500_nw_state_cb(int state, uint32_t ip)
{
	LUAT_DEBUG_PRINT("w5500 net state %d", state);
	if (state)
	{
		LUAT_DEBUG_PRINT("w5500 IP ready, ip : %d.%d.%d.%d", (ip) & 0xFF, (ip >> 8) & 0xFF, (ip >> 16) & 0xFF, (ip >> 24) & 0xFF);
	}
	else
	{
		LUAT_DEBUG_PRINT("tew5500 IP lose");
	}
}

static void luat_test_task(void *param)
{
	/* 
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET); 
    luat_spi_t spi = {0};
    spi.id = 1;
    spi.bandrate = 25600000;
	spi.CPHA = 0;
	spi.CPOL = 0;
	spi.master = 1;
	spi.mode = 1;
	spi.dataw = 8;
	spi.bit_dict = 1;
	spi.cs = HAL_GPIO_2;

    w5500_init(&spi, HAL_GPIO_3, HAL_GPIO_1, 0xff);

	if (!w5500_device_ready())
	{
		while (1)
		{
			LUAT_DEBUG_PRINT("device no ready");
			luat_rtos_task_sleep(1000);
		}
	}
	w5500_set_static_ip(0, 0, 0);
	w5500_set_param(2000, 8, 0, 0);
	w5500_reset();
	w5500_register_adapter(NW_ADAPTER_INDEX_ETH0);
	g_s_network_ctrl = network_alloc_ctrl(NW_ADAPTER_INDEX_ETH0);
	network_init_ctrl(g_s_network_ctrl, g_s_task_handle, luat_test_socket_callback, NULL);
	network_set_base_mode(g_s_network_ctrl, 1, 15000, 1, 300, 5, 9);
	g_s_network_ctrl->is_debug = 1;
	// 请访问 https://netlab.luatos.com 获取新的端口号,之后修改remote_ip port再进行编译
	const char remote_ip[] = "112.125.89.8";
	int port = 42320;
	const char hello[] = "hello, luatos!";
	uint8_t *tx_data = malloc(1024);
	uint8_t *rx_data = malloc(1024 * 8);
	uint32_t tx_len, rx_len, cnt;
	uint64_t uplink, downlink;
	int result;
	uint8_t is_break,is_timeout;
	cnt = 0;
	while(1)
	{
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

static void luat_test_init(void)
{
	luat_rtos_task_create(&g_s_task_handle, 2 * 1024, 90, "test", luat_test_task, NULL, 16);
}

INIT_TASK_EXPORT(luat_test_init, "1");
