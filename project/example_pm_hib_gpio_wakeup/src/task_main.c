#include "common_api.h"
#include "luat_rtos.h" //luat 头文件 封装FreeRTOS
#include "luat_debug.h"//luat DBUG 库
#include "luat_gpio.h"//luat GPIO 库
#include "platform_define.h"
#include "pad.h"
#include "luat_pm.h"
#include "luat_timer.h"
#include "luat_mobile.h"


int is_online;
int socket_rx_done;

void gpio_level_irq(void *data, void* args)
{
	int pin = (int)data;
	LUAT_DEBUG_PRINT("pin:%d, level:%d,", pin, luat_gpio_get(pin));
}

void key_init(void)
{
    luat_gpio_cfg_t key_fun_struct;
    luat_gpio_set_default_cfg(&key_fun_struct);
    key_fun_struct.pin=HAL_GPIO_0; // BOOT按钮
    key_fun_struct.pull=Luat_GPIO_PULLUP;
    key_fun_struct.mode=Luat_GPIO_IRQ;
    key_fun_struct.irq_type=LUAT_GPIO_FALLING_IRQ;
    key_fun_struct.irq_cb=(void*)gpio_level_irq;
    luat_gpio_open(&key_fun_struct);
}
/**********************wakeup 中断功能演示******************************/
//wakeup 引脚一般作为中断引脚或者休眠唤醒引脚
void wakeup_pad_init(void)
{
    luat_gpio_cfg_t cfg;
    luat_gpio_set_default_cfg(&cfg);
    cfg.pull=Luat_GPIO_PULLUP;
    cfg.mode=Luat_GPIO_IRQ;
    cfg.irq_type=LUAT_GPIO_BOTH_IRQ;
    cfg.irq_cb=(void*)gpio_level_irq;


    cfg.pin=HAL_WAKEUP_0;
    luat_gpio_open(&cfg);

    cfg.pin=HAL_WAKEUP_1;
    luat_gpio_open(&cfg);

    // cfg.pin=HAL_WAKEUP_2;
    // luat_gpio_open(&cfg);

    // cfg.pin=HAL_WAKEUP_3;
    // luat_gpio_open(&cfg);

    cfg.pin=HAL_WAKEUP_4;
    luat_gpio_open(&cfg);

    // cfg.pin=HAL_WAKEUP_5;
    // luat_gpio_open(&cfg);

    LUAT_DEBUG_PRINT("wakeup0/1/3/4/5 init ok");
}

/**********************wakeup 中断功能演示******************************/


/**********************pwrkey 关机功能演示******************************/
static void main_task(void *param)
{
    int lastState, rtcOrPad;

    LUAT_DEBUG_PRINT("FLASH_HIB_BACKUP_EXIST %d", FLASH_HIB_BACKUP_EXIST);

    // 首先, 把wakeup 引脚初始化
    key_init();
    wakeup_pad_init();

    // 是否需要退出飞行模式呢
    luat_pm_last_state(&lastState, &rtcOrPad);
    if(rtcOrPad > 0 && rtcOrPad != 0xff)        //深度休眠前若进入飞行模式，并且唤醒后需要进行联网操作的，需要退出飞行模式
    {
        //退出飞行模式
        LUAT_DEBUG_PRINT("退出飞行模式==============");
        luat_mobile_set_flymode(0, 0);
    }

    // 然后等待联网
    LUAT_DEBUG_PRINT("等待联网==============");
    while (is_online == 0)
    {
        luat_rtos_task_sleep(100);
    }
    // 等待socket线程收到数据
    LUAT_DEBUG_PRINT("等待socket收到数据==============");
    
    while (socket_rx_done == 0)
    {
        // LUAT_DEBUG_PRINT("等待socket收到数据==============");
        luat_rtos_task_sleep(100);
    }

    // 已经收到数据, 先等5秒
    LUAT_DEBUG_PRINT("服务器数据已收到,5秒后进入休眠==============");
    luat_rtos_task_sleep(5*1000);
    // 1分钟后唤醒 -- 现实场景应该设置在5分钟以上
    // 注意, dtimer id=0/1的最长是2.5小时, 超过2.5小时会截断到2.5小时
    // id=2/3 可以3个月, 精度低一些, 到秒级别
    LUAT_DEBUG_PRINT("时间已到, 关闭USB, 进飞行模式, 进HIB, 1分钟唤醒==============");
    // luat_pm_dtimer_start(0, 1*60*1000);
    luat_pm_dtimer_start(0, 60*1000);
    // 进飞行模式, 和hib
    luat_mobile_set_flymode(0, 1);
    luat_pm_power_ctrl(LUAT_PM_POWER_USB, 0);
    luat_pm_force(LUAT_PM_SLEEP_MODE_STANDBY);
    while (1)
    {
        luat_rtos_task_sleep(60*1000);
        LUAT_DEBUG_PRINT("休眠失败!!! 重启!!");
        luat_os_reboot(0);
    }
    
}

void gpio_interrupt_demo(void)
{
    luat_rtos_task_handle gpio_interrupt_task_handler;
    luat_rtos_task_create(&gpio_interrupt_task_handler,8*1024,50,"main_task",main_task,NULL,0);
}

void soc_hib_config(void){;}


/********************************************GPIO 中断功能     end *******************************************/

INIT_TASK_EXPORT(gpio_interrupt_demo,"3");
