#include "csdk.h"
#include "ec7xx.h"
#include "clock.h"
#include "luat_onewire.h"
#include "driver_gpio.h"
#include "soc_service.h"
#define BASE_CLK (26000000)
enum
{
	OW_OP_IDLE,
	OW_OP_RESET,
	OW_OP_WRITE,
	OW_OP_READ,
	OW_OP_READ_WITH_CMD,
};

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

typedef struct
{
	HANDLE finish_sem;
	Buffer_Struct data_buf;
    _RTCR RTCR;
    _ATCR ATCR;
    uint8_t CDR;
	uint8_t op;
	uint8_t is_init;
	uint8_t wait_finish;
	uint8_t cmd;
	uint8_t check_ack;
	uint8_t ack_result;
	uint8_t is_debug;
}luat_onewire_ctrl_t;
static luat_onewire_ctrl_t prv_ow;


static __CORE_FUNC_IN_RAM__ void OW_ISR(void)
{
	volatile uint32_t SR = OW->IIR;
	uint8_t op_done = 0;
    // Disable ONEW IRQ
    XIC_DisableIRQ(PXIC0_ONEW_IRQn);
    if (prv_ow.is_debug) {soc_fast_printf("ow sr %x", SR);}
    // Clear pending ONEW interrupts
    XIC_ClearPendingIRQ(PXIC0_ONEW_IRQn);
    if (SR & OW_IIR_INT_RESET_PD_Msk)
	{
    	if (SR & OW_IIR_INT_RESET_Msk)
    	{

    	}
    	else
    	{
    		OW->IIR = OW_IIR_INT_CLR_Msk;
    		XIC_EnableIRQ(PXIC0_ONEW_IRQn);
    		return;
    	}

	}
    if (SR & OW_IIR_INT_RESET_Msk)
	{
    	prv_ow.ack_result = (SR & OW_IIR_RESET_PD_RES_Msk)?1:0;
    	if (prv_ow.check_ack)
    	{
    		if (!prv_ow.ack_result)
    		{
    			if (prv_ow.op > OW_OP_RESET)
    			{
    				if (prv_ow.is_debug) {soc_fast_printf("ow no ack");}
    				op_done = 1;
    				goto DONE;
    			}
    		}
    	}
	}
	switch(prv_ow.op)
	{
	case OW_OP_RESET:
		if (SR & OW_IIR_INT_RESET_Msk)
		{
			op_done = 1;
		}
		break;
	case OW_OP_WRITE:
		if (SR & OW_IIR_INT_WRITE_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				op_done = 1;
			}
			else
			{
				OW->TBR = prv_ow.data_buf.Data[prv_ow.data_buf.Pos];
				OW->OCR = OW_OCR_CMD_WRITE_Msk;
			}
		}
		else if (SR & OW_IIR_INT_RESET_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			OW->TBR = prv_ow.data_buf.Data[0];
			OW->OCR = OW_OCR_CMD_WRITE_Msk;
		}
		break;
	case OW_OP_READ:
		if (SR & OW_IIR_INT_READ_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Data[prv_ow.data_buf.Pos] = OW->RBR;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				op_done = 1;
			}
			else
			{
				OW->OCR = OW_OCR_CMD_READ_Msk;
			}
		}
		else if (SR & OW_IIR_INT_RESET_Msk)
		{
			OW->OCR = OW_OCR_CMD_READ_Msk;
		}
		break;
	case OW_OP_READ_WITH_CMD:
		if (SR & OW_IIR_INT_READ_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			prv_ow.data_buf.Data[prv_ow.data_buf.Pos] = OW->RBR;
			prv_ow.data_buf.Pos++;
			if (prv_ow.data_buf.Pos >= prv_ow.data_buf.MaxLen)
			{
				op_done = 1;
			}
			else
			{
				OW->OCR = OW_OCR_CMD_READ_Msk;
			}
		}
		else if (SR & OW_IIR_INT_WRITE_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			OW->OCR = OW_OCR_CMD_READ_Msk;
		}
		else if (SR & OW_IIR_INT_RESET_Msk)
		{
			OW->IIR = OW_IIR_INT_CLR_Msk;
			OW->TBR = prv_ow.cmd;
			OW->OCR = OW_OCR_CMD_WRITE_Msk;
		}
		break;
	}
DONE:
	if (op_done)
	{
		OW->IER = 0;
		OW->IIR = OW_IIR_INT_CLR_Msk;
		OW->OCR = OW_OCR_CMD_FLUSH_Msk;
		if (prv_ow.wait_finish)
		{
			prv_ow.wait_finish = 0;
			OS_MutexRelease(prv_ow.finish_sem);
		}
	}
	XIC_EnableIRQ(PXIC0_ONEW_IRQn);
}

void luat_onewire_init(int id)
{
	if (prv_ow.is_init) return;
	GPR_clockEnable(PCLK_ONEW);
#if 0
	if (luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_ONEWIRE, 0))
	{
		GPIO_IomuxEC7XX(17, 4, 0, 1);
	}
