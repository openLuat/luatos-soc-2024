#include "csdk.h"
#include "dns_def.h"
#include "soc_spi.h"
#include "ps_lib_api.h"
#include "networkmgr.h"
#include "eth.h"

#define SPI4G_CMD_HEAD 0x454D4341
#define SPI4G_PKT_HEAD 0x4153
#define SPI4G_CACHE_SIZE (6 * 1024)
#define SPI4G_MTU (SPI4G_CACHE_SIZE + 128)

#define SPI_ID SPI_ID1
#define IRQ_PIN HAL_GPIO_25
#define INT0_PIN HAL_GPIO_26
#define INT1_PIN HAL_GPIO_27
#define DBG_PIN HAL_GPIO_30

#define ALIGN_UP(v, n) (((unsigned long)(v) + (n)-1) & ~((n)-1))

static uint8_t dbg_pin_level = 1;

#pragma pack(push, 1)

typedef struct
{
    uint32_t head;
    uint16_t len;
    uint16_t rsvd;
} spi4g_cmd_t;

typedef struct 
{
    uint16_t head;
    uint16_t flag;
    uint16_t len;
    uint16_t chan;
} spi4g_pkt_t;

typedef struct
{
    uint16_t result;
    uint16_t protocol_version;
    uint32_t firmware_version;
} query_rsp_t;

typedef struct
{
    uint16_t result;
    uint16_t vif_num;
    uint16_t mtu;
    uint16_t capacity;
    uint16_t tx_buffer_size;
    uint16_t rx_buffer_size;
} query_netcard_rsp_t;

typedef struct
{
    uint16_t vif_idx;
    uint8_t type;
    uint16_t capacity;
} reg_netcard_t;

typedef struct
{
    uint16_t result;
    uint16_t vif_idx;
} reg_netcard_rsp_t;

typedef struct
{
    uint16_t vif_idx;
} query_link_status_t;

typedef struct
{
    uint16_t result;
    uint16_t vif_idx;
    uint8_t link_state;
} query_link_status_rsp_t;

typedef struct
{
    uint16_t vif_idx;
} query_ip_t;

typedef struct
{
    uint16_t result;
    uint16_t vif_idx;
    uint8_t ip4_addr[4];
    uint8_t ip4_mask[4];
    uint8_t ip4_gw[4];
    uint8_t ip4_dns1[4];
    uint8_t ip4_dns2[4];
} query_ip_rsp_t;

typedef struct
{
    uint16_t result;
    uint8_t state;
} pm_rsp_t;

#pragma pack(pop)

typedef struct
{
    llist_head node;
    uint16_t len;
    uint8_t *data;
} spi4g_node_t;

enum
{
    SPI_INT0 = 0x1,
    SPI_INT1,
    SPI_NOTIFY,
    SPI_DECODE,
};

enum
{
    SPI_IDLE = 0x1,
    SPI_STAG0,
    SPI_STAG1,
};

enum
{
    SPI_FLAG_CTRL = 0x0,
    SPI_FLAG_IND,
    SPI_FLAG_NET,
    SPI_FLAG_SERIAL,
};

enum
{
    SPI_CMD_QUERY = 0x0,
    SPI_CMD_QUERY_NETCARD,
    SPI_CMD_REG_NETCARD,
    SPI_CMD_QUERY_LINK_STATUS,
    SPI_CMD_QUERY_IP,
    SPI_CMD_PM,
    SPI_CMD_MAX,
};

typedef union
{
    reg_netcard_t reg_netcard;
    query_link_status_t query_link_status;
    query_ip_t query_ip;
} spi_cmd_t;

typedef union
{
    query_rsp_t query_rsp;
    query_netcard_rsp_t query_netcard_rsp;
    reg_netcard_rsp_t reg_netcard_rsp;
    query_link_status_rsp_t query_link_status_rsp;
    query_ip_rsp_t query_ip_rsp;
    pm_rsp_t pm_rsp;
} spi_rsp_t;

typedef struct
{
    uint8_t spi_cmd_size;
    uint8_t spi_rsp_size;
} spi_cmd_info_t;

