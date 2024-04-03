#include "luat_debug.h"
#include "luat_rtos.h"
#include "luat_mobile.h"
#include "luat_gpio.h"
#include "platform_define.h"
#include "gpio_dec.h"
static uint8_t g_s_is_link_up;

uint8_t network_service_is_ready(void)
{
	return g_s_is_link_up;
}

static uint8_t g_s_sim_isready = 0;
static uint8_t g_s_net_status = 0;
static void mobile_event_cb(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	switch(event)
	{
	case LUAT_MOBILE_EVENT_CFUN:
		LUAT_DEBUG_PRINT("cfun event，status %d", status);
		break;
	case LUAT_MOBILE_EVENT_SIM:
		switch(status)
		{
		case LUAT_MOBILE_SIM_READY:
			g_s_sim_isready = 1;
			LUAT_DEBUG_PRINT("sim event ready");
			break;
		case LUAT_MOBILE_NO_SIM:
			g_s_sim_isready = 0;
			LUAT_DEBUG_PRINT("sim event not insert");
			break;
		case LUAT_MOBILE_SIM_NEED_PIN:
			LUAT_DEBUG_PRINT("sim event need pin");
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_REGISTER_STATUS:
		g_s_net_status = status;
		LUAT_DEBUG_PRINT("register event, status %d", status);
		break;
	case LUAT_MOBILE_EVENT_CELL_INFO:
		switch(status)
		{
		case LUAT_MOBILE_CELL_INFO_UPDATE:
			break;
		case LUAT_MOBILE_SIGNAL_UPDATE:
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_PDP:
        LUAT_DEBUG_PRINT("pdp event, cid %d, status %d", index, status);
		break;
	case LUAT_MOBILE_EVENT_NETIF:
		switch (status)
		{
		case LUAT_MOBILE_NETIF_LINK_ON:
			LUAT_DEBUG_PRINT("netif link on event");
			g_s_is_link_up = 1;
			break;
		default:
			LUAT_DEBUG_PRINT("netif link off event");
			g_s_is_link_up = 0;
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_TIME_SYNC:
		LUAT_DEBUG_PRINT("time sync event");
		break;
	case LUAT_MOBILE_EVENT_CSCON:
		LUAT_DEBUG_PRINT("rrc event, status %d", status);
		break;
	default:
		break;
	}
}

//控制NET指示灯闪烁
static void task_net_led_run(void *param)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);

	gpio_cfg.pin = Net_led_Pin;
	gpio_cfg.alt_fun=Net_led_Alt;
	luat_gpio_open(&gpio_cfg);
	while(1)
	{
		LUAT_DEBUG_PRINT("TEST STATUS g_s_sim_isready: %d, g_s_net_status: %d, g_s_is_link_up: %d, network_service_is_connect: %d", g_s_sim_isready, g_s_net_status, g_s_is_link_up, network_service_is_connect());
		if (g_s_sim_isready)
		{
			if(g_s_net_status != LUAT_MOBILE_STATUS_REGISTERED && g_s_net_status != LUAT_MOBILE_STATUS_REGISTERED_ROAMING)
			{
				luat_gpio_set(Net_led_Pin, 1);
				luat_rtos_task_sleep(200);
				luat_gpio_set(Net_led_Pin, 0);
				luat_rtos_task_sleep(200);
			}
			else
			{
				if(g_s_is_link_up)
				{
					if (network_service_is_connect())
					{
						luat_gpio_set(Net_led_Pin, 1);
						luat_rtos_task_sleep(1000);
					}
					else
					{
						luat_gpio_set(Net_led_Pin, 1);
						luat_rtos_task_sleep(1000);
						luat_gpio_set(Net_led_Pin, 0);
						luat_rtos_task_sleep(1000);
					}
				}
				else
				{
					luat_gpio_set(Net_led_Pin, 1);
					luat_rtos_task_sleep(1000);
					luat_gpio_set(Net_led_Pin, 0);
					luat_rtos_task_sleep(1000);
				}
			}
		}
		else
		{
			luat_gpio_set(Net_led_Pin, 0);
			luat_rtos_task_sleep(1000);
		}
	}
}

void network_service_init(void)
{

	luat_mobile_event_register_handler(mobile_event_cb);

	luat_rtos_task_handle task_net_led_handle;
	luat_rtos_task_create(&task_net_led_handle, 1024, 30, "net_led", task_net_led_run, NULL, 32);
}
