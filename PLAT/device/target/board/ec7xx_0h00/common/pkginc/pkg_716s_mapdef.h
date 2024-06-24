#ifndef __PKG_716S_MAPDEF_H__
#define __PKG_716S_MAPDEF_H__

#define AP_FLASH_BASE_LNA 0x800000



//For 716S, CP image in AP Flash
#define CP_PKGIMG_LNA (0x0081a000)

#define BL_PKGIMG_LNA (0x00803000)

#define BOOTLOADER_PKGIMG_LIMIT_SIZE (0x12000)

#if defined  GCF_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x16e000)
#elif defined MID_FEATURE_MODE
#define AP_PKGIMG_LIMIT_SIZE (0x132000)
#elif defined FEATURE_MORERAM_ENABLE
#define AP_PKGIMG_LIMIT_SIZE (0x12E000)
#elif defined FEATURE_MOREROM_ENABLE
#if defined FEATURE_LESSLOG_ENABLE //lesslog
#define AP_PKGIMG_LIMIT_SIZE (0x138000)
#else
#define AP_PKGIMG_LIMIT_SIZE (0x134000)
#endif
#else
#define AP_PKGIMG_LIMIT_SIZE (0x134000)
#endif
#if (defined MID_FEATURE_MODE) || (defined GCF_FEATURE_MODE)
#define AP_PKGIMG_LNA (0x00874000)
#define CP_PKGIMG_LIMIT_SIZE (0x5a000)
#elif defined FEATURE_MORERAM_ENABLE
#define AP_PKGIMG_LNA (0x00878000)
#define CP_PKGIMG_LIMIT_SIZE (0x5E000)
#elif defined FEATURE_MOREROM_ENABLE
#if defined FEATURE_LESSLOG_ENABLE //lesslog
#define AP_PKGIMG_LNA (0x0086e000)
#define CP_PKGIMG_LIMIT_SIZE (0x54000)
#else
#define AP_PKGIMG_LNA (0x00872000)
#define CP_PKGIMG_LIMIT_SIZE (0x58000)
#endif
#else
#define AP_PKGIMG_LNA (0x00872000)
#define CP_PKGIMG_LIMIT_SIZE (0x58000)
#endif

#endif

