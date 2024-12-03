/*
 * Copyright (c) 2013-2016 ARM Limited. All rights reserved.
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
#include "Driver_Common.h"
#include "bsp_usart.h"
#include "slpman.h"

#ifdef __USER_CODE__
#else
#define BSP_USART_TEXT_SECTION     SECTION_DEF_IMPL(.sect_bsp_usart_text)
#define BSP_USART_RODATA_SECTION   SECTION_DEF_IMPL(.sect_bsp_usart_rodata)
#define BSP_USART_DATA_SECTION     SECTION_DEF_IMPL(.sect_bsp_usart_data)
#define BSP_USART_BSS_SECTION      SECTION_DEF_IMPL(.sect_bsp_usart_bss)

#ifdef PM_FEATURE_ENABLE
#define USART_DEBUG  1
#if USART_DEBUG
#include DEBUG_LOG_HEADER_FILE
#endif
#endif

#define ARM_USART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0)  /* driver version */

#if ((!RTE_UART0) && (!RTE_UART1) && (!RTE_UART2) && (!RTE_UART3))
#error "UART not enabled in RTE_Device.h!"
#endif

#ifndef RTE_UART0_RX_IO_MODE
#define RTE_UART0_RX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART0_TX_IO_MODE
#define RTE_UART0_TX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART1_RX_IO_MODE
#define RTE_UART1_RX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART1_TX_IO_MODE
#define RTE_UART1_TX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART2_RX_IO_MODE
#define RTE_UART2_RX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART2_TX_IO_MODE
#define RTE_UART2_TX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART3_RX_IO_MODE
#define RTE_UART3_RX_IO_MODE POLLING_MODE
#endif

#ifndef RTE_UART3_TX_IO_MODE
#define RTE_UART3_TX_IO_MODE POLLING_MODE
#endif

#define UART_DMA_BURST_SIZE          DMA_BURST_8_BYTES
#define UART_DMA_MAX_TRANSFER_SIZE   (8191)

// Trigger level definitions
#ifndef USART0_RX_TRIG_LVL
#define USART0_RX_TRIG_LVL   16
#endif

#ifndef USART1_RX_TRIG_LVL
#define USART1_RX_TRIG_LVL   16
#endif

#ifndef USART2_RX_TRIG_LVL
#define USART2_RX_TRIG_LVL   16
#endif

#ifndef USART3_RX_TRIG_LVL
#define USART3_RX_TRIG_LVL   16
#endif

#define USART0_TX_TRIG_LVL   8
#define USART1_TX_TRIG_LVL   8
#define USART2_TX_TRIG_LVL   8
#define USART3_TX_TRIG_LVL   8

#ifdef PM_FEATURE_ENABLE
/** \brief Internal used data structure */
typedef struct _usart_database
{
    uint8_t                            isInited;            /**< Whether usart has been initialized */
    uint8_t                            reserved0;
    uint16_t                           reserved1;
    struct
    {
        uint32_t ENR;                           /**< Usart Enable Register,              offset: 0x0*/
        uint32_t DIVR;                          /**< Usart Divder Register,              offset: 0x4*/
        uint32_t LCR;                           /**< Usart Line Control Register,        offset: 0x8*/
        uint32_t FCR0;                          /**< Usart Fifo Control 0 Register,      offset: 0x10*/
        uint32_t MCR;                           /**< Usart Mode Control Register,        offset: 0x14*/
        uint32_t IER;                           /**< Usart Interrupt Enable Register,    offset: 0x28*/
        uint32_t FLOWCR;                        /**< Usart Flow Control Register,        offset: 0x34*/
        uint32_t HCR;                           /**< Usart Hw Control Register,          offset: 0x40*/
        uint32_t FCR1;                          /**< Usart Fifo Control 1 Register,      offset: 0x58*/
    } backup_registers;                         /**< Backup registers for low power restore */
} usart_database_t;

BSP_USART_BSS_SECTION static usart_database_t g_usartDataBase[USART_INSTANCE_NUM] = {0};

static USART_TypeDef* const g_usartBases[] = USART_INSTANCE_ARRAY;
#endif

static const ClockId_e g_uartClocks[] = UART_CLOCK_VECTOR;

static const ClockResetVector_t g_uartResetVectors[] = UART_RESET_VECTORS;

#ifdef PM_FEATURE_ENABLE
/**
  \brief usart initialization counter, for lower power callback register/de-register
 */
BSP_USART_BSS_SECTION static uint32_t g_usartInitCounter = 0;

/**
  \brief Bitmap of USART working status, upper 16 bits act as mask
         when all USART instances are not working, we can vote to enter to low power state.
 */
BSP_USART_DATA_SECTION static uint32_t g_usartWorkingStatus = 0xFFFF0000;

#ifdef TYPE_EC718M

// psram needs to keep alive for uart receiving data in dma mode when baudrate exceeds 500K bps, this gives us 640us(32*10/500K) before rxfifo overflows
#define USART_PSRAM_HYBD_VOTE_BAUDRATE_LIMIT   (500000)

/**
  \brief usart psram hybird mode vote bitmap
 */
BSP_USART_BSS_SECTION static uint32_t g_usartPsramVoteBitmap = 0;

static void USART_LockPsramHybd(uint32_t instance)
{
    uint32_t mask = SaveAndSetIRQMask();

    if((g_usartPsramVoteBitmap & (1 << instance)) == 0)
    {
        slpManPSRamVoteHybdDisable(PSRAM_HYBD_VOTE_UART);
        g_usartPsramVoteBitmap |= (1 << instance);
    }
    RestoreIRQMask(mask);

}
static void USART_UnlockPsramHybd(uint32_t instance)
{
    uint32_t mask = SaveAndSetIRQMask();
    if(g_usartPsramVoteBitmap & (1 << instance))
    {
        slpManPSRamVoteHybdEnable(PSRAM_HYBD_VOTE_UART);
        g_usartPsramVoteBitmap &=~(1 << instance);
    }
    RestoreIRQMask(mask);
}

#endif
/**
  \fn        static void USART_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, USART hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void USART_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_IDLE_STATE:

            for(i = 0; i < USART_INSTANCE_NUM; i++)
            {
                if((g_usartDataBase[i].isInited == true) && (g_usartBases[i]->MCR & USART_MCR_RX_DMA_EN_Msk))
                {
                    g_usartBases[i]->IER |= USART_IER_RXFIFO_WL_Msk;
                }
            }

            break;
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < USART_INSTANCE_NUM; i++)
            {
                if(g_usartDataBase[i].isInited == true)
                {
                    g_usartDataBase[i].backup_registers.ENR = g_usartBases[i]->ENR;
                    g_usartDataBase[i].backup_registers.DIVR = g_usartBases[i]->DIVR;
                    g_usartDataBase[i].backup_registers.LCR = g_usartBases[i]->LCR;
                    g_usartDataBase[i].backup_registers.FCR0 = g_usartBases[i]->FCR0;
                    g_usartDataBase[i].backup_registers.FCR1 = g_usartBases[i]->FCR1;
                    g_usartDataBase[i].backup_registers.MCR = g_usartBases[i]->MCR;
                    g_usartDataBase[i].backup_registers.IER = g_usartBases[i]->IER;
                    g_usartDataBase[i].backup_registers.FLOWCR = g_usartBases[i]->FLOWCR;
                    g_usartDataBase[i].backup_registers.HCR = g_usartBases[i]->HCR;
                }
            }
            break;
        default:
            break;
    }

}

/**
  \fn        static void USART_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, USART hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state

 */
static void USART_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;
    extern bool apmuGetSleepedFlag(void);

    switch (state)
    {
        case SLPMAN_IDLE_STATE:

            for(i = 0; i < USART_INSTANCE_NUM; i++)
            {
                if((g_usartDataBase[i].isInited == true) && (g_usartBases[i]->MCR & USART_MCR_RX_DMA_EN_Msk))
                {
                    g_usartBases[i]->IER &= ~USART_IER_RXFIFO_WL_Msk;
                    g_usartBases[i]->ICR = USART_ICR_RXFIFO_WL_Msk;
                }
            }
            break;

        case SLPMAN_SLEEP1_STATE:

            // no need to restore if failing to sleep
            if(apmuGetSleepedFlag() == false)
            {
                break;
            }

            for(i = 0; i < USART_INSTANCE_NUM; i++)
            {
                if(g_usartDataBase[i].isInited == true)
                {
                    GPR_clockEnable(g_uartClocks[2*i]);
                    GPR_clockEnable(g_uartClocks[2*i+1]);

                    g_usartBases[i]->DIVR = g_usartDataBase[i].backup_registers.DIVR;
                    g_usartBases[i]->LCR = g_usartDataBase[i].backup_registers.LCR;
                    g_usartBases[i]->RXSR = 0;
                    g_usartBases[i]->FCR0 = g_usartDataBase[i].backup_registers.FCR0;
                    g_usartBases[i]->FCR1 = g_usartDataBase[i].backup_registers.FCR1;
                    g_usartBases[i]->MCR = g_usartDataBase[i].backup_registers.MCR;
                    g_usartBases[i]->IER = g_usartDataBase[i].backup_registers.IER;

                    if(g_usartBases[i]->IER & USART_IER_AUTOBAUD_DONE_Msk)
                    {
                        g_usartBases[i]->ABSR = 1;
                    }

                    g_usartBases[i]->FLOWCR = g_usartDataBase[i].backup_registers.FLOWCR;
                    g_usartBases[i]->HCR = g_usartDataBase[i].backup_registers.HCR;
                    g_usartBases[i]->ENR = g_usartDataBase[i].backup_registers.ENR;
                }
            }
            break;

        default:
            break;
    }

}

