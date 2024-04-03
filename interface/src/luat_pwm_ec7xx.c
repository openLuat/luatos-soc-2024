#include "ec7xx.h"
#include "clock.h"
#include "timer.h"
#include "pad.h"
#include "common_api.h"
#include "driver_gpio.h"
#include "luat_debug.h"
#include "luat_pwm.h"
#include "luat_mcu.h"

#include "core_hwtimer.h"

#define EIGEN_TIMER(n)             ((TIMER_TypeDef *) (AP_TIMER0_BASE_ADDR + 0x1000*n))
#define PWM_CH_MAX (5)
#define EIGEN_GPIO(n)             ((GPIO_TypeDef *) (RMI_GPIO_BASE_ADDR + 0x1000*n))

typedef struct
{
	const int irq_line;
	const ClockId_e id;
	const ClockSelect_e select;
	CBFuncEx_t callback;
	void *user_param;

	uint32_t pulse_total_num;
	uint32_t pulse_cnt;
	uint32_t freq;
	uint32_t last_pulse_rate;
	uint8_t update_period;
	uint8_t reverse;
	int stop_level;
}luat_pwm_ctrl_t;

static luat_pwm_ctrl_t g_s_pwm_table[PWM_CH_MAX] =
{
		{
				PXIC0_TIMER0_IRQn,
				FCLK_TIMER0,
				FCLK_TIMER0_SEL_26M,
				NULL,
				NULL,
				0,0,0,0,0,0
		},
		{
				PXIC0_TIMER1_IRQn,
				FCLK_TIMER1,
				FCLK_TIMER1_SEL_26M,
				NULL,
				NULL,
				0,0,0,0,0,0
		},
		{
				PXIC0_TIMER2_IRQn,
				FCLK_TIMER2,
				FCLK_TIMER2_SEL_26M,
				NULL,
				NULL,
				0,0,0,0,0,0
		},
		{
				-1,
				0,0,
				NULL,
				NULL,
				0,0,0,0,0,0
		},
		{
				PXIC0_TIMER4_IRQn,
				FCLK_TIMER4,
				FCLK_TIMER4_SEL_26M,
				NULL,
				NULL,
				0,0,0,0,0,0
		},
};

__ISR_IN_RAM__ static void s_timer_handle(uint32_t instance)
{
    if (EIGEN_TIMER(instance)->TSR & TIMER_MATCH1_INTERRUPT_FLAG)
    {
    	EIGEN_TIMER(instance)->TSR = TIMER_MATCH1_INTERRUPT_FLAG;
    	g_s_pwm_table[instance].pulse_cnt++;
        if (g_s_pwm_table[instance].pulse_total_num && (g_s_pwm_table[instance].pulse_cnt >= g_s_pwm_table[instance].pulse_total_num))
        {
            TIMER_stop(instance);
            if (g_s_pwm_table[instance].callback)
            {
            	g_s_pwm_table[instance].callback(instance, g_s_pwm_table[instance].user_param);
            }
        }
    }
}

__ISR_IN_RAM__ static void timer0_isr(void)
{
	s_timer_handle(0);
}
__ISR_IN_RAM__ static void timer1_isr(void)
{
	s_timer_handle(1);
}
__ISR_IN_RAM__ static void timer2_isr(void)
{
	s_timer_handle(2);
}
__ISR_IN_RAM__ static void timer4_isr(void)
{
	s_timer_handle(4);
}


int luat_pwm_set_callback(int channel, CBFuncEx_t callback, void *param)
{
	if ((channel >= PWM_CH_MAX) || (channel == 3) || g_s_pwm_table[channel].freq) return -1;
	g_s_pwm_table[channel].callback = callback;
	g_s_pwm_table[channel].user_param = param;
	return 0;
}


