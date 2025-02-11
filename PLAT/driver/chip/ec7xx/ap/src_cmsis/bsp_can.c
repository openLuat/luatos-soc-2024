/*
 * Copyright (c) 2015-2020 Arm Limited. All rights reserved.
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 (the License); you may
 * not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an AS IS BASIS, WITHOUT
 * WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "bsp_can.h"
#include "ec7xx.h"
#include "slpman.h"

#ifdef PM_FEATURE_ENABLE
#include DEBUG_LOG_HEADER_FILE
#endif

#define BSP_CAN_TEXT_SECTION     SECTION_DEF_IMPL(.sect_bsp_can_text)
#define BSP_CAN_RODATA_SECTION   SECTION_DEF_IMPL(.sect_bsp_can_rodata)
#define BSP_CAN_DATA_SECTION     SECTION_DEF_IMPL(.sect_bsp_can_data)
#define BSP_CAN_BSS_SECTION      SECTION_DEF_IMPL(.sect_bsp_can_bss)

#define CAN_RX_OBJ_NUM           (1)       // Number of receive objects
#define CAN_TX_OBJ_NUM           (1)       // Number of transmit objects
#define CAN_TOT_OBJ_NUM          (CAN_RX_OBJ_NUM + CAN_TX_OBJ_NUM)

#define CAN_TX_OBJ_IDX_START     (CAN_RX_OBJ_NUM)

#ifndef CAN_CLOCK_TOLERANCE
#define CAN_CLOCK_TOLERANCE      (15U)   // 15 / 1000 = 1.5%
#endif


#define CAN_ABORT_SYNC_FLAG      BIT(0)

#define CAN_MAX_IRQ_ROUNDS       (2)

#define CAN_FRAME_INFO_FF        (0x80)    // Frame format(standard or extend)
#define CAN_FRAME_INFO_RTR       (0x40)    // RTR flag
#define CAN_FRAME_INFO_DLC_MSK   (0xF)     // DLC

#define CAN_INT_EN_MASK          (CAN_IER_BUS_ERR_Msk | CAN_IER_ARB_LOST_Msk | \
                                  CAN_IER_ERR_PASSIVE_Msk | CAN_IER_OVERRUN_Msk | \
                                  CAN_IER_ERR_WARNING_Msk | CAN_IER_TX_EMPTY_Msk | \
                                  CAN_IER_RX_REQ_Msk)
/* error code captured */
#define CAN_ECC_BIT              0
#define CAN_ECC_FORM             1
#define CAN_ECC_STUFF            2
#define CAN_ECC_OTHER            3

/* error location */
#define CAN_ECC_LOC_SOF          0x03  /* start of frame */
#define CAN_ECC_LOC_ID28_21      0x02  /* extended: ID bits 28 - 21, standard:  10 - 3 */
#define CAN_ECC_LOC_ID20_18      0x06  /* extended: ID bits 20 - 18, standard:  2 - 0 */
#define CAN_ECC_LOC_SRTR         0x04  /* extended: substitute RTR, standard: RTR */
#define CAN_ECC_LOC_IDE          0x05  /* identifier extension */

#define CAN_ECC_LOC_ID17_13      0x07  /* extended: ID bits 17 - 13 */
#define CAN_ECC_LOC_ID12_5       0x0F  /* extended: ID bits 12 - 5 */
#define CAN_ECC_LOC_ID4_0        0x0E  /* extended: ID bits 4 - 0 */
#define CAN_ECC_LOC_RTR          0x0C  /* RTR */
#define CAN_ECC_LOC_RES1         0x0D  /* reserved bit 1 */

#define CAN_ECC_LOC_RES0         0x09  /* reserved bit 0 */
#define CAN_ECC_LOC_DLC          0x0B  /* data length code */
#define CAN_ECC_LOC_DATA         0x0A  /* data section */
#define CAN_ECC_LOC_CRC_SEQ      0x08  /* CRC sequence */
#define CAN_ECC_LOC_CRC_DEL      0x18  /* CRC delimiter */

#define CAN_ECC_LOC_ACK          0x19  /* ACK slot */
#define CAN_ECC_LOC_ACK_DEL      0x1B  /* ACK delimiter */
#define CAN_ECC_LOC_EOF          0x1A  /* end of frame */
#define CAN_ECC_LOC_INTERM       0x12  /* intermission */
#define CAN_ECC_LOC_UNSPEC       0x00  /* unspecified */


const char * eccTypeToStr[] = {
                                [CAN_ECC_BIT] = "BIT",
                                [CAN_ECC_FORM] = "FORM",
                                [CAN_ECC_STUFF] = "STUFF",
                                [CAN_ECC_OTHER] = "OTHER",

                                     };

const char * eccLocToStr[] = {
                                [CAN_ECC_LOC_SOF] = "SOF",
                                [CAN_ECC_LOC_ID28_21] = "ID28_21",
                                [CAN_ECC_LOC_ID20_18] = "ID20_18",
                                [CAN_ECC_LOC_SRTR] = "SRTR",
                                [CAN_ECC_LOC_IDE] = "IDE",

                                [CAN_ECC_LOC_ID17_13] = "ID17_13",
                                [CAN_ECC_LOC_ID12_5] = "ID12_5",
                                [CAN_ECC_LOC_ID4_0] = "ID4_0",
                                [CAN_ECC_LOC_RTR] = "RTR",
                                [CAN_ECC_LOC_RES1] = "RES1",

                                [CAN_ECC_LOC_RES0] = "RES0",
                                [CAN_ECC_LOC_DLC] = "DLC",
                                [CAN_ECC_LOC_DATA] = "DATA",
                                [CAN_ECC_LOC_CRC_SEQ] = "CRC_SEQ",
                                [CAN_ECC_LOC_CRC_DEL] = "CRC_DEL",

                                [CAN_ECC_LOC_ACK] = "ACK SLOT",
                                [CAN_ECC_LOC_ACK_DEL] = "ACK_DEL",
                                [CAN_ECC_LOC_EOF] = "EOF",
                                [CAN_ECC_LOC_INTERM] = "INTERM",
                                    };
#ifdef PM_FEATURE_ENABLE
#define CAN_DEBUG  0
#endif

#if CAN_DEBUG
#include <stdio.h>
#define CANDEBUG(...)     printf(__VA_ARGS__)
#else
#define CANDEBUG(...)
#endif

typedef struct
{
  volatile uint8_t hasData;
  uint8_t buffer[13];
} CAN_RxBuffer_t; // Used to prefetch rx message from HW fifo so that we are able to mask rx int temporarily.


#ifdef PM_FEATURE_ENABLE

typedef struct
{
    uint8_t  MODER;                          /**< Mode Register */
    uint8_t  IER;                            /**< Interrupt Eable Register */
    uint8_t  BT0R;                           /**< BUS Timing 0 Register */
    uint8_t  BT1R;                           /**< BUS Timing 1 Register */
    uint8_t  EWLR;                           /**< Error Warning Limit Register */
    uint8_t  AC0R;                           /**< Acceptance Code 0 Register */
    uint8_t  AC1R;                           /**< Acceptance Code 1 Register */
    uint8_t  AC2R;                           /**< Acceptance Code 2 Register */
    uint8_t  AC3R;                           /**< Acceptance Code 3 Register */
    uint8_t  AM0R;                           /**< Acceptance Mask 0 Register */
    uint8_t  AM1R;                           /**< Acceptance Mask 1 Register */
    uint8_t  AM2R;                           /**< Acceptance Mask 2 Register */
    uint8_t  AM3R;                           /**< Acceptance Mask 3 Register */
    uint8_t  STBR;                           /**< Standby Register */
} can_backup_register_t;

