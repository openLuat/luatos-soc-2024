/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    storage.h
 * Description:  EC718 at command demo entry header file
 * History:      Rev1.0   2018-07-12
 *
 ****************************************************************************/
#ifndef  SUBSYS_STORAGE_H
#define  SUBSYS_STORAGE_H

#ifdef __cplusplus
extern "C" {
#endif


#include <stdint.h>
#include <stdbool.h>
#ifdef FEATURE_SUBSYS_LFSEX_ENABLE
#include "lfsex_port.h"
#else
#include "lfs_port.h"
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
#include "flashex.h"
#endif
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
#include "sdcard.h"
#endif


typedef enum
{
    FLASH_PARTITION_C = 0,      /* Internal Flash */
    FLASH_PARTITION_D = 1,      /* External Flash */
    FLASH_PARTITION_E = 2,      /* sdcard */
    FLASH_PARTITION_R = 10,     /* ROM or RAM */
} FlashPartitionT;


void        subStorageInit(void);
bool        flashexIsReady(void);
bool        pathPrefixIsValid(char *path);
int32_t     fsFileRemove(char *path);
int32_t     fsFileOpen(lfs_file_t *file, char *path, int32_t flags);
int32_t     fsFileClose(lfs_file_t *file);
lfs_ssize_t fsFileRead(lfs_file_t *file, void *buffer, lfs_size_t size);
lfs_ssize_t fsFileWrite(lfs_file_t *file, void *buffer, lfs_size_t size);
lfs_soff_t  fsFileSeek(lfs_file_t *file, lfs_soff_t off, int whence);
lfs_soff_t  fsFileTell(lfs_file_t *file);
int32_t     fsFileRewind(lfs_file_t *file);
lfs_soff_t  fsFileSize(lfs_file_t *file);
int32_t     fsFileTruncate(lfs_file_t *file, lfs_size_t size);
int32_t     fsDirOpen(lfs_dir_t *dir, char *path);
int32_t     fsDirClose(lfs_dir_t *dir);
lfs_ssize_t fsDirRead(lfs_dir_t *dir, struct lfs_info *info);
int32_t     fsStatFs(lfs_status_t *status);

#ifdef __cplusplus
}
#endif

#endif /* SUBSYS_STORAGE_H */