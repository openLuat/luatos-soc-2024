/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>
#ifdef FEATURE_FOTA_FS_ENABLE
#include "lfs_port.h"
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
#include "bl_lfs.h"
#else
#include "osasys.h"
#endif
#endif
#include "mem_map.h"
#include "cmsis_os2.h"
#include "sctdef.h"
#include "nvram.h"
#include "fota_sal.h"
#include "fota_utils.h"
#include "fota_chksum.h"
#include "fota_custom.h"
#include "fota_nvm.h"
extern uint8_t FLASH_write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size);
/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
#define FOTA_IS_CPFLASH_DISABLED()     1
#else
#define FOTA_IS_CPFLASH_DISABLED()     apmuIsCpSleeped()
#endif

#define FOTA_NVM_SECTOR_SIZE           FOTA_BUF_SIZE_4K
#define FOTA_NVM_BLOCK_SIZE            FOTA_BUF_SIZE_32K

#ifdef FEATURE_FOTA_FS_ENABLE
#define FOTA_DELTA_PAR_NAME            "FotaDelta.par"
/*  caution:
 *  the default FLASH_FS_REGION_SIZE is 48K, you can combine FS partition with FOTA partition.
 *  and the size of FS partition should be less than or equal to the previous FLASH_FOTA_REGION_LEN (@FS disabled),
 *  because enough space for REMAP(4K) and BACKUP area should be reserved.
 */
#define FOTA_DELTA_PAR_MAXSIZE         (FLASH_FS_REGION_SIZE - FOTA_BUF_SIZE_32K)

/* caution:
 * the remapping zone should be erased every time
 * after the delta.par is downloaded completely OR
 * before the new upgrade is executed!
 */
#define FOTA_NVM_REMAP_ADDR            (FOTA_NVM_DELTA_BACKUP_ADDR - FOTA_NVM_REMAP_SIZE)
#define FOTA_NVM_REMAP_SIZE            (FOTA_NVM_SECTOR_SIZE)

#define FOTA_NVM_DELTA_BACKUP_ADDR     (NVRAM_PHYSICAL_BASE - FOTA_NVM_DELTA_BACKUP_SIZE)

#else  /* NFS */
#define FOTA_NVM_DELTA_ADDR            (FLASH_FOTA_REGION_START)
#define FOTA_NVM_DELTA_SIZE            (FLASH_FOTA_REGION_LEN)

#define FOTA_NVM_DELTA_DOWNLOAD_ADDR   (FOTA_NVM_DELTA_ADDR)
#define FOTA_NVM_DELTA_DOWNLOAD_SIZE   (FOTA_NVM_DELTA_SIZE - FOTA_NVM_DELTA_BACKUP_SIZE)

#define FOTA_NVM_DELTA_BACKUP_ADDR     (FOTA_NVM_DELTA_ADDR + FOTA_NVM_DELTA_DOWNLOAD_SIZE)
#endif  /* FEATURE_FOTA_FS_ENABLE */

/*
 * FOR EC7xx CHIP, the layout of BKUP zone will be multiplexed as follows!
 *

FOTA_NVM_DELTA_BACKUP_ADDR   |---------------------------------| <---offset 0
                             |    SYS FOTA RESULT ZONE(4KB)    |
                             |---------------------------------| <---offset 4K
                             |    BL2 FOTA FLAGS ZONE(4KB)     |
                             |---------------------------------| <---offset 8K
                             |                                 |
                             |                                 |
                             |---------------------------------| <---offset (END - 16K)
                             |    EXCEP DUMP ZONE(16K)         |
FLASH_FOTA_REGION_END        |---------------------------------|

*/
#if defined CHIP_EC718 || defined CHIP_EC716
#define FOTA_NVM_DELTA_BACKUP_SIZE     (FOTA_BUF_SIZE_1K * 44)
#elif defined CHIP_EC626
#define FOTA_NVM_DELTA_BACKUP_SIZE     (FOTA_BUF_SIZE_4K)
#else /* defined CHIP_EC616 || defined CHIP_EC616_Z0 || defined CHIP_EC616S || CHIP_EC618 */
#define FOTA_NVM_DELTA_BACKUP_SIZE     (FOTA_BUF_SIZE_32K)
#endif
//4m flash csdk use full 96K space
#if defined TYPE_EC718P || defined TYPE_EC716E || defined TYPE_EC718U || defined TYPE_EC718M
#undef FOTA_NVM_DELTA_BACKUP_SIZE
#define FOTA_NVM_DELTA_BACKUP_SIZE		(FOTA_BUF_SIZE_1K * 96)
#endif

#define FOTA_NVM_REAL_BACKUP_ADDR      (FOTA_NVM_DELTA_BACKUP_ADDR)
#define FOTA_NVM_REAL_BACKUP_SIZE      (FOTA_NVM_DELTA_BACKUP_SIZE + FOTA_NVM_BACKUP_MUX_SIZE)
#if defined CHIP_EC718 || defined CHIP_EC716  || defined CHIP_EC626 || defined TYPE_EC718M
#define FOTA_NVM_BACKUP_MUX_SIZE       (NVRAM_PHYSICAL_SIZE)
#else /* defined CHIP_EC616 || defined CHIP_EC616_Z0 || defined CHIP_EC616S || CHIP_EC618 */
#define FOTA_NVM_BACKUP_MUX_SIZE       0
#endif
//4m flash csdk no need NVRAM_PHYSICAL
#if defined TYPE_EC718P || defined TYPE_EC716E || defined TYPE_EC718U || defined TYPE_EC718M
#undef FOTA_NVM_BACKUP_MUX_SIZE
#define FOTA_NVM_BACKUP_MUX_SIZE		0
#endif

#if defined CHIP_EC618 || defined CHIP_EC618_Z0 || defined CHIP_EC718 || defined CHIP_EC716
#define FOTA_NVM_A2AP_XIP_ADDR         (AP_FLASH_XIP_ADDR)
#if defined CHIP_EC618 || defined CHIP_EC618_Z0
#define FOTA_NVM_A2CP_XIP_ADDR         (AP_VIEW_CPFLASH_XIP_ADDR)
#elif defined CHIP_EC718 || defined CHIP_EC716 || defined TYPE_EC718M
#ifdef TYPE_EC718H
#define FOTA_NVM_A2CP_XIP_ADDR         (AP_VIEW_CPFLASH_XIP_ADDR)
#else
#define FOTA_NVM_A2CP_XIP_ADDR         (AP_FLASH_XIP_ADDR)
#endif
#endif

#define FOTA_NVM_AP_LOAD_ADDR          (AP_FLASH_LOAD_ADDR & (~AP_FLASH_XIP_ADDR))
#define FOTA_NVM_AP_LOAD_SIZE          (AP_FLASH_LOAD_SIZE)

