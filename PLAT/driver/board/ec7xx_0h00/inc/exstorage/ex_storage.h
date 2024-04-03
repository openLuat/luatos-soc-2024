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

#ifndef __EX_STORAGE_H__
#define __EX_STORAGE_H__


/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/


#include <stdint.h>
#include <stdbool.h>


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define EXSTO_OK           0
#define EXSTO_EUNDEF      -1   /* undefined error */
#define EXSTO_EINIT       -2   /* ex-storage init fail */
#define EXSTO_EDEINIT     -3   /* ex-storage deinit fail */
#define EXSTO_EERASE      -4   /* flash erase fail */
#define EXSTO_EWRITE      -5   /* flash write fail */
#define EXSTO_EREAD       -6   /* flash read fail */
#define EXSTO_EMALLOC     -7   /* memory alloc fail*/
#define EXSTO_EOVRFLOW    -8   /* data overflow */



typedef enum
{
    EXSTO_ZONE_BEGIN = 0,
    EXSTO_ZONE_EF_DATA = EXSTO_ZONE_BEGIN,
    EXSTO_ZONE_EF_RSVD,
    EXSTO_ZONE_SD_DATA,
    EXSTO_ZONE_SD_RSVD,

    EXSTO_ZONE_MAXNUM,
    EXSTO_ZONE_UNDEF = 0xff
}ExStoZoneId_e;

typedef uint32_t ExStoZoneId_bm;
#define EXSTO_BM_ZONE_EF_DATA        (1 << EXSTO_ZONE_EF_DATA)
#define EXSTO_BM_ZONE_EF_RSVD        (1 << EXSTO_ZONE_EF_RSVD)
#define EXSTO_BM_ZONE_SD_DATA        (1 << EXSTO_ZONE_SD_DATA)
#define EXSTO_BM_ZONE_SD_RSVD        (1 << EXSTO_ZONE_SD_RSVD)




/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
  \fn        uint8_t exStorageInit(void)
  \brief     init the external storage
  \param[in]
  \note
*/
int32_t exStorageInit(void);



/**
  \fn        int32_t exStorageDeinit(void)
  \brief     deinit the external storage
  \param[in]
  \note
*/
int32_t exStorageDeinit(void);



/**
  \fn        int32_t exStorageClear(uint32_t zid, uint32_t offset, uint32_t len)
  \brief     erase the area to be write
  \param[in] zid       resv for future use
             offset    erase start addr
             len       erase len
  \note
*/
int32_t exStorageClear(uint32_t zid, uint32_t offset, uint32_t len);



/**
  \fn        int32_t exStorageWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
  \brief     write data of the specified length at the specified address
  \param[in] zid       resv for future use
             offset    write start addr
             buf       write buf
             bufLen    write len
  \note
*/
int32_t exStorageWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen);



/**
  \fn        int32_t exStorageRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
  \brief     reading data of the specified length at the specified address
  \param[in] zid       resv for future use
             offset    read start addr
             buf       read buf
             bufLen    read len
  \note
*/
int32_t exStorageRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen);

/**
 * @brief exStorageGetZid(uint32_t addr, uint32_t *size, uint32_t *offset);
 * @details get the es zid according starting address and size of the zone
 *
 * @param addr   the starting addr of the zone
 * @param size   the size of the zone
 * @param offset the offset to starting addr of the zone
 * @return the zone ID.
 */
uint32_t exStorageGetZid(uint32_t addr, uint32_t *size, uint32_t *offset);

/**
 * @brief exStorageGetSize(uint32_t zid, uint8_t isOvhdExcl)
 * @details get the nvm size of specific fota zone
 *
 * @param zid         zone Id of fota nvm
 * @param isOvhdExcl  overhead size of the zone is excluded or not
 * @return the size of zone.
 */
uint32_t exStorageGetSize(uint32_t zid, uint8_t isOvhdExcl);

/**
 * @brief exStorageVerifyPkg(uint32_t zid, uint8_t *hash, uint32_t pkgSize, uint32_t *pkgState)
 * @details validate the data pkg
 *
 * @param pkgState  the last state of data pkg
 * @return 0 succ; < 0 failure with errno.
 */
int32_t exStorageVerifyPkg(uint32_t zid, uint8_t *hash, uint32_t pkgSize, uint32_t *pkgState);

/**
 * @brief exStorageGetUpdResult(uint32_t zid, int32_t *pkgState)
 * @details get the updated result
 *
 * @param pkgState  the last state of data pkg
 * @return 0 succ; < 0 failure with errno.
 */
int32_t  exStorageGetUpdResult(uint32_t zid, int32_t *pkgState);


#endif


