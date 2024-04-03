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
#include "common_api.h"
#include "luat_i2s.h"
#include "driver_usp.h"
#include "driver_gpio.h"

static luat_i2s_conf_t prv_i2s[I2S_MAX];

#ifdef __LUATOS__
#include "luat_msgbus.h"

int l_i2s_play(lua_State *L) {
    return -1;
}

int l_i2s_pause(lua_State *L) {
    return 0;
}

int l_i2s_stop(lua_State *L) {
	uint8_t id = luaL_checkinteger(L, 1);
	I2S_RxStop(id);
}
#endif

static __USER_FUNC_IN_RAM__ int32_t prv_i2s_cb(void *pdata, void *param)
{
	luat_i2s_conf_t *conf = (luat_i2s_conf_t *)param;
	if (pdata)
	{
		Buffer_Struct *buf = (Buffer_Struct *)pdata;
		conf->luat_i2s_event_callback(conf->id, LUAT_I2S_EVENT_RX_DONE, buf->Data, buf->Pos, conf->userdata);
	}
	else
	{
		conf->luat_i2s_event_callback(conf->id, conf->is_full_duplex?LUAT_I2S_EVENT_TRANSFER_DONE:LUAT_I2S_EVENT_TX_DONE, NULL, 0, conf->userdata);
	}
	return 0;
}

int luat_i2s_setup(const luat_i2s_conf_t *conf)
{
	if (conf->id >= I2S_MAX) return -1;

	uint8_t frame_size = I2S_FRAME_SIZE_16_16;
	if (conf->channel_bits != 16)
	{
		switch (conf->data_bits)
		{
		case 24:
			frame_size = I2S_FRAME_SIZE_24_32;
			break;
		case 32:
			frame_size = I2S_FRAME_SIZE_32_32;
			break;
		default:
			frame_size = I2S_FRAME_SIZE_16_32;
			break;
		}
	}
	I2S_BaseConfig(conf->id, conf->standard, frame_size);
	int pad;
	switch(conf->id)
	{
#if defined TYPE_EC716S
	case I2S_ID0:
		GPIO_IomuxEC7XX(11, 6, 1, 0);
		GPIO_IomuxEC7XX(25, 6, 1, 0);
		GPIO_IomuxEC7XX(26, 6, 1, 0);
		GPIO_IomuxEC7XX(28, 6, 1, 0);
		break;
#else
	case I2S_ID0:
		for(pad = 35; pad <= 39; pad++)
		{
			GPIO_IomuxEC7XX(pad, 1, 1, 0);
		}
		break;
	case I2S_ID1:
		for(pad = 18; pad <= 22; pad++)
		{
			GPIO_IomuxEC7XX(pad, 1, 1, 0);
		}
		break;
#endif
	default:
		return -1;
	}

	if (conf->channel_format < LUAT_I2S_CHANNEL_STEREO)
	{
		I2sDataFmt_t DataFmt;
		I2sSlotCtrl_t  SlotCtrl;
		I2sBclkFsCtrl_t BclkFsCtrl;
		I2sDmaCtrl_t DmaCtrl;
		I2S_GetConfig(conf->id, &DataFmt, &SlotCtrl, &BclkFsCtrl, &DmaCtrl);
		BclkFsCtrl.fsPolarity = conf->channel_format;
		I2S_FullConfig(conf->id, DataFmt, SlotCtrl,  BclkFsCtrl,  DmaCtrl);
	}

	prv_i2s[conf->id] = *conf;
	if (!prv_i2s[conf->id].cb_rx_len) prv_i2s[conf->id].cb_rx_len = 4000;
	return 0;
}

