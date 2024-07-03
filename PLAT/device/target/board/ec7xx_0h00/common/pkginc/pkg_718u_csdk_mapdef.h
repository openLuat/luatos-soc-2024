#ifndef __PKG_718U_MAPDEF_H__
#define __PKG_718U_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000

#define AP_PKGIMG_LNA   (0x008c8000)

//For 718U, CP image in AP Flash
#define CP_PKGIMG_LNA (0x00828000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x1f000)


#ifdef __USER_MAP_CONF_FILE__
#include __USER_MAP_CONF_FILE__
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE              (0x600000)//6M
#endif
#endif


#define CP_PKGIMG_LIMIT_SIZE (0xA0000)

#endif

