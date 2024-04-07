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
#include "luat_debug.h"
#include "luat_mcu.h"
#include "luat_i2c.h"
#include "luat_gpio.h"

luat_rtos_task_handle i2c_task_handle;

#define I2C_ID                  1
#define AHT10_ADDRESS_ADR_LOW   0x38

#define AHT10_INIT              0xE1 //初始化命令
#define AHT10_MEASURE           0xAC //触发测量命令
#define AHT10_SOFT_RESET        0xBA
#define AHT10_STATE             0x71 //状态字.

#define SHT30_ADDRESS	0x44


#define I2C_ID_da221 0
#define DA213B_ADDRESS 0x27
#define Motion_Sensor_Pin HAL_GPIO_12

static void task_test_aht10(void *param)
{
    char soft_reset[] = {AHT10_SOFT_RESET}; 
    char init_cmd[] = {AHT10_INIT,0x08,0x00}; 
    char measure_cmd[] = {AHT10_MEASURE, 0x33, 0x00}; 
    char read_cmd[] = {AHT10_STATE}; 
    char recv_date[7] = {0};
    luat_i2c_setup(I2C_ID,1);
    luat_rtos_task_sleep(40);
    luat_i2c_send(I2C_ID, AHT10_ADDRESS_ADR_LOW, soft_reset, 1, 1);
    luat_rtos_task_sleep(20);

    luat_i2c_recv(I2C_ID, AHT10_ADDRESS_ADR_LOW, recv_date, 1);

    if (recv_date[0]&(1<<3) == 0){
        luat_i2c_send(I2C_ID, AHT10_ADDRESS_ADR_LOW, init_cmd, 3, 1);
    }
    while (1)
    {
        luat_rtos_task_sleep(1000);
        luat_i2c_send(I2C_ID, AHT10_ADDRESS_ADR_LOW, measure_cmd, 3, 1);
        luat_rtos_task_sleep(80);
        luat_i2c_send(I2C_ID, AHT10_ADDRESS_ADR_LOW, read_cmd, 1, 1);
        luat_i2c_recv(I2C_ID, AHT10_ADDRESS_ADR_LOW, recv_date, 6);

        float cur_temp = ((recv_date[3] & 0xf) << 16 | recv_date[4] << 8 | recv_date[5]) * 200.0 / (1 << 20) - 50;
        LUAT_DEBUG_PRINT("temp: %f",cur_temp);

    }
    
}

static void task_test_sht30(void *param)
{
	uint8_t measure_cmd[] = {0x2c, 0x0d};
	uint8_t recv_date[7] = {0};
	uint8_t crc1,crc2;
	uint32_t t,r;
	double te,hu;
	int result;
	luat_i2c_setup(I2C_ID,1);
	luat_rtos_task_sleep(40);
	while(1)
	{
		result = luat_i2c_send(I2C_ID, SHT30_ADDRESS, measure_cmd, 2, 1);
		luat_rtos_task_sleep(1000);
		if (!result)
		{
			result = luat_i2c_recv(I2C_ID, SHT30_ADDRESS, recv_date, 6);
			if (!result)
			{
				crc1 = CRC8Cal(recv_date, 2, 0xff, 0x31, 0);
				crc2 = CRC8Cal(recv_date + 3, 2, 0xff, 0x31, 0);
				if ((recv_date[2] == crc1) && (recv_date[5] == crc2))
				{
					t = BytesGetBe16(recv_date);
					r = BytesGetBe16(recv_date);
					te = t * 175.0 / 65535.0 - 45.0;
					hu = r * 100.0 /65535.0;
					LUAT_DEBUG_PRINT("温度 %f，湿度 %f", te, hu);
				}
				else
				{
					LUAT_DEBUG_PRINT("校验失败 %x,%x,%x,%x", recv_date[2], crc1, recv_date[5], crc2);
				}

			}
		}
	}

}

int gpio_irq(int pin, void *args)//ACC，GS 运动传感器中断回调
{
	
	if (pin == Motion_Sensor_Pin)//GS中断传感器回调
	{
		LUAT_DEBUG_PRINT("enter motion sensor input");
	}
}


/// @brief 获取运动标识
/// @param  
/// @return 1 正在运动 0 不运动
static uint8_t luat_get_motion(void)
{
	char motionaddr[] = {0x09};
	char moti_data[1] = {0};
	
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, motionaddr, 1, 1); // 从0x02开始读取三轴速度
	luat_i2c_recv(I2C_ID_da221, DA213B_ADDRESS, moti_data, 1);
	return ((moti_data[0] & 0x04) >> 2);
}

