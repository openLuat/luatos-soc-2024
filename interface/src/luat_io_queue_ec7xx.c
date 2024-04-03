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

#include "luat_base.h"
#include "common_api.h"
#include "luat_lib_io_queue.h"

#ifdef __LUATOS__
#include "luat_msgbus.h"
static int32_t luat_io_queue_done_cb(void *pData, void *pParam)
{
	rtos_msg_t msg;
	msg.handler = l_io_queue_done_handler;
	msg.ptr = pParam;
	luat_msgbus_put(&msg, 1);
	return 0;
}

static int32_t __USER_FUNC_IN_RAM__ luat_io_queue_dummy_cb(void *pData, void *pParam)
{
	return 0;
}

static int32_t __USER_FUNC_IN_RAM__ luat_io_queue_capture_cb(void *pData, void *pParam)
{
	rtos_msg_t msg;
	uint64_t tick = soc_get_poweron_time_tick();
	msg.handler = l_io_queue_capture_handler;
	msg.ptr = ((uint32_t)pData << 8) | GPIO_Input(pData);
	msg.arg1 = (tick >> 32) & 0xffffffff;
	msg.arg2 = tick & 0xffffffff;
	luat_msgbus_put(&msg, 1);
	return 0;
}
#endif

#ifdef __LUATOS__
void luat_io_queue_init(uint8_t hw_timer_id, uint32_t cmd_cnt, uint32_t repeat_cnt)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_InitOperationQueue(hw_timer_id, cmd_cnt, repeat_cnt, luat_io_queue_done_cb, hw_timer_id);
}

#else
void luat_io_queue_init(uint8_t hw_timer_id, uint32_t cmd_cnt, uint32_t repeat_cnt, CBDataFun_t *cb)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_InitOperationQueue(hw_timer_id, cmd_cnt, repeat_cnt, cb, hw_timer_id);
}

#endif

void luat_io_queue_start(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_StartOperationQueue(hw_timer_id);
}

void luat_io_queue_stop(uint8_t hw_timer_id, uint32_t *repeat_cnt, uint32_t *cmd_cnt)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_Stop(hw_timer_id);
	HWTimer_GetResultOperationInfo(hw_timer_id, repeat_cnt, cmd_cnt);
}


void luat_io_queue_clear(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_ClearOperationQueue(hw_timer_id);
}

void luat_io_queue_release(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	HWTimer_FreeOperationQueue(hw_timer_id);
}

void luat_io_queue_set_delay(uint8_t hw_timer_id, uint16_t time, uint8_t sub_tick, uint8_t is_continue)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = is_continue?OP_QUEUE_CMD_CONTINUE_DELAY:OP_QUEUE_CMD_ONE_TIME_DELAY;
	cmd.PinOrDelay = sub_tick;
	cmd.uArg.Time = time;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_repeat_delay(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = OP_QUEUE_CMD_REPEAT_DELAY;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_add_io_config(uint8_t hw_timer_id, uint8_t pin, uint8_t is_input, uint8_t pull_mode, uint8_t level)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = is_input?OP_QUEUE_CMD_SET_GPIO_DIR_IN:OP_QUEUE_CMD_SET_GPIO_DIR_OUT;
	cmd.PinOrDelay = pin;
	cmd.uArg.IOArg.Level = level;
	cmd.uArg.IOArg.PullMode = pull_mode;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}


void luat_io_queue_add_io_out(uint8_t hw_timer_id, uint8_t pin, uint8_t level)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = OP_QUEUE_CMD_GPIO_OUT;
	cmd.PinOrDelay = pin;
	cmd.uArg.IOArg.Level = level;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_add_io_in(uint8_t hw_timer_id, uint8_t pin, CBFuncEx_t CB, void *user_data)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = CB;
	cmd.uParam.pParam = user_data;
	cmd.Operation = CB?OP_QUEUE_CMD_GPIO_IN_CB:OP_QUEUE_CMD_GPIO_IN;
	cmd.PinOrDelay = pin;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_capture_set(uint8_t hw_timer_id, uint32_t max_tick, uint8_t pin, uint8_t pull_mode, uint8_t irq_mode)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = OP_QUEUE_CMD_CAPTURE_SET;
	cmd.PinOrDelay = pin;
	cmd.uParam.MaxCnt = max_tick;
	cmd.uArg.ExitArg.ExtiMode = irq_mode;
	cmd.uArg.ExitArg.PullMode = pull_mode;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_capture(uint8_t hw_timer_id, CBFuncEx_t CB, void *user_data)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = CB;
	cmd.PinOrDelay = 0xff;
	cmd.uParam.pParam = user_data;
	cmd.Operation = CB?OP_QUEUE_CMD_CAPTURE_CB:OP_QUEUE_CMD_CAPTURE;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_capture_end(uint8_t hw_timer_id, uint8_t pin)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.CB = NULL;
	cmd.Operation = OP_QUEUE_CMD_CAPTURE_END;
	cmd.PinOrDelay = pin;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

