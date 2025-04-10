#include "common_api.h"
#include "soc_service.h"
#include "csdk.h"
#include "driver_gpio.h"
#ifdef CHIP_EC718
static luat_uart_pin_iomux_t prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 18,
						},
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 19,
						},
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 1,
						},
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 2,
						},
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 13,
						},
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 8,
						},
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 9,
						},
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 15,
						},
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 13,
						},
				},
		},
};
static luat_i2c_pin_iomux_t prv_i2c_iomux[I2C_MAX] =
{
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 15,
						},
				},
		},
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 18,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
};
static luat_pwm_pin_iomux_t prv_pwm_iomux[HW_TIMER_MAX - 1] =
{
		{	//0
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 1,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//1
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 24,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//2
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 25,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//3
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//4
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 27,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
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
						.uid =
						{
								.ec_gpio_id = 25,
						},
				},
				.pin_list[LUAT_PIN_CAN_TX] =
				{
						.altfun_id = 7,
						.uid =
						{
								.ec_gpio_id = 26,
						},
				},
				.pin_list[LUAT_PIN_CAN_STB] =
				{
						.altfun_id = 6,
						.uid =
						{
								.ec_gpio_id = 28,
						},
				},
		}
};
#endif
static pin_iomux_info prv_gpio_iomux[4] =
{
		{	//16

				.altfun_id = 4,
				.uid =
				{
						.ec_gpio_id = 16,
						.ec_gpio_is_altfun4 = 1,
				},

		},
		{	//17

				.altfun_id = 4,
				.uid =
				{
						.ec_gpio_id = 17,
						.ec_gpio_is_altfun4 = 1,
				},

		},
		{	//18

				.altfun_id = 0,
				.uid =
				{
						.ec_gpio_id = 18,
						.ec_gpio_is_altfun4 = 0,
				},

		},
		{	//19

				.altfun_id = 0,
				.uid =
				{
						.ec_gpio_id = 19,
						.ec_gpio_is_altfun4 = 0,
				},

		},
};
static pin_iomux_info prv_onewire_iomux =
{
		.altfun_id = 4,
		.uid =
		{
				.ec_gpio_id = 2,
		},
};
#else
static luat_uart_pin_iomux_t prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{ //rx
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 8,
						},
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{ //tx
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 9,
						},
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{ //rts
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{ //cts
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 20,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
		{
				.pin_list[LUAT_PIN_UART_RX] =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 4,
						},
				},
				.pin_list[LUAT_PIN_UART_TX] =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 5,
						},
				},
				.pin_list[LUAT_PIN_UART_RTS] =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.pin_list[LUAT_PIN_UART_CTS] =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 20,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
};

static luat_i2c_pin_iomux_t prv_i2c_iomux[I2C_MAX] =
{
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 17,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 18,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
		{
				.pin_list[LUAT_PIN_I2C_SCL] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.pin_list[LUAT_PIN_I2C_SDA] =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 20,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
};
static luat_pwm_pin_iomux_t prv_pwm_iomux[HW_TIMER_MAX - 1] =
{
		{	//0
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 10,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//1
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 11,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//2
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//3
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
		{	//4
				.pin_list[LUAT_PIN_PWM_P] =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.pin_list[LUAT_PIN_PWM_N] =
				{
						.altfun_id = 0xff,
						.uid =
						{
							.common_gpio_id = 0xff,
						},
				},
		},
};
static pin_iomux_info prv_onewire_iomux =
{
	.altfun_id = 2,
	.uid =
	{
			.ec_gpio_id = 1,
	},
};
#endif

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
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_RX].uid.ec_gpio_id = 6;
		prv_uart_iomux[2].pin_list[LUAT_PIN_UART_TX].uid.ec_gpio_id = 7;
	}

	if (!memcmp(name + 3, "780EPM", 6))
	{
		prv_pwm_iomux[2].pin_list[LUAT_PIN_PWM_P].uid.ec_gpio_id = 31;
		prv_pwm_iomux[4].pin_list[LUAT_PIN_PWM_P].uid.ec_gpio_id = 33;
	}
#endif
}

__USER_FUNC_IN_RAM__ int luat_pin_get_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, pin_iomux_info *pin_list)
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
			pin_list[0].uid.ec_gpio_id = id;
			pin_list[0].uid.ec_gpio_is_altfun4 = 0;
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
		pin_list[0].uid.ec_gpio_id = id;
		if (id >= 17 && id <= 20)
		{
			pin_list[0].altfun_id = 4;
			pin_list[0].uid.ec_gpio_is_altfun4 = 1;
		}
		else
		{
			pin_list[0].altfun_id = 0;
			pin_list[0].uid.ec_gpio_is_altfun4 = 0;
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

__USER_FUNC_IN_RAM__ int luat_pin_set_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, pin_iomux_info *pin_list)
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

iomux_uid_u luat_pin_get_from_num(uint32_t num)
{
	iomux_uid_u uid;
	uid.common_gpio_id = 0xff;
	return uid;
}

void luat_pin_iomux_config(pin_iomux_info pin, uint8_t use_altfunction_pull, uint8_t driver_strength)
{
	if (pin.altfun_id == 0xff || pin.uid.common_gpio_id == 0xff) return;
	uint32_t pad = GPIO_ToPadEC7XX(pin.uid.ec_gpio_id, pin.uid.ec_gpio_is_altfun4?4:0);
//	DBG("gpio %d, altfun %d, pad %d", pin.uid.ec_gpio_id, pin.altfun_id, pad);
	GPIO_IomuxEC7XX(pad, pin.altfun_id, use_altfunction_pull, driver_strength);
}

void luat_pin_iomux_print(pin_iomux_info *pin_list, uint8_t num)
{
	uint32_t pad;
	for(uint8_t i = 0; i < num; i++)
	{
		if (pin_list[i].altfun_id == 0xff || pin_list[i].uid.common_gpio_id == 0xff)
		{
			DBG("index %d no pin %x,%x", i, pin_list[i].altfun_id, pin_list[i].uid.common_gpio_id);
		}
		else
		{
			pad = GPIO_ToPadEC7XX(pin_list[i].uid.ec_gpio_id, pin_list[i].uid.ec_gpio_is_altfun4?4:0);
			DBG("index %d gpio %d, altfun %d, pad %d", i, pin_list[i].uid.ec_gpio_id, pin_list[i].altfun_id, pad);
		}

	}
}