#define  LOCK_SLEEP(instance, tx, rx)     do                                                                   \
                                          {                                                                    \
                                              if((g_usartWorkingStatus & (1 << (31 - instance))) == 0)         \
                                              {                                                                \
                                                  g_usartWorkingStatus |= (rx << (2 * instance));              \
                                                  g_usartWorkingStatus |= (tx << (2 * instance + 1));          \
                                                  slpManDrvVoteSleep(SLP_VOTE_USART, SLP_ACTIVE_STATE);        \
                                              }                                                                \
                                          }                                                                    \
                                          while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance, tx, rx)      do                                                                    \
                                                      {                                                                     \
                                                          g_usartWorkingStatus &= ~(rx << (2 * instance));                  \
                                                          g_usartWorkingStatus &= ~(tx << (2 * instance + 1));              \
                                                          if((g_usartWorkingStatus & 0xFF) == 0)                            \
                                                              slpManDrvVoteSleep(SLP_VOTE_USART, SLP_SLP1_STATE);           \
                                                      }                                                                     \
                                                      while(0)
#endif

#define USART_WAIT_TX_DONE(usart)     do                                                                       \
                                      {                                                                        \
                                         while(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, usart->reg->FSR) != 0);      \
                                         while(UART_readLSR(&(usart->reg->LSR)) & USART_LSR_TX_BUSY_Msk);                       \
                                      } while(0)

// declearation for DMA API
extern uint32_t DMA_getChannelCurrentTargetAddress(DmaInstance_e instance, uint32_t channel, bool sync);
extern void DMA_startChannelNoVote(DmaInstance_e instance, uint32_t channel);

// Driver Version
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_USART_DRV_VERSION
};

// Driver Capabilities
static const ARM_USART_CAPABILITIES DriverCapabilities = {
    1, /* supports UART (Asynchronous) mode */
    0, /* supports Synchronous Master mode */
    0, /* supports Synchronous Slave mode */
    0, /* supports UART Single-wire mode */
    0, /* supports UART IrDA mode */
    0, /* supports UART Smart Card mode */
    0, /* Smart Card Clock generator available */
    0, /* RTS Flow Control available */
    0, /* CTS Flow Control available */
    0, /* Transmit completed event: \ref ARM_USART_EVENT_TX_COMPLETE */
    0, /* Signal receive character timeout event: \ref ARM_USART_EVENT_RX_TIMEOUT */
    0, /* RTS Line: 0=not available, 1=available */
    0, /* CTS Line: 0=not available, 1=available */
    0, /* DTR Line: 0=not available, 1=available */
    0, /* DSR Line: 0=not available, 1=available */
    0, /* DCD Line: 0=not available, 1=available */
    0, /* RI Line: 0=not available, 1=available */
    0, /* Signal CTS change event: \ref ARM_USART_EVENT_CTS */
    0, /* Signal DSR change event: \ref ARM_USART_EVENT_DSR */
    0, /* Signal DCD change event: \ref ARM_USART_EVENT_DCD */
    0  /* Signal RI change event: \ref ARM_USART_EVENT_RI */
};

void USART0_IRQHandler(void);
void USART1_IRQHandler(void);
void USART2_IRQHandler(void);
void USART3_IRQHandler(void);

#if (RTE_UART0)

BSP_USART_BSS_SECTION static USART_INFO      USART0_Info = { 0U };
static const PIN USART0_pin_tx  = {RTE_UART0_TX_BIT,   RTE_UART0_TX_FUNC};
static const PIN USART0_pin_rx  = {RTE_UART0_RX_BIT,   RTE_UART0_RX_FUNC};
#if (RTE_UART0_CTS_PIN_EN == 1)
static const PIN USART0_pin_cts  = {RTE_UART0_CTS_BIT,   RTE_UART0_CTS_FUNC};
#endif
#if (RTE_UART0_RTS_PIN_EN == 1)
static const PIN USART0_pin_rts  = {RTE_UART0_RTS_BIT,   RTE_UART0_RTS_FUNC};
#endif

#if (RTE_UART0_TX_IO_MODE == DMA_MODE)

void USART0_DmaTxEvent(uint32_t event);
BSP_USART_DATA_SECTION static USART_TX_DMA USART0_DMA_Tx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART0_DMA_TX_REQID,
                                    NULL
                                 };
#endif

#if (RTE_UART0_RX_IO_MODE == DMA_MODE)

void USART0_DmaRxEvent(uint32_t event);

BSP_USART_DATA_SECTION static USART_RX_DMA USART0_DMA_Rx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART0_DMA_RX_REQID,
                                    USART0_DmaRxEvent
                                 };

#endif

#if (RTE_UART0_TX_IO_MODE != POLLING_MODE) || (RTE_UART0_RX_IO_MODE != POLLING_MODE)
BSP_USART_DATA_SECTION static USART_IRQ USART0_IRQ = {
                                PXIC0_UART0_IRQn,
                                USART0_IRQHandler
                              };
#endif


static const USART_RESOURCES USART0_Resources = {
    USART_0,
    {
      &USART0_pin_tx,
      &USART0_pin_rx,
#if (RTE_UART0_CTS_PIN_EN == 1)
      &USART0_pin_cts,
#else
      NULL,
#endif
#if (RTE_UART0_RTS_PIN_EN == 1)
      &USART0_pin_rts,
#else
      NULL,
#endif
    },

#if (RTE_UART0_TX_IO_MODE == DMA_MODE)
    &USART0_DMA_Tx,
#else
    NULL,
#endif

#if (RTE_UART0_RX_IO_MODE == DMA_MODE)
    &USART0_DMA_Rx,
#else
    NULL,
#endif
#if (RTE_UART0_TX_IO_MODE != POLLING_MODE) || (RTE_UART0_RX_IO_MODE != POLLING_MODE)
    &USART0_IRQ,
#else
    NULL,
#endif
    USART0_TX_TRIG_LVL,
    USART0_RX_TRIG_LVL,
    &USART0_Info,
    (RTE_UART0_RX_IO_MODE << USART_RX_IO_MODE_POS) | RTE_UART0_TX_IO_MODE
};

#endif

#if (RTE_UART1)

BSP_USART_BSS_SECTION static USART_INFO    USART1_Info  = { 0 };
static const PIN USART1_pin_tx  = {RTE_UART1_TX_BIT,   RTE_UART1_TX_FUNC};
static const PIN USART1_pin_rx  = {RTE_UART1_RX_BIT,   RTE_UART1_RX_FUNC};
#if (RTE_UART1_CTS_PIN_EN == 1)
static const PIN USART1_pin_cts  = {RTE_UART1_CTS_BIT,   RTE_UART1_CTS_FUNC};
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
static const PIN USART1_pin_rts  = {RTE_UART1_RTS_BIT,   RTE_UART1_RTS_FUNC};
#endif

#if (RTE_UART1_TX_IO_MODE == DMA_MODE)

void USART1_DmaTxEvent(uint32_t event);
BSP_USART_DATA_SECTION static USART_TX_DMA USART1_DMA_Tx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART1_DMA_TX_REQID,
                                    NULL
                                 };
#endif

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)

void USART1_DmaRxEvent(uint32_t event);

BSP_USART_DATA_SECTION static USART_RX_DMA USART1_DMA_Rx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART1_DMA_RX_REQID,
                                    USART1_DmaRxEvent
                                 };

#endif

#if (RTE_UART1_TX_IO_MODE != POLLING_MODE) || (RTE_UART1_RX_IO_MODE != POLLING_MODE)
BSP_USART_DATA_SECTION static USART_IRQ USART1_IRQ = {
                                PXIC0_UART1_IRQn,
                                USART1_IRQHandler
                              };
#endif

static const USART_RESOURCES USART1_Resources = {
    USART_1,
    {
      &USART1_pin_tx,
      &USART1_pin_rx,
#if (RTE_UART1_CTS_PIN_EN == 1)
      &USART1_pin_cts,
#else
      NULL,
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
      &USART1_pin_rts,
#else
      NULL,
#endif
    },

#if (RTE_UART1_TX_IO_MODE == DMA_MODE)
    &USART1_DMA_Tx,
#else
    NULL,
#endif

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)
    &USART1_DMA_Rx,
#else
    NULL,
