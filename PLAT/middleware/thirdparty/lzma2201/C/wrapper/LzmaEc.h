
#ifndef __LZMA_EC_H
#define __LZMA_EC_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


#define DECOMPRESS_DATA_TYPE_HEADER      0
#define DECOMPRESS_DATA_TYPE_TRUNK       1

#ifdef CORE_IS_AP
#define DECOMPRESS_RET_MAX               16
#endif
#ifdef CORE_IS_CP
#define DECOMPRESS_RET_MAX               12
#endif


#define AP_COMP_SECT_INFO_GAP            32     
#define AP_COMP_DATA_INSTREAM_GAP        64 
#define AP_DECOMP_MEM_RF_CALIB_MAX_LEN   0x1B000     
#define AP_DECOMP_MEM_MAX_LEN            0x20000     
#define AP_DECOMP_MEM_PROBS_OFFSET       0x400     
#define AP_DECOMP_MEM_DIC_OFFSET         0x4800     
#define AP_DECOMP_MEM_MAGIC_TAIL         0xADECBEAF
#define AP_HASH_MEM_OFFSET               0x29400     
#define AP_HASH_MEM_LEN                  (0x4000)//16k 


#define APCP_HASH_MSMB_MEM_START_ADDR      0x400000 
#define APCP_HASH_MSMB_MEM_END_ADDR        0x53FFFF 
#define APCP_HASH_OUT_MAX_LEN              32 
#define APCP_HASH_ALIGN_LEN                64 
#define APCP_HASH_ONCE_MAX_LEN             0x8000//32*1024 


#define CP_COMP_SECT_INFO_GAP            16     
#define CP_COMP_DATA_INSTREAM_GAP        16 
#define CP_DECOMP_MEM_RF_CALIB_MAX_LEN   0x1B000     
#define CP_DECOMP_MEM_MAX_LEN            0x20000  
#define CP_DECOMP_MEM_BASE               0x14C00     
#define CP_DECOMP_MEM_PROBS_OFFSET       0x14C00     
#define CP_DECOMP_MEM_DIC_OFFSET         0x19000     
#define CP_DECOMP_MEM_MAGIC_TAIL         0xCDECBEAF
#define CP_HASH_MEM_OFFSET               (AP_HASH_MEM_OFFSET+AP_HASH_MEM_LEN+0x400)     
#define CP_HASH_MEM_LEN                  (0x4000)//16k  

#define CP_RAM_BASE_ADDR                 0x200000
#define CP_LZMA_DEC_API_INDEX            31

#define BOOT_MSMB_MEM_START_ADDR         0x400000 


#ifdef CORE_IS_AP
#define DECOMPRESS_DATA_LEN_ONCE         72000  //0x5000
#endif
#ifdef CORE_IS_CP
#define DECOMPRESS_DATA_LEN_ONCE         80000  //0x2800
#endif


// system time typedef
typedef struct {
    UINT32  hfnsfnsbn;    // hfn(10)|sfn(10)|sbn(4); for BaseTime(AP used), 8 bits for OFN at bit[31:24]
    UINT32  spn;          // spn(15)
} ecSysTime;

typedef enum
{
    // Original Free Run Base Counter after PowerOn/Wakeup
    EC_BC_TYPE_BC_TIME     = 0x0,
    // BaseTime = BC+BaseTimeOfst, AP used only
    EC_BC_TYPE_BT_TIME,
    // RX time = BC+RxTimeOfst
    EC_BC_TYPE_RX_TIME,
    // TX time = BC+TxTimeOfst
    EC_BC_TYPE_TX_TIME
}ecBcTimeType;

typedef struct {
    volatile UINT32 bt_en;              //0x70
    volatile UINT32 rsv0;              //0x74
    volatile UINT32 rsv1;     //0x78
    volatile UINT32 rsv2;
    volatile UINT32 rsv3;           //0x80
    volatile UINT32 rsv4;          //0x84
    volatile UINT32 rsv5;          //0x88
    volatile UINT32 rsv6;       //0x8c
    volatile UINT32 rsv7;               // 0x90
    volatile UINT32 rsv8;               // 0x94
    volatile UINT32 rsvd[10]; //0x98~0xBC
    volatile UINT32 dbg_bc_l;            // 0xC0
    volatile UINT32 dbg_bc_h;          // 0xC4
    volatile UINT32 rsv9;              // 0xC8
    volatile UINT32 rsv10;               // 0xCC
    volatile UINT32 rsv11;               // 0xD0
    volatile UINT32 dbg_bt_l;            // 0xD4
    volatile UINT32 dbg_bt_h;            // 0xD8
    volatile UINT32 rsv12;              // 0xDC
    volatile UINT32 rsv13;            // 0xE0
    volatile UINT32 rsv14;              // 0xE4
    volatile UINT32 rsv15;            // 0xE8
    volatile UINT32 rsv16;              // 0xEC
} ecTmuBcRd;


typedef struct ecCompBinSectionInfo_S
{
    unsigned int validMagic;
    unsigned int compressedAddr;
    unsigned int compressedLen;
    unsigned int origAddr;
    unsigned int origLen;
    unsigned char type;
    unsigned char ziped;
    unsigned short resv;
	char hash[32];
} ecCompBinSectionInfo;

typedef struct ecCompBinHeader_S
{
    unsigned int magicHdr;
    unsigned int numOfSec;
    //ecCompBinSectionInfo  defined in "AP Section define"
    unsigned int magicTail;
} ecCompBinHeader;

typedef struct ecApCompExecuteRet_S
{
    int apDecompRet[DECOMPRESS_RET_MAX];
    int apDecompHashRet[DECOMPRESS_RET_MAX];
    int apDecompTimeRet[DECOMPRESS_RET_MAX];
    int apDecompHashTimeRet[DECOMPRESS_RET_MAX];
    int cpDecompHashRet[DECOMPRESS_RET_MAX];
    int cpDecompHashTimeRet[DECOMPRESS_RET_MAX];	
} apCompExecRet;

typedef struct ecCpCompExecuteRet_S
{
    int cpDecompRet[DECOMPRESS_RET_MAX];
    int cpDecompTimeRet[DECOMPRESS_RET_MAX];
} cpCompExecRet;


void decompressCodeFromBin(void);
void decompressRamCodeGetAddrInfo(void);
void decompressRamCodeFromBin(int ramCodeType);
void decompressGetLock(void);
void decompressRelLock(void);



#endif


