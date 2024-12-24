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
#include <string.h>
#include DEBUG_LOG_HEADER_FILE

#include "mem_map.h"
#include "bsp.h"
#include "ex_flash.h"
#include "ex_storage.h"
#include "cmsis_os2.h"
#include "mw_aal_hash.h"



/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define EXSTO_EF_DATA_ADDR             (EF_DATA_LOAD_ADDR)
#define EXSTO_EF_DATA_SIZE             (EF_DATA_LOAD_SIZE)


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t  handle;    /* starting addr @NFS, otherwise fd @FS */
    uint32_t  size;
    uint32_t  overhead;  /* reserved zone size for a special purpose */
    uint32_t  extras;    /* some supplementary info */
}ExStoZone_t;

typedef struct
{
    ExStoZoneId_bm  bmZoneId;
    ExStoZone_t     zone[EXSTO_ZONE_MAXNUM];
}ExStoZoneMan_t;



/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/


AP_PLAT_COMMON_BSS static ExStoZoneMan_t   gExStoZoneMan;


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/



/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/


/**
  \fn        uint8_t exStorageInit(void)
  \brief     init the external storage
  \param[in]
  \note
*/
int32_t exStorageInit(void)
{
    gExStoZoneMan.bmZoneId |= EXSTO_BM_ZONE_EF_DATA;
    gExStoZoneMan.zone[EXSTO_ZONE_EF_DATA].handle   = EXSTO_EF_DATA_ADDR;
    gExStoZoneMan.zone[EXSTO_ZONE_EF_DATA].size     = EXSTO_EF_DATA_SIZE;
    gExStoZoneMan.zone[EXSTO_ZONE_EF_DATA].overhead = 0;
    gExStoZoneMan.zone[EXSTO_ZONE_EF_DATA].extras   = 0;

    if(exFlashInit() == EXFLASH_OK)
        return EXSTO_OK;
    else
        return EXSTO_EINIT;
}


/**
  \fn        int32_t exStorageDeinit(void)
  \brief     deinit the external storage
  \param[in]
  \note
*/
int32_t exStorageDeinit(void)
{
    memset(&gExStoZoneMan, 0, sizeof(ExStoZoneMan_t));

    if(exFlashDeinit() == EXFLASH_OK)
        return EXSTO_OK;
    else
        return EXSTO_EDEINIT;
}


/**
  \fn        int32_t exStorageClear(uint32_t zid, uint32_t offset, uint32_t len)
  \brief     erase the area to be write
  \param[in] zid       resv for future use
             offset    erase start addr
             len       erase len
  \note
*/
int32_t exStorageClear(uint32_t zid, uint32_t offset, uint32_t len)
{
    uint32_t   currLen = len;

    if(zid >= EXSTO_ZONE_MAXNUM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_CLEAR_0, P_WARNING, "clr: invalid zoneId(%d)! max(%d)\n",
                                                               zid, EXSTO_ZONE_MAXNUM);
        return EXSTO_EUNDEF;
    }

    if(!(gExStoZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_CLEAR_1, P_WARNING, "clr: no exSto zone(%d)! bmZoneId(0x%x)\n",
                                                               zid, gExStoZoneMan.bmZoneId);
        return EXSTO_EERASE;
    }

    if(currLen > gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_CLEAR_2, P_WARNING, "clr zone(%d): len(%d) overflowed! set it max(%d)!\n",
                                                               zid, currLen, gExStoZoneMan.zone[zid].size);
        currLen = gExStoZoneMan.zone[zid].size;
    }

    if(offset >= gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_CLEAR_3, P_ERROR, "clr zone(%d): invalid offset(%d)! max(%d)\n",
                                                             zid, offset, gExStoZoneMan.zone[zid].size);
        return EXSTO_EOVRFLOW;
    }

    if(exFlashErase(gExStoZoneMan.zone[zid].handle + offset, len) == EXFLASH_OK)
        return EXSTO_OK;
    else
        return EXSTO_EERASE;
}


