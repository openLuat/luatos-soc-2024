/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    nvram_flash.h
 * Description:  EC618 NVRAM header file
 * History:      11/29/2017    Originated by cheng fang
 *
 ****************************************************************************/

#ifndef _NVRAM_FLASH_H
#define _NVRAM_FLASH_H

#ifdef __cplusplus
 extern "C" {
#endif

/*
618 and 718H has 1M CP flash, need cp flash API
718S/P/U AP/CP share one flash, only need AP flash API
*/


#if (defined CHIP_EC618)
#define nvram_flash_earse(a)        BSP_QSPI_Erase_Safe(a, 0x1000)
#define nvram_flash_write(a,b,c)    BSP_QSPI_Write_Safe(a,b,c)
#define nvram_flash_read(a,b,c)     BSP_QSPI_Read_Safe(a,b,c)
#define cp_nvram_flash_init()       CPXIP_QSPI_Init()
#define cp_nvram_flash_earse(a)     CPXIP_QSPI_Erase_Sector(a)
#define cp_nvram_flash_write(a,b,c) CPXIP_QSPI_Write(a,b,c)
#elif (defined TYPE_EC718H)
#define nvram_flash_earse(a)	    FLASH_eraseSafe(a, 0x1000)
#define nvram_flash_write(a,b,c)    FLASH_writeSafe(a,b,c)
#define nvram_flash_read(a,b,c)     FLASH_readSafe(a,b,c)
#define cp_nvram_flash_earse(a)     CPFLASH_eraseSector(a)
#define cp_nvram_flash_write(a,b,c) CPFLASH_write(a,b,c)



#elif (defined TYPE_EC718S) || (defined TYPE_EC718P) || (defined TYPE_EC718U) || (defined TYPE_EC716S) || (defined TYPE_EC716E) || (defined TYPE_EC718U) || (defined TYPE_EC718M)
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
#define nvram_flash_earse(a)        FLASH_eraseSectorSafe(a)
#define nvram_flash_write(a,b,c)    FLASH_writeBl(a,b,c)
#define nvram_flash_read(a,b,c)     FLASH_XIPRead(a,b,c)
#define cp_nvram_flash_earse(a)     FLASH_eraseSectorSafe(a)
#define cp_nvram_flash_write(a,b,c) FLASH_writeBl(a,b,c)
#else
#define nvram_flash_earse(a)        FLASH_eraseSafe(a, 0x1000)
#define nvram_flash_write(a,b,c)    FLASH_writeSafe(a,b,c)
#define nvram_flash_read(a,b,c)     FLASH_readSafe(a,b,c)
#define cp_nvram_flash_earse(a)     FLASH_eraseSafe(a, 0x1000)
#define cp_nvram_flash_write(a,b,c) FLASH_writeSafe(a,b,c)
#endif
#endif

#if defined CHIP_EC618 || defined CHIP_EC618_Z0
extern uint8_t  CPXIP_QSPI_Erase_Sector(uint32_t SectorAddress);
extern uint8_t  CPXIP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);

#else
extern uint8_t FLASH_XIPRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t FLASH_writeBl(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
extern uint8_t FLASH_eraseSectorSafe(uint32_t SectorAddress);
extern uint8_t CPFLASH_write(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t CPFLASH_eraseSector(uint32_t SectorAddress);

#endif


#ifdef __cplusplus
}
#endif

#endif /* _NVRAM_FLASH_H */
