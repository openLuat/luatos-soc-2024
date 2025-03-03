#include "csdk.h"
#include "ec7xx.h"
#include "clock.h"
#include "luat_onewire.h"
#include "driver_gpio.h"
#define BASE_CLK (26000000)
enum
{
	OW_OP_IDLE,
	OW_OP_RESET,
	OW_OP_WRITE,
	OW_OP_READ,
	OW_OP_READ_WITH_CMD,
};

typedef struct
{
	Buffer_Struct data_buf;
	uint8_t op;
	uint8_t op_done;
	uint8_t is_init;
}luat_onewire_ctrl_t;
static luat_onewire_ctrl_t prv_ow;
typedef union
{
	uint32_t v;
	struct
	{
		uint32_t send_time_div10:8;
		uint32_t wait_time_div10:8;
		uint32_t read_time_min:8;
		uint32_t read_time_max:8;
	};
}_RTCR;
typedef union
{
	uint32_t v;
	struct
	{
		uint32_t recovery_time:8;
		uint32_t slot_time_div10:4;
		uint32_t start_time:4;
		uint32_t write_start_time:8;
		uint32_t read_start_time:8;
	};
}_ATCR;
static __CORE_FUNC_IN_RAM__ void OW_ISR(void)
{
	volatile uint32_t SR = OW->IIR;
    // Disable ONEW IRQ
    XIC_DisableIRQ(PXIC0_ONEW_IRQn);
    soc_fast_printf("%x", SR);
    // Clear pending ONEW interrupts
    XIC_ClearPendingIRQ(PXIC0_ONEW_IRQn);
	switch(prv_ow.op)
	{
	case OW_OP_RESET:
		if (SR & (OW_OCR_CMD_RESET_Msk))
		{
			prv_ow.op_done = 1;
		}
		break;
	case OW_OP_WRITE:
		if (SR & OW_IIR_INT_WRITE_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				prv_ow.op_done = 1;
			}
			else
			{
				OW->TBR = prv_ow.data_buf.Data[prv_ow.data_buf.Pos];
				OW->OCR = OW_OCR_CMD_WRITE_Msk;
			}
		}
		break;
	case OW_OP_READ:
		if (SR & OW_IIR_INT_READ_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Data[prv_ow.data_buf.Pos] = OW->TBR;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				prv_ow.op_done = 1;
			}
			else
			{
				OW->OCR = OW_OCR_CMD_READ_Msk;
			}
		}
		break;
	case OW_OP_READ_WITH_CMD:
		if (SR & OW_IIR_INT_READ_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Data[prv_ow.data_buf.Pos] = OW->TBR;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				prv_ow.op_done = 1;
			}
			else
			{
				OW->OCR = OW_OCR_CMD_READ_Msk;
			}
		}
		if (SR & OW_IIR_INT_WRITE_Msk)
		{
			OW->IER = OW_IIR_INT_READ_Msk;
			OW->OCR = OW_OCR_CMD_READ_Msk;
		}
		break;
	}
	if (prv_ow.op_done)
	{
		OW->IER = 0;
		OW->IIR = OW_IIR_INT_CLR_Msk;
		OW->OCR = OW_OCR_CMD_FLUSH_Msk;
	}
	XIC_EnableIRQ(PXIC0_ONEW_IRQn);
}

void luat_onewire_init(void)
{
	if (prv_ow.is_init) return;
	GPR_clockEnable(PCLK_ONEW);
	if (luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_ONEWIRE, 0))
	{
		GPIO_IomuxEC7XX(17, 4, 0, 1);
	}
	prv_ow.is_init = 1;
	OW->IOR = 0;
	OW->ECR = OW_ECR_ENABLE_Msk|OW_ECR_CLK_EN_Msk|OW_ECR_AUTO_CGEN_Msk;
    XIC_SetVector(PXIC0_ONEW_IRQn, OW_ISR);
    XIC_EnableIRQ(PXIC0_ONEW_IRQn);
}