/** \brief Internal used data structure */
typedef struct _can_database
{
    bool                            isInited;            /**< Whether can has been initialized */
    can_backup_register_t           backup_registers;    /**< Backup registers for low power restore */
} can_database_t;
#endif

#define ARM_CAN_DRV_VERSION ARM_DRIVER_VERSION_MAJOR_MINOR(1,0) // CAN driver version


static const ClockId_e g_canClocks[] = CAN_CLOCK_VECTOR;
static const ClockResetVector_t g_canResetVectors[] = CAN_RESET_VECTORS;

// Driver Version
static const ARM_DRIVER_VERSION can_driver_version = { ARM_CAN_API_VERSION, ARM_CAN_DRV_VERSION };

// Driver Capabilities
static const ARM_CAN_CAPABILITIES can_driver_capabilities = {
  CAN_TOT_OBJ_NUM,  // Number of CAN Objects available
  0U,   // Does not support reentrant calls to ARM_CAN_MessageSend, ARM_CAN_MessageRead, ARM_CAN_ObjectConfigure and abort message sending used by ARM_CAN_Control.
  0U,   // Does not support CAN with Flexible Data-rate mode (CAN_FD)
  0U,   // Does not support restricted operation mode
  1U,   // Supports bus monitoring mode
  0U,   // Does not support internal loopback mode
  1U,   // Supports external loopback mode
  0U    // Reserved (must be zero)
};

// Object Capabilities
static const ARM_CAN_OBJ_CAPABILITIES can_object_capabilities_rx = {
  0U,   // Object supports transmission
  1U,   // Object supports reception
  0U,   // Object does not support RTR reception and automatic Data transmission
  0U,   // Object does not support RTR transmission and automatic Data reception
  1U,   // Object allows assignment of multiple filters to it
  0U,   // Object supports exact identifier filtering
  0U,   // Object does not support range identifier filtering
  0U,   // Object supports mask identifier filtering
  3U,   // Object can at least buffer 3 messages, the maximum number is 64 / (3 + DLC)
  0U    // Reserved (must be zero)
};

// Object Capabilities
static const ARM_CAN_OBJ_CAPABILITIES can_object_capabilities_tx = {
  1U,   // Object supports transmission
  0U,   // Object does not support reception
  0U,   // Object does not support RTR reception and automatic Data transmission
  0U,   // Object does not support RTR transmission and automatic Data reception
  0U,   // Object does not allow assignment of multiple filters to it
  0U,   // Object does not support exact identifier filtering
  0U,   // Object does not support range identifier filtering
  0U,   // Object does not support mask identifier filtering
  1U,   // Object can only buffer 1 message
  0U    // Reserved (must be zero)
};

BSP_CAN_BSS_SECTION static uint8_t can_obj_cfg[CAN_TOT_OBJ_NUM];
BSP_CAN_BSS_SECTION static CAN_RxBuffer_t can_rx_buffer;

#if (RTE_CAN0)

BSP_CAN_BSS_SECTION static CAN_CTRL CAN0_Ctrl = { 0 };
static const PIN CAN0_pin_tx  = {RTE_CAN0_TX_BIT,   RTE_CAN0_TX_FUNC};
static const PIN CAN0_pin_rx  = {RTE_CAN0_RX_BIT,   RTE_CAN0_RX_FUNC};
static const PIN CAN0_pin_stb = {RTE_CAN0_STB_BIT,  RTE_CAN0_STB_FUNC};

#if (RTE_CAN0_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif

#if (RTE_CAN0_IO_MODE == IRQ_MODE)
void CAN0_IRQHandler(void);

static const CAN_IRQ CAN0_IRQ = {
                            PXIC4_CAN0_IRQn,
                            CAN0_IRQHandler
                          };

#endif

static CAN_RESOURCES CAN0_Resources =
{
    CAN0,
    {
      &CAN0_pin_tx,
      &CAN0_pin_rx,
      &CAN0_pin_stb
    },

#if (RTE_CAN0_IO_MODE == IRQ_MODE)
    &CAN0_IRQ,
#else
    NULL,
#endif

    &CAN0_Ctrl
};
#endif


//
//   Functions
//

#ifdef PM_FEATURE_ENABLE

static CAN_TypeDef* const g_canBases[] = CAN_INSTANCE_ARRAY;

BSP_CAN_BSS_SECTION static can_database_t g_canDataBase[CAN_INSTANCE_NUM] = {0};

/**
  \brief can initialization counter, for lower power callback register/de-register
 */
BSP_CAN_BSS_SECTION static uint32_t g_canInitCounter = 0;

/**
  \brief Bitmap of CAN working status, when all CAN instances are not working, we can vote to enter to low power state.
 */

BSP_CAN_BSS_SECTION static uint32_t g_canWorkingStatus = 0;

//BSP_CAN_DATA_SECTION volatile static uint32_t g_canDebug = 1;

#define  LOCK_SLEEP(instance)     do                                                                     \
                                          {                                                              \
                                              g_canWorkingStatus |= (1U << instance);                    \
                                              slpManDrvVoteSleep(SLP_VOTE_CAN, SLP_ACTIVE_STATE);        \
                                          }                                                              \
                                          while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)      do                                                                     \
                                                      {                                                              \
                                                          g_canWorkingStatus &= ~(1U << instance);                   \
                                                          if(g_canWorkingStatus == 0)                                \
                                                              slpManDrvVoteSleep(SLP_VOTE_CAN, SLP_SLP1_STATE); \
                                                      }                                                              \
                                                      while(0)


/**
  \fn        static void CAN_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, CAN hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void CAN_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:
            for(i = 0; i < CAN_INSTANCE_NUM; i++)
            {
                if(g_canDataBase[i].isInited == true)
                {
                    g_canDataBase[i].backup_registers.MODER = g_canBases[i]->MODER;
                    g_canDataBase[i].backup_registers.IER = g_canBases[i]->IER;
                    g_canDataBase[i].backup_registers.BT0R = g_canBases[i]->BT0R;
                    g_canDataBase[i].backup_registers.BT1R = g_canBases[i]->BT1R;
                    g_canDataBase[i].backup_registers.EWLR = g_canBases[i]->EWLR;
                    g_canDataBase[i].backup_registers.STBR = g_canBases[i]->STBR;

                    // filter specific registers are backed up in CAN_ObjectSetFilter() for those registers can only be accessed in reset mode.

                    // Need to hold STANDBY pin
                    slpManAONIOLatchEn(AonIOLatch_Enable);
                }
            }
            break;
        case SLPMAN_IDLE_STATE:         // add doze process

            break;
        default:
            break;
    }

}

/**
  \fn        static void CAN_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, CAN hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state

 */
