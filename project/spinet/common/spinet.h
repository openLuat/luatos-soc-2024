#ifndef __SPINET_H__
#define __SPINET_H__
#define PACKET_MAX_LEN	4196	//1个包最大长度，只要不超过8188都可以
#define PAYLOAD_MAX_LEN	4144	//SPINET DATA，不超过PACKET_MAX_LEN - 8
typedef struct
{
	uint8_t type;//包类型，见SPINET_TYPE_xxx
	uint8_t cmd;//命令包中具体命令，目前没有作用
	uint8_t sn;//包序号，目前也没作用
	uint8_t more;//是否还有更多的包
}spinet_head_t;

typedef struct
{
	llist_head node;//链表节点
	uint16_t len;//有效载荷长度
	uint8_t type;//包类型
	uint8_t cmd;//命令包中具体命令
	uint8_t data[0];//有效载荷
}data_node_t;

typedef struct
{
	uint8_t *tx_buf;//SPI DMA发送缓存
	uint8_t *rx_buf;//SPI DMA接收缓存
	uint16_t buf_len;//SPI DMA缓存长度
	uint8_t spi_id;//SPI ID
	uint8_t cs_pin;//CS脚
}spinet_transfer_t; //主机SPI传输控制

typedef struct
{
	uint32_t data[4];
}ipv6_data_t;

typedef struct
{
	uint8_t rf_power_on;
	uint8_t sim_ready;
	uint8_t reg_state;
	uint8_t online_state;
	uint8_t rsrp;
	int8_t rsrq;
	int8_t snr;
	uint8_t ip_state;
	uint8_t dns_state[4];
	uint32_t ipv4;
	ipv6_data_t ipv6;
	ipv6_data_t dns[4];
}spinet_s_state_t;	//从机状态

enum
{
	SPINET_TYPE_M_NOP = 0x00,
	SPINET_TYPE_M_CMD,
	SPINET_TYPE_M_GET_IP_DATA,
	SPINET_TYPE_M_IP_DATA = 0xff,

	SPINET_TYPE_S_NOP = 0x00,
	SPINET_TYPE_S_RESPONSE,
	SPINET_TYPE_S_STATE,
	SPINET_TYPE_S_IP_DATA= 0xff
};

extern const uint8_t spi_sync_code[4];
/**
 * @brief 合成一个SPI数据包
 * @param head SPINET HEAD
 * @param data SPINET DATA
 * @param data_len SPINET DATA长度
 * @param out 输出缓存
 * @return 合成后包长度
 */
uint32_t pack_spinet_packet(spinet_head_t *head, uint8_t *data, uint32_t data_len, uint8_t *out);
/**
 * @brief 分解一个SPI数据包
 * @param input 输入数据
 * @param input_len输入数据长度
 * @param head SPINET HEAD
 * @param data SPINET DATA首地址
 * @param data_len SPINET DATA长度
 * @return =0成功，其他失败，包格式有误
 */
int unpack_spinet_packet(uint8_t *input, uint32_t input_len, spinet_head_t *head, uint8_t **data, uint32_t *data_len);
/**
 * @brief 主机进行一次SPI数据收发，供参考，也可以自行实现
 * @param trans 主机SPI传输控制
 * @param tx_buf 需要发送的数据包
 * @param tx_len 需要发送的数据包长度
 * @param timeout 发送超时，单位ms
 * @param head 接收到的SPINET HEAD
 * @param data 接收到的SPINET DATA首地址
 * @param data_len 接收到的SPINET DATA长度
 * @return
 */
int spinet_master_transfer(spinet_transfer_t *trans, uint8_t *tx_buf, uint16_t tx_len, uint32_t timeout, spinet_head_t *head, uint8_t **data, uint32_t *data_len);
/**
 * @brief 打印从机状态相关数据
 * @param state
 */
void print_spinet_slave_state(spinet_s_state_t *state);
#endif
