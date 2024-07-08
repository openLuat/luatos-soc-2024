
#ifndef MEM_MAP_7XX_H
#define MEM_MAP_7XX_H

/*
    自行修改分区有风险,请认真检查分区修改合理性并做好测试！！！！！！！
    自行修改分区有风险,请认真检查分区修改合理性并做好测试！！！！！！！
    自行修改分区有风险,请认真检查分区修改合理性并做好测试！！！！！！！

    依据给出的分区表,自行计算所要修改的分区,并修改对应的宏地址!
*/

#if defined TYPE_EC718S

/*2M flash only, no psram*/

/*
AP/CP flash layout, toatl 2MB
flash raw address: 0x00000000---0x00200000
flash xip address(from ap/cp view): 0x00800000---0x00a00000


0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      BL 72KB                    |
0x00015000          |---------------------------------|
                    |      rel (factory) 20KB         |----compress
#if (defined MID_FEATURE_MODE) || (defined GCF_FEATURE_MODE)
0x0001a000          |---------------------------------|
                    |      cp img 360KB               |
0x00074000          |---------------------------------|
                    |      ap img 1224KB              |
#else               
0x0001a000          |---------------------------------|
                    |      cp img 352KB               |
0x00072000          |---------------------------------|
                    |      ap img 1232KB              |
#endif              
0x001a6000          |---------------------------------|
                    |      lfs 48KB                   |
0x001b2000          |---------------------------------|
                    |      fota 256KB                 |
0x001f2000          |---------------------------------|
                    |      rel 52KB                   |
0x001ff000          |---------------------------------|
                    |      plat config 4KB            |---- read-modify-write
0x00200000          |---------------------------------|


*/

/* -------------------------------flash  address define-------------------------*/


#ifndef AP_FLASH_LOAD_SIZE
#ifdef MID_FEATURE_MODE
#define AP_FLASH_LOAD_SIZE              (0x132000)//1232KB-8KB for CP
#define AP_FLASH_LOAD_UNZIP_SIZE        (0x140000)//1288KB-8KB for CP,for ld
#else
#define AP_FLASH_LOAD_SIZE              (0x134000)//1232KB
#define AP_FLASH_LOAD_UNZIP_SIZE        (0x142000)//1288KB ,for ld
#endif
#else
#define AP_FLASH_LOAD_UNZIP_SIZE        (AP_FLASH_LOAD_SIZE + 0x10000)//AP_FLASH_LOAD_SIZE+64KB ,for ld
#endif //undef AP_FLASH_LOAD_SIZE


/*temp add here, need handle from caller !!!!*/
//hib bakcup addr and size but not used

#define FLASH_HIB_BACKUP_EXIST          (0)
#define FLASH_MEM_BACKUP_ADDR           (0)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0)
#define FLASH_MEM_BACKUP_SIZE           (0)
#define FLASH_MEM_BLOCK_SIZE            (0)
#define FLASH_MEM_BLOCK_CNT             (0)

//fs addr and size
#define FLASH_FS_REGION_START           (0x1a6000)
#define FLASH_FS_REGION_END             (0x1b2000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) // 48KB

#define FLASH_FOTA_REGION_START         (0x1b2000)
#define FLASH_FOTA_REGION_LEN           (0x40000)//256KB
#define FLASH_FOTA_REGION_END           (0x1f2000)

#if 0	//需要HIB参考这个配置
#define FLASH_HIB_BACKUP_EXIST          (1)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x1a6000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
#define FLASH_MEM_BLOCK_SIZE            (0x6000)
#define FLASH_MEM_BLOCK_CNT             (0x4)

//fs addr and size
#define FLASH_FS_REGION_START           (0x1be000)
#define FLASH_FS_REGION_END             (0x1ca000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) // 48KB

#define FLASH_FOTA_REGION_START         (0x1ca000)
#define FLASH_FOTA_REGION_LEN           (0x28000)   //160KB
#define FLASH_FOTA_REGION_END           (0x1f2000)

