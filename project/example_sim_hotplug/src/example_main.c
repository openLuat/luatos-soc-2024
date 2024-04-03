#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_gpio.h"
#include "luat_mobile.h"

#define SIM_HOTPLUG_PIN         HAL_WAKEUP_2
#define SIM_REMOVE              LUAT_GPIO_HIGH
#define SIM_INSERT              LUAT_GPIO_LOW

static luat_rtos_task_handle g_s_task_handle;

static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	luat_mobile_cell_info_t cell_info;
	luat_mobile_signal_strength_info_t signal_info;
	uint16_t mcc;
	uint8_t csq, i, mnc;
	char imsi[20];
	char iccid[24] = {0};
	char apn[32] = {0};
	ip_addr_t ipv4;
	ip_addr_t ipv6;
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
			luat_mobile_get_iccid(index, iccid, sizeof(iccid));
			LUAT_DEBUG_PRINT("ICCID %s", iccid);
			luat_mobile_get_imsi(index, imsi, sizeof(imsi));
			LUAT_DEBUG_PRINT("IMSI %s", imsi);
			luat_mobile_get_plmn_from_imsi(imsi, &mcc, &mnc);
			result = luat_mobile_get_isp_from_plmn(mcc, mnc);
			switch(result)
			{

			case LUAT_MOBILE_ISP_CMCC:
				LUAT_DEBUG_PRINT("中国移动卡");
				break;
			case LUAT_MOBILE_ISP_CTCC:
				LUAT_DEBUG_PRINT("中国电信卡");
				break;
			case LUAT_MOBILE_ISP_CUCC:
				LUAT_DEBUG_PRINT("中国联通卡");
				break;
			case LUAT_MOBILE_ISP_CRCC:
				LUAT_DEBUG_PRINT("中国广电卡");
				break;
			case LUAT_MOBILE_ISP_UNKNOW:
				LUAT_DEBUG_PRINT("未知运营商");
				break;
			default:
				LUAT_DEBUG_PRINT("非中国卡");
				break;
			}
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
		break;
	case LUAT_MOBILE_EVENT_NETIF:
		LUAT_DEBUG_PRINT("internet工作状态变更为 %d,cause %d", status,index);
		switch (status)
		{
		case LUAT_MOBILE_NETIF_LINK_ON:
			LUAT_DEBUG_PRINT("可以上网");
			LUAT_DEBUG_PRINT("net ready");
			if (luat_mobile_get_apn(0, 0, apn, sizeof(apn)))
			{
				LUAT_DEBUG_PRINT("默认apn %s", apn);
			}
			luat_mobile_get_local_ip(0, 1, &ipv4, &ipv6);
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
	case LUAT_MOBILE_EVENT_FATAL_ERROR:
		LUAT_DEBUG_PRINT("网络需要严重故障，建议在5秒后重启协议栈");
		break;
	default:
		break;
	}
}

static int gpio_cb(int pin, void *param)
{
    if (SIM_HOTPLUG_PIN == pin)
    {
        if(luat_gpio_get(pin) == SIM_REMOVE){
            luat_rtos_event_send(g_s_task_handle, 0, SIM_REMOVE, 0, 0, 0);
        }else{
            luat_rtos_event_send(g_s_task_handle, 0, SIM_INSERT, 0, 0, 0);
        }
    }
}

static void test_task(void *param){
    luat_gpio_cfg_t cfg = {0};
    cfg.pin = SIM_HOTPLUG_PIN;
    cfg.mode = LUAT_GPIO_IRQ;
    cfg.pull = LUAT_GPIO_PULLUP;
    cfg.irq_type = LUAT_GPIO_BOTH_IRQ;
    cfg.irq_cb = gpio_cb;
    luat_gpio_open(&cfg);
    luat_event_t event = {0};
    while (1)
    {
        if(0 == luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER))
        {
            if (SIM_REMOVE == event.param1)
            {
                LUAT_DEBUG_PRINT("sim 卡拔出");
                luat_mobile_set_flymode(0, 1);
            }
            else
            {
                LUAT_DEBUG_PRINT("sim 卡插入");
                luat_mobile_set_flymode(0, 0);
            }
        }
    }
}
	

static void task_demo(void)
{
    luat_mobile_event_register_handler(luatos_mobile_event_callback);
    luat_rtos_task_create(&g_s_task_handle, 2048, 20, "test", test_task, NULL, 10);
}

INIT_TASK_EXPORT(task_demo,"1");



