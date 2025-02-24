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


#include "luat_base.h"
#include "luat_spi.h"

#include "common_api.h"
#include <stdio.h>
#include <string.h>
#include "bsp_custom.h"
#include "soc_spi.h"
#include "driver_gpio.h"
#include "soc_service.h"
#include "mem_map.h"
#include "platform_define.h"
static uint8_t g_s_luat_spi_mode[SPI_MAX] ={0};

static int spi_exist(int id) {
	if (id < SPI_MAX) return 1;
    return 0;
}

static void spi_psram_dma_on_check(uint32_t tx_buf, uint32_t rx_buf)
{
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	uint8_t psram = 0;
	if ((tx_buf > PSRAM_START_ADDR) && (tx_buf < PSRAM_END_ADDR))
	{
		psram = 1;
	}
	if ((rx_buf > PSRAM_START_ADDR) && (rx_buf < PSRAM_END_ADDR))
	{
		psram = 1;
	}
	if (psram)
	{
		soc_psram_dma_on_off(SOC_SYS_CTRL_SPI, 1);
	}
#endif
}

static void spi_psram_dma_off_check(uint32_t tx_buf, uint32_t rx_buf)
{
#if defined (PSRAM_FEATURE_ENABLE) && (PSRAM_EXIST==1)
	uint8_t psram = 0;
	if ((tx_buf > PSRAM_START_ADDR) && (tx_buf < PSRAM_END_ADDR))
	{
		psram = 1;
	}
	if ((rx_buf > PSRAM_START_ADDR) && (rx_buf < PSRAM_END_ADDR))
	{
		psram = 1;
	}
	if (psram)
	{
		soc_psram_dma_on_off(SOC_SYS_CTRL_SPI, 0);
	}
#endif
}

#ifdef __LUATOS__
#include "luat_lcd.h"
#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */
#endif

int luat_spi_device_config(luat_spi_device_t* spi_dev) {
    if (!spi_exist(spi_dev->bus_id))
        return -1;
    uint8_t spi_mode = SPI_MODE_0;
    if(spi_dev->spi_config.CPHA&&spi_dev->spi_config.CPOL)spi_mode = SPI_MODE_3;
    else if(spi_dev->spi_config.CPOL)spi_mode = SPI_MODE_2;
    else if(spi_dev->spi_config.CPHA)spi_mode = SPI_MODE_1;
    SPI_SetNewConfig(spi_dev->bus_id, spi_dev->spi_config.bandrate, spi_mode);
    return 0;
}

int luat_spi_bus_setup(luat_spi_device_t* spi_dev){
    if (!spi_exist(spi_dev->bus_id))
        return -1;
    uint8_t spi_mode = SPI_MODE_0;
	if(spi_dev->spi_config.CPHA&&spi_dev->spi_config.CPOL)spi_mode = SPI_MODE_3;
	else if(spi_dev->spi_config.CPOL)spi_mode = SPI_MODE_2;
	else if(spi_dev->spi_config.CPHA)spi_mode = SPI_MODE_1;
#ifdef CHIP_EC716
	GPIO_IomuxEC7XX(15, 1, 1, 0);
	GPIO_IomuxEC7XX(16, 1, 1, 0);
	GPIO_IomuxEC7XX(17, 1, 1, 0);
#else
	if (spi_dev->bus_id)
	{
		GPIO_IomuxEC7XX(28, 1, 1, 0);
		GPIO_IomuxEC7XX(29, 1, 1, 0);
		GPIO_IomuxEC7XX(30, 1, 1, 0);
	}
	else
	{
		GPIO_IomuxEC7XX(24, 1, 1, 0);
		GPIO_IomuxEC7XX(25, 1, 1, 0);
		GPIO_IomuxEC7XX(26, 1, 1, 0);
	}
#endif
	g_s_luat_spi_mode[spi_dev->bus_id] = spi_dev->spi_config.mode;
	SPI_MasterInit(spi_dev->bus_id, spi_dev->spi_config.dataw, spi_mode, spi_dev->spi_config.bandrate, NULL, NULL);
	return 0;
}

int luat_spi_change_speed(int spi_id, uint32_t speed)
{
    if (!spi_exist(spi_id)) {
        return -1;
	}
	SPI_SetNewConfig(spi_id, speed, 0xff);
	return 0;
}

int luat_spi_get_mode(int spi_id) {
    if (!spi_exist(spi_id)) {
        return -1;
	}
    return g_s_luat_spi_mode[spi_id];
}

int luat_spi_set_mode(int spi_id, uint8_t mode) {
    if (!spi_exist(spi_id)) {
        return -1;
	}
    g_s_luat_spi_mode[spi_id] = mode;
	return 0;
}

int luat_spi_setup(luat_spi_t* spi) {
    if (!spi_exist(spi->id)) {
        return -1;
	}
    uint8_t spi_mode = SPI_MODE_0;
    if(spi->CPHA&&spi->CPOL)spi_mode = SPI_MODE_3;
    else if(spi->CPOL)spi_mode = SPI_MODE_2;
    else if(spi->CPHA)spi_mode = SPI_MODE_1;
#ifdef CHIP_EC716
	if (HAL_GPIO_2 == spi->cs)
	{
		GPIO_IomuxEC7XX(14, 1, 1, 0);
	}
	GPIO_IomuxEC7XX(15, 1, 1, 0);
	GPIO_IomuxEC7XX(16, 1, 1, 0);
	GPIO_IomuxEC7XX(17, 1, 1, 0);
#else
	if (spi->id)
	{
		if (HAL_GPIO_12 == spi->cs)
		{
			GPIO_IomuxEC7XX(27, 1, 1, 0);
		}
		GPIO_IomuxEC7XX(28, 1, 1, 0);
		GPIO_IomuxEC7XX(29, 1, 1, 0);
		GPIO_IomuxEC7XX(30, 1, 1, 0);
	}
	else
	{
		if (HAL_GPIO_8 == spi->cs)
		{
			GPIO_IomuxEC7XX(23, 1, 1, 0);
		}
		GPIO_IomuxEC7XX(24, 1, 1, 0);
		GPIO_IomuxEC7XX(25, 1, 1, 0);
		GPIO_IomuxEC7XX(26, 1, 1, 0);
	}
#endif
	g_s_luat_spi_mode[spi->id] = spi->mode;
	if (spi->master)
	{
		SPI_MasterInit(spi->id, spi->dataw, spi_mode, spi->bandrate, NULL, NULL);
	}
	else
	{
		SPI_SlaveInit(spi->id, spi->dataw, spi_mode, spi->bandrate, NULL, NULL);
	}
    return 0;
}