extern void netif_dump_ul_packet(u8_t *data, u16_t len, u8_t type);
extern void netif_dump_dl_packet(u8_t *data, u16_t len, u8_t type);
extern BOOL PsifRawUlOutput(UINT8, UINT8 *, UINT16);
extern void luat_debug_dump(uint8_t *data, uint32_t len);
extern void delay_us(uint32_t us);
extern uint8_t GPIO_Input(uint32_t Pin);
extern void GPIO_Output(uint32_t Pin, uint8_t Level);
#if(WDT_FEATURE_ENABLE==1)
extern void WDT_kick(void);
extern void slpManAonWdtFeed(void);
#endif

static luat_rtos_task_handle slave_handle;
static luat_rtos_task_handle cmd_handle;
static uint8_t tx_buff[SPI4G_MTU];
static uint8_t rx_buff[SPI4G_MTU];
static uint8_t rx_bak[SPI4G_MTU];
static uint16_t rx_bak_len = 0;
static llist_head tx_queue;
static llist_head rx_queue;
static luat_rtos_mutex_t tx_queue_mutex;
static luat_rtos_mutex_t rx_queue_mutex;
static luat_rtos_timer_t tx_notify_timer;
static luat_rtos_timer_t rx_decode_timer;

static spi_cmd_info_t cmd_info[SPI_CMD_MAX] = {
    {0, sizeof(query_rsp_t)},
    {0, sizeof(query_netcard_rsp_t)},
    {sizeof(reg_netcard_t), sizeof(reg_netcard_rsp_t)},
    {sizeof(query_link_status_t), sizeof(query_link_status_rsp_t)},
    {sizeof(query_ip_t), sizeof(query_ip_rsp_t)},
    {0, sizeof(pm_rsp_t)}
};

static uint8_t host_mac[ETH_HWADDR_LEN] = {0};
static uint8_t dev_mac[ETH_HWADDR_LEN] = {0xfa, 0x32, 0x47, 0x15, 0xe1, 0x88};

static uint8_t state = SPI_IDLE;
static bool notify_send = false;

static int32_t spi_irq(void *pData, void *pParam)
{
    // LUAT_DEBUG_PRINT("spi transfer done");
    return 0;
}

__USER_FUNC_IN_RAM__ static int int0_irq(int pin, void *params)
{
    if (state == SPI_IDLE || state == SPI_STAG0)
    {
        state += 1;
        GPIO_Output(IRQ_PIN, 0);

        if (__builtin_expect(GPIO_Input(INT1_PIN), 0))
        {
            luat_rtos_event_send(slave_handle, SPI_INT1, 0, 0, 0, 0);
        }
    }
    else
    {
        // LUAT_DEBUG_PRINT("error state %d", state);
    }

    // luat_rtos_event_send(slave_handle, SPI_INT0, 0, 0, 0, 0);
    return 0;
}

__USER_FUNC_IN_RAM__ static int int1_irq(int pin, void *params)
{
    luat_rtos_event_send(slave_handle, SPI_INT1, 0, 0, 0, 0);
    return 0;
}

static void hw_init(void)
{
    luat_spi_t spi_conf = {
        .id = SPI_ID,
        .CPOL = 0,
        .CPHA = 1,
        .dataw = 8,
        .bit_dict = 0,
        .master = 0,
        .mode = 1,
        .bandrate = 1000000,
        // .cs = HAL_GPIO_12,
        .cs = 0xff,
    };
    luat_spi_setup(&spi_conf);

    SPI_SetCallbackFun(SPI_ID, spi_irq, NULL);

    NVIC_SetPriority(PXIC1_GPIO_IRQn, 0);

    luat_gpio_cfg_t cfg;
    memset(&cfg, 0, sizeof(cfg));
    cfg.pin = INT0_PIN;
    cfg.pull = Luat_GPIO_PULLUP;
    cfg.mode = Luat_GPIO_IRQ;
    cfg.irq_type = LUAT_GPIO_FALLING_IRQ;
    cfg.irq_cb = (void *)int0_irq;
    luat_gpio_open(&cfg);

    memset(&cfg, 0, sizeof(cfg));
    cfg.pin = INT1_PIN;
    cfg.pull = Luat_GPIO_PULLDOWN;
    cfg.mode = Luat_GPIO_IRQ;
    cfg.irq_type = LUAT_GPIO_RISING_IRQ;
    cfg.irq_cb = (void *)int1_irq;
    luat_gpio_open(&cfg);

    SPI_SlaveTransferStart(SPI_ID, tx_buff, rx_buff, SPI4G_MTU);

    memset(&cfg, 0, sizeof(cfg));
    cfg.pin = IRQ_PIN;
    cfg.pull = Luat_GPIO_DEFAULT;
    cfg.mode = Luat_GPIO_OUTPUT;
    cfg.output_level = 1;
    luat_gpio_open(&cfg);

    memset(&cfg, 0, sizeof(cfg));
    cfg.pin = DBG_PIN;
    cfg.pull = Luat_GPIO_DEFAULT;
    cfg.mode = Luat_GPIO_OUTPUT;
    cfg.output_level = dbg_pin_level;
    luat_gpio_open(&cfg);
}

