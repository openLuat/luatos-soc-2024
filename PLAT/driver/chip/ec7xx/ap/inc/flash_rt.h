/******************************************************************************

 *(C) Copyright 2018 AirM2M International Ltd.

 * All Rights Reserved

 ******************************************************************************
 *  Filename:flash_rt.h
 *
 *  Description:EC718 flash header file
 *
 *  History:
 *
 *  Notes:
 *
 ******************************************************************************/


#ifndef _FLASH_EC718_RT_H
#define _FLASH_EC718_RT_H
/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
    extern "C" {
#endif

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

/* QSPI Error codes */
#define QSPI_OK            ((uint8_t)0x00)
#define QSPI_ERROR         ((uint8_t)0x01)
#define QSPI_BUSY          ((uint8_t)0x02)
#define QSPI_NOT_SUPPORTED ((uint8_t)0x04)
#define QSPI_SUSPENDED     ((uint8_t)0x08)

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


typedef enum
{
    QSPI_OP_ERASE = 0,
    QSPI_OP_WRITE,
}FlhOpType_e;




/**
  \brief       definition of the flash operation callback, register using BSP_QSPI_Reg_Operation_Cb
               call when flash erase and write happens. 
  \param[in]   result          the QSPI Error codes, listed above
  \param[in]   type            to indicate erase or write
  \param[in]   address         operation address
  \param[in]   size            operation size  
  \return      null
*/
typedef void (*flashOperCallback_t)(uint8_t result, FlhOpType_e type, uint32_t address, uint32_t size);



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

uint8_t FLASH_eraseSafe(uint32_t SectorAddress, uint32_t Size);
uint8_t FLASH_writeSafe(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
uint8_t FLASH_readSafe(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);



#ifdef __cplusplus
}
#endif

#endif
