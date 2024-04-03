#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include "cmsis_os2.h"
#include "ostask.h"
#include "osasys.h"
#include "bsp.h"
#include "bsp_custom.h"
#ifdef FEATURE_SUBSYS_MODE_ENABLE
#include "mode.h"
#endif
#ifdef FEATURE_SUBSYS_MEDIA_AUDIO_ENABLE
#include "audio.h"
#endif
#include "storage.h"
#include DEBUG_LOG_HEADER_FILE

#include "ff.h"

FATFS fsobject;
BYTE work[FF_MAX_SS];

void subStorageInit(void)
{
#ifdef FEATURE_SUBSYS_SDCARD_ENABLE
    if (sdInit() == 0)
    {
        // Init FAT32
        FRESULT  res ;  
        res = f_mount(&fsobject,  "0:",  1); 

        if(res == FR_NO_FILESYSTEM) 
        {
            printf("res = %d\r\n",res);
            res = f_mount(NULL,  "0:",  1); 
            // res = f_mount(&fsobject,  "0:",  1);
 	    }else
        {
            printf("f_mount  is  OK\r\n");
        }
    }else
        printf("sdInit err\r\n");
#endif
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    if (spiFlashInit() == 0)
    {
        LFSEX_init();
    }
#endif

}

bool flashexIsReady(void)
{
    bool retVal = false;

#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    if (spiFlashInfoGet() != NULL)
    {
        spiIoInit();
        retVal = true;
    }
    else
#endif

    {

        printf("No external Flash handle.\r\n");
    }

    return retVal;
}

bool pathPrefixIsValid(char *path)
{
    bool retVal = false;

    if (!((path[0] == 'c') || (path[0] == 'C') || (path[0] == 'd') || (path[0] == 'D') || (path[0] == 'r') || (path[0] == 'R')))
    {
        printf("Unsupported Flash partition. path : %s, partition: %c\r\n", path, path[0]);
        goto labelEnd;
    }

    if (!((path[1] == ':') && (path[2] == '/')))
    {
        printf("path format error. path : %s\r\n", path);

        goto labelEnd;
    }

    retVal = true;

labelEnd:
    return retVal;
}

int32_t fsFileRemove(char *path)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathPrefixIsValid(path) != true)
    {
        goto labelEnd;
    }

    switch (path[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_remove(&path[3]);
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_remove(&path[3]);
            }
#endif
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int32_t fsFileOpen(lfs_file_t *file, char *path, int32_t flags)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathPrefixIsValid(path) != true)
    {
        goto labelEnd;
    }

    switch (path[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_fileOpen(file, &path[3], flags);
            file->partition = FLASH_PARTITION_C;
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
            	ECPLAT_PRINTF(UNILOG_PLAT_MEDIA, fsFileOpen, P_INFO, "flashexIsReady");
                retVal = LFSEX_fileOpen(file, &path[3], flags);
                file->partition = FLASH_PARTITION_D;
            }
#endif
            break;
        case 'r':
        case 'R':
            memcpy(&(file->ctz.size), &path[7], 4);
            file->name      = &path[3];
            file->pos       = 0;
            file->partition = FLASH_PARTITION_R;
            retVal          = LFS_ERR_OK;
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int32_t fsFileClose(lfs_file_t *file)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileClose(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileClose(file);
            }
#endif
            break;
        case FLASH_PARTITION_R:
            file->name     = NULL;
            file->ctz.size = 0;
            file->pos      = 0;
            retVal         = LFS_ERR_OK;
            break;

        default:
            break;
    }

    return retVal;
}

lfs_ssize_t fsFileRead(lfs_file_t *file, void *buffer, lfs_size_t size)
{
    int32_t  retVal  = LFS_ERR_INVAL;
    uint32_t address = 0;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileRead(file, buffer, size);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileRead(file, buffer, size);
            }
#endif
            break;

        case FLASH_PARTITION_R:
            if (file->name != NULL)
            {
                memcpy(&address, file->name, sizeof(address));
                if ((file->pos + size) > file->ctz.size)
                {
                    size = file->ctz.size - file->pos;
                }
                memcpy(buffer, (void *)(address + file->pos), size);
                file->pos += size;
                retVal     = size;
            }
            break;

        default:
            break;
    }

    return retVal;
}

