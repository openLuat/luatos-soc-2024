/******************************************************************************

 *(C) Copyright 2018 AirM2M International Ltd.

 * All Rights Reserved

 ******************************************************************************
 *  Filename:psram.h
 *
 *  Description:EC718 psram header file
 *
 *  History: 11/06/2021    Originated by bchang
 *
 *  Notes:
 *
 ******************************************************************************/

#ifndef _FLASH_EC7XX_H
#define _FLASH_EC7XX_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include "ec7xx.h"

#ifdef __cplusplus
extern "C" {
#endif



/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/* QSPI Error codes */
#define PSRAM_OK            ((uint8_t)0x00)
#define PSRAM_ERROR         ((uint8_t)0x01)





/* ID-----------------------------------------------------------*/
#define PSRAM_MFID                      0x0D
#define PSRAM_KGD                       0x50



/* Size of page */
#define PSRAM_PAGE_SIZE                   0x200     /* 512 bytes */


/*********commands start*******************************************/

/* used cmd list
read                        03h
fast read                   0bh
fast read quad              ebh--used
write                       02h
quad write                  38h--used
wrapped read                8bh
wrapped write               82h
Mode reg read               b5h
Mode reg write              b1h
enter QPI mode              35h--used
exit  QPI mode              f5h--used
reset en                    66h--used
reset                       99h--used
burst length toggle         c0h
read id                     9fh--used

*/

/* read cmd */
#define PSRAM_FAST_READ_QUAD_CMD             0xEB
#define PSRAM_WRAP_READ_CMD                  0x8B


/* write cmd */
#define PSRAM_QUAD_WRITE_CMD                 0x38
#define PSRAM_WRAP_WRITE_CMD                 0x82


/* quad mode enter/exit cmd */
#define PSRAM_QUAD_MODE_ENTER_CMD            0x35
#define PSRAM_QUAD_MODE_EXIT_CMD             0xF5

/* reset cmd */
#define PSRAM_RST_EN_CMD                     0x66
#define PSRAM_RST_CMD                        0x99

/* read id cmd */
#define PSRAM_READ_ID_CMD                    0x9F

/* read/write mode reg cmd*/
#define PSRAM_READ_MR_CMD                    0xB5
#define PSRAM_WRITE_MR_CMD                   0xB1


/*********commands end*******************************************/







/*psram  cmd pattern dummy cycles*/
#define DUMMY_CYCLES_FAST_READ_QUAD_QPI             6 //0xeb instruction

#define DUMMY_CYCLES_FAST_READ_QUAD_SPI             6 //0xeb instruction


#define DUMMY_CYCLES_MR_READ_QPI                    6 //0xb5 instruction
#define DUMMY_CYCLES_MR_READ_SPI                    8 //0xb5 instruction







/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/








/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

void PSRAM_qspiInit( void );
uint8_t PSRAM_enterQPI(void);
uint8_t PSRAM_exitQPI(void);
uint8_t PSRAM_exitQPI(void);
uint8_t PSRAM_setClk122M( void );
uint8_t PSRAM_readMR(BOOL isQpiMod );
uint8_t PSRAM_writeMR(BOOL isQpiMod, uint8_t setVal );
uint8_t PSRAM_init( void );
uint8_t PSRAM_dmaAccessClkCtrl( BOOL onoff );
uint8_t PSRAM_setClk102M( void );



#ifdef __cplusplus
}
#endif


#endif

