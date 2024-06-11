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
#include "luat_mcu.h"
#include "luat_malloc.h"
#include "luat_spi.h"
#include "soc_spi.h"
#include "luat_fota.h"
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_gpio.h"
#include "platform_define.h"

#ifdef __LUATOS__
#define LUAT_LOG_TAG "fota"
#include "luat_log.h"
#else
#define LLOGD DBG
#define LLOGE DBG
#define LLOGI DBG
#endif

#ifdef LUAT_USE_TLS
#include "mbedtls/md5.h"
#else
typedef struct win32_md5_context
{
	uint32_t total[2];          /*!< number of bytes processed  */
	uint32_t state[4];          /*!< intermediate digest state  */
	unsigned char buffer[64];   /*!< data block being processed */
} win32_md5_context;
extern int win32_md5_starts_ret(win32_md5_context *ctx);
extern int win32_md5_update_ret(win32_md5_context *ctx,
	const unsigned char *input,
	size_t ilen);
extern int win32_md5_finish_ret(win32_md5_context *ctx,
	unsigned char output[16]);
#endif

#include "fota_nvm.h"
#include "mem_map.h"
#include "flash_rt.h"

#ifdef __LUATOS__
#if (FULL_OTA_SAVE_ADDR < 0xe0000000)
#undef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR	LUA_SCRIPT_OTA_ADDR
#endif
#endif

#define __SOC_OTA_COMMON_DATA_LOAD_ADDRESS__	(FULL_OTA_SAVE_ADDR + AP_FLASH_XIP_ADDR)
#define __SOC_OTA_COMMON_DATA_SAVE_ADDRESS__	(FULL_OTA_SAVE_ADDR)
#define __SOC_OTA_SDK_DATA_LOAD_ADDRESS__	(FLASH_FOTA_REGION_START + AP_FLASH_XIP_ADDR)
#define __SOC_OTA_SDK_DATA_SAVE_ADDRESS__	(FLASH_FOTA_REGION_START)
#define __SOC_OTA_COMMON_DATA_LOAD_ADDRESS_LIMIT__	(AP_FLASH_LOAD_ADDR + AP_FLASH_LOAD_SIZE)

#ifndef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR              (0x0)
#endif

#ifdef LUAT_USE_TLS
#if MBEDTLS_VERSION_NUMBER >= 0x03000000
#define mbedtls_md5_starts_ret mbedtls_md5_starts
#define mbedtls_md5_update_ret mbedtls_md5_update
#define mbedtls_md5_finish_ret mbedtls_md5_finish
#endif
typedef mbedtls_md5_context	MD5_CTX;
#define MD5_START	mbedtls_md5_starts_ret
#define MD5_UPDATE	mbedtls_md5_update_ret
#define MD5_FINISH	mbedtls_md5_finish_ret
#else
typedef win32_md5_context	MD5_CTX;
#define MD5_START	win32_md5_starts_ret
#define MD5_UPDATE	win32_md5_update_ret
#define MD5_FINISH	win32_md5_finish_ret
#endif

typedef struct
{
	Buffer_Struct data_buffer;
	MD5_CTX *md5_ctx;
	CoreUpgrade_FileHeadCalMD5Struct *p_fota_file_head;
	uint32_t start_address;
	uint32_t *crc32_table;
	uint32_t ota_done_len;
	uint32_t spi_flash_erase_point;
	uint8_t ota_state;
	uint8_t ota_type;
	uint8_t spi_id;
	uint8_t cs_pin;
	uint8_t power_pin;
	uint8_t is_inited;
}luat_fota_ctrl_t;

enum
{
	OTA_STATE_IDLE,
	OTA_STATE_WRITE_COMMON_DATA,
	OTA_STATE_WRITE_SDK_DATA,
	OTA_STATE_OK,
	OTA_STATE_ERROR,
};

static luat_fota_ctrl_t g_s_fota;

#ifdef LUAT_USE_TLS
#if MBEDTLS_VERSION_NUMBER >= 0x03000000
int mbedtls_sha256_starts_ret( mbedtls_sha256_context *ctx, int is224 ) {
	mbedtls_sha256_starts(ctx, is224);
}
int mbedtls_sha256_update_ret( mbedtls_sha256_context *ctx, const unsigned char *input, size_t ilen ) {
	mbedtls_sha256_update(ctx, input, ilen);
}
int mbedtls_sha256_finish_ret( mbedtls_sha256_context *ctx, unsigned char output[32]) {
	mbedtls_sha256_finish(ctx, output);
}
#endif
#endif

