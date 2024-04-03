#ifndef __PKG_718U_MAPDEF_H__
#define __PKG_718U_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000

#define AP_PKGIMG_LNA   (0x008c8000)

//For 718U, CP image in AP Flash
#define CP_PKGIMG_LNA (0x00828000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x20000)


#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
#define AP_PKGIMG_LIMIT_SIZE              (0x651000 - FLASH_EXCEP_DUMP_SIZE)//6468KB - 16KB
#else
#define AP_PKGIMG_LIMIT_SIZE              (0x651000)//6468KB
#endif


#define CP_PKGIMG_LIMIT_SIZE (0xA0000)

#endif

