#ifndef  BSP_LSPI_H
#define  BSP_LSPI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stdio.h"
#include "string.h"
#include "ec7xx.h"
#include "bsp.h"
#include "exception_process.h"


////////////////////////////////////////////////////////
typedef void (*lcdSlp1Cb_fn)();
extern lcdSlp1Cb_fn            lcdSlp1CbFn;


/** \brief LSPI DMA */
typedef struct 
{
    DmaInstance_e       txInstance;                 // Transmit DMA instance number
    int8_t              txCh;                       // Transmit channel number
    uint8_t             txReq;                      // Transmit DMA request number
    void                (*txCb)(uint32_t event);    // Transmit callback
    DmaDescriptor_t     *descriptor;                // Tx descriptor
} lspiDma_t;

// LSPI PINS
typedef const struct
{
#if (LCD_INTERFACE_SPI == 1)
    const PIN           *pinDs;                     //  spi ds
    const PIN           *pinClk;                    //  spi clk
    const PIN           *pinCs;                     //  spi cs
    const PIN           *pinMiso;                   //  spi din
    const PIN           *pinMosi0;                  //  spi d0
#if (SPI_2_DATA_LANE == 1)
    const PIN           *pinMosi1;                  //  spi d1
#endif

#elif (LCD_INTERFACE_MSPI == 1)
    const PIN           *pinScl;                    //  mspi clk
    const PIN           *pinCs;                     //  mspi cs
    const PIN           *pinDin;                    //  mspi din
    const PIN           *pinD0;                     //  mspi d0
    const PIN           *pinD1;                     //  mspi d1
    const PIN           *pinD2;                     //  mspi d2
    const PIN           *pinD3;                     //  mspi d3
#elif (LCD_INTERFACE_8080 == 1)
    const PIN           *pinScl;                    //  8080 wclk
    const PIN           *pinCs;                     //  8080 cs
    const PIN           *pinDcx;                    //  8080 dcx
    const PIN           *pinRclk;                   //  8080 rclk
    const PIN           *pinD0;                     //  8080 d0
    const PIN           *pinD1;                     //  8080 d1
    const PIN           *pinD2;                     //  8080 d2
    const PIN           *pinD3;                     //  8080 d3
    const PIN           *pinD4;                     //  8080 d4
    const PIN           *pinD5;                     //  8080 d5
    const PIN           *pinD6;                     //  8080 d6
    const PIN           *pinD7;                     //  8080 d7
#endif
} lspiPins_t;

typedef struct
{
    uint8_t     dataListIndex;
    uint8_t     trans;
    uint8_t     dataLen;
    uint32_t    tmp;
    uint32_t    dataList[16];
} lspiPrepareSendInfo_t;


// LSPI information (Run-time)
typedef struct 
{
    uint32_t                busSpeed;              // LSPI bus speed
    lspiPrepareSendInfo_t   prePareSendInfo;       // Prepare Send info
} lspiRteInfo_t;


// SPI Resources definition
typedef struct 
{
    LSPI_TypeDef         *reg;                     // SPI register pointer
    lspiPins_t           pins;                     // SPI PINS configuration
    lspiDma_t            *dma;                     // SPI DMA configuration pointer
    lspiRteInfo_t        *info;                    // Run-Time Information
} lspiRes_t;


/**
\brief General power states
*/
typedef enum 
{
    LSPI_POWER_OFF,                                 // Power off: no operation possible
    LSPI_POWER_FULL                                 // Power on: full operation at maximum performance
} lspiPowerState_e;

#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
typedef struct
{
    uint32_t slaveModeEn            : 1;            // 718 Slave Mode Enable
    uint32_t slotSize               : 5;            // 718 Slot Size
    uint32_t wordSize               : 5;            // 718 Word Size
    uint32_t alignMode              : 1;            // 718 Align Mode
    uint32_t endianMode             : 1;            // 718 Endian Mode
    uint32_t dataDly                : 2;            // 718 Data Delay 
    uint32_t txPad                  : 2;            // 718 Tx padding 
    uint32_t rxSignExt              : 1;            // 718 Rx Sign Entension
    uint32_t txPack                 : 2;            // 718 Tx Pack
    uint32_t rxPack                 : 2;            // 718 Rx Pack                -D21
    uint32_t txFifoEndianMode       : 1;            // 718 Tx Fifo Endian Mode    -D22
    uint32_t rxFifoEndianMode       : 1;            // 718 Rx Fifo Endian Mode    -D24
    uint32_t eorMode                : 1;            // 718 send last byte for DMA
}lspiDataFmt_t;