void luat_onewire_setup_timing(int id, onewire_timing_t *timing)
{
    _RTCR RTCR;
    _ATCR ATCR;
	if (timing->type)
	{
		if (timing->timing_tick.clock_div)
		{
			OW->CDR = timing->timing_tick.clock_div - 1;
		}
		else
		{
			OW->CDR = 25;
		}
		RTCR.send_time_div10 = timing->timing_tick.reset_keep_low_tick/10;
		RTCR.wait_time_div10 = timing->timing_tick.reset_wait_ack_tick/10;
		RTCR.read_time_min = timing->timing_tick.reset_read_ack_before_tick;
		RTCR.read_time_max = (timing->timing_tick.reset_read_ack_tick + timing->timing_tick.reset_read_ack_before_tick)/10;
		ATCR.recovery_time = timing->timing_tick.wr_recovery_tick;
		ATCR.slot_time_div10 = timing->timing_tick.wr_slot_tick/10;
		ATCR.start_time = timing->timing_tick.wr_start_tick;
		ATCR.write_start_time = timing->timing_tick.wr_write_start_tick;
		ATCR.read_start_time = timing->timing_tick.wr_read_start_tick;
	}
	else
	{
		OW->CDR = 25;
		RTCR.send_time_div10 = timing->timing_us.reset_keep_low_time/10;
		RTCR.wait_time_div10 = timing->timing_us.reset_wait_ack_time/10;
		RTCR.read_time_min = timing->timing_us.reset_read_ack_before_time;
		RTCR.read_time_max = (timing->timing_us.reset_read_ack_time + timing->timing_us.reset_read_ack_before_time)/10;
		ATCR.recovery_time = timing->timing_us.wr_recovery_time;
		ATCR.slot_time_div10 = timing->timing_us.wr_slot_time/10;
		ATCR.start_time = timing->timing_us.wr_start_time;
		ATCR.write_start_time = timing->timing_us.wr_write_start_time;
		ATCR.read_start_time = timing->timing_us.wr_read_start_time;
	}
	OW->RTCR = RTCR.v;
	OW->ATCR = ATCR.v;
	DBG("%d,%x,%x,%x", OW->CDR, OW->RTCR, OW->ATCR, OW->DFR);

}

void luat_onewire_reset(int id)
{
	XIC_EnableIRQ(PXIC0_ONEW_IRQn);
	uint16_t timeout = 1000;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_RESET;
	OW->IER = OW_IER_INTEN_RESET_Msk;
	OW->OCR = OW_OCR_CMD_RESET_Msk;

	while(!prv_ow.op_done) {
		luat_rtos_task_sleep(1);
		timeout--;
		if (!timeout)
		{
			DBG("!");
			OW->OCR = OW_OCR_CMD_FLUSH_Msk;
			break;
		}
	}
}

void luat_onewire_write_bit(int id, uint8_t level)
{
    XIC_DisableIRQ(PXIC0_ONEW_IRQn);
	uint16_t timeout = 20000;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_WRITE;
    OW->DFR = 0;
    OW->TBR = level;
    OW->IER = 0;
    OW->OCR = OW_OCR_CMD_WRITE_Msk;
	while(!(OW->IIR & OW_IIR_INT_WRITE_Msk)) {
		timeout--;
		if (!timeout) return;
	}
	OW->OCR = OW_OCR_CMD_FLUSH_Msk;
}
uint8_t luat_onewire_read_bit(int id)
{
	XIC_DisableIRQ(PXIC0_ONEW_IRQn);
	uint16_t timeout = 20000;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_READ;
    OW->DFR = 0;
    OW->IER = 0;
    OW->OCR = OW_OCR_CMD_READ_Msk;
	while(!(OW->IIR & OW_IIR_INT_READ_Msk)) {
		timeout--;
		if (!timeout)
		{
			break;
		}
	}
	uint8_t res = OW->TBR;
	OW->OCR = OW_OCR_CMD_FLUSH_Msk;
	return res;
}
void luat_onewire_write_byte(int id, const uint8_t *data, uint32_t len, uint8_t is_msb)
{
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	XIC_EnableIRQ(PXIC0_ONEW_IRQn);
	uint32_t timeout = len + 1;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_WRITE;
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->TBR = prv_ow.data_buf.Data[0];
	OW->IER = OW_IIR_INT_WRITE_Msk;
	OW->OCR = OW_OCR_CMD_WRITE_Msk;
	while(!prv_ow.op_done) {
		luat_rtos_task_sleep(1);
		timeout--;
		if (!timeout)
		{
			DBG("!");
			OW->OCR = OW_OCR_CMD_FLUSH_Msk;
			break;
		}
	}
}

void luat_onewire_read_byte_with_cmd(int id, const uint8_t cmd, uint8_t *data, uint32_t len, uint8_t is_msb)
{
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	uint32_t timeout = len + 1;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_READ_WITH_CMD;
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->TBR = cmd;
	OW->IER = OW_IIR_INT_WRITE_Msk;
	OW->OCR = OW_OCR_CMD_WRITE_Msk;
	while(!prv_ow.op_done) {
		luat_rtos_task_sleep(1);
		timeout--;
		if (!timeout)
		{
			DBG("!");
			OW->OCR = OW_OCR_CMD_FLUSH_Msk;
			break;
		}
	}
}

void luat_onewire_read_byte(int id, uint8_t *data, uint32_t len, uint8_t is_msb)
{
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	uint32_t timeout = len + 10;
	prv_ow.op_done = 0;
	prv_ow.op = OW_OP_READ;
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->IER = OW_IIR_INT_READ_Msk;
	OW->OCR = OW_OCR_CMD_READ_Msk;
	while(!prv_ow.op_done) {
		luat_rtos_task_sleep(1);
		timeout--;
		if (!timeout)
		{
			DBG("!");
			OW->OCR = OW_OCR_CMD_FLUSH_Msk;
			break;
		}
	}
}
