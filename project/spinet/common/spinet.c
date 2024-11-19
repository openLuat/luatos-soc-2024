#include "csdk.h"
#include "spinet.h"
#include "soc_spi.h"
const uint8_t spi_sync_code[4] = {0xf0, 0x5a, 0x0f, 0xa5};

uint32_t pack_spinet_packet(spinet_head_t *head, uint8_t *data, uint32_t data_len, uint8_t *out)
{
	uint16_t payload_len = data_len + sizeof(spinet_head_t);
	memcpy(out, spi_sync_code, 4);
	memcpy(out + 4, &payload_len, 2);
	memcpy(out + 6, head, sizeof(spinet_head_t));
	if (data && data_len)
	{
		memcpy(out + 6 + sizeof(spinet_head_t), data, data_len);
	}
	uint16_t crc16 = CRC16Cal(out + 6, payload_len, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0);
	memcpy(out + 6 + payload_len, &crc16, 2);
	return payload_len + 8;
}


int unpack_spinet_packet(uint8_t *input, uint32_t input_len, spinet_head_t *head, uint8_t **data, uint32_t *data_len)
{
	uint16_t payload_len,crc16,crc16_org;;
	uint32_t pos = 0;
	while (pos < (input_len - 4))
	{
		if (input[pos] == spi_sync_code[0])
		{
			if (!memcmp(&input[pos], spi_sync_code, 4))
			{
				goto FIND_SYNC;
			}
		}
		pos++;
	}
	return -ERROR_NO_DATA;
FIND_SYNC:
	memcpy(&payload_len, &input[pos + 4], 2);
	if (payload_len > PACKET_MAX_LEN)
	{
		return -ERROR_PARAM_OVERFLOW;
	}
	crc16 = CRC16Cal(&input[pos + 6], payload_len, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0);
	memcpy(&crc16_org, &input[pos + 6 + payload_len], 2);
	if (crc16 != crc16_org)
	{
		return -ERROR_PARAM_INVALID;
	}
	memcpy(head, &input[pos + 6], sizeof(spinet_head_t));
	*data = input + pos + 6 + sizeof(spinet_head_t);
	*data_len = payload_len - sizeof(spinet_head_t);
	return 0;
}


int spinet_master_transfer(spinet_transfer_t *trans, uint8_t *tx_buf, uint16_t tx_len, uint32_t timeout, spinet_head_t *head, uint8_t **data, uint32_t *data_len)
{
	uint64_t start_tamp;
	uint64_t to = timeout * SOC_TICK_1MS;
	uint8_t sync_tx_buf[8] = {0xff, 0xff, 0xff, 0xff,0xff, 0xff, 0xff, 0xff};
	uint8_t sync_rx_buf[8];
	uint32_t i;
	uint32_t dummy_byte = 0;
	int error_code = 0;
	Buffer_Struct cache;
	Buffer_StaticInit(&cache, trans->rx_buf, trans->buf_len);
	uint16_t rest_len, crc16, crc16_org, payload_len;
	uint8_t done, find_state;
	done = 0;
	find_state = 0;
	luat_gpio_set(trans->cs_pin, 0);
	start_tamp = luat_mcu_tick64();
	rest_len = 0;
	while(!done)
	{
		switch(find_state)
		{
		case 0:
			if (!cache.Pos)
			{
				dummy_byte += 8;
				SPI_BlockTransfer(trans->spi_id, sync_tx_buf, sync_rx_buf, 8);
				OS_BufferWrite(&cache, sync_rx_buf, 8);
			}
			for(i = 0; i < cache.Pos; i++)
			{
				if (cache.Data[i] == spi_sync_code[0])
				{
					OS_BufferRemove(&cache, i);
					find_state = 1;
					break;
				}
			}
			if (!find_state)
			{
				cache.Pos = 0;
				if (luat_mcu_tick64() - start_tamp > to)
				{
					done = 1;
					error_code = -ERROR_TIMEOUT;
				}
				else
				{
					if (dummy_byte > 24)
					{
						luat_rtos_task_sleep(1);
					}
				}
			}
			break;
		case 1:
			if (cache.Pos < 4)
			{
				dummy_byte += 8;
				SPI_BlockTransfer(trans->spi_id, sync_tx_buf, sync_rx_buf, 8);
				OS_BufferWrite(&cache, sync_rx_buf, 8);
			}
			if (!memcmp(cache.Data, spi_sync_code, 4))
			{
				find_state = 2;
			}
			else
			{
				OS_BufferRemove(&cache, 4);
				find_state = 0;
				if (luat_mcu_tick64() - start_tamp > to)
				{
					done = 1;
					error_code = -ERROR_TIMEOUT;
				}
			}
			break;
		case 2:
			if (cache.Pos < 6)
			{
				SPI_BlockTransfer(trans->spi_id, sync_tx_buf, sync_rx_buf, 2);
				OS_BufferWrite(&cache, sync_rx_buf, 2);
			}
			memcpy(&payload_len, &cache.Data[4], 2);
			if (payload_len > PAYLOAD_MAX_LEN)
			{
				LUAT_DEBUG_PRINT("data len too much %u", payload_len);
				find_state = 0;
				cache.Pos = 0;
				break;
			}
			rest_len = payload_len + 2;
			find_state = 3;
			break;
		default:
			if (rest_len < tx_len) rest_len = tx_len;
			memcpy(trans->tx_buf + cache.Pos, tx_buf, tx_len);
			SPI_BlockTransfer(trans->spi_id, trans->tx_buf + cache.Pos, trans->rx_buf + cache.Pos, rest_len);
			done = 1;
			memcpy(&crc16_org, cache.Data + 6 + payload_len, 2);
			crc16 = CRC16Cal(cache.Data + 6, payload_len, CRC16_CCITT_SEED, CRC16_CCITT_GEN, 0);
			if (crc16 == crc16_org)
			{
				memcpy(head, cache.Data + 6, sizeof(spinet_head_t));
				*data = cache.Data + 6 + sizeof(spinet_head_t);
				*data_len = payload_len - sizeof(spinet_head_t);
			}
			else
			{
				LUAT_DEBUG_PRINT("%x,%x", crc16, crc16_org);
				error_code = -ERROR_PARAM_INVALID;

			}
			break;
		}
	}
	luat_gpio_set(trans->cs_pin, 1);
	if (error_code) LUAT_DEBUG_PRINT("%d", error_code);
	start_tamp = luat_mcu_tick64();
	while((luat_mcu_tick64() - start_tamp) < 3900) {;}
	//luat_rtos_task_sleep(1);
	return error_code;
}