static void CAN_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            // no need to restore if failing to sleep
            if(apmuGetSleepedFlag() == false)
            {
                break;
            }

            for(i = 0; i < CAN_INSTANCE_NUM; i++)
            {
                if(g_canDataBase[i].isInited == true)
                {
                    GPR_clockEnable(g_canClocks[2*i]);
                    GPR_clockEnable(g_canClocks[2*i+1]);

                    g_canBases[i]->MODER = g_canDataBase[i].backup_registers.MODER | CAN_MODER_RESET_MODE_Msk;
                    g_canBases[i]->EMR = CAN_EMR_EXT_MODE_Msk;

                    g_canBases[i]->IER = g_canDataBase[i].backup_registers.IER;
                    g_canBases[i]->BT0R = g_canDataBase[i].backup_registers.BT0R;
                    g_canBases[i]->BT1R = g_canDataBase[i].backup_registers.BT1R;
                    g_canBases[i]->EWLR = g_canDataBase[i].backup_registers.EWLR;
                    g_canBases[i]->STBR = g_canDataBase[i].backup_registers.STBR;

                    g_canBases[i]->AC0R = g_canDataBase[i].backup_registers.AC0R;
                    g_canBases[i]->AC1R = g_canDataBase[i].backup_registers.AC1R;
                    g_canBases[i]->AC2R = g_canDataBase[i].backup_registers.AC2R;
                    g_canBases[i]->AC3R = g_canDataBase[i].backup_registers.AC3R;

                    g_canBases[i]->AM0R = g_canDataBase[i].backup_registers.AM0R;
                    g_canBases[i]->AM1R = g_canDataBase[i].backup_registers.AM1R;
                    g_canBases[i]->AM2R = g_canDataBase[i].backup_registers.AM2R;
                    g_canBases[i]->AM3R = g_canDataBase[i].backup_registers.AM3R;

                    g_canBases[i]->MODER &= ~CAN_MODER_RESET_MODE_Msk;

                }
            }
            break;

        case SLPMAN_IDLE_STATE:         // add doze process

            break;

        default:
            break;
    }

}

#endif


static ARM_DRIVER_VERSION CAN_GetVersion(void)
{
    // Return driver version
    return can_driver_version;
}

static ARM_CAN_CAPABILITIES CAN_GetCapabilities(void)
{
    // Return driver capabilities
    return can_driver_capabilities;
}

/**
  \fn          static uint32_t CAN_GetInstanceNumber(CAN_RESOURCES *can)
  \brief       Get instance number
  \param[in]   can       Pointer to CAN resources
  \returns     instance number
*/
static uint32_t CAN_GetInstanceNumber(CAN_RESOURCES *can)
{
    return ((uint32_t)can->reg - (uint32_t)CAN0) >> 12;
}

static int32_t CAN_Initialize(ARM_CAN_SignalUnitEvent_t   cb_unit_event,
                                   ARM_CAN_SignalObjectEvent_t cb_object_event,
                                   CAN_RESOURCES *can)
{
    PadConfig_t padConfig;

    if (can->ctrl->flags & CAN_FLAG_INIT)
    {
        return ARM_DRIVER_OK;
    }

    // Configure CAN Pins
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = can->pins.pin_tx->funcNum;
    PAD_setPinConfig(can->pins.pin_tx->pinNum, &padConfig);
    padConfig.mux = can->pins.pin_rx->funcNum;
    PAD_setPinConfig(can->pins.pin_rx->pinNum, &padConfig);
    padConfig.mux = can->pins.pin_stb->funcNum;
    PAD_setPinConfig(can->pins.pin_stb->pinNum, &padConfig);

    // Reset Run-Time information structure
    memset(can->ctrl, 0, sizeof(CAN_CTRL));

    can->ctrl->unit_event_cb = cb_unit_event;
    can->ctrl->object_event_cb = cb_object_event;

    can->ctrl->flags |= CAN_FLAG_INIT;

#ifdef PM_FEATURE_ENABLE
    g_canInitCounter++;

    uint32_t instance;

    instance = CAN_GetInstanceNumber(can);

    g_canDataBase[instance].isInited = true;

    if(g_canInitCounter == 1U)
    {
        g_canWorkingStatus = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_CAN_MODULE, CAN_EnterLowPowerStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_CAN_MODULE, CAN_ExitLowPowerStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}

static int32_t CAN_Uninitialize(CAN_RESOURCES *can) {

    PadConfig_t padConfig;
    can->ctrl->flags = 0;
    memset(can->ctrl, 0, sizeof(CAN_CTRL));

    // Unconfigure TX and RX pins
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;

    PAD_setPinConfig(can->pins.pin_tx->pinNum, &padConfig);
    PAD_setPinConfig(can->pins.pin_rx->pinNum, &padConfig);
    PAD_setPinConfig(can->pins.pin_stb->pinNum, &padConfig);

#ifdef PM_FEATURE_ENABLE
    uint32_t instance;

    instance = CAN_GetInstanceNumber(can);

    g_canDataBase[instance].isInited = false;

    g_canInitCounter--;

    if(g_canInitCounter == 0)
    {
      g_canWorkingStatus = 0;
      slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_CAN_MODULE);
      slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_CAN_MODULE);
    }
#endif

  return ARM_DRIVER_OK;
}