__USER_FUNC_IN_RAM__ void spi_add_tx_node(llist_head *node)
{
    luat_rtos_mutex_lock(tx_queue_mutex, LUAT_WAIT_FOREVER);
    llist_add_tail(node, &tx_queue);
    luat_rtos_mutex_unlock(tx_queue_mutex);

    uint32_t cr = luat_rtos_entry_critical();
    if (!notify_send)
    {
        notify_send = true;
        luat_rtos_exit_critical(cr);

        luat_rtos_event_send(slave_handle, SPI_NOTIFY, 0, 0, 0, 0);
    }
    else
    {
        luat_rtos_exit_critical(cr);
    }
}

__USER_FUNC_IN_RAM__ spi4g_node_t *spi_alloc_tx_node(uint16_t flag, uint16_t chan, uint16_t len)
{
    spi4g_node_t *node = malloc(sizeof(spi4g_node_t) + sizeof(spi4g_pkt_t) + len);
    if (node == NULL)
    {
        return NULL;
    }

    node->data = (uint8_t *)(node + 1);
    node->len = sizeof(spi4g_pkt_t) + len;

    spi4g_pkt_t *pkt = (spi4g_pkt_t *)node->data;
    pkt->head = SPI4G_PKT_HEAD;
    pkt->flag = flag;
    pkt->chan = chan;
    pkt->len  = len;

    return node;
}

__USER_FUNC_IN_RAM__ void soc_netif_input_to_user(struct pbuf *p)
{
    spi4g_node_t *node = spi_alloc_tx_node(SPI_FLAG_NET, 0, sizeof(struct eth_hdr) + p->tot_len);

    if (node)
    {
        struct eth_hdr eth_hdr;
        memcpy(&eth_hdr.dest, host_mac, ETH_HWADDR_LEN);
        memcpy(&eth_hdr.src, dev_mac, ETH_HWADDR_LEN);
        uint8_t *ipdata = (uint8_t *)p->payload;
        if ((ipdata[0] & 0xF0) == 0x40)
            eth_hdr.type = __htons(ETHTYPE_IP);
        else if ((ipdata[0] & 0xF0) == 0x60)
            eth_hdr.type = __htons(ETHTYPE_IPV6);

        memcpy(node->data + sizeof(spi4g_pkt_t), &eth_hdr, sizeof(struct eth_hdr));

        uint16_t pos = sizeof(struct eth_hdr);
        for (struct pbuf *q = p; q != NULL; q = q->next) {
            memcpy(node->data + sizeof(spi4g_pkt_t) + pos, q->payload, q->len);
            pos += q->len;
        }

        spi_add_tx_node(&node->node);
    }
}

__USER_FUNC_IN_RAM__ BOOL soc_netif_output_from_user(uint8_t *data, uint32_t len)
{
	return PsifRawUlOutput(1, data, len);
}

