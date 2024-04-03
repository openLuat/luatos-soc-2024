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
#include "FreeRTOS.h"
#include "task.h"
#include "luat_base.h"
#include "luat_mcu.h"
#include "driver_gpio.h"
#include "plat_config.h"

long luat_mcu_ticks(void) {
    return xTaskGetTickCount();
}

int luat_mcu_set_clk(size_t mhz) {
    return 0;
}

int luat_mcu_get_clk(void) {
    return 307;
}


// 隐藏API, 头文件里没有
uint8_t FLASH_readUUID(uint8_t* uuid, uint32_t* validlen);
uint8_t unique_id[16];
uint32_t unique_id_len;
const char* luat_mcu_unique_id(size_t* t) {
    if (unique_id[0] == 0) {
        FLASH_readUUID(unique_id, &unique_id_len);
        if (unique_id_len == 16) {
            for (size_t i = 0; i < 6; i++)
            {
                if (unique_id[unique_id_len - 1] == 0xFF) {
                    unique_id_len --;
                }
                else {
                    break;
                }
            }
        }
    }
    *t = unique_id_len;
    return (const char*)unique_id;
}

uint32_t luat_mcu_hz(void) {
    return 1000;
}

__USER_FUNC_IN_RAM__ uint64_t luat_mcu_tick64(void) {
    return soc_get_poweron_time_tick();
}
int luat_mcu_us_period(void) {
    return 26;
}
__USER_FUNC_IN_RAM__ uint64_t luat_mcu_tick64_ms(void) {
    return soc_get_poweron_time_ms();
}

void luat_mcu_set_clk_source(uint8_t source_main, uint8_t source_32k, uint32_t delay) {
    // nop
}

void luat_os_reboot(int code){
    (void)code;
    ResetECSystemReset();
}

static uint8_t luat_mcu_iomux_ctrl_by_user[LUAT_MCU_PERIPHERAL_PWM + 1];

uint8_t luat_mcu_iomux_is_default(uint8_t type, uint8_t sn)
{
	if (type > LUAT_MCU_PERIPHERAL_PWM) return 1;
	if (sn > 7) return 1;
	return (luat_mcu_iomux_ctrl_by_user[type] & (1 << sn))?0:1;
}

void luat_mcu_iomux_ctrl(uint8_t type, uint8_t sn, int pad_index, uint8_t alt, uint8_t is_input)
{
	if (type > LUAT_MCU_PERIPHERAL_PWM) return;
	if (sn > 7) return;
	if (pad_index != -1)
	{
		luat_mcu_iomux_ctrl_by_user[type] |= (1 << sn);
		if (LUAT_MCU_PERIPHERAL_UART == type)
		{
			GPIO_IomuxEC7XX(pad_index, alt, 0, 0);
			if (is_input)
			{
				GPIO_PullConfig(pad_index, 1, 1);
			}
		}
		else
		{
			GPIO_IomuxEC7XX(pad_index, alt, 1, 0);
		}
	}
	else
	{
		luat_mcu_iomux_ctrl_by_user[type] &= ~(1 << sn);
	}
}

void luat_mcu_set_hardfault_mode(int mode)
{
	uint8_t new_mode = EXCEP_OPTION_DUMP_FLASH_EPAT_RESET;
	switch (mode)
	{
	case 0:
		new_mode = EXCEP_OPTION_DUMP_FLASH_EPAT_LOOP;
		break;
	case 1:
		new_mode = EXCEP_OPTION_DUMP_FLASH_RESET;
		break;
	case 2:
		new_mode = EXCEP_OPTION_DUMP_FLASH_EPAT_RESET;
		break;
	default:
		return;
	}
	BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION, new_mode);
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION) == EXCEP_OPTION_SILENT_RESET)
        ResetLockupCfg(true, true);
    else
        ResetLockupCfg(false, false);
}
