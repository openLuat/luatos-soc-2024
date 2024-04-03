/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History: initiated by xxxx
*
*  Notes:
*
******************************************************************************/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "ex_flash.h"



/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define TRANSFER_DATA_WIDTH    (8)
#define SPI_SSN_GPIO_INSTANCE   RTE_SPI0_SSN_GPIO_INSTANCE
#define SPI_SSN_GPIO_INDEX      RTE_SPI0_SSN_GPIO_INDEX


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

/** \brief driver instance declare */
extern ARM_DRIVER_SPI Driver_SPI0;
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 0);
//volatile uint16_t gFid = 0;


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

/**
  \fn        exSpiFlashIoIint(void)
  \brief     init SPI interface
  \param[in]
  \note
 */
static void exSpiFlashIoIint(void)
{
    // Initialize master spi
    spiMasterDrv->Initialize(NULL);
    // Power on
    spiMasterDrv->PowerControl(ARM_POWER_FULL);

    // Configure master spi bus
    spiMasterDrv->Control(ARM_SPI_MODE_MASTER | ARM_SPI_CPOL0_CPHA0 | ARM_SPI_DATA_BITS(TRANSFER_DATA_WIDTH) |
                          ARM_SPI_MSB_LSB     | ARM_SPI_SS_MASTER_SW, 26000000U);
}


/**
  \fn        exSpiFlashIoDeIint(void)
  \brief     deinit SPI interface
  \param[in]
  \note
 */
static void exSpiFlashIoDeIint(void)
{
    // Initialize master spi
    spiMasterDrv->Uninitialize();
    // Power on
    spiMasterDrv->PowerControl(ARM_POWER_OFF);

}



/**
  \fn        exFlashSetSpiCsHigh(void)
  \brief     manual control CS PIN
  \param[in]
  \note
 */
static void exFlashSetSpiCsHigh(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 1 << SPI_SSN_GPIO_INDEX);
}



/**
  \fn        exFlashSetSpiCsLow(void)
  \brief     manual control CS PIN
  \param[in]
  \note
 */
static void exFlashSetSpiCsLow(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 0);
}


/**
  \fn        exflashWrByte(void)
  \brief     Software SPI_Flash bus driver basic function, send a single byte to MOSI,
    *        and accept MISO data at the same time. used for both cmd/data send
  \param[in] u8Data:Data sent on the MOSI data line
  \note
*/
static uint8_t exflashWrByte(uint8_t u8Data)
{
    uint8_t u8Out = 0;

    spiMasterDrv->Transfer(&u8Data, &u8Out, 1);

    return u8Out;
}



/**
  \fn        exFlashWrEn(void)
  \brief     send wr en cmd to flash
  \param[in]
  \note
 */
static void exFlashWrEn(void)
{
    exFlashSetSpiCsLow();
    exflashWrByte(WRITE_ENABLE_CMD);
    exFlashSetSpiCsHigh();
}


/**
  \fn        exFlashWrDisen(void)
  \brief     send wr disen cmd to flash
  \param[in]
  \note
 */
static void exFlashWrDisen(void)
{
    exFlashSetSpiCsLow();
    exflashWrByte(WRITE_DISABLE_CMD);
    exFlashSetSpiCsHigh();
}



/**
  \fn        exFlashPollingBusyFlag(void)
  \brief     read the BUSY field in flash reg and loop until done
  \param[in]
  \note
 */
static void exFlashPollingBusyFlag(void)
{
    uint8_t u8test;
    exFlashSetSpiCsLow();

    do
    {
        exflashWrByte(READ_STATUS_REG1_CMD);
        u8test = exflashWrByte(Dummy_Byte1);
    } while ((u8test & 0x01) == 0x01);

    exFlashSetSpiCsHigh();
}







/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/


/**
  \fn        exFlashChipErase(void)
  \brief     perform chip level erase, use with caution!!
  \param[in]
  \note
*/
void exFlashChipErase(void)
{
    exFlashWrEn();
    exFlashPollingBusyFlag();
    exFlashSetSpiCsLow();
    exflashWrByte(CHIP_ERASE_CMD);
    exFlashSetSpiCsHigh();
    exFlashPollingBusyFlag();
    exFlashWrDisen();
}