#endif
#if (RTE_UART1_TX_IO_MODE != POLLING_MODE) || (RTE_UART1_RX_IO_MODE != POLLING_MODE)
    &USART1_IRQ,
#else
    NULL,
#endif
    USART1_TX_TRIG_LVL,
    USART1_RX_TRIG_LVL,
    &USART1_Info,
    (RTE_UART1_RX_IO_MODE << USART_RX_IO_MODE_POS) | RTE_UART1_TX_IO_MODE
};
#endif

#if (RTE_UART2)

BSP_USART_BSS_SECTION static USART_INFO    USART2_Info  = { 0 };
static const PIN USART2_pin_tx  = {RTE_UART2_TX_BIT,   RTE_UART2_TX_FUNC};
static const PIN USART2_pin_rx  = {RTE_UART2_RX_BIT,   RTE_UART2_RX_FUNC};

#if (RTE_UART2_TX_IO_MODE == DMA_MODE)

void USART2_DmaTxEvent(uint32_t event);
BSP_USART_DATA_SECTION static USART_TX_DMA USART2_DMA_Tx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART2_DMA_TX_REQID,
                                    NULL
                                 };
#endif

#if (RTE_UART2_RX_IO_MODE == DMA_MODE)

void USART2_DmaRxEvent(uint32_t event);

BSP_USART_DATA_SECTION static USART_RX_DMA USART2_DMA_Rx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART2_DMA_RX_REQID,
                                    USART2_DmaRxEvent
                                 };

#endif

#if (RTE_UART2_TX_IO_MODE != POLLING_MODE) || (RTE_UART2_RX_IO_MODE != POLLING_MODE)
BSP_USART_DATA_SECTION static USART_IRQ USART2_IRQ = {
                                PXIC0_UART2_IRQn,
                                USART2_IRQHandler
                              };
#endif

static const USART_RESOURCES USART2_Resources = {
    USART_2,
    {
      &USART2_pin_tx,
      &USART2_pin_rx,
      NULL,
      NULL,
    },

#if (RTE_UART2_TX_IO_MODE == DMA_MODE)
    &USART2_DMA_Tx,
#else
    NULL,
#endif

#if (RTE_UART2_RX_IO_MODE == DMA_MODE)
    &USART2_DMA_Rx,
#else
    NULL,
#endif
#if (RTE_UART2_TX_IO_MODE != POLLING_MODE) || (RTE_UART2_RX_IO_MODE != POLLING_MODE)
    &USART2_IRQ,
#else
    NULL,
#endif
    USART2_TX_TRIG_LVL,
    USART2_RX_TRIG_LVL,
    &USART2_Info,
    (RTE_UART2_RX_IO_MODE << USART_RX_IO_MODE_POS) | RTE_UART2_TX_IO_MODE
};
#endif

#if (RTE_UART3)

BSP_USART_BSS_SECTION static USART_INFO    USART3_Info  = { 0 };
static const PIN USART3_pin_tx  = {RTE_UART3_TX_BIT,   RTE_UART3_TX_FUNC};
static const PIN USART3_pin_rx  = {RTE_UART3_RX_BIT,   RTE_UART3_RX_FUNC};

#if (RTE_UART3_TX_IO_MODE == DMA_MODE)

void USART3_DmaTxEvent(uint32_t event);
BSP_USART_DATA_SECTION static USART_TX_DMA USART3_DMA_Tx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART3_DMA_TX_REQID,
                                    NULL
                                 };
#endif

#if (RTE_UART3_RX_IO_MODE == DMA_MODE)

void USART3_DmaRxEvent(uint32_t event);

BSP_USART_DATA_SECTION static USART_RX_DMA USART3_DMA_Rx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART3_DMA_RX_REQID,
                                    USART3_DmaRxEvent
                                 };

#endif

#if (RTE_UART3_TX_IO_MODE != POLLING_MODE) || (RTE_UART3_RX_IO_MODE != POLLING_MODE)
BSP_USART_DATA_SECTION static USART_IRQ USART3_IRQ = {
                                PXIC0_UART3_IRQn,
                                USART3_IRQHandler
                              };
#endif

static const USART_RESOURCES USART3_Resources = {
    USART_3,
    {
      &USART3_pin_tx,
      &USART3_pin_rx,
      NULL,
      NULL,
    },

#if (RTE_UART3_TX_IO_MODE == DMA_MODE)
    &USART3_DMA_Tx,
#else
    NULL,
#endif

#if (RTE_UART3_RX_IO_MODE == DMA_MODE)
    &USART3_DMA_Rx,
#else
    NULL,
#endif
#if (RTE_UART3_TX_IO_MODE != POLLING_MODE) || (RTE_UART3_RX_IO_MODE != POLLING_MODE)
    &USART3_IRQ,
#else
    NULL,
#endif
    USART3_TX_TRIG_LVL,
    USART3_RX_TRIG_LVL,
    &USART3_Info,
    (RTE_UART3_RX_IO_MODE << USART_RX_IO_MODE_POS) | RTE_UART3_TX_IO_MODE
};
#endif

ARM_DRIVER_VERSION ARM_USART_GetVersion(void)
{
    return DriverVersion;
}

ARM_USART_CAPABILITIES USART_GetCapabilities(const USART_RESOURCES *usart)
{
    return DriverCapabilities;
}

PLAT_PA_RAMCODE static uint32_t USART_GetInstanceNumber(USART_RESOURCES *usart)
{
    return ((uint32_t)usart->reg - (uint32_t)USART_0) >> 12;
}

/*
 * when uart input clock is 26000000, the supported baudrate is:
 * 300,600,1200,2400,
 * 4800,9600,14400,19200,
 * 28800,38400,56000,57600,
 * 115200,230400,460800,921600,
 * 1000000,1500000,2000000,3000000
 *
 */
int32_t USART_SetBaudrate (uint32_t baudrate, USART_RESOURCES *usart)
{
    uint32_t uart_clock = 0;
    uint32_t div, instance;

    instance = USART_GetInstanceNumber(usart);

    uart_clock = GPR_getClockFreq(g_uartClocks[instance*2+1]);

    if(baudrate == 0)
    {
        // Disable uart first
        usart->reg->ENR = 0;
        // start detecting, irq is enabled in receive routine
        usart->reg->ABSR = 1;
        usart->reg->IER |= USART_IER_AUTOBAUD_DONE_Msk;
    }
    else
    {
        /*
         * formula to calculate baudrate, baudrate = clock_in / divisor_value,
         * where divisor_value = DIVR_INT.DIVR_FRAC(4 digits)
         */
        // round to nearest value
        div = ((uart_clock << 4) + (baudrate >> 1)) / baudrate;

        // Integer part of divisor value shall not be zero, otherwise, the result is invalid
        if ((div >> 4) == 0)
            return ARM_DRIVER_ERROR_PARAMETER;

        // Disable uart first
        usart->reg->ENR = 0;
        usart->reg->DIVR = div;

#ifdef PM_FEATURE_ENABLE
        // backup setting
        g_usartDataBase[instance].backup_registers.DIVR = div;
#endif
    }

    usart->info->baudrate = baudrate;

    return ARM_DRIVER_OK;
}

uint32_t USART_GetBaudRate(USART_RESOURCES *usart)
{
    return usart->info->baudrate;
}

