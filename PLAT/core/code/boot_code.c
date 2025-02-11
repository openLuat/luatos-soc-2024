#include "ec7xx.h"
#include "stdint.h"
#include "mem_map.h"
#include "pwrkey.h"
#define AP_BODY_MAX_SIZE           AP_FLASH_LOAD_SIZE
#define AP_BODY_FLASH_OFFSET_ADDR  (AP_FLASH_LOAD_ADDR - AP_FLASH_XIP_ADDR)
uint32_t soc_ap_body_max_size(void)
{
	return AP_BODY_MAX_SIZE;
}

uint32_t soc_ap_body_head(void)
{
	return AP_BODY_FLASH_OFFSET_ADDR;
}
__attribute__((weak)) void user_code_run(void *param) {;}
__attribute__((weak)) void user_code_init(void)
{
    if(pwrKeyGetPwrKeyMode() == PWRKEY_PWRON_MODE)
        pwrkeyPwrOnDebounce(2000);
}
__attribute__((weak)) uint8_t user_io_sel(uint8_t io_sel)
{
	return 0;
}
#include "sctdef.h"
extern void Bltransfer_Control(uint32_t p_base_addr);
PLAT_BL_AIRAM_PRE2_TEXT void soc_bl(void)
{
	Bltransfer_Control(AP_FLASH_LOAD_ADDR);
}



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "7zAlloc.h"
#include "7zFile.h"
#include "LzFind.h"
#include "LzmaEnc.h"
#include "LzmaDec.h"
#include "LzmaEc.h"
#include "mem_map.h"
#ifdef CORE_IS_CP
#include "shareinfo.h"
#endif
#ifdef CORE_IS_AP
#include "tls.h"
#include "apmu_external.h"
#endif

extern UINT32 flashXIPLimit;

#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
extern UINT32 _compress_buf_start;
extern UINT32 _decompress_buf_start;
#endif

//LZMA_BSS_SECTION CLzmaDec lzmaState = {0};
//LZMA_BSS_SECTION unsigned int lzmaUnpackSize = 0;
//LZMA_BSS_SECTION unsigned int lzmaUnpackSizeTotal = 0;
//LZMA_BSS_SECTION const ISzAlloc gLzmaEcAlloc = { SzAlloc, SzFree };
LZMA_BSS_SECTION ecCompBinSectionInfo *pCompBinSectionInfoAddr = NULL;
LZMA_BSS_SECTION ecCompBinHeader compBinHeader = {0};
LZMA_BSS_SECTION unsigned int hashBuffStartAddr = 0;
LZMA_BSS_SECTION unsigned int decompBuffStartAddr = 0;
LZMA_BSS_SECTION unsigned int decompBuffEndAddr = 0;
//LZMA_BSS_SECTION unsigned int decompDataTotalLen = 0;

LZMA_BSS_SECTION char *lzmaBootInputDataBase = NULL;
LZMA_BSS_SECTION char *lzmaBootInputDataCurr = NULL;
LZMA_BSS_SECTION int lzmaBootInputDataLen = 0;

LZMA_BSS_SECTION char *lzmaBootOutputDataBase = NULL;
LZMA_BSS_SECTION char *lzmaBootOutputDataCurr = NULL;
LZMA_BSS_SECTION int lzmaBootOutputDataLen = 0;

LZMA_BSS_SECTION char *lzmaBootBuffBase = NULL;


#ifdef CORE_IS_AP
LZMA_BSS_SECTION volatile char lzmaHashOut[APCP_HASH_OUT_MAX_LEN] = {0};
LZMA_BSS_SECTION volatile apCompExecRet apCompExecuteRet = {0};
#endif
#ifdef CORE_IS_CP
LZMA_BSS_SECTION volatile cpCompExecRet cpCompExecuteRet = {0};
#endif
typedef int (*lzmaDecDecodeToBuf)(CLzmaDec *, Byte *, SizeT *, const Byte *, SizeT *, ELzmaFinishMode, ELzmaStatus *);

#ifdef CORE_IS_AP
extern void sctInitLzma();
extern void sctDeInitLzma();
extern int32_t shaUpdateLzma(shaType_e shaMode, uint32_t srcAddr, uint32_t dstAddr, uint32_t length, uint32_t lastFlag);
extern int32_t dbusCacheForceOff( BOOL isForceOff );
extern uint32_t ShareInfoAPGetCPFlashXIPLimit(void);

#endif

