/******************************************************************************

 *(C) Copyright 2018 AirM2M International Ltd.

 * All Rights Reserved

 ******************************************************************************
 *  Filename:psram.h
 *
 *  Description:EC718 psram header file
 *
 *  History: 11/06/2021    Originated by bchang
 *
 *  Notes:
 *
 ******************************************************************************/

#ifndef _XPSRAM_EC7XX_H
#define _XPSRAM_EC7XX_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include "ec7xx.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/


/* QSPI Error codes */
#define PSRAM_OK            ((uint8_t)0x00)
#define PSRAM_ERROR         ((uint8_t)0x01)





/* ID-----------------------------------------------------------*/
#define PSRAM_MFID                      0x0D
#define PSRAM_KGD                       0x50



/* Size of page */
#define PSRAM_PAGE_SIZE                   0x400     /* 1024 bytes */


/*********CA start*******************************************/

/*
AP PSRAM: 8bit cmd + 24bits addr   byte addr

all types share same cmd ( AP_PSRAM_OPI_3208_E2_OT not support LSYNC rd/wr cmd)

*/

#define AP_PSRAM_SYNC_READ_CMD                 0x00
#define AP_PSRAM_SYNC_WRITE_CMD                0x80
#define AP_PSRAM_MR_READ_CMD                   0x10040
#define AP_PSRAM_MR_WRITE_CMD                  0x100C0
#define AP_PSRAM_GLOBAL_RESET_CMD              0xFF

#define AP_PSRAM_LSYNC_READ_CMD                0x20
#define AP_PSRAM_LSYNC_WRITE_CMD               0xA0


#define APOB_PSRAM_SYNC_READ_CMD                 0x0000
#define APOB_PSRAM_SYNC_WRITE_CMD                0x8080
#define APOB_PSRAM_MR_READ_CMD                   0x14040
#define APOB_PSRAM_MR_WRITE_CMD                  0x1C0C0
#define APOB_PSRAM_GLOBAL_RESET_CMD              0xFFFF

#define APOB_PSRAM_LSYNC_READ_CMD                0x2020
#define APOB_PSRAM_LSYNC_WRITE_CMD               0xA0A0


/*
WB PSRAM: 48bits CA, 3 words( 1 word = 2 bytes):
CA0[47:40]+CA0[39:32]+CA1[31:24]+CA1[23:16]+CA2[15:8]+CA2[7:0]


all types share same CA format
high 3 bits define the cmd

word addr
1 row = 1KB

256Mb(32MB) device has 32768 rows  X=36
128Mb(16MB) device has 16384 rows  X=35
64Mb (8MB)  device has 8192  rows  X=34
32Mb (4MB)  device has 4096  rows  X=33

row addr size is defined by device density
so row addr is CA[X:22] X is defined by density

upper column addr select which 8 word(named as half page) of the row, lower column addr select one of the 8 words
so upper column addr is always CA[21:16]  lower column addr is always CA[2:0] total 9 bits for 512 words



b[47]: 0 =wr 1=rd
b[46]: 0=mem 1=reg
b[45]  0=wrap burst 1 linear burst
b[44:16] row & upper column addr
b[15:3] resv
b[2:0] lower column addr


because of only support word addr
for read: SW should always read even bytes and ignore one bytes if necessary
for write: use RWDS as a mask


*/

//only 3 bits CA0[47:45] effective, but 43--40 should keep as 0 , so also define as 8 bit data
#define WB956MBY_PSRAM_WRAP_READ_CMD                 0x8000  //0b100
#define WB956MBY_PSRAM_WRAP_WRITE_CMD                0x000   //0b000
#define WB956MBY_PSRAM_MR_READ_CMD                   0x1C000  //0b111 or 0b110 not care linear or wrap bit
#define WB956MBY_PSRAM_MR_WRITE_CMD                  0x16000  //0b011

#define WB956MBY_PSRAM_LINEAR_READ_CMD               0xA000  //0b101
#define WB956MBY_PSRAM_LINEAR_WRITE_CMD              0x2000  //0b001


#define WB_PSRAM_IDMR0_READ_ADDR                0x00000000
#define WB_PSRAM_IDMR1_READ_ADDR                0x00000001