int32_t USART_Initialize(ARM_USART_SignalEvent_t cb_event, USART_RESOURCES *usart)
{
    int32_t returnCode;
#ifdef PM_FEATURE_ENABLE
    uint32_t instance, mask;
#endif

    if (usart->info->flags & USART_FLAG_INITIALIZED)
        return ARM_DRIVER_OK;

    // Pin initialize
    PadConfig_t config;
    PAD_getDefaultConfig(&config);

#if defined CHIP_EC716
    config.schmittTriggerEnable = 1;
#endif

    config.mux = usart->pins.pin_tx->funcNum;
    PAD_setPinConfig(usart->pins.pin_tx->pinNum, &config);

    config.pullSelect = PAD_PULL_INTERNAL;
    config.pullUpEnable = PAD_PULL_UP_ENABLE;
    config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    config.mux = usart->pins.pin_rx->funcNum;

    PAD_setPinConfig(usart->pins.pin_rx->pinNum, &config);

    if(usart->pins.pin_cts)
    {
        config.mux = usart->pins.pin_cts->funcNum;
        PAD_setPinConfig(usart->pins.pin_cts->pinNum, &config);
    }

    if(usart->pins.pin_rts)
    {
        config.mux = usart->pins.pin_rts->funcNum;
        PAD_setPinConfig(usart->pins.pin_rts->pinNum, &config);
    }

#ifdef PM_FEATURE_ENABLE
    instance = USART_GetInstanceNumber(usart);
    g_usartDataBase[instance].isInited = true;
#endif

    // Initialize USART run-time resources
    usart->info->cb_event = cb_event;
    memset(&(usart->info->rx_status), 0, sizeof(USART_STATUS));

    usart->info->xfer.send_active           = 0U;
    usart->info->xfer.tx_def_val            = 0U;

    if (usart->dma_tx)
    {
        returnCode = DMA_openChannel(usart->dma_tx->instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            usart->dma_tx->channel = returnCode;

        DMA_setChannelRequestSource(usart->dma_tx->instance, usart->dma_tx->channel, (DmaRequestSource_e)usart->dma_tx->request);
        DMA_rigisterChannelCallback(usart->dma_tx->instance, usart->dma_tx->channel, usart->dma_tx->callback);
    }

    if (usart->dma_rx)
    {
        returnCode = DMA_openChannel(usart->dma_rx->instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            usart->dma_rx->channel = returnCode;

        DMA_setChannelRequestSource(usart->dma_rx->instance, usart->dma_rx->channel, (DmaRequestSource_e)usart->dma_rx->request);
        DMA_rigisterChannelCallback(usart->dma_rx->instance, usart->dma_rx->channel, usart->dma_rx->callback);
    }

    usart->info->flags = USART_FLAG_INITIALIZED;  // USART is initialized

#ifdef PM_FEATURE_ENABLE

    mask = SaveAndSetIRQMask();

    g_usartInitCounter++;

    // Unmask so that it's able to lock sleep
    g_usartWorkingStatus &= ~(1 << (31 - instance));

    RestoreIRQMask(mask);

    if(g_usartInitCounter == 1)
    {
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_USART_MODULE, USART_EnterLowPowerStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_USART_MODULE, USART_ExitLowPowerStateRestore, NULL);
    }
#endif
    return ARM_DRIVER_OK;
}

int32_t USART_Uninitialize(USART_RESOURCES *usart)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance, mask;
    instance = USART_GetInstanceNumber(usart);
#endif

    usart->info->flags = 0U;
    usart->info->cb_event = NULL;

    if(usart->dma_tx)
    {
        DMA_closeChannel(usart->dma_tx->instance, usart->dma_tx->channel);
    }

    if(usart->dma_rx)
    {
        DMA_closeChannel(usart->dma_rx->instance, usart->dma_rx->channel);
    }

#ifdef PM_FEATURE_ENABLE

    mask = SaveAndSetIRQMask();

    g_usartDataBase[instance].isInited = false;

    // In case of forcing to uninitlize when rx/tx is ongoing
    CHECK_TO_UNLOCK_SLEEP(instance, 1, 1);

    // Mask so that it's unable to lock sleep
    g_usartWorkingStatus |= (1 << (31 - instance));

    g_usartInitCounter--;

    if(g_usartInitCounter == 0)
    {
        g_usartWorkingStatus = 0xFFFF0000;
        RestoreIRQMask(mask);

        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_USART_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_USART_MODULE);
    }
    else
    {
        RestoreIRQMask(mask);
    }
#endif

    return ARM_DRIVER_OK;
}

int32_t USART_PowerControl(ARM_POWER_STATE state,USART_RESOURCES *usart)
{
    uint32_t instance;
    uint32_t val = 0;

    instance = USART_GetInstanceNumber(usart);

    switch (state)
    {
        case ARM_POWER_OFF:

            // Reset USART registers
            GPR_swResetModule(&g_uartResetVectors[instance]);

            // DMA disable
            if(usart->dma_tx)
                DMA_stopChannel(usart->dma_tx->instance, usart->dma_tx->channel, false);
            if(usart->dma_rx)
                DMA_stopChannel(usart->dma_rx->instance, usart->dma_rx->channel, false);

            // Disable power to usart clock
            CLOCK_clockDisable(g_uartClocks[instance*2]);
            CLOCK_clockDisable(g_uartClocks[instance*2+1]);

            // Clear driver variables
            memset(&(usart->info->rx_status), 0, sizeof(USART_STATUS));
            usart->info->frame_code       = 0U;
            usart->info->xfer.send_active = 0U;

            // Disable USART IRQ
            if(usart->usart_irq)
            {
                XIC_ClearPendingIRQ(usart->usart_irq->irq_num);
                XIC_DisableIRQ(usart->usart_irq->irq_num);
            }

            usart->info->flags &= ~(USART_FLAG_POWERED | USART_FLAG_CONFIGURED);

#ifdef PM_FEATURE_ENABLE

#ifdef TYPE_EC718M
            USART_UnlockPsramHybd(instance);
#endif

#endif
            break;

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case ARM_POWER_FULL:
            if ((usart->info->flags & USART_FLAG_INITIALIZED) == 0U)
            {
                return ARM_DRIVER_ERROR;
            }
            if (usart->info->flags & USART_FLAG_POWERED)
            {
                return ARM_DRIVER_OK;
            }

            // Enable power to usart clock
            CLOCK_clockEnable(g_uartClocks[instance*2]);
            CLOCK_clockEnable(g_uartClocks[instance*2+1]);

            GPR_swResetModule(&g_uartResetVectors[instance]);

            // Disable interrupts
            usart->reg->ENR = 0U;
            usart->reg->IER = 0U;
            // Clear driver variables
            memset(&(usart->info->rx_status), 0, sizeof(USART_STATUS));
            usart->info->frame_code       = 0U;
            usart->info->xfer.send_active = 0U;

            // Configure FIFO Control register
            val = EIGEN_VAL2FLD(USART_FCR0_TXFIFO_TH, usart->tx_fifo_trig_lvl) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TH, usart->rx_fifo_trig_lvl - 1) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TO_BIT_NUM, 16 + 10);

            usart->reg->FCR0 = val;

            if(usart->dma_rx)
            {
                usart->reg->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, usart->tx_fifo_trig_lvl) | \
                                   EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, 0);
            }
            else
            {
                usart->reg->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, usart->tx_fifo_trig_lvl) | \
                                   EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, usart->rx_fifo_trig_lvl - 1);
            }

            if((usart->io_mode & USART_TX_IO_MODE_MASK) == UNILOG_MODE)
            {
                usart->reg->MCR |= USART_MCR_TX_DMA_EN_Msk;
            }

            if(usart->usart_irq)
            {
                XIC_SetVector(usart->usart_irq->irq_num, usart->usart_irq->cb_irq);
                XIC_EnableIRQ(usart->usart_irq->irq_num);
                XIC_SuppressOvfIRQ(usart->usart_irq->irq_num);
            }
            usart->info->flags |= USART_FLAG_POWERED;  // USART is powered on

            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

int32_t USART_Send(const void *data, uint32_t num, USART_RESOURCES *usart)
{
    uint32_t mask;
#ifdef PM_FEATURE_ENABLE
    uint32_t instance = USART_GetInstanceNumber(usart);
#endif

    if ((data == NULL) || (num == 0U))
        return ARM_DRIVER_ERROR_PARAMETER;
    if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0U)
        return ARM_DRIVER_ERROR;

#ifdef PM_FEATURE_ENABLE

    if(usart->reg->IER & USART_IER_AUTOBAUD_DONE_Msk)
    {
        if(usart->info->cb_event && usart->dma_tx)
        {
            usart->info->cb_event(ARM_USART_EVENT_SEND_COMPLETE);
        }

        return ARM_DRIVER_OK;
    }
#endif

    mask = SaveAndSetIRQMask();
    if (usart->info->xfer.send_active != 0)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    usart->info->xfer.send_active = 1U;
    RestoreIRQMask(mask);


    // Save transmit buffer info
    usart->info->xfer.tx_buf = (uint8_t *)data;
    usart->info->xfer.tx_num = num;
    usart->info->xfer.tx_cnt = 0U;

    // DMA mode
    if((usart->io_mode & USART_TX_IO_MODE_MASK) == DMA_MODE)
    {

        DmaTransferConfig_t dmaTxConfig =  {NULL, NULL,
                                               DMA_FLOW_CONTROL_TARGET, DMA_ADDRESS_INCREMENT_SOURCE,
                                               DMA_DATA_WIDTH_ONE_BYTE, DMA_BURST_16_BYTES, 0
                                              };

        mask = SaveAndSetIRQMask();

        usart->reg->IER |= USART_IER_TX_TRANS_DONE_Msk;
        usart->reg->MCR |= USART_MCR_TX_DMA_EN_Msk;

#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance, 1, 0);
#endif

        RestoreIRQMask(mask);

        dmaTxConfig.sourceAddress = (void*)data;
        dmaTxConfig.targetAddress = (void*)&(usart->reg->TDR);
        dmaTxConfig.totalLength = num;

        // Configure tx DMA and start it
        DMA_transferSetup(usart->dma_tx->instance, usart->dma_tx->channel, &dmaTxConfig);
        DMA_startChannelNoVote(usart->dma_tx->instance, usart->dma_tx->channel);


    }
    else
    {
        while (usart->info->xfer.tx_cnt < usart->info->xfer.tx_num)
        {
            if(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, usart->reg->FSR) < 31)
            {
                usart->reg->TDR = usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt++];
            }
        }

        USART_WAIT_TX_DONE(usart);

        mask = SaveAndSetIRQMask();
        usart->info->xfer.send_active = 0U;
        RestoreIRQMask(mask);
    }

    return ARM_DRIVER_OK;
}


