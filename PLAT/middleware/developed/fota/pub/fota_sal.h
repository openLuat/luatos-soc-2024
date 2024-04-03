/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: fota_sal.h
*
*  Description:
*
*  History: 2021/10/24 created by xuwang
*
*  Notes:
*
******************************************************************************/
#ifndef FOTA_SAL_H
#define FOTA_SAL_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdint.h>
#if defined CHIP_EC616 || defined CHIP_EC616_Z0 || defined CHIP_EC616S || defined CHIP_EC626
#define FOTA_PLAT_SCT_ZI /*PLAT_BL_SCT_ZI*/

#if defined CHIP_EC616 || defined CHIP_EC616_Z0
#include "system_ec616.h"
#include "flash_ec616_rt.h"
#elif defined CHIP_EC616S
#include "system_ec616s.h"
#include "flash_ec616s_rt.h"
#elif defined CHIP_EC626
#include "system_ec626.h"
#include "flash_ec626_rt.h"
#endif

#ifdef FEATURE_FOTA_ENABLE
#include "common.h"
#else
#include "debug_trace.h"
#include "debug_log.h"
#endif

#else /* ec618/ec7xx/... */
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#include "system_ec618.h"
#elif defined CHIP_EC718 || defined CHIP_EC716
#include "system_ec7xx.h"
#endif

#include "sctdef.h"
#include "flash_rt.h"

#ifdef FEATURE_FOTA_ENABLE
#include "common.h"
#else
#include DEBUG_LOG_HEADER_FILE
#endif

#endif


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
//#ifdef FEATURE_FOTA_ENABLE
#define FOTA_NVM_SECTOR_ERASE_MODE     1