static void luat_fota_finish(void)
{
	CoreUpgrade_HeadCalMD5Struct Head = {0};
	PV_Union uPV;
	Head.MaigcNum = __APP_START_MAGIC__;
	Head.DataStartAddress = g_s_fota.start_address?g_s_fota.start_address:__SOC_OTA_COMMON_DATA_LOAD_ADDRESS__;
	uPV.u8[0] = g_s_fota.ota_type;
	uPV.u8[1] = g_s_fota.start_address?CORE_OTA_OUT_SPI_FLASH:CORE_OTA_IN_FLASH;
	uPV.u8[2] = g_s_fota.spi_id;
	uPV.u8[3] = g_s_fota.cs_pin;
	Head.Param1 = uPV.u32;
	uPV.u8[0] = g_s_fota.power_pin;
	uPV.u8[1] = 0;
	uPV.u8[2] = g_s_fota.spi_id;
	uPV.u8[3] = g_s_fota.cs_pin;
	Head.Param2 = uPV.u32;
	Head.DataLen = g_s_fota.p_fota_file_head->CommonDataLen;
	memcpy(Head.CommonMD5, g_s_fota.p_fota_file_head->CommonMD5, 16);
	Head.CRC32 = CRC32_Cal(g_s_fota.crc32_table, (uint8_t *)&Head.Param1, sizeof(Head) - 8, 0xffffffff);

	if (FULL_OTA_SAVE_ADDR)
	{
		FLASH_eraseSafe(__SOC_OTA_INFO_DATA_SAVE_ADDRESS__, __FLASH_SECTOR_SIZE__);
		FLASH_writeSafe((uint8_t *)&Head, __SOC_OTA_INFO_DATA_SAVE_ADDRESS__, sizeof(Head));
	}

	g_s_fota.ota_state = OTA_STATE_OK;
	luat_heap_free(g_s_fota.crc32_table);
	g_s_fota.crc32_table = NULL;
	luat_heap_free(g_s_fota.md5_ctx);
	g_s_fota.md5_ctx = NULL;
	luat_heap_free(g_s_fota.p_fota_file_head);
	g_s_fota.p_fota_file_head = NULL;
	OS_DeInitBuffer(&g_s_fota.data_buffer);
	g_s_fota.start_address = 0;
	g_s_fota.is_inited = 0;
	LLOGI("fota type %d ok!, wait reboot", g_s_fota.ota_type);
}

static int luat_fota_check_spi_flash(void)
{
	uint8_t tx[4] = {0x9f,0};
	uint8_t rx[4] = {0};
	for(int i = 0; i < 3; i++)
	{
		luat_gpio_set(g_s_fota.cs_pin, 0);
		SPI_BlockTransfer(g_s_fota.spi_id, tx, rx, 4);
		luat_gpio_set(g_s_fota.cs_pin, 1);
		if (rx[1] != 0xff && rx[1] != 0x00)
		{
			return 0;
		}
	}
	return -1;
}

static void luat_fota_spi_flash_cmd(uint8_t *tx, uint8_t *rx, uint8_t len)
{
	luat_gpio_set(g_s_fota.cs_pin, 0);
	SPI_BlockTransfer(g_s_fota.spi_id, tx, rx, len);
	luat_gpio_set(g_s_fota.cs_pin, 1);

}

static uint8_t luat_fota_spi_flash_status(void)
{
	uint8_t tx[2] = {0x05, 0xff};
	uint8_t rx[2] = {0xff, 0xff};
	luat_fota_spi_flash_cmd(tx, rx, 2);
	return rx[1];
}

static int luat_fota_erase_spi_flash(uint32_t address)
{
	int i;
	uint8_t cmd[4];
	uint8_t status;
	status = luat_fota_spi_flash_status();
	if (status & 0x01)
	{
		LLOGE("spi flash busy %x!", status);
		return -1;
	}
	cmd[0] = 0x06;
	luat_fota_spi_flash_cmd(cmd, NULL, 1);
	BytesPutBe32(cmd, address & 0x00ffffff);
	cmd[0] = 0xd8;
	luat_fota_spi_flash_cmd(cmd, NULL, 4);
	luat_rtos_task_sleep(150);
	for (i = 0; i < 135; i++)
	{
		status = luat_fota_spi_flash_status();
		if (status & 0x01)
		{

		}
		else
		{
			return 0;
		}
		luat_rtos_task_sleep(10);
	}
	LLOGE("spi flash erase failed!");
	return -1;
}