#define WB_PSRAM_CFGMR0_RW_ADDR                0x01000000
#define WB_PSRAM_CFGMR1_RW_ADDR                0x01000001



//only 3 bits CA0[47:45] effective, but 43--40 should keep as 0 , so also define as 8 bit data
#define WB955MKY_PSRAM_WRAP_READ_CMD                 0x8000  //0b100
#define WB955MKY_PSRAM_WRAP_WRITE_CMD                0x000   //0b000
#define WB955MKY_PSRAM_MR_READ_CMD                   0x1e000  //0b111 or 0b110 not care linear or wrap bit
#define WB955MKY_PSRAM_MR_WRITE_CMD                  0x16000  //0b011






/* SCB, similar as APM OB*/
#define SCB_PSRAM_SYNC_READ_CMD                 0x0000
#define SCB_PSRAM_SYNC_WRITE_CMD                0x8080
#define SCB_PSRAM_MR_READ_CMD                   0x14040
#define SCB_PSRAM_MR_WRITE_CMD                  0x1C0C0
#define SCB_PSRAM_GLOBAL_RESET_CMD              0xFFFF

#define SCB_PSRAM_LSYNC_READ_CMD                0x2020
#define SCB_PSRAM_LSYNC_WRITE_CMD               0xA0A0


/*********CA end*******************************************/


/*********timing  start*******************************************/

/*
AP:
DQS for strobe. device drive when read/ host drive when write

AP_PSRAM_OPI_3208_E2_OT    has CLK#
AP_PSRAM_OPI_6408_E3_OT    has CLK#
AP_PSRAM_OPI_6408_E3_OB9   no  CLK#
AP_PSRAM_XPI_25616_E7_OB9  no  CLK#
AP_PSRAM_XPI_12816_E8_OB9  no  CLK#



WB:
RWDS for strobe.device drive when read/ host drive when write

WB_PSRAM_OPI_3208_955MKY   has CLK#
WB_PSRAM_OPI_6408_956MKY   has CLK#
WB_PSRAM_OPI_6408_956NKR   no  CLK#
WB_PSRAM_XPI_12816_957NKR  no  CLK#
WB_PSRAM_XPI_25616_958NKY  has CLK#





read timing tuning
1 tuning the DQS/DQ delay, to let DQS and DQ has less skew ----align tuning

use recursion method

step1: write  pattern 0x00ff0000 and read back, get the skew cnt for each DQ
step2: set delay chain according step1, go back to step1, until skew cnt is 1

2 sample point tuning
2 methods: use clk_3g to sample    or use dqs for sample
method1:
step1: write  pattern 0x10101010
step2: set pa_rx_0_smp_option = 0
step3: try rx_0_gt_positon from 1----7, get the min/max which will cause incorrect read back.
set the final rx_0_gt_positon to the mid-val. e.g min=2 max=6, set rx_0_gt_positon =4

method2:
step1: write  pattern 0x10101010
step2: set pa_rx_0_smp_option = 1
step3: try rx_0_dqs_smp_sel from 1----7, get the min/max which will cause incorrect read back.
set the final rx_0_dqs_smp_sel to the mid-val. e.g min=2 max=6, set rx_0_gt_positon =4

3 HW real time checking, will report int to SW if any err, SW should handle the fatal error

1)dqs track( q data check must en)

2)dq data check


*/





/*********timing  end*******************************************/



/*********frame direction start*******************************************/
/*

for all wr:
1 in bus(xip) mode, controller will handle the detail
2 in sw mode, wr len should: 2n & max=8B
  sw need to set mask(cmd_wr_data_0/1_mask_sw) if odd bytes is written
for all rd:
1 in bus(xip) mode, controller will handle the detail
2 in sw mode, rd len should: 2n & max=8B
  sw need to ignore no need byte



AP_PSRAM_OPI_3208_E2_OT

    linerar wr: not support
    linerar rd: not support
    wrap wr:
            8bit inst+24 bit byte addr(but should be even)
            write end dummy: 1 cycle has dedicate DM PIN
            wr lt: 0 or 2(>166Mhz)
            wr len should min=2B
    wrap rd:
            8bit inst+24 bit byte addr
            rd lt: 2@66M 3@109 4@133 5@166 6@200
    reg rd :
            8bit inst+24 bit byte addr(only LSB 8 bit is valid)
            rd lt: 5 cycle
    reg wr :
            8bit inst+24 bit byte addr(only LSB 8 bit is valid)
            wr lt: 0 or 2(>166Mhz)

    global rst:
            8bit inst(0xff)+ 0 addr bit + 4 dummy cycles
            no dedicate reset PIN


AP_PSRAM_OPI_6408_E3_OT

    linerar wr: same as 1K  wrap
    wrap wr:
            8bit inst+24 bit byte addr(but should be even)
            write end dummy: 1 cycle  has dedicate DM PIN
            wr lt: 0 or 2(>166Mhz) 4 is resv
            wr len should min=4B
    linerar rd: same as 1K  wrap
    wrap rd:
            8bit inst+24 bit byte addr
            rd lt: 2@66M 3@109 4@133 5@166 6@200
    reg rd :
            8bit inst+24 bit byte addr(only LSB 8 bit is valid)
            rd lt: same as setting in rd
    reg wr :
            8bit inst+24 bit byte addr(only LSB 8 bit is valid)
            wr lt: 0 or 2(>166Mhz)

    global rst:
            8bit inst(0xff)+ 0 addr bit + 4 dummy cycles
            no dedicate reset PIN


AP_PSRAM_OPI_6408_E3_OB9

    linerar wr: same as 1K  wrap
    wrap wr:
            8bit inst+32 bit byte addr(but should be even)
            write end dummy: no, so has no dedicate DM PIN
            wr lt: 3@66M 4@104 5@133 6@166 7@200 8@250
            wr len should min=2B
    linerar rd: same as 1K  wrap, also support !RBX! (cross 1K row)
    wrap rd:
            8bit inst+32 bit byte addr
            rd lt: 3@66M 4@104 5@133 6@166 7@200 8@250
    reg rd :
            8bit inst+32 bit byte addr(only LSB 8 bit is valid)
            rd lt: same as setting in rd
    reg wr :
            8bit inst+32 bit byte addr(only LSB 8 bit is valid)
            wr lt: always latency 1, not related to wr lt setting

    global rst:
            8bit inst(0xff)+ 0 addr bit + 4 dummy cycles
            has dedicate reset PIN


AP_PSRAM_XPI_25616_E7_OB9
    add later
AP_PSRAM_XPI_12816_E8_OB9
    add later





WB_PSRAM_OPI_3208_955MKY
    linerar wr: no linear
    wrap wr:
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            write end dummy: no
            wr lt: 3@83M 4@104 5@133 6@233
            wr len should min=2B
    linerar rd: no linear
    wrap rd:
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            rd lt: 3@83M 4@104 5@133 6@233
    reg rd :
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            rd lt: same as setting in rd
    reg wr :
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            wr lt: no latency

    global rst:
            no
            has dedicate reset PIN
            tvcs = max150us


WB_PSRAM_OPI_6408_956MBY
    linerar wr: real linear
    wrap wr:
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            write end dummy: no
            wr lt: 3@83M 4@100 5@133 6@166 7@200
            wr len should min=2B
    linerar rd: real linear
    wrap rd:
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            rd lt: 3@83M 4@100 5@133 6@166 7@200
    reg rd :
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            rd lt: same as setting in rd
    reg wr :
            2bit inst+1 bit burst type + 45bit addr(inculding resv)
            wr lt: no latency

    global rst:
            no
            has dedicate reset PIN
            tvcs = max150us



WB_PSRAM_OPI_6408_956MBY
WB_PSRAM_OPI_6408_956MKY
WB_PSRAM_OPI_6408_956NKR
WB_PSRAM_XPI_12816_957NKR
WB_PSRAM_XPI_25616_958NKY
    add later



*/







/*********frame direction end*******************************************/



/*psram  cmd pattern dummy cycles*/
#define DUMMY_CYCLES_FAST_READ_QUAD_QPI             6 //0xeb instruction

#define DUMMY_CYCLES_FAST_READ_QUAD_SPI             6 //0xeb instruction


#define DUMMY_CYCLES_MR_READ_QPI                    6 //0xb5 instruction
#define DUMMY_CYCLES_MR_READ_SPI                    8 //0xb5 instruction







/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/


/**
  * @brief xpi psram vendor enum
  */
typedef enum
{
    PSRAM_VENDOR_WB = 0x0,
    PSRAM_VENDOR_AP,
    PSRAM_VENDOR_GX,
    PSRAM_VENDOR_GD,
    PSRAM_VENDOR_INVALID
}XpiPsramVendor_e;



/**
  * @brief xpi psram type enum
  */
typedef enum
{

    AP_PSRAM_OPI_6408_E3_OB9 =0x0,//8M

    WB_PSRAM_OPI_3208_955MKY,//4M
    WB_PSRAM_OPI_6408_956NKR,//8M

    SCB_PSRAM_18X032800AF,//4M

    GSR_PSRAM_5x8AM, //4M
    GSR_PSRAM_6x8DM, //8M
    XPI_PSRAM_INVALID_PART_NUM,
    XPI_PSRAM_MAX_SUPPORT_NUM
}XpiPsramType_e;

/**
  * @brief xpi psram type enum
  */
typedef enum
{
    WB_ID_REG0,
    WB_ID_REG1,
    WB_CFG_REG0,
    WB_CFG_REG1,
    WB_REG_MAX_SUPPORT_NUM
}WbPsramRegType_e;


typedef enum
{
    XPSRAM_RET_NONE = 0,
    XPSRAM_RET_SLEEP1 = 1,
    XPSRAM_RET_SLEEP2 = 2,
    XPSRAM_RET_HIBNATE = 3,
}XpiPsramRet_e;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/

/**
  \fn        XPSRAM_getSipPsramType( void )
  \brief     used to read psram type info from fuse
  \note     will read fuse
 */
XpiPsramType_e XPSRAM_getSipPsramType( void );


/**
  \fn        XPSRAM_readAPMReg(uint8_t regNum)
  \brief     used to read apm mode register
  \param[in] regNum register num
  \note      will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint8_t XPSRAM_readAPMReg(uint8_t regNum);



/**
  \fn        XPSRAM_writeAPMReg(uint8_t regNum, uint8_t setVal )
  \brief     used to write apm mode register
  \param[in] regNum register num
             setVal val to write
  \note      will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint8_t XPSRAM_writeAPMReg(uint8_t regNum, uint8_t setVal );


/**
  \fn        XPSRAM_readWBReg(WbPsramRegType_e regType)
  \brief     used to read wb mode register
  \param[in] regType register type
  \note      will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint16_t XPSRAM_readWBReg(WbPsramRegType_e regType);


/**
  \fn        XPSRAM_writeWBReg(WbPsramRegType_e regType , uint16_t setVal)
  \brief     used to write wb mode register
  \param[in] regType register type
             setVal val to write
  \note      will lock sw force mode, only use when init and no xip access to PSRAM
 */

uint8_t XPSRAM_writeWBReg(WbPsramRegType_e regType , uint16_t setVal);


/**
  \fn        XPSRAM_readWB(uint32_t* pData, uint32_t readAddr, uint32_t size)
  \brief     used to read SW read wb psram
  \param[in] pData store read data
             readAddr addr to read
             size read size max 8 bytes
  \note      only for test,max read 8 bytes
             will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint16_t XPSRAM_readWB(uint32_t* pData, uint32_t readAddr, uint32_t size);


/**
  \fn        XPSRAM_writeWB(uint32_t* pData, uint32_t writeAddr, uint32_t size)
  \brief     used to read SW write wb psram
  \param[in] pData write data ptr
             readAddr addr to write
             size write size max 8 bytes
  \note      only for test,max write 8 bytes
             will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint8_t XPSRAM_writeWB(uint32_t* pData, uint32_t writeAddr, uint32_t size);


/**
  \fn        XPSRAM_readAPM(uint32_t* pData, uint32_t readAddr, uint32_t size)
  \brief     used to read SW read apm psram
  \param[in] pData store read data
             readAddr addr to read
             size read size max 8 bytes
  \note      only for test,max read 8 bytes
             will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint16_t XPSRAM_readAPM(uint32_t* pData, uint32_t readAddr, uint32_t size);


/**
  \fn        XPSRAM_writeAPM(uint32_t* pData, uint32_t writeAddr, uint32_t size)
  \brief     used to read SW write apm psram
  \param[in] pData write data ptr
             readAddr addr to write
             size write size max 8 bytes
  \note      only for test,max write 8 bytes
             will lock sw force mode, only use when init and no xip access to PSRAM
 */