static __USER_FUNC_IN_RAM__ void spi_netif_upload(void *param)
{
    luat_rtos_mutex_lock(rx_queue_mutex, LUAT_WAIT_FOREVER);
    while (1)
    {
        if (llist_empty(&rx_queue))
        {
            break;
        }

        spi4g_node_t *node = llist_entry(rx_queue.next, spi4g_node_t, node);
        llist_del(&node->node);

        luat_rtos_mutex_unlock(rx_queue_mutex);

        ethPacket_t *pkt = (ethPacket_t *)node->data;
        uint32_t len = node->len;

        if (isARPPackage(pkt, len))
        {
            // LUAT_DEBUG_PRINT("arp package");

            netif_dump_ul_packet(pkt->data, len, 5);

            struct etharp_hdr *reply = ARP_reply(pkt, dev_mac);
            if (reply)
            {
                // LUAT_DEBUG_PRINT("arp reply");

                memcpy(&host_mac, &reply->dhwaddr, ETH_HWADDR_LEN);

                spi4g_node_t *tmp = spi_alloc_tx_node(SPI_FLAG_NET, 0, sizeof(struct eth_hdr) + sizeof(struct etharp_hdr));
                if (tmp)
                {
                    struct eth_hdr eth_hdr;
                    memcpy(&eth_hdr.dest, host_mac, ETH_HWADDR_LEN);
                    memcpy(&eth_hdr.src, dev_mac, ETH_HWADDR_LEN);
                    eth_hdr.type = __htons(ETHTYPE_ARP);
                    memcpy(tmp->data + sizeof(spi4g_pkt_t), &eth_hdr, sizeof(struct eth_hdr));

                    memcpy(tmp->data + sizeof(spi4g_pkt_t) + sizeof(struct eth_hdr), reply, sizeof(struct etharp_hdr));

                    spi_add_tx_node(&tmp->node);
                }
            }
        }
        else if (isNeighborSolicitationPackage((ethPacket_t *)pkt, len))
        {
            // LUAT_DEBUG_PRINT("neighbor solicitation");
        }
        else
        {
            soc_netif_output_from_user(pkt->data, len - sizeof(struct eth_hdr));
        }

        free(node);

        luat_rtos_mutex_lock(rx_queue_mutex, LUAT_WAIT_FOREVER);
    }
    luat_rtos_mutex_unlock(rx_queue_mutex);
}

__USER_FUNC_IN_RAM__ static void rx_decode()
{
    if (rx_bak_len == 0)
    {
        return;
    }

    uint16_t pos = 0;
    while (1)
    {
        spi4g_pkt_t *pkt = (spi4g_pkt_t *)(rx_bak + pos);
        if (pkt->head != SPI4G_PKT_HEAD)
        {
            break;
        }

        if ((pos + sizeof(spi4g_pkt_t) + pkt->len) > rx_bak_len)
        {
            break;
        }

        pos += ALIGN_UP(sizeof(spi4g_pkt_t) + pkt->len, 8);

        if (pkt->flag == SPI_FLAG_CTRL)
        {
            void *mem = malloc(sizeof(spi4g_pkt_t) + pkt->len);
            if (mem)
            {
                memcpy(mem, pkt, sizeof(spi4g_pkt_t) + pkt->len);
                luat_rtos_event_send(cmd_handle, (uint32_t)mem, 0, 0, 0, 0);
            }
        }
        else if (pkt->flag == SPI_FLAG_NET)
        {
            spi4g_node_t *node = (spi4g_node_t *)malloc(sizeof(spi4g_node_t) + pkt->len);
            if (node)
            {
                node->len = pkt->len;
                node->data = (uint8_t *)(node + 1);
                memcpy(node->data, pkt + 1, pkt->len);

                uint8_t notify = 0;
                luat_rtos_mutex_lock(rx_queue_mutex, LUAT_WAIT_FOREVER);
                if (llist_empty(&rx_queue))
                {
                    notify = 1;
                }

                llist_add_tail(&node->node, &rx_queue);
                luat_rtos_mutex_unlock(rx_queue_mutex);

                if (notify)
                {
                    tcpip_callback_with_block(spi_netif_upload, NULL, 0);
                }
            }
        }
        else if (pkt->flag == SPI_FLAG_SERIAL)
        {
            spi4g_node_t *node = spi_alloc_tx_node(SPI_FLAG_SERIAL, pkt->chan, pkt->len);
            memcpy(node->data + sizeof(spi4g_pkt_t), pkt + 1, pkt->len);
            spi_add_tx_node(&node->node);
        }
        else
        {
            // LUAT_DEBUG_PRINT("error flag %d", pkt->flag);
        }
    }

    rx_bak_len = 0;
}