#define FOTA_NVM_CP_LOAD_ADDR          (CP_FLASH_LOAD_ADDR & (~CP_FLASH_XIP_ADDR))
#define FOTA_NVM_CP_LOAD_SIZE          (CP_FLASH_LOAD_SIZE)

#define FOTA_NVM_SYSH_LOAD_ADDR        (SYS_SEC_HAED_ADDR)
#define FOTA_NVM_SYSH_LOAD_SIZE        (SYS_FLASH_LOAD_SIZE)

#define FLASH_XIP_ADDR                 (AP_FLASH_XIP_ADDR)

#else /* @ec616/ec616s/ec626 */
#define FOTA_NVM_A2AP_XIP_ADDR         (FLASH_XIP_ADDR)
#define FOTA_NVM_A2CP_XIP_ADDR         (FLASH_XIP_ADDR)

/* APP type by default */
#define FOTA_CUST_APP_ENABLE
#endif

#ifdef FOTA_CUST_APP_ENABLE
#define FOTA_NVM_APP_LOAD_ADDR         (APP_FLASH_LOAD_ADDR & (~FOTA_NVM_A2AP_XIP_ADDR))
#define FOTA_NVM_APP_LOAD_SIZE         (APP_FLASH_LOAD_SIZE)
#endif

/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/
typedef struct
{
    uint32_t  handle;    /* means addr @NFS, otherwise fd @FS */
    uint32_t  size;
    uint32_t  overhead;  /* reserved zone size for a special purpose */
    uint32_t  extras;    /* some supplementary info */
}FotaNvmZone_t;

typedef struct
{
    FotaNvmZoneId_bm  bmZoneId;
    FotaNvmZone_t     zone[FOTA_NVM_ZONE_MAXNUM];
}FotaNvmZoneMan_t;


/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/


extern osStatus_t osDelay (uint32_t ticks);
extern bool apmuIsCpSleeped(void);

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

#ifdef TYPE_EC718M
AP_PLAT_COMMON_BSS static FotaNvmZoneMan_t   gFotaNvmZoneMan;

/* sha256 hash */
AP_PLAT_COMMON_BSS uint8_t  gFotaHash[FOTA_SHA256_HASH_LEN];
#else
static FotaNvmZoneMan_t   gFotaNvmZoneMan;

/* sha256 hash */
FOTA_PLAT_SCT_ZI uint8_t  gFotaHash[FOTA_SHA256_HASH_LEN];
#endif

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/

extern uint32_t VerifySignature(uint8_t is224, uint8_t *hash, uint8_t *ecsda, uint8_t *pubKey);

extern osStatus_t osDelay (uint32_t ticks);
extern bool apmuIsCpSleeped(void);

/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

#ifdef TYPE_EC718M
AP_PLAT_COMMON_BSS static FotaNvmZoneMan_t   gFotaNvmZoneMan;

/* sha256 hash */
AP_PLAT_COMMON_BSS uint8_t  gFotaHash[FOTA_SHA256_HASH_LEN];
#else
static FotaNvmZoneMan_t   gFotaNvmZoneMan;

/* sha256 hash */
FOTA_PLAT_SCT_ZI uint8_t  gFotaHash[FOTA_SHA256_HASH_LEN];
#endif

/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTIONS                                     *
 *----------------------------------------------------------------------------*/
static void fotaNvmSetZone(FotaNvmZoneId_e zid, uint32_t handle, uint32_t size, uint32_t ovhd, uint32_t extras)
{
    if(zid >= FOTA_NVM_ZONE_MAXNUM) return;

    gFotaNvmZoneMan.bmZoneId |= 1 << zid ;
    gFotaNvmZoneMan.zone[zid].handle   = handle;
    gFotaNvmZoneMan.zone[zid].size     = size;
    gFotaNvmZoneMan.zone[zid].overhead = ovhd;
    gFotaNvmZoneMan.zone[zid].extras   = extras;
}

#ifdef FEATURE_FOTA_FS_ENABLE
#define FOTA_DELTA_PAR_FD    1

typedef struct
{
    lfs_file_t fp;
    uint32_t   openFlag;
    int32_t    fd;
}FotaLfsHandler_t;

FotaLfsHandler_t gFotaLfsHndl = {{0}, 0, FOTA_DELTA_PAR_FD};

int32_t fotaNvmFsOpen(const char *fname, const char* mode)
{
    int32_t   err = 0;
    int32_t    fd = -1;
    int32_t flags = LFS_O_RDONLY;

    if(!strcmp(mode,"rw+"))
    {
        flags = LFS_O_RDWR | LFS_O_CREAT;
    }

    if(gFotaLfsHndl.openFlag == 1) return gFotaLfsHndl.fd;

    err = LFS_fileOpen(&gFotaLfsHndl.fp,fname,flags);
    if(err == LFS_ERR_OK)
    {
        fd = gFotaLfsHndl.fd;
        gFotaLfsHndl.openFlag = 1;
    }

     return fd;
}

int32_t fotaNvmFsClose(int32_t fd)
{
    if(fd != gFotaLfsHndl.fd) return FOTA_EOK;

    if(gFotaLfsHndl.openFlag == 1)
    {
        LFS_fileClose(&gFotaLfsHndl.fp);
        gFotaLfsHndl.openFlag = 0;
    }

    return FOTA_EOK;
}

int32_t fotaNvmFsSize(int32_t fd)
{
    lfs_soff_t fileSize = 0;

    if(fd < 0) return 0;

    if(fd == gFotaLfsHndl.fd)
    {
        fileSize = LFS_fileSize(&gFotaLfsHndl.fp);
    }

    return fileSize;
}

int32_t fotaNvmFsTell(int32_t fd)
{
    lfs_soff_t pos = 0;

    if(fd < 0) return 0;

    if(fd == gFotaLfsHndl.fd)
    {
        pos = LFS_fileTell(&gFotaLfsHndl.fp);
    }

    return pos;
}

uint32_t fotaNvmFsRead(void *buf, uint32_t size, uint32_t count, int32_t fd)
{
    uint32_t readSize = 0;

    if(fd < 0) return 0;

    if((fd == gFotaLfsHndl.fd) && (gFotaLfsHndl.openFlag == 1))
    {
        readSize = LFS_fileRead(&gFotaLfsHndl.fp, buf, size*count);
    }

    return readSize;
}

uint32_t fotaNvmFsWrite(void *buf, uint32_t size, uint32_t count, int32_t fd)
{
    uint32_t writeSize = 0;

    if(fd < 0) return 0;

    if((fd == gFotaLfsHndl.fd) && (gFotaLfsHndl.openFlag == 1))
    {
        writeSize = LFS_fileWrite(&gFotaLfsHndl.fp, buf, size*count);
    }
    LFS_fileSync(&gFotaLfsHndl.fp);

    return writeSize;
}

int32_t fotaNvmFsSeek(int32_t fd, int32_t offset, uint8_t seekType)
{
  if((fd == gFotaLfsHndl.fd) && (gFotaLfsHndl.openFlag == 1))
  {
      LFS_fileSeek(&gFotaLfsHndl.fp, offset, seekType);
  }

  return FOTA_EOK;
}