int32_t USART_SendPolling(const void *data, uint32_t num, USART_RESOURCES *usart)
{
    uint32_t mask;

    if ((data == NULL) || (num == 0U))
        return ARM_DRIVER_ERROR_PARAMETER;
    if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0U)
        return ARM_DRIVER_ERROR;

#ifdef PM_FEATURE_ENABLE
    if(usart->reg->IER & USART_IER_AUTOBAUD_DONE_Msk)
    {
        return ARM_DRIVER_OK;
    }
#endif

    mask = SaveAndSetIRQMask();
    if (usart->info->xfer.send_active != 0)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    usart->info->xfer.send_active = 1U;
    RestoreIRQMask(mask);

    // Save transmit buffer info
    usart->info->xfer.tx_buf = (uint8_t *)data;
    usart->info->xfer.tx_num = num;
    usart->info->xfer.tx_cnt = 0U;

    while (usart->info->xfer.tx_cnt < usart->info->xfer.tx_num)
    {
        if(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, usart->reg->FSR) < 31)
        {
            usart->reg->TDR = usart->info->xfer.tx_buf[usart->info->xfer.tx_cnt++];
        }
    }

    USART_WAIT_TX_DONE(usart);

    mask = SaveAndSetIRQMask();
    usart->info->xfer.send_active = 0U;
    RestoreIRQMask(mask);

    return ARM_DRIVER_OK;
}

int32_t USART_Receive(void *data, uint32_t num, USART_RESOURCES *usart)
{
    uint32_t mask;

    if ((data == NULL) || num == 0U)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((usart->info->flags & USART_FLAG_CONFIGURED) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    mask = SaveAndSetIRQMask();

    // check if receiver is busy
    if (usart->info->rx_status.rx_busy == 1U)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    usart->info->rx_status.rx_busy = 1U;

    RestoreIRQMask(mask);

    // save num of data to be received
    usart->info->xfer.rx_num = num;
    usart->info->xfer.rx_buf = (uint8_t *)data;
    usart->info->xfer.rx_cnt = 0U;

    if((usart->io_mode >> USART_RX_IO_MODE_POS) == DMA_MODE)
    {
        DmaTransferConfig_t dmaRxConfig =  {NULL, NULL,
                                               DMA_FLOW_CONTROL_SOURCE, DMA_ADDRESS_INCREMENT_TARGET,
                                               DMA_DATA_WIDTH_ONE_BYTE, UART_DMA_BURST_SIZE, 0
                                              };
        mask = SaveAndSetIRQMask();

        usart->reg->IER |= USART_IER_RX_START_Msk   | \
                           USART_IER_RX_BREAK_DET_Msk | \
                           USART_IER_RX_FRAME_ERR_Msk | \
                           USART_IER_RX_PARITY_ERR_Msk | \
                           USART_IER_RXFIFO_OF_Msk;

        usart->reg->MCR |= USART_MCR_RX_DMA_EN_Msk;

        usart->reg->FCR0 |= USART_FCR0_RXFIFO_TO_DMA_REQ_EN_Msk;

        RestoreIRQMask(mask);

        dmaRxConfig.sourceAddress = (void*)&(usart->reg->RDR);
        dmaRxConfig.targetAddress = (void*)data;
        dmaRxConfig.totalLength = num;

        // Configure rx DMA and start it
        DMA_transferSetup(usart->dma_rx->instance, usart->dma_rx->channel, &dmaRxConfig);
        DMA_enableChannelInterrupts(usart->dma_rx->instance, usart->dma_rx->channel, (DMA_END_INTERRUPT_ENABLE | DMA_EOR_INTERRUPT_ENABLE));
        DMA_startChannelNoVote(usart->dma_rx->instance, usart->dma_rx->channel);

    }
    else if((usart->io_mode >> USART_RX_IO_MODE_POS) == IRQ_MODE)
    {
        mask = SaveAndSetIRQMask();

        usart->reg->IER |= USART_IER_RX_START_Msk   | \
                           USART_IER_RX_BREAK_DET_Msk | \
                           USART_IER_RX_FRAME_ERR_Msk | \
                           USART_IER_RX_PARITY_ERR_Msk | \
                           USART_IER_RXFIFO_WL_Msk | \
                           USART_IER_RXFIFO_TO_Msk | \
                           USART_IER_RXFIFO_OF_Msk;

        RestoreIRQMask(mask);

    }
    else
    {
        while(usart->info->xfer.rx_cnt < usart->info->xfer.rx_num)
        {
            //wait until receive data is ready
            while((EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, usart->reg->FSR)) == 0);
            //read data
            usart->info->xfer.rx_buf[usart->info->xfer.rx_cnt++] = usart->reg->RDR;
        }
        usart->info->rx_status.rx_busy = 0U;
    }
    return ARM_DRIVER_OK;
}