static int32_t CAN_PowerControl (ARM_POWER_STATE state, CAN_RESOURCES *can)
{

    uint32_t instance = CAN_GetInstanceNumber(can);

    switch (state)
    {
        case ARM_POWER_OFF:
            can->ctrl->flags &= ~CAN_FLAG_POWER;

            // Add code to disable interrupts and put peripheral into reset mode,
            // and if possible disable clock
            // ..

            if(can->irq)
            {
                XIC_ClearPendingIRQ(can->irq->irq_num);
                XIC_DisableIRQ(can->irq->irq_num);
            }

            GPR_swResetModule(&g_canResetVectors[instance]);

            CLOCK_clockDisable(g_canClocks[instance*2]);
            CLOCK_clockDisable(g_canClocks[instance*2+1]);

            memset(can_obj_cfg, 0, CAN_TOT_OBJ_NUM);

            break;

        case ARM_POWER_FULL:
            if((can->ctrl->flags & CAN_FLAG_INIT) == 0U)
            {
                return ARM_DRIVER_ERROR;
            }
            if(can->ctrl->flags & CAN_FLAG_POWER)
            {
                return ARM_DRIVER_OK;
            }

            // Enable power to can clock
            CLOCK_clockEnable(g_canClocks[instance*2]);
            CLOCK_clockEnable(g_canClocks[instance*2+1]);

            memset(can_obj_cfg, 0, CAN_TOT_OBJ_NUM);

            // Enable I2C irq
            if(can->irq)
            {
                XIC_SetVector(can->irq->irq_num,can->irq->cb_irq);
                XIC_EnableIRQ(can->irq->irq_num);
                XIC_SuppressOvfIRQ(can->irq->irq_num);
            }

            can->ctrl->flags |= CAN_FLAG_POWER;

            break;

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

static uint32_t CAN_GetClock(CAN_RESOURCES *can)
{

    // Add code to return peripheral clock frequency
    // ..
#ifdef FPGA_TEST
    return 50000000UL;
#else
    uint32_t instance = CAN_GetInstanceNumber(can);

    return GPR_getClockFreq(g_canClocks[instance*2+1]);
#endif

}
/*
|----------------   NBT = (1 + TSEG1 + TSEG2)   ---------------|
v                                                              v
+---------+-------------------------------+--------------------+
|         |                               |                    |
| SYN = 1 | TSEG1 = prop_seg + phase_seg1 | TSEG2 = phase_seg2 |
|         |                               |                    |
+---------+-------------------------------+--------------------+
                                          ^
                                          |
                                    Sample point = (1 + TSEG1) / NBT

FCLK = 26MHz

Ref configuration:

  bitrate(KHz)     brp      Tq(us)    prop_seg      phase_seg1     phase_seg2        NBT      Sample point
    1000            2        1/13        5              4              3             13           76.9%
     650            2        1/13        9              6              4             20           80.0%
     500            4        1/6.5       5              4              3             13           76.9%
     250            8        1/3.25      5              4              3             13           76.9%
     200           10        1/2.6       5              4              3             13           76.9%
     125           16        1/1.625     5              4              3             13           76.9%
     100           20        1/1.3       5              4              3             13           76.9%
      50           26        1           9              6              4             20           80.0%
      25           52        2           9              6              4             20           80.0%
*/

static int32_t CAN_SetBitrate(ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments, CAN_RESOURCES *can)
{
    uint32_t sjw, prop_seg, phase_seg1, phase_seg2, fclk, target_fclk, brp, tq_num;

    if ((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if(select != ARM_CAN_BITRATE_NOMINAL)
    {
        return ARM_CAN_INVALID_BITRATE_SELECT;
    }

    if((can->reg->MODER & CAN_MODER_RESET_MODE_Msk) == 0)
    {
        // Must be in reset mode, that's ARM_CAN_MODE_INITIALIZATION
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    prop_seg   = (bit_segments & ARM_CAN_BIT_PROP_SEG_Msk) >> ARM_CAN_BIT_PROP_SEG_Pos;
    phase_seg1 = (bit_segments & ARM_CAN_BIT_PHASE_SEG1_Msk) >> ARM_CAN_BIT_PHASE_SEG1_Pos;
    phase_seg2 = (bit_segments & ARM_CAN_BIT_PHASE_SEG2_Msk) >> ARM_CAN_BIT_PHASE_SEG2_Pos;
    sjw        = (bit_segments & ARM_CAN_BIT_SJW_Msk) >> ARM_CAN_BIT_SJW_Pos;

    if(((prop_seg + phase_seg1) < 1U) || ((prop_seg + phase_seg1) > 16U))
    {
        return ARM_CAN_INVALID_BIT_PROP_SEG;
    }

    if((phase_seg2 < 1U) || (phase_seg2 > 8U))
    {
        return ARM_CAN_INVALID_BIT_PHASE_SEG2;
    }

    if((sjw < 1U) || (sjw > 4U))
    {
        return ARM_CAN_INVALID_BIT_SJW;
    }

    tq_num = 1 + prop_seg + phase_seg1 + phase_seg2;

    fclk = CAN_GetClock(can) / 2;

    brp = fclk / (tq_num * bitrate);

    if(brp > 64U)
    {
        return ARM_CAN_INVALID_BITRATE;
    }

    // Check tolerance, shall be less than 1.5%
    target_fclk = tq_num * bitrate * brp;

    if(fclk > target_fclk)
    {
        if((fclk - target_fclk) > ((fclk / 1000) * CAN_CLOCK_TOLERANCE))
        {
            return ARM_CAN_INVALID_BITRATE;
        }
    }
    else if(fclk < target_fclk)
    {
        if((target_fclk - fclk) > ((fclk / 1000) * CAN_CLOCK_TOLERANCE))
        {
            return ARM_CAN_INVALID_BITRATE;
        }

    }

    can->reg->BT0R = EIGEN_VAL2FLD(CAN_BT0R_BAUD_PRESC, brp - 1) | EIGEN_VAL2FLD(CAN_BT0R_SJW, sjw - 1);
    can->reg->BT1R = EIGEN_VAL2FLD(CAN_BT1R_TSEG1, prop_seg + phase_seg1 - 1) | EIGEN_VAL2FLD(CAN_BT1R_TSEG2, phase_seg2 - 1);

    if(bitrate <= 100000)
    {
        can->reg->BT1R |= CAN_BT1R_SAM_Msk; // Sample 3 times
    }

    return ARM_DRIVER_OK;
}

static int32_t CAN_enterResetMode(CAN_RESOURCES *can)
{
    int32_t i;

    uint8_t status = can->reg->MODER;

    // Disable interrupts
    can->reg->IER = 0;

    for(i = 0; i < 1000; i++)
    {
        if(status & CAN_MODER_RESET_MODE_Msk)
        {
            return ARM_DRIVER_OK;
        }

        can->reg->MODER |= CAN_MODER_RESET_MODE_Msk;

        delay_us(1);
        status = can->reg->MODER;
    }

    return ARM_DRIVER_ERROR_TIMEOUT;
}

static int32_t CAN_exitResetMode(CAN_RESOURCES *can)
{
    int32_t i;

    uint8_t status = can->reg->MODER;

    for(i = 0; i < 1000; i++)
    {
        // Check reset bit
        if((status & CAN_MODER_RESET_MODE_Msk) == 0)
        {
            return ARM_DRIVER_OK;
        }

        can->reg->MODER = status &~CAN_MODER_RESET_MODE_Msk;

        delay_us(1);

        status = can->reg->MODER;
    }

    return ARM_DRIVER_ERROR_TIMEOUT;

}

static void CAN_recoverFromBusOff(CAN_RESOURCES *can)
{

    CAN_TypeDef *reg = can->reg;

    /*
    CAN controller sets reset mode bit to 1 when entering bus off mode,
    SW shall clear it to recover and it will wait 128*11 recessive bit time to become error active state
    */

    if(reg->MODER & CAN_MODER_RESET_MODE_Msk)
    {
        can->reg->MODER &= ~CAN_MODER_RESET_MODE_Msk;

        while(reg->STR & CAN_STR_BUS_Msk)
        {
            delay_us(10);
        }
    }

}

static int32_t CAN_SetMode(ARM_CAN_MODE mode, CAN_RESOURCES *can)
{
    int ret;
    uint32_t unit_event = 0;

    if((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if((mode == ARM_CAN_MODE_RESTRICTED) || (mode == ARM_CAN_MODE_LOOPBACK_INTERNAL))
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    ret = CAN_enterResetMode(can);

    if(ret != ARM_DRIVER_OK)
    {
        return ret;
    }
    can->reg->MODER |= CAN_MODER_FILTER_MODE_Msk; // Use single fitlter in default
    can->reg->EMR = CAN_EMR_EXT_MODE_Msk;
    can->reg->MODER &= ~(CAN_MODER_SELF_TEST_Msk | CAN_MODER_LISTEN_ONLY_Msk);

    switch(mode)
    {
        case ARM_CAN_MODE_INITIALIZATION:
            can->ctrl->state = CAN_STATE_STOPPED;
            unit_event = ARM_CAN_EVENT_UNIT_BUS_OFF;
            break;

        case ARM_CAN_MODE_NORMAL:
            ret = CAN_exitResetMode(can);
            if(ret != ARM_DRIVER_OK)
            {
                return ret;
            }
            can->reg->IER = CAN_INT_EN_MASK;
            can->ctrl->state = CAN_STATE_ERROR_ACTIVE;
            unit_event = ARM_CAN_EVENT_UNIT_ACTIVE;
            break;

        case ARM_CAN_MODE_MONITOR:

            can->reg->MODER |= CAN_MODER_LISTEN_ONLY_Msk;

            ret = CAN_exitResetMode(can);
            if(ret != ARM_DRIVER_OK)
            {
                return ret;
            }

            can->reg->IER = CAN_INT_EN_MASK;
            can->ctrl->state = CAN_STATE_ERROR_PASSIVE;
            unit_event = ARM_CAN_EVENT_UNIT_PASSIVE;
            break;

        case ARM_CAN_MODE_LOOPBACK_EXTERNAL:
            // Enable self test mode
            can->reg->MODER |= CAN_MODER_SELF_TEST_Msk;

            ret = CAN_exitResetMode(can);
            if(ret != ARM_DRIVER_OK)
            {
                return ret;
            }
            can->reg->IER = CAN_INT_EN_MASK;
            can->ctrl->state = CAN_STATE_ERROR_PASSIVE;
            unit_event = ARM_CAN_EVENT_UNIT_PASSIVE;
            break;

        case ARM_CAN_MODE_LOOPBACK_INTERNAL:
        case ARM_CAN_MODE_RESTRICTED:
            break;
    }

    if((can->ctrl->unit_event_cb != NULL) && (unit_event != 0))
    {
        can->ctrl->unit_event_cb(unit_event);
    }

    return ARM_DRIVER_OK;
}

static ARM_CAN_OBJ_CAPABILITIES CAN_ObjectGetCapabilities(uint32_t obj_idx, CAN_RESOURCES *can)
{
    ARM_CAN_OBJ_CAPABILITIES obj_cap_null;

    if(obj_idx >= CAN_TOT_OBJ_NUM)
    {
        memset(&obj_cap_null, 0, sizeof(obj_cap_null));
        return obj_cap_null;
    }

    if(obj_idx >= CAN_TX_OBJ_IDX_START)
    {
        return can_object_capabilities_tx;
    }
    else
    {
        return can_object_capabilities_rx;
    }
}

static int32_t CAN_ObjectSetFilter(uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg, CAN_RESOURCES *can)
{

    if((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if(obj_idx >= CAN_RX_OBJ_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if((can->reg->MODER & CAN_MODER_RESET_MODE_Msk) == 0)
    {
        // Must be in reset mode, that's ARM_CAN_MODE_INITIALIZATION
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    switch(operation)
    {
        case ARM_CAN_FILTER_ID_EXACT_ADD:
            can->reg->MODER |= CAN_MODER_FILTER_MODE_Msk; // Use single fitlter

            if(id & ARM_CAN_ID_IDE_Msk)
            {
                // Extended frame
                id = (id << 3);
                can->reg->AC0R = (id >> 24) & 0xff;
                can->reg->AC1R = (id >> 16) & 0xff;
                can->reg->AC2R = (id >> 8)  & 0xff;
                can->reg->AC3R = (id)       & 0xff;
                can->reg->AM0R = 0;
                can->reg->AM1R = 0;
                can->reg->AM2R = 0;
                can->reg->AM3R = 0x7;
            }
            else
            {
                // Standard frame
                id = (id << 21);
                can->reg->AC0R = (id >> 24) & 0xff;
                can->reg->AC1R = (id >> 5) & 0xff;
                can->reg->AC2R = 0;
                can->reg->AC3R = 0;
                can->reg->AM0R = 0;
                can->reg->AM1R = 0x1F;
                can->reg->AM2R = 0xFF;
                can->reg->AM3R = 0xFF;
            }

            break;
        case ARM_CAN_FILTER_ID_MASKABLE_ADD:
            can->reg->MODER |= CAN_MODER_FILTER_MODE_Msk; // Use single fitlter

            // if mask(arg) bit is 0, the corresponding ID bit will be accepted, regardlesss of the value
            // if mask(arg) bit is 1, the corresponding ID bit will be compared with the value of the ID filter bit
            if(id & ARM_CAN_ID_IDE_Msk)
            {
                // Extended frame
                id = (id << 3);
                arg = ~(arg << 3);

                can->reg->AC0R = (id >> 24) & 0xff;
                can->reg->AC1R = (id >> 16) & 0xff;
                can->reg->AC2R = (id >> 8)  & 0xff;
                can->reg->AC3R = (id)       & 0xff;

                can->reg->AM0R = (arg >> 24) & 0xff;
                can->reg->AM1R = (arg >> 16) & 0xff;
                can->reg->AM2R = (arg >> 8)  & 0xff;
                can->reg->AM3R = (arg)       & 0xff;
            }
            else
            {
                // Standard frame
                id = (id << 21);
                arg = ~(arg << 21);

                can->reg->AC0R = (id >> 24) & 0xff;
                can->reg->AC1R = (id >> 5) & 0xff;
                can->reg->AC2R = 0;
                can->reg->AC3R = 0;

                can->reg->AM0R = (arg >> 24) & 0xff;
                can->reg->AM1R = (arg >> 5) & 0xff;
                can->reg->AM2R = 0xFF;
                can->reg->AM3R = 0xFF;
            }
            break;
        case ARM_CAN_FILTER_ID_RANGE_ADD:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case ARM_CAN_FILTER_ID_EXACT_REMOVE:
        case ARM_CAN_FILTER_ID_MASKABLE_REMOVE:
        case CAN_FILTER_SINGLE_MODE_REMOVE:

            can->reg->MODER |= CAN_MODER_FILTER_MODE_Msk; // Use single fitlter
            can->reg->AM0R = 0xFF;
            can->reg->AM1R = 0xFF;
            can->reg->AM2R = 0xFF;
            can->reg->AM3R = 0xFF;
            break;
        case ARM_CAN_FILTER_ID_RANGE_REMOVE:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case CAN_FILTER_SINGLE_MODE_ADD:
            can->reg->MODER |= CAN_MODER_FILTER_MODE_Msk; // Use single fitlter

            can->reg->AC0R = (id >> 24) & 0xff;
            can->reg->AC1R = (id >> 16) & 0xff;
            can->reg->AC2R = (id >> 8)  & 0xff;
            can->reg->AC3R = (id)       & 0xff;

            can->reg->AM0R = (arg >> 24) & 0xff;
            can->reg->AM1R = (arg >> 16) & 0xff;
            can->reg->AM2R = (arg >> 8)  & 0xff;
            can->reg->AM3R = (arg)       & 0xff;
            break;

        case CAN_FILTER_DUAL_MODE_ADD:
            can->reg->MODER &= ~CAN_MODER_FILTER_MODE_Msk; // Use dual fitlter

            can->reg->AC0R = (id >> 24) & 0xff;
            can->reg->AC1R = (id >> 16) & 0xff;
            can->reg->AC2R = (id >> 8)  & 0xff;
            can->reg->AC3R = (id)       & 0xff;

            can->reg->AM0R = (arg >> 24) & 0xff;
            can->reg->AM1R = (arg >> 16) & 0xff;
            can->reg->AM2R = (arg >> 8)  & 0xff;
            can->reg->AM3R = (arg)       & 0xff;
            break;

        case CAN_FILTER_DUAL_MODE_REMOVE:
            can->reg->MODER &= ~CAN_MODER_FILTER_MODE_Msk; // Use dual fitlter
            can->reg->AM0R = 0xFF;
            can->reg->AM1R = 0xFF;
            can->reg->AM2R = 0xFF;
            can->reg->AM3R = 0xFF;
            break;
    }

#ifdef PM_FEATURE_ENABLE
    // back up those regsiters for sleep1
    uint32_t instance = CAN_GetInstanceNumber(can);

    g_canDataBase[instance].backup_registers.AC0R = g_canBases[instance]->AC0R;
    g_canDataBase[instance].backup_registers.AC1R = g_canBases[instance]->AC1R;
    g_canDataBase[instance].backup_registers.AC2R = g_canBases[instance]->AC2R;
    g_canDataBase[instance].backup_registers.AC3R = g_canBases[instance]->AC3R;

    g_canDataBase[instance].backup_registers.AM0R = g_canBases[instance]->AM0R;
    g_canDataBase[instance].backup_registers.AM1R = g_canBases[instance]->AM1R;
    g_canDataBase[instance].backup_registers.AM2R = g_canBases[instance]->AM2R;
    g_canDataBase[instance].backup_registers.AM3R = g_canBases[instance]->AM3R;
#endif

    return ARM_DRIVER_OK;
}

static int32_t CAN_ObjectConfigure(uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg, CAN_RESOURCES *can)
{

    if((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if(obj_idx >= CAN_TOT_OBJ_NUM)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    switch (obj_cfg)
    {
        case ARM_CAN_OBJ_INACTIVE:
            can_obj_cfg[obj_idx] = ARM_CAN_OBJ_INACTIVE;
            break;

        case ARM_CAN_OBJ_RX_RTR_TX_DATA:
        case ARM_CAN_OBJ_TX_RTR_RX_DATA:
            can_obj_cfg[obj_idx] = ARM_CAN_OBJ_INACTIVE;
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case ARM_CAN_OBJ_TX:
            if(obj_idx < CAN_TX_OBJ_IDX_START)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            can_obj_cfg[obj_idx] = ARM_CAN_OBJ_TX;
            break;
        case ARM_CAN_OBJ_RX:
            if(obj_idx > CAN_RX_OBJ_NUM)
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }
            can_obj_cfg[obj_idx] = ARM_CAN_OBJ_RX;
            break;
        default:
            return ARM_DRIVER_ERROR_PARAMETER;
    }

    return ARM_DRIVER_OK;
}

static int32_t CAN_MessageSend (uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size, CAN_RESOURCES *can)
{
    uint8_t frame_info = 0, *txPtr = (uint8_t *)&(can->reg->TXD0);
    uint8_t cmd = 0;

    if((obj_idx < CAN_TX_OBJ_IDX_START) || (obj_idx >= CAN_TOT_OBJ_NUM))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if(can_obj_cfg[obj_idx] != ARM_CAN_OBJ_TX)
    {
        return ARM_DRIVER_ERROR;
    }


    if((can->reg->STR & CAN_STR_TX_BUF_Msk) == 0)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }



    if(msg_info->rtr)
    {
        size = msg_info->dlc;
    }

    if(size > 8U)
    {
        size = 8U;
    }

    if(msg_info->rtr)
    {
        frame_info = size | CAN_FRAME_INFO_RTR;
    }
    else
    {
        frame_info = size;
    }

    if(msg_info->id & ARM_CAN_ID_IDE_Msk)
    {
        // Extended Identifier
        frame_info |= CAN_FRAME_INFO_FF;
        *txPtr++ = frame_info;
        *txPtr++ = (msg_info->id & 0x1fe00000) >> 21;
        *txPtr++ = (msg_info->id & 0x001fe000) >> 13;
        *txPtr++ = (msg_info->id & 0x00001fe0) >> 5;
        *txPtr++ = (msg_info->id & 0x1f) << 3;
    }
    else
    {
        // Standard Identifier
        *txPtr++ = frame_info;
        *txPtr++ = (msg_info->id & 0x000007f8) >> 3;
        *txPtr++ = (msg_info->id & 0x00000007) << 5;
    }

    for(uint32_t i = 0; i < size; i++)
    {
        *txPtr++ = data[i];
    }

    if(can->ctrl->mode & CAN_MODE_ONESHOT)
    {
        cmd |= CAN_CMDR_TX_ABT_Msk;
    }

    if(can->reg->MODER & CAN_MODER_SELF_TEST_Msk)
    {
        cmd |= CAN_CMDR_SELF_RX_REQ_Msk;
    }
    else
    {
        cmd |= CAN_CMDR_TX_REQ_Msk;
    }
    can->reg->CMDR = cmd;

    return ((int32_t)size);
}

static int32_t CAN_MessageRead(uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size, CAN_RESOURCES *can)
{
    if ((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    // Add code to read previously received message
    // (reception was started when object was configured for reception)
    // ..

    if((can_obj_cfg[obj_idx] == ARM_CAN_OBJ_TX) || (can_obj_cfg[obj_idx] == ARM_CAN_OBJ_RX_RTR_TX_DATA))
    {
        return ARM_DRIVER_ERROR;
    }

    uint32_t dlc, id = 0, temp = 0, mask;
    uint8_t frame_info = 0;
    uint8_t *rxPtr = NULL;

    CAN_TypeDef *reg = can->reg;

    // Must be in critical section
    mask = SaveAndSetIRQMask();

    // Read from sw rx buffer first then hw rxfifo
    if(can_rx_buffer.hasData == true)
    {
        rxPtr = can_rx_buffer.buffer;
    }
    else
    {
        rxPtr = (uint8_t *)&(reg->RXD0);
    }

    frame_info = *rxPtr++;

    dlc = frame_info & CAN_FRAME_INFO_DLC_MSK;

    if((frame_info & CAN_FRAME_INFO_FF) == 0)
    {
        // Standard frame
        temp = *rxPtr++;
        id = temp << 3;
        temp = *rxPtr++;

        id |= (temp >> 5);
        msg_info->id = id;
    }
    else
    {
        // Extended frame
        temp = *rxPtr++;
        id = temp << 21;

        temp = *rxPtr++;
        id |= (temp << 13);

        temp = *rxPtr++;
        id |= (temp << 5);

        temp = *rxPtr++;
        id |= (temp >> 3);

        msg_info->id = id | ARM_CAN_ID_IDE_Msk;
    }

    if(frame_info & CAN_FRAME_INFO_RTR)
    {
        msg_info->rtr = 1;
        size = 0;
    }
    else
    {
        msg_info->rtr = 0;
        size = dlc;
    }

    msg_info->dlc = dlc;

    if(size > 8U)
    {
        size = 8U;
    }

    for(uint32_t i = 0; i < size; i++)
    {
        data[i] = *rxPtr++;
    }

    if(can_rx_buffer.hasData == true)
    {
        can_rx_buffer.hasData = false;
    }
    else
    {
        reg->CMDR = CAN_CMDR_RELEASE_BUF_Msk;
    }

    if(can->reg->RMCR == 0)
    {
        reg->IER |= CAN_ISR_RX_REQ_Msk; // Reable RX int
    }
    RestoreIRQMask(mask);


    return ((int32_t)size);
}

static int32_t CAN_GetRxMessageCount(uint32_t obj_idx, CAN_RESOURCES *can)
{
    if ((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if((can_obj_cfg[obj_idx] == ARM_CAN_OBJ_TX) || (can_obj_cfg[obj_idx] == ARM_CAN_OBJ_RX_RTR_TX_DATA))
    {
        return ARM_DRIVER_ERROR;
    }
    uint32_t rx_message_cnt = can->reg->RMCR;

    uint32_t mask = SaveAndSetIRQMask();

    if(can_rx_buffer.hasData == true)
    {
        rx_message_cnt++;
    }
    RestoreIRQMask(mask);

    return rx_message_cnt;
}

static int32_t CAN_Control(uint32_t control, uint32_t arg, CAN_RESOURCES *can)
{

    if ((can->ctrl->flags & CAN_FLAG_POWER) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    switch (control & ARM_CAN_CONTROL_Msk)
    {
        case ARM_CAN_ABORT_MESSAGE_SEND:

            if((arg < CAN_RX_OBJ_NUM) || (arg > CAN_TOT_OBJ_NUM))
            {
                return ARM_DRIVER_ERROR_PARAMETER;
            }

            // tx buffer is locked, then send abort request
            if((can->reg->STR & CAN_STR_TX_BUF_Msk) == 0)
            {
                uint32_t mask = SaveAndSetIRQMask();
                /*
                    Note:
                    TX complete irq will also be triggered when abort done,
                    however we have no way to know if message has been transimitted successfully or not.
                 */
                can->reg->CMDR = CAN_CMDR_TX_ABT_Msk;
                can->ctrl->pending |= CAN_ABORT_SYNC_FLAG;
                RestoreIRQMask(mask);

                // Wait tx buffer release
                while((can->reg->STR & CAN_STR_TX_BUF_Msk) == 0);
            }

            break;

        case ARM_CAN_RECOVER_FROM_BUS_OFF:
            CAN_recoverFromBusOff(can);
            break;

        case ARM_CAN_SET_TRANSCEIVER_STANDBY:
            // Drive high or low
            can->reg->STBR = !!arg;
            break;

#if 0
        case ARM_CAN_ABORT_RETRANS_ON_NACK:
            if(arg == 0)
            {
                can->ctrl->mode &= ~CAN_MODE_ABORT_RETRANS_ON_NACK;
            }
            else
            {
                /*
                Note:
                    there may be a few retransmissions before abort is taken effect
                    since the abort request is sent in isr and isr execution always has delay.
                */
                can->ctrl->mode |= CAN_MODE_ABORT_RETRANS_ON_NACK;
            }
            break;
#endif
        case ARM_CAN_CONTROL_RETRANSMISSION:

            if(arg == 0)
            {
                can->ctrl->mode |= CAN_MODE_ONESHOT;
            }
            else
            {
                can->ctrl->mode &= ~CAN_MODE_ONESHOT;
            }
            break;

        case ARM_CAN_SET_FD_MODE:
        case ARM_CAN_SET_TRANSCEIVER_DELAY:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

static ARM_CAN_STATUS CAN_GetStatus (CAN_RESOURCES *can)
{
    volatile ARM_CAN_STATUS temp = {0};
    CAN_TypeDef *reg = can->reg;
    CAN_CTRL    *ctrl = can->ctrl;
    uint32_t mask;

    switch(ctrl->state)
    {
        case CAN_STATE_ERROR_ACTIVE:
        case CAN_STATE_ERROR_WARNING:
            temp.unit_state = ARM_CAN_UNIT_STATE_ACTIVE;
            break;

        case CAN_STATE_ERROR_PASSIVE:
            temp.unit_state = ARM_CAN_UNIT_STATE_PASSIVE;
            break;
        case CAN_STATE_BUS_OFF:
            temp.unit_state = ARM_CAN_UNIT_STATE_BUS_OFF;
            break;

        default:
            temp.unit_state = ARM_CAN_UNIT_STATE_INACTIVE;
            break;

    }

    temp.tx_error_count = reg->TECR;
    temp.rx_error_count = reg->RECR;

    mask = SaveAndSetIRQMask();

    if(ctrl->lastErrorCode)
    {
        uint32_t ecc_type = EIGEN_FLD2VAL(CAN_ECCR_CODE, ctrl->lastErrorCode);
        uint32_t ecc_loc = EIGEN_FLD2VAL(CAN_ECCR_SEG, ctrl->lastErrorCode);

        ctrl->lastErrorCode = 0;
        RestoreIRQMask(mask);

        switch(ecc_type)
        {
            case CAN_ECC_BIT:
                temp.last_error_code = ARM_CAN_LEC_BIT_ERROR;
                break;

            case CAN_ECC_FORM:
                temp.last_error_code = ARM_CAN_LEC_FORM_ERROR;
                break;

            case CAN_ECC_STUFF:
                temp.last_error_code = ARM_CAN_LEC_STUFF_ERROR;
                break;

            case CAN_ECC_OTHER:
                if(ecc_loc == CAN_ECC_LOC_ACK)
                {
                    temp.last_error_code = ARM_CAN_LEC_ACK_ERROR;
                }

                break;
        }

    }

    RestoreIRQMask(mask);

    return temp;
}


// IRQ handlers
// Add interrupt routines to handle transmission, reception, error and status interrupts
// ..
void CAN_IRQHandler(CAN_RESOURCES *can)
{
    uint32_t n = 0, frame_len = 0;
    CAN_TypeDef *reg = can->reg;
    CAN_CTRL    *ctrl = can->ctrl;
    uint8_t frame_info = 0;
    uint8_t *rxPtr = NULL;
    volatile CAN_STATE can_state;
    uint32_t tx_object_event, rx_object_event, unit_event;

    volatile uint8_t isr;
    volatile uint8_t str;
    volatile uint8_t ecc;

    while((isr = reg->ISR) && (n < CAN_MAX_IRQ_ROUNDS))
    {
        tx_object_event = 0;
        rx_object_event = 0;
        ecc = 0;

        str = reg->STR;
        can_state = ctrl->state;

        ctrl->stat.txErrorCount = reg->TECR;
        ctrl->stat.rxErrorCount = reg->RECR;

#ifdef PM_FEATURE_ENABLE
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, CAN_IRQHandler_0, P_DEBUG, "ISR: 0x%x, STR: 0x%x, TEC: %d, REC: %d, RMC: %d", isr, str, reg->TECR, reg->RECR, reg->RMCR);
#else
        CANDEBUG("ISR: 0x%x, STR: 0x%x, TEC: %d, REC: %d, RMC: %d\n", isr, str, reg->TECR, reg->RECR, reg->RMCR);
#endif


        if(isr & CAN_ISR_TX_EMPTY_Msk)
        {
            ecc = reg->ECCR;

            if(ecc)
            {
                isr |= CAN_ISR_BUS_ERR_Msk;
            }

            // Abort op will also trigger this irq, so shall mask this event report
            if(ctrl->pending & CAN_ABORT_SYNC_FLAG)
            {
                ctrl->pending &= ~CAN_ABORT_SYNC_FLAG;
            }
            else
            {
                if((can_obj_cfg[CAN_TX_OBJ_IDX_START] == ARM_CAN_OBJ_TX) && (ecc == 0))
                {
                    tx_object_event = ARM_CAN_EVENT_SEND_COMPLETE;
                }
            }

        }

        if(isr & CAN_ISR_RX_REQ_Msk)
        {
            reg->IER &= ~CAN_ISR_RX_REQ_Msk;

            if(can_obj_cfg[0] == ARM_CAN_OBJ_RX)
            {

                // Store one message to rx buffer so that we can clear rx int source
                if(can_rx_buffer.hasData == false)
                {
                    can_rx_buffer.hasData = true;

                    rxPtr = (uint8_t *)&(reg->RXD0);

                    frame_info = *rxPtr++;

                    frame_len = frame_info & CAN_FRAME_INFO_DLC_MSK;

                    can_rx_buffer.buffer[0] = frame_info;

                    if((frame_info & CAN_FRAME_INFO_FF) == 0)
                    {
                        // Standard frame
                        frame_len += 2;
                    }
                    else
                    {
                        // Extended frame
                        frame_len += 4;
                    }

                    for(uint32_t i = 1; i <= frame_len; i++)
                    {
                        can_rx_buffer.buffer[i] = *rxPtr++;
                    }

                    reg->CMDR = CAN_CMDR_RELEASE_BUF_Msk;
                }
                else
                {
                    // Can't happen
                }

                if(isr & CAN_ISR_OVERRUN_Msk)
                {
                    reg->CMDR = CAN_CMDR_CLR_OVERRUN_Msk;
                    rx_object_event = ARM_CAN_EVENT_RECEIVE | ARM_CAN_EVENT_RECEIVE_OVERRUN;
                }
                else
                {
                    rx_object_event = ARM_CAN_EVENT_RECEIVE;
                }

            }
            else
            {

                if(isr & CAN_ISR_OVERRUN_Msk)
                {
                    reg->CMDR = CAN_CMDR_CLR_OVERRUN_Msk | CAN_CMDR_RELEASE_BUF_Msk;
                }
                else
                {
                    reg->CMDR = CAN_CMDR_RELEASE_BUF_Msk; // Release rxfifo if object not enabled for rx
                }
            }


        }

        if(isr & CAN_ISR_BUS_ERR_Msk)
        {
            // Not the TX complete case
            if(ecc == 0)
            {
                ecc = reg->ECCR;
            }
            ctrl->stat.busError++;
            ctrl->stat.ecc = ecc;
            ctrl->lastErrorCode = ecc;

            uint32_t ecc_type = EIGEN_FLD2VAL(CAN_ECCR_CODE, ecc);
            uint32_t ecc_dir = EIGEN_FLD2VAL(CAN_ECCR_DIR, ecc);
            uint32_t ecc_loc = EIGEN_FLD2VAL(CAN_ECCR_SEG, ecc);

            (void)ecc_type;
            (void)ecc_dir;
            (void)ecc_loc;

#ifdef PM_FEATURE_ENABLE
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, CAN_IRQHandler_1, P_DEBUG, "BUS error code: 0x%x, type: %d(0->bit, 1->form, 2->stuff, 3->other), dir: %d(1->rx, 0->tx), location: %s",
                                                                                          ecc, ecc_type, ecc_dir, eccLocToStr[ecc_loc]);
#else
            CANDEBUG("BUS error code: 0x%x, type: %s, dir: %s, location: %s\n", ecc, eccTypeToStr[ecc_type], (ecc_dir == 1) ? "rx" : "tx" , eccLocToStr[ecc_loc]);
#endif
        }

        if(isr & CAN_ISR_ARB_LOST_Msk)
        {
            ctrl->stat.arbLost++;
            ctrl->stat.arbLostLoc = reg->ALCR & 0x1f;

#ifdef PM_FEATURE_ENABLE
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, CAN_IRQHandler_2, P_DEBUG, "ARB lost, location: %d", ctrl->stat.arbLostLoc);
#else
            CANDEBUG("ARB lost, location: %d\n", ctrl->stat.arbLostLoc);
#endif
        }

        if(isr & CAN_ISR_ERR_WARNING_Msk)
        {
            if(str & CAN_STR_BUS_Msk)
            {
                can_state = CAN_STATE_BUS_OFF;
            }
            else if(str & CAN_STR_ERR_Msk)
            {
                can_state = CAN_STATE_ERROR_WARNING;
            }
            else
            {
                can_state = CAN_STATE_ERROR_ACTIVE;
            }
        }

        if(isr & CAN_ISR_ERR_PASSIVE_Msk)
        {
            if(can_state == CAN_STATE_ERROR_PASSIVE)
            {
                can_state = CAN_STATE_ERROR_WARNING;
                ctrl->stat.errorWarning++;
            }
            else
            {
                can_state = CAN_STATE_ERROR_PASSIVE;
                ctrl->stat.errorPassive++;
            }
        }

        if(can_state != ctrl->state)
        {
            ctrl->state = can_state;
            switch(can_state)
            {
                case CAN_STATE_ERROR_ACTIVE:
                    unit_event = ARM_CAN_EVENT_UNIT_ACTIVE;
                    break;
                case CAN_STATE_ERROR_WARNING:
                    unit_event = ARM_CAN_EVENT_UNIT_WARNING;
                    break;

                case CAN_STATE_ERROR_PASSIVE:
                    unit_event = ARM_CAN_EVENT_UNIT_PASSIVE;
                    break;
                case CAN_STATE_BUS_OFF:
                    unit_event = ARM_CAN_EVENT_UNIT_BUS_OFF;
                    break;
                default:
                    unit_event = 0;
                    break;

            }

            if((ctrl->unit_event_cb != NULL) && (unit_event != 0))
            {
                ctrl->unit_event_cb(unit_event);
            }
        }

        if((ctrl->object_event_cb != NULL))
        {
            if(tx_object_event != 0)
            {
                ctrl->object_event_cb(CAN_TX_OBJ_IDX_START, tx_object_event);
            }
            if(rx_object_event != 0)
            {
                ctrl->object_event_cb(0, rx_object_event);
            }
        }
        n++;
    }

    if((n == CAN_MAX_IRQ_ROUNDS) && (reg->STR & CAN_STR_RX_BUF_Msk))
    {
        //
    }

}

// CAN driver functions structure
#if (RTE_CAN0)
static int32_t CAN0_Initialize(ARM_CAN_SignalUnitEvent_t cb_unit_event, ARM_CAN_SignalObjectEvent_t cb_object_event)
{
    return CAN_Initialize(cb_unit_event, cb_object_event, &CAN0_Resources);
}
static int32_t CAN0_Uninitialize(void)
{
    return CAN_Uninitialize(&CAN0_Resources);
}
static int32_t CAN0_PowerControl(ARM_POWER_STATE state)
{
    return CAN_PowerControl(state, &CAN0_Resources);
}
static uint32_t CAN0_GetClock(void)
{
    return CAN_GetClock(&CAN0_Resources);
}
static int32_t CAN0_SetBitrate(ARM_CAN_BITRATE_SELECT select, uint32_t bitrate, uint32_t bit_segments)
{
    return CAN_SetBitrate(select, bitrate, bit_segments, &CAN0_Resources);
}
static int32_t CAN0_SetMode(ARM_CAN_MODE mode)
{
    return CAN_SetMode(mode, &CAN0_Resources);
}
static ARM_CAN_OBJ_CAPABILITIES CAN0_ObjectGetCapabilities(uint32_t obj_idx)
{
    return CAN_ObjectGetCapabilities(obj_idx, &CAN0_Resources);
}
static int32_t CAN0_ObjectSetFilter(uint32_t obj_idx, ARM_CAN_FILTER_OPERATION operation, uint32_t id, uint32_t arg)
{
    return CAN_ObjectSetFilter(obj_idx, operation, id, arg, &CAN0_Resources);
}
static int32_t CAN0_ObjectConfigure(uint32_t obj_idx, ARM_CAN_OBJ_CONFIG obj_cfg)
{
    return CAN_ObjectConfigure(obj_idx, obj_cfg, &CAN0_Resources);
}
static int32_t CAN0_MessageSend(uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, const uint8_t *data, uint8_t size)
{
    return CAN_MessageSend(obj_idx, msg_info, data, size, &CAN0_Resources);
}
static int32_t CAN0_MessageRead(uint32_t obj_idx, ARM_CAN_MSG_INFO *msg_info, uint8_t *data, uint8_t size)
{
    return CAN_MessageRead(obj_idx, msg_info, data, size, &CAN0_Resources);
}

static int32_t CAN0_GetRxMessageCount(uint32_t obj_idx)
{
    return CAN_GetRxMessageCount(obj_idx, &CAN0_Resources);
}

static int32_t CAN0_Control(uint32_t control, uint32_t arg)
{
    return CAN_Control(control, arg, &CAN0_Resources);
}
static ARM_CAN_STATUS CAN0_GetStatus(void)
{
    return CAN_GetStatus(&CAN0_Resources);
}
void CAN0_IRQHandler(void)
{
    CAN_IRQHandler(&CAN0_Resources);
}

ARM_DRIVER_CAN Driver_CAN0 = {
  CAN_GetVersion,
  CAN_GetCapabilities,
  CAN0_Initialize,
  CAN0_Uninitialize,
  CAN0_PowerControl,
  CAN0_GetClock,
  CAN0_SetBitrate,
  CAN0_SetMode,
  CAN0_ObjectGetCapabilities,
  CAN0_ObjectSetFilter,
  CAN0_ObjectConfigure,
  CAN0_MessageSend,
  CAN0_MessageRead,
  CAN0_GetRxMessageCount,
  CAN0_Control,
  CAN0_GetStatus
};
#endif