int32_t fotaNvmFsRemove(const char *fname)
{
    LFS_remove(fname);
    return FOTA_EOK;
}
#endif

/* non fs: raw flash */
static int32_t fotaNvmNfsClear(uint32_t zid, uint32_t offset, uint32_t len)
{
    uint8_t    retValue = 0;
    uint32_t   eraseLen = 0;
    uint32_t   currAddr = 0;
    uint32_t    adjAddr = 0;
    uint32_t    currLen = len;

    if(zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)) || (FLASH_FOTA_ADDR_UNDEF == gFotaNvmZoneMan.zone[zid].handle))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_1, P_WARNING, "clr: no fota zone(%d)? bmZid(0x%x), handle(0x%x)\n",
                                                    zid, gFotaNvmZoneMan.bmZoneId, gFotaNvmZoneMan.zone[zid].handle);
        return FOTA_EUNFOUND;
    }

    if(offset % FOTA_NVM_SECTOR_SIZE)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_2, P_WARNING, "clr zone(%d): offset(%d) unaligned by 4K!\n", zid, offset);
        offset = (offset / FOTA_NVM_SECTOR_SIZE) * FOTA_NVM_SECTOR_SIZE;
    }

    if(currLen > gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_3, P_WARNING, "clr zone(%d): len(%d) ovfl! set it with max(%d)!\n",
                                                                 zid, currLen, gFotaNvmZoneMan.zone[zid].size);
        currLen = gFotaNvmZoneMan.zone[zid].size;
    }

    if(offset >= gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_4, P_ERROR, "clr zone(%d): invalid offset(%d)! max(%d)\n",
                                                               zid, offset, gFotaNvmZoneMan.zone[zid].size);
        return FOTA_EARGS;
    }

    currAddr = ((gFotaNvmZoneMan.zone[zid].handle + offset) & (~ gFotaNvmZoneMan.zone[zid].extras));
    if(currAddr % FOTA_NVM_SECTOR_SIZE)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_5, P_ERROR, "clr zone(%d): addr(%d) unaligned by 4K err!\n", zid, currAddr);
        return FOTA_EPERM;
    }

    while(currLen > 0)
    {
        if(currLen >= FOTA_NVM_BLOCK_SIZE)
        {
            eraseLen = FOTA_NVM_BLOCK_SIZE;
            if(currAddr % FOTA_NVM_BLOCK_SIZE)
            {
                adjAddr  = FOTA_ALIGN_UP(currAddr, FOTA_NVM_BLOCK_SIZE);
                eraseLen = adjAddr - currAddr;
            }
        }
        else
        {
            eraseLen = currLen;
        }

        if(eraseLen > currLen || eraseLen % FOTA_NVM_SECTOR_SIZE)
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_6, P_ERROR, "clr zone(%d): curr/erase Len(0x%x/0x%x) err!\n",
                                                                   zid, currLen, eraseLen);
            return FOTA_EFLERASE;
        }

        /* if @bl, split it to 4KB one by one! */
        if(FOTA_NVM_SECTOR_ERASE_MODE && eraseLen < FOTA_NVM_BLOCK_SIZE)
        {
            eraseLen = FOTA_NVM_SECTOR_SIZE;
        }

        if(zid == FOTA_NVM_ZONE_CP)
        {
            if(eraseLen == FOTA_NVM_BLOCK_SIZE)
            {
                retValue = BSP_QSPI_ERASE_CP_FLASH_32K(currAddr, eraseLen);
            }
            else
            {
                retValue = BSP_QSPI_ERASE_CP_FLASH(currAddr, eraseLen);
            }
        }
        else
        {
            if(eraseLen == FOTA_NVM_BLOCK_SIZE)
            {
                retValue = BSP_QSPI_ERASE_AP_FLASH_32K(currAddr, eraseLen);
            }
            else
            {
                retValue = BSP_QSPI_ERASE_AP_FLASH(currAddr, eraseLen);
            }
        }

        if (retValue != QSPI_OK)
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLEAR_8, P_ERROR, "clr zone(%d): err! addr(0x%x), errno(%d)\n",
                                                                   zid, currAddr, retValue);
            return FOTA_EFLERASE;
        }

        currAddr += eraseLen;
        currLen  -= eraseLen;

        osDelay(100 / FOTA_TICK_RATE_MS);
    }

    return (retValue == QSPI_OK ? FOTA_EOK : FOTA_EFLERASE);
}

static int32_t fotaNvmNfsWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
    uint8_t  retValue = 0;
    uint32_t currAddr = 0;
    uint32_t  currLen = bufLen;

    if(zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)) || (FLASH_FOTA_ADDR_UNDEF == gFotaNvmZoneMan.zone[zid].handle))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_WRITE_1, P_WARNING, "wr: no fota zone(%d)? bmZid(0x%x), handle(0x%x)\n",
                                                   zid, gFotaNvmZoneMan.bmZoneId, gFotaNvmZoneMan.zone[zid].handle);
        return FOTA_EUNFOUND;
    }

    if(!currLen)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_WRITE_2, P_WARNING, "wr zone(%d): zero buf len?\n", zid);
        return FOTA_EOK;
    }
    else if(currLen > gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_WRITE_3, P_WARNING, "wr zone(%d): len(%d) ovfl! set it max(%d)!\n",
                                                                 zid, currLen, gFotaNvmZoneMan.zone[zid].size);
        currLen = gFotaNvmZoneMan.zone[zid].size;
    }

    if(offset >= gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_WRITE_4, P_ERROR, "wr zone(%d): invalid offset(%d)! max(%d)\n",
                                                               zid, offset, gFotaNvmZoneMan.zone[zid].size);
        return FOTA_EARGS;
    }

    currAddr = ((gFotaNvmZoneMan.zone[zid].handle + offset) & (~ gFotaNvmZoneMan.zone[zid].extras));

    if(zid == FOTA_NVM_ZONE_CP)
    {
        retValue = BSP_QSPI_WRITE_CP_FLASH(buf, currAddr, currLen);
    }
    else
    {
        retValue = BSP_QSPI_WRITE_AP_FLASH(buf, currAddr, currLen);
    }

    return (retValue == QSPI_OK ? FOTA_EOK : FOTA_EFLWRITE);
}