//关闭SPI，成功返回0
int luat_spi_close(int spi_id) {
    return 0;
}

//收发SPI数据，返回接收字节数
int luat_spi_transfer(int spi_id, const char* send_buf, size_t send_length, char* recv_buf, size_t recv_length) {
    if (!spi_exist(spi_id)) {
        return -1;
	}
    int ret = 0;
    spi_psram_dma_on_check((uint32_t)send_buf, (uint32_t)recv_buf);
    if(g_s_luat_spi_mode[spi_id])
    {
    	if (SPI_BlockTransfer(spi_id, send_buf, recv_buf, recv_length))
    	{

    	}
    	else
    	{
    		ret = recv_length;
    	}
    }
    else
    {
    	if (SPI_FlashBlockTransfer(spi_id, send_buf, send_length, recv_buf, recv_length))
    	{

    	}
    	else
    	{

    		ret =  recv_length;
    	}
    }
    spi_psram_dma_off_check((uint32_t)send_buf, (uint32_t)recv_buf);
    return ret;
}

//收SPI数据，返回接收字节数
int luat_spi_recv(int spi_id, char* recv_buf, size_t length) {
    if (!spi_exist(spi_id)) {
        return -1;
	}
//    if (SPI_GetSpeed(spi_id) > 12800000)
//    {
//    	SPI_SetDMAEnable(spi_id, 0);
//    }
    spi_psram_dma_on_check(0, (uint32_t)recv_buf);
    if (SPI_BlockTransfer(spi_id, recv_buf, recv_buf, length))
    {
//        if (SPI_GetSpeed(spi_id) > 12800000)
//        {
//        	SPI_SetDMAEnable(spi_id, 1);
//        }
    	spi_psram_dma_off_check(0, (uint32_t)recv_buf);
    	return 0;
    }
    else
    {
//        if (SPI_GetSpeed(spi_id) > 12800000)
//        {
//        	SPI_SetDMAEnable(spi_id, 1);
//        }
    	spi_psram_dma_off_check(0, (uint32_t)recv_buf);
    	return length;
    }
}
//发SPI数据，返回发送字节数
int luat_spi_send(int spi_id, const char* send_buf, size_t length) {
    if (!spi_exist(spi_id)) {
        return -1;
	}
    spi_psram_dma_on_check((uint32_t)send_buf, 0);
    if (SPI_BlockTransfer(spi_id, send_buf, NULL, length))
    {
    	spi_psram_dma_off_check((uint32_t)send_buf, 0);
    	return 0;
    }
    else
    {
    	spi_psram_dma_off_check((uint32_t)send_buf, 0);
    	return length;
    }
}

int luat_spi_no_block_transfer(int spi_id, uint8_t *tx_buff, uint8_t *rx_buff, size_t len, void *CB, void *pParam)
{
	if (SPI_IsTransferBusy(spi_id)) {
        return -1;
	}
	SPI_SetCallbackFun(spi_id, CB, pParam);
	SPI_SetNoBlock(spi_id);
	return SPI_TransferEx(spi_id, tx_buff, rx_buff, len, 0, 1);
}

int luat_spi_set_slave_callback(int spi_id, luat_spi_irq_callback_t callback, void *user_data)
{
    if (!spi_exist(spi_id)) {
        return -1;
	}
    SPI_SetCallbackFun(spi_id, (CBFuncEx_t)callback, user_data);
    return 0;
}

int luat_spi_slave_transfer(int spi_id, const char* send_buf,  char* recv_buf, size_t total_length)
{
    if (!spi_exist(spi_id)) {
        return -1;
	}
    return SPI_SlaveTransferStart(spi_id, send_buf, recv_buf, total_length);
}

int luat_spi_slave_transfer_pause_and_read_data(int spi_id)
{
    if (!spi_exist(spi_id)) {
        return -1;
	}
    return SPI_SlaveTransferStopAndGetRxLen(spi_id);
}

__USER_FUNC_IN_RAM__ int luat_spi_slave_transfer_pause_in_irq(int spi_id)
{
	SPI_SlaveFastStop(spi_id);
	return 0;
}

__USER_FUNC_IN_RAM__ int luat_spi_slave_fast_transfer_in_irq(int spi_id, const char* send_buf,  char* recv_buf, size_t total_length)
{
	SPI_SlaveStartNextFast(spi_id, send_buf, recv_buf, total_length);
	return 0;
}

__USER_FUNC_IN_RAM__ int luat_spi_slave_transfer_fast_pause_and_read_data_in_irq(int spi_id)
{
	return SPI_SlaveGetRxLenFast(spi_id);
}

int luat_spi_slave_transfer_stop(int spi_id)
{
    if (!spi_exist(spi_id)) {
        return -1;
	}
    SPI_TransferStop(spi_id);
    return 0;
}