#endif

// mapdef
#ifndef AP_PKGIMG_LIMIT_SIZE
#if defined  GCF_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x16e000)
#elif defined MID_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x132000)
#else
#define AP_PKGIMG_LIMIT_SIZE (0x134000)
#endif
#endif //undefined AP_PKGIMG_LIMIT_SIZE

#elif defined TYPE_EC718P

/* 4MB flash + 2MB psram*/

/*
flash layout, toatl 4MB
flash raw address: 0x00000000---0x00400000
flash xip address(from both ap/cp view): 0x00800000---0x00c00000

0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      bl 68KB + 4KB              |
0x00015000          |---------------------------------|
                    |      rel data(factory)20KB      |
0x0001a000          |---------------------------------|

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
                    |      cp img 640KB               |
0x000Ba000          |---------------------------------|
                    |      app img 2584KB             |
#else
                    |      cp img 400KB               |
0x0007e000          |---------------------------------|
                    |      app img 2826KB             |
#endif
0x00340000          |---------------------------------|
                    |      fota 420KB(324KB can use)  |
0x003a9000          |---------------------------------|
                    |      lfs  128KB                 |
0x003c9000          |---------------------------------|
                    |      kv   64KB                  |
0x003d9000          |---------------------------------|
                    |      hib   96KB                 |
0x003f1000          |---------------------------------|
                    |      rel data 52KB              |
0x003fe000          |---------------------------------|
                    |      plat config 8KB            |
0x00400000          |---------------------------------|


*/

#define BOOTLOADER_FLASH_LOAD_SIZE              (0x11000)//68kB, real region size, tool will check when zip TODO:ZIP
#define BOOTLOADER_FLASH_LOAD_UNZIP_SIZE        (0x18000)//96KB ,for ld

//ap image addr and size
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x286000)//2584KB
#endif
#else
#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x2c2000)//2824KB
#endif
#endif

#ifndef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR              (0x0)
#endif
#define AP_FLASH_LOAD_UNZIP_SIZE        (AP_FLASH_LOAD_SIZE + 0x10000)//AP_FLASH_LOAD_SIZE+64KB ,for ld


//fota addr and size
#define FLASH_FOTA_REGION_START         (0x340000)
#define FLASH_FOTA_REGION_LEN           (0x69000)//420KB
#define FLASH_FOTA_REGION_END           (0x3a9000)


//fs addr and size
#define FLASH_FS_REGION_START           (0x3a9000)
#define FLASH_FS_REGION_END             (0x3c9000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) //128KB


//fskv addr and size
#define FLASH_FDB_REGION_START			(0x3c9000)//64KB
#define FLASH_FDB_REGION_END            (0x3d9000)

//hib bakcup addr and size
#define FLASH_HIB_BACKUP_EXIST          (1)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x3d9000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
#define FLASH_MEM_BLOCK_SIZE            (0x6000)
#define FLASH_MEM_BLOCK_CNT             (0x4)

// mapdef
#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x11000)

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x286000)
#endif
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x2c2000)
#endif
#endif // (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)

#elif defined TYPE_EC718U

/* 8MB flash + 2MB psram*/

/*
flash layout, toatl 8MB
flash raw address: 0x00000000---0x00800000
flash xip address(from both ap/cp view): 0x00800000---0x01000000

0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      bl 124KB + 4KB             |
0x00023000          |---------------------------------|
                    |      rel data(factory)20KB      |
0x00028000          |---------------------------------|
                    |      cp img 640KB               |
0x000C8000          |---------------------------------|
                    |      app img 6144KB             |
0x006C8000          |---------------------------------|
                    |      fota 600KB                 |
0x0075E000          |---------------------------------|
                    |      lfs  428KB                 |
0x007C9000          |---------------------------------|
                    |      kv   64KB                  |
0x007D9000          |---------------------------------|
                    |      hib backup 96KB            |
0x007f1000          |---------------------------------|
                    |      rel data 52KB              |
0x007fe000          |---------------------------------|
                    |      plat config 8KB            |
0x00800000          |---------------------------------|


*/

