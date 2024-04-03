#ifndef __COMMON_H__
#define __COMMON_H__


//#include "ARMCM3.h"
#include "ec7xx.h"

#define ISRAM_PHY_ADDR 0x04000000

#define TICK_LOAD_VALUE 0x800000  //256 seconds for tick period

#ifdef EC_ASSERT
#undef EC_ASSERT
#endif
#define EC_ASSERT(x,v1,v2,v3)

#ifdef ECPLAT_PRINTF
#undef ECPLAT_PRINTF
#endif

extern int EC_Printf(const char * pFormat, ...);
#define ECPLAT_PRINTF(moduleId, subId, debugLevel, format, ...)    EC_Printf(format, ##__VA_ARGS__)

extern void trace(char*log,int len);
extern void show_err(uint32_t err);
void uDelay(void);

//#define QSPI_DRIVER_ORG
#endif
