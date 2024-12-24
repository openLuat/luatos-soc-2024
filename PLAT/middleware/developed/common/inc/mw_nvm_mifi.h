#ifndef __MW_NVM_MIFI_H__
#define __MW_NVM_MIFI_H__
/****************************************************************************
 *
 * Copy right:   2017-, Copyrigths of AirM2M Ltd.
 * File name:    mw_nvm_mifi.h
 * Description:  middleware NVM mifi header file
 * History:      2024/07/08, Originated by yflu
 ****************************************************************************/
#include "osasys.h"
#include "mw_common.h"
#ifdef FEATURE_MIFI_AT_ENABLE
/*
 * Differences between these MW config/AON files:
 * 1> mw_nvm_config.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is still VALID, after FOTA (SW upgrading) if not erase the NVM in flash.
 *   c) if not configed in NVM/flash, use the default value.
 * 2> mw_nvm_info.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is not VALID (reset to default value), after FOTA (SW upgrading), if:
 *       i> NVM in flash is erased, or
 *       ii> NVM info structure size is changed, or
 *       ii> NVM file version is changed.
 *   c) if not configed in NVM/flash, use the default value.
 * 3> mw_aon_info.h
 *   a) parameter value is still VALID, after wakeup from deep sleep
 *   b) parameter value is not VALID, after reboot
 * 4> mw_common.h
 *   a) middleware common header file, which included by "mw_nvm_config.h"&"mw_nvm_info.h"&"mw_aon_info.h"
 *   b) As customers maybe have different requirements about how to maintain the config,
 *      here could set the common structure in this file
 * 2> mw_nvm_mifi.h
 *   a) parameter value is still VALID, after reboot.
 *   b) parameter value is not VALID (reset to default value), after FOTA (SW upgrading), if:
 *       i> NVM in flash is erased, or
 *       ii> NVM info structure size is changed, or
 *       ii> NVM file version is changed.
*/


/******************************************************************************
 *****************************************************************************
 * MARCO/MARCO
 *****************************************************************************
******************************************************************************/
/*
 *
*/
#define MID_WARE_NVM_MIFI_FILE_NAME					"mifiCfg.nvm"
#define MID_WARE_NVM_MIFI_CUR_VER					0x0

#define MID_WARE_NVM_MIFI_DEF_SSID					"MIFI_"
#define	MID_WARE_NVM_MIFI_DEF_ACCESS_PWD			"1234567890"
#define MID_WARE_NVM_MIFI_DEF_ADMIN_ID				"admin"
#define MID_WARE_NVM_MIFI_DEF_ADMIN_PWD				"admin"
#define MID_WARE_NVM_MIFI_DEF_VERSION				"version"
#define MID_WARE_NVM_MIFI_DEF_MAC					"00-00-00-00-00-00"

#define MID_WARE_NVM_MIFI_DEF_PWMOD					(2)
#define	MID_WARE_NVM_MIFI_DEF_SLEEP_TIMEOUT			(10)	/*minute*/

#define MID_WARE_NVM_MIFI_SSID_LEN_MAX				(64)
#define MID_WARE_NVM_MIFI_ACCESS_PWD_LEN_MAX		(64)
#define MID_WARE_NVM_MIFI_ADMIN_ID_LEN_MAX			(64)
#define MID_WARE_NVM_MIFI_ADMIN_PWD_LEN_MAX			(64)
#define	MID_WARE_NVM_MIFI_VERSION_LEN_MAX			(64)

#define MID_WARE_NVM_MIFI_MAC_LEN_MAX				(18)
#define MID_WARE_NVM_MIFI_PWMOD_MIN					(1)
#define MID_WARE_NVM_MIFI_PWMOD_MAX					(4)

#define MID_WARE_NVM_MIFI_SLEEP_TIMEOUT_MIN			(0)		
#define MID_WARE_NVM_MIFI_SLEEP_TIMEOUT_MAX			(60)	/*minute*/
/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************/
 #pragma pack(1)
typedef struct _ecMifiCfg_Tag{
	
}ecMifiCfg_t;
typedef struct _ecMifiDevCfg_tag{
	UINT8 ssid[MID_WARE_NVM_MIFI_SSID_LEN_MAX];
	UINT8 acccessPwd[MID_WARE_NVM_MIFI_ACCESS_PWD_LEN_MAX];
	UINT8 adminId[MID_WARE_NVM_MIFI_ADMIN_ID_LEN_MAX];
	UINT8 adminPwd[MID_WARE_NVM_MIFI_ADMIN_PWD_LEN_MAX];
}ecMifiAccessCfg;
typedef struct _NVM_EPAT_MWMIFO
{
	ecMifiAccessCfg access;
	UINT8 mac[MID_WARE_NVM_MIFI_MAC_LEN_MAX];
	UINT8 version[MID_WARE_NVM_MIFI_VERSION_LEN_MAX];
	UINT8 pwMod;
	UINT8 sleepTimeout;
}ecNvmMifiCfgStore_t;
#pragma pack()
/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************/
BOOL 	mwNvmMifiCfgFormat(ecNvmMifiCfgStore_t *pMifiCfg);
UINT32 	mwNvmMifiCfgInit(void);

BOOL	mwNvmMificfgRead(ecNvmMifiCfgStore_t *pMifiCfg);
void	mwNvmMifiCfgSave(ecNvmMifiCfgStore_t *pWriteMifiCfg);

UINT32 	mwNvmMifiAccessCfgGet(ecMifiAccessCfg *access);
BOOL	mwNvmMifiAccessCfgSet(ecMifiAccessCfg *access);

UINT32 	mwNvmMifiMacAddrGet(UINT8 *mac);
BOOL	mwNvmMifiMacAddrSet(UINT8 *mac);

UINT32 	mwNvmMifiVersionGet(UINT8 *version);
BOOL	mwNvmMifiVersionSet(UINT8 *version);

UINT32 	mwNvmMifiPwmodeGet(UINT8 *pwMode);
BOOL	mwNvmMifiPwmodeSet(UINT8 pwMode);

UINT32 	mwNvmMifiSleepGet(UINT8 *timeout);
BOOL	mwNvmMifiSleepSet(UINT8 timeout);
#endif/*FEATURE_MIFI_AT_ENABLE*/
#endif/*__MW_NVM_MIFI_H__*/