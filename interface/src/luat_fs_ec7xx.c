/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "common_api.h"
#include "FreeRTOS.h"
#include "luat_base.h"
#include "luat_fs.h"
#include "osasys.h"
#include "lfs_port.h"
#include "luat_rtos.h"
#include "luat_debug.h"

#define LUAT_LOG_TAG "fs"
#include "luat_log.h"

#ifdef __LUATOS__
#include "luat_malloc.h"
#include "mem_map.h"
#else
#include "luat_mem.h"
#endif


static inline const char* check_path(const char* path) {
    // if (path == NULL || strlen(path) < 1)
    //     return NULL;
    // const char* dst = path;
    // if (dst[0] == '/')
    //     dst = path + 1;
    return path;
}

static inline int is_file(const char* path) {
    if (path == NULL || strlen(path) < 1 || path[strlen(path) - 1] == '/')
        return 0;
    return 1;
} 


// #ifdef LUAT_USE_FS_VFS
// #if 1

FILE* luat_vfs_ec7xx_fopen(__attribute__((unused)) void* userdata, const char *filename, const char *mode) {
    int flag = 0;
    const char* dst = check_path(filename);
    if (dst == NULL || is_file(dst) == 0)
        return NULL;
    //LLOGD("luat_fs_fopen %s %s", filename, mode);
    lfs_file_t *file = (lfs_file_t*)luat_heap_malloc(sizeof(lfs_file_t));
    if (NULL == file) {
        //LLOGE("out of memory when open file");
        return NULL;
    }
    memset(file, 0, sizeof(lfs_file_t));
/*
"r": 读模式（默认）；
"w": 写模式；
"a": 追加模式；
"r+": 更新模式，所有之前的数据都保留；
"w+": 更新模式，所有之前的数据都删除；
"a+": 追加更新模式，所有之前的数据都保留，只允许在文件尾部做写入。
*/
    if (!strcmp("r+", mode) || !strcmp("r+b", mode) || !strcmp("rb+", mode)) {
        flag = LFS_O_RDWR | LFS_O_CREAT;
    }
    else if(!strcmp("w+", mode) || !strcmp("w+b", mode) || !strcmp("wb+", mode)) {
        flag = LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC;
    }
    else if(!strcmp("a+", mode) || !strcmp("a+b", mode) || !strcmp("ab+", mode)) {
        flag = LFS_O_RDWR | LFS_O_APPEND | LFS_O_CREAT;
    }
    else if(!strcmp("w", mode) || !strcmp("wb", mode)) {
        flag = LFS_O_RDWR | LFS_O_CREAT | LFS_O_TRUNC;
    }
    else if(!strcmp("r", mode) || !strcmp("rb", mode)) {
        flag = LFS_O_RDONLY;
    }
    else if(!strcmp("a", mode) || !strcmp("ab", mode)) {
        flag = LFS_O_WRONLY | LFS_O_APPEND | LFS_O_CREAT;
    }
    else {
        //LLOGW("bad file open mode %s, fallback to 'r'", mode);
        flag = LFS_O_RDONLY;
    }
    int ret = LFS_fileOpen(file, dst, flag);
    //LLOGD("luat_fs_fopen %s %s ret %d", filename, mode, ret);
    if (ret == LFS_ERR_OK) {
        return (FILE*)file;
    }
    luat_heap_free(file);
    return NULL;
}

int luat_vfs_ec7xx_getc(__attribute__((unused))void* userdata, FILE* stream) {
    char buff[1];
    buff[0] = 0;
    int ret = LFS_fileRead((lfs_file_t*)stream, buff, 1);
    if (ret == 1)
        return buff[0];
    return -1;
}

int luat_vfs_ec7xx_fseek(__attribute__((unused))void* userdata, FILE* stream, long int offset, int origin) {
    //DBG("luat_fs_fseek fd=%p offset=%ld ori=%ld", stream, offset, origin);
    int ret = LFS_fileSeek((lfs_file_t*)stream, offset, origin);
    return ret < 0 ? -1 : 0;
}

