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
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"

#include "iconv.h"
#include "luat_iconv.h"

luat_rtos_task_handle task_handle;


#define OUT_BUF_LEN 256
static void demo_init_iconv()
{
	LUAT_DEBUG_PRINT("演示GBK转UTF8");
	//支付宝收款200元，感谢使用的GBK编码数据，为了防止源码的编码方式带来的影响，直接使用数组
	char org_string[] = {0xD6,0xA7,0xB8,0xB6,0xB1,0xA6,0xCA,0xD5,0xBF,0xEE,0x32,0x30,0x30,0xD4,0xAA,0xA3,0xAC,0xB8,0xD0,0xD0,0xBB,0xCA,0xB9,0xD3,0xC3,0x00};
	char *input = org_string; //不能直接传数组，必须传指针
	char buf1[OUT_BUF_LEN] = {0};
	char buf2[OUT_BUF_LEN] = {0};
	size_t input_len = sizeof(org_string);
	size_t out_buffer_rest_len = OUT_BUF_LEN;
	luat_iconv_t cd = luat_iconv_open("ucs2", "gb2312");
	if (cd == (void *)-1)
	{
		LUAT_DEBUG_PRINT("iconv not support");
		return;
	}
	char *ucs2_out = buf1;	//不能直接传数组，必须传指针
	char *utf8_out = buf2;  //不能直接传数组，必须传指针
	int result = luat_iconv_convert(cd, &input, &input_len, &ucs2_out, &out_buffer_rest_len);
	luat_iconv_close(cd);
	if (result)
	{
		LUAT_DEBUG_PRINT("The convert failed %d", result);
		return;
	}
	input_len = OUT_BUF_LEN - out_buffer_rest_len;
	LUAT_DEBUG_PRINT("ucs2 len%d", input_len);
	cd = luat_iconv_open("utf8", "ucs2");
	out_buffer_rest_len = OUT_BUF_LEN;
	result = luat_iconv_convert(cd, &ucs2_out, &input_len, &utf8_out, &out_buffer_rest_len);
	luat_iconv_close(cd);
	if (result)
	{
		LUAT_DEBUG_PRINT("The convert failed %d", result);
		return;
	}
	LUAT_DEBUG_PRINT("转换结果, %.*s", OUT_BUF_LEN - out_buffer_rest_len, utf8_out);
}

static void task(void *param)
{
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	while(1)
	{
		demo_init_iconv();
		luat_rtos_task_sleep(1000);
		LUAT_DEBUG_PRINT("==================iconv is done==================");
	}
}


static void task_demoE_init(void)
{
	luat_rtos_task_create(&task_handle, 4*1024, 50, "task", task, NULL, 0);
}

INIT_TASK_EXPORT(task_demoE_init, "1");