PLAT_BL_CIRAM_FLASH_TEXT static int32_t fotaNvmNfsRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
    uint8_t  retValue = 0;
    uint32_t currAddr = 0;
    uint32_t  currLen = bufLen;

    if(zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)) || (FLASH_FOTA_ADDR_UNDEF == gFotaNvmZoneMan.zone[zid].handle))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_READ_1, P_WARNING, "rd: no fota zone(%d)? bmZid(0x%x), handle(0x%x)\n",
                                                  zid, gFotaNvmZoneMan.bmZoneId, gFotaNvmZoneMan.zone[zid].handle);
        return FOTA_EUNFOUND;
    }

    if(!currLen)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_READ_2, P_WARNING, "rd zone(%d): zero buf len?\n", zid);
        return FOTA_EOK;
    }
    else if(currLen > gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_READ_3, P_WARNING, "rd zone(%d): len(%d) ovfl! set it max(%d)!\n",
                                                                zid, currLen, gFotaNvmZoneMan.zone[zid].size);
        currLen = gFotaNvmZoneMan.zone[zid].size;
    }

    if(offset >= gFotaNvmZoneMan.zone[zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_READ_4, P_ERROR, "rd zone(%d): invalid offset(%d)! max(%d)\n",
                                                              zid, offset, gFotaNvmZoneMan.zone[zid].size);
        return FOTA_EARGS;
    }

    currAddr = ((gFotaNvmZoneMan.zone[zid].handle + offset)  & (~ gFotaNvmZoneMan.zone[zid].extras));

    if(zid == FOTA_NVM_ZONE_CP)
    {
        retValue = BSP_QSPI_READ_CP_FLASH(buf, currAddr, currLen);
    }
    else
    {
        retValue = BSP_QSPI_READ_AP_FLASH(buf, currAddr, currLen);
    }

    return (retValue == QSPI_OK ? FOTA_EOK : FOTA_EFLREAD);
}

static int32_t fotaNvmCheckRemapZone(FotaDefChkRemapZone_t *remapZone)
{
#ifdef FEATURE_FOTA_FS_ENABLE
    remapZone->isEnable = 1;
#else
    remapZone->isEnable = 0;
#endif

    return FOTA_EOK;
}

static int32_t fotaNvmSetDfuResult(FotaDefDfuResult_t *result)
{
    uint8_t  buf[4] = {0};

    if(!result) return FOTA_EARGS;

    if(result->dfuResult == FOTA_DRC_DFU_SUCC)
    {
        snprintf((char*)buf, 4, "%s", "OK");
    }
    else if(result->dfuResult == FOTA_DRC_DFU_FAIL)
    {
        snprintf((char*)buf, 4, "%s", "NO");
    }
    /* errno */
    buf[3] = result->deltaState;

    if(FOTA_EOK != fotaNvmClear(FOTA_NVM_ZONE_BKUP, 0, fotaNvmGetSize(FOTA_NVM_ZONE_BKUP, 1)))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_SET_DFU_1, P_ERROR, "set DFU: clr bkup zone fail!\n");
        return FOTA_EFLERASE;
    }

    if(FOTA_EOK != fotaNvmWrite(FOTA_NVM_ZONE_BKUP, 0, buf, 4))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_SET_DFU_2, P_WARNING, "set DFU: '%s', errno(%d)!\n", buf, result->deltaState);
        return FOTA_EFLWRITE;
    }

    return FOTA_EOK;
}

PLAT_BL_CIRAM_FLASH_TEXT static int32_t fotaNvmGetDfuResult(FotaDefDfuResult_t *result)
{
    uint8_t   buf[4] = {0};

    if(!result) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & FOTA_NVM_BM_ZONE_BKUP))
    {
        fotaNvmSetZone(FOTA_NVM_ZONE_BKUP, FOTA_NVM_REAL_BACKUP_ADDR, FOTA_NVM_REAL_BACKUP_SIZE, FOTA_NVM_BACKUP_MUX_SIZE, FOTA_NVM_A2AP_XIP_ADDR);
    }

    if(FOTA_EOK != fotaNvmRead(FOTA_NVM_ZONE_BKUP, 0, buf, 4))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_DFU_1, P_ERROR, "get DFU: rd bkup zone fail!\n");
        return FOTA_EFLREAD;
    }

    if (strncmp((char*)buf, "OK", 2) == 0)
    {
        result->dfuResult = FOTA_DRC_DFU_SUCC;
    }
    else if (strncmp((char*)buf, "NO", 2) == 0)
    {
        result->dfuResult = FOTA_DRC_DFU_FAIL;
    }
    else
    {
        result->dfuResult = FOTA_DRC_DFU_UNDEF;
    }
    /* errno */
    result->deltaState = buf[3];

    ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_DFU_2, P_INFO, "get DFU: %s(%d)\n",
               (result->dfuResult == FOTA_DRC_DFU_SUCC ? "succ" :
               (result->dfuResult ? "fail" : "no result")), result->dfuResult);

    return FOTA_EOK;
}

static int32_t fotaNvmPrepareDfu(FotaDefPrepareDfu_t *preDfu)
{
    int32_t  retCode = FOTA_EOK;

#if (FOTA_NVM_BACKUP_MUX_SIZE != 0)
    if(NVRAM_CHECK_FAC_VALID())
    {
        //ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_PREP_DFU_1, P_WARNING, "prep DFU: fac nv ok!\n");
        return FOTA_EOK;
    }

    ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_PREP_DFU_2, P_WARNING, "prep DFU: fac nv err, bkup it!\n");

    if(NVRAM_CHECK_VALID())
    {
        if(0 == NVRAM_SAVE_TO_FAC())
        {
            if(NVRAM_CHECK_FAC_VALID())
            {
                ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_PREP_DFU_3, P_WARNING, "prep DFU: and succ!\n");
                return FOTA_EOK;
            }
        }
        else
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_PREP_DFU_4, P_WARNING, "prep DFU: save err!\n");
        }
    }

    retCode = FOTA_EPERM;
#endif

    return retCode;
}

static int32_t fotaNvmClosingDfu(FotaDefClosingDfu_t *clsDfu)
{
#if (FOTA_NVM_BACKUP_MUX_SIZE != 0)
    if(!NVRAM_CHECK_VALID())
    {
        NVRAM_RESTORE_FROM_FAC();

        if(!NVRAM_CHECK_FAC_VALID())
        {
            /* for ver-beta debug */
            fotaDoExtension(FOTA_DEF_WDT_STOP, NULL);
            while(1)
            {
                FotaDefUsDelay_t  delay = {1000000};
                fotaDoExtension(FOTA_DEF_US_DELAY, (void*)&delay);
                ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_CLS_DFU_1, P_WARNING, "cls DFU: restore nv fail!\n");
            }
        }
    }
#endif

    return FOTA_EOK;
}

static int32_t fotaNvmAdjustZoneSize(FotaDefAdjZoneSize_t *adjZone)
{
    if(adjZone->zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << adjZone->zid)))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_ADJ_ZONESZ_1, P_ERROR, "adj zone: no fota zone(%d)! bmZid(0x%x)\n", adjZone->zid, gFotaNvmZoneMan.bmZoneId);
        return FOTA_EUNFOUND;
    }

    uint32_t orginSize = gFotaNvmZoneMan.zone[adjZone->zid].size;
    gFotaNvmZoneMan.zone[adjZone->zid].size = FOTA_MAX(orginSize, adjZone->size);
    if(orginSize != gFotaNvmZoneMan.zone[adjZone->zid].size)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_ADJ_ZONESZ_2, P_INFO, "adj zone(%d): size %d --> %d\n", adjZone->zid, gFotaNvmZoneMan.bmZoneId);
    }
    else
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_ADJ_ZONESZ_3, P_INFO, "adj zone(%d): no need change!\n", adjZone->zid);
    }

    return FOTA_EOK;
}