int32_t USART_Transfer(const void *data_out, void *data_in, uint32_t num,USART_RESOURCES *usart)
{
    //maybe used by command transfer
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

int32_t USART_GetTxCount(USART_RESOURCES *usart)
{
    uint32_t cnt;
    if (!(usart->info->flags & USART_FLAG_CONFIGURED))
        return 0U;
    if(usart->dma_tx)
        cnt = DMA_getChannelCount(usart->dma_tx->instance, usart->dma_tx->channel);
    else
        cnt = usart->info->xfer.tx_cnt;
    return cnt;
}

PLAT_PA_RAMCODE int32_t USART_GetRxCount(USART_RESOURCES *usart)
{
    if (!(usart->info->flags & USART_FLAG_CONFIGURED))
        return 0U;
    return usart->info->xfer.rx_cnt;
}

int32_t USART_Control(uint32_t control, uint32_t arg, USART_RESOURCES *usart)
{
    //uint32_t enr;
    uint32_t lcr = usart->reg->LCR;

    switch (control & ARM_USART_CONTROL_Msk)
    {
        // Control TX
        case ARM_USART_CONTROL_TX:
            return ARM_DRIVER_OK;
        // Control RX
        case ARM_USART_CONTROL_RX:
            return ARM_DRIVER_OK;
        // Control break
        case ARM_USART_CONTROL_BREAK:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        // Abort Send
        case ARM_USART_ABORT_SEND:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        // Abort receive
        case ARM_USART_ABORT_RECEIVE:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        // Abort transfer
        case ARM_USART_ABORT_TRANSFER:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
        case ARM_USART_MODE_ASYNCHRONOUS:
            break;
        // Flush TX fifo
        case ARM_USART_CONTROL_FLUSH_TX:

            USART_WAIT_TX_DONE(usart);

            return ARM_DRIVER_OK;

        case ARM_USART_CONTROL_PURGE_COMM:

            //enr = usart->reg->ENR;
            //usart->reg->ENR = 0;

            //write 1 and dummy read twice and clear
            usart->reg->FCR0 |= USART_FCR0_RXFIFO_FLUSH_Msk;
            (void) usart->reg->FCR0;
            (void) usart->reg->FCR0;
            usart->reg->FCR0 &= ~(USART_FCR0_RXFIFO_FLUSH_Msk);

            // Clear IRQ flag
            //usart->reg->ICR = usart->reg->ISR;
            //usart->reg->ENR = enr;

            return ARM_DRIVER_OK;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    switch (control & ARM_USART_DATA_BITS_Msk)
    {
        case ARM_USART_DATA_BITS_5:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            break;
        case ARM_USART_DATA_BITS_6:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 1U;
            break;
        case ARM_USART_DATA_BITS_7:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 2U;
            break;
        case ARM_USART_DATA_BITS_8:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 3U;
            break;
        default:
            return ARM_USART_ERROR_DATA_BITS;
    }

    // USART Parity
    switch (control & ARM_USART_PARITY_Msk)
    {
        case ARM_USART_PARITY_NONE:
            lcr &= ~USART_LCR_PARITY_EN_Msk;
            break;
        case ARM_USART_PARITY_EVEN:
            lcr |= (USART_LCR_PARITY_EN_Msk | USART_LCR_EVEN_PARITY_Msk);
            break;
        case ARM_USART_PARITY_ODD:
            lcr |= USART_LCR_PARITY_EN_Msk;
            lcr &= ~USART_LCR_EVEN_PARITY_Msk;
            break;
        default:
            return (ARM_USART_ERROR_PARITY);
    }

    // USART Stop bits
    switch (control & ARM_USART_STOP_BITS_Msk)
    {
        case ARM_USART_STOP_BITS_1:
            lcr &=~ USART_LCR_STOP_BIT_NUM_Msk;
            break;
        case ARM_USART_STOP_BITS_1_5:
            lcr &=~ USART_LCR_STOP_BIT_NUM_Msk;
            lcr |= EIGEN_VAL2FLD(USART_LCR_STOP_BIT_NUM, 1);
            break;

        case ARM_USART_STOP_BITS_2:
            lcr &=~ USART_LCR_STOP_BIT_NUM_Msk;
            lcr |= EIGEN_VAL2FLD(USART_LCR_STOP_BIT_NUM, 2);
            break;
        default:
            return ARM_USART_ERROR_STOP_BITS;
    }

    // USART Flow Control
    switch (control & ARM_USART_FLOW_CONTROL_Msk)
    {
        case ARM_USART_FLOW_CONTROL_NONE:
        case ARM_USART_FLOW_CONTROL_RTS:
            // set RTS pin to low
            usart->reg->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            break;
        case ARM_USART_FLOW_CONTROL_CTS:
            usart->reg->MCR |= USART_MCR_AUTO_FLOW_CTS_EN_Msk;
            usart->reg->IER |= USART_IER_CTS_TOGGLE_Msk;
            break;
        case ARM_USART_FLOW_CONTROL_RTS_CTS:
            // set RTS pin to low
            usart->reg->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            usart->reg->MCR |= (USART_MCR_AUTO_FLOW_CTS_EN_Msk);
            usart->reg->IER |= USART_IER_CTS_TOGGLE_Msk;
            break;
    }
    // USART Baudrate
    if(ARM_DRIVER_OK != USART_SetBaudrate (arg, usart))
        return ARM_USART_ERROR_BAUDRATE;

#ifdef PM_FEATURE_ENABLE

#ifdef TYPE_EC718M

    uint32_t instance = USART_GetInstanceNumber(usart);

    if(usart->dma_rx && (usart->info->baudrate > USART_PSRAM_HYBD_VOTE_BAUDRATE_LIMIT))
    {
        USART_LockPsramHybd(instance);
    }
    else
    {
        USART_UnlockPsramHybd(instance);
    }

#endif

#endif

    // Configuration is OK - frame code is valid
    usart->info->frame_code = control;

    usart->reg->LCR = lcr;

    // don't check stopbits, tx use frac div
#if 0
    usart->reg->HCR = USART_HCR_DMA_EOR_MODE_Msk | \
                      USART_HCR_TX_USE_DIV_FRAC_Msk | \
                      EIGEN_VAL2FLD(USART_HCR_AUTO_CG, 0xFF);
#else
    usart->reg->HCR = USART_HCR_DMA_EOR_MODE_Msk | \
                      USART_HCR_TX_USE_DIV_FRAC_Msk | \
                      USART_HCR_RX_STOPBIT_CHK_Msk | \
                      EIGEN_VAL2FLD(USART_HCR_AUTO_CG, 0xFF);
#endif
    // usart enable
    usart->reg->ENR = USART_ENR_TX_EN_Msk | USART_ENR_RX_EN_Msk;

    usart->info->flags |= USART_FLAG_CONFIGURED;
    return ARM_DRIVER_OK;
}

ARM_USART_STATUS USART_GetStatus(USART_RESOURCES *usart)
{
    ARM_USART_STATUS status;

    status.tx_busy          = usart->info->xfer.send_active;
    status.rx_busy          = usart->info->rx_status.rx_busy;
    status.tx_underflow     = 0U;
    status.rx_overflow      = usart->info->rx_status.rx_overflow;
    status.rx_break         = usart->info->rx_status.rx_break;
    status.rx_framing_error = usart->info->rx_status.rx_framing_error;
    status.rx_parity_error  = usart->info->rx_status.rx_parity_error;
    status.is_send_block    = (usart->dma_tx == NULL);
    return status;
}

int32_t USART_SetModemControl(ARM_USART_MODEM_CONTROL control,USART_RESOURCES *usart)
{
    if((usart->info->flags & USART_FLAG_CONFIGURED) == 0U)
    {
        return ARM_DRIVER_ERROR;
    }

    if(usart->info->frame_code & ARM_USART_FLOW_CONTROL_RTS)
    {
        if(control == ARM_USART_RTS_CLEAR) //Deactivate RTS, put rts pin to high level
        {
            usart->reg->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
        }

        if(control == ARM_USART_RTS_SET) //Activate RTS, put rts pin to low level
        {
            usart->reg->FLOWCR |= USART_FLOWCR_RTS_Msk;
        }

    }

    if(control == ARM_USART_DTR_CLEAR)
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    if(control == ARM_USART_DTR_SET)
    {
        return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    return ARM_DRIVER_OK;
}

ARM_USART_MODEM_STATUS USART_GetModemStatus(USART_RESOURCES *usart)
{
    ARM_USART_MODEM_STATUS status = {0};

    if(usart->info->flags & USART_FLAG_CONFIGURED)
    {
        status.cts = !(EIGEN_FLD2VAL(USART_FLOWCR_CTS, usart->reg->FLOWCR));
    }

    return status;
}

PLAT_PA_RAMCODE void USART_IRQHandler (USART_RESOURCES *usart)
{

    uint32_t instance, event;

    uint32_t isr_reg;
    uint32_t i, current_cnt, total_cnt, left_to_recv, bytes_in_fifo;

IRQ_HANDLE:

    event = 0;
    USART_INFO *info = usart->info;

    instance = USART_GetInstanceNumber(usart);

    // Check interrupt source
    isr_reg = usart->reg->ISR;
    usart->reg->ICR = isr_reg;

#ifdef PM_FEATURE_ENABLE

#if USART_DEBUG
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, USART_IRQHandler_0, P_DEBUG, "instance:%d, isr:0x%x, fsr_reg:0x%x, rx_cnt:%d", instance, isr_reg, usart->reg->FSR, info->xfer.rx_cnt);
#endif

#endif

    // RXFIFO overflow
    if (isr_reg & USART_ISR_RXFIFO_OF_Msk)
    {
        info->rx_status.rx_busy = 0;
        info->rx_status.rx_overflow = 1U;
        event |= ARM_USART_EVENT_RX_OVERFLOW;
    }

    // Break detected
    if(isr_reg & USART_ISR_RX_BREAK_DET_Msk)
    {
        info->rx_status.rx_busy = 0;
        info->rx_status.rx_break = 1;
        event |= ARM_USART_EVENT_RX_BREAK;
    }

    // Framing error
    if(isr_reg & USART_ISR_RX_FRAME_ERR_Msk)
    {
        info->rx_status.rx_busy = 0;
        info->rx_status.rx_framing_error = 1;
        event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
    }

    // Parity error
    if(isr_reg & USART_ISR_RX_PARITY_ERR_Msk)
    {
        info->rx_status.rx_busy = 0;
        info->rx_status.rx_parity_error = 1;
        event |= ARM_USART_EVENT_RX_PARITY_ERROR;
    }

    // TX transfer done
    if(isr_reg & USART_ISR_TX_TRANS_DONE_Msk)
    {
        info->xfer.tx_cnt = info->xfer.tx_num;
        info->xfer.send_active = 0U;

        event |= ARM_USART_EVENT_SEND_COMPLETE;

        usart->reg->IER &= ~USART_IER_TX_TRANS_DONE_Msk;
        usart->reg->MCR &= ~USART_MCR_TX_DMA_EN_Msk;

#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(instance, 1, 0);
#endif
    }

    // RX water level
    if(isr_reg & USART_ISR_RXFIFO_WL_Msk)
    {
        current_cnt = info->xfer.rx_cnt;
        total_cnt = info->xfer.rx_num;

        left_to_recv = total_cnt - current_cnt;

        bytes_in_fifo = usart->reg->FSR >> USART_FSR_RXFIFO_WL_Pos;

        // leave at least one byte in fifo to trigger timeout interrupt
        i = bytes_in_fifo - 1;

        if(i == 0)
            i = 1;

        i = MIN(i, left_to_recv);

        while(i--)
        {
            info->xfer.rx_buf[current_cnt++] = usart->reg->RDR;
        }

        // The clear action has no effect when enters irqHandler since the water level holds, let's clear twice here since we've moved data from FIFO
        usart->reg->ICR = USART_ICR_RXFIFO_WL_Msk;

        info->xfer.rx_cnt = current_cnt;

        if(current_cnt == total_cnt)
        {
            // Clear RX busy flag and set receive transfer complete event
            event |= ARM_USART_EVENT_RECEIVE_COMPLETE;

            usart->reg->IER &= ~(USART_IER_RX_START_Msk      | \
                                 USART_IER_RX_BREAK_DET_Msk  | \
                                 USART_IER_RX_FRAME_ERR_Msk  | \
                                 USART_IER_RX_PARITY_ERR_Msk | \
                                 USART_IER_RXFIFO_WL_Msk     | \
                                 USART_IER_RXFIFO_TO_Msk     | \
                                 USART_IER_RXFIFO_OF_Msk);

            info->rx_status.rx_busy = 0U;
        }

    }

    // RX timeout
    if(isr_reg & USART_ISR_RXFIFO_TO_Msk)
    {
        if(usart->reg->IER & USART_IER_AUTOBAUD_DONE_Msk)
        {
            usart->reg->IER &= ~USART_IER_AUTOBAUD_DONE_Msk;
            event |= ARM_USART_EVENT_AUTO_BAUDRATE_DONE;
        }

        current_cnt = info->xfer.rx_cnt;

        total_cnt = info->xfer.rx_num;

        bytes_in_fifo = usart->reg->FSR >> USART_FSR_RXFIFO_WL_Pos;

        left_to_recv = total_cnt - current_cnt;

        i = MIN(bytes_in_fifo, left_to_recv);

        // if still have space to recv
        if(left_to_recv > 0)
        {
            while(i--)
            {
                info->xfer.rx_buf[current_cnt++] = usart->reg->RDR;
            }
        }

        info->xfer.rx_cnt = current_cnt;

        // Check if required amount of data is received
        if (current_cnt == total_cnt)
        {
            // Clear RX busy flag and set receive transfer complete event
            event |= ARM_USART_EVENT_RECEIVE_COMPLETE;

            //Disable RDA interrupt
            usart->reg->IER &= ~(USART_IER_RX_START_Msk      | \
                                 USART_IER_RX_BREAK_DET_Msk  | \
                                 USART_IER_RX_FRAME_ERR_Msk  | \
                                 USART_IER_RX_PARITY_ERR_Msk | \
                                 USART_IER_RXFIFO_WL_Msk     | \
                                 USART_IER_RXFIFO_TO_Msk     | \
                                 USART_IER_RXFIFO_OF_Msk);
        }
        else
        {
            event |= ARM_USART_EVENT_RX_TIMEOUT;
        }

        info->rx_status.rx_busy = 0U;


    }

    // CTS state changed
    if(isr_reg & USART_ISR_CTS_TOGGLE_Msk)
    {
        event |= ARM_USART_EVENT_CTS;
    }

    // autobaud detecting done
    if(isr_reg & USART_ISR_AUTOBAUD_DONE_Msk)
    {
        // backup for sleep restore
        usart->reg->DIVR = usart->reg->DIVIR;
        info->baudrate = (GPR_getClockFreq(g_uartClocks[instance*2+1]) << 4)/ (usart->reg->DIVIR);

#ifdef PM_FEATURE_ENABLE

#ifdef TYPE_EC718M
        if(usart->dma_rx && (info->baudrate > USART_PSRAM_HYBD_VOTE_BAUDRATE_LIMIT))
        {
            USART_LockPsramHybd(instance);
        }
#endif

#endif

    }


    // check fifo number for case where start/end order is reversed(irq is blocked)
    // since dma request level is 16 and burst size is 8, there's always bytes left in fifo
    if(((isr_reg & USART_ISR_RX_START_Msk) == USART_ISR_RX_START_Msk) && ((EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, usart->reg->FSR)) != 0))
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance, 0, 1);
#endif
        info->rx_status.rx_busy = 1;
    }

    if((info->cb_event != NULL) && (event != 0U))
    {
        info->cb_event(event);

#ifdef PM_FEATURE_ENABLE
        if((event & ARM_USART_RX_EVENTS) != 0)
        {
            CHECK_TO_UNLOCK_SLEEP(instance, 0, 1);
        }
#endif

    }

    if(usart->reg->ISR != 0)
    {
#ifdef PM_FEATURE_ENABLE

#if USART_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, USART_IRQHandler_1, P_INFO, "Exit with pending irq, instance:%d, isr:0x%x, fsr_reg:0x%x, rx_cnt:%d", instance, usart->reg->ISR, usart->reg->FSR, info->xfer.rx_cnt);
#endif

#endif
        goto IRQ_HANDLE;
        //XIC_SetPendingIRQ(usart->usart_irq->irq_num);
    }

}

