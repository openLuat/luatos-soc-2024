#include "common_api.h"
#include "luat_rtos.h" //luat 头文件 封装FreeRTOS
#include "luat_debug.h"//luat DBUG 库
#include "luat_gpio.h"//luat GPIO 库
#include "platform_define.h"
#include "pad.h"
#include "luat_mobile.h"

static void gpio_fun_task(void *param)
{
	luat_gpio_cfg_t gpio = {0};
    gpio.pin = 15;
    gpio.pull = Luat_GPIO_PULLUP;
    gpio.output_level = 1;
    luat_gpio_open(&gpio);
    uint8_t read_val = 0;
    uint8_t write_val = 0;
    int result;
    luat_mobile_set_flymode(0, 1);	//建议在飞行模式下操作YHM27XX
    while (1)
    {
    	result = luat_gpio_driver_yhm27xx(gpio.pin, 0x04, 0x08, 1, &read_val);
    	if (!result && read_val == 0xa0)
    	{
    		result = luat_gpio_driver_yhm27xx(gpio.pin, 0x04, 0x00, 1, &read_val);
    		if (!result)
    		{
    			LUAT_DEBUG_PRINT("初始值,%x", read_val);
    		}
    		write_val = 0x04;
    		result = luat_gpio_driver_yhm27xx(gpio.pin, 0x04, 0x00, 0, &write_val);
    		if (!result)
    		{
        		result = luat_gpio_driver_yhm27xx(gpio.pin, 0x04, 0x00, 1, &read_val);
        		if (read_val != write_val)
        		{
        			LUAT_DEBUG_PRINT("写入失败,%x,%x", read_val, write_val);
        		}
        		else
        		{
        			LUAT_DEBUG_PRINT("测试成功");
        		}
        		write_val = 0;
        		result = luat_gpio_driver_yhm27xx(gpio.pin, 0x04, 0x00, 0, &write_val);
    		}
    		else
    		{
    			LUAT_DEBUG_PRINT("读取失败");
    		}

    	}
    	else
    	{
    		LUAT_DEBUG_PRINT("yhm27xx不存在");
    	}
    	luat_rtos_task_sleep(1000);
    }
    
}

void gpio_fun_demo(void)
{
    luat_rtos_task_handle gpio_fun_task_handler;
    luat_rtos_task_create(&gpio_fun_task_handler,4*1024,50,"gpio_fun_task",gpio_fun_task,NULL,0);
}

INIT_TASK_EXPORT(gpio_fun_demo,"2");