#endif
	pin_iomux_info iomux_info;
	luat_pin_get_iomux_info(LUAT_MCU_PERIPHERAL_ONEWIRE, 0, &iomux_info);
	luat_pin_iomux_print(&iomux_info, 1);
	luat_pin_iomux_config(iomux_info, 0, 1);
	prv_ow.finish_sem = OS_MutexCreate();
	prv_ow.is_init = 1;
	OW->IOR = 0;
	OW->ECR = OW_ECR_ENABLE_Msk|OW_ECR_CLK_EN_Msk|OW_ECR_AUTO_CGEN_Msk;
	prv_ow.CDR = OW->CDR;
	prv_ow.ATCR.v = OW->ATCR;
	prv_ow.RTCR.v = OW->RTCR;
    XIC_SetVector(PXIC0_ONEW_IRQn, OW_ISR);
    XIC_EnableIRQ(PXIC0_ONEW_IRQn);
}

void luat_onewire_deinit(int id)
{
	if (!prv_ow.is_init) return;
	GPR_clockDisable(PCLK_ONEW);
	prv_ow.is_init = 0;
}

void luat_onewire_setup_timing(int id, onewire_timing_t *timing)
{
	if (!prv_ow.is_init) return;
	if (timing->type)
	{
		if (timing->timing_tick.clock_div)
		{
			prv_ow.CDR = timing->timing_tick.clock_div - 1;
		}
		else
		{
			prv_ow.CDR = 25;
		}
		prv_ow.RTCR.send_time_div10 = timing->timing_tick.reset_keep_low_tick/10;
		prv_ow.RTCR.wait_time_div10 = timing->timing_tick.reset_wait_ack_tick/10;
		prv_ow.RTCR.read_time_min = timing->timing_tick.reset_read_ack_before_tick;
		prv_ow.RTCR.read_time_max = (timing->timing_tick.reset_read_ack_tick + timing->timing_tick.reset_read_ack_before_tick)/10;
		prv_ow.ATCR.recovery_time = timing->timing_tick.wr_recovery_tick;
		prv_ow.ATCR.slot_time_div10 = timing->timing_tick.wr_slot_tick/10;
		prv_ow.ATCR.start_time = timing->timing_tick.wr_start_tick;
		prv_ow.ATCR.write_start_time = timing->timing_tick.wr_write_start_tick;
		prv_ow.ATCR.read_start_time = timing->timing_tick.wr_read_start_tick;
	}
	else
	{
		prv_ow.CDR = 25;
		prv_ow.RTCR.send_time_div10 = timing->timing_us.reset_keep_low_time/10;
		prv_ow.RTCR.wait_time_div10 = timing->timing_us.reset_wait_ack_time/10;
		prv_ow.RTCR.read_time_min = timing->timing_us.reset_read_ack_before_time;
		prv_ow.RTCR.read_time_max = (timing->timing_us.reset_read_ack_time + timing->timing_us.reset_read_ack_before_time)/10;
		prv_ow.ATCR.recovery_time = timing->timing_us.wr_recovery_time;
		prv_ow.ATCR.slot_time_div10 = timing->timing_us.wr_slot_time/10;
		prv_ow.ATCR.start_time = timing->timing_us.wr_start_time;
		prv_ow.ATCR.write_start_time = timing->timing_us.wr_write_start_time;
		prv_ow.ATCR.read_start_time = timing->timing_us.wr_read_start_time;
	}

}

static void luat_onewire_wait_timeout(uint32_t timeout)
{
	OS_MutexLockWtihTime(prv_ow.finish_sem, timeout);
	if (prv_ow.wait_finish)
	{
		DBG("to");
		OW->OCR = OW_OCR_CMD_FLUSH_Msk;
		prv_ow.wait_finish = 0;
		prv_ow.check_ack = 0;
	}
	soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_USER - 1, 0);
}

static void luat_onewire_prepare(void)
{
	GPR_clockEnable(PCLK_ONEW);
	OW->ECR = OW_ECR_ENABLE_Msk|OW_ECR_CLK_EN_Msk|OW_ECR_AUTO_CGEN_Msk;
	OW->CDR = prv_ow.CDR;
	OW->RTCR = prv_ow.RTCR.v;
	OW->ATCR = prv_ow.ATCR.v;
	soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_USER - 1, 1);
}

int luat_onewire_reset(int id, uint8_t check_ack)
{
	if (!prv_ow.is_init) return -ERROR_PERMISSION_DENIED;
	prv_ow.op = OW_OP_RESET;
	prv_ow.wait_finish = 1;
	prv_ow.ack_result = 0;
	prv_ow.check_ack = check_ack;
	luat_onewire_prepare();
	OW->IER = 0x0f;
	OW->OCR = OW_OCR_CMD_RESET_Msk;
	luat_onewire_wait_timeout(100);
	if (check_ack)
	{
		return prv_ow.ack_result?ERROR_NONE:-ERROR_NO_SUCH_ID;
	}
	else
	{
		return ERROR_NONE;
	}
}