typedef struct
{
    uint32_t rxDmaReqEn             : 1;            // 718 Tx Dma Req Enable 
    uint32_t txDmaReqEn             : 1;            // 718 Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn         : 1;            // 718 Tx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle       : 5;            // 718 Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1     : 4;            // 718 Tx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1     : 4;            // 718 Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold        : 4;            // 718 Tx Dma Threadhold
    uint32_t txDmaThreadHold        : 4;            // 718 Tx Dma Threadhold
    uint32_t rxFifoFlush            : 1;            // 718 Tx Fifo flush
    uint32_t txFifoFlush            : 1;            // 718 Tx Fifo flush
}lspiDmaCtrl_t;

typedef struct
{
    uint32_t txUnderRunIntEn        : 1;            // 718 Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn          : 1;            // 718 Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn             : 1;            // 718 Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn        : 1;            // 718 Tx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn          : 1;            // 718 Tx Dma Err Interrupt Enable
    uint32_t rxDatIntEn             : 1;            // 718 Tx Data Interrupt Enable
    uint32_t rxTimeOutIntEn         : 1;            // 718 Tx Timeout Interrupt Enable
    uint32_t fsErrIntEn             : 1;            // 718 Frame Start Interrupt Enable
    uint32_t frameStartIntEn        : 1;            // 718 Frame End Interrupt Enable
    uint32_t frameEndIntEn          : 1;            // 718 Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn    : 1;            // 718 Not use
    uint32_t lspiRamWrBreakIntEn    : 1;            // 718 Lspi ram wr break int enable
    uint32_t lspiRamWrFrameStartEn  : 1;            // 718 Lspi ram wr Frame start enable
    uint32_t lspiRamWrFrameEndEn    : 1;            // 718 Lspi ram wr Frame end enable
    uint32_t lspiCmdEndEn           : 1;            // 718 Lspi sending command end
    uint32_t cspiOtsuEndEn          : 1;            // 718 Cspi OTSU one frame end enable
    uint32_t lspiRamWrEndEn         : 1;            // 718 Lspi Ram wr end int enable
    uint32_t txIntThreshHold        : 4;            // 718 Tx Interrupt Threadhold 
    uint32_t rxIntThreshHold        : 4;            // 718 Tx Interrupt Threadhold
}lspiIntCtrl_t;

typedef struct
{
    uint32_t rxTimeOutCycle         : 24;           // 718 Tx Timeout Cycle
    uint32_t dummyCycle             : 4;            // 718 Dummy cycle
}lspiTimeOutCycle_t;


typedef struct
{
    uint32_t txUnderRun             : 1;            // 718 Tx Underrun
    uint32_t txDmaErr               : 1;            // 718 Tx Dma Err
    uint32_t txDataRdy              : 1;            // 718 Tx Data ready, readOnly
    uint32_t rxOverFlow             : 1;            // 718 Tx OverFlow
    uint32_t rxDmaErr               : 1;            // 718 Tx Dma Err
    uint32_t rxDataRdy              : 1;            // 718 Tx Data ready, readOnly
    uint32_t rxFifoTimeOut          : 1;            // 718 Tx Fifo timeout
    uint32_t fsErr                  : 4;            // 718 Frame synchronization Err
    uint32_t frameStart             : 1;            // 718 Frame start
    uint32_t frameEnd               : 1;            // 718 Frame end
    uint32_t txFifoLevel            : 6;            // 718 Tx Fifo Level, readOnly
    uint32_t rxFifoLevel            : 6;            // 718 Tx Fifo level, readOnly
    uint32_t cspiBusTimeOut         : 1;            // 718 Cspi Bus timeout
    uint32_t lspiRamWrBreak         : 1;            // 718 Lspi ram wr break
    uint32_t lspiRamWrFrameStart    : 1;            // 718 Lspi ram wr frame start
    uint32_t lspiRamWrFrameEnd      : 1;            // 718 Lspi ram wr frame end
    uint32_t cspiOtsuEnd            : 1;            // 718 Cspi otsu one frame end
    uint32_t lspiRamWrEnd           : 1;            // 718 Lspi ram wr end
}lspiStats_t;