lfs_ssize_t fsFileWrite(lfs_file_t *file, void *buffer, lfs_size_t size)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileWrite(file, buffer, size);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileWrite(file, buffer, size);
            }
#endif
            break;

        default:
            break;
    }

    return retVal;
}

lfs_soff_t fsFileSeek(lfs_file_t *file, lfs_soff_t off, int whence)
{
    int32_t  retVal  = LFS_ERR_INVAL;
    uint32_t address = 0;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileSeek(file, off, whence);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileSeek(file, off, whence);
            }
#endif
            break;

        case FLASH_PARTITION_R:
            if (file->name != NULL)
            {
                switch (whence)
                {
                    case LFS_SEEK_SET:
                        memcpy(&address, file->name, sizeof(address));
                        file->pos = off;
                        retVal    = file->pos;
                        break;

                    case LFS_SEEK_CUR:
                        file->pos += off;
                        retVal     = file->pos;
                        break;

                    case LFS_SEEK_END:
                        file->pos = file->ctz.size - off;
                        retVal    = file->pos;
                        break;

                    default:
                        break;
                }
            }
            break;

        default:
            break;
    }

    return retVal;
}

lfs_soff_t fsFileTell(lfs_file_t *file)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileTell(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileTell(file);
            }
#endif
            break;

        case FLASH_PARTITION_R:
            if (file->name != NULL)
            {
                retVal = file->pos;
            }
            break;

        default:
            break;
    }

    return retVal;
}

int32_t fsFileRewind(lfs_file_t *file)
{
    int32_t  retVal  = LFS_ERR_INVAL;
    uint32_t address = 0;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileRewind(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileRewind(file);
            }
#endif
            break;

        case FLASH_PARTITION_R:
            if (file->name != NULL)
            {
                file->pos = address;
                retVal    = LFS_ERR_OK;
            }
            break;

        default:
            break;
    }

    return retVal;
}

lfs_soff_t fsFileSize(lfs_file_t *file)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileSize(file);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileSize(file);
            }
#endif
            break;

        case FLASH_PARTITION_R:
            if (file->name != NULL)
            {
                retVal = file->ctz.size;
            }
            break;

        default:
            break;
    }

    return retVal;
}

lfs_soff_t fsFileTruncate(lfs_file_t *file, lfs_size_t size)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (file->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_fileTruncate(file, size);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_fileTruncate(file, size);
            }
#endif
            break;

        default:
            break;
    }

    return retVal;
}

int32_t fsDirOpen(lfs_dir_t *dir, char *path)
{
    int32_t retVal = LFS_ERR_INVAL;

    if (pathPrefixIsValid(path) != true)
    {
        goto labelEnd;
    }

    switch (path[0])
    {
        case 'c':
        case 'C':
            retVal = LFS_dirOpen(dir, &path[2]);
            dir->partition = FLASH_PARTITION_C;
            break;

        case 'd':
        case 'D':
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_dirOpen(dir, &path[2]);
                dir->partition = FLASH_PARTITION_D;
            }
#endif
            break;

        default:
            break;
    }

labelEnd:
    return retVal;
}

int32_t fsDirClose(lfs_dir_t *dir)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (dir->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirClose(dir);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_dirClose(dir);
            }
#endif
            break;

        default:
            break;
    }

    return retVal;
}

int32_t fsDirRead(lfs_dir_t *dir, struct lfs_info *info)
{
    int32_t retVal = LFS_ERR_INVAL;

    switch (dir->partition)
    {
        case FLASH_PARTITION_C:
            retVal = LFS_dirRead(dir, info);
            break;

        case FLASH_PARTITION_D:
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
            if (flashexIsReady() == true)
            {
                retVal = LFSEX_dirRead(dir, info);
            }
#endif
            break;

        default:
            break;
    }

    return retVal;
}

int32_t fsStatFs(lfs_status_t *status)
{
    int32_t retVal = LFS_ERR_INVAL;

    retVal = LFS_statfs(status);
    if (retVal != LFS_ERR_OK)
    {
        goto labelEnd;
    }
    
#ifdef FEATURE_SUBSYS_FLASHEX_ENABLE
    if (flashexIsReady() == true)
    {
        retVal = LFSEX_statfs(status + 1);
        if (retVal != LFS_ERR_OK)
        {
            goto labelEnd;
        }
    }
#endif

labelEnd:
    return retVal;
}