int luat_pwm_open(int channel, size_t freq,  size_t pulse, int pnum) {
	// uint8_t instance = channel % 10;
	if ((channel >= PWM_CH_MAX) || (channel == 3) || (channel < 0) || (freq > 13000000)) return -1;
	CLOCK_setClockSrc(g_s_pwm_table[channel].id, g_s_pwm_table[channel].select);
	CLOCK_setClockDiv(g_s_pwm_table[channel].id, 1);
	XIC_DisableIRQ(g_s_pwm_table[channel].irq_line);
	TIMER_stop(channel);
	TIMER_deInit(channel);
	TimerConfig_t config;
	TIMER_getDefaultConfig(&config);
	TIMER_init(channel, &config);
	if (pulse > 1000) pulse = 1000;
	g_s_pwm_table[channel].freq = freq;
	g_s_pwm_table[channel].last_pulse_rate = pulse;
	g_s_pwm_table[channel].pulse_total_num = pnum;
	g_s_pwm_table[channel].pulse_cnt = 0;
	uint32_t period = 26000000 / freq;
	uint64_t temp = period;
	temp *= pulse;
	uint32_t low_cnt = period - temp / 1000;
	/* Set PWM period */
	EIGEN_TIMER(channel)->TMR[1] = period - 1;
	switch(pulse)
	{
	case 0:
		EIGEN_TIMER(channel)->TMR[0] = period;
		break;
	case 1000:
		EIGEN_TIMER(channel)->TMR[0] = period - 1;
		break;
	default:
		EIGEN_TIMER(channel)->TMR[0] = low_cnt?(low_cnt - 1):0;
		break;
	}

	EIGEN_TIMER(channel)->TIVR = 0;
	uint8_t stop_level = TIMER_PWM_STOP_HOLD;
	switch (g_s_pwm_table[channel].stop_level)
	{
	case 0:
		stop_level = TIMER_PWM_STOP_HOLD;
		break;
	case 1:
		stop_level = TIMER_PWM_STOP_LOW;
		break;
	case 2:
		stop_level = TIMER_PWM_STOP_HIGH;
		break;
	default:
		break;
	}

	/* Enable PWM out */
	EIGEN_TIMER(channel)->TCTLR =  ((stop_level << TIMER_TCTLR_PWM_STOP_VALUE_Pos) & TIMER_TCTLR_PWM_STOP_VALUE_Msk) | ((2 << TIMER_TCTLR_MCS_Pos) | TIMER_TCTLR_PWMOUT_Msk);

    if(0 != pnum)
    {
        uint32_t registerValue = EIGEN_TIMER(channel)->TCTLR;
        registerValue |= TIMER_TCTLR_IE_1_Msk;
        registerValue &= ~TIMER_TCTLR_IT_1_Msk;
    	EIGEN_TIMER(channel)->TCTLR |= registerValue;
        switch(channel)
        {
        case 0:
        	XIC_SetVector(g_s_pwm_table[channel].irq_line,timer0_isr);
        	break;
        case 1:
        	XIC_SetVector(g_s_pwm_table[channel].irq_line,timer1_isr);
        	break;
        case 2:
        	XIC_SetVector(g_s_pwm_table[channel].irq_line,timer2_isr);
        	break;
        case 4:
        	XIC_SetVector(g_s_pwm_table[channel].irq_line,timer4_isr);
        	break;
        }
        XIC_ClearPendingIRQ(g_s_pwm_table[channel].irq_line);
        XIC_EnableIRQ(g_s_pwm_table[channel].irq_line);
        XIC_SuppressOvfIRQ(g_s_pwm_table[channel].irq_line);
    }
	if(0 == g_s_pwm_table[channel].reverse)
	{
    	if (luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_PWM, channel))
    	{
#if defined TYPE_EC716S
			switch (channel)
        	{
        	case 0:
        	    GPIO_IomuxEC7XX(22, 3, 1, 0);
        	    break;
        	case 1:
        	    GPIO_IomuxEC7XX(23, 3, 1, 0);
        	    break;
        	case 2:
        	    GPIO_IomuxEC7XX(24, 3, 1, 0);
        	    break;
        	case 4:
        	    GPIO_IomuxEC7XX(26, 3, 1, 0);
        	    break;
        	default:
        	    break;
        	}
#else
        	switch (channel)
        	{
        	case 0:
        	    GPIO_IomuxEC7XX(16, 5, 1, 0);
        	    break;
        	case 1:
        	    GPIO_IomuxEC7XX(49, 5, 1, 0);
        	    break;
        	case 2:
        	    GPIO_IomuxEC7XX(50, 5, 1, 0);
        	    break;
        	case 4:
        	    GPIO_IomuxEC7XX(52, 5, 1, 0);
        	    break;
        	default:
        	    break;
        	}
#endif
    	}
	}
    TIMER_start(channel);
    return 0;
}