typedef struct
{
    uint32_t enable                 : 1;            // 718 lspi Enable
    uint32_t data2Lane              : 1;            // 718 2 data lane enable
    uint32_t line4                  : 1;            // 718 0: not use port as DCX; 1: use port as DCX
    uint32_t datSrc                 : 1;            // 718 data from camera or memory
    uint32_t colorModeIn            : 2;            // 718 Input data color mode
    uint32_t colorModeOut           : 3;            // 718 Output data color mode
    uint32_t yAdjEn                 : 1;            // 718 Y adjust enable
    uint32_t yAdjSel                : 1;            // 718 Y adjustment from cspi or sw
    uint32_t yAdjBound              : 17;           // 718 Y adjustment bound
    uint32_t dcDummy                : 1;            // 718 Send DCX or dummy
    uint32_t busType                : 1;            // 718 718-> 0: Interface I, SDA=INOUT; 1: Interface II, SDA=input, SDO=output
}lspiCtrl_t;

typedef struct
{
    uint32_t wrRdn                  : 1;            // 718 0:rd; 1:wr
    uint32_t ramWr                  : 1;            // 718 start to fill frame memory
    uint32_t rdatDummyCycle         : 6;            // 718 Dummy cycle before data read
    uint32_t dataLen                : 18;           // 718 data len for wr/rd
    uint32_t rsvd                   : 5;
    uint32_t init                   : 1;            // 718 0:lspi normal; 1:lspi initial
}lspiCmdCtrl_t;

typedef struct
{
    uint32_t addr                   : 8;            // 718 command addr
}lspiCmdAddr_t;


typedef struct
{
    uint32_t idle                   : 1;            // 718 finish formar command or not
}lspiCmdStats_t;

typedef struct
{
    uint32_t ramWrLen               : 18;           // 718 Len of ramwr
}lspiRamWrLen_t;

typedef struct
{
    uint32_t frameHeight            : 16;           // 718 frame height
    uint32_t frameWidth             : 16;           // 718 frame weight
}lspiInfo_t;

typedef struct
{
    uint32_t tailorBottom           : 10;           // 718 cut bottom lines
    uint32_t tailorTop              : 10;           // 718 cut top lines
}lspitailorInfo0_t;

typedef struct
{
    uint32_t tailorLeft             : 10;           // 718 cut left lines
    uint32_t tailorRight            : 10;           // 718 cut right lines
}lspitailorInfo_t;

typedef struct
{
    uint32_t rowScaleFrac           : 7;            // 718 row scale frac
    uint32_t                        : 1;
    uint32_t colScaleFrac           : 7;            // 718 col scale frac
}lspiScaleInfo_t;

typedef struct
{
    uint32_t grayCtrl               : 2;            // 718 gray ctrl
    uint32_t quartileSel            : 1;            // 718 quartile from cspi or sw
    uint32_t quartile1              : 8;            // 718 quartile 1
    uint32_t quartile2              : 8;            // 718 quartile 2
    uint32_t quartile3              : 8;            // 718 quartile 3
}lspiQuartileCtrl_t;

typedef struct
{
    uint32_t quartile1InUse         : 8;            // 718 quartile 1 in use
    uint32_t quartile2InUse         : 8;            // 718 quartile 2 in use
    uint32_t quartile3InUse         : 8;            // 718 quartile 3 in use
}lspiQuartileInUse_t;

typedef struct
{
    uint32_t yadjYmin               : 8;            // 718 y adj min
    uint32_t yadjYmax               : 8;            // 718 y adj max
    uint32_t yadjStrech             : 8;            // 718 y adj stretch 
    uint32_t yadjStrechFwl          : 8;            // 718 y adj stretch fwl
}lspiYAdj_t;

typedef struct
{
    uint32_t yadjYminInUse          : 8;            // 718 y adj min
    uint32_t yadjYmaxInUse          : 8;            // 718 y adj max
    uint32_t yadjStrechInUse        : 8;            // 718 y adj stretch 
    uint32_t yadjStrechFwlInUse     : 8;            // 718 y adj stretch fwl
}lspiYAdjInUse_t;


typedef struct
{
    uint32_t pageCmd                : 8;            // 718 page cmd
    uint32_t pageCmd0               : 16;           // 718 page cmd 0
    uint32_t                        : 4;
    uint32_t pageCmd01ByteNum       : 4;            // 718 page cmd0 + page cmd1 byte num 
}lspiGrayPageCmd0_t;

typedef struct
{
    uint32_t pageCmd1               : 32;           // 718 page cmd1
}lspiGrayPageCmd1_t;

typedef struct
{
    uint32_t frameHeightOut         : 10;           // 718 frame height out
    uint32_t frameWidthOut          : 10;           // 718 frame width out
}lspiFrameInfoOut_t;

