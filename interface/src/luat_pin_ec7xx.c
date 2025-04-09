#include "common_api.h"
#include "soc_service.h"
#include "luat_pin.h"
#include "driver_gpio.h"
#ifdef CHIP_EC718
static uart_iomux_info prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.rx =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 18,
						},
				},
				.tx =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 19,
						},
				},
				.rts =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 1,
						},
				},
				.cts =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 2,
						},
				},
		},
		{
				.rx =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.tx =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 13,
						},
				},
				.rts =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 8,
						},
				},
				.cts =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 9,
						},
				},
		},
		{
				.rx =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.tx =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 15,
						},
				},
				.rts =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.cts =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 13,
						},
				},
		},
};
static i2c_iomux_info prv_i2c_iomux[I2C_MAX] =
{
		{
				.scl =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.sda =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 15,
						},
				},
		},
		{
				.scl =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 18,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.sda =
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
static pwm_iomux_info prv_pwm_iomux[HW_TIMER_MAX] =
{
		{	//0
				.pwm_p =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 1,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//1
				.pwm_p =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 24,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//2
				.pwm_p =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 25,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//3
				.pwm_p =
				{
						.altfun_id = 0xff,
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//4
				.pwm_p =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 27,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//5
				.pwm_p =
				{
						.altfun_id = 0xff,
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		}
};
#ifdef TYPE_EC718M
static can_iomux_info prv_can_iomux[1] =
{
		{//0
				.rx =
				{
						.altfun_id = 7,
						.uid =
						{
								.ec_gpio_id = 25,
						},
				},
				.tx =
				{
						.altfun_id = 7,
						.uid =
						{
								.ec_gpio_id = 26,
						},
				},
				.stb =
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
static gpio_iomux_info prv_gpio_iomux[4] =
{
		{	//16
			.io =
			{
					.altfun_id = 4,
					.uid =
					{
							.ec_gpio_id = 16,
							.ec_gpio_is_altfun4 = 1,
					},
			},
		},
		{	//17
			.io =
			{
					.altfun_id = 4,
					.uid =
					{
							.ec_gpio_id = 17,
							.ec_gpio_is_altfun4 = 1,
					},
			},
		},
		{	//18
			.io =
			{
					.altfun_id = 0,
					.uid =
					{
							.ec_gpio_id = 18,
							.ec_gpio_is_altfun4 = 0,
					},
			},
		},
		{	//19
			.io =
			{
					.altfun_id = 0,
					.uid =
					{
							.ec_gpio_id = 19,
							.ec_gpio_is_altfun4 = 0,
					},
			},
		},
};
static onewire_iomux_info prv_onewire_iomux[1] =
{
		{
			.io =
			{
					.altfun_id = 4,
					.uid =
					{
							.ec_gpio_id = 2,
					},
			},
		},
};
#else
static uart_iomux_info prv_uart_iomux[UART_MAX] =
{
		{},
		{
				.rx =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 8,
						},
				},
				.tx =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 9,
						},
				},
				.rts =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.cts =
				{
						.altfun_id = 5,
						.uid =
						{
								.ec_gpio_id = 20,
								.ec_gpio_is_altfun4 = 1,
						},
				},
		},
		{
				.rx =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 4,
						},
				},
				.tx =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 5,
						},
				},
				.rts =
				{
						.altfun_id = 2,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.cts =
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

static i2c_iomux_info prv_i2c_iomux[I2C_MAX] =
{
		{
				.scl =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 17,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.sda =
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
				.scl =
				{
						.altfun_id = 1,
						.uid =
						{
								.ec_gpio_id = 19,
								.ec_gpio_is_altfun4 = 1,
						},
				},
				.sda =
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
static pwm_iomux_info prv_pwm_iomux[HW_TIMER_MAX] =
{
		{	//0
				.pwm_p =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 10,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//1
				.pwm_p =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 11,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//2
				.pwm_p =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 12,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//3
				.pwm_p =
				{
						.altfun_id = 0xff,
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//4
				.pwm_p =
				{
						.altfun_id = 3,
						.uid =
						{
								.ec_gpio_id = 14,
						},
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		},
		{	//5
				.pwm_p =
				{
						.altfun_id = 0xff,
				},
				.pwm_n =
				{
						.altfun_id = 0xff,
				},
		}
};
static onewire_iomux_info prv_onewire_iomux[1] =
{
		{
			.io =
			{
					.altfun_id = 2,
					.uid =
					{
							.ec_gpio_id = 1,
					},
			},
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
		prv_uart_iomux[2].rx.altfun_id = 2;
		prv_uart_iomux[2].tx.altfun_id = 2;
		prv_uart_iomux[2].rx.uid.ec_gpio_id = 6;
		prv_uart_iomux[2].tx.uid.ec_gpio_id = 7;
	}

	if (!memcmp(name + 3, "780EPM", 6))
	{
		prv_pwm_iomux[2].pwm_p.uid.ec_gpio_id = 31;
		prv_pwm_iomux[4].pwm_p.uid.ec_gpio_id = 33;
	}
#endif
}

__USER_FUNC_IN_RAM__ int luat_pin_get_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, peripheral_iomux_info *pin_iomux_info)
{
#ifdef CHIP_EC718
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id >= UART_MAX) return -1;
		pin_iomux_info->uart = prv_uart_iomux[id];
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		pin_iomux_info->i2c = prv_i2c_iomux[id];
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= HW_TIMER_MAX) return -1;
		pin_iomux_info->pwm = prv_pwm_iomux[id];
		break;
#ifdef TYPE_EC718M
	case LUAT_MCU_PERIPHERAL_CAN:
		pin_iomux_info->can = prv_can_iomux[0];
		break;
#endif
	case LUAT_MCU_PERIPHERAL_GPIO:
		if (id >= 16 && id <= 19)
		{
			pin_iomux_info->gpio = prv_gpio_iomux[id - 16];
		}
		else
		{
			pin_iomux_info->gpio.io.altfun_id = 0;
			pin_iomux_info->gpio.io.uid.ec_gpio_id = id;
			pin_iomux_info->gpio.io.uid.ec_gpio_is_altfun4 = 0;
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		pin_iomux_info->onewire = prv_onewire_iomux[0];
		break;
	default:
		return -1;
	}
#else
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id >= UART_MAX) return -1;
		pin_iomux_info->uart = prv_uart_iomux[id];
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		pin_iomux_info->i2c = prv_i2c_iomux[id];
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= HW_TIMER_MAX) return -1;
		pin_iomux_info->pwm = prv_pwm_iomux[id];
		break;
	case LUAT_MCU_PERIPHERAL_GPIO:
		pin_iomux_info->gpio.io.uid.ec_gpio_id = id;
		if (id >= 17 && id <= 20)
		{
			pin_iomux_info->gpio.io.altfun_id = 4;
			pin_iomux_info->gpio.io.uid.ec_gpio_is_altfun4 = 1;
		}
		else
		{
			pin_iomux_info->gpio.io.altfun_id = 0;
			pin_iomux_info->gpio.io.uid.ec_gpio_is_altfun4 = 0;
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		pin_iomux_info->onewire = prv_onewire_iomux[0];
		break;
	default:
		return -1;
	}
#endif
	return 0;
}

