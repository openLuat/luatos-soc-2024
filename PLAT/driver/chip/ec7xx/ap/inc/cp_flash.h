/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:cp_flash.h
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#ifndef CP_FLASH_H
#define CP_FLASH_H

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#define CPFLH_FSR1_BP_1M                   ((uint8_t)0x14)    /*!< protect 0x0000-0x0fffff full 1MB, only for cp 1M flash */
#define CPFLH_FSR1_BP_768K                 ((uint8_t)0x0c)    /*!< protect 0x0000-0x0C0000 lower 768KB, only for cp 1M flash */
#define CPFLH_FSR2_QE_BP_768K              ((uint8_t)0x42)    /*!< quad enable and CMP=1 */


#define CPFLH_FSR2_QE                      ((uint8_t)0x02)    /*!< quad enable */


#define CPFLH_FSR2_QE                      ((uint8_t)0x02)    /*!< quad enable */



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

void CPFLASH_qspiInit( void );
uint8_t CPFLASH_init(void);
uint8_t CPFLASH_read(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t CPFLASH_write(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
uint8_t CPFLASH_eraseSector(uint32_t SectorAddress);
uint8_t CPFLASH_eraseBlock(uint32_t BlockAddress);
uint8_t CPFLASH_erase64KBlock(uint32_t BlockAddress);
uint8_t CPFLASH_setBurstWithWrap32( void );
uint8_t CPFLASH_exitBurstWithWrap32( void );
uint8_t CPFLASH_xipInit( void );



#endif