typedef struct
{
    uint32_t i2sBusEn               : 1;            // 718 I2S bus enable
    uint32_t cspiBusEn              : 1;            // 718 Cspi bus enable
    uint32_t lspiBusEn              : 1;            // 718 Lspi bus enable
}lspiBusSel_t;

extern lspiDataFmt_t lspiDataFmt;
extern lspiDmaCtrl_t lspiDmaCtrl;
extern lspiIntCtrl_t lspiIntCtrl;
extern lspiCtrl_t    lspiCtrl;
extern lspiCmdCtrl_t lspiCmdCtrl;
extern lspiCmdAddr_t lspiCmdAddr;
extern lspiInfo_t    lspiInfo;
extern lspiYAdj_t    lspiYAdj;
extern lspiBusSel_t  lspiBusSel;
extern lspitailorInfo0_t  lspiTailorInfo0;
extern lspitailorInfo_t   lspiTailorInfo;
extern lspiScaleInfo_t    lspiScaleInfo;
extern lspiQuartileCtrl_t lspiQuartileCtrl;
extern lspiGrayPageCmd0_t lspiGrayPageCmd0;
extern lspiGrayPageCmd1_t lspiGrayPageCmd1;
extern lspiFrameInfoOut_t lspiFrameInfoOut;


#else // CHIP 719
typedef struct
{
    uint32_t slaveModeEn            : 1;            // 719 Slave Mode Enable
    uint32_t slotSize               : 5;            // 719 Slot Size
    uint32_t wordSize               : 5;            // 719 Word Size
    uint32_t alignMode              : 1;            // 719 Align Mode
    uint32_t endianMode             : 1;            // 719 Endian Mode
    uint32_t dataDly                : 2;            // 719 Data Delay 
    uint32_t txPad                  : 2;            // 719 Tx padding 
    uint32_t rxSignExt              : 1;            // 719 Rx Sign Entension
    uint32_t txPack                 : 2;            // 719 Tx Pack
    uint32_t rxPack                 : 2;            // 719 Rx Pack                -D21
    uint32_t txFifoEndianMode       : 1;            // 719 Tx Fifo Endian Mode    -D22
    uint32_t rxFifoEndianMode       : 1;            // 719 Rx Fifo Endian Mode    -D24
    uint32_t eorMode                : 1;            // 719 send last byte for DMA
}lspiDataFmt_t;

typedef struct
{
    uint32_t rxDmaReqEn             : 1;            // 719 Tx Dma Req Enable 
    uint32_t txDmaReqEn             : 1;            // 719 Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn         : 1;            // 719 Tx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle       : 5;            // 719 Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1     : 4;            // 719 Tx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1     : 4;            // 719 Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold        : 4;            // 719 Tx Dma Threadhold
    uint32_t txDmaThreadHold        : 4;            // 719 Tx Dma Threadhold
    uint32_t rxFifoFlush            : 1;            // 719 Tx Fifo flush
    uint32_t txFifoFlush            : 1;            // 719 Tx Fifo flush
}lspiDmaCtrl_t;

typedef struct
{
    uint32_t txUnderRunIntEn        : 1;            // 719 Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn          : 1;            // 719 Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn             : 1;            // 719 Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn        : 1;            // 719 Tx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn          : 1;            // 719 Tx Dma Err Interrupt Enable
    uint32_t rxDatIntEn             : 1;            // 719 Tx Data Interrupt Enable
    uint32_t rxTimeOutIntEn         : 1;            // 719 Tx Timeout Interrupt Enable
    uint32_t fsErrIntEn             : 1;            // 719 Frame Start Interrupt Enable
    uint32_t frameStartIntEn        : 1;            // 719 Frame End Interrupt Enable
    uint32_t frameEndIntEn          : 1;            // 719 Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn    : 1;            // 719 Not use
    uint32_t lspiRamWrBreakIntEn    : 1;            // 719 Lspi ram wr break int enable
    uint32_t lspiRamWrFrameStartEn  : 1;            // 719 Lspi ram wr Frame start enable
    uint32_t lspiRamWrFrameEndEn    : 1;            // 719 Lspi ram wr Frame end enable
    uint32_t lspiCmdEndEn           : 1;            // 719 Lspi sending command end
    uint32_t cspiOtsuEndEn          : 1;            // 719 Cspi OTSU one frame end enable
    uint32_t lspiRamWrEndEn         : 1;            // 719 Lspi Ram wr end int enable
    uint32_t txIntThreshHold        : 4;            // 719 Tx Interrupt Threadhold 
    uint32_t rxIntThreshHold        : 4;            // 719 Tx Interrupt Threadhold
    uint32_t ramWrIntCtrl           : 3;            // 719 Lspi Ram write Int ctrl
    uint32_t ramWrIntCtrlEn         : 1;            // 719 Lspi Ram write Int ctrl enable bit
}lspiIntCtrl_t;