int luat_pwm_update_dutycycle(int channel,size_t pulse)
{
	if ((channel >= PWM_CH_MAX) || (channel == 3)) return -1;
	uint32_t period = 26000000 / g_s_pwm_table[channel].freq;
	uint64_t temp = period;
	temp *= pulse;
	uint32_t low_cnt = period - temp / 1000;
	if (g_s_pwm_table[channel].update_period)
	{
		while (EIGEN_TIMER(channel)->TACR > 5) {;}
		EIGEN_TIMER(channel)->TMR[1] = period - 1;
	}
	if (pulse && g_s_pwm_table[channel].last_pulse_rate && (g_s_pwm_table[channel].last_pulse_rate != 1000) && (low_cnt < EIGEN_TIMER(channel)->TMR[0]))
	{
		while (EIGEN_TIMER(channel)->TACR <= EIGEN_TIMER(channel)->TMR[0]) {;}
	}
	switch(pulse)
	{
	case 0:
		EIGEN_TIMER(channel)->TMR[0] = period;
		break;
	case 1000:
		EIGEN_TIMER(channel)->TMR[0] = period - 1;
		break;
	default:
		EIGEN_TIMER(channel)->TMR[0] = low_cnt;
		break;
	}
	g_s_pwm_table[channel].last_pulse_rate = pulse;
    return 0;
}

int luat_pwm_setup(luat_pwm_conf_t* conf)
{
    int channel = conf->channel;
    if ((channel >= PWM_CH_MAX) || (channel == 3)) return -1;
	switch(conf->precision)
	{
	case 100:
		conf->pulse *= 10;
		break;
	case 256:
		conf->pulse = (conf->pulse * 1000) >> 8;
		break;
	case 1000:
		break;
	default:
		return -1;
	}
	if (conf->pulse && g_s_pwm_table[channel].freq)
	{
		if (!conf->pnum && !g_s_pwm_table[channel].pulse_total_num)
		{
			if ((g_s_pwm_table[channel].freq == conf->period) && (g_s_pwm_table[channel].last_pulse_rate == conf->pulse))
			{
				return 0;
			}
			if (g_s_pwm_table[channel].freq != conf->period)
			{
				g_s_pwm_table[channel].update_period = 1;
				g_s_pwm_table[channel].freq = conf->period;
			}
			luat_pwm_update_dutycycle(channel, conf->pulse);
			g_s_pwm_table[channel].update_period = 0;
			return 0;
		}
	}
	g_s_pwm_table[channel].reverse = conf->reverse;
	g_s_pwm_table[channel].stop_level = conf->stop_level;
    return luat_pwm_open(conf->channel,conf->period,conf->pulse,conf->pnum);
}

int luat_pwm_capture(int channel,int freq)
{
	return -1;
}

int luat_pwm_close(int channel)
{
	if ((channel >= PWM_CH_MAX) || (channel == 3)) return -1;
	XIC_DisableIRQ(g_s_pwm_table[channel].irq_line);
	TIMER_stop(channel);
	TIMER_deInit(channel);
	g_s_pwm_table[channel].freq = 0;
	g_s_pwm_table[channel].pulse_total_num = 0;
	g_s_pwm_table[channel].pulse_cnt = 0;
    return 0;
}

typedef struct
{
	OPQueue_CmdStruct *Cmd;
	CBFuncEx_t CmdDoneCB;
	void *pCmdDoneParam;
	uint32_t TotalCount;
	uint32_t TotalRepeat;
	uint32_t CurCount;
	uint32_t RepeatCnt;
	uint32_t CmdQueuePos;
	uint32_t LastRepeatCnt;
	uint32_t LastCount;
	uint8_t IsQueueRunning;
	uint8_t ContinueDelay;
}HWTimer_CtrlStruct;

static HWTimer_CtrlStruct prvHWTimer[HW_TIMER_MAX - 1];