int luat_vfs_ec7xx_ftell(__attribute__((unused))void* userdata, FILE* stream) {
    //DBG("luat_fs_ftell fd=%ld", stream);
    int ret = LFS_fileTell((lfs_file_t *)stream);
    return ret < 0 ? -1 : ret;
}

int luat_vfs_ec7xx_fclose(__attribute__((unused))void* userdata, FILE* stream) {
    if (stream == NULL)
        return 0;
    LFS_fileClose((lfs_file_t *)stream);
    free(stream);
    return 0;
}
int luat_vfs_ec7xx_feof(__attribute__((unused))void* userdata, FILE* stream) {
    return LFS_fileTell((lfs_file_t *)stream) == LFS_fileSize((lfs_file_t *)stream) ? 1 : 0;
}
int luat_vfs_ec7xx_ferror(__attribute__((unused))void* userdata, __attribute__((unused))FILE *stream) {
    return 0;
}
size_t luat_vfs_ec7xx_fread(__attribute__((unused))void* userdata, void *ptr, size_t size, size_t nmemb, FILE *stream) {
    lfs_ssize_t ret = LFS_fileRead((lfs_file_t*)stream, ptr, size * nmemb);
    //DBG("luat_fs_fread fd=%p size=%ld nmemb=%ld ret=%ld", stream, size, nmemb, t);
    //DBG("luat_fs_fread data[0-7] %p %X %X %X %X %X %X %X %X", data, *(data), *(data+1), *(data+2), *(data+3), *(data+4), *(data+5), *(data+6), *(data+7));
    if (ret < 0)
        return 0;
    return ret;
}
size_t luat_vfs_ec7xx_fwrite(__attribute__((unused))void* userdata, const void *ptr, size_t size, size_t nmemb, FILE *stream) {
    //DBG("luat_fs_fwrite fd=%p size=%ld nmemb=%ld", stream, size, nmemb);
    lfs_ssize_t ret = LFS_fileWrite((lfs_file_t*)stream, ptr, size * nmemb);
    if (ret < 0)
        return 0;
    return ret;
}
int luat_vfs_ec7xx_remove(__attribute__((unused))void* userdata, const char *filename) {
    const char* dst = check_path(filename);
    if (dst == NULL)
        return 0;
    return LFS_remove(dst);
}
int luat_vfs_ec7xx_rename(__attribute__((unused))void* userdata, const char *old_filename, const char *new_filename) {
    const char* src = check_path(old_filename);
    if (src == NULL)
        return 0;
    const char* dst = check_path(new_filename);
    if (dst == NULL)
        return 0;
    return LFS_rename(src, dst);
}
int luat_vfs_ec7xx_fexist(__attribute__((unused))void* userdata, const char *filename) {
    const char* dst = check_path(filename);
    if (dst == NULL)
        return 0;
    lfs_file_t file = {0};
    int ret = LFS_fileOpen(&file, dst, LFS_O_RDONLY);
    if (ret == LFS_ERR_OK) {
        LFS_fileClose(&file);
        return 1;
    }
    return 0;
}

size_t luat_vfs_ec7xx_fsize(__attribute__((unused))void* userdata, const char *filename) {
    const char* dst = check_path(filename);
    if (dst == NULL)
        return 0;
    lfs_file_t file = {0};
    int ret = LFS_fileOpen(&file, dst, LFS_O_RDONLY);
    if (ret == LFS_ERR_OK) {
        size_t sz = LFS_fileSize(&file);
        LFS_fileClose(&file);
        return sz;
    }
    return 0;
}

int luat_vfs_ec7xx_truncate(__attribute__((unused))void* userdata, const char* filename, size_t len) {
    const char* dst = check_path(filename);
    if (dst == NULL)
        return -1;
    lfs_file_t file = {0};
    int ret = LFS_fileOpen(&file, dst, LFS_O_RDWR);
    if (ret != LFS_ERR_OK) {
        return -2;
    }
    ret = LFS_fileTruncate(&file, len);
    LFS_fileClose(&file);
    return ret;
}