__USER_FUNC_IN_RAM__ int luat_pin_set_iomux_info(LUAT_MCU_PERIPHERAL_E type, uint8_t id, peripheral_iomux_info *pin_iomux_info)
{
#ifdef CHIP_EC718
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_UART:
		if (id < 2 || id >= UART_MAX) return -1;
		prv_uart_iomux[id] = pin_iomux_info->uart;
		break;
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id >= I2C_MAX) return -1;
		prv_i2c_iomux[id] = pin_iomux_info->i2c;
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= HW_TIMER_MAX) return -1;
		prv_pwm_iomux[id] = pin_iomux_info->pwm;
		break;
#ifdef TYPE_EC718M
	case LUAT_MCU_PERIPHERAL_CAN:
		prv_can_iomux[0] = pin_iomux_info->can;
		break;
#endif
	case LUAT_MCU_PERIPHERAL_GPIO:
		if (id >= 16 && id <= 19)
		{
			prv_gpio_iomux[id - 16] = pin_iomux_info->gpio;
		}
		else
		{
			return -1;
		}
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		prv_onewire_iomux[0] = pin_iomux_info->onewire;
		break;
	default:
		return -1;
	}
#else
	switch (type)
	{
	case LUAT_MCU_PERIPHERAL_I2C:
		if (id != 1) return -1;
		prv_i2c_iomux[id] = pin_iomux_info->i2c;
		break;
	case LUAT_MCU_PERIPHERAL_PWM:
		if (id >= HW_TIMER_MAX) return -1;
		prv_pwm_iomux[id] = pin_iomux_info->pwm;
		break;
	case LUAT_MCU_PERIPHERAL_ONEWIRE:
		prv_onewire_iomux[0] = pin_iomux_info->onewire;
		break;
	default:
		return -1;
	}
#endif
	return 0;
}

void luat_pin_iomux_config(pin_iomux_info pin, uint8_t use_altfunction_pull, uint8_t driver_strength)
{
	uint32_t pad = GPIO_ToPadEC7XX(pin.uid.ec_gpio_id, pin.uid.ec_gpio_is_altfun4?4:0);
	DBG("gpio %d, altfun %d, pad %d", pin.uid.ec_gpio_id, pin.altfun_id, pad);
	GPIO_IomuxEC7XX(pad, pin.altfun_id, use_altfunction_pull, driver_strength);
}