//ap flash operation
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        BSP_QSPI_Erase_Sector_Safe(addr)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    BSP_QSPI_Erase_32KBlk_Safe(addr)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   BSP_QSPI_Write(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    BSP_QSPI_XIP_Read(buf, addr, size)
#elif defined CHIP_EC718 || defined CHIP_EC716
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        FLASH_eraseSectorSafe(addr)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    FLASH_erase32KBlkSafe(addr)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   FLASH_write(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    FLASH_XIPRead(buf, addr, size)
#else
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        BSP_QSPI_Erase_Sector(addr)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    BSP_QSPI_Erase_Block(addr)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   BSP_QSPI_Write(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    ImageGeneralRead(buf, addr, size)
#endif


//cp flash operation(share or dedicate cp flash)
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#define BSP_QSPI_ENABLE_CP_FLASH()                 CPXIP_Enable()
#define BSP_QSPI_DISABLE_CP_FLASH()                CPXIP_DeInit()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        CPXIP_QSPI_Erase_Sector(addr)
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    CPXIP_QSPI_Erase_32Kblk(addr)
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   CPXIP_QSPI_Write(buf, addr, size)
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    CPXIP_QSPI_Read(buf, addr, size)
#elif defined CHIP_EC718 || defined CHIP_EC716
#ifdef TYPE_EC718H//dedicate cp flash
#define BSP_QSPI_ENABLE_CP_FLASH()                 CPFLASH_xipInit()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        CPFLASH_eraseSector(addr)
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    CPFLASH_eraseBlock(addr)
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   CPFLASH_write(buf, addr, size)
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    CPFLASH_read(buf, addr, size)
#else//share ap flash
#define BSP_QSPI_ENABLE_CP_FLASH()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        FLASH_eraseSectorSafe(addr)
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    FLASH_erase32KBlkSafe(addr)
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   FLASH_write(buf, addr, size)
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    FLASH_XIPRead(buf, addr, size)
#endif
#else
#define BSP_QSPI_ENABLE_CP_FLASH()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        1
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    1
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   1
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    1
#endif

#else
#define FOTA_NVM_SECTOR_ERASE_MODE     0

//ap flash operation
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        BSP_QSPI_Erase_Safe(addr, size)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    BSP_QSPI_Erase_Safe(addr, size)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   BSP_QSPI_Write_Safe(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    BSP_QSPI_Read_Safe(buf, addr, size)
#elif defined CHIP_EC718 || defined CHIP_EC716
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        FLASH_eraseSafe(addr, size)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    FLASH_eraseSafe(addr, size)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   FLASH_writeSafe(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    FLASH_XIPRead(buf, addr, size)
#else
#define BSP_QSPI_ERASE_AP_FLASH(addr, size)        BSP_QSPI_Erase_Safe(addr, size)
#define BSP_QSPI_ERASE_AP_FLASH_32K(addr, size)    BSP_QSPI_Erase_Safe(addr, size)
#define BSP_QSPI_WRITE_AP_FLASH(buf, addr, size)   BSP_QSPI_Write_Safe(buf, addr, size)
#define BSP_QSPI_READ_AP_FLASH(buf, addr, size)    BSP_QSPI_Read_Safe(buf, addr, size)
#endif


#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#define BSP_QSPI_ENABLE_CP_FLASH()                  CPXIP_Enable()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    -1
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        -1
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   -1
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    CPXIP_QSPI_Read(buf, addr, size)
#elif defined CHIP_EC718 || defined CHIP_EC716
#ifdef TYPE_EC718H
#define BSP_QSPI_ENABLE_CP_FLASH()                 CPFLASH_xipInit()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        CPFLASH_eraseSector(addr)
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    CPFLASH_eraseBlock(addr)
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   CPFLASH_write(buf, addr, size)
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    CPFLASH_read(buf, addr, size)
#else//share ap flash
#define BSP_QSPI_ENABLE_CP_FLASH()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        FLASH_eraseSafe(addr, size)
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    FLASH_eraseSafe(addr, size)
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   FLASH_writeSafe(buf, addr, size)
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    FLASH_XIPRead(buf, addr, size)
#endif
#else
#define BSP_QSPI_ENABLE_CP_FLASH()
#define BSP_QSPI_DISABLE_CP_FLASH()
#define BSP_QSPI_ERASE_CP_FLASH(addr, size)        1
#define BSP_QSPI_ERASE_CP_FLASH_32K(addr, size)    1
#define BSP_QSPI_WRITE_CP_FLASH(buf, addr, size)   1
#define BSP_QSPI_READ_CP_FLASH(buf, addr, size)    1
#endif

#endif


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/


#if defined CHIP_EC618 || defined CHIP_EC618_Z0
extern uint8_t  BSP_QSPI_Erase_Sector_Safe(uint32_t SectorAddress);
extern uint8_t  BSP_QSPI_Erase_32KBlk_Safe(uint32_t BlockAddress);
extern uint8_t  BSP_QSPI_Erase_Sector(uint32_t SectorAddress);
extern uint8_t  BSP_QSPI_Erase_Block(uint32_t BlockAddress);
extern uint8_t  BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t  BSP_QSPI_XIP_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

extern void     CPXIP_Enable(void);
extern void     CPXIP_DeInit(void);
extern uint8_t  CPXIP_QSPI_Erase_32Kblk(uint32_t SectorAddress);
extern uint8_t  CPXIP_QSPI_Erase_Sector(uint32_t SectorAddress);
extern uint8_t  CPXIP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint32_t CPXIP_QSPI_Read(uint8_t *pData,uint32_t ReadAddr, uint32_t Size);

#elif defined CHIP_EC718 || defined CHIP_EC716

extern uint8_t FLASH_XIPRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t FLASH_write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t FLASH_eraseSectorSafe(uint32_t SectorAddress);
extern uint8_t FLASH_erase32KBlkSafe(uint32_t SectorAddress);
extern uint8_t CPFLASH_read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t CPFLASH_write(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t CPFLASH_eraseSector(uint32_t SectorAddress);
extern uint8_t CPFLASH_eraseBlock(uint32_t BlockAddress);
extern uint8_t CPFLASH_xipInit( void );

#else

extern uint8_t  BSP_QSPI_Erase_Sector(uint32_t SectorAddress);
extern uint8_t  BSP_QSPI_Erase_Block(uint32_t BlockAddress);
extern uint8_t  BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint32_t ImageGeneralRead(uint8_t * pData, uint32_t  ReadAddr, uint32_t Size);
//extern uint8_t  BSP_QSPI_Read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);

#endif


#ifdef __cplusplus
}
#endif
#endif

