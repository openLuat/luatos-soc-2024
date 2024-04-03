/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/


#ifndef _NVRAM_H
#define _NVRAM_H

 /*----------------------------------------------------------------------------*
  *                    INCLUDES                                                *
  *----------------------------------------------------------------------------*/

#ifdef __cplusplus
 extern "C" {
#endif
#ifndef WIN32
#else
#pragma pack(1)
#include "win32_config.h"
#endif

#if (defined CHIP_EC618) || (defined TYPE_EC718H)
// the size of rf calibration table is 100K bytes.
#define RF_CALI_TABLE_SIZE_100K
#define RF_CALI_NV_SECTOR_BANK_NUM_MAX     25
#elif (defined CHIP_EC716) || (defined TYPE_EC718S) || (defined TYPE_EC718P) || (defined TYPE_EC718U)// EC716, EC718S, EC718P || (defined TYPE_EC718U)
// the size of rf calibration table is 48K bytes.
#define RF_CALI_TABLE_SIZE_48K
#define RF_CALI_NV_SECTOR_BANK_NUM_MAX     12
#endif

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define NVRAM_SECTOR_SIZE   4096
#define NVRAM_PAGE_SIZE     256

#define NVRAM_FAC_RESTORE_ERR	0xFFFFFFFE

#define NVRAM_COMPRESS_FAC_MAX_SIZE   (0x4000)//16k
#define NVRAM_DECOMPRESS_FAC_MAX_SIZE (0xC000)//48k

#define NVRAM_COMPRESS_HEADER_MAGIC     0xECAC0129

#if (defined TYPE_EC716S) ||(defined TYPE_EC718S) || (defined TYPE_EC718P) || (defined TYPE_EC716E) || (defined TYPE_EC718U)// EC716S, EC718S, EC718P, EC716E

#define AP_NV_IMEISN_OFFSET            (0)
#define AP_NV_IMEISN_MAX_SZIE          (256)

#define AP_NV_NPI_OFFSET               (256)
#define AP_NV_NPI_MAX_SZIE             (256)

#define AP_NV_DXCO_OFFSET              (512)
#define AP_NV_DCXO_MAX_SZIE            (1536-32)//32B hdr to ensure resv region is 2KB

#define AP_NV_RESV_OFFSET              (2048)
#define AP_NV_RESV_MAX_SZIE            (2048)

#endif



/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef enum
{
    NVRAM_OK=0,
    NVRAM_ERR,
    NVRAM_FORMAT_ERR,
    NVRAM_ERASE_FLASH_ERR,
    NVRAM_WRITE_FLASH_ERR,
    NVRAM_CHECK_ITEM_ERR,
    NVRAM_CHECK_ITEM_NO_EXIST,
    NVRAM_FAC_BROKEN,
    NVRAM_PART_LEN_ERR,
} NvErr_e;

typedef enum 
{
	NV = 0x0,
	NV_FAC,
	NV_BANK_MAX,
} NvBank_e;

typedef enum 
{
    SAVE_ALL = 0x0,
    SAVE_CALI,
    SAVE_OTHER,
} NvSaveFac_e;

#if(defined TYPE_EC718H)
typedef enum 
{
    APNV1 = 0x0,
    APNV2,
    APNV3,
    APNV4,
    CPNV1,
    CPNV2,
    CPNV3,
    CPNV4,
    CPNV5,
    CPNV6,
    NV_MAX,
} NvType_t;
#elif (defined TYPE_EC716S) ||(defined TYPE_EC718S)|| (defined TYPE_EC718P) || (defined TYPE_EC716E) || (defined TYPE_EC718U)// EC716S, EC718S, EC718P, EC716E

typedef enum
{
    APNV1 = 0x0,
    CPNV1,
    CPNV2,
    CPNV3,
    CPNV4,
    NV_MAX,
} NvType_t;
#endif

#if (defined TYPE_EC716S) ||(defined TYPE_EC718S) || (defined TYPE_EC718P) || (defined TYPE_EC716E) || (defined TYPE_EC718U) // EC716S, EC718S, EC718P, EC716E
typedef enum
{
    APNV_IMEISN_PART = 0x0,
    APNV_NPI_PART,
    APNV_DCXO_PART,
    APNV_RESV_PART,
    APNV_PART_MAX,
} ApNvPart_t;
#endif

typedef struct
{
    uint32_t nv_flag;
    uint32_t time_counter; //default:0x00000001
    uint16_t used_size;
    uint16_t size_dummy;    //0xFFFF
    uint32_t chksum;    //CRC32
    uint32_t dummy1; //0xFFFFFFFF
    uint32_t dummy2; //0xFFFFFFFF
    uint32_t dummy3; //0xFFFFFFFF
    uint16_t header_chk_dummy; //0xFFFF
    uint16_t header_chk;
} NvHeader_t;

typedef struct
{
    uint32_t magic;          //default:0xECAC2013
    uint32_t compressedSize; //
    uint32_t crc;     //opt
    uint32_t resv;    //
} NvRfCompHeader_t;

/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

NvErr_e nvramInit(void);
uint32_t nvramRead(NvType_t nvt,uint8_t * data,uint32_t size,uint32_t offset);
uint32_t nvramWrite(NvType_t nvt,uint8_t * data,uint32_t size);
uint32_t nvramGetAddr(NvType_t nvt);
uint32_t nvramSave2Fac(void);
uint32_t nvramSave2FacAp();
uint32_t nvramGetnvAddr(NvType_t nvt);

#ifdef CORE_IS_AP
uint32_t nvramGetnvFacAddr(NvType_t nvt);
uint32_t nvramGetnvLength(NvType_t nvt);
#endif

#if (defined TYPE_EC716S) || (defined TYPE_EC718S) || (defined TYPE_EC718P) || (defined TYPE_EC716E) || (defined TYPE_EC718U)
uint32_t apNvramWrite(ApNvPart_t partType,uint8_t * data,uint32_t size);
uint32_t apNvramRead(ApNvPart_t partType,uint8_t * data,uint32_t size,uint32_t offset);
uint32_t nvramSave2CprsFac(uint8_t * data,uint32_t size, uint8_t *decprsBuf);
#endif


#ifdef __cplusplus
}
#endif

#endif /* _NVRAM_H */