#define BOOTLOADER_FLASH_LOAD_SIZE              (0x1f000)//128kB, real region size, tool will check when zip TODO:ZIP
#define BOOTLOADER_FLASH_LOAD_UNZIP_SIZE        (0x22000)//136KB ,for ld

//ap image addr and size
#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x600000)//6m
#endif
#ifndef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR              (0x0)
#endif

#define AP_FLASH_LOAD_UNZIP_SIZE        (0x6D6000)//7000KB ,for ld

//fota addr and size
#define FLASH_FOTA_REGION_START         (0x6C8000)
#define FLASH_FOTA_REGION_LEN           (0x96000)//600KB
#define FLASH_FOTA_REGION_END           (0x75E000)


//fs addr and size
#define FLASH_FS_REGION_START           (0x75E000)
#define FLASH_FS_REGION_END             (0x7C9000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) // 428KB


//fskv addr and size
#define FLASH_FDB_REGION_START			(0x7C9000)//64KB
#define FLASH_FDB_REGION_END            (0x7d9000)

//hib bakcup addr and size
#define FLASH_HIB_BACKUP_EXIST          (1)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x7d9000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
#define FLASH_MEM_BLOCK_SIZE            (0x6000)
#define FLASH_MEM_BLOCK_CNT             (0x4)

// mapdef
#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x1f000)

#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE              (0x600000)//6M
#endif

#elif defined TYPE_EC716S

/*2M flash only, no psram*/

/*
AP/CP flash layout, toatl 2MB
flash raw address: 0x00000000---0x00200000
flash xip address(from ap/cp view): 0x00800000---0x00a00000


0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      BL 72KB                    |
0x00015000          |---------------------------------|
                    |      rel (factory) 20KB         |----compress
#if (defined MID_FEATURE_MODE) || (defined GCF_FEATURE_MODE)
0x0001a000          |---------------------------------|
                    |      cp img 360KB               |
0x00074000          |---------------------------------|
                    |      ap img 1224KB              |
#else               
0x0001a000          |---------------------------------|
                    |      cp img 352KB               |
0x00072000          |---------------------------------|
                    |      ap img 1232KB              |
#endif              
0x001a6000          |---------------------------------|
                    |      lfs 48KB                   |
0x001b2000          |---------------------------------|
                    |      fota 256KB                 |
0x001f2000          |---------------------------------|
                    |      rel 52KB                   |
0x001ff000          |---------------------------------|
                    |      plat config 4KB            |---- read-modify-write
0x00200000          |---------------------------------|


*/

/* -------------------------------flash  address define-------------------------*/

#ifndef AP_FLASH_LOAD_SIZE
#ifdef MID_FEATURE_MODE
#define AP_FLASH_LOAD_SIZE              (0x132000)//1232KB-8KB for CP
#define AP_FLASH_LOAD_UNZIP_SIZE        (0x140000)//1288KB-8KB for CP,for ld
#else
#define AP_FLASH_LOAD_SIZE              (0x134000)//1232KB
#define AP_FLASH_LOAD_UNZIP_SIZE        (0x142000)//1288KB ,for ld
#endif
#else
#define AP_FLASH_LOAD_UNZIP_SIZE        (AP_FLASH_LOAD_SIZE + 0x10000)//AP_FLASH_LOAD_SIZE+64KB ,for ld
#endif //undef AP_FLASH_LOAD_SIZE

/*temp add here, need handle from caller !!!!*/
//hib bakcup addr and size but not used

#define FLASH_HIB_BACKUP_EXIST          (0)
#define FLASH_MEM_BACKUP_ADDR           (0)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0)
#define FLASH_MEM_BACKUP_SIZE           (0)
#define FLASH_MEM_BLOCK_SIZE            (0)
#define FLASH_MEM_BLOCK_CNT             (0)

