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
#include "luat_mobile.h"
#include "luat_debug.h"
#define AUTO_APN_TEST	//使用自动设置APN的模式，如果要自己手动设置，注释掉，需要用默认用特殊APN激活的，必须打开
#ifndef AUTO_APN_TEST
static g_s_test_cid = 2;
#endif
static void sms_event_cb(uint32_t event, void *param)
{
	LUAT_DEBUG_PRINT("短信event%d,%x",event, param);
}

static void mobile_event_cb(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	luat_mobile_cell_info_t cell_info;
	luat_mobile_signal_strength_info_t signal_info;
	int apn_len;
	uint8_t csq, i;
	char imsi[20];
	char iccid[24] = {0};
	char apn[32] = {0};
	ip_addr_t ipv4;
	ip_addr_t ipv6;
	switch(event)
	{
	case LUAT_MOBILE_EVENT_CFUN:
		LUAT_DEBUG_PRINT("CFUN消息，status %d", status);
		break;
	case LUAT_MOBILE_EVENT_SIM:
		LUAT_DEBUG_PRINT("SIM卡消息，卡槽%d", index);
		switch(status)
		{
		case LUAT_MOBILE_SIM_READY:
			LUAT_DEBUG_PRINT("SIM卡正常工作");
			luat_mobile_get_iccid(0, iccid, sizeof(iccid));
			LUAT_DEBUG_PRINT("ICCID %s", iccid);
			luat_mobile_get_imsi(0, imsi, sizeof(imsi));
			LUAT_DEBUG_PRINT("IMSI %s", imsi);
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
			luat_mobile_get_last_notify_cell_info(&cell_info);
			if (cell_info.lte_service_info.cid)
			{
				LUAT_DEBUG_PRINT("服务小区信息 mcc %x mnc %x cellid %u band %d tac %u pci %u earfcn %u is_tdd %d rsrp %d rsrq %d snr %d rssi %d",
						cell_info.lte_service_info.mcc, cell_info.lte_service_info.mnc, cell_info.lte_service_info.cid,
						cell_info.lte_service_info.band, cell_info.lte_service_info.tac, cell_info.lte_service_info.pci, cell_info.lte_service_info.earfcn,
						cell_info.lte_service_info.is_tdd, cell_info.lte_service_info.rsrp, cell_info.lte_service_info.rsrq,
						cell_info.lte_service_info.snr, cell_info.lte_service_info.rssi);
			}
			for (i = 0; i < cell_info.lte_neighbor_info_num; i++)
			{
				if (cell_info.lte_info[i].cid)
				{
					LUAT_DEBUG_PRINT("邻小区 %d mcc %x mnc %x cellid %u tac %u pci %u", i + 1, cell_info.lte_info[i].mcc,
							cell_info.lte_info[i].mnc, cell_info.lte_info[i].cid, cell_info.lte_info[i].tac, cell_info.lte_info[i].pci);
					LUAT_DEBUG_PRINT("邻小区 %d earfcn %u rsrp %d rsrq %d snr %d", i + 1, cell_info.lte_info[i].earfcn, cell_info.lte_info[i].rsrp,
							cell_info.lte_info[i].rsrq, cell_info.lte_info[i].snr);
				}
			}
			break;
		case LUAT_MOBILE_SIGNAL_UPDATE:
			LUAT_DEBUG_PRINT("服务小区信号状态变更");
			luat_mobile_get_last_notify_signal_strength_info(&signal_info);
			luat_mobile_get_last_notify_signal_strength(&csq);
			if (signal_info.luat_mobile_lte_signal_strength_vaild)
			{
				LUAT_DEBUG_PRINT("rsrp %d, rsrq %d, snr %d, rssi %d, csq %d %d", signal_info.lte_signal_strength.rsrp,
						signal_info.lte_signal_strength.rsrq, signal_info.lte_signal_strength.snr,
						signal_info.lte_signal_strength.rssi, csq, luat_mobile_rssi_to_csq(signal_info.lte_signal_strength.rssi));
			}

			break;
		}
		break;
	case LUAT_MOBILE_EVENT_PDP:
		LUAT_DEBUG_PRINT("CID %d PDP激活状态变更为 %d", index, status);
#ifndef AUTO_APN_TEST
		if ((g_s_test_cid == index) && !status)
		{
			luat_mobile_active_netif(0, index);
		}
#endif
		break;
	case LUAT_MOBILE_EVENT_NETIF:
		LUAT_DEBUG_PRINT("internet工作状态变更为 %d", status);
		switch (status)
		{
		case LUAT_MOBILE_NETIF_LINK_ON:
			LUAT_DEBUG_PRINT("可以上网");
#ifndef AUTO_APN_TEST
			luat_mobile_get_local_ip(0, g_s_test_cid, &ipv4, &ipv6);
#else
			luat_mobile_get_local_ip(0, 1, &ipv4, &ipv6);
#endif
			if (ipv4.type != 0xff)
			{
				LUAT_DEBUG_PRINT("IPV4 %s", ip4addr_ntoa(&ipv4.u_addr.ip4));
			}
			if (ipv6.type != 0xff)
			{
				LUAT_DEBUG_PRINT("IPV6 %s", ip6addr_ntoa(&ipv4.u_addr.ip6));
			}
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
	case LUAT_MOBILE_EVENT_BEARER:
		switch(status)
		{
		case LUAT_MOBILE_BEARER_GET_DEFAULT_APN:
			//如果不使用默认APN激活，可以直接设置APN，不需要等到拿到默认APN
			apn_len = luat_mobile_get_apn(0, 0, apn, sizeof(apn));
			if (apn_len > 0)
			{
				LUAT_DEBUG_PRINT("默认apn %s", apn);
			}
#ifndef AUTO_APN_TEST
			luat_mobile_set_apn_base_info(0, g_s_test_cid, 3, apn, apn_len);
#endif
			break;
		case LUAT_MOBILE_BEARER_APN_SET_DONE:
#ifndef AUTO_APN_TEST
			//可以不设置，直接激活
			luat_mobile_set_apn_auth_info(0, g_s_test_cid, 0xff, NULL, 0, NULL, 0);
#endif
			break;
		case LUAT_MOBILE_BEARER_AUTH_SET_DONE:
#ifndef AUTO_APN_TEST
			luat_mobile_active_apn(0, g_s_test_cid, 1);
#endif
			break;
		case LUAT_MOBILE_BEARER_DEL_DONE:
			break;
		case LUAT_MOBILE_BEARER_SET_ACT_STATE_DONE:
			//这里只是说操作执行了，但是能不能激活成功不在这个地方看
			break;
		}
		break;
	default:
		break;
	}
}

static void task_run(void *param)
{
	int i;
	luat_mobile_cell_info_t  cell_info;
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
#ifndef AUTO_APN_TEST
	luat_mobile_user_ctrl_apn();
#endif
	/*
		设置专网卡APN信息时，如需要设置在第一路承载（CID 1），则必须使用自动模式去激活

		如果是其他路承载，自动模式还是手动模式都可以

		如果设置过APN信息的模块需要换回普通卡用，需要把apn清空

		如果APN需要设置密码，根据APN信息和加密协议设置

		apn信息需要在开机注网前配置好
	*/
#ifdef AUTO_APN_TEST
	//如果之前用过特殊APN的卡，现在开机就要转回普通卡，建议删除原先设置的APN信息，调用下面的del接口即可
//	luat_mobile_del_apn(0,1,0);
	luat_mobile_user_apn_auto_active(0, 1, 3,3, "CMIOTTQJ",8,"ceshi",5,"tqj123456",9);
#endif

	char imei[20] = {0};
	luat_mobile_get_imei(0, imei, sizeof(imei));
	LUAT_DEBUG_PRINT("IMEI %s", imei);
	luat_mobile_set_sn("1234567890abcdefghijklmnopqrstuv",32);
	char sn[33] = {0};
	luat_mobile_get_sn(sn, sizeof(sn));
	LUAT_DEBUG_PRINT("SN %s", sn);
	char muid[64] = {0};
	luat_mobile_get_muid(muid, sizeof(muid));
	LUAT_DEBUG_PRINT("MUID %s", muid);
	char apn[64] = {0};
	luat_rtos_task_sleep(10000);
	//这里演示运行一段时间后，再删掉已经设置的APN
	luat_mobile_user_apn_auto_active(0, 0, 0,0, NULL, 0, NULL, 0, NULL, 0);	//删掉RAM里缓存的
	luat_mobile_user_ctrl_apn_stop();//变回自动配置
	luat_mobile_set_flymode(0, 1);//删除系统保存的要先进飞行模式
	luat_rtos_task_sleep(100);
	luat_mobile_del_apn(0,1,0);//删除系统保存的
	luat_mobile_set_flymode(0, 0);//退出飞行模式

	//luat_mobile_reset_stack();
	while(1)
	{
		luat_rtos_task_sleep(10000);
		luat_mobile_get_apn(0, 0, apn, 10);	//这里故意不获取完整的
		DBG("%s", apn);
	}
}

void task_init(void)
{
	luat_mobile_event_register_handler(mobile_event_cb);
	luat_mobile_sms_event_register_handler(sms_event_cb);
	luat_mobile_set_period_work(90000, 0, 4);
	luat_mobile_set_sim_id(2);
	luat_rtos_task_handle task_handle;
	luat_rtos_task_create(&task_handle, 4 * 1204, 50, "test", task_run, NULL, 32);

}

INIT_TASK_EXPORT(task_init, "0");