void print_spinet_slave_state(spinet_s_state_t *state)
{
	char buf[64];
	int16_t rsrp;
	uint16_t temp;
	luat_ip_addr_t ip;
	if (state->rf_power_on)
	{
		LUAT_DEBUG_PRINT("正常工作");
	}
	else
	{
		LUAT_DEBUG_PRINT("飞行模式");
		return;
	}
	if (state->sim_ready)
	{
		LUAT_DEBUG_PRINT("SIM卡正常");
	}
	else
	{
		LUAT_DEBUG_PRINT("SIM卡异常");
		return;
	}
	LUAT_DEBUG_PRINT("网络注册状态%d", state->reg_state);
	temp = state->rsrp;
	rsrp = -temp;
	LUAT_DEBUG_PRINT("rsrp %d rsrq %d snr %d", rsrp, state->rsrq, state->snr);
	if (state->online_state)
	{
		LUAT_DEBUG_PRINT("可以上网");
		if (state->ip_state <= 0x03)
		{
			if (state->ip_state & 0x01)
			{
				ip.type = IPADDR_TYPE_V4;
				ip.u_addr.ip4.addr = state->ipv4;
				ipaddr_ntoa_r(&ip, buf, sizeof(buf));
				LUAT_DEBUG_PRINT("IPV4 %s", buf);
			}
			if (state->ip_state & 0x02)
			{
				ip.type = IPADDR_TYPE_V6;
				memcpy(ip.u_addr.ip6.addr, state->ipv6.data, 16);
				ipaddr_ntoa_r(&ip, buf, sizeof(buf));
				LUAT_DEBUG_PRINT("IPV6 %s", buf);
			}
		}
		for (int i = 0; i < 4; i++)
		{
			if (state->dns_state[i] && state->dns_state[i] < 3)
			{
				if (state->ip_state & 0x01)
				{
					ip.type = IPADDR_TYPE_V4;
					ip.u_addr.ip4.addr = state->dns[i].data[0];
					ipaddr_ntoa_r(&ip, buf, sizeof(buf));
					LUAT_DEBUG_PRINT("dns server %d %s",i, buf);
				}
				else
				{
					ip.type = IPADDR_TYPE_V6;
					memcpy(ip.u_addr.ip6.addr, state->dns[i].data, 16);
					ipaddr_ntoa_r(&ip, buf, sizeof(buf));
					LUAT_DEBUG_PRINT("dns server %d %s",i, buf);
				}
			}
		}
	}
	else
	{
		LUAT_DEBUG_PRINT("不能上网");
	}
}