int luat_i2s_modify(uint8_t id,uint8_t channel_format,uint8_t data_bits,uint32_t sample_rate)
{
	if (id >= I2S_MAX) return -1;
	if (!sample_rate)
	{
		if (I2S_IsWorking(id))
		{
			I2S_Stop(id);
		}
		prv_i2s[id].state = LUAT_I2S_STATE_STOP;
		DBG("i2s%d stop", id);
		return 0;
	}
	if (data_bits != prv_i2s[id].data_bits)
	{
		prv_i2s[id].data_bits = data_bits;
		uint8_t frame_size = I2S_FRAME_SIZE_16_16;
		if (prv_i2s[id].channel_bits != 16)
		{
			switch (data_bits)
			{
			case 24:
				frame_size = I2S_FRAME_SIZE_24_32;
				break;
			case 32:
				frame_size = I2S_FRAME_SIZE_32_32;
				break;
			default:
				frame_size = I2S_FRAME_SIZE_16_32;
				break;
			}
		}
		I2S_BaseConfig(id, prv_i2s[id].standard, frame_size);
	}
	prv_i2s[id].sample_rate = sample_rate;
	prv_i2s[id].channel_format = channel_format;
	//DBG("%d,%d,%d,%d,%d", id, prv_i2s[id].mode, sample_rate, channel_format, prv_i2s[id].is_full_duplex);
	if (LUAT_I2S_MODE_SLAVE == prv_i2s[id].mode)
	{
		I2S_Start(id, 0, sample_rate, (channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1);
	}
	else if (prv_i2s[id].is_full_duplex)
	{
		I2S_StartTransfer(id, sample_rate, (channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1, prv_i2s[id].cb_rx_len, prv_i2s_cb, (void *)&prv_i2s[id]);
	}
	else
	{
		I2S_Start(id, 1, sample_rate, (channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1);
	}
	prv_i2s[id].state = LUAT_I2S_STATE_RUNING;

}
static __USER_FUNC_IN_RAM__ void luat_i2s_check_start(id)
{
	if (!prv_i2s[id].state)
	{
		DBG("i2s%d need start samplerate %u,channel_format %d, mode %d full duplex %d", id, prv_i2s[id].sample_rate, prv_i2s[id].channel_format, prv_i2s[id].mode, prv_i2s[id].is_full_duplex);
		if (LUAT_I2S_MODE_SLAVE == prv_i2s[id].mode)
		{
			I2S_Start(id, 0, prv_i2s[id].sample_rate, (prv_i2s[id].channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1);
		}
		else if (prv_i2s[id].is_full_duplex)
		{
			I2S_StartTransfer(id, prv_i2s[id].sample_rate, (prv_i2s[id].channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1, prv_i2s[id].cb_rx_len, prv_i2s_cb, (void *)&prv_i2s[id]);
		}
		else
		{
			I2S_Start(id, 1, prv_i2s[id].sample_rate, (prv_i2s[id].channel_format == LUAT_I2S_CHANNEL_STEREO)?2:1);
		}
		prv_i2s[id].state = LUAT_I2S_STATE_RUNING;
	}
}
// 传输(异步接口)
__USER_FUNC_IN_RAM__ int luat_i2s_send(uint8_t id, uint8_t* buff, size_t len)
{
	if (id >= I2S_MAX) return -1;
	luat_i2s_check_start();
	I2S_Tx(id, buff, len, prv_i2s_cb, (void *)&prv_i2s[id]);
	return 0;
}

int luat_i2s_recv(uint8_t id, uint8_t* buff, size_t len)
{
	if (id >= I2S_MAX) return -1;
	if (prv_i2s[id].is_full_duplex) return -1;
	if (len) prv_i2s[id].cb_rx_len = len;
	luat_i2s_check_start();
	I2S_Rx(id, prv_i2s[id].cb_rx_len, prv_i2s_cb, (void *)&prv_i2s[id]);
}

__USER_FUNC_IN_RAM__ int luat_i2s_transfer(uint8_t id, uint8_t* txbuff, size_t len)
{
	if (id >= I2S_MAX) return -1;
	luat_i2s_check_start();
	I2S_Transfer(id, txbuff, len);
	return 0;
}

int luat_i2s_transfer_loop(uint8_t id, uint8_t* buff, uint32_t one_truck_byte_len, uint32_t total_trunk_cnt, uint8_t need_callback)
{
	if (id >= I2S_MAX) return -1;
	luat_i2s_check_start();
	I2S_TransferLoop(id, buff, one_truck_byte_len, total_trunk_cnt, need_callback);
}
// 控制
int luat_i2s_pause(uint8_t id)
{
	if (id >= I2S_MAX) return -1;
	if (prv_i2s[id].is_full_duplex) return -1;
	I2S_TxPause(id);
}
int luat_i2s_resume(uint8_t id)
{
	return -1;
}
int luat_i2s_close(uint8_t id)
{
	if (I2S_IsWorking(id))
	{
		I2S_Stop(id);
	}
	prv_i2s[id].state = LUAT_I2S_STATE_STOP;
}

// 获取配置
luat_i2s_conf_t *luat_i2s_get_config(uint8_t id) {return &prv_i2s[id];}

int luat_i2s_txbuff_info(uint8_t id, size_t *buffsize, size_t* remain)
{
	return -1;
}
int luat_i2s_rxbuff_info(uint8_t id, size_t *buffsize, size_t* remain)
{
	return -1;
}
