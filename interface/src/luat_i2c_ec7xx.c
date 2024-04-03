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


#include "luat_i2c.h"
#include "common_api.h"
#include "soc_i2c.h"
#include "driver_gpio.h"
#include "luat_mcu.h"
#include "gpr_common.h"
static uint32_t luat_i2c_global_timeout = 100;
//static uint8_t luat_i2c_iomux[I2C_MAX];

int luat_i2c_exist(int id) {
    return (id < I2C_MAX);
}

int luat_i2c_set_polling_mode(int id, uint8_t on_off)
{
	if (!luat_i2c_exist(id)) return -1;
    I2C_UsePollingMode(id, on_off);
    return 0;
}

//int luat_i2c_set_iomux(int id, uint8_t value)
//{
//	if (!luat_i2c_exist(id)) return -1;
//	luat_i2c_iomux[id] = value;
//}

int luat_i2c_setup(int id, int speed) {
    if (speed == 0) {
        speed = 100 * 1000; // SLOW
    }
    else if (speed == 1) {
        speed = 400 * 1000; // FAST
    }
    else if (speed == 2) {
        speed = 1000 * 1000; // SuperFast
    }
    if (!luat_i2c_exist(id)) return -1;
    if (luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_I2C, id))
    {
#if defined TYPE_EC716S
		if (id)
    	{
    		GPIO_IomuxEC7XX(10, 1, 1, 0);
    		GPIO_IomuxEC7XX(11, 1, 1, 0);
    	}
    	else
    	{
    		GPIO_IomuxEC7XX(8, 1, 1, 0);
    		GPIO_IomuxEC7XX(9, 1, 1, 0);
    	}
#else
    	if (id)
    	{
    		GPIO_IomuxEC7XX(13, 3, 1, 0);
    		GPIO_IomuxEC7XX(14, 3, 1, 0);
    	}
    	else
    	{
    		GPIO_IomuxEC7XX(29, 2, 1, 0);
    		GPIO_IomuxEC7XX(30, 2, 1, 0);
    	}
#endif
    }
	I2C_MasterSetup(id, speed);
    return 0;
}

int luat_i2c_close(int id) {
    if (!luat_i2c_exist(id)) return -1;
    if (luat_mcu_iomux_is_default(LUAT_MCU_PERIPHERAL_I2C, id))
    {
    	if (id)
    	{
    		GPIO_IomuxEC7XX(13, 0, 0, 0);
    		GPIO_IomuxEC7XX(14, 0, 0, 0);
    	}
    	else
    	{
    		GPIO_IomuxEC7XX(29, 0, 0, 0);
    		GPIO_IomuxEC7XX(30, 0, 0, 0);
    	}
    }
    return 0;
}

void luat_i2c_set_global_timeout(uint32_t time)
{
	luat_i2c_global_timeout = time;
}

static void i2c_failed(int id, int addr, int code)
{
	switch(code)
	{
	case -ERROR_NO_SUCH_ID:
		DBG("i2c%d 从机地址%x 无应答", id, addr);
		break;
	case -ERROR_TIMEOUT:
		DBG("i2c%d 从机地址%x 传输超时", id, addr);
		break;
	default:
		DBG("i2c%d 从机地址%x 传输过程发生异常，错误码%d", id, addr, code);
		break;
	}
}

int luat_i2c_send(int id, int addr, void* buff, size_t len, uint8_t stop) {
	if (!luat_i2c_exist(id)) return -1;
	int result = I2C_BlockWrite(id, addr, (const uint8_t *)buff, len, luat_i2c_global_timeout, NULL, NULL);
	if (result)
	{
		i2c_failed(id, addr, result);
	}
	return result;
}

int luat_i2c_recv(int id, int addr, void* buff, size_t len) {
	if (!luat_i2c_exist(id)) return -1;
	int result = I2C_BlockRead(id, addr, 0, 0, (uint8_t *)buff, len, luat_i2c_global_timeout, NULL, NULL);
	if (result)
	{
		i2c_failed(id, addr, result);
	}
	return result;
}

int luat_i2c_transfer(int id, int addr, uint8_t *reg, size_t reg_len, uint8_t *buff, size_t len) {
	if (!luat_i2c_exist(id)) return -1;
	int result;
	if (reg && reg_len) {
		result = I2C_BlockRead(id, addr, reg, reg_len, (uint8_t *)buff, len, luat_i2c_global_timeout, NULL, NULL);
	} else {
		result = I2C_BlockWrite(id, addr, (const uint8_t *)buff, len, luat_i2c_global_timeout, NULL, NULL);
	}
	if (result)
	{
		i2c_failed(id, addr, result);
	}
	return result;
}
extern void I2C_SetNoBlock(uint8_t I2CID);
int luat_i2c_no_block_transfer(int id, int addr, uint8_t is_read, uint8_t *reg, size_t reg_len, uint8_t *buff, size_t len, uint16_t Toms, void *CB, void *pParam) {
	if (!luat_i2c_exist(id)) return -1;
	int32_t Result;
	if (!I2C_WaitResult(id, &Result)) {
		return -1;
	}
	if (Result)
	{
		I2C_Reset(id);
	}
	I2C_Prepare(id, addr, 2, CB, pParam);
	I2C_SetNoBlock(id);
	if (reg && reg_len)
	{
		I2C_MasterXfer(id, I2C_OP_READ_REG, reg, reg_len, buff, len, Toms);
	}
	else if (is_read)
	{
		I2C_MasterXfer(id, I2C_OP_READ, NULL, 0, buff, len, Toms);
	}
	else
	{
		I2C_MasterXfer(id, I2C_OP_WRITE, NULL, 0, buff, len, Toms);
	}
	return 0;
}


