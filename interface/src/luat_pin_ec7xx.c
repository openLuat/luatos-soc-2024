#include "common_api.h"
#include "soc_service.h"
#include "csdk.h"
#include "driver_gpio.h"

const luat_pin_function_description_t air780epx[43]=
{
{{0xffff,0xffff,0xffff,0xffff,0x510,0xffff,0xffff,0xffff},97,11},
{{0xffff,0xffff,0xffff,0xffff,0x511,0xffff,0xffff,0xffff},100,12},
{{0xffff,0xffff,0x100,0x110,0x512,0x300,0xffff,0xffff},67,13},
{{0xffff,0xffff,0x101,0x111,0x513,0x310,0xffff,0xffff},66,14},
{{0x500,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},82,15},
{{0x501,0xffff,0xffff,0x12,0x311,0x300,0xffff,0xffff},22,16},
{{0x502,0xffff,0xffff,0x13,0xffff,0x310,0xffff,0xffff},23,17},
{{0x503,0xffff,0xffff,0xffff,0xffff,0x320,0xffff,0xffff},54,18},
{{0x504,0xffff,0x111,0x12,0xffff,0xffff,0xffff,0xffff},80,19},
{{0x505,0xffff,0x110,0x13,0xffff,0xffff,0xffff,0xffff},81,20},
{{0x506,0xffff,0x20,0x12,0xffff,0xffff,0xffff,0xffff},55,21},
{{0x507,0xffff,0x21,0x13,0xffff,0xffff,0xffff,0xffff},56,22},
{{0x508,0x203,0x111,0x22,0x2,0xffff,0xffff,0xffff},83,23},
{{0x509,0x200,0x110,0x23,0x3,0xffff,0xffff,0xffff},85,24},
{{0x50a,0x201,0xffff,0x20,0xffff,0xffff,0xffff,0xffff},84,25},
{{0x50b,0x202,0xffff,0x21,0xffff,0xffff,0xffff,0xffff},86,26},
{{0x50c,0x213,0x12,0x20,0xffff,0x32,0xffff,0xffff},28,27},
{{0x50d,0x210,0x13,0x21,0xffff,0x33,0xffff,0xffff},29,28},
{{0x50e,0x211,0x101,0x30,0xffff,0x300,0xffff,0xffff},58,29},
{{0x50f,0x212,0x100,0x31,0xffff,0x310,0xffff,0xffff},57,30},
{{0x510,0x0,0x101,0xffff,0xffff,0xffff,0xffff,0xffff},38,31},
{{0x511,0x1,0x100,0xffff,0xffff,0xffff,0xffff,0xffff},39,32},
{{0x512,0x10,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},17,33},
{{0x513,0x11,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},18,34},
{{0x51d,0xffff,0xffff,0xffff,0xffff,0x300,0xffff,0xffff},30,35},
{{0x51e,0xffff,0xffff,0xffff,0xffff,0x310,0xffff,0xffff},31,36},
{{0x51f,0xffff,0xffff,0xffff,0xffff,0x320,0xffff,0xffff},32,37},
{{0x520,0xffff,0xffff,0xffff,0xffff,0x330,0xffff,0xffff},33,38},
{{0x521,0xffff,0xffff,0xffff,0xffff,0x340,0xffff,0xffff},26,39},
{{0x522,0xffff,0x101,0x30,0xffff,0xffff,0xffff,0xffff},53,40},
{{0x523,0xffff,0x100,0x31,0xffff,0xffff,0xffff,0xffff},52,41},
{{0x524,0xffff,0x110,0x2,0xffff,0xffff,0xffff,0xffff},49,42},
{{0x525,0xffff,0x111,0x3,0xffff,0xffff,0xffff,0xffff},50,43},
{{0x526,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff,0xffff},51,44},
{{0x514,0xffff,0xffff,0x341,0xffff,0x330,0xffff,0xffff},102,45},
{{0x515,0xffff,0xffff,0x331,0xffff,0x340,0xffff,0xffff},107,46},
{{0x516,0xffff,0xffff,0x341,0xffff,0x350,0xffff,0xffff},19,47},
{{0x517,0xffff,0xffff,0x311,0xffff,0x300,0xffff,0xffff},99,48},
{{0x518,0xffff,0xffff,0x301,0xffff,0x310,0xffff,0xffff},20,49},
{{0x519,0xffff,0xffff,0x331,0xffff,0x320,0xffff,0xffff},106,50},
{{0x51a,0xffff,0xffff,0x321,0xffff,0x330,0xffff,0xffff},25,51},
{{0x51b,0xffff,0xffff,0x351,0xffff,0x340,0xffff,0xffff},16,52},
{{0x51c,0xffff,0xffff,0x341,0xffff,0x350,0xffff,0xffff},78,53},
};