/**
  \fn        int32_t exStorageWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
  \brief     write data of the specified length at the specified address
  \param[in] zid       resv for future use
             offset    write start addr
             buf       write buf
             bufLen    write len
  \note
*/
int32_t exStorageWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
    uint32_t   currLen = bufLen;
    uint8_t     *rdPtr = NULL;
    int32_t        ret = 0;

    if(zid >= EXSTO_ZONE_MAXNUM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_WRITE_0, P_WARNING, "wr: invalid zoneId(%d)! max(%d)\n",
                                                               zid, EXSTO_ZONE_MAXNUM);
        return EXSTO_EUNDEF;
    }

    if(!(gExStoZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_WRITE_1, P_WARNING, "wr: no exSto zone(%d)! bmZoneId(0x%x)\n",
                                                               zid, gExStoZoneMan.bmZoneId);
        return EXSTO_EWRITE;
    }

    if(currLen > gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_WRITE_2, P_WARNING, "wr zone(%d): len(%d) overflowed! set it max(%d)!\n",
                                                               zid, currLen, gExStoZoneMan.zone[zid].size);
        currLen = gExStoZoneMan.zone[zid].size;
    }

    if(offset >= gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_WRITE_3, P_ERROR, "wr zone(%d): invalid offset(%d)! max(%d)\n",
                                                             zid, offset, gExStoZoneMan.zone[zid].size);
        return EXSTO_EOVRFLOW;
    }

    //step1 write to flash
    if(exFlashWrite(buf, gExStoZoneMan.zone[zid].handle + offset, currLen) != EXFLASH_OK)
        return EXSTO_EWRITE;

    //step2 read back and check
    rdPtr = mallocEc(currLen);
    if(rdPtr == NULL)
    {
        return EXSTO_EMALLOC;
    }

    if(exFlashRead(rdPtr, gExStoZoneMan.zone[zid].handle + offset, currLen) != EXFLASH_OK)
        return EXSTO_EREAD;

    ret = memcmp(buf,rdPtr,currLen);
    freeEc(rdPtr);

    return ret ? EXSTO_EWRITE : EXSTO_OK;
}


/**
  \fn        int32_t exStorageRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
  \brief     reading data of the specified length at the specified address
  \param[in] zid       resv for future use
             offset    read start addr
             buf       read buf
             bufLen    read len
  \note
*/
int32_t exStorageRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
    uint32_t   currLen = bufLen;

    if(zid >= EXSTO_ZONE_MAXNUM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_READ_0, P_WARNING, "rd: invalid zoneId(%d)! max(%d)\n",
                                                              zid, EXSTO_ZONE_MAXNUM);
        return EXSTO_EUNDEF;
    }

    if(!(gExStoZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_READ_1, P_WARNING, "rd: no exSto zone(%d)! bmZoneId(0x%x)\n",
                                                              zid, gExStoZoneMan.bmZoneId);
        return EXSTO_EWRITE;
    }

    if(currLen > gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_READ_2, P_WARNING, "rd zone(%d): len(%d) overflowed! set it max(%d)!\n",
                                                              zid, currLen, gExStoZoneMan.zone[zid].size);
        currLen = gExStoZoneMan.zone[zid].size;
    }

    if(offset >= gExStoZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_READ_3, P_ERROR, "rd zone(%d): invalid offset(%d)! max(%d)\n",
                                                            zid, offset, gExStoZoneMan.zone[zid].size);
        return EXSTO_EOVRFLOW;
    }

    if(exFlashRead(buf, gExStoZoneMan.zone[zid].handle + offset, currLen) == EXFLASH_OK)
        return EXSTO_OK;
    else
        return EXSTO_EREAD;
}

/**
 * @brief exStorageGetZid(uint32_t addr, uint32_t *size, uint32_t *offset);
 * @details get the es zid according starting address and size of the zone
 *
 * @param addr   the starting addr of the zone
 * @param size   the size of the zone
 * @param offset the offset to starting addr of the zone
 * @return the zone ID.
 */
uint32_t exStorageGetZid(uint32_t addr, uint32_t *size, uint32_t *offset)
{
    uint32_t   zid = EXSTO_ZONE_EF_DATA;
    uint32_t   zsz = EXSTO_EF_DATA_SIZE;

    if(!size)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_GET_ZID_1, P_ERROR, "get zid: null size ptr!\n");
        goto GET_ZID_END;
    }

    if(!addr && !(*size))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_GET_ZID_2, P_SIG, "get zid: set it default(%d: %d)!\n", zid, zsz);
        goto GET_ZID_END;
    }

    switch(addr)
    {
        case EXSTO_EF_DATA_ADDR:
        {
            zid = EXSTO_ZONE_EF_DATA;
            zsz = EXSTO_EF_DATA_SIZE;
            break;
        }

        default:
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_GET_ZID_3, P_WARNING, "get zid: unknown starting addr(0x%x)!\n", addr);
            break;
        }
    }

GET_ZID_END:
    if(!(*size))
    {
        *size = zsz;
    }
    else
    {
        if(*size != zsz)
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_STO_GET_ZID_4, P_WARNING, "get zid: in_size(%d) != zone_size(%d)!\n", *size, zsz);
        }
    }

    if(offset) *offset = 0;

    return zid;
}