int luat_vfs_ec7xx_mkfs(__attribute__((unused))void* userdata, __attribute__((unused))luat_fs_conf_t *conf) {
    LFS_format();
    return 0;
}
int luat_vfs_ec7xx_mount(__attribute__((unused))void** userdata, __attribute__((unused))luat_fs_conf_t *conf) {
    //DBG("not support yet : mount");
    return 0;
}
int luat_vfs_ec7xx_umount(__attribute__((unused))void* userdata, __attribute__((unused))luat_fs_conf_t *conf) {
    //DBG("not support yet : umount");
    return 0;
}

int LFS_mkdir(const char* dir);
int luat_vfs_ec7xx_mkdir(__attribute__((unused))void* userdata, __attribute__((unused))char const* _DirName) {
    const char* dir = check_path(_DirName);
    char buff[64] = {0};
    //LUAT_DEBUG_PRINT("mkdir %s %s", dir, _DirName);
    if (dir == NULL)
        return -1;
    size_t len = strlen(dir);
    if (strlen(dir) > 63 || strlen(dir) < 2) {
        return -2;
    }
    memcpy(buff, dir, len);
    if (buff[len - 1] == '/')
        buff[len - 1] = 0x00;
    int ret = LFS_mkdir(buff);
    //LUAT_DEBUG_PRINT("mkdir %s %s %d", dir, _DirName, ret);
    return ret;
}
int luat_vfs_ec7xx_rmdir(__attribute__((unused))void* userdata, __attribute__((unused))char const* _DirName) {
    const char* dir = check_path(_DirName);
    if (dir == NULL)
        return -1;
    return LFS_remove(dir);
}

int luat_vfs_ec7xx_dexist(__attribute__((unused))void* userdata, char const* dir_name) {
    int ret = 0;
    lfs_dir_t dir;
    const char* dirpath = check_path(dir_name);
    if (dirpath == NULL)
        return 0;
    ret = LFS_dirOpen(&dir, dirpath);
    if (ret < 0) {
        // LLOGE("no such dir %s _DirName");
        return 0;
    }
    LFS_dirClose(&dir);
    return 1;
}

int luat_vfs_ec7xx_lsdir(__attribute__((unused))void* userdata, char const* dir_name, luat_fs_dirent_t* ents, size_t offset, size_t len) {
    int ret = 0;
    size_t num = 0;
    lfs_dir_t *dir = NULL;
    struct lfs_info info = {0};
    const char* dirpath = check_path(dir_name);
    // if (fs->filecount > offset) {
        // if (offset + len > fs->filecount)
            // len = fs->filecount - offset;
        dir = luat_heap_malloc(sizeof(lfs_dir_t));
        if (dir == NULL) {
            // LLOGE("out of memory when lsdir");
            return 0;
        }
        ret = LFS_dirOpen(dir, dirpath); // 固定值, 因为不支持文件夹
        if (ret < 0) {
            luat_heap_free(dir);
            // LLOGE("no such dir %s _DirName");
            return 0;
        }

        // TODO 使用seek/tell组合更快更省
        for (size_t i = 0; i < offset; i++)
        {
            ret = LFS_dirRead(dir, &info);
            if (ret <= 0) {
                LFS_dirClose(dir);
                luat_heap_free(dir);
                return 0;
            }
        }

        while (num < len)
        {
            ret = LFS_dirRead(dir, &info);
            if (ret < 0) {
                LFS_dirClose(dir);
                luat_heap_free(dir);
                return 0;
            }
            if (ret == 0) {
                break;
            }
            if (info.type == 2 && (memcmp(info.name, ".", 2) ==0 ||memcmp(info.name, "..", 3)==0))
                continue;
            ents[num].d_type = info.type - 1; // lfs file =1, dir=2
            strcpy(ents[num].d_name, info.name);
            num++;
        }
        LFS_dirClose(dir);
        luat_heap_free(dir);
        return num;
    // }
    // return 0;
}