#ifdef CHIP_EC718
static luat_uart_pin_iomux_t prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 1,
						.uid = 33,
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 1,
						.uid = 34,
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 3,
						.uid = 16,
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 3,
						.uid = 17,
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 3,
						.uid = 27,
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 3,
						.uid = 28,
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 3,
						.uid = 23,
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 3,
						.uid = 24,
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 3,
						.uid = 29,
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 3,
						.uid = 30,
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 5,
						.uid = 27,
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 5,
						.uid = 28,
				},
		},
};
static luat_i2c_pin_iomux_t prv_i2c_iomux[I2C_MAX] =
{
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid = 29,
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid = 30,
				},
		},
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 3,
						.uid = 13,
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 3,
						.uid = 14,
				},
		},
};
static luat_pwm_pin_iomux_t prv_pwm_iomux[HW_TIMER_MAX - 1] =
{
		{	//0
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid = 16,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,

				},
		},
		{	//1
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid = 49,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//2
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid = 50,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//3
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//4
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid = 52,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
};
#ifdef TYPE_EC718M
static luat_can_pin_iomux_t prv_can_iomux[1] =
{
		{//0
				.pin_list[LUAT_PIN_CAN_RX] =
				{
						.altfun_id = 7,
						.uid = 50,

				},
				.pin_list[LUAT_PIN_CAN_TX] =
				{
						.altfun_id = 7,
						.uid = 51,
				},
				.pin_list[LUAT_PIN_CAN_STB] =
				{
						.altfun_id = 6,
						.uid = 53,
				},
		}
};
#endif
static luat_pin_iomux_info prv_gpio_iomux[4] =
{
		{	//16

				.altfun_id = 4,
				.uid = 11,

		},
		{	//17

				.altfun_id = 4,
				.uid = 13,

		},
		{	//18

				.altfun_id = 0,
				.uid = 33,

		},
		{	//19

				.altfun_id = 0,
				.uid = 34,
		},
};
static luat_pin_iomux_info prv_onewire_iomux =
{
		.altfun_id = 4,
		.uid = 17,
};
#else
static luat_uart_pin_iomux_t prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{ //rx
						.altfun_id = 1,
						.uid = 20,
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{ //tx
						.altfun_id = 1,
						.uid = 21,
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{ //rts
						.altfun_id = 5,
						.uid = 10,
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{ //cts
						.altfun_id = 5,
						.uid = 11,
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 2,
						.uid = 16,
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 2,
						.uid = 17,
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 2,
						.uid = 10,
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 2,
						.uid = 11,
				},
		},
};

static luat_i2c_pin_iomux_t prv_i2c_iomux[I2C_MAX] =
{
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid = 8,
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid = 9,
				},
		},
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid = 10,
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid = 11,
				},
		},
};
static luat_pwm_pin_iomux_t prv_pwm_iomux[HW_TIMER_MAX - 1] =
{
		{	//0
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid = 22,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//1
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid = 23,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//2
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid = 24,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//3
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
		{	//4
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid = 26,
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid = 0xff,
				},
		},
};
static luat_pin_iomux_info prv_onewire_iomux =
{
	.altfun_id = 2,
	.uid = 13,
};
#endif
static luat_pin_function_description_t *prv_pin_table;
static uint32_t prv_pin_table_len;
void luat_pin_init(void)
{
	char name[30] = {0};
	if (soc_get_model_name(name, 0))
	{
		DBG("unknow model!");
		return;
	}
#ifdef CHIP_EC718
	if (!memcmp(name + 3, "8000", 4))
	{
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_RX].altfun_id = 2;
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_TX].altfun_id = 2;
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_RX].uid = GPIO_ToPadEC7XX(6, 0);
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_TX].uid = GPIO_ToPadEC7XX(7, 0);
	}

	if (!memcmp(name + 3, "780EPM", 6))
	{
		prv_pwm_iomux[2].pin_list[LUAT_PIN_PWM_P].uid = GPIO_ToPadEC7XX(31, 0);
		prv_pwm_iomux[4].pin_list[LUAT_PIN_PWM_P].uid = GPIO_ToPadEC7XX(33, 0);
	}
	prv_pin_table = air780epx;
	prv_pin_table_len = sizeof(air780epx)/sizeof(luat_pin_function_description_t);
#endif
}