static void da221_task_proc(void *arg)
{
	char recv_data_xyz[6] = {0};
	uint8_t acc_x;
	uint8_t acc_y;
	uint8_t acc_z;

	char soft_reset[2]={0x00,0x24};//软件复位地址
	char chipidaddr[] = {0x01};//芯片chipid
	char recv_chipid_data[1] = {0};
   	char rangeaddr[] = {0x0f, 0x00};//设置加速度量程，默认为2G
	

	char int_set1_reg[] = {0x16, 0x87};//设置x,y,z发生变化时，产生中断
	char int_set2_reg[]={0x17,0x10};//使能新数据中断，数据变化时，产生中断，本程序不设置
	char int_map1_reg[] = {0x19, 0x04};//运动的时候，产生中断
	char int_map2_reg[] = {0x1a, 0x01};//运动的时候，产生中断

	char activeDURaddr[] = {0x27, 0x00};
	char activeTHSaddr[] = {0x28, 0x05};


	char modedddr[] = {0x11, 0x34};
	char ODRaddr[] = {0x10, 0x08};
	char INTlatchaddr[] = {0x21, 0x02};

	char x_lsb_reg[]={0x02};
	

	char active_state[] = {0x0b};
	char active_state_data[1] = {0};

	
	//初始化GS传感器中断
	luat_gpio_cfg_t gpio_cfg;

	// 配置计算器传感器震动中断引脚
	luat_gpio_set_default_cfg(&gpio_cfg);
	//中断使用GPIO12
	gpio_cfg.pin = Motion_Sensor_Pin;
	gpio_cfg.mode = LUAT_GPIO_IRQ;
	gpio_cfg.irq_type = LUAT_GPIO_RISING_IRQ;
	gpio_cfg.pull = LUAT_GPIO_PULLDOWN;
	gpio_cfg.irq_cb = gpio_irq;
	luat_gpio_open(&gpio_cfg);

    luat_i2c_setup(I2C_ID_da221, 1);
	luat_rtos_task_sleep(50);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, soft_reset, 2, 1);
	luat_rtos_task_sleep(50);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, chipidaddr, 1, 1);
	luat_i2c_recv(I2C_ID_da221, DA213B_ADDRESS, recv_chipid_data, 1);
	if (recv_chipid_data[0]==0x13)
	{
		LUAT_DEBUG_PRINT("chip is da213B or da221");
	}
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, int_set1_reg, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, int_map1_reg, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, activeDURaddr, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, activeTHSaddr, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, modedddr, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, ODRaddr, 2, 1);
	luat_rtos_task_sleep(5);
	luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, INTlatchaddr, 2, 1);
	luat_rtos_task_sleep(5);

	while (1)
	{

		luat_i2c_send(I2C_ID_da221, DA213B_ADDRESS, x_lsb_reg, 1, 1);//从0x02开始读取三轴速度
		luat_i2c_recv(I2C_ID_da221, DA213B_ADDRESS, recv_data_xyz, 6);
		luat_rtos_task_sleep(50);
		acc_x=(recv_data_xyz[1]<<8)|recv_data_xyz[0];
		acc_y=(recv_data_xyz[3]<<8)|recv_data_xyz[2];
		acc_z=(recv_data_xyz[5]<<8)|recv_data_xyz[4];

		LUAT_DEBUG_PRINT("acc_x%0.1f",acc_x/9.8);
		LUAT_DEBUG_PRINT("acc_y%0.1f",acc_y/9.8);
		LUAT_DEBUG_PRINT("acc_z%0.1f",acc_z/9.8);
		luat_rtos_task_sleep(1000);
	}
	luat_rtos_task_delete(i2c_task_handle);
}


static void task_demo_i2c(void)
{
//    luat_rtos_task_create(&i2c_task_handle, 2048, 20, "i2c", task_test_aht10, NULL, 0);
    //luat_rtos_task_create(&i2c_task_handle, 2048, 20, "i2c", task_test_sht30, NULL, 0);
	luat_rtos_task_create(&i2c_task_handle, 2048, 20, "i2c", da221_task_proc, NULL, 0);
}

INIT_TASK_EXPORT(task_demo_i2c,"1");



