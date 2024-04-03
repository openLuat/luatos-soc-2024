#include "common_api.h"

#include "luat_rtos.h"
#include "luat_audio_play.h"
#include "luat_i2s.h"
#include "luat_gpio.h"
#include "luat_debug.h"
#include "luat_i2c.h"
#include "interf_enc.h"
#include "luat_uart.h"
#include "luat_mem.h"

#define TEST_I2S_ID I2S_ID0
static luat_rtos_task_handle g_s_task_handle;

enum
{
	I2S_RX_DONE = 1,
	I2S_TX_DONE,
};

__USER_FUNC_IN_RAM__ int record_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param)
{
	switch(event)
	{
	case LUAT_I2S_EVENT_RX_DONE:
		luat_rtos_event_send(g_s_task_handle, I2S_RX_DONE, (uint32_t)rx_data, rx_len, 0, 0);
		break;
	case LUAT_I2S_EVENT_TRANSFER_DONE:
		luat_rtos_event_send(g_s_task_handle, I2S_TX_DONE, 0, 0, 0, 0);
		break;
	default:
		break;
	}
	return 0;
}

static uint32_t test_data[2000];
static void es8311_demo_task(void *arg)
{
	PV_Union upv;
	luat_i2s_conf_t i2s_conf = {
		.id = TEST_I2S_ID,
		.mode = LUAT_I2S_MODE_MASTER,
		.channel_format = LUAT_I2S_CHANNEL_RIGHT,
		.standard = LUAT_I2S_MODE_LSB,
		.channel_bits = LUAT_I2S_BITS_32,
		.data_bits = LUAT_I2S_BITS_24,
		.is_full_duplex = 1,
		.sample_rate = 0,
		.cb_rx_len = 8000,
		.luat_i2s_event_callback = record_cb,
	};
	for(int i = 0; i < 2000; i++)
	{
		test_data[i] = 0x12345678;
	}
	luat_i2s_setup(&i2s_conf);
	luat_i2s_modify(TEST_I2S_ID, LUAT_I2S_CHANNEL_RIGHT, 24, 8000);
	luat_i2s_transfer_loop(TEST_I2S_ID, (uint8_t *)test_data, 4000, 2, 1);
	luat_event_t event;
	while (1)
	{
		luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case I2S_RX_DONE:
			upv.u32 = event.param1;
			DBG("%x,%x,%x,%x", upv.pu32[0], upv.pu32[1], upv.pu32[2], upv.pu32[3]);
			break;
		case I2S_TX_DONE:
			break;

		}
	}
}

static void test_record_demo_init(void)
{
	luat_rtos_task_create(&g_s_task_handle, 4096, 20, "es8311", es8311_demo_task, NULL, 16);
}

INIT_TASK_EXPORT(test_record_demo_init, "1");
