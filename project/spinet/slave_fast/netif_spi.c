#include "csdk.h"
#include "spinet.h"
#include "dns_def.h"
#include "bget.h"
#include "soc_spi.h"
/**
 * 特别说明，为了在中断里使用heap分配，所以使用bget，而不是freertos自带的heap
 */
#define SPI_ID	SPI_ID0
#define CS_PIN HAL_WAKEUP_5
#define NEW_DATA_PIN	HAL_GPIO_14		//新数据提示
extern void netif_dump_ul_packet(u8_t *data, u16_t len, u8_t type);
extern void netif_dump_dl_packet(u8_t *data, u16_t len, u8_t type);
static uint64_t netif_mem[128 * 128];
typedef struct
{
	llist_head downlink_queue;		//网络下发的数据，需要发送给主机
	llist_head uplink_queue;		//主机发送过来的数据，发给网络
	uint8_t *spi_tx_buf;			//SPI DMA发送缓存
	uint8_t *spi_rx_buf;			//SPI DMA接收缓存
	uint8_t *spi_data_buf;			//中断里数据处理缓存
	spinet_s_state_t state;			//从机状态
	luat_rtos_task_handle handle;
	uint8_t new_state_flag;			//从机状态更新，需要发送给主机
	uint8_t spi_new_data_flag;		//从机接收到了SPI数据
	uint8_t spi_sleep;				//SPI休眠状态
}spi_netif_slave_ctrl_t;

enum
{
	SPI_CS_EVENT = 0x01,	//CSpin有变化
	SPI_SLEEP,				//SPI休眠状态变化
	SPI_NEW_IP_STATE,		//从机的IP状态更新
	SPI_NEW_STATE,			//从机其他状态更新
};

extern int soc_mobile_get_default_pdp_part_info(uint8_t *ip_type, uint8_t *apn,uint8_t *apn_len, uint8_t *dns_num, ip_addr_t *dns_ip);

static spi_netif_slave_ctrl_t g_s_spi_netif;

/**
 * @brief cs pin中断
 * @param pin
 * @param pParam
 * @return
 */
static int slave_spi_cs(int pin, void *pParam)
{
	uint8_t level = luat_gpio_get(pin);
	if (level)	//CS拉高，说明单次SPI传输结束
	{
		g_s_spi_netif.spi_new_data_flag = 1;
		spinet_head_t head, s_head;
		uint8_t *master_data;
		uint32_t tx_len, rx_len, master_data_len, cr;
		data_node_t *data = NULL;
		int result;
		rx_len = SPI_SlaveGetRxLenFast(SPI_ID);	//获取本次传输的长度
		if (rx_len > 12)
		{
			memcpy(g_s_spi_netif.spi_data_buf, g_s_spi_netif.spi_rx_buf, rx_len);
			result = unpack_spinet_packet(g_s_spi_netif.spi_data_buf, rx_len, &head, &master_data, &master_data_len);
			if (!result)
			{
				if (llist_empty(&g_s_spi_netif.downlink_queue) && !g_s_spi_netif.new_state_flag) //没有任何数据了
				{
					memset(g_s_spi_netif.spi_tx_buf, 0, PACKET_MAX_LEN);
					s_head.type = SPINET_TYPE_S_NOP;
					s_head.more = 0;
					tx_len = pack_spinet_packet(&s_head, NULL, 0, g_s_spi_netif.spi_tx_buf + 4);//从机SPI第一个发送数据总是0，所以跳过4个字节
					luat_gpio_set(NEW_DATA_PIN, 0);
				}
				else if (!g_s_spi_netif.new_state_flag)
				{
					data = (data_node_t *)g_s_spi_netif.downlink_queue.next;
					llist_del(&data->node);
					s_head.type = data->type;
					s_head.cmd = data->cmd;
					s_head.more = 1;
					tx_len = pack_spinet_packet(&s_head, data->data, data->len, g_s_spi_netif.spi_tx_buf + 4);
					brel(data);
				}
				else
				{
					g_s_spi_netif.new_state_flag = 0;
					s_head.type = SPINET_TYPE_S_STATE;
					s_head.more = 1;
					tx_len = pack_spinet_packet(&s_head, &g_s_spi_netif.state, sizeof(g_s_spi_netif.state), g_s_spi_netif.spi_tx_buf + 4);
				}
				SPI_SlaveStartNextFast(SPI_ID, g_s_spi_netif.spi_tx_buf, g_s_spi_netif.spi_rx_buf, PACKET_MAX_LEN);
				switch(head.type)
				{
				//case SPINET_TYPE_M_CMD:
				case SPINET_TYPE_M_IP_DATA:
					data = bget(sizeof(data_node_t) + master_data_len);
					if (!data)
					{
						break;
					}
					data->len = master_data_len;
					memcpy(data->data, master_data, master_data_len);
					data->type = head.type;
					data->cmd = head.cmd;
					llist_add_tail(&data->node, &g_s_spi_netif.uplink_queue);
					luat_rtos_event_send(pParam, SPI_CS_EVENT, 1, 0, 0, 0);
					break;
				}
			}
			else
			{
				memset(g_s_spi_netif.spi_rx_buf, 0, PACKET_MAX_LEN);
				SPI_SlaveStartNextFast(SPI_ID, g_s_spi_netif.spi_tx_buf, g_s_spi_netif.spi_rx_buf, PACKET_MAX_LEN);
			}
		}
		else
		{
			SPI_SlaveStartNextFast(SPI_ID, g_s_spi_netif.spi_tx_buf, g_s_spi_netif.spi_rx_buf, PACKET_MAX_LEN);
		}
	}
	else
	{
		if (g_s_spi_netif.spi_sleep)	//CS拉低，如果从机SPI休眠状态，则唤醒进入传输状态
		{
			g_s_spi_netif.spi_sleep = 0;
			uint32_t tx_len;
			spinet_head_t s_head;
			s_head.type = SPINET_TYPE_S_NOP;
			s_head.more = 0;
			tx_len = pack_spinet_packet(&s_head, NULL, 0, g_s_spi_netif.spi_tx_buf + 4);
			SPI_SlaveTransferStart(SPI_ID, g_s_spi_netif.spi_tx_buf, g_s_spi_netif.spi_rx_buf, PACKET_MAX_LEN);
		}
		return luat_rtos_event_send(pParam, SPI_CS_EVENT, 0, 0, 0, 0);
	}
	return 0;
}

