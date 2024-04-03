#ifndef __PKG_716E_MAPDEF_H__
#define __PKG_716E_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000



//For 716E, CP image in AP Flash
#define CP_PKGIMG_LNA (0x0081a000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x11000)

#define AP_PKGIMG_LNA (0x0087e000)

#ifdef __USER_MAP_CONF_FILE__
#include __USER_MAP_CONF_FILE__
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x2c2000)
#endif
#endif

#define CP_PKGIMG_LIMIT_SIZE (0x64000)


#endif