PLAT_PA_RAMCODE void USART_DmaRxEvent(uint32_t event, USART_RESOURCES *usart)
{

#ifdef PM_FEATURE_ENABLE
    uint32_t instance = USART_GetInstanceNumber(usart);
#endif

    uint32_t usartEvent = 0;

    USART_INFO *info = usart->info;

    uint32_t dmaCurrentTargetAddress = DMA_getChannelCurrentTargetAddress(usart->dma_rx->instance, usart->dma_rx->channel, false);

    usart->reg->MCR &= ~USART_MCR_RX_DMA_EN_Msk;

    switch (event)
    {
        case DMA_EVENT_EOR:

            if(usart->reg->IER & USART_IER_AUTOBAUD_DONE_Msk)
            {
                usart->reg->IER &= ~USART_IER_AUTOBAUD_DONE_Msk;
                usartEvent |= ARM_USART_EVENT_AUTO_BAUDRATE_DONE;
            }

            info->xfer.rx_cnt = dmaCurrentTargetAddress - (uint32_t)info->xfer.rx_buf;

            // start next recv
            DMA_startChannelNoVote(usart->dma_rx->instance, usart->dma_rx->channel);
            usart->reg->MCR |= USART_MCR_RX_DMA_EN_Msk;

            info->rx_status.rx_busy = 0;

            if(info->cb_event)
            {
                info->cb_event(usartEvent | ARM_USART_EVENT_RX_TIMEOUT);
            }

#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(instance, 0, 1);
#endif

            break;

        case DMA_EVENT_END:

#if USART_DEBUG
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, USART_DmaRxEvent_1, P_INFO, "uart dma rx end, fsr:%x", usart->reg->FSR);
#endif

            //Disable all recv interrupt
            usart->reg->IER &= ~(USART_IER_RX_START_Msk     | \
                                 USART_IER_RX_BREAK_DET_Msk | \
                                 USART_IER_RX_FRAME_ERR_Msk | \
                                 USART_IER_RXFIFO_OF_Msk    | \
                                 USART_IER_RX_PARITY_ERR_Msk);

            info->rx_status.rx_busy = 0;
            info->xfer.rx_cnt = info->xfer.rx_num;

            if(info->cb_event)
            {
                info->cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
            }

#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(instance, 0, 1);
#endif

            break;
        case DMA_EVENT_ERROR:
        default:
            break;
    }
}


#if (RTE_UART0)
static ARM_USART_CAPABILITIES     USART0_GetCapabilities (void)                                                { return USART_GetCapabilities (&USART0_Resources); }
static int32_t                    USART0_Initialize      (ARM_USART_SignalEvent_t cb_event)                    { return USART_Initialize (cb_event, &USART0_Resources); }
static int32_t                    USART0_Uninitialize    (void)                                                { return USART_Uninitialize (&USART0_Resources); }
static int32_t                    USART0_PowerControl    (ARM_POWER_STATE state)                               { return USART_PowerControl (state, &USART0_Resources); }
static int32_t                    USART0_Send            (const void *data, uint32_t num)                      { return USART_Send (data, num, &USART0_Resources); }
static int32_t                    USART0_Receive         (void *data, uint32_t num)                            { return USART_Receive (data, num, &USART0_Resources); }
static int32_t                    USART0_Transfer        (const void *data_out, void *data_in, uint32_t num)   { return USART_Transfer (data_out, data_in, num, &USART0_Resources); }
static int32_t                    USART0_SendPolling     (const void *data, uint32_t num)                      { return USART_SendPolling (data, num, &USART0_Resources); }
static uint32_t                   USART0_GetTxCount      (void)                                                { return USART_GetTxCount (&USART0_Resources); }
PLAT_PA_RAMCODE static uint32_t   USART0_GetRxCount      (void)                                                { return USART_GetRxCount (&USART0_Resources); }
static uint32_t                   USART0_GetBaudRate     (void)                                                { return USART_GetBaudRate (&USART0_Resources); }
static int32_t                    USART0_Control         (uint32_t control, uint32_t arg)                      { return USART_Control (control, arg, &USART0_Resources); }
static ARM_USART_STATUS           USART0_GetStatus       (void)                                                { return USART_GetStatus (&USART0_Resources); }
static int32_t                    USART0_SetModemControl (ARM_USART_MODEM_CONTROL control)                     { return USART_SetModemControl (control, &USART0_Resources); }
static ARM_USART_MODEM_STATUS     USART0_GetModemStatus  (void)                                                { return USART_GetModemStatus (&USART0_Resources); }
PLAT_PA_RAMCODE       void        USART0_IRQHandler      (void)                                                {        USART_IRQHandler (&USART0_Resources); }


#if (RTE_UART0_RX_IO_MODE == DMA_MODE)
      void USART0_DmaRxEvent(uint32_t event) { USART_DmaRxEvent(event, &USART0_Resources);}
#endif

ARM_DRIVER_USART Driver_USART0 = {
    ARM_USART_GetVersion,
    USART0_GetCapabilities,
    USART0_Initialize,
    USART0_Uninitialize,
    USART0_PowerControl,
    USART0_Send,
    USART0_Receive,
    USART0_Transfer,
    USART0_GetTxCount,
    USART0_GetRxCount,
    USART0_Control,
    USART0_GetStatus,
    USART0_SetModemControl,
    USART0_GetModemStatus,
    USART0_GetBaudRate,
    USART0_SendPolling
};

#endif

