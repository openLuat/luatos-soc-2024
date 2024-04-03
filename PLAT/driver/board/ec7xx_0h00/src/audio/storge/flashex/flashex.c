#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include "bsp.h"
#include "cmsis_os2.h"
#include "flashex.h"

/** \brief SPI data width */
#define TRANSFER_DATA_WIDTH (8)

/** \brief driver instance declare */
extern ARM_DRIVER_SPI Driver_SPI0;
static ARM_DRIVER_SPI *spiMasterDrv = &CREATE_SYMBOL(Driver_SPI, 0);
#define SPI_SSN_GPIO_INSTANCE   RTE_SPI0_SSN_GPIO_INSTANCE
#define SPI_SSN_GPIO_INDEX      RTE_SPI0_SSN_GPIO_INDEX

#define DRIVER_FLASHEX_THREAD_SAFE_ENABLE
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
static osMutexId_t  gFlashexMutex    = NULL;
#endif
static FlashInfoT   gFlashUnverified = {0};
static uint16_t     gFlashIndex      = 0xFFFF;
static FlashInfoT   gFlashList[]     =
{
    {0x8513, 0x100000},   /* P25Q80H      1MB */
    {0x8514, 0x200000},   /* P25Q16SH     2MB */
    {0x8515, 0x400000},   /* P25Q32SH     4MB */
    {0x8516, 0x800000},   /* P25Q64SL     8MB */
    {0x2515, 0x400000},   /* SK25LE032    4MB */
    {0x2516, 0x800000},   /* SK25LE064    8MB */
    {0x2015, 0x400000},   /* XM25QH32CHIG 4MB */
    {0xC814, 0x200000},   /* MD25Q16CSIG  2MB */
    {0xC414, 0x200000},   /* GT25Q16A     2MB */
    {0xC415, 0x400000},   /* GT25Q32A     4MB */
    {0xC815, 0x800000},   /* GT25Q64A     8MB */
};


/**
  * @brief Software SPI port initialization
  * @param[in] None
  * @return  None
  */
void spiIoInit(void)
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
  * @brief Software SPI_Flash bus driver basic function, send a single byte to MOSI,
  *        and accept MISO data at the same time.
  * @param[in] u8Data:Data sent on the MOSI data line
  * @return    u8Out: Data received on the MISO data line
  */
uint8_t spiFlashWriteByte(uint8_t u8Data)
{
    uint8_t u8Out = 0;

    spiMasterDrv->Transfer(&u8Data, &u8Out, 1);

    return u8Out;
}

void spiCsSetHigh(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 1 << SPI_SSN_GPIO_INDEX);
}

void spiCsSetLow(void)
{
    GPIO_pinWrite(SPI_SSN_GPIO_INSTANCE, 1 << SPI_SSN_GPIO_INDEX, 0);
}



/**
  * @brief Flash Write Enable
  * @param[in] None
  * @return  None
  */
void spiFlashWriteEnable(void)
{
    spiCsSetLow();
    spiFlashWriteByte(W25X_WriteEnable);
    spiCsSetHigh();
}


/**
  * @brief Flash Write Disable
  * @param[in] None
  * @return  None
  */
void spiFlashWriteDisable(void)
{
    spiCsSetLow();
    spiFlashWriteByte(W25X_WriteDisable);
    spiCsSetHigh();
}


/**
  * @brief Read the BUSY of FLASH and wait if it is busy.
  *        The reason for BUSY is erase, or continuous read and write.
  * @param[in] None
  * @return  None
  */
void spiFlashWaitBusy(void)
{
    uint8_t u8Test;

    spiCsSetLow();

    do
    {
        spiFlashWriteByte(W25X_ReadStatusReg1);
        u8Test = spiFlashWriteByte(Dummy_Byte1);
    } while ((u8Test & 0x01) == 0x01);

    spiCsSetHigh();
}


/**
  * @brief read FLASH BUSY Status
  * @param[in] None
  * @return  Free return NotBUSY，Busy return BUSY
  */
uint8_t spiFlashReadBusy(void)
{
    uint8_t u8Test;

    spiCsSetLow();
    spiFlashWriteByte(W25X_ReadStatusReg1);
    u8Test = spiFlashWriteByte(Dummy_Byte1);
    spiCsSetHigh();

    if (u8Test & 0x01)return (W25X_BUSY);
    else return (W25X_NotBUSY);
}


/**
  * @brief Erase the entire flash data
  * @param[in] None
  * @return  None
  */
void spiFlashEraseChip(void)
{
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();
    spiFlashWriteByte(W25X_ChipErase);
    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}


/**
  * @brief Erase a 32K or 64K block
  * @param[in] u32DataAddr  :Block first address to start erasing
  * @param[in] u8Mode       :Erase mode 1=32K other=64K
  * @return  None
  */
void spiFlashEraseBlock(uint32_t u32EraseAddr, uint8_t u8Mode)
{
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();

    if (u8Mode == 1)
    {
        spiFlashWriteByte(W25X_BlockErase32K);
    }
    else
    {
        spiFlashWriteByte(W25X_BlockErase64K);
    }

    spiFlashWriteByte(u32EraseAddr >> 16);
    spiFlashWriteByte(u32EraseAddr >> 8);
    spiFlashWriteByte(u32EraseAddr);
    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}


/**
  * @brief Erase a 4K sector
  * @param[in] u32DataAddr  :sector first address to start erasing
  * @return  None
  */
