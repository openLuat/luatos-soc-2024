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

#include "luat_debug.h"
#include "common_api.h"
#include "plat_config.h"
#include "reset.h"
static unsigned char g_s_debug_onoff = 1;
extern const uint8_t ByteToAsciiTable[];
extern void soc_assert(const char *fun_name, uint32_t line_no, const char *fmt, va_list ap);
extern void soc_vsprintf(uint8_t no_print, const char *fmt, va_list ap);
extern void soc_printf_onoff(uint8_t no_printf);
extern void soc_debug_out(char *string, uint32_t size);
void luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_MODE_E mode)
{
	uint32_t new = BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION);
	switch(mode)
	{
	case LUAT_DEBUG_FAULT_RESET:
		new = EXCEP_OPTION_SILENT_RESET;
		break;
	case LUAT_DEBUG_FAULT_HANG:
		new = EXCEP_OPTION_DUMP_FLASH_EPAT_LOOP;
		break;
	case LUAT_DEBUG_FAULT_SAVE_RESET:
		new = EXCEP_OPTION_DUMP_FLASH_RESET;
		break;
	default:
		new = EXCEP_OPTION_DUMP_FLASH_EPAT_RESET;
		break;
	}
	BSP_SetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION, new);
    if(BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_FAULT_ACTION) == EXCEP_OPTION_SILENT_RESET)
        ResetLockupCfg(true, true);
    else

        ResetLockupCfg(false, false);
}

void luat_debug_assert(const char *fun_name, unsigned int line_no, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);
	soc_assert(fun_name, line_no, fmt, ap);
	va_end(ap);
}

void luat_debug_print(const char *fmt, ...)
{
	if (!g_s_debug_onoff) return;
	va_list ap;
	va_start(ap, fmt);
	soc_vsprintf(0, fmt, ap);
	va_end(ap);
}

void luat_debug_print_onoff(unsigned char onoff)
{
	soc_printf_onoff(!onoff);
	g_s_debug_onoff = onoff;
}

void luat_debug_dump(uint8_t *data, uint32_t len)
{
	if (!len) return;
	char *uart_buf = malloc(len * 3);
	if (uart_buf)
	{
		uint32_t i,j = 0;
	    for (i = 0; i < len; i++){
			uart_buf[j++] = ByteToAsciiTable[(data[i] & 0xf0) >> 4];
			uart_buf[j++] = ByteToAsciiTable[data[i] & 0x0f];
			uart_buf[j++] = ' ';
	    }
	    soc_debug_out(uart_buf, len * 3);
		free(uart_buf);
	}
}