__USER_FUNC_IN_RAM__ int luat_pin_get_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, luat_pin_iomux_info *pin_list)
{
#ifdef CHIP_EC718
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id >= UART_MAX) return -1;
		memcpy(pin_list, &prv_uart_iomux[id], sizeof(luat_uart_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		memcpy(pin_list, &prv_i2c_iomux[id], sizeof(luat_i2c_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= HW_TIMER_MAX) return -1;
		memcpy(pin_list, &prv_pwm_iomux[id], sizeof(luat_pwm_pin_iomux_t));
		break;
#ifdef TYPE_EC718M
	case LUAT_MCU_PERIPHERAL_CAN:
		memcpy(pin_list, &prv_can_iomux, sizeof(luat_can_pin_iomux_t));
		break;
#endif
	case LUAT_MCU_PERIPHERAL_GPIO:
		if (id >= 16 && id <= 19)
		{
			pin_list[0] = prv_gpio_iomux[id - 16];
		}
		else
		{
			pin_list[0].altfun_id = 0;
			pin_list[0].uid = GPIO_ToPadEC7XX(id, 0);
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		pin_list[0] = prv_onewire_iomux;
		break;
	default:
		return -1;
	}
#else
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id >= UART_MAX) return -1;
		memcpy(pin_list, &prv_uart_iomux[id], sizeof(luat_uart_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		memcpy(pin_list, &prv_i2c_iomux[id], sizeof(luat_i2c_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= (HW_TIMER_MAX - 1)) return -1;
		memcpy(pin_list, &prv_pwm_iomux[id], sizeof(luat_pwm_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_GPIO:
		if (id >= 17 && id <= 20)
		{
			pin_list[0].altfun_id = 4;
			pin_list[0].uid = GPIO_ToPadEC7XX(id, 4);
		}
		else
		{
			pin_list[0].altfun_id = 0;
			pin_list[0].uid = GPIO_ToPadEC7XX(id, 0);
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		pin_list[0] = prv_onewire_iomux;
		break;
	default:
		return -1;
	}
#endif
	return 0;
}

__USER_FUNC_IN_RAM__ int luat_pin_set_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, luat_pin_iomux_info *pin_list)
{
#ifdef CHIP_EC718
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id < 2 || id >= UART_MAX) return -1;
		memcpy(&prv_uart_iomux[id], pin_list, sizeof(luat_uart_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		memcpy(&prv_i2c_iomux[id], pin_list, sizeof(luat_i2c_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= (HW_TIMER_MAX - 1)) return -1;
		memcpy(&prv_pwm_iomux[id], pin_list, sizeof(luat_pwm_pin_iomux_t));
		break;
#ifdef TYPE_EC718M
	case LUAT_MCU_PERIPHERAL_CAN:
		memcpy(&prv_can_iomux, pin_list, sizeof(luat_can_pin_iomux_t));
		break;
#endif
	case LUAT_MCU_PERIPHERAL_GPIO:
		if (id >= 16 && id <= 19)
		{
			prv_gpio_iomux[id - 16] = pin_list[0];
		}
		else
		{
			return -1;
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		prv_onewire_iomux = pin_list[0];
		break;
	default:
		return -1;
	}
#else
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id != 1) return -1;
		memcpy(&prv_i2c_iomux[id], pin_list, sizeof(luat_i2c_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= (HW_TIMER_MAX - 1)) return -1;
		memcpy(&prv_pwm_iomux[id], pin_list, sizeof(luat_pwm_pin_iomux_t));
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		prv_onewire_iomux = pin_list[0];
		break;
	default:
		return -1;
	}
#endif
	return 0;
}

int luat_pin_get_description_from_num(uint32_t num, luat_pin_function_description_t *description)
{
	description->uid = 0xff;
	if (!prv_pin_table)
	{
		return -ERROR_PERMISSION_DENIED;
	}
	for (uint32_t i = 0;i < prv_pin_table_len; i++)
	{
		if (prv_pin_table[i].index == num)
		{
			*description = prv_pin_table[i];
			return 0;
		}
	}
	return -ERROR_ID_INVALID;
}

uint8_t luat_pin_get_altfun_id_from_description(uint16_t code, luat_pin_function_description_t *description)
{
	uint8_t altfun_id;
	for(altfun_id = 0; altfun_id < LUAT_PIN_ALT_FUNCTION_MAX; altfun_id++)
	{
		if (description->function_code[altfun_id] == code)
		{
			return altfun_id;
		}
	}
	return 0xff;
}

void luat_pin_iomux_config(luat_pin_iomux_info pin, uint8_t use_altfunction_pull, uint8_t driver_strength)
{
	if (pin.altfun_id == 0xff || pin.uid == 0xff) return;
	GPIO_IomuxEC7XX(pin.uid, pin.altfun_id, use_altfunction_pull, driver_strength);
}

void luat_pin_iomux_print(luat_pin_iomux_info *pin_list, uint8_t num)
{
	uint32_t pad;
	for(uint8_t i = 0; i < num; i++)
	{
		if (pin_list[i].altfun_id == 0xff || pin_list[i].uid == 0xff)
		{
			DBG("index %d no pin %x,%x", i, pin_list[i].altfun_id, pin_list[i].uid);
		}
		else
		{
			DBG("index %d pad %d altfun %d", i, pin_list[i].uid, pin_list[i].altfun_id);
		}

	}
}
