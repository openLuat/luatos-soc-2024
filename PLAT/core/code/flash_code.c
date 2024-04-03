#include "ec7xx.h"
#include "stdint.h"
#include "stdbool.h"
#include "mem_map.h"

typedef struct
{
    bool     isDmic;
    bool     isExPa;
    uint8_t  exPaGain;
    uint8_t  txDigGain;
    uint8_t  txAnaGain;

    uint8_t  rxDigGain0;
    uint8_t  rxAnaGain0;
    uint8_t  rxDigGain50;
    uint8_t  rxAnaGain50;
    uint8_t  rxDigGain100;
    uint8_t  rxAnaGain100;
}HalCodecTlvDefault_t;

/**
  \fn        sysROAddrCheck(uint32_t addr)
  \brief     This function is called in flash erase or write apis to prevent
             unexpected access the bootloader image read only area..
  \param[in] Addr: flash erase or write addr
  \note      Be careful if you want change this function.
 */
static uint8_t sysROAddrCheck(uint32_t addr)
{
    //protect 2 hdr
    if(addr <FUSE_FLASH_MIRROR_ADDR)
    {
          return 1;
    }

    //protect bl
    if((addr >=(BOOTLOADER_FLASH_LOAD_ADDR-AP_FLASH_XIP_ADDR))
          && (addr <(BOOTLOADER_FLASH_LOAD_ADDR+BOOTLOADER_FLASH_LOAD_SIZE-AP_FLASH_XIP_ADDR)))
    {
          return 1;
    }

    //protect app
    if((addr >=(AP_FLASH_LOAD_ADDR-AP_FLASH_XIP_ADDR))
        && (addr <(AP_FLASH_LOAD_ADDR+AP_FLASH_LOAD_SIZE-AP_FLASH_XIP_ADDR)))
    {
        return 1;
    }

    return 0;
}



 /*----------------------------------------------------------------------------*
  *                      GLOBAL FUNCTIONS                                      *
  *----------------------------------------------------------------------------*/


 /**
  \fn        sysROSpaceCheck(uint32_t addr, uint32_t size)
  \brief     This function is called in flash erase or write apis to prevent
             unexpected access the bootloader image read only area..
  \param[in] Addr: flash erase or write addr
             Addr: flash erase or write size
  \note      Be careful if you want change this function.
 */
uint8_t sysROSpaceCheck(uint32_t addr, uint32_t size)

{
    if(sysROAddrCheck(addr))
    {
        return 1;
    }
    if (sysROAddrCheck(addr+size - 1))
    {
        return 1;
    }
    return 0;
}

uint32_t GET_PMU_RAWFLASH_OFFSET(void)
{
	return FLASH_MEM_BACKUP_ADDR;
}

const HalCodecTlvDefault_t codecTlvDefaultVal =
{
    .isDmic         = false,
    .isExPa         = true,
    .exPaGain       = 0,
    .txDigGain      = 0xbf,
    .txAnaGain      = 0x8,

    .rxDigGain0     = 0,
    .rxAnaGain0     = 0,
    .rxDigGain50    = 0x80,
    .rxAnaGain50    = 0x80,
    .rxDigGain100   = 0xff,
    .rxAnaGain100   = 0xff,
};
