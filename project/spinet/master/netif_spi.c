#include "csdk.h"
#include "spinet.h"
#ifdef CHIP_EC718
#define SPI_ID	SPI_ID1
#define CS_PIN HAL_GPIO_25
#define NEW_DATA_PIN	HAL_WAKEUP_0   //新数据提示
#endif
#ifdef CHIP_EC716
#define SPI_ID	SPI_ID0
#define CS_PIN  HAL_GPIO_14
#define NEW_DATA_PIN HAL_WAKEUP_5			//新数据提示
#endif
#define TIMEOUT 20
extern void netif_dump_ul_packet(u8_t *data, u16_t len, u8_t type);
extern void netif_dump_dl_packet(u8_t *data, u16_t len, u8_t type);
extern void soc_set_netif(struct netif *netif);
extern void soc_set_dns_local_port(uint16_t port);
typedef struct
{
	struct netif netif;
	llist_head downlink_queue;	//从机发给主机的数据，交给LWIP去处理
	llist_head uplink_queue;   //主机上行数据，需要发给从机
	llist_head cmd_queue;
	uint8_t link_state;
}spi_netif_ctrl_t;
static spi_netif_ctrl_t g_s_spi_netif;
enum
{
	SPI_NEW_TRANS = 0x01,
};

int master_new_data_in(int pin, void *pParam)
{
	return luat_rtos_event_send(pParam, SPI_NEW_TRANS, luat_gpio_get(NEW_DATA_PIN), 0, 0, 0);
}


static void hw_init(void)
{
    luat_spi_t spi_conf = {
        .id = SPI_ID,
        .CPHA = 1,
        .CPOL = 0,
        .dataw = 8,
        .bit_dict = 0,
		.master = 1,
        .mode = 1,             // mode设置为1，全双工
		.bandrate = 18600000,
//		.bandrate = 25600000,  // 从机是780EPM的话，可以提升到25.6M
        .cs = 0xff
    };
    luat_spi_setup(&spi_conf);

    luat_gpio_cfg_t cfg = {0};
    cfg.pin = CS_PIN;
    cfg.mode = LUAT_GPIO_OUTPUT;
    cfg.output_level = 1;
    luat_gpio_open(&cfg);


    cfg.pin = NEW_DATA_PIN;
    cfg.mode = LUAT_GPIO_IRQ;
    cfg.irq_type = LUAT_GPIO_BOTH_IRQ;
    cfg.irq_cb = master_new_data_in;
    cfg.irq_args = luat_rtos_get_current_handle();
    luat_gpio_open(&cfg);
}


static err_t spi_netif_init(struct netif *netif)
{
	return ERR_OK;
}

__USER_FUNC_IN_RAM__ err_t spi_netif_tx(struct pbuf *p)
{
	struct pbuf *q;
	uint32_t pos = 0;
	data_node_t *data;
	uint8_t *start;
	uint32_t cr = luat_rtos_entry_critical();
	if (!llist_empty(&g_s_spi_netif.uplink_queue))
	{
		data = (data_node_t *)g_s_spi_netif.uplink_queue.prev;
		if ((data->len + p->tot_len + 2) < PAYLOAD_MAX_LEN)
		{
			memcpy(data->data + data->len, &p->tot_len, 2);
			pos = data->len + 2;
			start = data->data + pos;
			for (q = p; q != NULL; q = q->next) {
				if (q->payload && q->len)
				{
					memcpy(data->data + pos, q->payload, q->len);
					pos += q->len;
				}
			}
			data->len = pos;
			luat_rtos_exit_critical(cr);
			netif_dump_ul_packet(start, p->tot_len, 5);
			luat_rtos_event_send(g_s_spi_netif.netif.state, SPI_NEW_TRANS, 1, 0, 0, 0);
			return ERR_OK;
		}
	}
	luat_rtos_exit_critical(cr);
	data = luat_heap_malloc(sizeof(data_node_t) + PAYLOAD_MAX_LEN);
	if (!data) return -ERR_MEM;
	memcpy(data->data, &p->tot_len, 2);
	pos = 2;
	start = data->data + pos;
	for (q = p; q != NULL; q = q->next) {
		if (q->payload && q->len)
		{
			memcpy(data->data + pos, q->payload, q->len);
			pos += q->len;
		}
	}
	netif_dump_ul_packet(start, p->tot_len, 5);
	data->len = pos;
	data->type = SPINET_TYPE_M_IP_DATA;
	cr = luat_rtos_entry_critical();
	llist_add_tail(&data->node, &g_s_spi_netif.uplink_queue);
	luat_rtos_exit_critical(cr);
	luat_rtos_event_send(g_s_spi_netif.netif.state, SPI_NEW_TRANS, 1, 0, 0, 0);
	return ERR_OK;
}

__USER_FUNC_IN_RAM__ err_t spi_netif_output(struct netif *netif, struct pbuf *p,
	       const ip4_addr_t *ipaddr)
{
	return spi_netif_tx(p);
}

__USER_FUNC_IN_RAM__ err_t spi_netif_output_ip6(struct netif *netif, struct pbuf *p,
	       const ip6_addr_t *ipaddr)
{
	return spi_netif_tx(p);
}