__USER_FUNC_IN_RAM__ static void slave_task(void *param)
{
    // luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
    luat_mobile_data_ip_mode(0xa7); // IPV4包全部透传给主机，且不给本地LWIP。如果从机也要能收发，需要改成0x01，且主机使用的端口 >= 100, < 50000

    hw_init();

    INIT_LLIST_HEAD(&tx_queue);
    INIT_LLIST_HEAD(&rx_queue);

    uint16_t ul = 0;
    uint16_t dl = 0;
    while (1)
    {
    #if(WDT_FEATURE_ENABLE==1)
        WDT_kick();
        slpManAonWdtFeed();
    #endif
    
        luat_event_t event = {0};
        luat_rtos_event_recv(slave_handle, 0, &event, NULL, 0);

        // LUAT_DEBUG_PRINT("event id %d, state %d", event.id, state);
        dbg_pin_level = !dbg_pin_level;
        luat_gpio_set(DBG_PIN, dbg_pin_level);

        switch (event.id)
        {
            case SPI_INT0:
            {
                break;
            }

            case SPI_INT1:
            {
                bool need_decode = false;
                uint32_t rx_len = SPI_SlaveTransferStopAndGetRxLen(SPI_ID);

                if (state == SPI_STAG1)
                {
                    if (dl != 0)
                    {
                        if (rx_len == sizeof(spi4g_cmd_t) + dl)
                        {
                            memcpy(rx_bak, rx_buff + sizeof(spi4g_cmd_t), dl);
                            rx_bak_len = dl;

                            luat_start_rtos_timer(rx_decode_timer, 1, 0);
                        }
                        else
                        {
                            // LUAT_DEBUG_PRINT("error rx len %d", rx_len);
                        }
                    }

                    state = SPI_IDLE;
                }
                else
                {
                    if (state == SPI_STAG0)
                    {
                        if (rx_len == sizeof(spi4g_cmd_t))
                        {
                            spi4g_cmd_t *cmd = (spi4g_cmd_t *)rx_buff;
                            if (cmd->head == SPI4G_CMD_HEAD)
                            {
                                dl = cmd->len;
    
                                cmd = (spi4g_cmd_t *)tx_buff;
                                cmd->head = SPI4G_CMD_HEAD;
                                cmd->rsvd = 0;

                                if (dl == 0)
                                {
                                    uint16_t pos = 0;
                                    llist_head head;
                                    INIT_LLIST_HEAD(&head);

                                    luat_rtos_mutex_lock(tx_queue_mutex, LUAT_WAIT_FOREVER);
                                    while (!llist_empty(&tx_queue))
                                    {
                                        spi4g_node_t *node = llist_entry(tx_queue.next, spi4g_node_t, node);

                                        if (ALIGN_UP(pos + node->len, 8) > SPI4G_CACHE_SIZE)
                                        {
                                            break;
                                        }

                                        llist_del(&node->node);
                                        luat_rtos_mutex_unlock(tx_queue_mutex);

                                        pos += ALIGN_UP(node->len, 8);

                                        llist_add_tail(&node->node, &head);

                                        luat_rtos_mutex_lock(tx_queue_mutex, LUAT_WAIT_FOREVER);
                                    }
                                    luat_rtos_mutex_unlock(tx_queue_mutex);

                                    if (!llist_empty(&head))
                                    {
                                        cmd->len = pos;

                                        pos = 0;
                                        while (!llist_empty(&head))
                                        {
                                            spi4g_node_t *node = llist_entry(head.next, spi4g_node_t, node);
                                            llist_del(&node->node);

                                            memcpy((uint8_t *)(cmd + 1) + pos, node->data, node->len);

                                            pos += ALIGN_UP(node->len, 8);

                                            free(node);
                                        }
                                    }
                                    else
                                    {
                                        cmd->len  = 0;

                                        // LUAT_DEBUG_PRINT("no data");
                                    }

                                    ul = cmd->len;
                                }
                                else
                                {
                                    ul = 0;
                                    cmd->len = 0;
                                    need_decode = (rx_bak_len > 0);
                                }
                            }
                            else
                            {
                                // LUAT_DEBUG_PRINT("cmd head error");
    
                                state = SPI_IDLE;
                            }
                        }
                        else
                        {
                            // LUAT_DEBUG_PRINT("cmd len error");
    
                            state = SPI_IDLE;
                        }
                    }
                    else
                    {
                        // LUAT_DEBUG_PRINT("error state %d", state);

                        state = SPI_IDLE;
                    }
                }

                if (state == SPI_IDLE)
                {
                    memset(tx_buff, 0, sizeof(spi4g_cmd_t) + ul);
                    // memset(rx_buff, 0, SPI4G_MTU);
                }

                SPI_SlaveTransferStart(SPI_ID, tx_buff, rx_buff, SPI4G_MTU);
                luat_gpio_set(IRQ_PIN, 1);

                if (need_decode)
                {
                    luat_stop_rtos_timer(rx_decode_timer);
                    rx_decode();
                }

                break;
            }

            case SPI_NOTIFY:
            {
                uint32_t cr = luat_rtos_entry_critical();

                if (state == SPI_IDLE)
                {
                    GPIO_Output(IRQ_PIN, 0);
                    delay_us(2);
                    GPIO_Output(IRQ_PIN, 1);

                    notify_send = false;

                    luat_rtos_exit_critical(cr);
                }
                else
                {
                    luat_rtos_exit_critical(cr);

                    luat_start_rtos_timer(tx_notify_timer, 1, 0);
                }

                break;
            }

            case SPI_DECODE:
            {
                if (rx_bak_len > 0)
                {
                    if (state == SPI_IDLE)
                    {
                        rx_decode();
                    }
                    else
                    {
                        luat_start_rtos_timer(rx_decode_timer, 1, 0);
                    }
                }
                break;
            }

            default:
                break;
        }
    }
}

