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
#include "luat_base.h"
#include "luat_fs.h"

luat_rtos_task_handle task1_handle;

static void task1(void *param)
{
	luat_rtos_task_sleep(3000);
	luat_fs_init(); // 必须先初始化   
    luat_fs_info_t fs_info = {0};

    luat_fs_info("/", &fs_info);
    // 打印文件系统空间信息
    LUAT_DEBUG_PRINT("fs_info %s %d %d %d %d", 
        fs_info.filesystem, 
        fs_info.type, 
        fs_info.total_block, 
        fs_info.block_used, 
        fs_info.block_size);
	while(1)
	{
		luat_rtos_task_sleep(10000);
		LUAT_DEBUG_PRINT("task1 loop");
	}
}


static void task_demoE_init(void)
{
	luat_rtos_task_create(&task1_handle, 2*1024, 50, "task1", task1, NULL, 0);
}

// 启动task_demoE_init，启动位置任务1级
INIT_TASK_EXPORT(task_demoE_init, "1");