__USER_FUNC_IN_RAM__ void spi_netif_input(void *param)
{
	data_node_t *data = NULL;
	uint32_t pos;
	uint8_t end = 0;
	uint8_t *pp;
	struct pbuf p = {0};
	p.bIgnorFree = 1;
	p.type = PBUF_ROM;

	uint32_t cr;
	while (!end)
	{
		cr = luat_rtos_entry_critical();
		if (llist_empty(&g_s_spi_netif.downlink_queue))
		{
			end = 1;
			data = NULL;
		}
		else
		{
			data = (data_node_t *)g_s_spi_netif.downlink_queue.next;
			llist_del(&data->node);
		}
		luat_rtos_exit_critical(cr);
		if (data)
		{

			pos = 0;
			while(pos < data->len)
			{
				memcpy(&p.tot_len, data->data + pos, 2);
				p.len = p.tot_len;
				pos += 2;
				p.payload = data->data + pos;
				pp = p.payload;
				netif_dump_dl_packet(p.payload, p.tot_len, 5);
				p.ref = 1;
				pos += p.tot_len;
				ip_input(&p, &g_s_spi_netif.netif);
			}
			if (pos != data->len) {LUAT_DEBUG_PRINT("ERROR %d,%d", pos, data->len);}
			luat_heap_free(data);
		}
	}

}

static void spi_netif_rx(uint8_t *data, uint32_t len)
{
	data_node_t *node = luat_heap_malloc(sizeof(data_node_t) + len);
	if (!node)
	{
		LUAT_DEBUG_PRINT("NO MEM for spi netif rx!!!");
		return;
	}
	memcpy(node->data, data, len);
	node->len = len;
	uint32_t cr = luat_rtos_entry_critical();
	llist_add_tail(&node->node, &g_s_spi_netif.downlink_queue);
	luat_rtos_exit_critical(cr);
	tcpip_callback_with_block(spi_netif_input, NULL, 0);
}

static uint8_t netif_state_check(spinet_s_state_t *state, uint8_t old_link_state)
{
	ip_addr_t ip;
	uint8_t link_state;
	if (state->rf_power_on && state->sim_ready && state->online_state)
	{
		link_state = 1;
	}
	else
	{
		link_state = 0;
	}
	if (old_link_state != link_state)
	{
		print_spinet_slave_state(state);
		if (link_state && state->ip_state <= 0x03)
		{
			if (state->ip_state & 0x01)
			{
				ip.type = IPADDR_TYPE_V4;
				ip.u_addr.ip4.addr = state->ipv4;
				netif_set_ipaddr(&g_s_spi_netif.netif, &ip.u_addr.ip4);
			}
			if (state->ip_state & 0x02)
			{
				ip.type = IPADDR_TYPE_V6;
				memcpy(ip.u_addr.ip6.addr, state->ipv6.data, 16);
				netif_ip6_addr_set(&g_s_spi_netif.netif, 0, &ip.u_addr.ip6);
				netif_ip6_addr_set_state(&g_s_spi_netif.netif, 0, IP6_ADDR_PREFERRED);
			}
			for (int i = 0; i < 4; i++)
			{
				if (state->dns_state[i] && state->dns_state[i] < 3)
				{
					if (state->ip_state & 0x01)
					{
						ip.type = IPADDR_TYPE_V4;
						ip.u_addr.ip4.addr = state->dns[i].data[0];
					}
					else
					{
						ip.type = IPADDR_TYPE_V6;
						memcpy(ip.u_addr.ip6.addr, state->dns[i].data, 16);
					}
					network_set_dns_server(1, i, &ip);
				}
			}
			netif_set_link_up(&g_s_spi_netif.netif);
			net_lwip_set_link_state(1, 1);

		}
		else
		{
			netif_set_link_down(&g_s_spi_netif.netif);
			net_lwip_set_link_state(1, 0);
		}
	}
	return link_state;
}

static void *find_next_spi_packet(void)
{
	if (!llist_empty(&g_s_spi_netif.uplink_queue))
	{
		return g_s_spi_netif.uplink_queue.next;
	}
	if (!llist_empty(&g_s_spi_netif.cmd_queue))
	{
		return g_s_spi_netif.cmd_queue.next;
	}
	return NULL;
}

