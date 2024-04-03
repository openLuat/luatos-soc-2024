/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename: i2s.h
*
*  Description:
*
*  History: Rev1.0   2020-02-24
*
*  Notes: i2s driver
*
******************************************************************************/

#ifndef  BSP_I2S_H
#define  BSP_I2S_H

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#ifdef __cplusplus
extern "C" {
#endif
#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "ec7xx.h"
#include "bsp.h"

/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/  
// Sample rate that 618 supports
#define SAMPLE_RATE_8K                      0x0
#define SAMPLE_RATE_16K                     0x1
#define SAMPLE_RATE_22_05K                  0x2
#define SAMPLE_RATE_24K		                0x3
#define SAMPLE_RATE_32K                     0x4
#define SAMPLE_RATE_44_1K                   0x5
#define SAMPLE_RATE_48K                     0x6
#define SAMPLE_RATE_96K                     0x7

// I2S DMA chain num
#define I2S_DMA_TX_DESCRIPTOR_CHAIN_NUM     20
#define I2S_DMA_RX_DESCRIPTOR_CHAIN_NUM     20

// I2s Event
#define ARM_I2S_EVENT_TRANSFER_COMPLETE     (1UL << 0)
#define ARM_I2S_EVENT_DATA_LOST             (1UL << 1)
#define ARM_I2S_EVENT_MODE_FAULT            (1UL << 2)

// DMA I2S Request ID
#define RTE_I2S0_DMA_TX_REQID               DMA_REQUEST_USP0_TX
#define RTE_I2S0_DMA_RX_REQID               DMA_REQUEST_USP0_RX
#define RTE_I2S1_DMA_TX_REQID               DMA_REQUEST_USP1_TX
#define RTE_I2S1_DMA_RX_REQID               DMA_REQUEST_USP1_RX
#define RTE_I2S2_DMA_TX_REQID               DMA_REQUEST_USP2_TX
#define RTE_I2S2_DMA_RX_REQID               DMA_REQUEST_USP2_RX

// I2S control bits
#define I2S_CTRL_TRANSABORT                 (1UL << 0)     // I2S trans abort
#define I2S_CTRL_SAMPLE_RATE_SLAVE          (1UL << 1)     // I2S sample rate setting, used in ec618 slave mode
#define I2S_CTRL_SAMPLE_RATE_MASTER         (1UL << 2)     // I2S sample rate setting, used in ec618 master mode
#define I2S_CTRL_SET_TOTAL_NUM              (1UL << 3)     // Audio source total num
#define I2S_CTRL_DATA_FORMAT                (1UL << 4)     // I2S data format
#define I2S_CTRL_SLOT_CTRL                  (1UL << 5)     // I2S slot ctrl
#define I2S_CTRL_BCLK_FS_CTRL               (1UL << 6)     // I2S bclk fs ctrl
#define I2S_CTRL_DMA_CTRL                   (1UL << 7)     // I2S dma ctrl
#define I2S_CTRL_INT_CTRL                   (1UL << 8)     // I2S int ctrl
#define I2S_CTRL_TX_DESCRIPTOR              (1UL << 9)     // I2S tx dma descriptor num set
#define I2S_CTRL_RX_DESCRIPTOR              (1UL << 10)    // I2S rx dma descriptor num set
#define I2S_CTRL_I2SCTL                     (1UL << 11)    // I2S control
#define I2S_CTRL_AUTO_CG_CTRL               (1UL << 12)    // I2S auto cg ctrl
#define I2S_CTRL_INIT                       (1UL << 13)    // I2S init
#define I2S_CTRL_START_STOP                 (1UL << 14)    // I2S audio play start/stop ctrl 
#define I2S_CTRL_DMA_TRUNK_NUM              (1UL << 15)    // I2S dma trunk num for every transfer


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

typedef void (*i2sCbEvent_fn) (uint32_t event, uint32_t arg);     // i2s callback event.
#if (VOLTE_EC_OWN_BOARD_SUPPORT)
typedef void (*i2sSlp1Cb_fn)();
#endif

// I2S IRQ
typedef struct 
{
  IRQn_Type                 irqNum;                             // I2S IRQ Number
  IRQ_Callback_t            cbIrq;
} I2sIrq_t;

// I2S DMA
typedef struct 
{
    DmaInstance_e           txInstance;                  // Transmit DMA instance number
    int8_t                  txCh;                        // Transmit channel number
    uint8_t                 txReq;                       // Transmit DMA request number
    void                    (*txCb)(uint32_t event);     // Transmit callback
    DmaDescriptor_t         *txDescriptor;               // Tx descriptor
    DmaInstance_e           rxInstance;                  // Receive DMA instance number
    int8_t                  rxCh;                        // Receive channel number
    uint8_t                 rxReq;                       // Receive DMA request number
    void                    (*rxCb)(uint32_t event);     // Receive callback
    DmaDescriptor_t         *rxDescriptor;               // Rx descriptor
} I2sDma_t;


// I2S PINS
typedef const struct
{
    PIN                     *mclk;                       //  main clk Pin identifier
    PIN                     *bclk;                       //  pixel clk Pin identifier
    PIN                     *lrck;                       //  cs Pin identifier
    PIN                     *din;                        //  din Pin identifier
    PIN                     *dout;                       //  dout Pin identifier
} I2sPins_t;


typedef struct 
{
    uint8_t                 busy;                        // Transmitter/Receiver busy flag
    uint8_t                 dataLost;                    // Data lost: Receive overflow / Transmit underflow (cleared on start of transfer operation)
    uint8_t                 modeFault;                   // Mode fault detected; optional (cleared on start of transfer operation)
} I2sStatus_t;

// I2S information (Run-time)
typedef struct 
{
    i2sCbEvent_fn           txCbEvent;                  // tx event callback
    i2sCbEvent_fn           rxCbEvent;                  // rx event callback
    uint32_t                mode;                       // I2S mode
    uint32_t                busSpeed;                   // I2S bus speed
    uint16_t                chainCnt;
    uint32_t                totalNum;                   // Total length of audio source
    uint32_t                trunkNum;                   // Trunk lengthe for every dma transfer
    uint32_t                bps;
    uint32_t                txDmaDescNum;
    uint32_t                rxDmaDescNum;
    bool                    isEnableFracdiv;
} I2sInfo_t;


// I2S Resources definition
typedef struct 
{
    I2S_TypeDef             *reg;                       // I2S register pointer
    I2sPins_t               pins;                       // I2S PINS configuration
    I2sDma_t                *dma;                       // I2S DMA configuration pointer
    I2sInfo_t               *info;                      // Run-Time Information
} I2sResources_t;

typedef enum
{
    I2S_SLAVE_MODE          = 0,        ///< I2S is slave 
    I2S_MASTER_MODE         = 1,        ///< I2S is master
}I2sRole_e;

typedef enum
{
    MSB_MODE                = 0,        ///< Left aligned mode
    LSB_MODE                = 1,        ///< Right aligned mode
    I2S_MODE                = 2,        ///< I2S mode
    PCM_MODE                = 3,        ///< PCM mode
}I2sMode_e;

typedef enum
{
    STOP_I2S                = 0,
    ONLY_SEND               = 1,
    ONLY_RECV               = 2,
    SEND_RECV               = 3,
}I2sCtrlMode_e;


// General power states
typedef enum 
{
    I2S_POWER_OFF,                                      // Power off: no operation possible
    I2S_POWER_FULL                                      // Power on: full operation at maximum performance
} I2sPowerState_e;


// Access structure of the I2S Driver.
typedef struct 
{
    int32_t  (*init)       (i2sCbEvent_fn txCbEvent, i2sCbEvent_fn rxCbEvent);             // Initialize I2S Interface.
    int32_t  (*deInit)     (void);                                                         // De-initialize I2S Interface.
    int32_t  (*powerCtrl)  (I2sPowerState_e state);                                        // Control I2S Interface Power.
    int32_t  (*send)       (bool needStop, bool needIrq, bool needChain, void *data, uint32_t chunkNum);   // Start sending data from I2S Interface.
    int32_t  (*recv)       (bool needStop, bool needIrq, bool needChain, void *data, uint32_t chunkNum);   // Start receiving data from I2S Interface.
    int32_t  (*ctrl)       (uint32_t control, uint32_t arg);                               // Control I2S Interface.
    uint32_t (*getTotalCnt)(void);
    uint32_t (*getTrunkCnt)(void);
    uint32_t (*getCtrlReg) (void);
} const I2sDrvInterface_t;

typedef struct
{
    uint32_t slaveModeEn            : 1;    // Slave Mode Enable
    uint32_t slotSize               : 5;    // Slot Size
    uint32_t wordSize               : 5;    // Word Size
    uint32_t alignMode              : 1;    // Align Mode
    uint32_t endianMode             : 1;    // Endian Mode
    uint32_t dataDly                : 2;    // Data Delay
    uint32_t txPad                  : 2;    // Tx Padding
    uint32_t rxSignExt              : 1;    // Rx Sign extention
    uint32_t txPack                 : 2;    // Tx Pack
    uint32_t rxPack                 : 2;    // Rx Pack
    uint32_t txFifoEndianMode       : 1;    // Tx Fifo Endian Mode
    uint32_t rxFifoEndianMode       : 1;    // Rx Fifo Endian Mode
}I2sDataFmt_t;

// I2S Slot Control
typedef struct
{
    uint32_t slotEn                 : 8;    // Slot Enable
    uint32_t slotNum                : 3;    // Slot number per frame synchronization
}I2sSlotCtrl_t;

// I2S Bclk Frame Synchronization Control
typedef struct
{
    uint32_t bclkPolarity           : 1;    // Bclk Polarity
    uint32_t fsPolarity             : 1;    // Frame Synchronization Polarity
    uint32_t fsWidth                : 6;    // Frame Synchronization width
}I2sBclkFsCtrl_t;

// I2S DMA Control
typedef struct
{
    uint32_t rxDmaReqEn             : 1;    // Rx Dma Req Enable
    uint32_t txDmaReqEn             : 1;    // Tx Dma Req Enable
    uint32_t rxDmaTimeOutEn         : 1;    // Rx Dma Timeout Enable
    uint32_t dmaWorkWaitCycle       : 5;    // Dma Work Wait Cycle
    uint32_t rxDmaBurstSizeSub1     : 4;    // Rx Dma Burst Size subtract 1
    uint32_t txDmaBurstSizeSub1     : 4;    // Tx Dma Burst Size subtract 1
    uint32_t rxDmaThreadHold        : 4;    // Rx Dma Threadhold
    uint32_t txDmaThreadHold        : 4;    // Tx Dma Threadhold
    uint32_t rxFifoFlush            : 1;    // Rx Fifo flush
    uint32_t txFifoFlush            : 1;    // Tx Fifo flush
}I2sDmaCtrl_t;

// I2S Interrupt Control
typedef struct
{
    uint32_t txUnderRunIntEn        : 1;    // Tx Underrun interrupt Enable
    uint32_t txDmaErrIntEn          : 1;    // Tx Dma Err Interrupt Enable
    uint32_t txDatIntEn             : 1;    // Tx Data Interrupt Enable
    uint32_t rxOverFlowIntEn        : 1;    // Rx Overflow Interrupt Enable
    uint32_t rxDmaErrIntEn          : 1;    // Rx Dma Err Interrupt Enable
    uint32_t rxDatIntEn             : 1;    // Rx Data Interrupt Enable
    uint32_t rxTimeOutIntEn         : 1;    // Rx Timeout Interrupt Enable
    uint32_t fsErrIntEn             : 1;    // Frame Start Interrupt Enable
    uint32_t frameStartIntEn        : 1;    // Frame End Interrupt Enable
    uint32_t frameEndIntEn          : 1;    // Frame End Interrupt Enable
    uint32_t cspiBusTimeOutIntEn    : 1;    // Not use
    uint32_t txIntThreshHold        : 4;    // Tx Interrupt Threadhold 
    uint32_t rxIntThreshHold        : 4;    // Rx Interrupt Threadhold
}I2sIntCtrl_t;

// I2S Timeout Cycle
typedef struct
{
    uint32_t rxTimeOutCycle         : 24;   // Rx Timeout cycle
}I2sTimeOutCycle_t;

// I2S Status
typedef struct
{   
    uint32_t txUnderRun             : 1;    // Tx Underrun
    uint32_t txDmaErr               : 1;    // Tx Dma Err
    uint32_t txDataRdy              : 1;    // Tx Data ready, readOnly
    uint32_t rxOverFlow             : 1;    // Rx OverFlow
    uint32_t rxDmaErr               : 1;    // Rx Dma Err
    uint32_t rxDataRdy              : 1;    // Rx Data ready, readOnly
    uint32_t rxFifoTimeOut          : 1;    // Rx Fifo timeout
    uint32_t fsErr                  : 4;    // Frame synchronization Err
    uint32_t frameStart             : 1;    // Frame start
    uint32_t frameEnd               : 1;    // Frame end
    uint32_t txFifoLevel            : 6;    // Tx Fifo Level, readOnly
    uint32_t rxFifoLevel            : 6;    // Rx Fifo level, readOnly
    uint32_t cspiBusTimeOut         : 1;    // Not use
}I2sStats_t;

// I2S Control
typedef struct
{
    uint32_t i2sMode                : 2;    // I2S Mode
}I2sCtrl_t;

// I2S Auto Configure Control
typedef struct
{
    uint32_t autoCgEn               : 1;    // Auto Configure Enable
}I2sAutoCgCtrl_t;


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/



#ifdef __cplusplus
}
#endif

#endif /* BSP_I2S_H */