static int luat_fota_write_spi_flash(uint32_t address, uint8_t *data, uint32_t len)
{
	uint64_t start_tick;
	uint32_t finish_len = 0;
	uint32_t write_len;
	uint8_t cmd[260];
	uint8_t status;
	uint8_t pp_ok;
	status = luat_fota_spi_flash_status();
	if (status & 0x01)
	{
		LLOGE("spi flash busy %x!", status);
		return -1;
	}
	while(finish_len < len)
	{
		cmd[0] = 0x06;
		luat_fota_spi_flash_cmd(cmd, NULL, 1);
		pp_ok = 0;
		BytesPutBe32(cmd, address & 0x00ffffff);
		cmd[0] = 0x02;
		write_len = ((len - finish_len) > 256)?256:(len - finish_len);
		memcpy(cmd + 4, data + finish_len, write_len);
		luat_gpio_set(g_s_fota.cs_pin, 0);
		SPI_BlockTransfer(g_s_fota.spi_id, cmd, NULL, write_len + 4);
		luat_gpio_set(g_s_fota.cs_pin, 1);
		start_tick = luat_mcu_tick64();
		while(luat_mcu_tick64() < (start_tick + 5 * SOC_TICK_1MS))
		{
			status = luat_fota_spi_flash_status();
			if (status & 0x01)
			{

			}
			else
			{
				pp_ok = 1;
				break;
			}
		}
		if (!pp_ok)
		{
			LLOGE("spi flash program failed!");
			return -1;
		}
		finish_len += write_len;
		address += write_len;
	}
	return 0;

}

static void luat_fota_cal_spi_flash_data_md5(unsigned char output[16])
{
	uint8_t *tx = malloc(4096);
	uint8_t *rx = malloc(4096);
	MD5_START(g_s_fota.md5_ctx);
	luat_gpio_set(g_s_fota.cs_pin, 0);
	uint8_t cmd[4] = {0};
	BytesPutBe32(cmd, g_s_fota.start_address & 0x00ffffff);
	cmd[0] = 0x03;
	SPI_BlockTransfer(g_s_fota.spi_id, cmd, rx, 4);
	uint32_t finish_len = 0;
	uint32_t read_len = 0;
	while(finish_len < g_s_fota.p_fota_file_head->CommonDataLen)
	{
		read_len = ((g_s_fota.p_fota_file_head->CommonDataLen - finish_len) > 4096)?4096:(g_s_fota.p_fota_file_head->CommonDataLen - finish_len);
		SPI_BlockTransfer(g_s_fota.spi_id, tx, rx, read_len);
		MD5_UPDATE(g_s_fota.md5_ctx, rx, read_len);
		finish_len += read_len;
	}
//	SPI_BlockTransfer(g_s_fota.spi_id, tx, rx, 4096);
//	MD5_UPDATE(g_s_fota.md5_ctx, rx, 4096);
	luat_gpio_set(g_s_fota.cs_pin, 1);
	MD5_FINISH(g_s_fota.md5_ctx, output);
	free(tx);
	free(rx);
}