typedef struct
{
    uint32_t rxTimeOutCycle         : 24;           // 719 Tx Timeout Cycle
    uint32_t dummyCycle             : 4;            // 719 Dummy cycle
}lspiTimeOutCycle_t;


typedef struct
{
    uint32_t txUnderRun             : 1;            // 719 Tx Underrun
    uint32_t txDmaErr               : 1;            // 719 Tx Dma Err
    uint32_t txDataRdy              : 1;            // 719 Tx Data ready, readOnly
    uint32_t rxOverFlow             : 1;            // 719 Tx OverFlow
    uint32_t rxDmaErr               : 1;            // 719 Tx Dma Err
    uint32_t rxDataRdy              : 1;            // 719 Tx Data ready, readOnly
    uint32_t rxFifoTimeOut          : 1;            // 719 Tx Fifo timeout
    uint32_t fsErr                  : 4;            // 719 Frame synchronization Err
    uint32_t frameStart             : 1;            // 719 Frame start
    uint32_t frameEnd               : 1;            // 719 Frame end
    uint32_t txFifoLevel            : 6;            // 719 Tx Fifo Level, readOnly
    uint32_t rxFifoLevel            : 6;            // 719 Tx Fifo level, readOnly
    uint32_t cspiBusTimeOut         : 1;            // 719 Cspi Bus timeout
    uint32_t lspiRamWrBreak         : 1;            // 719 Lspi ram wr break
    uint32_t lspiRamWrFrameStart    : 1;            // 719 Lspi ram wr frame start
    uint32_t lspiRamWrFrameEnd      : 1;            // 719 Lspi ram wr frame end
    uint32_t cspiOtsuEnd            : 1;            // 719 Cspi otsu one frame end
    uint32_t lspiRamWrEnd           : 1;            // 719 Lspi ram wr end
}lspiStats_t;

typedef struct
{
    uint32_t enable                 : 1;            // 719 lspi Enable
    uint32_t dspiEn                 : 1;            // 719 dual spi mode enable
    uint32_t line4                  : 1;            // 719 0: not use port as DCX; 1: use port as DCX
    uint32_t datSrc                 : 1;            // 719 data from camera or memory
    uint32_t colorModeIn            : 3;            // 719 Input data color mode
    uint32_t colorModeOut           : 3;            // 719 Output data color mode
    uint32_t yAdjEn                 : 1;            // 719 Y adjust enable
    uint32_t yAdjSel                : 1;            // 719 Y adjustment from cspi or sw
    uint32_t yAdjBound              : 17;           // 719 Y adjustment bound
    uint32_t dcDummy                : 1;            // 719 Send DCX or dummy
    uint32_t dspiCfg                : 2;            // 719 0: RGB565/666/888;  1: rsv; 2: RGB444/565/666/888;  3: RGB565/888
}lspiCtrl_t;

typedef struct
{
    uint32_t wrRdn                  : 1;            // 719 0:rd; 1:wr
    uint32_t ramWr                  : 1;            // 719 start to fill frame memory
    uint32_t rdatDummyCycle         : 6;            // 719 Dummy cycle before data read
    uint32_t dataLen                : 22;           // 719 data len for wr/rd
    uint32_t ramWrHaltMode          : 1;            // 719 0:pull up lspi_csn to stop trans; 1: lspi_csn is low, by stopping lspi_clk to stop trans;
    uint32_t init                   : 1;            // 719 0:lspi normal; 1:lspi initial
}lspiCmdCtrl_t;

typedef struct
{
    uint32_t addr                   : 8;            // 719 command addr
    uint32_t csnHighCycleMin        : 8;            // 719 the num cycles(lspi_clk) during lspi_csn high 
    uint32_t rsb                    : 15;
    uint32_t busType                : 1;            
}lspiCmdAddr_t;


typedef struct
{
    uint32_t idle                   : 1;            // 719 finish formar command or not
}lspiCmdStats_t;

typedef struct
{
    uint32_t ramWrLen               : 18;           // 719 Len of ramwr
}lspiRamWrLen_t;

typedef struct
{
    uint32_t frameHeight            : 16;           // 719 frame height
    uint32_t frameWidth             : 16;           // 719 frame weight
}lspiInfo_t;

typedef struct
{
    uint32_t tailorBottom           : 10;           // 719 cut bottom lines
    uint32_t tailorTop              : 10;           // 719 cut top lines
}lspitailorInfo0_t;

typedef struct
{
    uint32_t tailorLeft             : 10;           // 719 cut left lines
    uint32_t tailorRight            : 10;           // 719 cut right lines
}lspitailorInfo_t;

typedef struct
{
    uint32_t rowScaleFrac           : 7;            // 719 row scale frac
    uint32_t                        : 1;
    uint32_t colScaleFrac           : 7;            // 719 col scale frac
}lspiScaleInfo_t;

typedef struct
{
    uint32_t grayCtrl               : 2;            // 719 gray ctrl
    uint32_t quartileSel            : 1;            // 719 quartile from cspi or sw
    uint32_t quartile1              : 8;            // 719 quartile 1
    uint32_t quartile2              : 8;            // 719 quartile 2
    uint32_t quartile3              : 8;            // 719 quartile 3
}lspiQuartileCtrl_t;

typedef struct
{
    uint32_t quartile1InUse         : 8;            // 719 quartile 1 in use
    uint32_t quartile2InUse         : 8;            // 719 quartile 2 in use
    uint32_t quartile3InUse         : 8;            // 719 quartile 3 in use
}lspiQuartileInUse_t;

typedef struct
{
    uint32_t yadjYmin               : 8;            // 719 y adj min
    uint32_t yadjYmax               : 8;            // 719 y adj max
    uint32_t yadjStrech             : 8;            // 719 y adj stretch 
    uint32_t yadjStrechFwl          : 8;            // 719 y adj stretch fwl
}lspiYAdj_t;

typedef struct
{
    uint32_t yadjYminInUse          : 8;            // 719 y adj min
    uint32_t yadjYmaxInUse          : 8;            // 719 y adj max
    uint32_t yadjStrechInUse        : 8;            // 719 y adj stretch 
    uint32_t yadjStrechFwlInUse     : 8;            // 719 y adj stretch fwl
}lspiYAdjInUse_t;


typedef struct
{
    uint32_t pageCmd                : 8;            // 719 page cmd
    uint32_t pageCmd0               : 16;           // 719 page cmd 0
    uint32_t                        : 4;
    uint32_t pageCmd01ByteNum       : 4;            // 719 page cmd0 + page cmd1 byte num 
}lspiGrayPageCmd0_t;

typedef struct
{
    uint32_t pageCmd1               : 32;           // 719 page cmd1
}lspiGrayPageCmd1_t;

typedef struct
{
    uint32_t frameHeightOut         : 11;           // 719 frame height out
    uint32_t rsvd                   : 1;
    uint32_t frameWidthOut          : 11;           // 719 frame width out
}lspiFrameInfoOut_t;

typedef struct
{
    uint32_t mspiEn                 : 1;            // 719 I2S bus enable
    uint32_t mspiAddrLane           : 2;            // 719 Cspi bus enable
    uint32_t mspiDataLane           : 2;            // 719 Lspi bus enable
    uint32_t rsv                    : 3;
    uint32_t mspiInst               : 8;            // 719 Lspi bus enable
    uint32_t mspiVsyncEn            : 1;            // 719 Lspi bus enable
    uint32_t vsyncLineCycle         : 15;           // 719 Lspi bus enable
}lspiMspiCtrl_t;

typedef struct
{
    uint32_t hsyncInst              : 8;            // 719 I2S bus enable
    uint32_t hsyncAddr              : 8;            // 719 Cspi bus enable
    uint32_t vbp                    : 16;           // 719 Lspi bus enable
}lspiVsyncCtrl_t;

typedef struct
{
    uint32_t lspi8080En             : 1;            // 719 I2S bus enable
    uint32_t rsvd                   : 15;
    uint32_t vfp                    : 16;
}lspi8080Ctrl_t;

typedef struct
{
    uint32_t lspiCmd0PreEn          : 1;            // 719 Lspi cmd0 pre en
    uint32_t lspiCmd1PreEn          : 1;            // 719 Lspi cmd1 pre en
    uint32_t lspiCmd0PreParaLen     : 3;            // 719 Lspi cmd0 pre param len
    uint32_t lspiCmd1PreParaLen     : 3;            // 719 Lspi cmd1 pre param len
    uint32_t lspiCmdPreMspiAddrLane : 2;            // 719 Lspi cmd pre mspi addr lane
    uint32_t lspiCmdPreMspiDataLane : 2;            // 719 Lspi cmd pre mspi data lane
}lspiCmdPreParam0_t;