static LUAT_RT_RET_TYPE spi_sleep_enable(LUAT_RT_CB_PARAM)
{
	if (g_s_spi_netif.spi_new_data_flag)
	{
		g_s_spi_netif.spi_new_data_flag = 0;
	}
	else
	{
		luat_rtos_event_send(g_s_spi_netif.handle, SPI_SLEEP, 0, 0, 0, 0);
	}
}

static void hw_init(void)
{
    luat_spi_t spi_conf = {
        .id = SPI_ID,
        .CPHA = 1,
        .CPOL = 0,
        .dataw = 8,
        .bit_dict = 0,
		.master = 0,
        .mode = 1,             // mode设置为1，全双工
		.bandrate = 52000000,
        .cs = 0xff
    };
    luat_spi_setup(&spi_conf);

    luat_gpio_cfg_t cfg = {0};
    cfg.pin = NEW_DATA_PIN;
    cfg.mode = LUAT_GPIO_OUTPUT;
    cfg.output_level = 0;
    luat_gpio_open(&cfg);


    cfg.pin = CS_PIN;
    cfg.mode = LUAT_GPIO_IRQ;
    cfg.irq_type = LUAT_GPIO_BOTH_IRQ;
    cfg.irq_cb = slave_spi_cs;
    cfg.irq_args = g_s_spi_netif.handle;
    luat_gpio_open(&cfg);
}