int luat_vfs_ec7xx_info(__attribute__((unused))void* userdata, __attribute__((unused))const char* path, luat_fs_info_t *conf) {
    lfs_status_t status = {0};
    int ret = LFS_statfs(&status);
    if (ret == LFS_ERR_OK) {
        conf->total_block = status.total_block;
        conf->block_used = status.block_used;
        conf->block_size = status.block_size;
        conf->type = 1; // 片上FLASH
        memcpy(conf->filesystem, "lfs", 3);
        conf->filesystem[4] = 0;
        return 0;
    }
    // else {
    //     DBG("LFS_Statfs return %d", ret);
    // }
    return -1;
}
__attribute__((weak)) int luat_fs_dexist(char const *dir){
    return luat_vfs_ec7xx_dexist(NULL, dir);
}
#ifdef LUAT_USE_FS_VFS
#define T(name) .name = luat_vfs_ec7xx_##name
const struct luat_vfs_filesystem vfs_fs_ec7xx = {
    .name = "ec7xx",
    .opts = {
        T(mkfs),
        T(mount),
        T(umount),
        T(mkdir),
        T(rmdir),
        T(remove),
        T(rename),
        T(fsize),
        T(fexist),
        T(info),
        T(lsdir)
    },
    .fopts = {
        T(fopen),
        T(getc),
        T(fseek),
        T(ftell),
        T(fclose),
        T(feof),
        T(ferror),
        T(fread),
        T(fwrite)
    }
};

extern const struct luat_vfs_filesystem vfs_fs_lfs2;
#ifdef __LUATOS__
extern const struct luat_vfs_filesystem vfs_fs_luadb;
extern const struct luat_vfs_filesystem vfs_fs_ram;
extern size_t luat_luadb_act_size;
#endif

static int fs_inited = 0;
int luat_fs_init(void) {
    if (fs_inited)
        return 0;
    fs_inited = 1;
    luat_vfs_init(NULL);
    luat_vfs_reg(&vfs_fs_ec7xx);
    luat_vfs_reg(&vfs_fs_lfs2);
#ifdef __LUATOS__
	luat_vfs_reg(&vfs_fs_luadb);
    luat_vfs_reg(&vfs_fs_ram);
#endif

	luat_fs_conf_t conf = {
		.busname = "",
		.type = "ec7xx",
		.filesystem = "ec7xx",
		.mount_point = ""
	};
	luat_fs_mount(&conf);

#ifdef __LUATOS__
    luat_fs_conf_t conf3 = {
		.busname = NULL,
		.type = "ram",
		.filesystem = "ram",
		.mount_point = "/ram/"
	};
	luat_fs_mount(&conf3);
#endif

#ifdef __LUATOS__
    #define LUADB_ADDR ((uint32_t)LUA_SCRIPT_ADDR | AP_FLASH_XIP_ADDR)
    luat_luadb_act_size = LUA_SCRIPT_OTA_ADDR - LUA_SCRIPT_ADDR;
    //DBG("luadb tmp addr %p", LUADB_ADDR);
	luat_fs_conf_t conf2 = {
		.busname = (char*)(const char*)LUADB_ADDR,
		.type = "luadb",
		.filesystem = "luadb",
		.mount_point = "/luadb/",
	};
	luat_fs_mount(&conf2);
#endif
	return 0;
}


// extern luat_vfs_mount_t * getmount(const char* filename);
// extern int luat_vfs_lfs2_truncate(void* userdata, const char *filename, size_t len);

// int luat_fs_truncate(const char* filename, size_t len) {
//     luat_vfs_mount_t *mount = getmount(filename);
//     if (mount == NULL ) return -1;
//     if (strcmp(mount->fs->name, "lfs2") == 0) {
//         return luat_vfs_lfs2_truncate(mount->userdata, filename + strlen(mount->prefix), len);
//     }else if(strcmp(mount->fs->name, "ec7xx") == 0){
//         return luat_vfs_ec7xx_truncate(NULL, filename, len);
//     }
//     return -1;
// }

#endif
