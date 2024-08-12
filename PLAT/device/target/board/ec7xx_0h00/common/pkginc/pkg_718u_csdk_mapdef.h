#ifndef __PKG_718U_MAPDEF_H__
#define __PKG_718U_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000

//For 718U, CP image in AP Flash
#define CP_PKGIMG_LNA (0x00828000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x1f000)


#ifdef __USER_MAP_CONF_FILE__
#include __USER_MAP_CONF_FILE__
#else

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x600000)
#endif
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x63c000)
#endif
#endif // (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)

#endif

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#define AP_PKGIMG_LNA (0x008c8000)
#define CP_PKGIMG_LIMIT_SIZE (0xA0000)
#else
#define AP_PKGIMG_LNA (0x0088c000)
#define CP_PKGIMG_LIMIT_SIZE    (0x64000)
#endif

#endif