void spiFlashEraseSector(uint32_t u32EraseAddr)
{
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();

    spiFlashWriteByte(W25X_SectorErase);
    
    spiFlashWriteByte(u32EraseAddr >> 16);
    spiFlashWriteByte(u32EraseAddr >> 8);
    spiFlashWriteByte(u32EraseAddr);
    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}



/**
  * @brief Erase a page
  * @param[in] u32DataAddr  :page first address to start erasing
  * @return  None
  */
void spiFlashErasePage(uint32_t u32EraseAddr)
{
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiFlashWaitBusy();
    spiCsSetLow();

    spiFlashWriteByte(W25X_PageErase);

    spiFlashWriteByte(u32EraseAddr >> 16);
    spiFlashWriteByte(u32EraseAddr >> 8);
    spiFlashWriteByte(u32EraseAddr);
    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}



/**
  * @brief Start reading data of the specified length at the specified address
  * @param[in] u32ReadAddr       Start reading address(24bit)
  * @param[in] pu8Buffer        Data storage buffer
  * @param[in] u16 NumByteToRead  The number of bytes to read(max 65535)
  * @return  None
  */
void spiFlashRead(uint32_t u32ReadAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToRead)
{
    uint16_t i;

#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiCsSetLow();/* Enable chip select */
    spiFlashWriteByte(W25X_ReadData);
    spiFlashWriteByte(u32ReadAddr >> 16);
    spiFlashWriteByte(u32ReadAddr >> 8);
    spiFlashWriteByte(u32ReadAddr);

    for (i = 0; i < u16NumByteToRead; i++)
    {
        pu8Buffer[i] = spiFlashWriteByte(Dummy_Byte1); //Read one byte
    }

    spiCsSetHigh();/* Disable chip select */
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}

/**
  * @brief SPI starts writing data of up to 256 bytes at a specified address on one page (0~65535)
  * @param[in] u32WriteAddr:Address to start writing(24bit)
  * @param[in] pu8Buffer:Data storage buffer
  * @param[in] u16NumByteToWrite:The number of bytes to write (maximum 256),
  *            the number should not exceed the number of remaining bytes on the page!!!
  * @return  None
  */
void spiFlashWritePage(uint32_t u32WriteAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToWrite)
{
    uint16_t i;

#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    spiFlashWriteEnable();
    spiCsSetLow();
    spiFlashWriteByte(W25X_PageProgram);
    spiFlashWriteByte((uint8_t)((u32WriteAddr) >> 16));
    spiFlashWriteByte((uint8_t)((u32WriteAddr) >> 8));
    spiFlashWriteByte((uint8_t)u32WriteAddr);

    for (i = 0; i < u16NumByteToWrite; i++)spiFlashWriteByte(pu8Buffer[i]);

    spiCsSetHigh();
    spiFlashWaitBusy();
    spiFlashWriteDisable();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif
}


/**
  * @brief Reads FLASH identification.
  * @param[in] None
  * @return  FLASH identification
  */
uint16_t spiFlashReadMdId(void)
{
    uint16_t u16Temp = 0;

#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexAcquire(gFlashexMutex, osWaitForever);
#endif
    /* Enable chip select */
    spiCsSetLow();
    /* Send "RDID " instruction */
    spiFlashWriteByte(W25X_ManufactDeviceID);
    spiFlashWriteByte(0x00);
    spiFlashWriteByte(0x00);
    spiFlashWriteByte(0x00);
    /* Read a byte from the FLASH */
    u16Temp |= spiFlashWriteByte(Dummy_Byte1) << 8;
    u16Temp |= spiFlashWriteByte(Dummy_Byte1);
    /* Disable chip select */
    spiCsSetHigh();
#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    osMutexRelease(gFlashexMutex);
#endif

    return u16Temp;
}

FlashInfoT *spiFlashInfoGet(void)
{
    FlashInfoT *info = NULL;

    if ((gFlashIndex >= 0) && (gFlashIndex < sizeof(gFlashList) / sizeof(gFlashList[0])))
    {
        info = &gFlashList[gFlashIndex];
    }
    else if ((gFlashUnverified.id != 0) && (gFlashUnverified.id != 0xFFFF))
    {
        info = &gFlashUnverified;
    }

    return info;
}

int32_t spiFlashInit(void)
{
    int32_t  retVal = -1;
    uint32_t size   = sizeof(gFlashList) / sizeof(gFlashList[0]);
    uint16_t id     = 0;

#ifdef DRIVER_FLASHEX_THREAD_SAFE_ENABLE
    gFlashexMutex = osMutexNew(NULL);
    if(gFlashexMutex == NULL)
    {
        printf("Failed to create mutex for gFlashexMutex\r\n");
        goto labelEnd;
    }
#endif

    spiIoInit();
    id = spiFlashReadMdId();
    for (uint32_t i=0; i<size; i++)
    {
        if (id == gFlashList[i].id)
        {
            retVal      = 0;
            gFlashIndex = i;
            goto labelEnd;
        }
    }

    if ((id == 0x0000) || (id == 0x00FF) || (id == 0xFF00) || (id == 0xFFFF))
    {
        printf("No external Flash.");
    }
    else
    {
        retVal                = 0;
        gFlashUnverified.id   = id;
        gFlashUnverified.size = 0;
        printf("Unverified external Flash: %04X\r\n", id);
    }

labelEnd:
    return retVal;
}