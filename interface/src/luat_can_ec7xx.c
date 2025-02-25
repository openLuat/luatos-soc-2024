#include "common_api.h"
#include "luat_can.h"
#include "luat_mcu.h"
#include "soc_can.h"
#include "driver_gpio.h"
#ifdef TYPE_EC718M
typedef struct
{
	luat_can_callback_t callback;
	PV_Union last_error;
	uint8_t last_tx_stop;
}luat_can_ctrl_t;

static luat_can_ctrl_t prv_can;

static int luat_can_cb(void *data, void *param)
{
	CAN_ErrorMsg *Msg = param;
	switch ((uint32_t)data)
	{
	case CAN_NEW_MSG:
		prv_can.callback(0, LUAT_CAN_CB_NEW_MSG, NULL);
		break;
	case CAN_TX_OK:
		prv_can.callback(0, prv_can.last_tx_stop?LUAT_CAN_CB_TX_FAILED:LUAT_CAN_CB_TX_OK, NULL);
		prv_can.last_tx_stop = 0;
		break;
	case CAN_TX_FAILED:
		prv_can.callback(0, LUAT_CAN_CB_TX_FAILED, NULL);
		break;
	case CAN_ERROR_REPORT:
		if (CAN_GetState() >= CAN_STATE_NODE_PASSIVE_ERROR)
		{
			prv_can.last_tx_stop = 1;
			CAN_TxStop();
		}
		prv_can.last_error.u8[2] = Msg->ErrorDir;
		prv_can.last_error.u8[1] = Msg->ErrorType;
		prv_can.last_error.u8[0] = Msg->ErrorLOC;
		prv_can.callback(0, LUAT_CAN_CB_ERROR_REPORT, prv_can.last_error.p);
		break;
	case CAN_STATE_CHANGE:
		prv_can.callback(0, LUAT_CAN_CB_STATE_CHANGE, (void *)luat_can_get_state(0));
		break;
	}
	return 0;
}

void luat_can_dummy_callback(int can_id, LUAT_CAN_CB_E cb_type, void *cb_param)
{

}

int luat_can_base_init(uint8_t can_id, uint32_t rx_msg_cache_max)
{

	if (!rx_msg_cache_max) rx_msg_cache_max = 128;
	if(luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_CAN, 0))
	{
		GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(25, 0), 7, 1, 1);
		GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(26, 0), 7, 1, 1);
		GPIO_IomuxEC7XX(GPIO_ToPadEC7XX(28, 0), 6, 1, 1);
	}
	if (!prv_can.callback) prv_can.callback = luat_can_dummy_callback;
	return CAN_BaseInit(rx_msg_cache_max, luat_can_cb);
}

int luat_can_set_callback(uint8_t can_id, luat_can_callback_t callback)
{
	prv_can.callback = callback?callback:luat_can_dummy_callback;
	return 0;
}

int luat_can_set_work_mode(uint8_t can_id, LUAT_CAN_WORK_MODE_E mode)
{
	int ret = CAN_EnterResetMode(1000);
	if (ret) return ret;
	return CAN_SetWorkMode(mode);
}


int luat_can_set_timing(uint8_t can_id, uint32_t bit_rate, uint8_t PTS, uint8_t PBS1, uint8_t PBS2, uint8_t SJW)
{
	return CAN_SetBitRate(bit_rate, PTS, PBS1, PBS2, SJW);
}


int luat_can_set_node(uint8_t can_id, uint32_t node_id, uint8_t is_extend_id)
{
	uint8_t ACR[4];
	uint8_t AMR[4];
	uint32_t mask;
    if(is_extend_id)
    {
        // Extended frame
    	node_id = (node_id << 3);
        mask = 0x07;
    }
    else
    {
        // Standard frame
    	node_id = (node_id << 21);
        mask = 0x0001fffff;
    }
    BytesPutBe32(ACR, node_id);
    BytesPutBe32(AMR, mask);
	return CAN_SetFilter(0, ACR, AMR);
}


int luat_can_set_filter(uint8_t can_id, uint8_t is_dual_mode, uint8_t ACR[4], uint8_t AMR[4])
{
	return CAN_SetFilter(is_dual_mode, ACR, AMR);
}


int luat_can_tx_message(uint8_t can_id, uint32_t message_id, uint8_t is_extend_id, uint8_t is_RTR, uint8_t need_ack, uint8_t data_len, uint8_t *data)
{
	CAN_MsgStruct Msg = {
			.ID = message_id,
			.ExtendID = is_extend_id,
			.RTR = is_RTR,
			.OneShot = !need_ack,
			.Len = data_len
	};
	if (data_len > 8) return -ERROR_PARAM_OVERFLOW;
	if (data_len)
	{
		memcpy(Msg.Data, data, data_len);
	}
	return CAN_TxMsg(&Msg);
}


int luat_can_tx_stop(uint8_t can_id)
{
	prv_can.last_tx_stop = 1;
	CAN_TxStop();
	return 0;
}


int luat_can_rx_message_from_cache(uint8_t can_id, luat_can_message_t *message)
{
	CAN_MsgStruct RxMsg;
	int ret = CAN_RxMsg(&RxMsg);
	if (ret > 0)
	{
		message->id = RxMsg.ID;
		message->RTR = RxMsg.RTR;
		message->is_extend = RxMsg.ExtendID;
		message->len = RxMsg.Len;
		if (message->len)
		{
			memcpy(message->data, RxMsg.Data, message->len);
		}
	}
	return ret;
}


int luat_can_reset(uint8_t can_id)
{
	int ret = CAN_EnterResetMode(1000);
	if (ret) return ret;
	CAN_SetWorkMode(CAN_WORK_MODE_NORMAL);
	return 0;
}


int luat_can_close(uint8_t can_id)
{
	CAN_DeInit();
	return 0;
}

int luat_can_get_state(uint8_t can_id)
{
	uint8_t state = CAN_GetState();
	if (state < CAN_STATE_NODE_ACTIVE_ERROR || state > CAN_STATE_SLEEP)
	{
		return LUAT_CAN_STOP;
	}
	else
	{
		return (state - CAN_STATE_NODE_ACTIVE_ERROR) + LUAT_CAN_ACTIVE_ERROR;
	}
}

uint32_t luat_can_get_last_error(uint8_t can_id)
{
	return prv_can.last_error.u32;
}

void luat_can_set_debug(uint8_t on_off)
{
	CAN_SetDebug(on_off);
}
#endif