static int32_t fotaResetParhHashField(CustFotaParHdr_t *parh, int32_t buflen)
{
    if(buflen < sizeof(CustFotaParHdr_t)) return FOTA_EARGS;

    memset(parh->parHash, 0, FOTA_SHA256_HASH_LEN);

    return FOTA_EOK;
}

static int32_t fotaNvmCheckDeltaState(FotaDefChkDeltaState_t *chkDelta)
{
    uint32_t          readLen = 0;
    uint32_t           offset = 0;
    CustFotaParHdr_t     parh;
#ifdef FEATURE_FOTA_SIGN_EANBLE
    int32_t           retCode = FOTA_EUNDEF;
    uint8_t         *signHash = gFotaHash;
    CustFotaAuthHdr_t    pauh;
#endif

    if(chkDelta == NULL) return FOTA_EARGS;

    chkDelta->isValid = 0;
    chkDelta->state   = FOTA_DCS_DELTA_UNFOUND;

#ifdef FEATURE_FOTA_SIGN_EANBLE
    fotaNvmRead(FOTA_NVM_ZONE_DELTA, offset, (uint8_t*)&pauh, sizeof(CustFotaAuthHdr_t));
    if(!FOTA_CHECK_AUTH_MAGIC((uint8_t*)pauh.magic))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_1, P_SIG, "delta: not a auth par! pmagic(%x%x)\n", pauh.magic[0], pauh.magic[1]);
        return FOTA_EAUTH;
    }

    offset = sizeof(CustFotaAuthHdr_t);
#endif

    fotaNvmRead(FOTA_NVM_ZONE_DELTA, offset, (uint8_t*)&parh, sizeof(CustFotaParHdr_t));
    if(!FOTA_CHECK_PAR_MAGIC(parh.pmagic))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_2, P_SIG, "delta: not a par! pmagic(%x%x)\n", parh.pmagic[0], parh.pmagic[1]);
        return FOTA_EPAR;
    }

#ifdef FEATURE_FOTA_FS_ENABLE
    uint32_t deltaSize = fotaNvmFsSize(gFotaNvmZoneMan.zone[FOTA_NVM_ZONE_DELTA].handle);
    if(deltaSize < parh.parLen)
    {
        chkDelta->state = FOTA_DCS_DELTA_PARTIAL;
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_3, P_SIG, "delta: partial(%d) par! real size(%d)\n", deltaSize, parh.parLen);
        return FOTA_EPARSZ;
    }
#endif

#ifdef FEATURE_FOTA_SIGN_EANBLE
    chkDelta->state = FOTA_DCS_DELTA_SIGNFAIL;
    if(pauh.curveType == 1)
    {
        memset(signHash, 0, FOTA_SHA256_HASH_LEN);
        fotaInitChksum(FOTA_CA_SHA224SUM, NULL);
        retCode = FOTA_chksumBufData(FOTA_CA_SHA224SUM, parh.parHash, FOTA_SHA256_HASH_LEN, signHash, 1, NULL);
        fotaDeinitChksum(FOTA_CA_SHA224SUM, NULL);
        if(FOTA_EOK != retCode)
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_4, P_ERROR, "delta: sha224 hash failed(%d)!\n", retCode);
            return retCode;
        }
    }
    else
    {
        signHash = parh.parHash;
    }

    if(VerifySignature(pauh.curveType, signHash, pauh.sign, fotaGetDeltaPubKey()))
    {
        //ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_6, P_ERROR, "delta signature err!\n");
        return FOTA_EVERIFY;
    }

    //ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_7, P_SIG, "delta signature ok!\n");
#endif

    chkDelta->state = FOTA_DCS_DELTA_INVALID;

    memset(gFotaHash, 0, FOTA_SHA256_HASH_LEN);
    fotaInitChksum(FOTA_CA_SHA256SUM, NULL);

    readLen = 2 * FOTA_SHA256_HWALIGN_SIZE;
    if(FOTA_EOK != FOTA_chksumFlashData(FOTA_CA_SHA256SUM, FOTA_NVM_ZONE_DELTA, offset, readLen, &gFotaHash[0], 0, (buf_handle_callback)fotaResetParhHashField))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_8, P_ERROR, "delta: parh chksum calc fail!\n");

        fotaDeinitChksum(FOTA_CA_SHA256SUM, NULL);
        return FOTA_ECHKSUM;
    }

    offset += readLen;
    readLen = parh.parLen - readLen;
    if(FOTA_EOK != FOTA_chksumFlashData(FOTA_CA_SHA256SUM, FOTA_NVM_ZONE_DELTA, offset, readLen, &gFotaHash[0], 1, NULL))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_9, P_ERROR, "delta: par-pl chksum calc fail!\n");

        fotaDeinitChksum(FOTA_CA_SHA256SUM, NULL);
        return FOTA_ECHKSUM;
    }
    fotaDeinitChksum(FOTA_CA_SHA256SUM, NULL);

    if(0 != memcmp(&parh.parHash[0], &gFotaHash[0], FOTA_SHA256_HASH_LEN))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_DELTA_10, P_WARNING, "delta: unmatched chksum! curr/wanted as follows:\n");
        FOTA_dumpOctets(gFotaHash, FOTA_SHA256_HASH_LEN);
        FOTA_dumpOctets(parh.parHash, FOTA_SHA256_HASH_LEN);
        return FOTA_EPAR;
    }

    /* valid *.par */
    chkDelta->isValid = 1;
    chkDelta->state   = FOTA_DCS_DELTA_CHECKOK;

    return FOTA_EOK;
}

PLAT_BL_CIRAM_FLASH_TEXT static int32_t fotaNvmIsImageIdentical(FotaDefIsImageIdentical_t *isIdent)
{
    memset(gFotaHash, 0, FOTA_SHA256_HASH_LEN);

    fotaInitChksum(FOTA_CA_SHA256SUM, NULL);

    if(FOTA_EOK != FOTA_chksumFlashData(FOTA_CA_SHA256SUM, isIdent->zid, 0, isIdent->size, &gFotaHash[0], 1, NULL))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_IS_IDENTICAL_1, P_ERROR, "image(%d): fw chksum calc fail!\n", isIdent->zid);

        fotaDeinitChksum(FOTA_CA_SHA256SUM, NULL);
        return FOTA_ECHKSUM;
    }

    fotaDeinitChksum(FOTA_CA_SHA256SUM, NULL);

    if(0 != memcmp((void*)isIdent->hash, (void*)&gFotaHash[0], FOTA_SHA256_HASH_LEN))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_IS_IDENTICAL_2, P_WARNING, "image(%d): non-identical! curr/wanted as follows:\n", isIdent->zid);
        FOTA_dumpOctets(gFotaHash, FOTA_SHA256_HASH_LEN);
        FOTA_dumpOctets(isIdent->hash, FOTA_SHA256_HASH_LEN);
        return FOTA_EFWNIDENT;
    }

    ECPLAT_PRINTF(UNILOG_FOTA, FOTA_IS_IDENTICAL_3, P_INFO, "image(%d): identical!\n", isIdent->zid);

    return FOTA_EOK;
}

