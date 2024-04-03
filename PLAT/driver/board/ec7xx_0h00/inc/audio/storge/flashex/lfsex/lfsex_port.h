#ifndef LFSEX_PORT_H
#define LFSEX_PORT_H

#include "lfs.h"
#include "lfs_port.h"
#include "cmsis_os2.h"

#ifdef __cplusplus
extern "C"
{
#endif


int LFSEX_getFileWriteMonitorResult(file_operation_statistic_result_t* result);
int LFSEX_getBlockEraseCountResult(uint32_t* result);
void LFSEX_resetMonitorResult(void);

void LFSEX_adjustDaemonTaskPriority(void);

// Initialize
int LFSEX_init(void);

// Deinit
void LFSEX_deinit(void);

// Wrapper of lfs_format
int LFSEX_format(void);

// Wrapper of lfs_stat
int LFSEX_stat(const char *path, struct lfs_info *info);

// Wrapper of lfs_remove
int LFSEX_remove(const char *path);

// Wrapper of lfs_rename
int LFSEX_rename(const char *oldpath, const char *newpath);

// Wrapper of lfs_file_open
int LFSEX_fileOpen( lfs_file_t *file, const char *path, int flags);

// Wrapper of lfs_file_close
int LFSEX_fileClose(lfs_file_t *file);

// Wrapper of lfs_file_read
lfs_ssize_t LFSEX_fileRead(lfs_file_t *file, void *buffer, lfs_size_t size);

// Wrapper of lfs_file_write
lfs_ssize_t LFSEX_fileWrite(lfs_file_t *file, const void *buffer, lfs_size_t size);

// Wrapper of lfs_file_sync
int LFSEX_fileSync(lfs_file_t *file);

// Wrapper of lfs_file_seek
lfs_soff_t LFSEX_fileSeek(lfs_file_t *file, lfs_soff_t off, int whence);

// Wrapper of lfs_file_truncate
int LFSEX_fileTruncate(lfs_file_t *file, lfs_off_t size);

// Wrapper of lfs_file_tell
lfs_soff_t LFSEX_fileTell(lfs_file_t *file);

// Wrapper of lfs_file_rewind
int LFSEX_fileRewind(lfs_file_t *file);

// Wrapper of lfs_file_size
lfs_soff_t LFSEX_fileSize(lfs_file_t *file);

// Wrapper of lfs_dir_open
int LFSEX_dirOpen(lfs_dir_t *dir, const char *path);

// Wrapper of lfs_dir_close
int LFSEX_dirClose(lfs_dir_t *dir);

// Wrapper of lfs_dir_read
int LFSEX_dirRead(lfs_dir_t *dir, struct lfs_info *info);

// Get fs status
int LFSEX_statfs(lfs_status_t *status);

// Callback func, running in LFS daemon task
int LFSEX_blockCallback(lfs_callback_func callback, void *arg);

// Non-thread-safe version API, used internally
int LFSEX_removeUnsafe(const char *path);

int LFSEX_fileOpenUnsafe(lfs_file_t *file, const char *path, int flags);

int LFSEX_fileCloseUnsafe(lfs_file_t *file);

lfs_ssize_t LFSEX_fileReadUnsafe(lfs_file_t *file, void *buffer, lfs_size_t size);

lfs_ssize_t LFSEX_fileWriteUnsafe(lfs_file_t *file, const void *buffer, lfs_size_t size);

lfs_soff_t LFSEX_fileSeekUnsafe(lfs_file_t *file, lfs_soff_t off, int whence);

int LFSEX_fileSyncUnsafe(lfs_file_t *file);

int LFSEX_fileTruncateUnsafe(lfs_file_t *file, lfs_off_t size);

lfs_soff_t LFSEX_fileTellUnsafe(lfs_file_t *file);

int LFSEX_fileRewindUnsafe(lfs_file_t *file);

lfs_soff_t LFSEX_fileSizeUnsafe(lfs_file_t *file);

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif
