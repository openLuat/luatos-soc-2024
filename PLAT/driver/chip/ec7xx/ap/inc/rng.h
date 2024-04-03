#ifndef __RNG_EC718_H__
#define __RNG_EC718_H__
#include "ec7xx.h"
#include "Driver_Common.h"
#include "clock.h"
#include "ic.h"
#include "string.h"


typedef struct 
{
    __IM uint32_t Rsv00_FC[64];
    __IO uint32_t IMR;                          //Interrupt mask reg ,0x100
    //__IM uint32_t ISR;                          //Interrupt status reg
    __IO uint32_t ISR;                          //Interrupt status reg
    
    __OM uint32_t ICR;                          //Interrupt clear reg
    __IO uint32_t Config;                       //Configuration reg
    __IM uint32_t Valid;                        //Valid reg
    __IM uint32_t Rsv114_128[6];
    __IO uint32_t SourceEnable;         //Random source enable reg
    __IO uint32_t SampleCnt1;           //Sample Count reg
    __IO uint32_t AutoCorrStatistic;  //Auto correlation reg
    __IM uint32_t DebugControl;         //Debug Control reg
    __IM uint32_t Rsv13C;
    __OM uint32_t SwReset;              //Reset reg
    __IM uint32_t Rsv144_1B4[29];
    __IM uint32_t Busy;                     //Busy reg
    __OM uint32_t RstBitsCounter;   //Reset bits counter reg
    __IM uint32_t Rsv1C0_1DC[8];
    __IM uint32_t BistCntr0_1_2[3]; //BIST Counter reg
    __IO uint32_t Rsv1EC_1FC[5];
}RNGDesc;

typedef enum
{
    RngSrcShortestType = 0,
    RngSrcShortType = 1,
    RngSrcLongType = 2,
    RngSrcLongestType = 3,
}RngSrcSelType;

#define RNGDRV_OK 0
#define RNGDRV_IntErr (-1)
#define RNGDRV_TimeOutErr (-2)

typedef enum{
    RNGEHRValidType = 1,
    RNGAutoCorrErrtype = 2,
    RNGCRNGTErrType = 4,
    RNGVNErrType = 8
}RNGIntStatType;

#define RNG_MAX_POLL_DELAY_US (8000*2)  
#define RNG_MAX_TRY_NUM 10
#define RNGPtr ((RNGDesc*)GP_TRNG_BASE_ADDR)

int32_t rngGenRandom(uint8_t Rand[24]);

#endif

