#ifndef __COMMON_H__
#define __COMMON_H__


//#include "ARMCM3.h"
#include "ec7xx.h"

#define ISRAM_PHY_ADDR 0x04000000

#define TICK_LOAD_VALUE 0x800000  //256 seconds for tick period


#define LOG_ON      1


#ifdef EC_ASSERT
#undef EC_ASSERT
#endif
#define EC_ASSERT(x,v1,v2,v3)

#if (FOTA_PRESET_RAM_ENABLE == 1)
extern int EC_Printf(const char * pFormat, ...);
extern int EC_Sprintf(char *pBuf, const char * pFormat, ...);

#define BL_LOGI(fmt,args...)         EC_Printf(fmt, ##args)
#if (LOG_ON == 1)
#define BL_TRACE(fmt,args...)        EC_Printf(fmt, ##args)
#else
#define BL_TRACE(fmt,args...)
#endif
#define BL_SPRINTF(buf,args...)      EC_Sprintf(buf, ##args)

#ifdef ECPLAT_PRINTF
#undef ECPLAT_PRINTF
#endif
#define ECPLAT_PRINTF(moduleId, subId, debugLevel, format, ...)    EC_Printf(format, ##__VA_ARGS__)

#else
#define BL_LOGI(fmt,args...)         printf(fmt, ##args)
#define BL_TRACE(fmt,args...)        printf(fmt, ##args)
#define BL_SPRINTF(buf,args...)      sprintf(buf, ##args)

#ifdef ECPLAT_PRINTF
#undef ECPLAT_PRINTF
#endif
#define ECPLAT_PRINTF(moduleId, subId, debugLevel, format, ...)    printf(format, ##__VA_ARGS__)

#endif



extern void trace(char*log,int len);
extern void show_err(uint32_t err);
void uDelay(void);

//#define QSPI_DRIVER_ORG
#endif