int luat_fota_init(uint32_t start_address, uint32_t len, luat_spi_device_t* spi_device, const char *path, uint32_t pathlen)
{
	if (g_s_fota.is_inited) return 0;
	if ((start_address >= 0xe0000000) && spi_device)
	{
		g_s_fota.start_address = start_address;
		g_s_fota.spi_id = spi_device->bus_id;
		g_s_fota.cs_pin = spi_device->spi_config.cs;
		if (spi_device->user_data)
		{
			g_s_fota.power_pin = *(uint8_t *)(spi_device->user_data);
		}
		else
		{
			g_s_fota.power_pin = 0xff;
		}
		if (luat_fota_check_spi_flash())
		{
			LLOGE("no spi flash!");
			return -1;
		}
		LLOGD("spi flash use spi %d cs %d power pin %d!", g_s_fota.spi_id, g_s_fota.cs_pin, g_s_fota.power_pin);
	}

	if (!g_s_fota.crc32_table)
	{
		g_s_fota.p_fota_file_head = luat_heap_calloc(1, sizeof(CoreUpgrade_FileHeadCalMD5Struct));
		g_s_fota.crc32_table = luat_heap_calloc(256, sizeof(uint32_t));

		g_s_fota.md5_ctx = luat_heap_calloc(1, sizeof(MD5_CTX));

		if (g_s_fota.p_fota_file_head == NULL || 
			g_s_fota.crc32_table == NULL ||
			g_s_fota.md5_ctx == NULL) {
			LLOGE("fota init fail, out of sys memory!!!");
			if (g_s_fota.p_fota_file_head) {
				luat_heap_free(g_s_fota.p_fota_file_head);
				g_s_fota.p_fota_file_head = NULL;
			}
			if (g_s_fota.crc32_table) {
				luat_heap_free(g_s_fota.crc32_table);
				g_s_fota.crc32_table = NULL;
			}
			if (g_s_fota.md5_ctx) {
				luat_heap_free(g_s_fota.md5_ctx);
				g_s_fota.md5_ctx = NULL;
			}
			return -1;
		}
		CRC32_CreateTable(g_s_fota.crc32_table, CRC32_GEN);
	}
	g_s_fota.ota_state = OTA_STATE_IDLE;
	g_s_fota.ota_type = CORE_OTA_MODE_FULL;
	if (FULL_OTA_SAVE_ADDR)
	{
		FLASH_eraseSafe(__SOC_OTA_INFO_DATA_SAVE_ADDRESS__, __FLASH_SECTOR_SIZE__);
		CoreUpgrade_HeadCalMD5Struct Head = {0};
		Head.MaigcNum = __APP_START_MAGIC__;
		FLASH_writeSafe((uint8_t *)&Head, __SOC_OTA_INFO_DATA_SAVE_ADDRESS__, sizeof(Head));
	}
	OS_ReInitBuffer(&g_s_fota.data_buffer, __FLASH_SECTOR_SIZE__ * 4);
	g_s_fota.is_inited = 1;
	return 0;
}