static int32_t fotaNvmCheckBaseImage(FotaDefChkBaseImage_t *chkImage)
{
    uint32_t                  offset = 0;
    uint16_t                bmFwAttr = 0;
    CustFotaParHdr_t            parh;
    CustFotaPkgHdr_t            pkgh;
    FotaDefIsImageIdentical_t  ident;
    FotaDefChkBootState_t       boot;
#ifdef FEATURE_FOTA_SIGN_EANBLE
    CustFotaAuthHdr_t           auth;
#endif

    if(chkImage == NULL) return FOTA_EARGS;

    chkImage->isMatched = 0;

#ifdef FEATURE_FOTA_SIGN_EANBLE
    fotaNvmRead(FOTA_NVM_ZONE_DELTA, offset, (uint8_t*)&auth, sizeof(CustFotaAuthHdr_t));
    if(!FOTA_CHECK_AUTH_MAGIC((uint8_t*)auth.magic))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_IMAGE_0, P_SIG, "delta: not a auth par! pmagic(%x%x)\n", auth.magic[0], auth.magic[1]);
        return FOTA_EAUTH;
    }

    offset = sizeof(CustFotaAuthHdr_t);
#endif

    fotaNvmRead(FOTA_NVM_ZONE_DELTA, offset, (uint8_t*)&parh, sizeof(CustFotaParHdr_t));
    if(!FOTA_CHECK_PAR_MAGIC(parh.pmagic)) return FOTA_EPAR;

    if(!parh.parLen || parh.parLen > fotaNvmGetDeltaSize(1))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_IMAGE_1, P_SIG, "image: *.par len(%d) err, maxsize(%d)\n",
                                                             parh.parLen, fotaNvmGetDeltaSize(1));
        return FOTA_EPARSZ;
    }

    ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_IMAGE_2, P_INFO, "image: *.par pcap(%d)\n", parh.fotaCap);

    offset += sizeof(CustFotaParHdr_t) + (parh.fotaCap ? FOTA_PAR_RETEN_SIZE_16M : FOTA_PAR_RETEN_SIZE_4M);
    for(; offset + sizeof(CustFotaPkgHdr_t) < parh.parLen; offset += pkgh.pkgLen)
    {
        fotaNvmRead(FOTA_NVM_ZONE_DELTA, offset, (uint8_t*)&pkgh, sizeof(CustFotaPkgHdr_t));
        if(!pkgh.pkgLen) return FOTA_EPERM;

        bmFwAttr |= (1 << pkgh.fwAttr);

        ident.zid  = FOTA_convToZoneId(pkgh.fwAttr);
        ident.size = pkgh.baseFwSize;
        ident.hash = pkgh.baseFwHash;
        if(FOTA_EOK != fotaNvmIsImageIdentical(&ident)) return FOTA_EFWNIDENT;
    }

    /* check system head bin exists or not if security boot is enabled! */
    fotaDoExtension(FOTA_DEF_CHK_BOOT_STATE, &boot);
    if(boot.isSigned && (!parh.psigned || !(bmFwAttr & (1 << FOTA_FA_SYSH))))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_CHK_IMAGE_3, P_WARNING, "image: no signed bin(0x%x) err, psigned(%d)!\n", bmFwAttr, parh.psigned);
        return FOTA_EPERM;
    }

    chkImage->isMatched = 1;

    return FOTA_EOK;
}

/*----------------------------------------------------------------------------*
 *                      GLOBAL FUNCTIONS                                      *
 *----------------------------------------------------------------------------*/
/******************************************************************************
 * @brief : fotaNvmInit
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmInit(void)
{
    //if(gFotaNvmZoneMan.bmZoneId) return FOTA_EOK;

#if (FOTA_NVM_A2CP_XIP_ADDR != FOTA_NVM_A2AP_XIP_ADDR)
    if(FOTA_IS_CPFLASH_DISABLED())
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_INIT_1, P_SIG, "cp flash wil be enabled...\n");

        uint32_t mask = SaveAndSetIRQMask();
        BSP_QSPI_ENABLE_CP_FLASH();
        RestoreIRQMask(mask);
    }
#endif

    gFotaNvmZoneMan.bmZoneId = FOTA_NVM_BM_ZONE_NONE;

#ifdef FEATURE_FOTA_FS_ENABLE
    fotaNvmSetZone(FOTA_NVM_ZONE_REMAP, FOTA_NVM_REMAP_ADDR, FOTA_NVM_REMAP_SIZE, 0, FOTA_NVM_A2AP_XIP_ADDR);

#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
    LFS_init();
    int32_t fd = fotaNvmFsOpen(FOTA_DELTA_PAR_NAME, "rd");
#else
    fotaNvmFsRemove(FOTA_DELTA_PAR_NAME);
    fotaNvmClearRemap(0, fotaNvmGetRemapSize(0));
    int32_t fd = fotaNvmFsOpen(FOTA_DELTA_PAR_NAME, "rw+");
#endif
    if(fd < 0)
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_INIT_2, P_WARNING, "init: fd(%d), no *.par?\n", fd);
    }

    fotaNvmSetZone(FOTA_NVM_ZONE_DELTA, fd, FOTA_DELTA_PAR_MAXSIZE, 0, 0);
#else /* NFS */
    fotaNvmSetZone(FOTA_NVM_ZONE_DELTA, FOTA_NVM_DELTA_ADDR, FOTA_NVM_DELTA_SIZE, FOTA_NVM_DELTA_BACKUP_SIZE, FOTA_NVM_A2AP_XIP_ADDR);
#endif
    fotaNvmSetZone(FOTA_NVM_ZONE_BKUP, FOTA_NVM_REAL_BACKUP_ADDR, FOTA_NVM_REAL_BACKUP_SIZE, FOTA_NVM_BACKUP_MUX_SIZE, FOTA_NVM_A2AP_XIP_ADDR);