static void mobile_event_cb(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
    NmAtiNetifInfo net_info;

    switch (event)
    {
        case LUAT_MOBILE_EVENT_NETIF:
        {
            if (status == LUAT_MOBILE_NETIF_LINK_ON)
            {
                appGetNetInfoSync(0, &net_info);

                LUAT_DEBUG_PRINT("ip addr %s", inet_ntoa(net_info.ipv4Info.ipv4Addr));
            }
            break;
        }

        default:
        {
            break;
        }
    }
}

static void handle_cmd(uint8_t cmd, spi_cmd_t *spi_cmd, spi_rsp_t *spi_rsp)
{
    query_rsp_t *query_rsp = &spi_rsp->query_rsp;
    query_netcard_rsp_t *query_netcard_rsp = &spi_rsp->query_netcard_rsp;
    reg_netcard_rsp_t *reg_netcard_rsp = &spi_rsp->reg_netcard_rsp;
    query_link_status_rsp_t *query_link_status_rsp = &spi_rsp->query_link_status_rsp;
    query_ip_rsp_t *query_ip_rsp = &spi_rsp->query_ip_rsp;
    pm_rsp_t *pm_rsp = &spi_rsp->pm_rsp;

    NmAtiNetifInfo net_info;

    switch (cmd)
    {
        case SPI_CMD_QUERY:
        {
            query_rsp->result = 0;
            query_rsp->protocol_version = 0x1;
            query_rsp->firmware_version = 0x1;
            break;
        }

        case SPI_CMD_QUERY_NETCARD:
        {
            query_netcard_rsp->result = 0;
            query_netcard_rsp->vif_num = 1;
            query_netcard_rsp->mtu = 1500;
            query_netcard_rsp->capacity = 0xb;
            query_netcard_rsp->tx_buffer_size = SPI4G_CACHE_SIZE;
            query_netcard_rsp->rx_buffer_size = SPI4G_CACHE_SIZE;
            break;
        }

        case SPI_CMD_REG_NETCARD:
        {
            reg_netcard_rsp->result = 0;
            reg_netcard_rsp->vif_idx = 0;
            break;
        }

        case SPI_CMD_QUERY_LINK_STATUS:
        {
            query_link_status_rsp->result = 0;
            reg_netcard_rsp->vif_idx = 0;
            appGetNetInfoSync(0, &net_info);
            if (net_info.netStatus == NM_NETIF_ACTIVATED)
            {
                query_link_status_rsp->link_state = 1;
            }
            else
            {
                query_link_status_rsp->link_state = 0;
            }
            break;
        }

        case SPI_CMD_QUERY_IP:
        {
            appGetNetInfoSync(0, &net_info);
            if (net_info.netStatus != NM_NETIF_ACTIVATED)
            {
                query_ip_rsp->result = 1;
            }
            else
            {
                if (net_info.ipType == NM_NET_TYPE_IPV4 || net_info.ipType == NM_NET_TYPE_IPV4V6)
                {
                    query_ip_rsp->result = 0;
                    query_ip_rsp->vif_idx = 0;
                    memcpy(query_ip_rsp->ip4_addr, &net_info.ipv4Info.ipv4Addr, 4);
                    if (net_info.ipv4Info.dnsNum >= 1)
                    {
                        memcpy(query_ip_rsp->ip4_dns1, &net_info.ipv4Info.dns[0], 4);
                    }
                    if (net_info.ipv4Info.dnsNum >= 2)
                    {
                        memcpy(query_ip_rsp->ip4_dns2, &net_info.ipv4Info.dns[1], 4);
                    }
                }
                else
                {
                    query_ip_rsp->result = 1;
                }
            }
            break;
        }

        case SPI_CMD_PM:
        {
            break;
        }

        default:
        {
            break;
        }
    }
}

