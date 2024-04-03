
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

/*

0x00000000          |---------------------------------|
                    |      header1 4KB                |
0x00001000          |---------------------------------|
                    |      header2 4KB                |
0x00002000          |---------------------------------|
                    |      fuse mirror 4KB            |
0x00003000          |---------------------------------|
                    |      bl 72KB                    |
0x00015000          |---------------------------------|
                    |      rel data(factory)20KB      |
0x0001a000          |---------------------------------|

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
                    |      cp img 640KB               |
0x000Ba000          |---------------------------------|
                    |      app img 2616KB             |
#else
                    |      cp img 400KB               |
0x0007e000          |---------------------------------|
                    |      app img 2856KB             |
#endif
0x00348000          |---------------------------------|
                    |      fota 260KB  (164KB可用)    |     因增大fs导致fota缩小至260KB
0x00389000          |---------------------------------|
                    |      fs 256KB                    |    此demo增大fs至256KB
0x003c9000          |---------------------------------|
                    |      kv  64KB                   |
0x003d9000          |---------------------------------|
                    |      hib backup 96KB            |
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
#define AP_FLASH_LOAD_ADDR              (0x008BA000)
#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x28E000)//2616KB
#endif
#else
#define AP_FLASH_LOAD_ADDR              (0x0087e000)
#ifndef AP_FLASH_LOAD_SIZE
#define AP_FLASH_LOAD_SIZE              (0x2ca000)//2856KB
#endif
#endif

#ifndef FULL_OTA_SAVE_ADDR
#define FULL_OTA_SAVE_ADDR              (0x0)
#endif
#define AP_FLASH_LOAD_UNZIP_SIZE        (AP_FLASH_LOAD_SIZE + 0x10000)//AP_FLASH_LOAD_SIZE+64KB ,for ld


//fota addr and size
#ifndef FLASH_FOTA_REGION_START
#define FLASH_FOTA_REGION_START         (0x348000)
#define FLASH_FOTA_REGION_LEN           (0x41000)//260KB
#define FLASH_FOTA_REGION_END           (0x389000)
#endif


//fs addr and size
#ifndef FLASH_FS_REGION_START
#define FLASH_FS_REGION_START           (0x389000)
#define FLASH_FS_REGION_END             (0x3c9000)
#endif
#define FLASH_FS_REGION_SIZE            (FLASH_FS_REGION_END-FLASH_FS_REGION_START) //256KB


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

#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x2ca000)
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

#else
    #error "Need define chip type"
#endif

#endif