#if defined CHIP_EC618 || defined CHIP_EC618_Z0 || defined CHIP_EC718 || defined CHIP_EC716
    fotaNvmSetZone(FOTA_NVM_ZONE_AP, FOTA_NVM_AP_LOAD_ADDR, FOTA_NVM_AP_LOAD_SIZE, 0, FOTA_NVM_A2AP_XIP_ADDR);
    fotaNvmSetZone(FOTA_NVM_ZONE_CP, FOTA_NVM_CP_LOAD_ADDR, FOTA_NVM_CP_LOAD_SIZE, 0, FOTA_NVM_A2CP_XIP_ADDR);
    fotaNvmSetZone(FOTA_NVM_ZONE_SYSH, FOTA_NVM_SYSH_LOAD_ADDR, FOTA_NVM_SYSH_LOAD_SIZE, 0, FOTA_NVM_A2AP_XIP_ADDR);
#endif
#ifdef FOTA_CUST_APP_ENABLE
    fotaNvmSetZone(FOTA_NVM_ZONE_APP, FOTA_NVM_APP_LOAD_ADDR, FOTA_NVM_APP_LOAD_SIZE, 0, FOTA_NVM_A2AP_XIP_ADDR);
    //fotaNvmSetZone(FOTA_NVM_ZONE_APP2, FOTA_NVM_APP2_LOAD_ADDR, FOTA_NVM_APP2_LOAD_SIZE, 0, FOTA_NVM_A2AP_XIP_ADDR);
#endif

    return FOTA_EOK;
}

/******************************************************************************
 * @brief : fotaNvmDeinit
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmDeinit(void)
{
#if (FOTA_NVM_A2CP_XIP_ADDR != FOTA_NVM_A2AP_XIP_ADDR)
    ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_DEINIT, P_SIG, "cp flash wil be disabled...\n");

    uint32_t mask = SaveAndSetIRQMask();
    BSP_QSPI_DISABLE_CP_FLASH(); /* dummy API even if cpflash exists! */
    RestoreIRQMask(mask);
#endif

#ifdef FEATURE_FOTA_FS_ENABLE
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
    fotaNvmFsClose(fotaNvmGetHandle(FOTA_NVM_ZONE_DELTA));
    fotaNvmFsRemove(FOTA_DELTA_PAR_NAME);

    LFS_deinit();
#endif
#endif

    /* CAUTION:
     * some zones will be acessed later, e.g. getting DFU result,
     * therefore, NEVER deinit the flags of these zones.
     */
    //gFotaNvmZoneMan.bmZoneId = FOTA_NVM_BM_ZONE_NONE;

    return FOTA_EOK;
}

/******************************************************************************
 * @brief : fotaNvmClear
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmClear(uint32_t zid, uint32_t offset, uint32_t len)
{
#ifdef FEATURE_FOTA_FS_ENABLE
    if(zid == FOTA_NVM_ZONE_DELTA)
    {
        return FOTA_EOK;
    }
    else
#endif
    {
        return fotaNvmNfsClear(zid, offset, len);
    }
}

/******************************************************************************
 * @brief : fotaNvmWrite
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmWrite(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
#ifdef FEATURE_FOTA_FS_ENABLE
    if(zid == FOTA_NVM_ZONE_DELTA)
    {
    #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_WRITE_0, P_WARNING, "dont wr delta! wr remap instead!\n");
        return FOTA_EPERM;
    #else
        int32_t   fd = gFotaNvmZoneMan.zone[FOTA_NVM_ZONE_DELTA].handle;

        if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)) || (fd < 0)) return FOTA_EFLWRITE;

        fotaNvmFsSeek(fd, offset, SEEK_SET);
        fotaNvmFsWrite(buf, bufLen, 1, fd);

        return FOTA_EOK;
    #endif
    }
    else
#endif
    {
        return fotaNvmNfsWrite(zid, offset, buf, bufLen);
    }
}

/******************************************************************************
 * @brief : fotaNvmRead
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmRead(uint32_t zid, uint32_t offset, uint8_t *buf, uint32_t bufLen)
{
#ifdef FEATURE_FOTA_FS_ENABLE
    int32_t   fd = gFotaNvmZoneMan.zone[FOTA_NVM_ZONE_DELTA].handle;

    if(zid == FOTA_NVM_ZONE_DELTA)
    {
        if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)) || (fd < 0)) return FOTA_EFLREAD;

        fotaNvmFsSeek(fd, offset, SEEK_SET);
        fotaNvmFsRead(buf, bufLen, 1, fd);

        return FOTA_EOK;
    }
    else
#endif
    {
        return fotaNvmNfsRead(zid, offset, buf, bufLen);
    }
}

/******************************************************************************
 * @brief : fotaNvmGetZoneId
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
uint32_t fotaNvmGetZoneId(uint32_t addr, uint32_t *size, uint32_t *offset)
{
    uint32_t   zid = FOTA_NVM_ZONE_DELTA;
#ifdef FEATURE_FOTA_FS_ENABLE
    uint32_t   zsz = FOTA_DELTA_PAR_MAXSIZE;
#else
    uint32_t   zsz = FOTA_NVM_DELTA_DOWNLOAD_SIZE;
#endif/*FEATURE_FOTA_FS_ENABLE*/

    if(!size) goto GET_ZID_END;

    if(!addr && !(*size))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_ZID_2, P_SIG, "get zid: set default zone(%d: %d)!\n", zid, zsz);
        goto GET_ZID_END;
    }

    /* no xip offset */
    addr &= (~FLASH_XIP_ADDR);

    switch(addr)
    {
    #ifdef FEATURE_FOTA_FS_ENABLE
        case FOTA_NVM_REMAP_ADDR:
        {
            zid = FOTA_NVM_ZONE_REMAP;
            zsz = FOTA_NVM_REMAP_SIZE;
        }
    #else
        case FOTA_NVM_DELTA_ADDR:
        {
            zid = FOTA_NVM_ZONE_DELTA;
            zsz = FOTA_NVM_DELTA_DOWNLOAD_SIZE;
            break;
        }
    #endif
        case FOTA_NVM_DELTA_BACKUP_ADDR:
        {
            zid = FOTA_NVM_ZONE_BKUP;
            zsz = FOTA_NVM_DELTA_BACKUP_SIZE;
            break;
        }
    #if defined CHIP_EC618 || defined CHIP_EC618_Z0 || defined CHIP_EC718 || defined CHIP_EC716
        case FOTA_NVM_AP_LOAD_ADDR:
        {
            zid = FOTA_NVM_ZONE_AP;
            zsz = FOTA_NVM_AP_LOAD_SIZE;
            break;
        }
        case FOTA_NVM_CP_LOAD_ADDR:
        {
            zid = FOTA_NVM_ZONE_CP;
            zsz = FOTA_NVM_CP_LOAD_SIZE;
            break;
        }
    #endif

        default:
        {
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_ZID_3, P_WARNING, "get zid: unknown starting addr(0x%x)!\n", addr);
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
            ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_ZID_4, P_WARNING, "get zid: in_size(%d) != zone_size(%d)!\n", *size, zsz);
        }
    }

    if(offset) *offset = 0;

    return zid;
}