static void cmd_task(void *param)
{
    spi4g_pkt_t *pkt = NULL;
    spi_cmd_t spi_cmd = {0};
    spi_rsp_t spi_rsp = {0};

    while (1)
    {
        if (pkt != NULL)
        {
            free(pkt);
        }

        luat_event_t event = {0};
        luat_rtos_event_recv(cmd_handle, 0, &event, NULL, 0);

        pkt = (spi4g_pkt_t *)event.id;

        uint16_t cmd = pkt->chan;
        if (cmd >= SPI_CMD_MAX)
        {
            // LUAT_DEBUG_PRINT("cmd >= SPI_CMD_MAX");
            continue;
        }

        if (pkt->len != cmd_info[cmd].spi_cmd_size)
        {
            // LUAT_DEBUG_PRINT("pkt->len(%d) != %d", pkt->len, cmd_info[cmd].spi_cmd_size);
            continue;
        }

        memset(&spi_rsp, 0, sizeof(spi_rsp));
        if (cmd_info[cmd].spi_cmd_size != 0)
        {
            memcpy(&spi_cmd, (uint8_t *)pkt + sizeof(spi4g_pkt_t), cmd_info[cmd].spi_cmd_size);
        }
        handle_cmd(cmd, &spi_cmd, &spi_rsp);

        spi4g_node_t *node = calloc(1, sizeof(spi4g_node_t) + sizeof(spi4g_pkt_t) + cmd_info[cmd].spi_rsp_size);
        if (node != NULL)
        {
            node->data = (uint8_t *)(node + 1);
            node->len = sizeof(spi4g_pkt_t) + cmd_info[cmd].spi_rsp_size;

            spi4g_pkt_t *tmp = (spi4g_pkt_t *)node->data;
            tmp->head = SPI4G_PKT_HEAD;
            tmp->flag = SPI_FLAG_CTRL;
            tmp->len  = cmd_info[cmd].spi_rsp_size;
            tmp->chan = cmd | 0x8000;

            memcpy(tmp + 1, &spi_rsp, cmd_info[cmd].spi_rsp_size);

            spi_add_tx_node(&node->node);
        }
    }
}

static LUAT_RT_RET_TYPE rx_decode_timer_cb(LUAT_RT_CB_PARAM)
{
    luat_rtos_event_send(slave_handle, SPI_DECODE, 0, 0, 0, 0);
}

static LUAT_RT_RET_TYPE tx_notify_timer_cb(LUAT_RT_CB_PARAM)
{
    luat_rtos_event_send(slave_handle, SPI_NOTIFY, 0, 0, 0, 0);
}

static void spinet_netif_slave_init(void)
{
    luat_rtos_mutex_create(&tx_queue_mutex);
    luat_rtos_mutex_create(&rx_queue_mutex);

    luat_mobile_event_register_handler(mobile_event_cb);
    rx_decode_timer = luat_create_rtos_timer(rx_decode_timer_cb, NULL, NULL);
    tx_notify_timer = luat_create_rtos_timer(tx_notify_timer_cb, NULL, NULL);
    luat_rtos_task_create(&slave_handle, 4096, 200, "slave", slave_task, NULL, 256);
    luat_rtos_task_create(&cmd_handle, 4096, 100, "cmd", cmd_task, NULL, 256);
    luat_mobile_set_default_pdn_ipv6(0);
}

INIT_TASK_EXPORT(spinet_netif_slave_init, "1");