PLAT_BL_UNCOMP_FLASH_TEXT SRes lzmaDecodeEcExc(CLzmaDec *state, unsigned char *outStream,  unsigned int *outStreamLen, unsigned char *inDataStream,  unsigned int inDataStreamLen, unsigned int unpackSize)
{
    int thereIsSize = (unpackSize != (UInt64)(Int64)-1);
    //Byte inBuf[IN_BUF_SIZE];
    //Byte outBuf[OUT_BUF_SIZE];
    unsigned char *inBuf = inDataStream;
    unsigned char *outBuf = outStream;
    size_t inPos = 0, inSize = 0, outPos = 0;
    SRes res;
    SizeT outProcessedBefore = DECOMPRESS_DATA_LEN_ONCE;
    SizeT outProcessedAfter = 0;
    SizeT inProcessedBefore = 0;
    SizeT inProcessedAfter = 0;
    ELzmaFinishMode finishMode = LZMA_FINISH_ANY;
    ELzmaStatus status;

    //LzmaDec_Init(state);
    for (;;)
    {
        if (inPos == inSize)
        {
            inSize = inDataStreamLen;
            //RINOK(inStream->Read(inStream, inBuf, &inSize));
            inPos = 0;
        }
        {
            //SRes res;
            inProcessedBefore = inSize - inPos;
            inProcessedAfter = inSize - inPos;
            outProcessedBefore = DECOMPRESS_DATA_LEN_ONCE;
            finishMode = LZMA_FINISH_ANY;

            if (thereIsSize && outProcessedBefore > unpackSize)
            {
                outProcessedBefore = (SizeT)unpackSize;
                finishMode = LZMA_FINISH_END;
            }

            #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
            res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessedBefore,
            inBuf + inPos, &inProcessedAfter, finishMode, &status);
            #else
                #ifdef CORE_IS_AP
                lzmaDecDecodeToBuf lzmaDecDecodeToBufPtr = (lzmaDecDecodeToBuf)(*(int *)(CP_FLASH_LOAD_ADDR + CP_LZMA_DEC_API_INDEX*4));
                res = (*lzmaDecDecodeToBufPtr)(state, outBuf + outPos, &outProcessedBefore,
                inBuf + inPos, &inProcessedAfter, finishMode, &status);
                #endif

                #ifdef CORE_IS_CP
                res = LzmaDec_DecodeToBuf(state, outBuf + outPos, &outProcessedBefore,
                inBuf + inPos, &inProcessedAfter, finishMode, &status);
                #endif
            #endif

            inPos += inProcessedAfter;
            outPos += outProcessedBefore;
            unpackSize -= outProcessedBefore;

            outProcessedAfter = outProcessedAfter + outProcessedBefore;
            *outStreamLen = outProcessedAfter;

            //if (outStream)
            //  if (outStream->Write(outStream, outBuf, outPos) != outPos)
            //      return SZ_ERROR_WRITE;

            //outPos = 0;

            if (res != SZ_OK || (thereIsSize && unpackSize == 0))
                return res;

            if (inProcessedAfter == 0 && outProcessedBefore == 0)
            {
                if (thereIsSize || status != LZMA_STATUS_FINISHED_WITH_MARK)
                    return SZ_ERROR_DATA;
                return res;
            }

            if(inProcessedAfter == inProcessedBefore)
            {
                return res;


}
        }
    }
}

PLAT_BL_UNCOMP_FLASH_TEXT int lzmaDecodeEc(unsigned char *outStream, unsigned int *outStreamLen, unsigned char *inStream, unsigned int inStreamLen, unsigned int dataType, unsigned int *lzmaUnpackLen)
{
    int i;
    int res = 0;
    unsigned char *inDataStream = NULL;
    unsigned int inDataStreamLen = 0;
    unsigned char *inStreamPtr = inStream;
    CLzmaDec lzmaState;
    unsigned int lzmaUnpackSize = 0;
    ISzAlloc gLzmaEcAlloc = { SzAlloc, SzFree };

    /* header: 5 bytes of LZMA properties and 8 bytes of uncompressed size */
    unsigned char header[LZMA_PROPS_SIZE + 8];

    if(dataType == 0)
    {
        /* Read and parse header */
        for(i=0; i<sizeof(header); i++)
        {
            header[i] = *inStreamPtr;
            inStreamPtr++;
        }
        //memcpy(header, inStream, sizeof(header));
        inDataStream = inStream + sizeof(header);
        inDataStreamLen = inStreamLen - sizeof(header);

        lzmaUnpackSize = 0;
        for (i = 0; i < 8; i++)
        lzmaUnpackSize += (UInt64)header[LZMA_PROPS_SIZE + i] << (i * 8);

        LzmaDec_Construct(&lzmaState);
        RINOK(LzmaDec_Allocate(&lzmaState, header, LZMA_PROPS_SIZE, &gLzmaEcAlloc));
        LzmaDec_Init(&lzmaState);
        *lzmaUnpackLen = lzmaUnpackSize;//lzmaUnpackSizeTotal total len of decomp data
    }
    else
    {
        inDataStream = inStream;
        inDataStreamLen = inStreamLen;
    }

    // param is all right
    res = (int)lzmaDecodeEcExc(&lzmaState, outStream, outStreamLen, inDataStream, inDataStreamLen, *lzmaUnpackLen);
    //LzmaDec_Free(&state, &g_Alloc);
    return res;
}
#ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE

PLAT_BL_UNCOMP_FLASH_TEXT static SRes lzmaEncodeEcExc(ISeqOutStream *outStream, ISeqInStream *inStream, UInt64 fileSize, char *rs)
{
  CLzmaEncHandle enc;
  SRes res;
  CLzmaEncProps props;
  ISzAlloc gLzmaEcAlloc = { SzAlloc, SzFree };

  enc = LzmaEnc_Create(&gLzmaEcAlloc);
  if (enc == 0)
    return SZ_ERROR_MEM;

  LzmaEncProps_Init(&props);
  res = LzmaEnc_SetProps(enc, &props);

  if (res == SZ_OK)
  {
    Byte header[LZMA_PROPS_SIZE + 8];
    size_t headerSize = LZMA_PROPS_SIZE;
    int i;

    res = LzmaEnc_WriteProperties(enc, header, &headerSize);
    for (i = 0; i < 8; i++)
      header[headerSize++] = (Byte)(fileSize >> (8 * i));
    if (outStream->Write(outStream, header, headerSize) != headerSize)
      res = SZ_ERROR_WRITE;
    else
    {
      if (res == SZ_OK)
        res = LzmaEnc_Encode(enc, outStream, inStream, NULL, &gLzmaEcAlloc, &gLzmaEcAlloc);
    }
  }
  //LzmaEnc_Destroy(enc, &gLzmaEcAlloc, &gLzmaEcAlloc);
  return res;
}

PLAT_BL_UNCOMP_FLASH_TEXT int lzmaEncodeEc(unsigned char *outStreamData, unsigned int *outStreamDataLen, unsigned char *inStreamData, unsigned int inStreamDataLen)
{
  CFileSeqInStream inStream;
  CFileOutStream outStream;
  int res;
  char rs = 0;

  LzFindPrepare();

  FileSeqInStream_CreateVTable(&inStream);
  File_Construct(&inStream.file);
  inStream.wres = 0;

  FileOutStream_CreateVTable(&outStream);
  File_Construct(&outStream.file);
  outStream.wres = 0;

  lzmaBootBuffBase = (char *)&_compress_buf_start;

  lzmaBootInputDataBase = (char *)inStreamData;
  lzmaBootInputDataCurr = (char *)inStreamData;
  lzmaBootInputDataLen = inStreamDataLen;

  lzmaBootOutputDataBase = (char *)outStreamData;
  lzmaBootOutputDataCurr = (char *)outStreamData;
  lzmaBootOutputDataLen  = *outStreamDataLen;

  memset((char *)((uint32_t)lzmaBootBuffBase + LZMA_BOOT_BUFF_HASH_OFFSET), 0, LZMA_BOOT_BUFF_HASH_LEN);

  res = lzmaEncodeEcExc(&outStream.vt, &inStream.vt, inStreamDataLen, &rs);

  *outStreamDataLen = lzmaBootOutputDataLen;

  return res;
}
#endif


/*===========================================================================
* Function:    TMU_APTimeRead: time read for AP
* Description: AP latch and read timeType
* Called by:   after bcLd Isr
* Calls:       none.
* Input:       timeType: BC_TYPE_BC_TIME/BC_TYPE_BT_TIME
* Output:      ecSysTime *sysTime, OFN included
* History:     2020/06/17      updated by zyfang
*===========================================================================*/
PLAT_BL_UNCOMP_FLASH_TEXT void lzma_APTimeRead(ecSysTime *sysTime, ecBcTimeType timeType)
{
	#ifdef CORE_IS_AP
	ecTmuBcRd* bcRd = (ecTmuBcRd* )(0x4F070000+0x0070); //APB_GP_PERIPH_BASE  HW_TmuBcRd
	#endif
	#ifdef CORE_IS_CP
	ecTmuBcRd* bcRd = (ecTmuBcRd* )(0x5F070000+0x0070); //APB_GP_PERIPH_BASE  HW_TmuBcRd
	#endif

    UINT32 mask;
    UINT32 ofnHfnSfnSbn1, ofnHfnSfnSbn2, spn = 0;

    mask = SaveAndSetIRQMask();
//    CLOCK_clockEnable(PCLK_TMU);

    ofnHfnSfnSbn1 = 0;
    ofnHfnSfnSbn2 = 0xFFFFFFFF;

    if(timeType == EC_BC_TYPE_BC_TIME)
    {
        while(ofnHfnSfnSbn2 != ofnHfnSfnSbn1)
        {
            ofnHfnSfnSbn1 = bcRd->dbg_bc_h;
            spn = bcRd->dbg_bc_l;
            ofnHfnSfnSbn2 = bcRd->dbg_bc_h;
        }
    }
    else if(timeType == EC_BC_TYPE_BT_TIME)
    {
        bcRd->bt_en = 1;
        while(ofnHfnSfnSbn2 != ofnHfnSfnSbn1)
        {
            ofnHfnSfnSbn1 = bcRd->dbg_bt_h;
            spn = bcRd->dbg_bt_l;
            ofnHfnSfnSbn2 = bcRd->dbg_bt_h;
        }
        bcRd->bt_en = 0;
    }

    sysTime->hfnsfnsbn = ofnHfnSfnSbn1;
    sysTime->spn = spn;

//    CLOCK_clockDisable(PCLK_TMU);

    if(timeType ==EC_BC_TYPE_BT_TIME)
    {
//        void tcocBTSystickCheck(uint32_t hfnsfnsbn);
//        tcocBTSystickCheck(sysTime->hfnsfnsbn);
    }
    RestoreIRQMask(mask);
}


#define LZMA_DECOMPRESS_EC_WRAPPER_API