int luat_fota_write(uint8_t *data, uint32_t len)
{
	uint32_t save_len;
	if (!g_s_fota.crc32_table) {
		LLOGE("fota need init");
		return -1;
	}
	OS_BufferWrite(&g_s_fota.data_buffer, data, len);
REPEAT:
	switch(g_s_fota.ota_state)
	{
	case OTA_STATE_IDLE:
		if (g_s_fota.data_buffer.Pos > sizeof(CoreUpgrade_FileHeadCalMD5Struct))
		{
			memcpy(g_s_fota.p_fota_file_head, g_s_fota.data_buffer.Data, sizeof(CoreUpgrade_FileHeadCalMD5Struct));
			OS_BufferRemove(&g_s_fota.data_buffer, sizeof(CoreUpgrade_FileHeadCalMD5Struct));
			if (g_s_fota.p_fota_file_head->MaigcNum != __APP_START_MAGIC__)
			{
				LLOGE("OTA包头标志error %x", g_s_fota.p_fota_file_head->MaigcNum);
				g_s_fota.data_buffer.Pos = 0;
				return -1;
			}
			uint32_t crc32 = CRC32_Cal(g_s_fota.crc32_table, g_s_fota.p_fota_file_head->MainVersion, sizeof(CoreUpgrade_FileHeadCalMD5Struct) - 8, 0xffffffff);
			if (crc32 != g_s_fota.p_fota_file_head->CRC32)
			{
				LLOGE("file head crc32 error %x,%x", crc32, g_s_fota.p_fota_file_head->CRC32);
				g_s_fota.data_buffer.Pos = 0;
				return -1;
			}
			g_s_fota.ota_done_len = 0;
			if (g_s_fota.p_fota_file_head->CommonDataLen)
			{
				if (g_s_fota.start_address)
				{
					luat_fota_erase_spi_flash(g_s_fota.start_address);
					g_s_fota.spi_flash_erase_point = g_s_fota.start_address + 0x10000;
				}
				else
				{
					if (__SOC_OTA_COMMON_DATA_LOAD_ADDRESS__ < __SOC_OTA_COMMON_DATA_LOAD_ADDRESS_LIMIT__)
					{
						DBG("全量或者脚本包保存地址(%x)在程序区(%x)内，请检查FULL_OTA_SAVE_ADDR和AP_FLASH_LOAD_SIZE定义是否正确！", __SOC_OTA_COMMON_DATA_LOAD_ADDRESS__, __SOC_OTA_COMMON_DATA_LOAD_ADDRESS_LIMIT__);
						g_s_fota.ota_state = OTA_STATE_ERROR;
						return -1;
					}
				}

				g_s_fota.ota_state = OTA_STATE_WRITE_COMMON_DATA;
				LLOGI("write common data");
				goto REPEAT;
			}
			else
			{
				g_s_fota.ota_state = OTA_STATE_WRITE_SDK_DATA;
				g_s_fota.ota_type = CORE_OTA_MODE_DIFF;
				LLOGI("write core data");
				goto REPEAT;
			}
		}
		break;
	case OTA_STATE_WRITE_COMMON_DATA:
		save_len = ((g_s_fota.ota_done_len + __FLASH_SECTOR_SIZE__) < g_s_fota.p_fota_file_head->CommonDataLen)?__FLASH_SECTOR_SIZE__:(g_s_fota.p_fota_file_head->CommonDataLen - g_s_fota.ota_done_len);
		if (g_s_fota.data_buffer.Pos >= save_len)
		{
			if (g_s_fota.start_address)
			{
				if ((g_s_fota.start_address + g_s_fota.ota_done_len) >= g_s_fota.spi_flash_erase_point)
				{
					luat_fota_erase_spi_flash(g_s_fota.spi_flash_erase_point);
					g_s_fota.spi_flash_erase_point = g_s_fota.spi_flash_erase_point + 0x10000;
				}
				luat_fota_write_spi_flash(g_s_fota.start_address + g_s_fota.ota_done_len, g_s_fota.data_buffer.Data, save_len);
			}
			else
			{
				FLASH_eraseSafe(__SOC_OTA_COMMON_DATA_SAVE_ADDRESS__ + g_s_fota.ota_done_len, __FLASH_SECTOR_SIZE__);
				FLASH_writeSafe(g_s_fota.data_buffer.Data, __SOC_OTA_COMMON_DATA_SAVE_ADDRESS__ + g_s_fota.ota_done_len, save_len);

			}
			OS_BufferRemove(&g_s_fota.data_buffer, save_len);
		}
		else
		{
			break;
		}
		g_s_fota.ota_done_len += save_len;
		if (g_s_fota.ota_done_len >= g_s_fota.p_fota_file_head->CommonDataLen)
		{
			LLOGI("common data done, now checking %x", g_s_fota.start_address);
			uint8_t md5[16] = {0};
			if (g_s_fota.start_address)
			{
				luat_fota_cal_spi_flash_data_md5(md5);
			}
			else
			{
				MD5_START(g_s_fota.md5_ctx);
				MD5_UPDATE(g_s_fota.md5_ctx, (uint8_t *)__SOC_OTA_COMMON_DATA_LOAD_ADDRESS__, g_s_fota.p_fota_file_head->CommonDataLen );
				MD5_FINISH(g_s_fota.md5_ctx, md5);
			}

			if (memcmp(md5, g_s_fota.p_fota_file_head->CommonMD5, 16))
			{
				DBG("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x", md5[0],md5[1],md5[2],md5[3],md5[4],md5[5],md5[6],md5[7],md5[8],md5[9],md5[10],md5[11],md5[12],md5[13],md5[14],md5[15]);
				DBG("%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x%02x",
						g_s_fota.p_fota_file_head->CommonMD5[0],
						g_s_fota.p_fota_file_head->CommonMD5[1],
						g_s_fota.p_fota_file_head->CommonMD5[2],
						g_s_fota.p_fota_file_head->CommonMD5[3],
						g_s_fota.p_fota_file_head->CommonMD5[4],
						g_s_fota.p_fota_file_head->CommonMD5[5],
						g_s_fota.p_fota_file_head->CommonMD5[6],
						g_s_fota.p_fota_file_head->CommonMD5[7],
						g_s_fota.p_fota_file_head->CommonMD5[8],
						g_s_fota.p_fota_file_head->CommonMD5[9],
						g_s_fota.p_fota_file_head->CommonMD5[10],
						g_s_fota.p_fota_file_head->CommonMD5[11],
						g_s_fota.p_fota_file_head->CommonMD5[12],
						g_s_fota.p_fota_file_head->CommonMD5[13],
						g_s_fota.p_fota_file_head->CommonMD5[14],
						g_s_fota.p_fota_file_head->CommonMD5[15]);
				LLOGE("全量包或者脚本包完整性检测失败");
				g_s_fota.ota_state = OTA_STATE_IDLE;
				g_s_fota.data_buffer.Pos = 0;
				return -1;
			}
			else
			{
				LLOGI("common data md5 ok");
				if (g_s_fota.p_fota_file_head->SDKDataLen)
				{
					g_s_fota.ota_state = OTA_STATE_WRITE_SDK_DATA;
					g_s_fota.ota_done_len = 0;
					g_s_fota.ota_type = CORE_OTA_MODE_DIFF;
					LLOGI("write core data");
					goto REPEAT;
				}
				else
				{
					LLOGI("only common data");
					g_s_fota.ota_type = CORE_OTA_MODE_FULL;
					luat_fota_finish();
					return 0;
				}
			}
		}
		else
		{
			goto REPEAT;
		}
		break;
	case OTA_STATE_WRITE_SDK_DATA:
		save_len = ((g_s_fota.ota_done_len + __FLASH_SECTOR_SIZE__) < (g_s_fota.p_fota_file_head->SDKDataLen))?__FLASH_SECTOR_SIZE__:(g_s_fota.p_fota_file_head->SDKDataLen - g_s_fota.ota_done_len);
		if (g_s_fota.data_buffer.Pos >= save_len)
		{
			FLASH_eraseSafe(__SOC_OTA_SDK_DATA_SAVE_ADDRESS__ + g_s_fota.ota_done_len, __FLASH_SECTOR_SIZE__);
			FLASH_writeSafe(g_s_fota.data_buffer.Data, __SOC_OTA_SDK_DATA_SAVE_ADDRESS__ + g_s_fota.ota_done_len, save_len);
			OS_BufferRemove(&g_s_fota.data_buffer, save_len);
			//			mbedtls_md5_update_ret(g_s_fota.md5_ctx, (uint8_t *)(__SOC_OTA_COMMON_DATA_SAVE_ADDRESS__ + g_s_fota.ota_done_len), save_len );
		}
		else
		{
			break;
		}
		g_s_fota.ota_done_len += save_len;
		if (g_s_fota.ota_done_len >= g_s_fota.p_fota_file_head->SDKDataLen)
		{
			fotaNvmInit();
		    FotaDefChkDeltaState_t    chkDelta = {0};
		    FotaDefChkBaseImage_t      chkBase = {0};

			fotaNvmDoExtension(FOTA_DEF_CHK_DELTA_STATE, (void*)&chkDelta);
			if(!chkDelta.isValid)
			{
				LLOGE("差分包完整性检查错误! errno(%d)", chkDelta.state);
				g_s_fota.ota_state = OTA_STATE_IDLE;
				g_s_fota.data_buffer.Pos = 0;
				return -1;
			}
			else
			{
				LLOGI("validate delta ok!");
				fotaNvmDoExtension(FOTA_DEF_CHK_BASE_IMAGE, (void*)&chkBase);
				if(!chkBase.isMatched)
				{
					LLOGE("模块固件版本和差分中的旧固件版本不一致，不能升级!");
					fotaNvmClearDelta(0, 4096);
					g_s_fota.ota_state = OTA_STATE_IDLE;
					g_s_fota.data_buffer.Pos = 0;
					return -1;
				}
			}
			LLOGI("sdk data ok!");
			luat_fota_finish();
			return 0;
		}
		else
		{
			goto REPEAT;
		}
		break;
	case OTA_STATE_ERROR:
		return -1;
	default:
		return 0;
		break;
	}
	return 1;
}