uint8_t XPSRAM_writeAPM(uint32_t* pData, uint32_t writeAddr, uint32_t size);


/**
  \fn        XPSRAM_skewTuning( void )
  \brief     used to tuning dqs and dq[7:0] skew
  \note
*/
uint8_t XPSRAM_skewTuning( void );


/**
  \fn        XPSRAM_sampleTuning( void )
  \brief     used to tuning smaple point
  \param[in]
  \note
 */
uint8_t XPSRAM_sampleTuning( void );


/**
  \fn        XPSRAM_wbBistTest( void )
  \brief     used to perform wb psram bist
  \param[in]
  \note
 */
uint8_t XPSRAM_wbBistTest( void );

/**
   \fn        XPSRAM_rxCheckandTrackEn( void )
   \brief     used to en xpi controller check and track
   \note      called both in when init psram
*/
void  XPSRAM_rxCheckandTrackEn( void );


/**
   \fn        void PSRAM_qspiInit( bool isNeedCfgReg )
   \brief     used to init psram xpi controller
   \param[in] isNeedCfgReg is need really cfg the controller
   \note      called both in BL and APP
*/
 void XPSRAM_qspiInit( bool isNeedCfgReg );

/**
 \fn        uint8_t PSRAM_swReset( void )
 \brief     used to reset psram via send cmd
 \param[in]
 \note
*/
 uint8_t XPSRAM_swReset( void );


/**
 \fn        uint8_t XPSRAM_hwReset( void )
 \brief     used to reset psram via reset pin
 \param[in]
 \note      sw take control the reset pin
*/
uint8_t XPSRAM_hwReset( void );


/**
 \fn        uint8_t PSRAM_dmaAccessClkCtrl( BOOL onoff )
 \brief     used to en/disen psram clk when dma access
 \param[in] onoff: TRUE enable clk  FALSE disable clk
 \note      for open source usage
*/
uint8_t XPSRAM_dmaAccessClkCtrl( BOOL onoff );

/**
 \fn        uint8_t XPSRAM_init(void)
 \brief     used to init all psram related feature
            called by chip init process by ap.
 \note      not dis-int, take care only called in no task/int function
*/
 uint8_t XPSRAM_init(void);

/**
 \fn        XPSRAM_setRetMode(bool isPartialRet, XpiPsramRet_e retCfg)
 \brief     used to cfg psram ret mode
 \param[in] isPartialRet: is need PartialRet, not apply now
            retCfg cfg the ret mode
 \note      not dis-int, take care only called in no task/int function
*/
void XPSRAM_setRetMode(bool isPartialRet, XpiPsramRet_e retCfg);


/**
 \fn        XPSRAM_preSleepFlow(uint8_t state)
 \brief     called in apmu process before enter sleep
 \param[in] state: soc sleep mode
 \note      not dis-int, take care only called in no task/int function
*/
bool XPSRAM_preSleepFlow(uint8_t state);

/**
 \fn        XPSRAM_csDirectCtrl(void)
 \brief     called in apmu process to exit from psram ret mode
 \note      not dis-int, take care only called in no task/int function
*/
void XPSRAM_csDirectCtrl(void);

/**
 \fn        XPSRAM_postSleepFlow(void)
 \brief     called in apmu process after exit from sleep
 \note      not dis-int, take care only called in no task/int function
*/
void XPSRAM_postSleepFlow(void);


#if !defined(FEATURE_BOOTLOADER_PROJECT_ENABLE)
uint8_t XPSRAM_enterHySlp(void);
#endif


#ifdef __cplusplus
}
#endif


#endif