/******************************************************************************
 * @brief : fotaNvmGetSize
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT uint32_t fotaNvmGetSize(uint32_t zid, uint8_t isOvhdExcl)
{
    if(zid >= FOTA_NVM_ZONE_MAXNUM) return 0;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_SZ_1, P_WARNING, "get size: no fota zone(%d)! bmZid(0x%x)\n",
                                                                  zid, gFotaNvmZoneMan.bmZoneId);
        return 0;
    }

    return (gFotaNvmZoneMan.zone[zid].size - (isOvhdExcl ? gFotaNvmZoneMan.zone[zid].overhead : 0));
}

/******************************************************************************
 * @brief : fotaNvmGetHandle
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmGetHandle(uint32_t zid)
{
    if(zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_HNDL_2, P_WARNING, "get handle: no fota zone(%d)! bmZid(0x%x)\n",
                                                                    zid, gFotaNvmZoneMan.bmZoneId);
        return FOTA_EUNFOUND;
    }

    return gFotaNvmZoneMan.zone[zid].handle;
}

/******************************************************************************
 * @brief : fotaNvmGetExtras
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
PLAT_BL_CIRAM_FLASH_TEXT int32_t fotaNvmGetExtras(uint32_t zid)
{
    if(zid >= FOTA_NVM_ZONE_MAXNUM) return FOTA_EARGS;

    if(!(gFotaNvmZoneMan.bmZoneId & (1 << zid)))
    {
        ECPLAT_PRINTF(UNILOG_FOTA, FOTA_NVM_GET_XTRAS_2, P_WARNING, "get xtras: no fota zone(%d)! bmZid(0x%x)\n",
                                                                     zid, gFotaNvmZoneMan.bmZoneId);
        return FOTA_EUNFOUND;
    }

    return gFotaNvmZoneMan.zone[zid].extras;
}

/******************************************************************************
 * @brief : fotaNvmVerifyDelta
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
int32_t fotaNvmVerifyDelta(uint32_t zid, uint8_t *hash, uint32_t pkgSize, uint32_t *deltaState)
{
    int32_t                    retCode = FOTA_EOK;
    FotaDefChkDeltaState_t    chkDelta = {0};
    FotaDefChkBaseImage_t      chkBase = {0};

    fotaNvmCheckDeltaState(&chkDelta);
    if(!chkDelta.isValid)
    {
        retCode = FOTA_EPAR;
        ECPLAT_PRINTF(UNILOG_FOTA, VERIFY_DELTA_1, P_ERROR, "validate delta err! errno(%d)", chkDelta.state);
        goto VERIFY_DELTA_END;
    }

    ECPLAT_PRINTF(UNILOG_FOTA, VERIFY_DELTA_2, P_SIG, "validate delta ok!");

    fotaNvmCheckBaseImage(&chkBase);
    if(!chkBase.isMatched)
    {
        retCode = FOTA_EFWNIDENT;
        chkDelta.state = FOTA_DCS_DELTA_UNMATCHB;
        ECPLAT_PRINTF(UNILOG_FOTA, VERIFY_DELTA_3, P_WARNING, "however, base fw is unmatched!");
        goto VERIFY_DELTA_END;
    }

    if(deltaState)
    {
        *deltaState = chkDelta.state;
    }

VERIFY_DELTA_END:
#ifdef FEATURE_FOTA_FS_ENABLE
#ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
    fotaNvmFsClose(fotaNvmGetHandle(FOTA_NVM_ZONE_DELTA));
#endif
#endif

    return retCode;
}

/******************************************************************************
 * @brief : fotaNvmGetOtaResult
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
int32_t  fotaNvmGetOtaResult(uint32_t zid, int32_t *deltaState)
{
    FotaDefDfuResult_t  dfuResult = {0};

    if(FOTA_EOK != fotaNvmGetDfuResult(&dfuResult))
    {
        dfuResult.deltaState = FOTA_DCS_DELTA_UNDEF;
    }

    if(deltaState)
    {
        *deltaState = dfuResult.deltaState;
    }

    return dfuResult.dfuResult;
}

/******************************************************************************
 * @brief : fotaNvmDoExtension
 * @author: Xu.Wang
 * @note  :
 ******************************************************************************/
int32_t fotaNvmDoExtension(uint32_t flags, void *args)
{
    int32_t retCode = FOTA_EUNDEF;

    switch(flags)
    {
        case FOTA_DEF_CHK_REMAP_ZONE:
            retCode = fotaNvmCheckRemapZone((FotaDefChkRemapZone_t*)args);
            break;
        case FOTA_DEF_CHK_DELTA_STATE:
            retCode = fotaNvmCheckDeltaState((FotaDefChkDeltaState_t*)args);
            break;
        case FOTA_DEF_CHK_BASE_IMAGE:
            retCode = fotaNvmCheckBaseImage((FotaDefChkBaseImage_t*)args);
            break;
        case FOTA_DEF_IS_IMAGE_IDENTICAL:
            retCode = fotaNvmIsImageIdentical((FotaDefIsImageIdentical_t*)args);
            break;
        case FOTA_DEF_SET_DOWNLOAD_OVER:
            break;
        case FOTA_DEF_SET_DFU_RESULT:
            retCode = fotaNvmSetDfuResult((FotaDefDfuResult_t*)args);
            break;
        case FOTA_DEF_GET_DFU_RESULT:
            retCode = fotaNvmGetDfuResult((FotaDefDfuResult_t*)args);
            break;
        case FOTA_DEF_PREPARE_DFU:
            retCode = fotaNvmPrepareDfu((FotaDefPrepareDfu_t*)args);
            break;
        case FOTA_DEF_CLOSING_DFU:
            retCode = fotaNvmClosingDfu((FotaDefClosingDfu_t*)args);
            break;
        case FOTA_DEF_ADJ_ZONE_SIZE:
            retCode = fotaNvmAdjustZoneSize((FotaDefAdjZoneSize_t*)args);

        default:
            break;
    }

    return retCode;
}


#ifdef __BL_MODE__
uint8_t BSP_QSPI_Write(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
    return FLASH_writeBl(pData, WriteAddr, Size);
}

uint8_t  BSP_QSPI_Erase_Sector(uint32_t SectorAddress)
{
    return FLASH_eraseSectorSafe(SectorAddress);
}
#endif

uint32_t BL_OTAInfoAddress(void) {return (BOOTLOADER_FLASH_LOAD_ADDR + BOOTLOADER_FLASH_LOAD_SIZE);}
uint32_t BL_DFotaAddress(void) {return (FLASH_FOTA_REGION_START);}
uint8_t BL_WriteFlash(uint8_t* pData, uint32_t WriteAddr, uint32_t Size)
{
    return FLASH_writeBl(pData, WriteAddr, Size);
}
#ifdef TYPE_EC718M
uint32_t BL_MemAddress(void) {return (PSRAM_P2_START_ADDR);}
#else
uint32_t BL_MemAddress(void) {return (MSMB_START_ADDR);}
#endif

