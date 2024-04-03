/******************************************************************************

 *(C) Copyright 2018 AirM2M International Ltd.

 * All Rights Reserved

 ******************************************************************************
 *  Filename:pcache.h
 *
 *  Description:EC718 pcache header file
 *
 *  History: 09/13/2022    Originated by pxu
 *
 *  Notes:
 *
 ******************************************************************************/

#ifndef _PCACHE_H
#define _PCACHE_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include "ec7xx.h"

#ifdef __cplusplus
extern "C" {
#endif

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/
//#define PCACHE_USE_INTERRUPT
//#define PCACHE_AUTO_MAINT_OFF

#define HBURST_FORCE_OVERITE_DAT_NONCACHEABLE_NONBUFFABLE         (0x7F030000)
#define HBURST_FORCE_OVERITE_DAT_NONCACHEABLE_BUFFABLE            (0x7F070000)
#define HBURST_FORCE_OVERITE_DAT_CACHEABLE_NONBUFFABLE            (0x7F7B0000)
#define HBURST_FORCE_OVERITE_DAT_CACHEABLE_BUFFABLE               (0x7F7F0000)
#define HBURST_FORCE_OVERITE_DAT_CACHEABLE_BUFFABLE_NONOVERWRITE  (0X04050000)


#define HBURST_FORCE_OVERITE_INS_NONCACHEABLE_NONBUFFABLE         (0x00007F03)
#define HBURST_FORCE_OVERITE_INS_NONCACHEABLE_BUFFABLE            (0x00007F07)
#define HBURST_FORCE_OVERITE_INS_CACHEABLE_NONBUFFABLE            (0x00007F7B)
#define HBURST_FORCE_OVERITE_INS_CACHEABLE_BUFFABLE               (0x00007F7F)

/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/
#define PCACHE_ENABLED_STATUS					(0x1<<0)
#define PCACHE_EN_DIS_ONGONG_STATUS				(0x1<<1)
#define PCACHE_MAINT_ONGOING_STATUS				(0x1<<2)
#define PCACHE_POWERMAINT_ONGOING_STATUS		(0x1<<3)
#define PCACHE_ISCLEAN_STATUS					(0x1<<8)

#define PCACHE_CACHELINE_SIZE       (32)

#define PCACHE_ENABLED              (1)
#define PCACHE_DISABLE              (0)

typedef enum
{
	PCACHE_CLEAN 		= 1,
	PCACHE_INVALIDATE	= 2,
	PCACHE_FLUSH		= 3
}PCacheMaint_Type;


typedef struct
{
	uint32_t endianness					: 2;
	uint32_t xom						: 1;
	uint32_t snapShot					: 1;
	uint32_t cacheSize					: 8;
	uint32_t masterId 					: 8;
	uint32_t disCacheEnMaint			: 1;
	uint32_t disCacheDisMaint			: 1;
	uint32_t disPowerDownMaint			: 1;
	uint32_t powerOnEnable				: 1;
	uint32_t apbViolationResp 			: 1;
	uint32_t ahbViolationResp			: 1;
}PCacheHwInfo_Type;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/
int PCacheInit(void);
void PcacheSetHburstForceReg(uint8_t type, uint32_t regVal);
void PcacheclrAllHburstForceReg(void);
void PCacheEnableBitSet(void);
void PCacheDisableBitSet(void);
void EnablePCache(void);
void DisablePCache(void);
uint32_t PCacheGetHwInfo(void);
void EnablePCacheForceWt(void);
void EnablePCacheWB(void);
BOOL IsPCacheEnabled(void);
BOOL isPCacheClean(void);
uint32_t PCacheGetStatus(void);
BOOL PCacheCleanAll(void);
BOOL PCacheInvalidateAll(void);
BOOL PCacheFlushAll(void);
BOOL PCacheMaintenanceAll(PCacheMaint_Type maintType);
BOOL PCacheCleanByAddress(uint32_t address);
BOOL PCacheInvalidateByAddress(uint32_t address);
BOOL PCacheFlushByAddress(uint32_t address);
BOOL PCacheMaintenanceByAddress(uint32_t address, PCacheMaint_Type maintType);
void PCacheHitrateInit(void);
BOOL PCacheHitrateGet(uint32_t *pHitCount, uint32_t *pMissCount);
void PCacheSnapshotGet(uint32_t *pHitCount, uint32_t *pMissCount);
BOOL PCacheGetCIDPID(uint8_t *cid, uint8_t *pid);


#ifdef __cplusplus
}
#endif


#endif