static void prvHWTimer_IrqHandlerOperationQueue(uint32_t HWTimerID);
static void prvHWTimer_StartOperationQueue(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin);

static int32_t __USER_FUNC_IN_RAM__ prvHWTimer_DummyCB(void *pData, void *pParam)
{
	return 0;
}

static int32_t __USER_FUNC_IN_RAM__ prvHWTimer_OperationQueuExti(void *pData, void *pParam)
{
	uint32_t HWTimerID = (uint32_t)pParam;
	prvHWTimer_StartOperationQueue(HWTimerID, &prvHWTimer[HWTimerID], (uint32_t)pData);
	return 0;
}
extern uint8_t luat_gpio_get_alt(uint8_t GPIO);
__USER_FUNC_IN_RAM__ uint32_t prvGPIO_ToPadEC7XXFast(uint32_t Pin)
{
#ifdef TYPE_EC716S
	if (Pin <= HAL_GPIO_16) return Pin + 12;
	if (Pin <= HAL_GPIO_20) return Pin - 9;
#else
	if (Pin < HAL_GPIO_16) return Pin + 15;
	if (Pin <= HAL_GPIO_19)
	{
		if (luat_gpio_get_alt(Pin))
		{
			return Pin - 5;
		}
		else
		{
			return Pin + 15;
		}

	}
	if (Pin <= HAL_GPIO_28) return Pin + 25;
	if (Pin <= HAL_GPIO_38) return Pin + 6;
#endif
	return 0;

}

typedef union
{
	PadConfig_t Config;
	__IO uint32_t Value;
}PadConfig_u;

__USER_FUNC_IN_RAM__ uint32_t prvGPIO_PullFast(uint32_t Pin, uint8_t IsPull, uint8_t IsUp)
{
	uint32_t Pad = prvGPIO_ToPadEC7XXFast(Pin);
	PadConfig_u uConfig;
	CLOCK_clockEnable(PCLK_PAD);
	uConfig.Value = PAD->PCR[Pad];
	uConfig.Config.pullSelect = PAD_PULL_INTERNAL;
	uConfig.Config.pullUpEnable = PAD_PULL_UP_DISABLE;
	uConfig.Config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
	if (IsPull)
	{
		if (IsUp)
		{
			uConfig.Config.pullUpEnable = PAD_PULL_UP_ENABLE;
		}
		else
		{
			uConfig.Config.pullDownEnable = PAD_PULL_DOWN_ENABLE;
		}
	}

	PAD->PCR[Pad] = uConfig.Value;
	CLOCK_clockDisable(PCLK_PAD);
}