/**
  \fn        exFlashBlockErase(uint32_t u32Erase_Addr, uint8_t u8mode)
  \brief     perform block level erase
  \param[in] u32Data_Addr  :Block first address to start erasing
             u8mode        :Erase mode 1=32K other=64K
  \note
*/
void exFlashBlockErase(uint32_t u32Erase_Addr, uint8_t u8mode)
{
    exFlashWrEn();
    exFlashPollingBusyFlag();
    exFlashSetSpiCsLow();

    if (u8mode == 1)
    {
        exflashWrByte(BLOCK_ERASE_32K_CMD);
    }
    else
    {
        exflashWrByte(BLOCK_ERASE_64K_CMD);
    }

    exflashWrByte(u32Erase_Addr >> 16);
    exflashWrByte(u32Erase_Addr >> 8);
    exflashWrByte(u32Erase_Addr);
    exFlashSetSpiCsHigh();
    exFlashPollingBusyFlag();
    exFlashWrDisen();
}



/**
  \fn        exFlashSectorErase(uint32_t u32Erase_Addr)
  \brief     perform sector level erase, normally sector is 4K
  \param[in] u32Data_Addr  :Block first address to start erasing
  \note
*/
void exFlashSectorErase(uint32_t u32Erase_Addr)
{
    exFlashWrEn();
    exFlashPollingBusyFlag();
    exFlashSetSpiCsLow();

    exflashWrByte(SECTOR_ERASE_CMD);

    exflashWrByte(u32Erase_Addr >> 16);
    exflashWrByte(u32Erase_Addr >> 8);
    exflashWrByte(u32Erase_Addr);
    exFlashSetSpiCsHigh();
    exFlashPollingBusyFlag();
    exFlashWrDisen();
}






/**
  \fn        exFlashPagePro(void)
  \brief     starts writing data of up to 256 bytes at a specified address on one page (0~65535)
  \param[in] u32ReadAddr       Start reading address(24bit)
             pu8Buffer:Data storage buffer
             u16NumByteToWrite:The number of bytes to write (maximum 256),
  *          the number should not exceed the number of remaining bytes on the page!!!
  \note
*/
void exFlashPagePro(uint32_t u32WriteAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToWrite)
{
    uint16_t i;
    exFlashWrEn();
    exFlashSetSpiCsLow();
    exflashWrByte(PAGE_PROG_CMD);
    exflashWrByte((uint8_t)((u32WriteAddr) >> 16));
    exflashWrByte((uint8_t)((u32WriteAddr) >> 8));
    exflashWrByte((uint8_t)u32WriteAddr);

    for (i = 0; i < u16NumByteToWrite; i++)exflashWrByte(pu8Buffer[i]);

    exFlashSetSpiCsHigh();
    exFlashPollingBusyFlag();
    exFlashWrDisen();
}


/**
  \fn        exFlashReadMDID(void)
  \brief     reading device ID from flash
  \param[in]
  \note
*/
uint16_t exFlashReadMDID(void)
{
    uint16_t        u16Temp = 0;
    /* Enable chip select */
    exFlashSetSpiCsLow();
    /* Send "RDID " instruction */
    exflashWrByte(READ_ID_CMD);
    exflashWrByte(0x00);
    exflashWrByte(0x00);
    exflashWrByte(0x00);
    /* Read a byte from the FLASH */
    u16Temp |= exflashWrByte(Dummy_Byte1) << 8;
    u16Temp |= exflashWrByte(Dummy_Byte1);
    /* Disable chip select */
    exFlashSetSpiCsHigh();
    return u16Temp;
}