void luat_onewire_write_bit(int id, uint8_t level)
{
	if (!prv_ow.is_init) return ;
	uint16_t timeout = 20000;
	luat_onewire_prepare();
    OW->DFR = 0;
    OW->TBR = level;
    OW->IER = 0;
    OW->OCR = OW_OCR_CMD_WRITE_Msk;
	while(!(OW->IIR & OW_IIR_INT_WRITE_Msk)) {
		timeout--;
		if (!timeout)
		{
			DBG("to");
			break;
		}
	}
	OW->OCR = OW_OCR_CMD_FLUSH_Msk;
	soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_USER - 1, 0);
}

uint8_t luat_onewire_read_bit(int id)
{
	if (!prv_ow.is_init) return 0;
	uint16_t timeout = 20000;
	luat_onewire_prepare();
    OW->DFR = 0;
    OW->IER = 0;
    OW->OCR = OW_OCR_CMD_READ_Msk;
	while(!(OW->IIR & OW_IIR_INT_READ_Msk)) {
		timeout--;
		if (!timeout)
		{
			DBG("to");
			break;
		}
	}
	uint8_t res = OW->RBR;
	OW->OCR = OW_OCR_CMD_FLUSH_Msk;
	soc_sys_force_wakeup_on_off(SOC_SYS_CTRL_USER - 1, 0);
	return res;
}

int luat_onewire_write_byte(int id, const uint8_t *data, uint32_t len, uint8_t is_msb, uint8_t need_reset, uint8_t check_ack)
{
	if (!prv_ow.is_init) return -ERROR_PERMISSION_DENIED;
	prv_ow.wait_finish = 1;
	prv_ow.ack_result = 0;
	prv_ow.check_ack = check_ack;
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	prv_ow.op = OW_OP_WRITE;
	luat_onewire_prepare();
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->IER = OW_IER_INTEN_RESET_Msk|OW_IER_INTEN_RESET_PD_Msk|OW_IER_INTEN_WRITE_Msk|OW_IER_INTEN_READ_Msk;
	if (need_reset)
	{
		OW->OCR = OW_OCR_CMD_RESET_Msk;
	}
	else
	{
		OW->TBR = prv_ow.data_buf.Data[0];
		OW->OCR = OW_OCR_CMD_WRITE_Msk;
	}
	luat_onewire_wait_timeout(len + 100);
	if (need_reset && check_ack)
	{
		return prv_ow.ack_result?ERROR_NONE:-ERROR_NO_SUCH_ID;
	}
	else
	{
		return ERROR_NONE;
	}
}

int luat_onewire_read_byte_with_cmd(int id, const uint8_t cmd, uint8_t *data, uint32_t len, uint8_t is_msb, uint8_t need_reset, uint8_t check_ack)
{
	if (!prv_ow.is_init) return -ERROR_PERMISSION_DENIED;
	prv_ow.wait_finish = 1;
	prv_ow.ack_result = 0;
	prv_ow.check_ack = check_ack;
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	uint32_t timeout = (len + 1);
	prv_ow.op = OW_OP_READ_WITH_CMD;
	luat_onewire_prepare();
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->IER = OW_IER_INTEN_RESET_Msk|OW_IER_INTEN_RESET_PD_Msk|OW_IER_INTEN_WRITE_Msk|OW_IER_INTEN_READ_Msk;
	if (need_reset)
	{
		prv_ow.cmd = cmd;
		OW->OCR = OW_OCR_CMD_RESET_Msk;
	}
	else
	{
		OW->TBR = cmd;
		OW->OCR = OW_OCR_CMD_WRITE_Msk;
	}
	luat_onewire_wait_timeout(len + 100);
	if (need_reset && check_ack)
	{
		return prv_ow.ack_result?ERROR_NONE:-ERROR_NO_SUCH_ID;
	}
	else
	{
		return ERROR_NONE;
	}
}

int luat_onewire_read_byte(int id, uint8_t *data, uint32_t len, uint8_t is_msb, uint8_t need_reset, uint8_t check_ack)
{
	if (!prv_ow.is_init) return -ERROR_PERMISSION_DENIED;
	prv_ow.wait_finish = 1;
	prv_ow.ack_result = 0;
	prv_ow.check_ack = check_ack;
	Buffer_StaticInit(&prv_ow.data_buf, data, len);
	uint32_t timeout = len + 10;
	prv_ow.op = OW_OP_READ;
	luat_onewire_prepare();
	OW->DFR = OW_DFR_MODE_BYTE_Msk|(is_msb?OW_DFR_MODE_ENDIAN_Msk:0);
	OW->IER = OW_IER_INTEN_RESET_Msk|OW_IER_INTEN_RESET_PD_Msk|OW_IER_INTEN_WRITE_Msk|OW_IER_INTEN_READ_Msk;
	if (need_reset)
	{
		OW->OCR = OW_OCR_CMD_RESET_Msk;
	}
	else
	{
		OW->OCR = OW_OCR_CMD_READ_Msk;
	}
	luat_onewire_wait_timeout(len + 100);
	if (need_reset && check_ack)
	{
		return prv_ow.ack_result?ERROR_NONE:-ERROR_NO_SUCH_ID;
	}
	else
	{
		return ERROR_NONE;
	}
}

void luat_onewire_debug(int id, uint8_t on_off)
{
	prv_ow.is_debug = on_off;
}