/**
 * @brief exStorageGetSize(uint32_t zid, uint8_t isOvhdExcl)
 * @details get the nvm size of specific exstorage zone
 *
 * @param zid         zone Id of exstorage zone
 * @param isOvhdExcl  overhead size of the zone is excluded or not
 * @return the size of zone.
 */
uint32_t exStorageGetSize(uint32_t zid, uint8_t isOvhdExcl)
{
    if(zid >= EXSTO_ZONE_MAXNUM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_GET_SZ_0, P_WARNING, "get size: invalid zoneId(%d)! max(%d)\n",
                                                            zid, EXSTO_ZONE_MAXNUM);
        return EXSTO_EUNDEF;
    }

    if(!(gExStoZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_GET_SZ_1, P_WARNING, "get size: no exSto zone(%d)! bmZoneId(0x%x)\n",
                                                             zid, gExStoZoneMan.bmZoneId);
        return EXSTO_EWRITE;
    }

    return (gExStoZoneMan.zone[zid].size - (isOvhdExcl ? gExStoZoneMan.zone[zid].overhead : 0));
}


/**
 * @brief exStorageVerifyPkg(uint32_t *pkgState)
 * @details validate the data pkg
 *
 * @param pkgState  the last state of data pkg
 * @return 0 succ; < 0 failure with errno.
 */
int32_t exStorageVerifyPkg(uint32_t zid, uint8_t *hash, uint32_t pkgSize, uint32_t *pkgState)
{
    int32_t            retCode = EXSTO_EUNDEF;
    uint8_t            *buffer = NULL;
    uint8_t             isLast = 0;
    uint32_t            offset = 0;
    uint32_t          readSize = 0;
    uint32_t          remnSize = pkgSize;
    uint32_t          stepSize = 4096;
    uint8_t        outHash[32] = {0};
    MwAalSha256Ctx_t    sha256;

    /* no need to verify the pkg */
    if(!hash) return EXSTO_OK;

    if(zid >= EXSTO_ZONE_MAXNUM)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_1, P_WARNING, "verify pkg: invalid zoneId(%d)! max(%d)\n",
                                                                zid, EXSTO_ZONE_MAXNUM);
        return EXSTO_EUNDEF;
    }

    if(!(gExStoZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_2, P_WARNING, "verify pkg: no exSto zone(%d)! bmZoneId(0x%x)\n",
                                                                zid, gExStoZoneMan.bmZoneId);
        return EXSTO_EWRITE;
    }

    mwAalInitSha256(&sha256, 0);

    buffer = (uint8_t*)mallocEc(stepSize);
    if(buffer == NULL)
    {
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_3, P_WARNING, "verify pkg: zone(%d) malloc(%d) failure!\n",
                                                                zid, stepSize);
        goto VERIFY_PKG_END;
    }


    for(offset = 0; offset < pkgSize; offset += readSize)
    {
        if(remnSize > stepSize)
        {
            isLast   = 0;
            readSize = stepSize;
        }
        else
        {
            isLast   = 1;
            readSize = remnSize;
        }
        remnSize -= readSize;

        if(EXSTO_OK != exStorageRead(zid, offset, buffer, readSize))
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_4, P_WARNING, "zid(%d) flash read failure!\n", zid);
            goto VERIFY_PKG_END;
        }

        if(0 != mwAalUpdateSha256(&sha256, buffer, outHash, readSize, isLast))
        {
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_5, P_ERROR, "zid(%d) flash sha256sum fail!\n", zid);
            goto VERIFY_PKG_END;
        }
    }

    if(memcmp(outHash, hash, 32))
    {
        ECPLAT_DUMP(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_6, P_WARNING, "input hash: ", 32, hash);
        ECPLAT_DUMP(UNILOG_PLA_DRIVER, EX_VERIFY_PKG_7, P_WARNING, "calc hash : ", 32, outHash);
    }
    else
    {
        retCode = EXSTO_OK;
    }

VERIFY_PKG_END:
    if(buffer) freeEc(buffer);

    mwAalDeinitSha256(&sha256);

    if(pkgState)
    {
        *pkgState = 0;
    }

    return retCode;
}

/**
 * @brief exStorageGetUpdResult(uint32_t zid, int32_t *pkgState)
 * @details get the updated result
 *
 * @param pkgState  the last state of data pkg
 * @return 0 succ; < 0 failure with errno.
 */
int32_t  exStorageGetUpdResult(uint32_t zid, int32_t *pkgState)
{
    if(pkgState)
    {
        *pkgState = 0;
    }

    return 1;
}


