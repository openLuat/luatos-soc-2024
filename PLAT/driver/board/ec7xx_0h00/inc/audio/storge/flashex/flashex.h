#ifndef __FLASHEX_H__
#define __FLASHEX_H__


#include <stdint.h>
#include <stdbool.h>


#define FLASHEX_SIZE                0x00400000 //4M
#define FLASHEX_IO_REGION_START     0x00000000
#define FLASHEX_TTS_IO_REGION_START 0x00100000
#define FLASHEX_IO_REGION_SIZE      0x00200000 // 1MB
#define FLASHEX_FS_REGION_START     (FLASHEX_IO_REGION_START + FLASHEX_IO_REGION_SIZE)
#define FLASHEX_FS_REGION_SIZE      (FLASHEX_SIZE - FLASHEX_IO_REGION_SIZE)
#define PAGE_SIZE                   256
#define SECTOR_SIZE                 4096


#define W25X_BUSY       0
#define W25X_NotBUSY    1
#define Dummy_Byte1     0xFF

/*********************************************
W25X operation instruction list, the MCU can perform the following operations
on the W25X by sending the following commands to the W25X.
*********************************************/
#define W25X_WriteEnable            0x06    //Write Enable
#define W25X_WriteEnableVSR         0x50    //Write Enable for Volatile Status Register
#define W25X_WriteDisable           0x04    //Write Disable
#define W25X_ReadStatusReg1         0x05    //Read Status Register1:S7~S0
#define W25X_ReadStatusReg2         0x35    //Read Status Register2:S15~S8
#define W25X_WriteStatusReg         0x01    //Write Status Register2:BYTE1:S7~S0  BYTE2：S15~S8
#define W25X_PageProgram            0x02    //PageProgram:BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0  BYTE4:D7~D0
#define W25X_PageErase              0x81    //Page Erase:256   BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0
#define W25X_SectorErase            0x20    //Sector Erase:4K  BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0
#define W25X_BlockErase32K          0x52    //Block Erase:32K  BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0
#define W25X_BlockErase64K          0xD8    //Block Erase:64K  BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0
#define W25X_ChipErase              0xC7
#define W25X_EraseSuspend           0x75
#define W25X_EraseResume            0x7A
#define W25X_PowerDown              0xB9
#define W25X_ContinuousReadMode     0xFF
#define W25X_ReadData               0x03    //Read Data:BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0  BYTE4:D7~D0
#define W25X_FastReadData           0x0B    //Fast Read Data:BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0  BYTE4:dummy  BYTE5:D7~D0
#define W25X_FastReadDual           0x3B    //Fast Read Dual:BYTE1:A23~A16  BYTE2:A15~A8  BYTE3:A7~A0  BYTE4:dummy  BYTE5:D7~D0
#define W25X_ReleasePowerDown       0xAB
#define W25X_DeviceID               0xAB
#define W25X_ManufactDeviceID       0x90
#define W25X_JedecDeviceID          0x9F


typedef struct
{
    uint16_t id;
    uint32_t size;
} FlashInfoT;


void        spiIoInit(void);
int32_t     spiFlashInit(void);
void        spiFlashRead(uint32_t u32ReadAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToRead);
void        spiFlashWritePage(uint32_t u32WriteAddr, uint8_t *pu8Buffer, uint16_t u16NumByteToWrite);
void        spiFlashErasePage(uint32_t u32EraseAddr);
void        spiFlashEraseSector(uint32_t u32EraseAddr);
void        spiFlashEraseBlock(uint32_t u32EraseAddr, uint8_t u8Mode);
void        spiFlashEraseChip(void);
uint16_t    spiFlashReadMdId(void);
FlashInfoT *spiFlashInfoGet(void);


#endif