typedef int (*queue_fun)(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin);
#define OPERATION_GO_ON		(0)
#define OPERATION_OUT_IRQ	(-1)
#define OPERATION_RESET_TIMER	(1)
static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationEnd(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->CurCount = 0;
	HWTimer->RepeatCnt++;
	if (HWTimer->TotalRepeat && (HWTimer->RepeatCnt >= HWTimer->TotalRepeat))
	{
		prvHWTimer[HWTimerID].LastCount = prvHWTimer[HWTimerID].CurCount;
		prvHWTimer[HWTimerID].LastRepeatCnt = prvHWTimer[HWTimerID].RepeatCnt;
		HWTimer_Stop(HWTimerID);
		prvHWTimer[HWTimerID].CmdDoneCB(0, prvHWTimer[HWTimerID].pCmdDoneParam);
		return OPERATION_OUT_IRQ;
	}
	else
	{
		return OPERATION_GO_ON;
	}
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationDelayOnce(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->ContinueDelay = 0;
	return OPERATION_RESET_TIMER;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationDelay(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->ContinueDelay = 1;
	return OPERATION_RESET_TIMER;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationRepeatDelay(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->CurCount++;
	return OPERATION_OUT_IRQ;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationSetGPIOOut(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	GPIO_Output(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level);
	uint32_t Port = (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay >> 4);
	uint32_t PinMask = 1 << (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay & 0x0000000f);
	EIGEN_GPIO(Port)->OUTENSET = PinMask;
	prvGPIO_PullFast(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.PullMode, (HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.PullMode > 1)?0:1);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationSetGPIOIn(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	uint32_t Port = (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay >> 4);
	uint32_t PinMask = 1 << (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay & 0x0000000f);
	EIGEN_GPIO(Port)->OUTENCLR = PinMask;
	prvGPIO_PullFast(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.PullMode, (HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.PullMode > 1)?0:1);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationGPIOOut(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	GPIO_Output(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationGPIOIn(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level = GPIO_Input(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationGPIOInCB(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level = GPIO_Input(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay);
	HWTimer->Cmd[HWTimer->CurCount].CB(HWTimerID, &HWTimer->Cmd[HWTimer->CurCount]);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationCB(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->Cmd[HWTimer->CurCount].CB(HWTimerID, HWTimer->Cmd[HWTimer->CurCount].uParam.pParam);
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationCaptureSet(uint32_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	prvGPIO_PullFast(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.ExitArg.PullMode, (HWTimer->Cmd[HWTimer->CurCount].uArg.ExitArg.PullMode > 1)?0:1);
	uint32_t Port = (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay >> 4);
	uint32_t PinMask = 1 << (HWTimer->Cmd[HWTimer->CurCount].PinOrDelay & 0x0000000f);
	EIGEN_GPIO(Port)->OUTENCLR = PinMask;

	EIGEN_TIMER(HWTimerID)->TCCR = 0;
	EIGEN_TIMER(HWTimerID)->TMR[1] = HWTimer->Cmd[HWTimer->CurCount].uParam.MaxCnt;
	EIGEN_TIMER(HWTimerID)->TIVR = 0;
	EIGEN_TIMER(HWTimerID)->TCCR = 1;
	GPIO_ExtiSetCB(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, prvHWTimer_OperationQueuExti, HWTimerID);
	prvHWTimer[HWTimerID].ContinueDelay = 0;
	switch(HWTimer->Cmd[HWTimer->CurCount].uArg.ExitArg.ExtiMode)
	{
	case OP_QUEUE_CMD_IO_EXTI_BOTH:
		GPIO_ExtiConfig(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, 0, 1, 1);
		break;
	case OP_QUEUE_CMD_IO_EXTI_UP:
		GPIO_ExtiConfig(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, 0, 1, 0);
		break;
	case OP_QUEUE_CMD_IO_EXTI_DOWN:
		GPIO_ExtiConfig(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, 0, 0, 1);
		break;
	}

	HWTimer->CurCount++;
	return OPERATION_OUT_IRQ;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationCapture(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->Cmd[HWTimer->CurCount].PinOrDelay = Pin;
	if (!EIGEN_TIMER(HWTimerID)->TCCR)
	{
		HWTimer->Cmd[HWTimer->CurCount + 1].Operation = OP_QUEUE_CMD_CAPTURE_END;
	}
	else
	{
		HWTimer->Cmd[HWTimer->CurCount].uParam.MaxCnt =  EIGEN_TIMER(HWTimerID)->TACR;
		HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level = GPIO_Input(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay);
	}
	HWTimer->CurCount++;
	if (OP_QUEUE_CMD_CAPTURE_END != HWTimer->Cmd[HWTimer->CurCount].Operation)
	{
		return OPERATION_OUT_IRQ;
	}
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationCaptureCB(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	HWTimer->Cmd[HWTimer->CurCount].PinOrDelay = Pin;
	if (!EIGEN_TIMER(HWTimerID)->TCCR)
	{
		HWTimer->Cmd[HWTimer->CurCount + 1].Operation = OP_QUEUE_CMD_CAPTURE_END;
	}
	else
	{
		HWTimer->Cmd[HWTimer->CurCount].uParam.MaxCnt = EIGEN_TIMER(HWTimerID)->TACR;
		HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level = GPIO_Input(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay);
	}
	HWTimer->Cmd[HWTimer->CurCount].CB(HWTimerID, &HWTimer->Cmd[HWTimer->CurCount]);
	HWTimer->CurCount++;
	if (OP_QUEUE_CMD_CAPTURE_END != HWTimer->Cmd[HWTimer->CurCount].Operation)
	{
		return OPERATION_OUT_IRQ;
	}
	return OPERATION_GO_ON;
}

static int __USER_FUNC_IN_RAM__ prvHWTimer_OperationCaptureEnd(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	GPIO_ExtiSetCB(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, NULL, NULL);
	GPIO_ExtiConfig(HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, 0, 0, 0);
	EIGEN_TIMER(HWTimerID)->TCCR = 0;
	HWTimer->CurCount++;
	return OPERATION_GO_ON;
}

static queue_fun queueFunlist[] =
{
		prvHWTimer_OperationEnd,
		prvHWTimer_OperationDelayOnce,	//只有一次delay
		prvHWTimer_OperationDelay,	//连续delay，配合OP_QUEUE_CMD_REPEAT_DELAY使用
		prvHWTimer_OperationRepeatDelay,	//重复OP_QUEUE_CMD_CONTINUE_DELAY
		prvHWTimer_OperationSetGPIOOut,
		prvHWTimer_OperationSetGPIOIn,
		prvHWTimer_OperationGPIOOut,
		prvHWTimer_OperationGPIOIn,
		prvHWTimer_OperationGPIOInCB,
		prvHWTimer_OperationCB,
		prvHWTimer_OperationCaptureSet,
		prvHWTimer_OperationCapture,
		prvHWTimer_OperationCaptureCB,
		prvHWTimer_OperationCaptureEnd,
};

static void __USER_FUNC_IN_RAM__ prvHWTimer_StartOperationQueue(uint8_t HWTimerID, HWTimer_CtrlStruct *HWTimer, uint8_t Pin)
{
	volatile uint32_t Period;
	int result;
	while(HWTimer->IsQueueRunning)
	{

//		DBG("%u,%u,%u,%u,%u,%u,%u,%u,%u", HWTimer->TotalRepeat, HWTimer->RepeatCnt,
//				HWTimer->TotalCount, HWTimer->CurCount, HWTimer->Cmd[HWTimer->CurCount].Operation,
//				HWTimer->Cmd[HWTimer->CurCount].PinOrDelay, HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.Level,
//				HWTimer->Cmd[HWTimer->CurCount].uArg.IOArg.PullMode, HWTimer->Cmd[HWTimer->CurCount].uArg.Time);
		result = queueFunlist[HWTimer->Cmd[HWTimer->CurCount].Operation](HWTimerID, HWTimer, Pin);
		switch(result)
		{
		case OPERATION_OUT_IRQ:
			return;
		case OPERATION_RESET_TIMER:
			goto START_HWTIMER;
			break;
		}
	}
	return ;
START_HWTIMER:
	EIGEN_TIMER(HWTimerID)->TCCR = 0;
	Period = HWTimer->Cmd[HWTimer->CurCount].uArg.Time;
	Period = Period * SOC_TICK_1US + HWTimer->Cmd[HWTimer->CurCount].PinOrDelay;
	EIGEN_TIMER(HWTimerID)->TMR[1] = Period - 1;
	EIGEN_TIMER(HWTimerID)->TIVR = 0;
	EIGEN_TIMER(HWTimerID)->TCCR = 1;
	HWTimer->CurCount++;
	return ;
}


static void __USER_FUNC_IN_RAM__ prvHWTimer_IrqHandlerOperationQueue(uint32_t HWTimerID)
{

	EIGEN_TIMER(HWTimerID)->TSR = TIMER_MATCH1_INTERRUPT_FLAG;

	if (!prvHWTimer[HWTimerID].ContinueDelay)
	{
		EIGEN_TIMER(HWTimerID)->TCCR = 0;
	}
	prvHWTimer_StartOperationQueue(HWTimerID, &prvHWTimer[HWTimerID], HAL_GPIO_NONE);
}


void HWTimer_Stop(uint8_t HWTimerID)
{
	XIC_DisableIRQ(g_s_pwm_table[HWTimerID].irq_line);
	if (prvHWTimer[HWTimerID].IsQueueRunning)
	{
		EIGEN_TIMER(HWTimerID)->TSR = TIMER_MATCH1_INTERRUPT_FLAG;
		TIMER_stop(HWTimerID);
		TIMER_deInit(HWTimerID);
	}
	prvHWTimer[HWTimerID].IsQueueRunning = 0;
	prvHWTimer[HWTimerID].ContinueDelay = 0;
	prvHWTimer[HWTimerID].LastCount = prvHWTimer[HWTimerID].CurCount;
	prvHWTimer[HWTimerID].LastRepeatCnt = prvHWTimer[HWTimerID].RepeatCnt;
}

void HWTimer_GetResultOperationInfo(uint8_t HWTimerID, uint32_t *Repeat, uint32_t *Count)
{
	*Repeat = prvHWTimer[HWTimerID].LastRepeatCnt;
	*Count = prvHWTimer[HWTimerID].LastCount;
}

void HWTimer_InitOperationQueue(uint8_t HWTimerID, uint32_t nCount, uint32_t Repeat, CBFuncEx_t CmdDoneCB, void *pCmdDoneParam)
{
	if (prvHWTimer[HWTimerID].IsQueueRunning)
	{
		HWTimer_Stop(HWTimerID);
		prvHWTimer[HWTimerID].CmdDoneCB(-ERROR_OPERATION_FAILED, prvHWTimer[HWTimerID].pCmdDoneParam);
	}
	prvHWTimer[HWTimerID].TotalCount = nCount;
	prvHWTimer[HWTimerID].TotalRepeat = Repeat;
	if (prvHWTimer[HWTimerID].Cmd)
	{
		free(prvHWTimer[HWTimerID].Cmd);
		prvHWTimer[HWTimerID].Cmd = NULL;
	}
	prvHWTimer[HWTimerID].Cmd = calloc((nCount + 1), sizeof(OPQueue_CmdStruct));
	prvHWTimer[HWTimerID].CmdQueuePos = 0;
	if (CmdDoneCB)
	{
		prvHWTimer[HWTimerID].CmdDoneCB = CmdDoneCB;
	}
	else
	{
		prvHWTimer[HWTimerID].CmdDoneCB = prvHWTimer_DummyCB;
	}
	prvHWTimer[HWTimerID].pCmdDoneParam = pCmdDoneParam;
}

void HWTimer_AddOperation(uint8_t HWTimerID, OPQueue_CmdStruct *pCmd)
{

	if (prvHWTimer[HWTimerID].TotalCount > prvHWTimer[HWTimerID].CmdQueuePos)
	{
		memcpy(&prvHWTimer[HWTimerID].Cmd[prvHWTimer[HWTimerID].CmdQueuePos], pCmd, sizeof(OPQueue_CmdStruct));
		prvHWTimer[HWTimerID].CmdQueuePos++;
	}
}


static void HWTimer_ResetOperationQueue(uint8_t HWTimerID)
{
	prvHWTimer[HWTimerID].CurCount = 0;
	prvHWTimer[HWTimerID].RepeatCnt = 0;
}


__ISR_IN_RAM__ static void timer0_io_queue_isr(void)
{
	prvHWTimer_IrqHandlerOperationQueue(0);
}
__ISR_IN_RAM__ static void timer1_io_queue_isr(void)
{
	prvHWTimer_IrqHandlerOperationQueue(1);
}
__ISR_IN_RAM__ static void timer2_io_queue_isr(void)
{
	prvHWTimer_IrqHandlerOperationQueue(2);
}
__ISR_IN_RAM__ static void timer4_io_queue_isr(void)
{
	prvHWTimer_IrqHandlerOperationQueue(4);
}

void HWTimer_StartOperationQueue(uint8_t HWTimerID)
{
	HWTimer_Stop(HWTimerID);
	if (prvHWTimer[HWTimerID].IsQueueRunning)
	{

		prvHWTimer[HWTimerID].CmdDoneCB(-ERROR_OPERATION_FAILED, prvHWTimer[HWTimerID].pCmdDoneParam);
	}

	CLOCK_setClockSrc(g_s_pwm_table[HWTimerID].id, g_s_pwm_table[HWTimerID].select);
	CLOCK_setClockDiv(g_s_pwm_table[HWTimerID].id, 1);

	TimerConfig_t config;
	TIMER_getDefaultConfig(&config);
	TIMER_init(HWTimerID, &config);

    uint32_t registerValue = EIGEN_TIMER(HWTimerID)->TCTLR;
    registerValue |= TIMER_TCTLR_IE_1_Msk;
    registerValue &= ~TIMER_TCTLR_IT_1_Msk;
	EIGEN_TIMER(HWTimerID)->TCTLR |= registerValue;

    switch(HWTimerID)
    {
    case 0:
    	XIC_SetVector(g_s_pwm_table[HWTimerID].irq_line,timer0_io_queue_isr);
    	break;
    case 1:
    	XIC_SetVector(g_s_pwm_table[HWTimerID].irq_line,timer1_io_queue_isr);
    	break;
    case 2:
    	XIC_SetVector(g_s_pwm_table[HWTimerID].irq_line,timer2_io_queue_isr);
    	break;
    case 4:
    	XIC_SetVector(g_s_pwm_table[HWTimerID].irq_line,timer4_io_queue_isr);
    	break;
    }
	prvHWTimer[HWTimerID].Cmd[prvHWTimer[HWTimerID].CmdQueuePos].Operation = OP_QUEUE_CMD_END;
	HWTimer_ResetOperationQueue(HWTimerID);
	prvHWTimer[HWTimerID].IsQueueRunning = 1;
    XIC_ClearPendingIRQ(g_s_pwm_table[HWTimerID].irq_line);
    XIC_EnableIRQ(g_s_pwm_table[HWTimerID].irq_line);
    XIC_SuppressOvfIRQ(g_s_pwm_table[HWTimerID].irq_line);
    EIGEN_TIMER(HWTimerID)->TSR = 0xff;
	prvHWTimer_StartOperationQueue(HWTimerID, &prvHWTimer[HWTimerID], HAL_GPIO_NONE);
}

void HWTimer_ClearOperationQueue(uint8_t HWTimerID)
{
//	HWTimer_ResetOperationQueue(HWTimerID);
	prvHWTimer[HWTimerID].CmdQueuePos = 0;
}


void HWTimer_FreeOperationQueue(uint8_t HWTimerID)
{
	free(prvHWTimer[HWTimerID].Cmd);
	prvHWTimer[HWTimerID].Cmd = NULL;
	prvHWTimer[HWTimerID].CmdDoneCB = prvHWTimer_DummyCB;
}

void HWTimer_AddEndCmdInOperationQueue(uint8_t HWTimerID)
{
	HWTimer_CtrlStruct *HWTimer = &prvHWTimer[HWTimerID];
	if (HWTimer->Cmd[HWTimer->CurCount].Operation != OP_QUEUE_CMD_END)
	{
		HWTimer->Cmd[HWTimer->CurCount + 1].Operation = OP_QUEUE_CMD_END;
	}
}

uint8_t HWTimer_CheckOperationQueueDone(uint8_t HWTimerID)
{
	return !prvHWTimer[HWTimerID].IsQueueRunning;
}

uint32_t HWTimer_GetOperationQueueCaptureResult(uint8_t HWTimerID, CBFuncEx_t CB, void *pParam)
{
	uint32_t i = 0;
	uint32_t Cnt = 0;
	if (!prvHWTimer[HWTimerID].Cmd) return 0;
	for(i = 0; i < prvHWTimer[HWTimerID].CmdQueuePos; i++)
	{
		if (OP_QUEUE_CMD_CAPTURE == prvHWTimer[HWTimerID].Cmd[i].Operation)
		{
			CB(&prvHWTimer[HWTimerID].Cmd[i], pParam);
			Cnt++;
		}
	}
	return Cnt;
}

uint32_t HWTimer_GetOperationQueueLen(uint8_t HWTimerID)
{
	if (!prvHWTimer[HWTimerID].Cmd) return 0;
	return prvHWTimer[HWTimerID].CmdQueuePos;
}

OPQueue_CmdStruct *HWTimer_GetOperationQueue(uint8_t HWTimerID)
{
	return 	prvHWTimer[HWTimerID].Cmd;
}

int __USER_FUNC_IN_RAM__ HWTimer_GetIrqLine(uint8_t HWTimerID)
{
	return g_s_pwm_table[HWTimerID].irq_line;
}