void netif_master_task(void *param)
{
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	uint8_t *temp_buf = luat_heap_malloc(PACKET_MAX_LEN);
	spinet_head_t m_head = {0};
	spinet_head_t s_head = {0};
	luat_event_t event;
	spinet_transfer_t trans;

	luat_rtos_task_handle handle = luat_rtos_get_current_handle();
	uint32_t tx_len;
	data_node_t *data = NULL;
	uint8_t *slave_data;
	uint32_t slave_data_len;
	spinet_s_state_t s_state = {0};
	uint8_t retry;
	int result;
	hw_init();
	trans.spi_id = SPI_ID;
	trans.cs_pin = CS_PIN;
	trans.tx_buf = luat_heap_malloc(PACKET_MAX_LEN);
	trans.rx_buf = luat_heap_malloc(PACKET_MAX_LEN);
	trans.buf_len = PACKET_MAX_LEN;
	luat_mobile_set_flymode(0, 1);
	luat_rtos_task_sleep(5000);
	INIT_LLIST_HEAD(&g_s_spi_netif.uplink_queue);
	INIT_LLIST_HEAD(&g_s_spi_netif.downlink_queue);
	INIT_LLIST_HEAD(&g_s_spi_netif.cmd_queue);
	g_s_spi_netif.netif.name[0] = 'S';
	g_s_spi_netif.netif.name[1] = 'P';
	g_s_spi_netif.netif.mtu = 1500;
	netif_add(&g_s_spi_netif.netif, NULL, NULL, NULL, NULL, spi_netif_init, NULL);
	netif_set_default(&g_s_spi_netif.netif);
	netif_set_up(&g_s_spi_netif.netif);
	g_s_spi_netif.netif.output = spi_netif_output;
	g_s_spi_netif.netif.output_ip6 = spi_netif_output_ip6;
	g_s_spi_netif.netif.netif_type = LWIP_NETIF_TYPE_WAN_INTERNET;
	g_s_spi_netif.netif.primary_ipv4_cid = 1;
	g_s_spi_netif.netif.primary_ipv6_cid = 1;
	g_s_spi_netif.netif.state = handle;
	soc_set_netif(&g_s_spi_netif.netif);
	soc_set_dns_local_port(200);
	uint32_t cr;
	while(1)
	{
		cr = luat_rtos_entry_critical();
		data = find_next_spi_packet();
		if (data)
		{
			llist_del(&data->node);
		}
		luat_rtos_exit_critical(cr);
		if (!luat_gpio_get(NEW_DATA_PIN) && !data)
		{
			luat_rtos_event_recv(handle, 0, &event, NULL, 0);
		}
		else
		{
			event.id = SPI_NEW_TRANS;
			event.param1 = 1;
		}
		switch(event.id)
		{
		case SPI_NEW_TRANS:
			if (event.param1)
			{
				if (data)
				{
					m_head.type = data->type;
					m_head.cmd = data->cmd;
					m_head.sn++;
					tx_len = pack_spinet_packet(&m_head, data->data, data->len, temp_buf);
					luat_heap_free(data);
					data = NULL;
				}
				else
				{
					m_head.type = SPINET_TYPE_M_GET_IP_DATA;
					tx_len = pack_spinet_packet(&m_head, NULL, 0, temp_buf);
				}
				for (retry = 0; retry < 3; retry++)
				{
					result = spinet_master_transfer(&trans, temp_buf, tx_len, TIMEOUT, &s_head, &slave_data, &slave_data_len);
					if (!result)
					{
						break;
					}
				}

				if (!result)
				{
					if (SPINET_TYPE_S_STATE == s_head.type)
					{
						memcpy(&s_state, slave_data, slave_data_len);
						g_s_spi_netif.link_state = netif_state_check(&s_state, g_s_spi_netif.link_state);
					}
					else if (SPINET_TYPE_S_IP_DATA == s_head.type)
					{
						if (g_s_spi_netif.link_state)
						{
							spi_netif_rx(slave_data, slave_data_len);
						}
					}
					if (s_head.more)
					{
						while (!result && s_head.more)
						{
							cr = luat_rtos_entry_critical();
							data = find_next_spi_packet();
							if (data)
							{
								llist_del(&data->node);
							}
							luat_rtos_exit_critical(cr);
							if (data)
							{
								m_head.type = data->type;
								m_head.cmd = data->cmd;
								m_head.sn++;
								tx_len = pack_spinet_packet(&m_head, data->data, data->len, temp_buf);
								luat_heap_free(data);
								data = NULL;
							}
							else
							{
								m_head.type = SPINET_TYPE_M_GET_IP_DATA;
								tx_len = pack_spinet_packet(&m_head, NULL, 0, temp_buf);
							}
							for (retry = 0; retry < 3; retry++)
							{
								result = spinet_master_transfer(&trans, temp_buf, tx_len, TIMEOUT, &s_head, &slave_data, &slave_data_len);
								if (!result) break;
							}
							if (!result)
							{
								if (SPINET_TYPE_S_STATE == s_head.type)
								{
									memcpy(&s_state, slave_data, slave_data_len);
									g_s_spi_netif.link_state = netif_state_check(&s_state, g_s_spi_netif.link_state);
								}
								else if (SPINET_TYPE_S_IP_DATA == s_head.type)
								{
									if (g_s_spi_netif.link_state)
									{
										spi_netif_rx(slave_data, slave_data_len);
									}
								}

							}
							else
							{
								DBG("failed,%x,%d", m_head.type, result);
							}
						}
					}
				}
				else
				{
					DBG("failed,%x,%d", m_head.type, result);
				}
			}
			break;
		}
	}
}

static void spinet_netif_master_init(void)
{
	luat_rtos_task_handle handle;
	luat_rtos_task_create(&handle, 4096, 50, "master", netif_master_task, NULL, 0);

}

INIT_TASK_EXPORT(spinet_netif_master_init, "1");
