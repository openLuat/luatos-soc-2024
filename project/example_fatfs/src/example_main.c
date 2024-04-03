/*
 * Copyright (c) 2022 OpenLuat & AirM2M
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
#include "luat_rtos.h"
#include "luat_debug.h"

#include "luat_gpio.h"
#include "luat_spi.h"
#include "luat_fs.h"
#include "bsp_custom.h"
#include "luat_mem.h"
#include "luat_crypto.h"
#include "ff.h"
#include "diskio.h"

#define SPI_TF_SPI_ID 1                 // SPIID
#define SPI_TF_CS_PIN	HAL_GPIO_24		// 如果不需要低功耗可以选用普通GPIO
#define SPI_TF_POWER_PIN HAL_GPIO_28	// 如果没有电源控制脚可以写0xff，强烈建议加入电源控制，如果不需要低功耗可以选用普通GPIO

extern BYTE FATFS_DEBUG;                // debug log, 0 -- disable , 1 -- enable
extern BYTE FATFS_POWER_PIN;            // POWER PIN
extern uint16_t FATFS_POWER_DELAY;      // POWER DELAY
DRESULT diskio_open_spitf(BYTE pdrv, void* userdata);


luat_rtos_task_handle fatfs_task_handle;

static int print_fs_info(const char* dir_path)
{
    luat_fs_info_t fs_info = {0};

    luat_fs_info(dir_path, &fs_info);
    // 打印文件系统空间信息
    LUAT_DEBUG_PRINT("fs_info %s %d %d %d %d", 
        fs_info.filesystem, 
        fs_info.type, 
        fs_info.total_block, 
        fs_info.block_used, 
        fs_info.block_size);
}

static int recur_fs(const char* dir_path)
{
    luat_fs_dirent_t *fs_dirent = LUAT_MEM_MALLOC(sizeof(luat_fs_dirent_t)*100);
    memset(fs_dirent, 0, sizeof(luat_fs_dirent_t)*100);

    int lsdir_cnt = luat_fs_lsdir(dir_path, fs_dirent, 0, 100);

    if (lsdir_cnt > 0)
    {
        char path[255] = {0};

        LUAT_DEBUG_PRINT("dir_path=%s, lsdir_cnt=%d", dir_path, lsdir_cnt);

        for (size_t i = 0; i < lsdir_cnt; i++)
        {
            memset(path, 0, sizeof(path));            

            switch ((fs_dirent+i)->d_type)
            {
            // 文件类型
            case 0:   
                snprintf(path, sizeof(path)-1, "%s%s", dir_path, (fs_dirent+i)->d_name);             
                LUAT_DEBUG_PRINT("\tfile=%s, size=%d", path, luat_fs_fsize(path));
                break;
            case 1:
                snprintf(path, sizeof(path)-1, "%s/%s/", dir_path, (fs_dirent+i)->d_name);
                recur_fs(path);
                break;

            default:
                break;
            }
        }        
    }

    LUAT_MEM_FREE(fs_dirent);
    fs_dirent = NULL;
    
    return lsdir_cnt;
}

// 演示文件操作, luat_fs_XXX 文件函数
void exmaple_fs_luat_file(void) {
    FILE* fp = NULL;
    uint8_t *buff = NULL;
    const char* filepath = "/tf/lfs_test.txt";
    const char* newpath = "/tf/newpath";
    char tmp[100];
    int ret = 0;
    size_t size = 0;

    LUAT_DEBUG_PRINT("check file exists? %s", filepath);
    // 通过获取大小, 判断文件是否存在, 存在就删除之
    if (luat_fs_fexist(filepath)) {
        LUAT_DEBUG_PRINT("remove %s", filepath);
        luat_fs_remove(filepath);
    }

    //----------------------------------------------
    //            文件写入演示
    //----------------------------------------------
    // 开始写入文件
    LUAT_DEBUG_PRINT("test lfs file write");
    fp = luat_fs_fopen(filepath, "wb+");
    if (!fp) {
        LUAT_DEBUG_PRINT("file open failed %s", filepath);
        return;
    }
    // 产生随机数据, 模拟业务写入
    LUAT_DEBUG_PRINT("call malloc and rngGenRandom");
    buff = malloc(24 * 100);
    if (buff == NULL) {
        LUAT_DEBUG_PRINT("out of memory ?");
        luat_fs_fclose(fp);
        goto exit;
    }
    luat_crypto_trng(buff, 24 * 50);
    // 按块写入数据
    LUAT_DEBUG_PRINT("call luat_fs_write");
    for (size_t i = 0; i < 24; i++)
    {
        ret = luat_fs_fwrite((const void*)(buff + i * 100), 100, 1, fp);
        if (ret < 0) {
            LUAT_DEBUG_PRINT("fail to write ret %d", ret);
            luat_fs_fclose(fp);
            goto exit;
        }
    }
    // 关闭文件
    luat_fs_fclose(fp);

    //----------------------------------------------
    //            文件读取演示
    //----------------------------------------------
    // 读取文件
    fp = luat_fs_fopen(filepath, "r");
    if (!fp) {
        LUAT_DEBUG_PRINT("file open failed %s", filepath);
        goto exit;
    }
    for (size_t i = 0; i < 24; i++)
    {
        ret = luat_fs_fread(tmp, 100, 1, fp);
        if (ret < 0) {
            LUAT_DEBUG_PRINT("fail to write ret %d", ret);
            luat_fs_fclose(fp);
            goto exit;
        }
        if (memcmp(tmp, buff + i * 100, 100) != 0) {
            LUAT_DEBUG_PRINT("file data NOT match");
        }
    }
    // 直接定位 offset=100的位置, 重新读取
    luat_fs_fseek(fp, 100, SEEK_SET);
    ret = luat_fs_fread(tmp, 100, 1, fp);
    if (memcmp(tmp, buff + 100, 100) != 0) {
        LUAT_DEBUG_PRINT("file data NOT match at offset 100");
    }
    ret = luat_fs_ftell(fp);
    if (ret != 200) {
        // 按前面的逻辑, 先设置到100, 然后读取100, 当前偏移量应该是200
        LUAT_DEBUG_PRINT("file seek NOT match at offset 200");
    }
    
    // 关闭句柄
    luat_fs_fclose(fp);

    //----------------------------------------------
    //            文件截断演示
    //----------------------------------------------

    // 直接用路径裁剪
    luat_fs_truncate(filepath, 300);	//fatfs不支持，所以是无效的

    //----------------------------------------------
    //            文件改名演示
    //----------------------------------------------
    // 文件还在, 测试改名
    luat_fs_remove(newpath);
    luat_fs_rename(filepath, newpath);
    // 读取文件, 老路径应该得到文件大小不大于0
    
    size = luat_fs_fsize(filepath);
    if (size > 0) {
        LUAT_DEBUG_PRINT("file shall not exist");
    }
    // 读取文件, 新路径应该得到文件大小等于300
    size = luat_fs_fsize(newpath);
    if (size != 300) {
        LUAT_DEBUG_PRINT("file shall 300 byte but %d", size);
    }

    luat_fs_remove(filepath);
    luat_fs_remove(newpath);

    //------------------------------------------------------
    // 演示完毕, 清理资源
    exit:
        if (buff != NULL) {
            free(buff);
        }
        LUAT_DEBUG_PRINT("file example exited");
        return;
}
// 演示文件夹操作
void exmaple_fs_lfs_dir(void)
{
    int ret=-1;
    ret=luat_fs_mkdir("/tf/luatos");
    LUAT_DEBUG_PRINT("mkdir result%d",ret);
    if (0==ret)
    {
       LUAT_DEBUG_PRINT("mkdir succeed");
    }
    FILE* fp = NULL;
    uint8_t *buff = NULL;
    const char* filepath = "/tf/luatos/luatos_test.txt";
    fp = luat_fs_fopen(filepath, "wb+");
    if (!fp)
    {
       LUAT_DEBUG_PRINT("file open failed %s", filepath);
       return;
    }
    luat_fs_fclose(fp);
}


static FATFS *fs = NULL;		/* FatFs work area needed for each volume */
extern const struct luat_vfs_filesystem vfs_fs_fatfs;
static void task_test_fatfs(void *param)
{
    FATFS_DEBUG = 0;
    FATFS_POWER_PIN = SPI_TF_POWER_PIN;
    FATFS_POWER_DELAY = 200;
	luat_spi_t fatfs_spi_flash = {
	        .id = SPI_TF_SPI_ID,
	        .CPHA = 0,
	        .CPOL = 0,
	        .dataw = 8,
	        .bit_dict = 0,
	        .master = 1,
	        .mode = 1,
	        .bandrate = 400000,
	        .cs = SPI_TF_CS_PIN,
	};
	luat_gpio_cfg_t gpio_cfg;
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
	luat_fs_init();
    int re = -1;
    luat_spi_setup(&fatfs_spi_flash);
    luat_gpio_set_default_cfg(&gpio_cfg);
    gpio_cfg.pin = SPI_TF_POWER_PIN;
    gpio_cfg.output_level = 0;
    luat_gpio_open(&gpio_cfg);
    gpio_cfg.pin = SPI_TF_CS_PIN;
    gpio_cfg.output_level = 1;
    luat_gpio_open(&gpio_cfg);

    luat_fatfs_spi_t *spit;

	spit = calloc(sizeof(luat_fatfs_spi_t), 1);
	spit->type = 0;
	spit->spi_id = SPI_TF_SPI_ID;
	spit->spi_cs = SPI_TF_CS_PIN;
	spit->fast_speed = 25600000;

    if(NULL == fs)
        fs = calloc(sizeof(FATFS), 1);
    re = diskio_open_spitf(0, (void*)spit);
    if (re != 0)
    {
        LUAT_DEBUG_PRINT("fatfs_init error is %d\n", re);
        while (1)
        {
            luat_rtos_task_sleep(1000);
        }
    }
    
    re = f_mount(fs, "spi", 1);

    if (re != 0)
    {
        LUAT_DEBUG_PRINT("fatfs_init error is %d\n", re);
        while (1)
        {
            luat_rtos_task_sleep(1000);
        }
    }

    luat_vfs_reg(&vfs_fs_fatfs);
    luat_fs_conf_t conf2 = {
			.busname = (char*)fs,
			.type = "fatfs",
			.filesystem = "fatfs",
			.mount_point = "/tf",
		};
	luat_fs_mount(&conf2);
    print_fs_info("/tf");
    exmaple_fs_lfs_dir();
    exmaple_fs_luat_file();
    recur_fs("/");
    recur_fs("/tf");
    while (1)
    {
        luat_rtos_task_sleep(1000);
    }
}


static void task_demo_fatfs(void)
{
    luat_rtos_task_create(&fatfs_task_handle, 4096, 20, "fatfs", task_test_fatfs, NULL, NULL);
}

INIT_TASK_EXPORT(task_demo_fatfs,"1");