void luat_io_queue_end(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return;
	OPQueue_CmdStruct cmd;
	cmd.Operation = OP_QUEUE_CMD_END;
	HWTimer_AddOperation(hw_timer_id, &cmd);
}

uint8_t luat_io_queue_check_done(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return 0;
	return HWTimer_CheckOperationQueueDone(hw_timer_id);

}

int luat_io_queue_get_size(uint8_t hw_timer_id)
{
	if (hw_timer_id >= (HW_TIMER_MAX - 1)) return 0;
	return HWTimer_GetOperationQueueLen(hw_timer_id);

}

void luat_io_queue_get_data(uint8_t hw_timer_id, uint8_t *input_buff, uint32_t *input_cnt, uint8_t *capture_buff, uint32_t *capture_cnt)
{
	if ((hw_timer_id >= (HW_TIMER_MAX - 1)) || !HWTimer_GetOperationQueue(hw_timer_id))
	{
		*input_cnt = 0;
		*capture_cnt = 0;
	}
	else
	{
		OPQueue_CmdStruct *Cmd = HWTimer_GetOperationQueue(hw_timer_id);
		uint32_t len = HWTimer_GetOperationQueueLen(hw_timer_id);
		uint32_t input_pos = 0;
		uint32_t capture_pos = 0;
		uint32_t i;
		for(i = 0; i < len; i++)
		{
			switch(Cmd[i].Operation)
			{
			case OP_QUEUE_CMD_GPIO_IN:
			case OP_QUEUE_CMD_GPIO_IN_CB:
				input_buff[input_pos * 2] = Cmd[i].PinOrDelay;
				input_buff[input_pos * 2 + 1] = Cmd[i].uArg.IOArg.Level;
				input_pos++;
				break;
			case OP_QUEUE_CMD_CAPTURE:
			case OP_QUEUE_CMD_CAPTURE_CB:
				if (Cmd[i].PinOrDelay >= HAL_GPIO_NONE)
				{
					len = 0;
					break;
				}

				capture_buff[capture_pos * 6] = Cmd[i].PinOrDelay;
				capture_buff[capture_pos * 6 + 1] = Cmd[i].uArg.IOArg.Level;
				BytesPutLe32(&capture_buff[capture_pos * 6 + 2], Cmd[i].uParam.MaxCnt);
				capture_pos++;
				break;
			}
		}
		*input_cnt = input_pos;
		*capture_cnt = capture_pos;
	}
	return ;

}
#ifdef __LUATOS__
extern uint8_t luat_gpio_get_alt(uint8_t GPIO);
extern uint32_t prvGPIO_ToPadEC7XXFast(uint32_t Pin);
void luat_io_queue_capture_start_with_sys_tick(uint8_t pin, uint8_t pull_mode, uint8_t irq_mode)
{
	GPIO_GlobalInit(NULL);
	GPIO_PullConfig(prvGPIO_ToPadEC7XXFast(pin), pull_mode, (pull_mode > 1)?0:1);
	if (pin > HAL_GPIO_16 && pin <= HAL_GPIO_19)
	{
		GPIO_IomuxEC7XX(prvGPIO_ToPadEC7XXFast(pin), luat_gpio_get_alt(pin), 0, 7);
	}
	else
	{
		GPIO_IomuxEC7XX(prvGPIO_ToPadEC7XXFast(pin), 0, 0, 7);
	}
	GPIO_Config(pin, 1, 0);
	GPIO_ExtiSetCB(pin, luat_io_queue_capture_cb, NULL);
	switch(irq_mode)
	{
	case OP_QUEUE_CMD_IO_EXTI_BOTH:
		GPIO_ExtiConfig(pin, 0, 1, 1);
		break;
	case OP_QUEUE_CMD_IO_EXTI_UP:
		GPIO_ExtiConfig(pin, 0, 1, 0);
		break;
	case OP_QUEUE_CMD_IO_EXTI_DOWN:
		GPIO_ExtiConfig(pin, 0, 0, 1);
		break;
	}
}
#else
void luat_io_queue_capture_start_with_sys_tick(uint8_t pin, uint8_t pull_mode, uint8_t irq_mode, CBDataFun_t *cb)
{
	GPIO_PullConfig(pin, pull_mode, (pull_mode > 1)?0:1);
	GPIO_Config(pin, 1, 0);
	GPIO_ExtiSetCB(pin, cb, NULL);
	switch(irq_mode)
	{
	case OP_QUEUE_CMD_IO_EXTI_BOTH:
		GPIO_ExtiConfig(pin, 0, 1, 1);
		break;
	case OP_QUEUE_CMD_IO_EXTI_UP:
		GPIO_ExtiConfig(pin, 0, 1, 0);
		break;
	case OP_QUEUE_CMD_IO_EXTI_DOWN:
		GPIO_ExtiConfig(pin, 0, 0, 1);
		break;
	}
}
#endif

void luat_io_queue_capture_end_with_sys_tick(uint8_t pin)
{
	GPIO_ExtiSetCB(pin, NULL, NULL);
	GPIO_ExtiConfig(pin, 0, 0, 0);
}