typedef struct
{
    uint32_t lspiCmd0PreInst          : 8;           // 719 Lspi cmd0 pre inst
    uint32_t lspiCmd1PreInst          : 8;           // 719 Lspi cmd1 pre inst
    uint32_t lspiCmd0Pre              : 8;           // 719 Lspi cmd0 pre 
    uint32_t lspiCmd1Pre              : 8;           // 719 Lspi cmd1 pre 
}lspiCmdPreParam1_t;

typedef struct
{
    uint32_t lspiCmd0PrePara          : 32;          // 719 Lspi cmd0 pre para
}lspiCmdPreParam2_t;

typedef struct
{
    uint32_t lspiCmd1PrePara          : 32;          // 719 Lspi cmd1 pre para
}lspiCmdPreParam3_t;


typedef struct
{
    uint32_t lspiCmd0PostEn           : 1;            // 719 Lspi cmd0 pre en
    uint32_t lspiCmd1PostEn           : 1;            // 719 Lspi cmd1 pre en
    uint32_t lspiCmd0PostParaLen      : 3;            // 719 Lspi cmd0 pre param len
    uint32_t lspiCmd1PostParaLen      : 3;            // 719 Lspi cmd1 pre param len
    uint32_t lspiCmdPostMspiAddrLane  : 2;            // 719 Lspi cmd pre mspi addr lane
    uint32_t lspiCmdPostMspiDataLane  : 2;            // 719 Lspi cmd pre mspi data lane
}lspiCmdPostParam0_t;

typedef struct
{
    uint32_t lspiTeEn          		  : 1;            // 719 Lspi te en
    uint32_t lspiTeEdgeSel            : 1;            // 719 Lspi te edge sel: 0->te rise edge; 1->te fall edge
    uint32_t rsvd					  : 2;
    uint32_t lspiTePos0		          : 26;           // 719 Lspi te pos0
}lspiTeParam0_t;

typedef struct
{
    uint32_t lspiTePos1		          : 26;            // 719 Lspi te pos1
}lspiTeParam1_t;


typedef struct
{
    uint32_t lspiCmd0PostInst          : 8;           // 719 Lspi cmd0 post inst
    uint32_t lspiCmd1PostInst          : 8;           // 719 Lspi cmd1 post inst
    uint32_t lspiCmd0Post              : 8;           // 719 Lspi cmd0 post 
    uint32_t lspiCmd1Post              : 8;           // 719 Lspi cmd1 post 
}lspiCmdPostParam1_t;

typedef struct
{
    uint32_t lspiCmd0PostPara          : 32;          // 719 Lspi cmd0 post para
}lspiCmdPostParam2_t;

typedef struct
{
    uint32_t lspiCmd1PostPara          : 32;          // 719 Lspi cmd1 post para
}lspiCmdPostParam3_t;

typedef struct
{
    uint32_t i2sBusEn               : 1;              // 719 I2S bus enable
    uint32_t cspiBusEn              : 1;              // 719 Cspi bus enable
    uint32_t lspiBusEn              : 1;              // 719 Lspi bus enable
}lspiBusSel_t;

extern lspiDataFmt_t lspiDataFmt;
extern lspiDmaCtrl_t lspiDmaCtrl;
extern lspiIntCtrl_t lspiIntCtrl;
extern lspiCtrl_t    lspiCtrl;
extern lspiCmdCtrl_t lspiCmdCtrl;
extern lspiCmdAddr_t lspiCmdAddr;
extern lspiMspiCtrl_t lspiMspiCtrl;
extern lspiVsyncCtrl_t lspiVsyncCtrl;
extern lspi8080Ctrl_t lspi8080Ctrl;
extern lspiInfo_t    lspiInfo;
extern lspiYAdj_t    lspiYAdj;
extern lspiBusSel_t  lspiBusSel;
extern lspitailorInfo0_t  lspiTailorInfo0;
extern lspitailorInfo_t   lspiTailorInfo;
extern lspiScaleInfo_t    lspiScaleInfo;
extern lspiQuartileCtrl_t lspiQuartileCtrl;
extern lspiGrayPageCmd0_t lspiGrayPageCmd0;
extern lspiGrayPageCmd1_t lspiGrayPageCmd1;
extern lspiFrameInfoOut_t lspiFrameInfoOut;
extern lspiTeParam0_t 	  lspiTeParam0;
extern lspiTeParam1_t 	  lspiTeParam1;
extern lspiCmdPreParam0_t lspiPreParam0;
extern lspiCmdPreParam2_t lspiPreParam2;
extern lspiCmdPreParam3_t lspiPreParam3;
extern uint8_t 			  lspiDiv;
#endif




