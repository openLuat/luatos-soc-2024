/*
 * lfs util functions
 *
 * Copyright (c) 2017, Arm Limited. All rights reserved.
 * SPDX-License-Identifier: BSD-3-Clause
 */
#include "lfs_util.h"
#include "plat_config.h"
#include DEBUG_LOG_HEADER_FILE


// Only compile if user does not provide custom config
#ifndef LFS_CONFIG

#if defined CHIP_EC618 || defined CHIP_EC618_Z0
uint8_t BSP_QSPI_SWReset(void);
uint8_t BSP_QSPI_XIP_Mode_Disable(void);
uint8_t BSP_QSPI_XIP_Mode_Enable(void);
#else
extern void FLASH_clrStatus( void );
#endif


static uint32_t gLFSAssertFlag = 0;

// Software CRC implementation with small lookup table
uint32_t lfs_crc(uint32_t crc, const void *buffer, size_t size) {
    static uint32_t rtable[16] = {
        0x00000000, 0x1db71064, 0x3b6e20c8, 0x26d930ac,
        0x76dc4190, 0x6b6b51f4, 0x4db26158, 0x5005713c,
        0xedb88320, 0xf00f9344, 0xd6d6a3e8, 0xcb61b38c,
        0x9b64c2b0, 0x86d3d2d4, 0xa00ae278, 0xbdbdf21c,
    };

#if 0
    const uint8_t *data = buffer;

    for (size_t i = 0; i < size; i++) {
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data[i] >> 4)) & 0xf];
    }

#else

    uint32_t *data_32 = (uint32_t*)buffer;

    for(size_t i = 0; i < (size >> 2); i++, data_32++)
    {
        uint32_t aWord = *data_32;

        for(size_t j = 0; j < 4; j++)
        {
            crc = (crc >> 4) ^ rtable[(crc ^ ((aWord & 0xff) >> 0)) & 0xf];
            crc = (crc >> 4) ^ rtable[(crc ^ ((aWord & 0xff) >> 4)) & 0xf];
            aWord >>= 8;
        }

    }
    const uint8_t* data_8 = (uint8_t*)data_32;

    for (size_t i = 0; i < (size & 0x3); i++)
    {
        crc = (crc >> 4) ^ rtable[(crc ^ (data_8[i] >> 0)) & 0xf];
        crc = (crc >> 4) ^ rtable[(crc ^ (data_8[i] >> 4)) & 0xf];
    }

#endif
    return crc;
}

void lfs_setAssertFlag(uint32_t flag)
{
    gLFSAssertFlag |= flag;
}

void lfs_assert(bool test)
{
    if(test == 0)
    {
        // shall be atomic op below
        __disable_irq();

        if (gLFSAssertFlag & EC_FS_ASSERT_FLASH_RESET_FLAG)
        {
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
            BSP_QSPI_XIP_Mode_Disable();
            BSP_QSPI_SWReset();
            BSP_QSPI_XIP_Mode_Enable();
#else
            FLASH_clrStatus();
#endif
        }

        uint32_t excepFsAssertCount = BSP_GetFSAssertCount();
        BSP_SetFSAssertCount(++excepFsAssertCount);

        ECPLAT_PRINTF(UNILOG_LFS, lfs_assert_0, P_ERROR, "lfs assert count:%d", excepFsAssertCount);

        if(excepFsAssertCount && ((excepFsAssertCount % EC_FS_ASSERT_REFORMAT_THRESHOLD) == 0))
        {
            ECPLAT_PRINTF(UNILOG_LFS, lfs_assert_1, P_ERROR, "!!!File System and Plat config will be clear!!!");
        }


        EC_ASSERT(0, 0, 0, 0);
    }

}

#endif