//fs addr and size
#define FLASH_FS_REGION_START           (0x1a6000)
#define FLASH_FS_REGION_END             (0x1b2000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) // 48KB

#define FLASH_FOTA_REGION_START         (0x1b2000)
#define FLASH_FOTA_REGION_LEN           (0x40000)//256KB
#define FLASH_FOTA_REGION_END           (0x1f2000)

#if 0	//需要HIB参考这个配置
#define FLASH_HIB_BACKUP_EXIST          (1)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x1a6000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
#define FLASH_MEM_BLOCK_SIZE            (0x6000)
#define FLASH_MEM_BLOCK_CNT             (0x4)

//fs addr and size
#define FLASH_FS_REGION_START           (0x1be000)
#define FLASH_FS_REGION_END             (0x1ca000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) // 48KB

#define FLASH_FOTA_REGION_START         (0x1ca000)
#define FLASH_FOTA_REGION_LEN           (0x28000)   //160KB
#define FLASH_FOTA_REGION_END           (0x1f2000)

#endif

// mapdef
#ifndef AP_PKGIMG_LIMIT_SIZE
#if defined  GCF_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x16e000)
#elif defined MID_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x132000)
#else
#define AP_PKGIMG_LIMIT_SIZE (0x134000)
#endif
#endif //undefined AP_PKGIMG_LIMIT_SIZE

#elif defined TYPE_EC716E

/* 4MB flash + no psram*/

/*
flash layout, toatl 4MB
flash raw address: 0x00000000---0x00400000
flash xip address(from both ap/cp view): 0x00800000---0x00c00000

0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      bl 68KB + 4KB              |
0x00015000          |---------------------------------|
                    |      rel data(factory)20KB      |
0x0001a000          |---------------------------------|
                    |      cp img 400KB               |
0x0007e000          |---------------------------------| user can config from here
                    |      app img 2824KB             |
0x00340000          |---------------------------------|
                    |      fota 420KB(324KB can use)  |
0x003a9000          |---------------------------------|
                    |      lfs  128KB                 |
0x003c9000          |---------------------------------|
                    |      kv   64KB                  |
0x003d9000          |---------------------------------|
                    |      hib   96KB                 |
0x003f1000          |---------------------------------| to here by mem_map_7xx.h
                    |      rel data 52KB              |
0x003fe000          |---------------------------------|
                    |      plat config 8KB            |
0x00400000          |---------------------------------|


*/

/* -------------------------------flash  address define-------------------------*/

#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x2c2000)//2824KB
#endif
#ifndef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR              (0x0)
#endif

#define AP_FLASH_LOAD_UNZIP_SIZE        (AP_FLASH_LOAD_SIZE + 0x20000)//3172KB ,for ld

//fs addr and size
#define FLASH_FOTA_REGION_START         (0x340000)
#define FLASH_FOTA_REGION_LEN           (0x69000)//420KB
#define FLASH_FOTA_REGION_END           (0x3a9000)

#define FLASH_FS_REGION_START           (0x3a9000)
#define FLASH_FS_REGION_END             (0x3c9000)
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) //128KB

#define FLASH_FDB_REGION_START			(0x3c9000)//64KB
#define FLASH_FDB_REGION_END            (0x3d9000)

//hib bakcup addr and size
#define FLASH_HIB_BACKUP_EXIST          (1)
#define FLASH_MEM_BACKUP_ADDR           (AP_FLASH_XIP_ADDR+FLASH_MEM_BACKUP_NONXIP_ADDR)
#define FLASH_MEM_BACKUP_NONXIP_ADDR    (0x389000)
#define FLASH_MEM_BACKUP_SIZE           (0x18000)//96KB
#define FLASH_MEM_BLOCK_SIZE            (0x6000)
#define FLASH_MEM_BLOCK_CNT             (0x4)

// mapdef
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x2c2000)
#endif

#else
    #error "Need define chip type"
#endif

#endif