static void mobile_event_cb(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	luat_mobile_signal_strength_info_t signal_info;
	uint8_t type, dns_num;
	switch(event)
	{
	case LUAT_MOBILE_EVENT_CFUN:
		LUAT_DEBUG_PRINT("CFUN消息，status %d", status);
		g_s_spi_netif.state.rf_power_on = (status == 1)?1:0;
		luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
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
			g_s_spi_netif.state.sim_ready = 1;
			luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
			break;
		case LUAT_MOBILE_NO_SIM:
			g_s_spi_netif.state.sim_ready = 1;
			luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
			LUAT_DEBUG_PRINT("SIM卡不存在");
			break;
		case LUAT_MOBILE_SIM_NEED_PIN:
			LUAT_DEBUG_PRINT("SIM卡需要输入PIN码");
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_REGISTER_STATUS:
		LUAT_DEBUG_PRINT("移动网络服务状态变更，当前为%d", status);
		g_s_spi_netif.state.reg_state = status;
		luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
		break;
	case LUAT_MOBILE_EVENT_CELL_INFO:
		switch(status)
		{
		case LUAT_MOBILE_CELL_INFO_UPDATE:
			LUAT_DEBUG_PRINT("周期性搜索小区信息完成一次");
			break;
		case LUAT_MOBILE_SIGNAL_UPDATE:
			LUAT_DEBUG_PRINT("服务小区信号状态变更");
			luat_mobile_get_last_notify_signal_strength_info(&signal_info);
			g_s_spi_netif.state.rsrp = -signal_info.lte_signal_strength.rsrp;
			g_s_spi_netif.state.rsrq = signal_info.lte_signal_strength.rsrq;
			g_s_spi_netif.state.snr = signal_info.lte_signal_strength.snr;
			luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
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
			soc_mobile_get_default_pdp_part_info(&type, NULL, NULL, &dns_num, NULL);
			if (type & 0x80)
			{
				if (index != 4)
				{
					return;
				}
			}
			g_s_spi_netif.state.online_state = 1;
			g_s_spi_netif.state.ip_state = 0;
			luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_IP_STATE, 0, 0, 0, 0);
			LUAT_DEBUG_PRINT("可以上网");
			break;
		default:
			LUAT_DEBUG_PRINT("不能上网");
			g_s_spi_netif.state.online_state = 0;
			luat_rtos_event_send(g_s_spi_netif.handle, SPI_NEW_STATE, 0, 0, 0, 0);
			break;
		}
		break;
	case LUAT_MOBILE_EVENT_TIME_SYNC:
		if (!status)
		{
			LUAT_DEBUG_PRINT("通过移动网络同步了UTC时间");
		}
		else
		{
			LUAT_DEBUG_PRINT("移动网络同步UTC时间出错");
		}

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
//底层自动调用，为了加快速度，必须放在ram中运行
__USER_FUNC_IN_RAM__ void soc_netif_input_to_user(struct pbuf *p)
{
	struct pbuf *q;
	uint32_t pos = 0;
	uint8_t *start;
	data_node_t *data;
	uint32_t cr = luat_rtos_entry_critical();

	if (!llist_empty(&g_s_spi_netif.downlink_queue))
	{
		data = (data_node_t *)g_s_spi_netif.downlink_queue.prev;
		if ((data->len + p->tot_len + 2) < PAYLOAD_MAX_LEN)
		{
			memcpy(data->data + data->len, &p->tot_len, 2);
			pos = data->len + 2;
			start = data->data + pos;
			for (q = p; q != NULL; q = q->next) {

				memcpy(data->data + pos, q->payload, q->len);
				pos += q->len;
			}
			data->len = pos;
			netif_dump_dl_packet(start, p->tot_len, 5);
			luat_rtos_exit_critical(cr);
			return;
		}
	}
	data = bgetz(sizeof(data_node_t) + PAYLOAD_MAX_LEN);
	luat_rtos_exit_critical(cr);

	if (!data) return;
	memcpy(data->data, &p->tot_len, 2);
	pos = 2;
	start = data->data + 2;
	for (q = p; q != NULL; q = q->next) {
		memcpy(data->data + pos, q->payload, q->len);
		pos += q->len;
	}
	netif_dump_dl_packet(start, p->tot_len, 5);
	data->len = pos;
	data->type = SPINET_TYPE_S_IP_DATA;
	cr = luat_rtos_entry_critical();
	llist_add_tail(&data->node, &g_s_spi_netif.downlink_queue);
	luat_rtos_exit_critical(cr);
	luat_gpio_set(NEW_DATA_PIN, 1);
}

__USER_FUNC_IN_RAM__ BOOL soc_netif_output_from_user(uint8_t *data, uint32_t len)
{
	extern BOOL PsifRawUlOutput(UINT8, UINT8 *, UINT16);
	return PsifRawUlOutput(1, data, len);
}

static __USER_FUNC_IN_RAM__ void spi_netif_upload(void *param)
{
	data_node_t *data = NULL;
	uint32_t pos;
	uint16_t len;
	uint8_t end = 0;
	uint32_t cr;
	while (!end)
	{
		cr = luat_rtos_entry_critical();
		if (llist_empty(&g_s_spi_netif.uplink_queue))
		{
			end = 1;
			data = NULL;
		}
		else
		{
			data = (data_node_t *)g_s_spi_netif.uplink_queue.next;
			llist_del(&data->node);
		}
		luat_rtos_exit_critical(cr);
		if (data)
		{

			pos = 0;
			while(pos < data->len)
			{
				memcpy(&len, data->data + pos, 2);
				pos += 2;
				soc_netif_output_from_user(data->data + pos, len);//理论上应该再加一个返回值判断，失败后应该等一段时间重发
				netif_dump_ul_packet(data->data + pos, len, 5);
				pos += len;
			}
			if (pos != data->len) {LUAT_DEBUG_PRINT("ERROR %d,%d", pos, data->len);}
			cr = luat_rtos_entry_critical();
			brel(data);
			luat_rtos_exit_critical(cr);
		}
	}

}

void netif_slave_task(void *param)
{
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	luat_mobile_data_ip_mode(0xa7);	//IPV4包全部透传给主机，且不给本地LWIP。如果从机也要能收发，需要改成0x01，且主机使用的端口 >= 100, < 50000
	g_s_spi_netif.state.rf_power_on = 1;
	luat_event_t event;
	g_s_spi_netif.spi_tx_buf = luat_heap_calloc(1, PACKET_MAX_LEN);
	g_s_spi_netif.spi_rx_buf = luat_heap_calloc(1, PACKET_MAX_LEN);
	g_s_spi_netif.spi_data_buf = luat_heap_calloc(1, PACKET_MAX_LEN);
	uint32_t i;
	luat_rtos_task_handle handle = luat_rtos_get_current_handle();
	luat_ip_addr_t ip;
	struct netif *netif;
	dns_client_t *dns;
	ip_addr_t *ipv6;
	uint32_t cr;
	void *sleep_timer = luat_create_rtos_timer(spi_sleep_enable, NULL, NULL);
	INIT_LLIST_HEAD(&g_s_spi_netif.uplink_queue);
	INIT_LLIST_HEAD(&g_s_spi_netif.downlink_queue);
	hw_init();
	g_s_spi_netif.spi_sleep = 1;
	while(1)
	{
		luat_rtos_event_recv(handle, 0, &event, NULL, 0);
		switch (event.id)
		{
		case SPI_CS_EVENT:
			if (event.param1)
			{
				luat_start_rtos_timer(sleep_timer, 1000, 1);//无SPI传输1秒后SPI休眠，可以根据实际情况调整时间
				tcpip_callback_with_block(spi_netif_upload, NULL, 0);
			}
			break;
		case SPI_SLEEP:
			DBG("!");
			if (!g_s_spi_netif.spi_sleep && luat_gpio_get(CS_PIN))
			{
				luat_stop_rtos_timer(sleep_timer);
				g_s_spi_netif.spi_sleep = 1;
				SPI_SlaveTransferStop(SPI_ID);
				DBG("SPI SLEEP!");
			}
			break;
		case SPI_NEW_IP_STATE:
			netif = net_lwip_get_netif(1);
			if (netif->ip_addr.type == IPADDR_TYPE_V4)
			{
				g_s_spi_netif.state.ip_state |= 0x1;
				g_s_spi_netif.state.ipv4 = netif->ip_addr.u_addr.ip4.addr;
			}
			ipv6 = net_lwip_get_ip6_preferred(1);
			if (ipv6)
			{
				g_s_spi_netif.state.ip_state |= 0x2;
				memcpy(g_s_spi_netif.state.ipv6.data, ipv6->u_addr.ip6.addr, 16);
			}
			dns = net_lwip_get_dns_client(1);
			for(i = 0; i < 4; i++)
			{
				if (network_ip_is_vaild(&dns->dns_server[i]))
				{
					if ((dns->dns_server[i].type == IPADDR_TYPE_V4))
					{
						g_s_spi_netif.state.dns_state[i] = 1;
						g_s_spi_netif.state.dns[i].data[0] = dns->dns_server[i].u_addr.ip4.addr;
					}
					else
					{
						g_s_spi_netif.state.dns_state[i] = 2;
						memcpy(g_s_spi_netif.state.dns[i].data, dns->dns_server[i].u_addr.ip6.addr, 16);

					}
				}
				else
				{
					g_s_spi_netif.state.dns_state[i] = 0;
				}
			}
			cr = luat_rtos_entry_critical();
			g_s_spi_netif.new_state_flag = 1;
			luat_rtos_exit_critical(cr);
			luat_gpio_set(NEW_DATA_PIN, 1);
			break;
		case SPI_NEW_STATE:
			cr = luat_rtos_entry_critical();
			g_s_spi_netif.new_state_flag = 1;
			luat_rtos_exit_critical(cr);
			luat_gpio_set(NEW_DATA_PIN, 1);
			break;
		}
	}
}

static void spinet_netif_slave_init(void)
{
	luat_mobile_event_register_handler(mobile_event_cb);
	luat_rtos_task_create(&g_s_spi_netif.handle, 4096, 100, "slave", netif_slave_task, NULL, 256);
	luat_mobile_set_default_pdn_ipv6(0);
	bpool(netif_mem, sizeof(netif_mem));//使用bget做一部分heap管理
}

INIT_TASK_EXPORT(spinet_netif_slave_init, "1");