#ifdef RAMCODE_COMPRESS_EN
#ifdef CORE_IS_AP
PLAT_BL_UNCOMP_FLASH_TEXT __attribute__((noinline)) void decompressCpRamCodeHashCheck(int ramCodeType)
{
    unsigned int *asmbData = NULL;
    unsigned int *msmbData = NULL;
    unsigned int index = ramCodeType-1;
    unsigned int i = 0;
    unsigned int length = 0;
    unsigned int lengthLeft = 0;
    unsigned int RetValue = 0;
    unsigned int hashCount = 0;
    unsigned int pFlashXIPAddr = 0;
    ecSysTime mSysCpTime0;
    ecSysTime mSysCpTime1;
    UINT32 lzmaCurrCpTime0 = 0;
    UINT32 lzmaCurrCpTime1 = 0;
    unsigned int decompInfoFlag = 0;
/*
    pFlashXIPAddr = ShareInfoAPGetCPFlashXIPLimit();
    compBinHeader.magicHdr = *(unsigned int *)pFlashXIPAddr;
    compBinHeader.numOfSec = *(unsigned int *)(pFlashXIPAddr+4);
    pCompBinSectionInfoAddr = (ecCompBinSectionInfo *)(pFlashXIPAddr+8);
    decompBuffStartAddr  = up_buf_start;
    hashBuffStartAddr    = decompBuffStartAddr + CP_HASH_MEM_OFFSET;
*/
    pFlashXIPAddr = *(int *)((CP_FLASH_LOAD_SIZE-8)+CP_FLASH_LOAD_ADDR)+CP_FLASH_LOAD_ADDR;

    compBinHeader.magicHdr = *(unsigned int *)pFlashXIPAddr;
    compBinHeader.numOfSec = *(unsigned int *)(pFlashXIPAddr+4);
    pCompBinSectionInfoAddr = (ecCompBinSectionInfo *)(pFlashXIPAddr+8);


    lzma_APTimeRead(&mSysCpTime0, EC_BC_TYPE_BC_TIME);
    lzmaCurrCpTime0 = (((mSysCpTime0.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysCpTime0.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysCpTime0.hfnsfnsbn & 0xf))*1000 + (mSysCpTime0.spn*1000)/30720; //jing que dao 0.001ms
    //lzmaCurrCpTime0 = (((mSysCpTime0.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysCpTime0.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysCpTime0.hfnsfnsbn & 0xf));    //jing que dao ms

    //the ramCodeType maybe not be the type which 'compBinSectionInfoPtr[index]' stores
//    if(pCompBinSectionInfoAddr[index].type != ramCodeType)
    {
        for(i=0; i<compBinHeader.numOfSec; i++)
        {
            if((pCompBinSectionInfoAddr[i].type == ramCodeType)&&(pCompBinSectionInfoAddr[i].ziped == 0x1)&&(pCompBinSectionInfoAddr[i].secFlag == 0x1))
            {
                index = i;
                decompInfoFlag = 0xff;
                break;
            }
        }
    }

    if(decompInfoFlag != 0xff)  //not find section in the bin file
    {
        return ;
    }

    /*SHA_TYPE_256*/
    if(apmuGetAPBootFlag()==AP_BOOT_FROM_POWER_ON)
    {
        if(((0 <= index)&&(index < SECTIONCP_HASH_CHECK_MAX_INDEX)) && (pCompBinSectionInfoAddr[index].origLen > 0))
        {
            sctInitLzma();

            for(i=0; i<sizeof(lzmaHashOut);i++)
            {
                lzmaHashOut[i] = 0;
            }

            if((APCP_HASH_MSMB_MEM_START_ADDR <= pCompBinSectionInfoAddr[index].origAddr)&&(pCompBinSectionInfoAddr[index].origAddr <= APCP_HASH_MSMB_MEM_END_ADDR ))
            {
                if(pCompBinSectionInfoAddr[index].origLen <= APCP_HASH_ONCE_MAX_LEN)
                {
                    RetValue = shaUpdateLzma(2, (uint32_t)pCompBinSectionInfoAddr[index].origAddr, (uint32_t)lzmaHashOut, pCompBinSectionInfoAddr[index].origLen, 1);
                }
                else
                {
                    hashCount = pCompBinSectionInfoAddr[index].origLen/APCP_HASH_ONCE_MAX_LEN;
                    for(i=0; i<hashCount; i++)
                    {
                        RetValue = shaUpdateLzma(2, (uint32_t)(pCompBinSectionInfoAddr[index].origAddr+i*APCP_HASH_ONCE_MAX_LEN), (uint32_t)lzmaHashOut, APCP_HASH_ONCE_MAX_LEN, 0);
                    }
                    lengthLeft = pCompBinSectionInfoAddr[index].origLen%APCP_HASH_ONCE_MAX_LEN;
                    RetValue = shaUpdateLzma(2, (uint32_t)(pCompBinSectionInfoAddr[index].origAddr+hashCount*APCP_HASH_ONCE_MAX_LEN), (uint32_t)lzmaHashOut, lengthLeft, 1);
                }
            }
            else
            {
                length = pCompBinSectionInfoAddr[index].origLen;
                if(length <= CP_HASH_MEM_LEN)
                {
                    length /= sizeof(unsigned int);
                    asmbData = (unsigned int *)pCompBinSectionInfoAddr[index].origAddr + CP_RAM_BASE_ADDR;
                    msmbData = (unsigned int *)hashBuffStartAddr;

                    while(length > 0)
                    {
                        msmbData[length-1] = asmbData[length-1];
                        length--;
                    }
                    RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, pCompBinSectionInfoAddr[index].origLen, 1);
                }
                else
                {
                    hashCount = pCompBinSectionInfoAddr[index].origLen/CP_HASH_MEM_LEN;
                    for(i=0; i<hashCount; i++)
                    {
                        length = CP_HASH_MEM_LEN/sizeof(unsigned int);
                        asmbData = (unsigned int *)(pCompBinSectionInfoAddr[index].origAddr + CP_RAM_BASE_ADDR + i*CP_HASH_MEM_LEN);
                        msmbData = (unsigned int *)hashBuffStartAddr;

                        while(length > 0)
                        {
                            msmbData[length-1] = asmbData[length-1];
                            length--;
                        }
                        RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, CP_HASH_MEM_LEN, 0);
                    }
                    lengthLeft = pCompBinSectionInfoAddr[index].origLen%CP_HASH_MEM_LEN;
                    length = lengthLeft/sizeof(unsigned int);
                    asmbData = (unsigned int *)(pCompBinSectionInfoAddr[index].origAddr + CP_RAM_BASE_ADDR + hashCount*CP_HASH_MEM_LEN);
                    msmbData = (unsigned int *)hashBuffStartAddr;

                    while(length > 0)
                    {
                        msmbData[length-1] = asmbData[length-1];
                        length--;
                    }

                    RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, lengthLeft, 1);

                }
            }
            for(i=0; i<APCP_HASH_OUT_MAX_LEN; i++)
            {
                if(lzmaHashOut[i] != pCompBinSectionInfoAddr[index].hash[i])
                {
                    apCompExecuteRet.cpDecompHashRet[index] = 0xCDE00001;
                    __asm volatile("movw r12, #0xecaa");
                    __asm volatile("movt r12, #0xecaa");
                    __asm volatile("str  r12, [r12]");
                    break;
                }
            }
            sctDeInitLzma();
        }
    }

    apCompExecuteRet.cpDecompHashRet[index] = RetValue;

    lzma_APTimeRead(&mSysCpTime1, EC_BC_TYPE_BC_TIME);
    lzmaCurrCpTime1 = (((mSysCpTime1.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysCpTime1.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysCpTime1.hfnsfnsbn & 0xf))*1000 + (mSysCpTime1.spn*1000)/30720; //jing que dao 0.001ms
    //lzmaCurrCpTime0 = (((mSysCpTime1.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysCpTime1.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysCpTime1.hfnsfnsbn & 0xf));    //jing que dao ms
    apCompExecuteRet.cpDecompHashTimeRet[index] = lzmaCurrCpTime1-lzmaCurrCpTime0;
}
#endif
#endif

