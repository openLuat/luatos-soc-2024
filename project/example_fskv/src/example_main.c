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


//fskv空间可以用程序区内空余空间，也可以使用fdb的空间（不能和老的kv兼容了）

#include "common_api.h"

#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_fskv.h"

static void kv_demo(void *param)
{
	int ret = 0;
	char value[128] = {0};
	char value2[128] = {0};
	/* 
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
	luat_fskv_init();


	LUAT_DEBUG_PRINT("luat_kv demo");

	// 设置一个key-value
	// key 最大长度是   64
	// value 最大长度是 4096

	// 设置方式, \0结尾的数据,直接来
	ret = luat_fskv_set("my123", "123", strlen("123"));
	LUAT_DEBUG_PRINT("kv_set my123 ret %d", ret);
	memset(value, 0x21, 32);
	ret = luat_fskv_set("my456", value, 32);
	LUAT_DEBUG_PRINT("kv_set my456 ret %d", ret);

	ret = luat_fskv_get("my123", value, 32);
	LUAT_DEBUG_PRINT("kv_get ret %d", ret);
	if (ret > 0) {
		value[ret] = 0x00;
		LUAT_DEBUG_PRINT("kv read value %s", value);
		// 写进入的值应该等于写出的值
		if (memcmp("123", value, strlen("123"))) {
			LUAT_DEBUG_PRINT("kv value NOT match");
		}
	}
	else {
		// 前面的逻辑写入字符串"123", 获取时肯定大于0,除非底层出错了
		LUAT_DEBUG_PRINT("kv read failed");
	}

	for(ret = 0; ret < 128; ret++)
	{
		value2[ret] = ret;
	}
	ret = luat_fskv_set("hextest", value2, 128);
	ret = luat_fskv_get("hextest", value, 128);

	if (memcmp(value, value2, 128))
	{
		LUAT_DEBUG_PRINT("测试失败");
	}

	luat_rtos_task_sleep(1000);
	// 清空整个kv数据, 慎用
	// 这里只是演示有这个API, 并非使用后要调用该API
//	luat_fskv_clear();
	
	// 演示结束, task退出.
	while (1) {
		luat_rtos_task_sleep(1000);
	}
}

static void task_demo_kv(void)
{
	
	luat_rtos_task_handle handle;
#ifdef TYPE_EC716S
	// EC716S系列模组不支持这个示例
#else
	luat_rtos_task_create(&handle, 8*1024, 50, "kv", kv_demo, NULL, 0);
#endif
}

INIT_TASK_EXPORT(task_demo_kv, "1");

