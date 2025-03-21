#ifndef __PKG_718PM_MAPDEF_H__
#define __PKG_718PM_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000

//For 718P, CP image in AP Flash
#define CP_PKGIMG_LNA (0x0081e000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x13000)

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#define AP_PKGIMG_LNA (0x008Be000)
#define CP_PKGIMG_LIMIT_SIZE (0xA0000)
#else
#define AP_PKGIMG_LNA (0x00882000)
#define CP_PKGIMG_LIMIT_SIZE (0x64000)
#endif

#ifdef __USER_MAP_CONF_FILE__
#include __USER_MAP_CONF_FILE__
#else

#ifdef __LUATOS__
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x263000)
#endif
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x29F000)
#endif
#endif // (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#else
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x289000)
#endif
#else
#ifndef AP_PKGIMG_LIMIT_SIZE
#define AP_PKGIMG_LIMIT_SIZE (0x2c5000)
#endif
#endif // (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#endif //__LUATOS__
#endif //__USER_MAP_CONF_FILE__

#define XPKGDBLSH_VIRTIMG_MERGE_LNA    (0x0)
#define XPKGDSYSH_VIRTIMG_MERGE_LNA    (0x1000)
#define XPKGDPRM_VIRTIMG_MERGE_LNA  (0x0)
#define XPKGDCMN_VIRTIMG_MERGE_LNA  (0x0)
#define XPKG_VIRTIMG_LOAD_SIZE      (0x100000)

#endif

