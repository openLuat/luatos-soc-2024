#ifndef  BSP_LSPI_H
#define  BSP_LSPI_H

#ifdef __cplusplus
extern "C" {
#endif
#include "stdio.h"
#include "string.h"
#include "ec7xx.h"
#include "bsp.h"


#include "lcdDrv.h"


/////////////////////////////////////////////////////////


#define LSPI_RST_LOW    do {GPIO_pinWrite(LSPI_RST_GPIO_INSTANCE, 1 << LSPI_RST_GPIO_PIN, 0);}while(0)
#define LSPI_RST_HIGH   do {GPIO_pinWrite(LSPI_RST_GPIO_INSTANCE, 1 << LSPI_RST_GPIO_PIN, 1 << LSPI_RST_GPIO_PIN);}while(0)
////////////////////////////////////////////////////////



/**
  \addtogroup lspi_interface_gr
  \{
 */

 

typedef void (*lspiCbEvent_fn) (void);            ///< lspi callback event.


/** \brief LSPI DMA */
typedef struct 
{
    DmaInstance_e       txInstance;                 ///< Transmit DMA instance number
    int8_t              txCh;                       ///< Transmit channel number
    uint8_t             txReq;                      ///< Transmit DMA request number
    void                (*txCb)(uint32_t event);    ///< Transmit callback
    DmaDescriptor_t     *descriptor;                ///< Tx descriptor
} lspiDma_t;

// LSPI PINS
typedef const struct
{
    const PIN           *pinDs;                     ///<  Ds Pin identifier
    const PIN           *pinClk;                    ///<  Clk Pin identifier
    const PIN           *pinCs;                     ///<  Cs Pin identifier
    const PIN           *pinMiso;                   ///<  MISO Pin identifier
    const PIN           *pinMosi;                   ////<  MOSI Pin identifier
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
    lspiCbEvent_fn          cbEvent;                   ///< Event callback
    uint32_t                busSpeed;                  ///< LSPI bus speed
    lspiPrepareSendInfo_t   prePareSendInfo;           ///< Prepare Send info
} lspiRteInfo_t;


// SPI Resources definition
typedef struct 
{
    LSPI_TypeDef         *reg;                      ///< SPI register pointer
    lspiPins_t           pins;                      ///< SPI PINS configuration
    lspiDma_t            *dma;                      ///< SPI DMA configuration pointer
    lspiRteInfo_t        *info;                     ///< Run-Time Information
} lspiRes_t;


/**
\brief General power states
*/
typedef enum 
{
    LSPI_POWER_OFF,                                  ///< Power off: no operation possible
    LSPI_POWER_FULL                                  ///< Power on: full operation at maximum performance
} lspiPowerState_e;

typedef struct
{
    uint32_t slaveModeEn            : 1;            ///< Slave Mode Enable
    uint32_t slotSize               : 5;            ///< Slot Size
    uint32_t wordSize               : 5;            ///< Word Size
    uint32_t alignMode              : 1;            ///< Align Mode
    uint32_t endianMode             : 1;            ///< Endian Mode
    uint32_t dataDly                : 2;            ///< Data Delay 
    uint32_t txPad                  : 2;            ///< Tx padding 
    uint32_t rxSignExt              : 1;            ///< Rx Sign Entension
    uint32_t txPack                 : 2;            ///< Tx Pack
    uint32_t rxPack                 : 2;            ///< Rx Pack                -D21
    uint32_t txFifoEndianMode       : 1;            ///< Tx Fifo Endian Mode    -D22
    uint32_t rxFifoEndianMode       : 1;            ///< Rx Fifo Endian Mode    -D24
    uint32_t eorMode                : 1;            ///< send last byte for DMA
}lspiDataFmt_t;

typedef struct
{
    uint32_t rxDmaReqEn             : 1;            ///< Tx Dma Req Enable 
    uint32_t txDmaReqEn             : 1;            ///< Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn         : 1;            ///< Tx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle       : 5;            ///< Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1     : 4;            ///< Tx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1     : 4;            ///< Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold        : 4;            ///< Tx Dma Threadhold
    uint32_t txDmaThreadHold        : 4;            ///< Tx Dma Threadhold
    uint32_t rxFifoFlush            : 1;            ///< Tx Fifo flush
    uint32_t txFifoFlush            : 1;            ///< Tx Fifo flush
}lspiDmaCtrl_t;

typedef struct
{
    uint32_t txUnderRunIntEn        : 1;            ///< Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn          : 1;            ///< Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn             : 1;            ///< Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn        : 1;            ///< Tx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn          : 1;            ///< Tx Dma Err Interrupt Enable
    uint32_t rxDatIntEn             : 1;            ///< Tx Data Interrupt Enable
    uint32_t rxTimeOutIntEn         : 1;            ///< Tx Timeout Interrupt Enable
    uint32_t fsErrIntEn             : 1;            ///< Frame Start Interrupt Enable
    uint32_t frameStartIntEn        : 1;            ///< Frame End Interrupt Enable
    uint32_t frameEndIntEn          : 1;            ///< Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn    : 1;            ///< Not use
    uint32_t lspiRamWrBreakIntEn    : 1;            ///< Lspi ram wr break int enable
    uint32_t lspiRamWrFrameStartEn  : 1;            ///< Lspi ram wr Frame start enable
    uint32_t lspiRamWrFrameEndEn    : 1;            ///< Lspi ram wr Frame end enable
    uint32_t lspiCmdEndEn           : 1;            ///< Lspi sending command end
    uint32_t cspiOtsuEndEn          : 1;            ///< Cspi OTSU one frame end enable
    uint32_t lspiRamWrEndEn         : 1;            ///< Lspi Ram wr end int enable
    uint32_t txIntThreshHold        : 4;            ///< Tx Interrupt Threadhold 
    uint32_t rxIntThreshHold        : 4;            ///< Tx Interrupt Threadhold
}lspiIntCtrl_t;

typedef struct
{
    uint32_t rxTimeOutCycle         : 24;           ///< Tx Timeout Cycle
    uint32_t dummyCycle             : 4;            ///< Dummy cycle
}lspiTimeOutCycle_t;


typedef struct
{
    uint32_t txUnderRun             : 1;            ///< Tx Underrun
    uint32_t txDmaErr               : 1;            ///< Tx Dma Err
    uint32_t txDataRdy              : 1;            ///< Tx Data ready, readOnly
    uint32_t rxOverFlow             : 1;            ///< Tx OverFlow
    uint32_t rxDmaErr               : 1;            ///< Tx Dma Err
    uint32_t rxDataRdy              : 1;            ///< Tx Data ready, readOnly
    uint32_t rxFifoTimeOut          : 1;            ///< Tx Fifo timeout
    uint32_t fsErr                  : 4;            ///< Frame synchronization Err
    uint32_t frameStart             : 1;            ///< Frame start
    uint32_t frameEnd               : 1;            ///< Frame end
    uint32_t txFifoLevel            : 6;            ///< Tx Fifo Level, readOnly
    uint32_t rxFifoLevel            : 6;            ///< Tx Fifo level, readOnly
    uint32_t cspiBusTimeOut         : 1;            ///< Cspi Bus timeout
    uint32_t lspiRamWrBreak         : 1;            ///< Lspi ram wr break
    uint32_t lspiRamWrFrameStart    : 1;            ///< Lspi ram wr frame start
    uint32_t lspiRamWrFrameEnd      : 1;            ///< Lspi ram wr frame end
    uint32_t cspiOtsuEnd            : 1;            ///< Cspi otsu one frame end
    uint32_t lspiRamWrEnd           : 1;            ///< Lspi ram wr end
}lspiStats_t;

typedef struct
{
    uint32_t enable                 : 1;            ///< lspi Enable
    uint32_t data2Lane              : 1;            ///< 2 data lane enable
    uint32_t line4                  : 1;            ///< 0: not use port as DCX; 1: use port as DCX
    uint32_t datSrc                 : 1;            ///< data from camera or memory
    uint32_t colorModeIn            : 2;            ///< Input data color mode
    uint32_t colorModeOut           : 3;            ///< Output data color mode
    uint32_t yAdjEn                 : 1;            ///< Y adjust enable
    uint32_t yAdjSel                : 1;            ///< Y adjustment from cspi or sw
    uint32_t yAdjBound              : 17;           ///< Y adjustment bound
    uint32_t dcDummy                : 1;            ///< Send DCX or dummy
    uint32_t busType                : 1;            ///< 0: Interface I; 1: Interface II
}lspiCtrl_t;

typedef struct
{
    uint32_t wrRdn                  : 1;            ///< 0:rd; 1:wr
    uint32_t ramWr                  : 1;            ///< start to fill frame memory
    uint32_t rdatDummyCycle         : 6;            ///< Dummy cycle before data read
    uint32_t dataLen                : 18;           ///< data len for wr/rd
    uint32_t init                   : 1;            ///< 0:lspi normal; 1:lspi initial
}lspiCmdCtrl_t;

typedef struct
{
    uint32_t addr                   : 8;            ///< command addr
}lspiCmdAddr_t;


typedef struct
{
    uint32_t idle                   : 1;            ///< finish formar command or not
}lspiCmdStats_t;

typedef struct
{
    uint32_t ramWrLen               : 18;           ///< Len of ramwr
}lspiRamWrLen_t;

typedef struct
{
    uint32_t frameHeight            : 16;           ///< frame height
    uint32_t frameWidth             : 16;           ///< frame weight
}lspiInfo_t;

typedef struct
{
    uint32_t tailorBottom           : 10;           ///< cut bottom lines
    uint32_t tailorTop              : 10;           ///< cut top lines
}lspitailorInfo0_t;

typedef struct
{
    uint32_t tailorLeft             : 10;           ///< cut left lines
    uint32_t tailorRight            : 10;           ///< cut right lines
}lspitailorInfo_t;

typedef struct
{
    uint32_t rowScaleFrac           : 7;            ///< row scale frac
    uint32_t                        : 1;
    uint32_t colScaleFrac           : 7;            ///< col scale frac
}lspiScaleInfo_t;

typedef struct
{
    uint32_t grayCtrl               : 2;            ///< gray ctrl
    uint32_t quartileSel            : 1;            ///< quartile from cspi or sw
    uint32_t quartile1              : 8;            ///< quartile 1
    uint32_t quartile2              : 8;            ///< quartile 2
    uint32_t quartile3              : 8;            ///< quartile 3
}lspiQuartileCtrl_t;

typedef struct
{
    uint32_t quartile1InUse         : 8;            ///< quartile 1 in use
    uint32_t quartile2InUse         : 8;            ///< quartile 2 in use
    uint32_t quartile3InUse         : 8;            ///< quartile 3 in use
}lspiQuartileInUse_t;

typedef struct
{
    uint32_t yadjYmin               : 8;            ///< y adj min
    uint32_t yadjYmax               : 8;            ///< y adj max
    uint32_t yadjStrech             : 8;            ///< y adj stretch 
    uint32_t yadjStrechFwl          : 8;            ///< y adj stretch fwl
}lspiYAdj_t;

typedef struct
{
    uint32_t yadjYminInUse          : 8;            ///< y adj min
    uint32_t yadjYmaxInUse          : 8;            ///< y adj max
    uint32_t yadjStrechInUse        : 8;            ///< y adj stretch 
    uint32_t yadjStrechFwlInUse     : 8;            ///< y adj stretch fwl
}lspiYAdjInUse_t;


typedef struct
{
    uint32_t pageCmd                : 8;            ///< page cmd
    uint32_t pageCmd0               : 16;           ///< page cmd 0
    uint32_t                        : 4;
    uint32_t pageCmd01ByteNum       : 4;            ///< page cmd0 + page cmd1 byte num 
}lspiGrayPageCmd0_t;

typedef struct
{
    uint32_t pageCmd1               : 32;           ///< page cmd1
}lspiGrayPageCmd1_t;

typedef struct
{
    uint32_t frameHeightOut         : 10;           ///< frame height out
    uint32_t frameWidthOut          : 10;           ///< frame width out
}lspiFrameInfoOut_t;

typedef struct
{
    uint32_t i2sBusEn               : 1;            ///< I2S bus enable
    uint32_t cspiBusEn              : 1;            ///< Cspi bus enable
    uint32_t lspiBusEn              : 1;            ///< Lspi bus enable
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


/**
\brief Lspi control bits.
*/
#define LSPI_CTRL_DATA_FORMAT               (1UL << 0)     ///< LSPI trans abort
#define LSPI_CTRL_BUS_SPEED                 (1UL << 1)     ///< LSPI bus speed
#define LSPI_CTRL_CTRL                      (1UL << 2)     ///< LSPI ctrl
#define LSPI_CTRL_CMD_CTRL                  (1UL << 3)     ///< LSPI cmd ctrl
#define LSPI_CTRL_CMD_ADDR                  (1UL << 4)     ///< LSPI cmd addr
#define LSPI_CTRL_FRAME_INFO                (1UL << 5)     ///< LSPI frame info
#define LSPI_CTRL_TAILOR_INFO0              (1UL << 6)     ///< LSPI tailor info0
#define LSPI_CTRL_TAILOR_INFO               (1UL << 7)     ///< LSPI tailor info
#define LSPI_CTRL_SCALE_INFO                (1UL << 8)     ///< LSPI scale info
#define LSPI_CTRL_QUARTILE_CTRL             (1UL << 9)     ///< LSPI quartile ctrl
#define LSPI_CTRL_YADJ                      (1UL << 10)    ///< LSPI Y adjustment
#define LSPI_CTRL_GRAY_PAGE_CMD0            (1UL << 11)    ///< LSPI gray page cmd0
#define LSPI_CTRL_GRAY_PAGE_CMD1            (1UL << 12)    ///< LSPI gray page cmd1
#define LSPI_CTRL_FRAME_INFO_OUT            (1UL << 13)    ///< LSPI frame info out
#define LSPI_CTRL_YUV2RGB_INFO0             (1UL << 14)    ///< LSPI YUV2RGB info0
#define LSPI_CTRL_YUV2RGB_INFO1             (1UL << 15)    ///< LSPI YUV2RGB info1
#define LSPI_CTRL_BUS_SEL                   (1UL << 16)    ///< LSPI bus select
#define LSPI_CTRL_DMA_CTRL                  (1UL << 17)    ///< LSPI DMA control
#define LSPI_CTRL_INT_CTRL                  (1UL << 18)    ///< LSPI INT control




/**
\brief Access structure of the LSPI Driver.
*/
typedef struct 
{
  int32_t              (*init)            (lspiCbEvent_fn cb_event);          ///< Initialize LSPI Interface.
  int32_t              (*deInit)          (void);                             ///< De-initialize LSPI Interface.
  int32_t              (*powerCtrl)       (lspiPowerState_e state);           ///< Control LSPI Interface Power.
  void                 (*prepareSend)     (uint8_t data);                     ///< Fill the dataList which is used to send data.
  int32_t              (*send)            (void *data, uint32_t num);         ///< Start receiving data from LSPI Interface.
  int32_t              (*ctrl)            (uint32_t control, uint32_t arg);   ///< Control LSPI Interface.
} const lspiDrvInterface_t;

/** \} */


#ifdef __cplusplus
}
#endif
#endif