#if (RTE_UART1)
static ARM_USART_CAPABILITIES     USART1_GetCapabilities (void)                                                { return USART_GetCapabilities (&USART1_Resources); }
static int32_t                    USART1_Initialize      (ARM_USART_SignalEvent_t cb_event)                    { return USART_Initialize (cb_event, &USART1_Resources); }
static int32_t                    USART1_Uninitialize    (void)                                                { return USART_Uninitialize (&USART1_Resources); }
static int32_t                    USART1_PowerControl    (ARM_POWER_STATE state)                               { return USART_PowerControl (state, &USART1_Resources); }
static int32_t                    USART1_Send            (const void *data, uint32_t num)                      { return USART_Send (data, num, &USART1_Resources); }
static int32_t                    USART1_Receive         (void *data, uint32_t num)                            { return USART_Receive (data, num, &USART1_Resources); }
static int32_t                    USART1_Transfer        (const void *data_out, void *data_in, uint32_t num)   { return USART_Transfer (data_out, data_in, num, &USART1_Resources); }
static int32_t                    USART1_SendPolling     (const void *data, uint32_t num)                      { return USART_SendPolling (data, num, &USART1_Resources); }
static uint32_t                   USART1_GetTxCount      (void)                                                { return USART_GetTxCount (&USART1_Resources); }
PLAT_PA_RAMCODE static uint32_t   USART1_GetRxCount      (void)                                                { return USART_GetRxCount (&USART1_Resources); }
static uint32_t                   USART1_GetBaudRate     (void)                                                { return USART_GetBaudRate (&USART1_Resources); }
static int32_t                    USART1_Control         (uint32_t control, uint32_t arg)                      { return USART_Control (control, arg, &USART1_Resources); }
static ARM_USART_STATUS           USART1_GetStatus       (void)                                                { return USART_GetStatus (&USART1_Resources); }
static int32_t                    USART1_SetModemControl (ARM_USART_MODEM_CONTROL control)                     { return USART_SetModemControl (control, &USART1_Resources); }
static ARM_USART_MODEM_STATUS     USART1_GetModemStatus  (void)                                                { return USART_GetModemStatus (&USART1_Resources); }
PLAT_PA_RAMCODE       void        USART1_IRQHandler      (void)                                                {        USART_IRQHandler (&USART1_Resources); }

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)
      void USART1_DmaRxEvent(uint32_t event) { USART_DmaRxEvent(event, &USART1_Resources);}
#endif


ARM_DRIVER_USART Driver_USART1 = {
    ARM_USART_GetVersion,
    USART1_GetCapabilities,
    USART1_Initialize,
    USART1_Uninitialize,
    USART1_PowerControl,
    USART1_Send,
    USART1_Receive,
    USART1_Transfer,
    USART1_GetTxCount,
    USART1_GetRxCount,
    USART1_Control,
    USART1_GetStatus,
    USART1_SetModemControl,
    USART1_GetModemStatus,
    USART1_GetBaudRate,
    USART1_SendPolling
};

#endif

#if (RTE_UART2)
static ARM_USART_CAPABILITIES     USART2_GetCapabilities (void)                                                { return USART_GetCapabilities (&USART2_Resources); }
static int32_t                    USART2_Initialize      (ARM_USART_SignalEvent_t cb_event)                    { return USART_Initialize (cb_event, &USART2_Resources); }
static int32_t                    USART2_Uninitialize    (void)                                                { return USART_Uninitialize (&USART2_Resources); }
static int32_t                    USART2_PowerControl    (ARM_POWER_STATE state)                               { return USART_PowerControl (state, &USART2_Resources); }
static int32_t                    USART2_Send            (const void *data, uint32_t num)                      { return USART_Send (data, num, &USART2_Resources); }
static int32_t                    USART2_Receive         (void *data, uint32_t num)                            { return USART_Receive (data, num, &USART2_Resources); }
static int32_t                    USART2_Transfer        (const void *data_out, void *data_in, uint32_t num)   { return USART_Transfer (data_out, data_in, num, &USART2_Resources); }
static int32_t                    USART2_SendPolling     (const void *data, uint32_t num)                      { return USART_SendPolling (data, num, &USART2_Resources); }
static uint32_t                   USART2_GetTxCount      (void)                                                { return USART_GetTxCount (&USART2_Resources); }
PLAT_PA_RAMCODE static uint32_t   USART2_GetRxCount      (void)                                                { return USART_GetRxCount (&USART2_Resources); }
static uint32_t                   USART2_GetBaudRate     (void)                                                { return USART_GetBaudRate (&USART2_Resources); }
static int32_t                    USART2_Control         (uint32_t control, uint32_t arg)                      { return USART_Control (control, arg, &USART2_Resources); }
static ARM_USART_STATUS           USART2_GetStatus       (void)                                                { return USART_GetStatus (&USART2_Resources); }
static int32_t                    USART2_SetModemControl (ARM_USART_MODEM_CONTROL control)                     { return USART_SetModemControl (control, &USART2_Resources); }
static ARM_USART_MODEM_STATUS     USART2_GetModemStatus  (void)                                                { return USART_GetModemStatus (&USART2_Resources); }
PLAT_PA_RAMCODE       void        USART2_IRQHandler      (void)                                                {        USART_IRQHandler (&USART2_Resources); }

#if (RTE_UART2_RX_IO_MODE == DMA_MODE)
      void USART2_DmaRxEvent(uint32_t event) { USART_DmaRxEvent(event, &USART2_Resources);}
#endif


ARM_DRIVER_USART Driver_USART2 = {
    ARM_USART_GetVersion,
    USART2_GetCapabilities,
    USART2_Initialize,
    USART2_Uninitialize,
    USART2_PowerControl,
    USART2_Send,
    USART2_Receive,
    USART2_Transfer,
    USART2_GetTxCount,
    USART2_GetRxCount,
    USART2_Control,
    USART2_GetStatus,
    USART2_SetModemControl,
    USART2_GetModemStatus,
    USART2_GetBaudRate,
    USART2_SendPolling
};

#endif

#if (RTE_UART3)
static ARM_USART_CAPABILITIES     USART3_GetCapabilities (void)                                                { return USART_GetCapabilities (&USART3_Resources); }
static int32_t                    USART3_Initialize      (ARM_USART_SignalEvent_t cb_event)                    { return USART_Initialize (cb_event, &USART3_Resources); }
static int32_t                    USART3_Uninitialize    (void)                                                { return USART_Uninitialize (&USART3_Resources); }
static int32_t                    USART3_PowerControl    (ARM_POWER_STATE state)                               { return USART_PowerControl (state, &USART3_Resources); }
static int32_t                    USART3_Send            (const void *data, uint32_t num)                      { return USART_Send (data, num, &USART3_Resources); }
static int32_t                    USART3_Receive         (void *data, uint32_t num)                            { return USART_Receive (data, num, &USART3_Resources); }
static int32_t                    USART3_Transfer        (const void *data_out, void *data_in, uint32_t num)   { return USART_Transfer (data_out, data_in, num, &USART3_Resources); }
static int32_t                    USART3_SendPolling     (const void *data, uint32_t num)                      { return USART_SendPolling (data, num, &USART3_Resources); }
static uint32_t                   USART3_GetTxCount      (void)                                                { return USART_GetTxCount (&USART3_Resources); }
PLAT_PA_RAMCODE static uint32_t   USART3_GetRxCount      (void)                                                { return USART_GetRxCount (&USART3_Resources); }
static uint32_t                   USART3_GetBaudRate     (void)                                                { return USART_GetBaudRate (&USART3_Resources); }
static int32_t                    USART3_Control         (uint32_t control, uint32_t arg)                      { return USART_Control (control, arg, &USART3_Resources); }
static ARM_USART_STATUS           USART3_GetStatus       (void)                                                { return USART_GetStatus (&USART3_Resources); }
static int32_t                    USART3_SetModemControl (ARM_USART_MODEM_CONTROL control)                     { return USART_SetModemControl (control, &USART3_Resources); }
static ARM_USART_MODEM_STATUS     USART3_GetModemStatus  (void)                                                { return USART_GetModemStatus (&USART3_Resources); }
PLAT_PA_RAMCODE       void        USART3_IRQHandler      (void)                                                {        USART_IRQHandler (&USART3_Resources); }

#if (RTE_UART3_RX_IO_MODE == DMA_MODE)
      void USART3_DmaRxEvent(uint32_t event) { USART_DmaRxEvent(event, &USART3_Resources);}
#endif


ARM_DRIVER_USART Driver_USART3 = {
    ARM_USART_GetVersion,
    USART3_GetCapabilities,
    USART3_Initialize,
    USART3_Uninitialize,
    USART3_PowerControl,
    USART3_Send,
    USART3_Receive,
    USART3_Transfer,
    USART3_GetTxCount,
    USART3_GetRxCount,
    USART3_Control,
    USART3_GetStatus,
    USART3_SetModemControl,
    USART3_GetModemStatus,
    USART3_GetBaudRate,
    USART3_SendPolling
};

#endif

#endif
