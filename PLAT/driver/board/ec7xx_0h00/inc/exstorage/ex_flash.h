/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/

#ifndef __EX_FLASH_H__
#define __EX_FLASH_H__


/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/


#include <stdint.h>
#include <stdbool.h>


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/* EXFLASH Error codes */
#define EXFLASH_OK            ((uint8_t)0x00)
#define EXFLASH_ERROR         ((uint8_t)0x01)
#define EXFLASH_BUSY          ((uint8_t)0x02)
#define EXFLASH_NOT_SUPPORTED ((uint8_t)0x04)


#define PAGE_SIZE                   256
#define SECTOR_SIZE                 4096

#define Dummy_Byte1     0xFF




/*********commands*******************************************/

/* Read Operations */
#define READ_CMD                             0x03
#define FAST_READ_CMD                        0x0B


/* Write Operations */
#define WRITE_ENABLE_CMD                     0x06
#define WRITE_DISABLE_CMD                    0x04

#define WRITE_VOLATILE_REG_CMD               0x50

/* Register Operations */
#define READ_STATUS_REG1_CMD                 0x05
#define READ_STATUS_REG2_CMD                 0x35
#define WRITE_STATUS_REG1_CMD                0x01
#define WRITE_STATUS_REG2_CMD                0x31

/* Program Operations */
#define PAGE_PROG_CMD                        0x02
#define QUAD_INPUT_PAGE_PROG_CMD             0x32

/* Erase Operations */
#define SECTOR_ERASE_CMD                     0x20
#define BLOCK_ERASE_32K_CMD                  0x52
#define BLOCK_ERASE_64K_CMD                  0xD8
#define CHIP_ERASE_CMD                       0xC7//or 0x60


/* Identification Operations */
#define READ_ID_CMD                          0x90
#define DUAL_READ_ID_CMD                     0x92
#define QUAD_READ_ID_CMD                     0x94
#define READ_JEDEC_ID_CMD                    0x9F


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/


/**
  \fn        exFlashChipErase(void)
  \brief     perform chip level erase, use with caution!!
  \param[in]
  \note
*/
void exFlashChipErase(void);

/**
  \fn        exFlashBlockErase(void)
  \brief     perform block level erase
  \param[in] u32Data_Addr  :Block first address to start erasing
             u8mode        :Erase mode 1=32K other=64K
  \note
*/
void exFlashBlockErase(uint32_t u32Erase_Addr, uint8_t u8mode);

/**
  \fn        exFlashSectorErase(void)
  \brief     perform sector level erase, normally sector is 4K
  \param[in] u32Data_Addr  :Block first address to start erasing
  \note
*/
void exFlashSectorErase(uint32_t u32Erase_Addr);


/**
  \fn        exFlashRead(void)
  \brief     reading data of the specified length at the specified address
  \param[in] u32ReadAddr       Start reading address(24bit)
             pu8Buffer         Data storage buffer
             u16NumByteToRead  The number of bytes to read(max 65535)
  \note
*/
uint8_t exFlashRead(uint8_t *pu8Buffer, uint32_t u32ReadAddr, uint16_t u16NumByteToRead);


/**
  \fn        exFlashPagePro(void)
  \brief     starts writing data of up to 256 bytes at a specified address on one page (0~65535)
  \param[in] u32ReadAddr       Start reading address(24bit)
             pu8Buffer:Data storage buffer
             u16NumByteToWrite:The number of bytes to write (maximum 256),
  *          the number should not exceed the number of remaining bytes on the page!!!
  \note
*/
void exFlashPagePro(uint32_t u32WriteAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToWrite);



/**
  \fn        exFlashReadMDID(void)
  \brief     reading device ID from flash
  \param[in]
  \note
*/
uint16_t exFlashReadMDID(void);



/**
  \fn        exFlashErase(uint32_t eAddr, uint32_t size)
  \brief     Erases the flash region. use different erase cmd according to size
  \param[in] eAddr addr to erase
             size  erase len
  \note
*/
uint8_t exFlashErase(uint32_t eAddr, uint32_t size);


/**
  \fn        exFlashWrite(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
  \brief     Writes an amount of data to the QSPI flash.
  \param[in] pData:  Data Pointer to write
             WriteAddr:  Write start address
             Size:  Size of data to write
  \note
*/
uint8_t exFlashWrite(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);





/**
  \fn        exFlashInit(void)
  \brief     init the external spi flash
  \param[in]
  \note
*/
uint8_t exFlashInit(void);

/**
  \fn        exFlashDeinit(void)
  \brief     deinit the external spi flash
  \param[in]
  \note
*/
uint8_t exFlashDeinit(void);


#endif