int luat_fota_done(void)
{
	switch(g_s_fota.ota_state)
	{
	case OTA_STATE_IDLE:
		return -1;
	case OTA_STATE_ERROR:
	case OTA_STATE_OK:
		return 0;
	default:
		return 1;
	}
	return 1;
}

int luat_fota_end(uint8_t is_ok)
{
	if (g_s_fota.ota_state != OTA_STATE_OK)
	{
		if (g_s_fota.crc32_table) {
			luat_heap_free(g_s_fota.crc32_table);
			g_s_fota.crc32_table = NULL;
		}
		if (g_s_fota.md5_ctx) {
			luat_heap_free(g_s_fota.md5_ctx);
			g_s_fota.md5_ctx = NULL;
		}
		if (g_s_fota.p_fota_file_head) {
			luat_heap_free(g_s_fota.p_fota_file_head);
			g_s_fota.p_fota_file_head = NULL;
		}
		OS_DeInitBuffer(&g_s_fota.data_buffer);
		if (FULL_OTA_SAVE_ADDR)
		{
			FLASH_eraseSafe(__SOC_OTA_INFO_DATA_SAVE_ADDRESS__, __FLASH_SECTOR_SIZE__);
		}
		g_s_fota.is_inited = 0;
		LLOGE("fota failed");
		return -1;
	}
	g_s_fota.is_inited = 0;
	return 0;
}

uint8_t luat_fota_wait_ready(void)
{
	return 1;
}