/**
\brief Lspi control bits.
*/
#define LSPI_CTRL_DATA_FORMAT               (1UL << 0)     // LSPI trans abort
#define LSPI_CTRL_BUS_SPEED                 (1UL << 1)     // LSPI bus speed
#define LSPI_CTRL_CTRL                      (1UL << 2)     // LSPI ctrl
#define LSPI_CTRL_CMD_CTRL                  (1UL << 3)     // LSPI cmd ctrl
#define LSPI_CTRL_CMD_ADDR                  (1UL << 4)     // LSPI cmd addr
#define LSPI_CTRL_FRAME_INFO                (1UL << 5)     // LSPI frame info
#define LSPI_CTRL_TAILOR_INFO0              (1UL << 6)     // LSPI tailor info0
#define LSPI_CTRL_TAILOR_INFO               (1UL << 7)     // LSPI tailor info
#define LSPI_CTRL_SCALE_INFO                (1UL << 8)     // LSPI scale info
#define LSPI_CTRL_QUARTILE_CTRL             (1UL << 9)     // LSPI quartile ctrl
#define LSPI_CTRL_YADJ                      (1UL << 10)    // LSPI Y adjustment
#define LSPI_CTRL_GRAY_PAGE_CMD0            (1UL << 11)    // LSPI gray page cmd0
#define LSPI_CTRL_GRAY_PAGE_CMD1            (1UL << 12)    // LSPI gray page cmd1
#define LSPI_CTRL_FRAME_INFO_OUT            (1UL << 13)    // LSPI frame info out
#define LSPI_CTRL_YUV2RGB_INFO0             (1UL << 14)    // LSPI YUV2RGB info0
#define LSPI_CTRL_YUV2RGB_INFO1             (1UL << 15)    // LSPI YUV2RGB info1
#define LSPI_CTRL_BUS_SEL                   (1UL << 16)    // LSPI bus select
#define LSPI_CTRL_DMA_CTRL                  (1UL << 17)    // LSPI DMA control
#define LSPI_CTRL_INT_CTRL                  (1UL << 18)    // LSPI INT control
#define LSPI_MSPI_CTRL                      (1UL << 19)    // LSPI MSPI control
#define LSPI_VSYNC_CTRL                     (1UL << 20)    // LSPI VSYNC control
#define LSPI_8080_CTRL                      (1UL << 21)    // LSPI 8080 control
#define LSPI_PRE_PARA0_CTRL                 (1UL << 22)    // LSPI pre para0 control
#define LSPI_PRE_PARA1_CTRL                 (1UL << 23)    // LSPI pre para1 control
#define LSPI_PRE_PARA2_CTRL                 (1UL << 24)    // LSPI pre para2 control
#define LSPI_PRE_PARA3_CTRL                 (1UL << 25)    // LSPI pre para3 control
#define LSPI_POST_PARA0_CTRL                (1UL << 26)    // LSPI post para0 control
#define LSPI_POST_PARA1_CTRL                (1UL << 27)    // LSPI post para1 control
#define LSPI_POST_PARA2_CTRL                (1UL << 28)    // LSPI post para2 control
#define LSPI_POST_PARA3_CTRL                (1UL << 29)    // LSPI post para3 control
#define LSPI_TE_CTRL                		(1UL << 30)    // LSPI te control



typedef struct 
{
  int32_t              (*init)            ();                                 // Initialize LSPI Interface.
  int32_t              (*deInit)          (void);                             // De-initialize LSPI Interface.
  int32_t              (*powerCtrl)       (lspiPowerState_e state);           // Control LSPI Interface Power.
  void                 (*prepareSend)     (uint8_t data);                     // Fill the dataList which is used to send data.
  int32_t              (*send)            (void *data, uint32_t num);         // Start receiving data from LSPI Interface.
  int32_t              (*ctrl)            (uint32_t control, uint32_t arg);   // Control LSPI Interface.
} const lspiDrvInterface_t;

extern lspiDrvInterface_t lspiDrvInterface2;


#ifdef __cplusplus
}
#endif
#endif


