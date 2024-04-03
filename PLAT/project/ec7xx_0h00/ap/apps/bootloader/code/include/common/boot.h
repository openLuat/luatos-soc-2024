#ifndef __BOOTROM_H__
#define __BOOTROM_H__

#define RESET_STAT_NORMAL 0 
#define RESET_STAT_UNDEF 1
//#include "ARMCM3.h"
#include "ec7xx.h"

typedef struct {
    uint32_t Reserved; 
}FuseInfo;

#define ECDSA_PUBLIC_KEY_LEN 64 //BYTES
typedef struct {
    //uint32_t BRDate;
    //uint32_t BRVersion;
    uint32_t PlatformType;
    uint32_t PlatformSubType;
    uint32_t ResetStat;
    FuseInfo Fuse;    
    uint32_t Error;
    uint32_t TransferAddr;
    uint8_t SecurityInitialized;
    uint8_t JTAGDisable;
    uint8_t SecureBootEnabled;    
    uint8_t DbgDisable;
    uint8_t DownloadDisable;
    uint8_t Reserved[3];    
    uint8_t ECDSAPublickKey[ECDSA_PUBLIC_KEY_LEN];
}BRInfo, *PBRInfo;

extern PBRInfo GetBRInfo(void);
extern BRInfo GBRInfo;


#endif