PLAT_BL_UNCOMP_FLASH_TEXT void decompressGetAddrInfo(void)
{
    unsigned int pFlashXIPAddr = 0;

    #ifdef CORE_IS_AP
    pFlashXIPAddr = *(int *)((AP_FLASH_LOAD_SIZE-8)+AP_FLASH_LOAD_ADDR)+AP_FLASH_LOAD_ADDR;
    #endif
    #ifdef CORE_IS_CP
    pFlashXIPAddr = *(int *)((CP_FLASH_LOAD_SIZE-8)+CP_FLASH_LOAD_ADDR)+CP_FLASH_LOAD_ADDR;
    #endif
    #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
    pFlashXIPAddr = *(int *)((BOOTLOADER_FLASH_LOAD_SIZE-8)+BOOTLOADER_FLASH_LOAD_ADDR)+BOOTLOADER_FLASH_LOAD_ADDR;
    #endif

    compBinHeader.magicHdr = *(unsigned int *)pFlashXIPAddr;
    compBinHeader.numOfSec = *(unsigned int *)(pFlashXIPAddr+4);
    pCompBinSectionInfoAddr = (ecCompBinSectionInfo *)(pFlashXIPAddr+8);

#ifdef TYPE_EC718M
    #if (defined CORE_IS_AP) && !(defined FEATURE_BOOTLOADER_PROJECT_ENABLE)
    extern void *psDialGetUpMemAndSize(UINT32 *pUpMemSize);
    decompBuffStartAddr = (uint32_t)psDialGetUpMemAndSize(NULL);
    decompBuffEndAddr    = decompBuffStartAddr + AP_DECOMP_MEM_RF_CALIB_MAX_LEN;
    hashBuffStartAddr    = decompBuffStartAddr + AP_HASH_MEM_OFFSET;
    #endif
#else
    #ifdef CORE_IS_AP
    decompBuffStartAddr  = up_buf_start;
    decompBuffEndAddr    = decompBuffStartAddr + AP_DECOMP_MEM_RF_CALIB_MAX_LEN;
    hashBuffStartAddr    = decompBuffStartAddr + AP_HASH_MEM_OFFSET;
    #endif
#endif

    #ifdef CORE_IS_CP
    decompBuffStartAddr = ShareInfoCPGetCpdprsBufAddr();
    decompBuffEndAddr   = decompBuffStartAddr + CP_DECOMP_MEM_RF_CALIB_MAX_LEN;
    hashBuffStartAddr   = decompBuffStartAddr + CP_HASH_MEM_OFFSET;
    #endif

    #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
    decompBuffStartAddr  = (unsigned int)&_decompress_buf_start;
    decompBuffEndAddr    = decompBuffStartAddr + AP_DECOMP_MEM_RF_CALIB_MAX_LEN;
    hashBuffStartAddr    = decompBuffStartAddr + AP_HASH_MEM_OFFSET;

    #endif
}