/**
  \fn        uint8_t exFlashRead(uint8_t *pu8Buffer, uint32_t u32ReadAddr, uint16_t u16NumByteToRead)
  \brief     reading data of the specified length at the specified address
  \param[in] u32ReadAddr       Start reading address(24bit)
             pu8Buffer         Data storage buffer
             u16NumByteToRead  The number of bytes to read(max 65535)
  \note
*/
uint8_t exFlashRead(uint8_t *pu8Buffer, uint32_t u32ReadAddr, uint16_t u16NumByteToRead)
{
    uint16_t i;
    exFlashSetSpiCsLow();/* Enable chip select */
    exflashWrByte(READ_CMD);
    exflashWrByte(u32ReadAddr >> 16);
    exflashWrByte(u32ReadAddr >> 8);
    exflashWrByte(u32ReadAddr);

    for (i = 0; i < u16NumByteToRead; i++)
    {
        pu8Buffer[i] = exflashWrByte(Dummy_Byte1); //Read one byte
    }

    exFlashSetSpiCsHigh();/* Disable chip select */

    return EXFLASH_OK;
}



/**
  \fn        exFlashErase(uint32_t eAddr, uint32_t size)
  \brief     Erases the flash region. use different erase cmd according to size
  \param[in] eAddr addr to erase
             size  erase len
  \note
*/
uint8_t exFlashErase(uint32_t eAddr, uint32_t size)
{
    uint32_t offsetAddress;
    uint32_t remainLen;
    uint32_t currEraseSize;


    if ((eAddr&0xfff) !=0)
    {
        return EXFLASH_ERROR;
    }

    offsetAddress = eAddr;
    remainLen = size;

    while(remainLen > 0)
    {
        if (((offsetAddress&0xffff) ==0) && (remainLen >=0x10000))
        {
            exFlashBlockErase(offsetAddress,0);
            remainLen -= 0x10000;
            currEraseSize = 0x10000;
        }
        else if (((offsetAddress&0x7fff) ==0)&& (remainLen >=0x8000))
        {
            exFlashBlockErase(offsetAddress,1);
            remainLen-=0x8000;
            currEraseSize = 0x8000;
        }
        else
        {
            exFlashSectorErase(offsetAddress);
            currEraseSize = (remainLen >= 0x1000) ? 0x1000 : remainLen;
            remainLen -= currEraseSize;
        }

        offsetAddress += currEraseSize;
    }

    return EXFLASH_OK;
}




/**
  \fn        exFlashWrite(uint8_t* pData, uint32_t WriteAddr, uint32_t size)
  \brief     Writes an amount of data to the QSPI flash.
  \param[in] pData:  Data Pointer to write
             WriteAddr:  Write start address
             Size:  Size of data to write
  \note
*/
uint8_t exFlashWrite(uint8_t* pData, uint32_t WriteAddr, uint32_t size)
{
    uint32_t end_addr, current_size, current_addr;
    //uint32_t irqMask;

    // Calculation of the size between the write address and the end of the page
    //write non-aligned bytes first
    current_size = PAGE_SIZE - (WriteAddr%PAGE_SIZE);

    // Check if the size of the data is less than the remaining place in the page
    if (current_size > size)
    {
        current_size = size;
    }

    // Initialize the adress variables
    current_addr = WriteAddr;
    end_addr = WriteAddr + size;

    // Perform the write page by page
    do
    {
        //irqMask = SaveAndSetIRQMask();

        exFlashPagePro(current_addr,pData,current_size);

        // Update the address and size variables for next page programming
        current_addr += current_size;
        pData += current_size;
        current_size = ((current_addr + PAGE_SIZE) > end_addr) ? (end_addr - current_addr) : PAGE_SIZE;

        //RestoreIRQMask(irqMask);
    } while (current_addr < end_addr);

    return EXFLASH_OK;
}




/**
  \fn        exFlashInit(void)
  \brief     init the external spi flash
  \param[in]
  \note
*/
uint8_t exFlashInit(void)
{

    exSpiFlashIoIint();
    //gFid = exFlashReadMDID();

    return EXFLASH_OK;
}


/**
  \fn        exFlashDeinit(void)
  \brief     deinit the external spi flash
  \param[in]
  \note
*/
uint8_t exFlashDeinit(void)
{
    exSpiFlashIoDeIint();
    return EXFLASH_OK;
}

