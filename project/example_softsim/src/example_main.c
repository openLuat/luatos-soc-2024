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
extern void tgt_app_service_stop_softsim(void);
extern void luat_mobile_vsim_user_heartbeat_once(void);
static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	luat_mobile_scell_extern_info_t scell;
	int result;
	switch(event)
	{
	case LUAT_MOBILE_EVENT_CFUN:
		LUAT_DEBUG_PRINT("CFUN消息，status %d", status);
		break;
	case LUAT_MOBILE_EVENT_SIM:
		if (status != LUAT_MOBILE_SIM_NUMBER)
		{
			LUAT_DEBUG_PRINT("SIM卡消息，卡槽%d", index);
		}
		switch(status)
		{
		case LUAT_MOBILE_SIM_READY:
			LUAT_DEBUG_PRINT("SIM卡正常工作");
			break;
		case LUAT_MOBILE_NO_SIM:
			LUAT_DEBUG_PRINT("SIM卡不存在");
			break;
		case LUAT_MOBILE_SIM_NEED_PIN:
			LUAT_DEBUG_PRINT("SIM卡需要输入PIN码");
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_REGISTER_STATUS:
		LUAT_DEBUG_PRINT("移动网络服务状态变更，当前为%d", status);
		break;
	case LUAT_MOBILE_EVENT_CELL_INFO:
		switch(status)
		{
		case LUAT_MOBILE_CELL_INFO_UPDATE:
			LUAT_DEBUG_PRINT("周期性搜索小区信息完成一次");
			break;
		case LUAT_MOBILE_SIGNAL_UPDATE:
			LUAT_DEBUG_PRINT("服务小区信号状态变更");
			break;
		case LUAT_MOBILE_PLMN_UPDATE:
			break;
		case LUAT_MOBILE_SERVICE_CELL_UPDATE:
			luat_mobile_get_extern_service_cell_info(&scell);
			LUAT_DEBUG_PRINT("%x,%x,%u,%u", scell.mcc, scell.mnc, scell.earfcn, scell.pci);
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_PDP:
		LUAT_DEBUG_PRINT("CID %d PDP激活状态变更为 %d", index, status);
		break;
	case LUAT_MOBILE_EVENT_NETIF:
		LUAT_DEBUG_PRINT("internet工作状态变更为 %d,cause %d", status,index);
		switch (status)
		{
		case LUAT_MOBILE_NETIF_LINK_ON:
			LUAT_DEBUG_PRINT("可以上网");
			break;
		default:
			LUAT_DEBUG_PRINT("不能上网");
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_TIME_SYNC:
		LUAT_DEBUG_PRINT("通过移动网络同步了UTC时间");
		break;
	case LUAT_MOBILE_EVENT_CSCON:
		LUAT_DEBUG_PRINT("RRC状态 %d", status);
		break;
	case LUAT_MOBILE_EVENT_FATAL_ERROR:
		LUAT_DEBUG_PRINT("网络也许遇到问题，15秒内不能恢复的建议重启协议栈");
		break;
	default:
		break;
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
	size_t total, used, max_used;
	/*
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	//系统切换到虚拟卡
	luat_mobile_set_flymode(0, 1);
	luat_rtos_task_sleep(2000);
	tgt_app_service_init();	//途鸽虚拟卡服务
	luat_rtos_task_sleep(500);
	luat_mobile_softsim_onoff(1);
	luat_mobile_set_flymode(0, 0);
	luat_rtos_task_sleep(30000);

	while(1)
	{

		luat_meminfo_sys(&total, &used, &max_used);
		LUAT_DEBUG_PRINT("meminfo %d,%d,%d",total, used, max_used);
#if 0
		//系统切换到实体卡
		luat_mobile_set_flymode(0, 1);
		luat_rtos_task_sleep(2000);
		tgt_app_service_stop_softsim();
		luat_rtos_task_sleep(500);
		luat_mobile_softsim_onoff(0);
		luat_mobile_set_flymode(0, 0);
		luat_rtos_task_sleep(30000);
		luat_meminfo_sys(&total, &used, &max_used);
		LUAT_DEBUG_PRINT("meminfo %d,%d,%d",total, used, max_used);
		//系统切换到虚拟卡
		luat_mobile_set_flymode(0, 1);
		luat_rtos_task_sleep(2000);
		tgt_app_service_init();	//途鸽虚拟卡服务
		luat_rtos_task_sleep(500);
		luat_mobile_softsim_onoff(1);
		luat_mobile_set_flymode(0, 0);
		luat_rtos_task_sleep(30000);
#endif
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(60000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(90000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(120000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(150000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(180000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(210000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(240000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(270000);
		luat_mobile_vsim_user_heartbeat_once();
		luat_rtos_task_sleep(300000);
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