PLAT_BL_UNCOMP_FLASH_TEXT void decompressRamCodeGetAddrInfo(void)
{
    decompressGetAddrInfo();
}

#ifdef RAMCODE_COMPRESS_EN
PLAT_BL_UNCOMP_FLASH_TEXT void decompressRamCodeFromBin(int ramCodeType)
{
    unsigned int *origData = NULL;
    unsigned int *flashData = NULL;
    unsigned char *compData = NULL;
    unsigned int index = ramCodeType-1;
    unsigned int i = 0;
    unsigned int k = 0;
    unsigned int length = 0;
    unsigned int lzmaDecompDataTotalLen = 0;
    unsigned int lzmaDecompDataLen = 0;
    unsigned int lzmaCompDataLeft = 0;
    unsigned int lzmaDecompMaxCount = 0;
    unsigned int decompDataTotalLen = 0;
    int res = 0;
    ecSysTime mSysTime0;
    UINT32 lzmaCurrTime0 = 0;
    ecSysTime mSysTime1;
    UINT32 lzmaCurrTime1 = 0;
    #ifdef CORE_IS_AP
    ecSysTime mSysTime2;
    UINT32 lzmaCurrTime2 = 0;
    unsigned int RetValue = 0;
    unsigned int hashCount = 0;
    unsigned int *asmbData = NULL;
    unsigned int *msmbData = NULL;
    unsigned int lengthLeft = 0;
    #endif
    unsigned int decompInfoFlag = 0;

    #ifdef CORE_IS_AP
    #ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
    //index must >0, because dbusCacheForceOff is decompressed in index0
    if(index > 0)
    {
        dbusCacheForceOff(FALSE);
    }
    #endif
    #endif

    //the ramCodeType maybe not be the type which 'compBinSectionInfoPtr[index]' stores
    {
        for(i=0; i<compBinHeader.numOfSec; i++)
        {
            if(pCompBinSectionInfoAddr[i].type == ramCodeType)
            {
                index = i;
                decompInfoFlag = 0xff;
                break;
            }
        }
    }

    if(decompInfoFlag != 0xff)  //not find section in the bin file
    {
        #ifdef CORE_IS_AP
        #ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
        //index must >0, because dbusCacheForceOff is decompressed in index0
        if(index > 0)
        {
            dbusCacheForceOff(TRUE);
        }
        #endif
        #endif

        return ;
    }
    lzma_APTimeRead(&mSysTime0, EC_BC_TYPE_BC_TIME);
    lzmaCurrTime0 = (((mSysTime0.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime0.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime0.hfnsfnsbn & 0xf))*1000 + (mSysTime0.spn*1000)/30720; //jing que dao 0.001ms
    //lzmaCurrTime0 = (((mSysTime0.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime0.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime0.hfnsfnsbn & 0xf));    //jing que dao ms

    if((pCompBinSectionInfoAddr[index].ziped == 0x1)&&(pCompBinSectionInfoAddr[index].compressedLen > 0))
    {
        if(pCompBinSectionInfoAddr[index].compressedLen <= DECOMPRESS_DATA_LEN_ONCE)
        {
            #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
            compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + BOOTLOADER_FLASH_LOAD_ADDR);
            #else
                #ifdef CORE_IS_AP
                compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + AP_FLASH_LOAD_ADDR);
                #endif
                #ifdef CORE_IS_CP
                compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + CP_FLASH_LOAD_ADDR);
                #endif
            #endif

            lzmaDecompDataLen = DECOMPRESS_DATA_LEN_ONCE;
            res = lzmaDecodeEc((unsigned char *)pCompBinSectionInfoAddr[index].origAddr, &lzmaDecompDataLen, (unsigned char *)compData, pCompBinSectionInfoAddr[index].compressedLen, DECOMPRESS_DATA_TYPE_HEADER, &lzmaDecompDataTotalLen);
        }
        else
        {
            lzmaDecompMaxCount = pCompBinSectionInfoAddr[index].compressedLen/DECOMPRESS_DATA_LEN_ONCE;
            lzmaCompDataLeft = pCompBinSectionInfoAddr[index].compressedLen%DECOMPRESS_DATA_LEN_ONCE;

            decompDataTotalLen = 0;

            for(k=0; k<lzmaDecompMaxCount; k++)
            {
                #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
                compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + BOOTLOADER_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                #else
                    #ifdef CORE_IS_AP
                    compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + AP_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                    #endif
                    #ifdef CORE_IS_CP
                    compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + CP_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                    #endif
                #endif
                lzmaDecompDataLen = DECOMPRESS_DATA_LEN_ONCE;
                if(k == 0)
                {
                    res = lzmaDecodeEc((unsigned char *)pCompBinSectionInfoAddr[index].origAddr, &lzmaDecompDataLen, (unsigned char *)compData, DECOMPRESS_DATA_LEN_ONCE, DECOMPRESS_DATA_TYPE_HEADER, &lzmaDecompDataTotalLen);
                    decompDataTotalLen = decompDataTotalLen+lzmaDecompDataLen;
                }
                else
                {
                    lzmaDecompDataTotalLen = lzmaDecompDataTotalLen - k*DECOMPRESS_DATA_LEN_ONCE;
                    res = lzmaDecodeEc((unsigned char *)(pCompBinSectionInfoAddr[index].origAddr+decompDataTotalLen), &lzmaDecompDataLen, (unsigned char *)compData, DECOMPRESS_DATA_LEN_ONCE, DECOMPRESS_DATA_TYPE_TRUNK, &lzmaDecompDataTotalLen);
                    decompDataTotalLen = decompDataTotalLen+lzmaDecompDataLen;
                }
            }
            if(lzmaCompDataLeft > 0)
            {
                #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
                compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + BOOTLOADER_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                #else
                    #ifdef CORE_IS_AP
                    compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + AP_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                    #endif
                    #ifdef CORE_IS_CP
                    compData = (unsigned char *)(pCompBinSectionInfoAddr[index].compressedAddr + CP_FLASH_LOAD_ADDR + k*DECOMPRESS_DATA_LEN_ONCE);
                    #endif
                #endif

                lzmaDecompDataLen = DECOMPRESS_DATA_LEN_ONCE;
                lzmaDecompDataTotalLen = lzmaDecompDataTotalLen - lzmaDecompMaxCount*DECOMPRESS_DATA_LEN_ONCE;
                res = lzmaDecodeEc((unsigned char *)(pCompBinSectionInfoAddr[index].origAddr+decompDataTotalLen), &lzmaDecompDataLen, (unsigned char *)compData, lzmaCompDataLeft, DECOMPRESS_DATA_TYPE_TRUNK, &lzmaDecompDataTotalLen);
            }
        }
    }
    else
    {
        // origData and flashData should be 4 byte aligned generated by fcelf
        // length is 4 byte aligned defined in LD
        if(pCompBinSectionInfoAddr[index].origLen > 0)
        {
            length = pCompBinSectionInfoAddr[index].origLen;
            length /= sizeof(unsigned int);
            origData = (unsigned int *)pCompBinSectionInfoAddr[index].origAddr;
            #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
            flashData = (unsigned int *)(pCompBinSectionInfoAddr[index].compressedAddr + BOOTLOADER_FLASH_LOAD_ADDR);
            #else
                #ifdef CORE_IS_AP
                flashData = (unsigned int *)(pCompBinSectionInfoAddr[index].compressedAddr + AP_FLASH_LOAD_ADDR);
                #endif
                #ifdef CORE_IS_CP
                flashData = (unsigned int *)(pCompBinSectionInfoAddr[index].compressedAddr + CP_FLASH_LOAD_ADDR);
                #endif
            #endif

            if(origData != flashData)
            {
                while(length > 0)
                {
                    origData[length-1] = flashData[length-1];
                    length--;
                }
            }
        }
    }
    #ifdef CORE_IS_AP
    apCompExecuteRet.apDecompRet[index] = res;
    #endif
    #ifdef CORE_IS_CP
    cpCompExecuteRet.cpDecompRet[index] = res;
    #endif

    #ifdef CORE_IS_AP
    #ifndef FEATURE_BOOTLOADER_PROJECT_ENABLE
    if(index > 0)
    {
        dbusCacheForceOff(TRUE);
    }
    #endif
    #endif

    lzma_APTimeRead(&mSysTime1, EC_BC_TYPE_BC_TIME);
    lzmaCurrTime1 = (((mSysTime1.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime1.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime1.hfnsfnsbn & 0xf))*1000 + (mSysTime1.spn*1000)/30720; //jing que dao 0.001ms
    //lzmaCurrTime1 = (((mSysTime1.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime1.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime1.hfnsfnsbn & 0xf));    //jing que dao ms

    #ifdef CORE_IS_AP
    apCompExecuteRet.apDecompTimeRet[index] = lzmaCurrTime1-lzmaCurrTime0;
    #endif
    #ifdef CORE_IS_CP
    cpCompExecuteRet.cpDecompTimeRet[index] = lzmaCurrTime1-lzmaCurrTime0;
    #endif


#if 1
    #ifdef CORE_IS_AP
    /*for AP, after decompressing ram code, hash check will be done.
      for CP, the hash check is done with decompressCpRamCodeHashCheck()*/
    /*SHA_TYPE_256*/
    #ifdef FEATURE_BOOTLOADER_PROJECT_ENABLE
    if(((*(uint32_t *)0x4f0201A0)&0xe0) <= 0)
    #else
    if(apmuGetAPBootFlag() == AP_BOOT_FROM_POWER_ON)
    #endif
    {
        if(((0 <= index)&&(index < SECTIONAP_HASH_CHECK_MAX_INDEX)) && (pCompBinSectionInfoAddr[index].origLen > 0))
        {
            sctInitLzma();
            for(i=0; i<sizeof(lzmaHashOut);i++)
            {
                lzmaHashOut[i] = 0;
            }

            if((APCP_HASH_MSMB_MEM_START_ADDR <= pCompBinSectionInfoAddr[index].origAddr)&&(pCompBinSectionInfoAddr[index].origAddr <= APCP_HASH_MSMB_MEM_END_ADDR ))
            {
                if(pCompBinSectionInfoAddr[index].origLen <= APCP_HASH_ONCE_MAX_LEN)
                {
                    RetValue = shaUpdateLzma(2, (uint32_t)pCompBinSectionInfoAddr[index].origAddr, (uint32_t)lzmaHashOut, pCompBinSectionInfoAddr[index].origLen, 1);
                }
                else
                {
                    hashCount = pCompBinSectionInfoAddr[index].origLen/APCP_HASH_ONCE_MAX_LEN;
                    for(i=0; i<hashCount; i++)
                    {
                        RetValue = shaUpdateLzma(2, (uint32_t)(pCompBinSectionInfoAddr[index].origAddr+i*APCP_HASH_ONCE_MAX_LEN), (uint32_t)lzmaHashOut, APCP_HASH_ONCE_MAX_LEN, 0);
                    }
                    lengthLeft = pCompBinSectionInfoAddr[index].origLen%APCP_HASH_ONCE_MAX_LEN;
                    RetValue = shaUpdateLzma(2, (uint32_t)(pCompBinSectionInfoAddr[index].origAddr+hashCount*APCP_HASH_ONCE_MAX_LEN), (uint32_t)lzmaHashOut, lengthLeft, 1);
                }
            }
            else
            {
                length = pCompBinSectionInfoAddr[index].origLen;//0x10000;
                if(length <= AP_HASH_MEM_LEN)
                {
                    length /= sizeof(unsigned int);
                    asmbData = (unsigned int *)pCompBinSectionInfoAddr[index].origAddr;
                    msmbData = (unsigned int *)hashBuffStartAddr;

                    while(length > 0)
                    {
                        msmbData[length-1] = asmbData[length-1];
                        length--;
                    }

                    RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, pCompBinSectionInfoAddr[index].origLen, 1);
                }
                else
                {
                    hashCount = pCompBinSectionInfoAddr[index].origLen/AP_HASH_MEM_LEN;
                    for(i=0; i<hashCount; i++)
                    {
                        length = AP_HASH_MEM_LEN/sizeof(unsigned int);
                        asmbData = (unsigned int *)(pCompBinSectionInfoAddr[index].origAddr + i*AP_HASH_MEM_LEN);
                        msmbData = (unsigned int *)hashBuffStartAddr;

                        while(length > 0)
                        {
                            msmbData[length-1] = asmbData[length-1];
                            length--;
                        }

                        RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, AP_HASH_MEM_LEN, 0);
                    }
                    lengthLeft = pCompBinSectionInfoAddr[index].origLen%AP_HASH_MEM_LEN;
                    length = lengthLeft/sizeof(unsigned int);
                    asmbData = (unsigned int *)(pCompBinSectionInfoAddr[index].origAddr + hashCount*AP_HASH_MEM_LEN);
                    msmbData = (unsigned int *)hashBuffStartAddr;

                    while(length > 0)
                    {
                        msmbData[length-1] = asmbData[length-1];
                        length--;
                    }

                    RetValue = shaUpdateLzma(2, (uint32_t)msmbData, (uint32_t)lzmaHashOut, lengthLeft, 1);
                }
            }
            for(i=0; i<APCP_HASH_OUT_MAX_LEN; i++)
            {
                if(lzmaHashOut[i] != pCompBinSectionInfoAddr[index].hash[i])
                {
                    apCompExecuteRet.apDecompHashRet[index] = 0xADE00001;
                    __asm volatile("movw r12, #0xecad");
                    __asm volatile("movt r12, #0xecad");
                    __asm volatile("str  r12, [r12]");
                    break;
                }
            }
            sctDeInitLzma();
        }
    }

    lzma_APTimeRead(&mSysTime2, EC_BC_TYPE_BC_TIME);
    lzmaCurrTime2 = (((mSysTime2.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime2.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime2.hfnsfnsbn & 0xf))*1000 + (mSysTime2.spn*1000)/30720; //jing que dao 0.001ms
    //lzmaCurrTime1 = (((mSysTime2.hfnsfnsbn >> 14)&0x3FF)*10240 + ((mSysTime2.hfnsfnsbn >> 4)&0x3FF)*10 + (mSysTime2.hfnsfnsbn & 0xf));    //jing que dao ms
    apCompExecuteRet.apDecompHashTimeRet[index] = lzmaCurrTime2-lzmaCurrTime1;

    apCompExecuteRet.apDecompHashRet[index] = RetValue;
    #endif
#endif
}
#endif
