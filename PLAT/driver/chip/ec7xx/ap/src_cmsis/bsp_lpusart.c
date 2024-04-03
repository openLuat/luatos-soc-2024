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
#include "bsp_lpusart.h"
#include "slpman.h"
#ifdef __USER_CODE__
#else
#ifdef PM_FEATURE_ENABLE

#include DEBUG_LOG_HEADER_FILE
#define LPUSART_DRIVER_DEBUG  1

#endif

#if 0
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

#define BSP_LPUSART_TEXT_SECTION     SECTION_DEF_IMPL(.sect_bsp_lpusart_text)
#define BSP_LPUSART_RODATA_SECTION   SECTION_DEF_IMPL(.sect_bsp_lpusart_rodata)
#define BSP_LPUSART_DATA_SECTION     SECTION_DEF_IMPL(.sect_bsp_lpusart_data)
#define BSP_LPUSART_BSS_SECTION      SECTION_DEF_IMPL(.sect_bsp_lpusart_bss)

#define ARM_LPUSART_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0)  /* driver version */

#if (!RTE_UART1)
#error "Cooperating UART not enabled in RTE_Device.h!"
#endif

#define LPUSART_TX_TRIG_LVL      (8)
#define LPUSART_RX_TRIG_LVL      (16)

#ifdef PM_FEATURE_ENABLE
/** \brief Internal used data structure */
typedef struct _lpusart_database
{
    uint8_t                            isInited;            /**< Whether usart has been initialized */
    uint8_t                            reserved0;
    uint16_t                           reserved1;
    struct
    {
        uint32_t ENR;                           /**< Usart Enable Register,              offset: 0x0*/
        uint32_t DIVR;                          /**< Usart Divder Register,              offset: 0x4*/
        uint32_t LCR;                           /**< Usart Line Control Register,        offset: 0x8*/
        uint32_t RXSR;                          /**< Usart Rx Select Register,           offset: 0xC*/
        uint32_t FCR0;                          /**< Usart Fifo Control 0 Register,      offset: 0x10*/
        uint32_t MCR;                           /**< Usart Mode Control Register,        offset: 0x14*/
        uint32_t IER;                           /**< Usart Interrupt Enable Register,    offset: 0x28*/
        uint32_t FLOWCR;                        /**< Usart Flow Control Register,        offset: 0x34*/
        uint32_t HCR;                           /**< Usart Hw Control Register,          offset: 0x40*/
        uint32_t FCR1;                          /**< Usart Fifo Control 1 Register,      offset: 0x58*/
    } core_registers;
} lpusart_database_t;

BSP_LPUSART_BSS_SECTION static lpusart_database_t g_lpusartDataBase = {0};

#endif

#ifdef PM_FEATURE_ENABLE
/**
  \brief Bitmap of LPUSART working status, msb acts as mask*/
BSP_LPUSART_DATA_SECTION static uint32_t g_lpusartWorkingStatus = 0x80000000;

/**
  \fn        static void LPUSART_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LPUSART hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void LPUSART_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    switch (state)
    {
        case SLPMAN_IDLE_STATE:

            if((g_lpusartDataBase.isInited == true) && (LPUSART_CORE->MCR & USART_MCR_RX_DMA_EN_Msk))
            {
                LPUSART_CORE->IER |= USART_IER_RXFIFO_WL_Msk;
            }
            break;

        case SLPMAN_SLEEP1_STATE:

            if(g_lpusartDataBase.isInited == true)
            {
                g_lpusartDataBase.core_registers.ENR = LPUSART_CORE->ENR;
                g_lpusartDataBase.core_registers.DIVR = LPUSART_CORE->DIVR;
                g_lpusartDataBase.core_registers.LCR = LPUSART_CORE->LCR;
                g_lpusartDataBase.core_registers.RXSR = LPUSART_CORE->RXSR;
                g_lpusartDataBase.core_registers.FCR0 = LPUSART_CORE->FCR0;
                g_lpusartDataBase.core_registers.FCR1 = LPUSART_CORE->FCR1;
                g_lpusartDataBase.core_registers.MCR = LPUSART_CORE->MCR;
                g_lpusartDataBase.core_registers.IER = LPUSART_CORE->IER;
                g_lpusartDataBase.core_registers.FLOWCR = LPUSART_CORE->FLOWCR;
                g_lpusartDataBase.core_registers.HCR = LPUSART_CORE->HCR;
            }
            break;
        default:
            break;
    }

}

extern bool apmuGetSleepedFlag(void);

/**
  \fn        static void LPUSART_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LPUSART hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state

 */
static void LPUSART_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    switch (state)
    {
        case SLPMAN_IDLE_STATE:

            if((g_lpusartDataBase.isInited == true) && (LPUSART_CORE->MCR & USART_MCR_RX_DMA_EN_Msk))
            {
                LPUSART_CORE->IER &= ~USART_IER_RXFIFO_WL_Msk;
                LPUSART_CORE->ICR = USART_ICR_RXFIFO_WL_Msk;
            }
            break;

        case SLPMAN_SLEEP1_STATE:

            // no need to restore if failing to sleep

            if(apmuGetSleepedFlag() == false)
            {
                break;
            }

            if(g_lpusartDataBase.isInited == true)
            {
                GPR_clockEnable(PCLK_UART1);
                GPR_clockEnable(FCLK_UART1);

                LPUSART_CORE->DIVR = g_lpusartDataBase.core_registers.DIVR;
                LPUSART_CORE->LCR = g_lpusartDataBase.core_registers.LCR;
                LPUSART_CORE->RXSR = g_lpusartDataBase.core_registers.RXSR;
                LPUSART_CORE->FCR0 = g_lpusartDataBase.core_registers.FCR0;
                LPUSART_CORE->FCR1 = g_lpusartDataBase.core_registers.FCR1;

                LPUSART_CORE->IER = g_lpusartDataBase.core_registers.IER;

                LPUSART_CORE->FLOWCR = g_lpusartDataBase.core_registers.FLOWCR;
                LPUSART_CORE->HCR = g_lpusartDataBase.core_registers.HCR;

                LPUSART_CORE->ENR = g_lpusartDataBase.core_registers.ENR;
                LPUSART_CORE->MCR = g_lpusartDataBase.core_registers.MCR;

                if(LPUSART_CORE->IER & USART_IER_AUTOBAUD_DONE_Msk)
                {
                    LPUSART_CORE->ABSR = 1;
                }
                else
                {
                    LPUSART_CORE->TCR = USART_TCR_TOCNT_SWTRG_Msk;
                }
             }

            break;

        default:
            break;
    }

}

#define  LOCK_SLEEP(tx, rx)               do                                                                       \
                                          {                                                                        \
                                              if((g_lpusartWorkingStatus & 0x80000000) == 0)                       \
                                              {                                                                    \
                                                  g_lpusartWorkingStatus |= (rx);                                  \
                                                  g_lpusartWorkingStatus |= (tx << 1);                             \
                                                  slpManDrvVoteSleep(SLP_VOTE_LPUSART, SLP_ACTIVE_STATE);          \
                                              }                                                                    \
                                          }                                                                        \
                                          while(0)

#define  CHECK_TO_UNLOCK_SLEEP(tx, rx)                do                                                            \
                                                      {                                                             \
                                                          g_lpusartWorkingStatus &= ~(rx);                          \
                                                          g_lpusartWorkingStatus &= ~(tx << 1);                     \
                                                          if((g_lpusartWorkingStatus & 0xFF) == 0)                  \
                                                          {                                                         \
                                                              NVIC_ClearPendingIRQ(LpuartWakeup_IRQn);              \
                                                              slpManDrvVoteSleep(SLP_VOTE_LPUSART, SLP_SLP1_STATE); \
                                                          }                                                         \
                                                      }                                                             \
                                                      while(0)
#endif

#define USART_WAIT_TX_DONE(lpusart)   do                                                                                 \
                                      {                                                                                  \
                                         while(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, lpusart->core_regs->FSR) != 0);        \
                                         while(UART_readLSR(&(lpusart->core_regs->LSR)) & USART_LSR_TX_BUSY_Msk);                         \
                                      } while(0)

// declearation for DMA API
extern uint32_t DMA_getChannelCurrentTargetAddress(DmaInstance_e instance, uint32_t channel, bool sync);
extern void DMA_startChannelNoVote(DmaInstance_e instance, uint32_t channel);

// Driver Version
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_USART_API_VERSION,
    ARM_LPUSART_DRV_VERSION
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

void LPUSART1_IRQHandler(void);

#if (RTE_UART1)

BSP_LPUSART_BSS_SECTION static LPUSART_INFO    LPUSART1_Info  = { 0 };
static const PIN LPUSART1_pin_tx  = {RTE_UART1_TX_BIT,   RTE_UART1_TX_FUNC};
static const PIN LPUSART1_pin_rx  = {RTE_UART1_RX_BIT,   RTE_UART1_RX_FUNC};
#if (RTE_UART1_CTS_PIN_EN == 1)
static const PIN LPUSART1_pin_cts  = {RTE_UART1_CTS_BIT,   RTE_UART1_CTS_FUNC};
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
static const PIN LPUSART1_pin_rts  = {RTE_UART1_RTS_BIT,   RTE_UART1_RTS_FUNC};
#endif

#if (RTE_UART1_TX_IO_MODE == DMA_MODE)

void LPUSART1_DmaTxEvent(uint32_t event);
BSP_LPUSART_DATA_SECTION static LPUSART_TX_DMA LPUSART1_DMA_Tx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART1_DMA_TX_REQID,
                                    NULL
                                 };
#endif

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)

void LPUSART1_DmaRxEvent(uint32_t event);

BSP_LPUSART_DATA_SECTION static LPUSART_RX_DMA LPUSART1_DMA_Rx = {
                                    DMA_INSTANCE_MP,
                                    -1,
                                    RTE_UART1_DMA_RX_REQID,
                                    LPUSART1_DmaRxEvent
                                 };

#endif

#if (RTE_UART1_TX_IO_MODE != POLLING_MODE) || (RTE_UART1_RX_IO_MODE != POLLING_MODE)
BSP_LPUSART_DATA_SECTION static LPUSART_IRQ LPUSART1_IRQ = {
                                PXIC0_UART1_IRQn,
                                LPUSART1_IRQHandler
                              };
#endif

static const LPUSART_RESOURCES LPUSART1_Resources = {
    LPUSART_AON,
    LPUSART_CORE,
    {
      &LPUSART1_pin_tx,
      &LPUSART1_pin_rx,
#if (RTE_UART1_CTS_PIN_EN == 1)
      &LPUSART1_pin_cts,
#else
      NULL,
#endif
#if (RTE_UART1_RTS_PIN_EN == 1)
      &LPUSART1_pin_rts,
#else
      NULL,
#endif
    },

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)
    &LPUSART1_DMA_Rx,
#else
    NULL,
#endif

#if (RTE_UART1_TX_IO_MODE == DMA_MODE)
    &LPUSART1_DMA_Tx,
#else
    NULL,
#endif

#if (RTE_UART1_TX_IO_MODE != POLLING_MODE) || (RTE_UART1_RX_IO_MODE != POLLING_MODE)
    &LPUSART1_IRQ,
#else
    NULL,
#endif
    &LPUSART1_Info
};
#endif


BSP_LPUSART_DATA_SECTION static DmaTransferConfig_t dmaTxConfig = {NULL, NULL,
                                       DMA_FLOW_CONTROL_TARGET, DMA_ADDRESS_INCREMENT_SOURCE,
                                       DMA_DATA_WIDTH_ONE_BYTE, DMA_BURST_16_BYTES, 0
                                      };

BSP_LPUSART_DATA_SECTION static DmaTransferConfig_t dmaRxConfig =  {NULL, NULL,
                                       DMA_FLOW_CONTROL_SOURCE, DMA_ADDRESS_INCREMENT_TARGET,
                                       DMA_DATA_WIDTH_ONE_BYTE, DMA_BURST_8_BYTES, 0
                                      };

ARM_DRIVER_VERSION LPUSART_GetVersion(void)
{
    return DriverVersion;
}

ARM_USART_CAPABILITIES LPUSART_GetCapabilities(void)
{
    return DriverCapabilities;
}

int32_t LPUSART_SetBaudrate(uint32_t baudrate, uint32_t aon_lcr, LPUSART_RESOURCES *lpusart)
{
    uint32_t uart_clock = 0;
    uint32_t div;

    // Enable lpuart and uart autobaud, if lpuart is enabled, received data is forced to push into lpuart fifo even uart is enabled at the same time
    if(baudrate == 0)
    {
        lpusart->aon_regs->CR0 = LPUSARTAON_CR0_RX_ENABLE_Msk | LPUSARTAON_CR0_CLK_ENABLE_Msk;
        lpusart->aon_regs->LCR = aon_lcr;
        lpusart->aon_regs->CR1 = LPUSARTAON_CR1_AUTO_BAUD_Msk;
        lpusart->aon_regs->CR1 |= LPUSARTAON_CR1_ENABLE_Msk;

        lpusart->core_regs->RXSR = 1;
        lpusart->core_regs->ENR = USART_ENR_TX_EN_Msk | USART_ENR_RX_EN_Msk;
        lpusart->core_regs->ABSR = 1;
        lpusart->core_regs->MCR &= ~USART_MCR_RX_DMA_EN_Msk;
        lpusart->core_regs->IER = USART_IER_AUTOBAUD_DONE_Msk;

        return ARM_DRIVER_OK;
    }

    if(lpusart->info->flags & LPUSART_FLAG_POWER_LOW)
    {
        if((baudrate < LPUSART_MIN_BAUDRATE) || (baudrate > LPUSART_MAX_BAUDRATE))
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }

        div = ((32768 << 5) + (baudrate >> 1)) / baudrate;

        if(((UART_readLSR(&(lpusart->core_regs->LSR)) & (USART_LSR_LUAC_RX_BUSY_Msk | USART_LSR_LUAC_RXFIFO_EMPTY_Msk | USART_LSR_RX_BUSY_Msk)) == USART_LSR_LUAC_RXFIFO_EMPTY_Msk) &&
            (EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, lpusart->core_regs->FSR)  == 0))
        {
            lpusart->aon_regs->CR0 = LPUSARTAON_CR0_RX_ENABLE_Msk | LPUSARTAON_CR0_CLK_ENABLE_Msk;
            lpusart->aon_regs->DLR = div;
            lpusart->aon_regs->LCR = aon_lcr;
            //lpusart->aon_regs->CR1 = LPUSARTAON_CR1_ENABLE_Msk | LPUSARTAON_CR1_ACG_EN_Msk | LPUSARTAON_CR1_AUTO_ADJ_Msk;
            lpusart->aon_regs->CR1 = LPUSARTAON_CR1_ENABLE_Msk | LPUSARTAON_CR1_ACG_EN_Msk;
        }
    }

    uart_clock = GPR_getClockFreq(FCLK_UART1);

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
    lpusart->core_regs->ENR = 0;
    lpusart->core_regs->DIVR = div;

#ifdef PM_FEATURE_ENABLE
    // backup setting
    g_lpusartDataBase.core_registers.DIVR = div;
#endif

    if(lpusart->info->flags & LPUSART_FLAG_POWER_FULL)
    {
        lpusart->core_regs->ENR = USART_ENR_TX_EN_Msk | USART_ENR_RX_EN_Msk;
    }
    else
    {
        lpusart->core_regs->ENR = USART_ENR_TX_EN_Msk;
    }

    lpusart->info->baudrate = baudrate;

    return ARM_DRIVER_OK;
}

uint32_t LPUSART_GetBaudRate(LPUSART_RESOURCES *lpusart)
{
    return lpusart->info->baudrate;
}

/*
 * Check whether rx is ongoing, return true if rx is ongoing at this moment, otherwise false
 */
bool LPUSART_IsRxActive(void)
{
#ifdef PM_FEATURE_ENABLE

    if((LPUSART_CORE->RXSR == 1) && ((LPUSART_CORE->IER & USART_IER_AUTOBAUD_DONE_Msk) == 0))
    {
        return !(((UART_readLSR(&(LPUSART_CORE->LSR)) & (USART_LSR_LUAC_RX_BUSY_Msk | USART_LSR_LUAC_RXFIFO_EMPTY_Msk | USART_LSR_RX_BUSY_Msk)) == USART_LSR_LUAC_RXFIFO_EMPTY_Msk) &&
                 (EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, LPUSART_CORE->FSR)  == 0));
    }
    else
    {
        return 0;
    }
#else
    return 0;
#endif
}

PLAT_PA_RAMCODE void LPUSART_ClearStopFlag(void)
{
#ifdef PM_FEATURE_ENABLE
    if((LPUSART_CORE->RXSR == 1) && ((LPUSART_CORE->IER & USART_IER_AUTOBAUD_DONE_Msk) == 0))
    {
        LPUSART_AON->SCR = LPUSARTAON_SCR_STOP_SW_CLR_Msk;
        while(UART_readLSR(&(LPUSART_CORE->LSR)) & USART_LSR_LUAC_STOPFLG_Msk);
        LPUSART_AON->SCR = 0;
    }
#endif
}

void LPUSART_SetStopFlag(void)
{
#ifdef PM_FEATURE_ENABLE
    if((LPUSART_CORE->RXSR == 1) && ((LPUSART_CORE->IER & USART_IER_AUTOBAUD_DONE_Msk) == 0))
    {
        LPUSART_AON->SCR = LPUSARTAON_SCR_STOP_SW_SET_Msk;
        while((UART_readLSR(&(LPUSART_CORE->LSR)) & USART_LSR_LUAC_STOPFLG_Msk) == 0);
        LPUSART_AON->SCR = 0;
    }
#endif
}

int32_t LPUSART_Initialize(ARM_USART_SignalEvent_t cb_event, LPUSART_RESOURCES *lpusart)
{
    int32_t returnCode;

    if (lpusart->info->flags & LPUSART_FLAG_INITIALIZED)
        return ARM_DRIVER_OK;

    // Pin initialize
    PadConfig_t config;
    PAD_getDefaultConfig(&config);

    config.mux = lpusart->pins.pin_tx->funcNum;
    PAD_setPinConfig(lpusart->pins.pin_tx->pinNum, &config);

    config.pullSelect = PAD_PULL_INTERNAL;
    config.pullUpEnable = PAD_PULL_UP_ENABLE;
    config.pullDownEnable = PAD_PULL_DOWN_DISABLE;
    config.mux = lpusart->pins.pin_rx->funcNum;
    PAD_setPinConfig(lpusart->pins.pin_rx->pinNum, &config);

    if(lpusart->pins.pin_cts)
    {
        config.mux = lpusart->pins.pin_cts->funcNum;
        PAD_setPinConfig(lpusart->pins.pin_cts->pinNum, &config);
    }

    if(lpusart->pins.pin_rts)
    {
        config.mux = lpusart->pins.pin_rts->funcNum;
        PAD_setPinConfig(lpusart->pins.pin_rts->pinNum, &config);
    }
#ifdef PM_FEATURE_ENABLE
    g_lpusartDataBase.isInited = true;
#endif

    // Initialize LPUSART run-time resources
    lpusart->info->cb_event = cb_event;
    memset(&(lpusart->info->rx_status), 0, sizeof(LPUSART_STATUS));

    lpusart->info->xfer.send_active           = 0;
    lpusart->info->xfer.tx_def_val            = 0;

    if(lpusart->dma_tx)
    {
        returnCode = DMA_openChannel(lpusart->dma_tx->instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            lpusart->dma_tx->channel = returnCode;

        DMA_setChannelRequestSource(lpusart->dma_tx->instance, lpusart->dma_tx->channel, (DmaRequestSource_e)lpusart->dma_tx->request);
        DMA_rigisterChannelCallback(lpusart->dma_tx->instance, lpusart->dma_tx->channel, lpusart->dma_tx->callback);
    }

    if(lpusart->dma_rx)
    {
        returnCode = DMA_openChannel(lpusart->dma_rx->instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            lpusart->dma_rx->channel = returnCode;

        DMA_setChannelRequestSource(lpusart->dma_rx->instance, lpusart->dma_rx->channel, (DmaRequestSource_e)lpusart->dma_rx->request);
        DMA_rigisterChannelCallback(lpusart->dma_rx->instance, lpusart->dma_rx->channel, lpusart->dma_rx->callback);
    }

    lpusart->info->flags = LPUSART_FLAG_INITIALIZED;  // LPUSART is initialized

#ifdef PM_FEATURE_ENABLE
    g_lpusartWorkingStatus = 0;
    slpManRegisterPredefinedBackupCb(SLP_CALLBACK_LPUSART_MODULE, LPUSART_EnterLowPowerStatePrepare, NULL);
    slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_LPUSART_MODULE, LPUSART_ExitLowPowerStateRestore, NULL);
#endif
    return ARM_DRIVER_OK;
}

int32_t LPUSART_Uninitialize(LPUSART_RESOURCES *lpusart)
{
    lpusart->info->flags = 0;
    lpusart->info->cb_event = NULL;

    if(lpusart->dma_tx)
    {
        DMA_closeChannel(lpusart->dma_tx->instance, lpusart->dma_tx->channel);
    }

    if(lpusart->dma_rx)
    {
        DMA_closeChannel(lpusart->dma_rx->instance, lpusart->dma_rx->channel);
    }

#ifdef PM_FEATURE_ENABLE
    CHECK_TO_UNLOCK_SLEEP(1, 1);

    g_lpusartDataBase.isInited = false;
    g_lpusartWorkingStatus = 0x80000000;
    slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_LPUSART_MODULE);
    slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_LPUSART_MODULE);
#endif

    return ARM_DRIVER_OK;
}

int32_t LPUSART_PowerControl(ARM_POWER_STATE state,LPUSART_RESOURCES *lpusart)
{
    uint32_t val = 0;

    switch (state)
    {
        case ARM_POWER_OFF:

            // Reset LPUSART
            GPR_swReset(RST_PCLK_UART1);
            GPR_swReset(RST_FCLK_UART1);

            // LPUART AON part is enabled
            if(lpusart->aon_regs->CR1 & LPUSARTAON_CR1_ENABLE_Msk)
            {
                GPR_swReset(RST_LPUA);
                lpusart->aon_regs->CR1 = 0;
            }

            lpusart->aon_regs->CR0 = 0;

            // DMA disable
            if(lpusart->dma_tx)
                DMA_stopChannel(lpusart->dma_tx->instance, lpusart->dma_tx->channel, false);

            if(lpusart->dma_rx)
                DMA_stopChannel(lpusart->dma_rx->instance, lpusart->dma_rx->channel, false);

            // Disable clock
            GPR_clockDisable(PCLK_UART1);
            GPR_clockDisable(FCLK_UART1);

            // Clear driver variables
            memset(&(lpusart->info->rx_status), 0, sizeof(LPUSART_STATUS));
            lpusart->info->frame_code       = 0;
            lpusart->info->xfer.send_active = 0;

            // Disable LPUSART IRQ
            if(lpusart->irq)
            {
                XIC_ClearPendingIRQ(lpusart->irq->irq_num);
                XIC_DisableIRQ(lpusart->irq->irq_num);
            }

            lpusart->info->flags &= ~(LPUSART_FLAG_POWER_MSK | LPUSART_FLAG_CONFIGURED);

            break;

        case ARM_POWER_LOW:
            if((lpusart->info->flags & LPUSART_FLAG_INITIALIZED) == 0)
            {
                return ARM_DRIVER_ERROR;
            }

            if(lpusart->info->flags & LPUSART_FLAG_POWER_LOW)
            {
                return ARM_DRIVER_OK;
            }

            // Enable lpusart clock
            GPR_clockEnable(PCLK_UART1);
            GPR_clockEnable(FCLK_UART1);

            // Disable interrupts
            lpusart->core_regs->IER = 0;
            lpusart->core_regs->ICR = lpusart->core_regs->ISR;

            // Clear driver variables
            memset(&(lpusart->info->rx_status), 0, sizeof(LPUSART_STATUS));
            lpusart->info->frame_code       = 0;
            lpusart->info->xfer.send_active = 0;

            // Configure FIFO Control register
            val = EIGEN_VAL2FLD(USART_FCR0_TXFIFO_TH, LPUSART_TX_TRIG_LVL) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TH, LPUSART_RX_TRIG_LVL - 1) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TO_BIT_NUM, 16);

            lpusart->core_regs->FCR0 = val;

            if(lpusart->dma_rx)
            {
                // Set rx lvl to 1 to wake up from doze
                lpusart->core_regs->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, LPUSART_TX_TRIG_LVL) | \
                                           EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, 0);
            }
            else
            {
                lpusart->core_regs->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, LPUSART_TX_TRIG_LVL) | \
                                           EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, LPUSART_RX_TRIG_LVL - 1);
            }

            if(lpusart->irq)
            {
                XIC_SetVector(lpusart->irq->irq_num, lpusart->irq->cb_irq);
                XIC_EnableIRQ(lpusart->irq->irq_num);
                XIC_SuppressOvfIRQ(lpusart->irq->irq_num);
            }

            // rx mux select to lpuart rxfifo
            lpusart->core_regs->RXSR = 1;

            lpusart->info->flags |= LPUSART_FLAG_POWER_LOW;  // LPUSART is powered on

            break;

        case ARM_POWER_FULL:
            if((lpusart->info->flags & LPUSART_FLAG_INITIALIZED) == 0)
            {
                return ARM_DRIVER_ERROR;
            }
            if(lpusart->info->flags & LPUSART_FLAG_POWER_FULL)
            {
                return ARM_DRIVER_OK;
            }

            GPR_clockEnable(PCLK_AON);
            GPR_clockEnable(PCLK_UART1);
            GPR_clockEnable(FCLK_UART1);

            GPR_swResetModule(RESET_VECTOR_PTR(UART1_RESET_VECTOR));

            // Clear driver variables
            memset(&(lpusart->info->rx_status), 0, sizeof(LPUSART_STATUS));
            lpusart->info->frame_code       = 0;
            lpusart->info->xfer.send_active = 0;

            // Configure FIFO Control register
            val = EIGEN_VAL2FLD(USART_FCR0_TXFIFO_TH, LPUSART_TX_TRIG_LVL) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TH, LPUSART_RX_TRIG_LVL - 1) | \
                  EIGEN_VAL2FLD(USART_FCR0_RXFIFO_TO_BIT_NUM, 16);

            lpusart->core_regs->FCR0 = val;

            if(lpusart->dma_rx)
            {
                // Set rx lvl to 1 to wake up from doze
                lpusart->core_regs->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, LPUSART_TX_TRIG_LVL) | \
                                           EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, 0);
            }
            else
            {
                lpusart->core_regs->FCR1 = EIGEN_VAL2FLD(USART_FCR1_TXFIFO_INT_TH, LPUSART_TX_TRIG_LVL) | \
                                           EIGEN_VAL2FLD(USART_FCR1_RXFIFO_INT_TH, LPUSART_RX_TRIG_LVL - 1);
            }

            if(lpusart->irq)
            {
                XIC_SetVector(lpusart->irq->irq_num, lpusart->irq->cb_irq);
                XIC_EnableIRQ(lpusart->irq->irq_num);
                XIC_SuppressOvfIRQ(lpusart->irq->irq_num);
            }

            lpusart->core_regs->RXSR = 0;

            lpusart->info->flags |= LPUSART_FLAG_POWER_FULL;  // LPUSART is powered on

            // Enable wakeup feature only
            lpusart->aon_regs->CR0 =  LPUSARTAON_CR0_RX_ENABLE_Msk;

            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

int32_t LPUSART_Send(const void *data, uint32_t num, LPUSART_RESOURCES *lpusart)
{
    uint32_t mask;
    if ((data == NULL) || (num == 0))
        return ARM_DRIVER_ERROR_PARAMETER;
    if ((lpusart->info->flags & LPUSART_FLAG_CONFIGURED) == 0)
        return ARM_DRIVER_ERROR;

    if(lpusart->core_regs->IER & USART_IER_AUTOBAUD_DONE_Msk)
    {
        if(lpusart->info->cb_event && lpusart->dma_tx)
        {
            lpusart->info->cb_event(ARM_USART_EVENT_SEND_COMPLETE);
        }

        return ARM_DRIVER_OK;
    }

    mask = SaveAndSetIRQMask();
    if (lpusart->info->xfer.send_active != 0)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    lpusart->info->xfer.send_active = 1U;
    RestoreIRQMask(mask);

    // Save transmit buffer info
    lpusart->info->xfer.tx_buf = (uint8_t *)data;
    lpusart->info->xfer.tx_num = num;
    lpusart->info->xfer.tx_cnt = 0;

    // DMA mode
    if(lpusart->dma_tx)
    {
        mask = SaveAndSetIRQMask();

        lpusart->core_regs->IER |= USART_IER_TX_TRANS_DONE_Msk;
        lpusart->core_regs->MCR |= USART_MCR_TX_DMA_EN_Msk;

#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(1, 0);
#endif

        RestoreIRQMask(mask);

        dmaTxConfig.sourceAddress = (void*)data;
        dmaTxConfig.targetAddress = (void*)&(lpusart->core_regs->TDR);
        dmaTxConfig.totalLength = num;

        // Configure tx DMA and start it
        DMA_transferSetup(lpusart->dma_tx->instance, lpusart->dma_tx->channel, &dmaTxConfig);
        DMA_startChannelNoVote(lpusart->dma_tx->instance, lpusart->dma_tx->channel);

    }
    else
    {
        while (lpusart->info->xfer.tx_cnt < lpusart->info->xfer.tx_num)
        {
            if(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, lpusart->core_regs->FSR) < 31)
            {
                lpusart->core_regs->TDR = lpusart->info->xfer.tx_buf[lpusart->info->xfer.tx_cnt++];
            }
        }

        USART_WAIT_TX_DONE(lpusart);

        mask = SaveAndSetIRQMask();
        lpusart->info->xfer.send_active = 0;
        RestoreIRQMask(mask);
    }

    return ARM_DRIVER_OK;
}

int32_t LPUSART_SendPolling(const void *data, uint32_t num, LPUSART_RESOURCES *lpusart)
{
    uint32_t mask;
    if ((data == NULL) || (num == 0))
        return ARM_DRIVER_ERROR_PARAMETER;
    if ((lpusart->info->flags & LPUSART_FLAG_CONFIGURED) == 0)
        return ARM_DRIVER_ERROR;

    if(lpusart->core_regs->IER & USART_IER_AUTOBAUD_DONE_Msk)
    {
        return ARM_DRIVER_OK;
    }

    mask = SaveAndSetIRQMask();
    if (lpusart->info->xfer.send_active != 0)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    lpusart->info->xfer.send_active = 1U;
    RestoreIRQMask(mask);

    // Save transmit buffer info
    lpusart->info->xfer.tx_buf = (uint8_t *)data;
    lpusart->info->xfer.tx_num = num;
    lpusart->info->xfer.tx_cnt = 0;

    while (lpusart->info->xfer.tx_cnt < lpusart->info->xfer.tx_num)
    {
        if(EIGEN_FLD2VAL(USART_FSR_TXFIFO_WL, lpusart->core_regs->FSR) < 31)
        {
            lpusart->core_regs->TDR = lpusart->info->xfer.tx_buf[lpusart->info->xfer.tx_cnt++];
        }
    }

    USART_WAIT_TX_DONE(lpusart);

    mask = SaveAndSetIRQMask();
    lpusart->info->xfer.send_active = 0;
    RestoreIRQMask(mask);

    return ARM_DRIVER_OK;
}

#define LPUSART_RX_COMMON_IRQ_ENABLE_MASK   (USART_IER_RX_START_Msk   | \
                                             USART_IER_RX_BREAK_DET_Msk | \
                                             USART_IER_RX_FRAME_ERR_Msk | \
                                             USART_IER_RX_PARITY_ERR_Msk | \
                                             USART_IER_LPUART_RX_FRAME_ERR_Msk | \
                                             USART_IER_LPUART_PARITY_ERR_Msk | \
                                             USART_IER_LPUART_RXFIFO_OF_Msk | \
                                             USART_IER_RXFIFO_OF_Msk \
                                            )

int32_t LPUSART_Receive(void *data, uint32_t num, LPUSART_RESOURCES *lpusart)
{
    uint32_t mask;

    if ((data == NULL) || num == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if ((lpusart->info->flags & LPUSART_FLAG_CONFIGURED) == 0)
    {
        return ARM_DRIVER_ERROR;
    }

    mask = SaveAndSetIRQMask();

    // check if receiver is busy
    if (lpusart->info->rx_status.rx_busy == 1U)
    {
        RestoreIRQMask(mask);
        return ARM_DRIVER_ERROR_BUSY;
    }

    lpusart->info->rx_status.rx_busy = 1U;

    RestoreIRQMask(mask);

    // save num of data to be received
    lpusart->info->xfer.rx_num = num;
    lpusart->info->xfer.rx_buf = (uint8_t *)data;
    lpusart->info->xfer.rx_cnt = 0;

    // dma mode
    if(lpusart->dma_rx)
    {
#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_recv_0, P_DEBUG, "lpuart recv enter, isr: 0x%x, fsr: 0x%x", lpusart->core_regs->ISR, lpusart->core_regs->FSR);
#endif
        mask = SaveAndSetIRQMask();

        lpusart->core_regs->FCR0 |= USART_FCR0_RXFIFO_TO_DMA_REQ_EN_Msk;

        if(lpusart->core_regs->IER & USART_IER_AUTOBAUD_DONE_Msk)
        {
            /*
               !!!Note!!!
               USART_IER_RX_TO: start to count down on receiving frist byte is writen into rxfifo(so stop control bit shall not be set)and counter value is refreshed on each byte reception,
               it may not start if lpuart autobaud fails to detect the larger baudrate, e.g. 230400

               USART_IER_RXFIFO_TO: start to count down on read/write rxfifo, since rx_to may not trigger, use this one
            */
            lpusart->core_regs->IER |= USART_IER_RX_TO_Msk;
        }
        else
        {
            lpusart->core_regs->MCR |= USART_MCR_RX_DMA_EN_Msk;

            if(lpusart->info->flags & LPUSART_FLAG_POWER_FULL)
            {
                lpusart->core_regs->IER |= LPUSART_RX_COMMON_IRQ_ENABLE_MASK;
            }
            else if(lpusart->info->flags & LPUSART_FLAG_POWER_LOW)
            {
                // refresh timeout counter when wakeup from low power state
                lpusart->core_regs->ICR = lpusart->core_regs->ISR & LPUSART_RX_COMMON_IRQ_ENABLE_MASK;
                lpusart->core_regs->IER |= LPUSART_RX_COMMON_IRQ_ENABLE_MASK;
                lpusart->core_regs->TCR |= USART_TCR_TOCNT_SWTRG_Msk;
            }
        }

        RestoreIRQMask(mask);

        dmaRxConfig.sourceAddress = (void*)&(lpusart->core_regs->RDR);
        dmaRxConfig.targetAddress = (void*)data;
        dmaRxConfig.totalLength = num;

        // Configure tx DMA and start it
        DMA_transferSetup(lpusart->dma_rx->instance, lpusart->dma_rx->channel, &dmaRxConfig);
        DMA_enableChannelInterrupts(lpusart->dma_rx->instance, lpusart->dma_rx->channel, (DMA_END_INTERRUPT_ENABLE | DMA_EOR_INTERRUPT_ENABLE));
        DMA_startChannelNoVote(lpusart->dma_rx->instance, lpusart->dma_rx->channel);

#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_recv_1, P_DEBUG, "lpuart recv exit, isr: 0x%x, fsr: 0x%x", lpusart->core_regs->ISR, lpusart->core_regs->FSR);
#endif

    }
    // irq mode
    else if(lpusart->irq)
    {
#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_recv_0, P_DEBUG, "lpuart recv enter, isr: 0x%x, fsr: 0x%x", lpusart->core_regs->ISR, lpusart->core_regs->FSR);
#endif
        mask = SaveAndSetIRQMask();

        if(lpusart->core_regs->IER & USART_IER_AUTOBAUD_DONE_Msk)
        {
            lpusart->core_regs->IER |= USART_IER_RX_TO_Msk;
        }
        else
        {
            if(lpusart->info->flags & LPUSART_FLAG_POWER_FULL)
            {
                lpusart->core_regs->IER |= LPUSART_RX_COMMON_IRQ_ENABLE_MASK | USART_IER_RXFIFO_WL_Msk | USART_IER_RXFIFO_TO_Msk;
            }
            else if(lpusart->info->flags & LPUSART_FLAG_POWER_LOW)
            {

                // refresh timeout counter when wakeup from low power state
                lpusart->core_regs->ICR = lpusart->core_regs->ISR;

                lpusart->core_regs->IER |= LPUSART_RX_COMMON_IRQ_ENABLE_MASK | USART_IER_RXFIFO_WL_Msk | USART_IER_RXFIFO_TO_Msk;

                lpusart->core_regs->TCR |= USART_TCR_TOCNT_SWTRG_Msk;
            }
        }

        RestoreIRQMask(mask);

#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_recv_1, P_DEBUG, "lpuart recv exit, isr: 0x%x, fsr: 0x%x", lpusart->core_regs->ISR, lpusart->core_regs->FSR);
#endif

    }
    else
    {
        while(lpusart->info->xfer.rx_cnt < lpusart->info->xfer.rx_num)
        {
            //wait until receive data is ready
            while((EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, lpusart->core_regs->FSR)) == 0);
            //read data
            lpusart->info->xfer.rx_buf[lpusart->info->xfer.rx_cnt++] = lpusart->core_regs->RDR;
        }
        lpusart->info->rx_status.rx_busy = 0;
    }

    return ARM_DRIVER_OK;
}

int32_t LPUSART_Transfer(const void *data_out, void *data_in, uint32_t num,LPUSART_RESOURCES *lpusart)
{
    //maybe used by command transfer
    return ARM_DRIVER_ERROR_UNSUPPORTED;
}

int32_t LPUSART_GetTxCount(LPUSART_RESOURCES *lpusart)
{
    uint32_t cnt;
    if (!(lpusart->info->flags & LPUSART_FLAG_CONFIGURED))
        return 0;
    if(lpusart->dma_tx)
        cnt = DMA_getChannelCount(lpusart->dma_tx->instance, lpusart->dma_tx->channel);
    else
        cnt = lpusart->info->xfer.tx_cnt;
    return cnt;
}

PLAT_PA_RAMCODE int32_t LPUSART_GetRxCount(LPUSART_RESOURCES *lpusart)
{
    if (!(lpusart->info->flags & LPUSART_FLAG_CONFIGURED))
        return 0;
    return lpusart->info->xfer.rx_cnt;
}

int32_t LPUSART_Control(uint32_t control, uint32_t arg, LPUSART_RESOURCES *lpusart)
{
    //uint32_t enr;
    uint32_t aon_lcr = lpusart->aon_regs->LCR;
    uint32_t lcr = lpusart->core_regs->LCR;

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

            USART_WAIT_TX_DONE(lpusart);

            return ARM_DRIVER_OK;

        case ARM_USART_CONTROL_PURGE_COMM:

            //if(lpusart->core_regs->RXSR == 1)
            {
                GPR_swReset(RST_LPUA);
            }

            //enr = lpusart->core_regs->ENR;
            //lpusart->core_regs->ENR = 0;

            //write 1 and dummy read twice and clear
            lpusart->core_regs->FCR0 |= USART_FCR0_RXFIFO_FLUSH_Msk;
            (void) lpusart->core_regs->FCR0;
            (void) lpusart->core_regs->FCR0;
            lpusart->core_regs->FCR0 &= ~(USART_FCR0_RXFIFO_FLUSH_Msk);

            // Clear IRQ flag
            //lpusart->core_regs->ICR = lpusart->core_regs->ISR;
            //lpusart->core_regs->ENR = enr;


            return ARM_DRIVER_OK;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }

    switch (control & ARM_USART_DATA_BITS_Msk)
    {
        case ARM_USART_DATA_BITS_5:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            aon_lcr &= ~LPUSARTAON_LCR_CHAR_LEN_Msk;
            break;
        case ARM_USART_DATA_BITS_6:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 1U;
            aon_lcr &= ~LPUSARTAON_LCR_CHAR_LEN_Msk;
            aon_lcr |= 1U;
            break;
        case ARM_USART_DATA_BITS_7:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 2U;
            aon_lcr &= ~LPUSARTAON_LCR_CHAR_LEN_Msk;
            aon_lcr |= 2U;
            break;
        case ARM_USART_DATA_BITS_8:
            lcr &= ~USART_LCR_CHAR_LEN_Msk;
            lcr |= 3U;
            aon_lcr &= ~LPUSARTAON_LCR_CHAR_LEN_Msk;
            aon_lcr |= 3U;
            break;
        default:
            return ARM_USART_ERROR_DATA_BITS;
    }

    // LPUSART Parity
    switch (control & ARM_USART_PARITY_Msk)
    {
        case ARM_USART_PARITY_NONE:
            lcr &= ~USART_LCR_PARITY_EN_Msk;
            aon_lcr &= ~LPUSARTAON_LCR_PARITY_EN_Msk;
            break;
        case ARM_USART_PARITY_EVEN:
            lcr |= (USART_LCR_PARITY_EN_Msk | USART_LCR_EVEN_PARITY_Msk);
            aon_lcr |= (LPUSARTAON_LCR_PARITY_EN_Msk | LPUSARTAON_LCR_EVEN_PARITY_Msk);
            break;
        case ARM_USART_PARITY_ODD:
            lcr |= USART_LCR_PARITY_EN_Msk;
            lcr &= ~USART_LCR_EVEN_PARITY_Msk;
            aon_lcr |= LPUSARTAON_LCR_PARITY_EN_Msk;
            aon_lcr &= ~LPUSARTAON_LCR_EVEN_PARITY_Msk;
            break;
        default:
            return (ARM_USART_ERROR_PARITY);
    }

    // LPUSART Stop bits
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

    // LPUSART Flow Control
    switch (control & ARM_USART_FLOW_CONTROL_Msk)
    {
        case ARM_USART_FLOW_CONTROL_NONE:
        case ARM_USART_FLOW_CONTROL_RTS:
            // set RTS pin to low
            lpusart->core_regs->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            break;
        case ARM_USART_FLOW_CONTROL_CTS:
            lpusart->core_regs->MCR |= USART_MCR_AUTO_FLOW_CTS_EN_Msk;
            lpusart->core_regs->IER |= USART_IER_CTS_TOGGLE_Msk;
            break;
        case ARM_USART_FLOW_CONTROL_RTS_CTS:
            // set RTS pin to low
            lpusart->core_regs->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
            lpusart->core_regs->MCR |= (USART_MCR_AUTO_FLOW_CTS_EN_Msk);
            lpusart->core_regs->IER |= USART_IER_CTS_TOGGLE_Msk;
            break;
    }

    // set rx timeout to max, that's (63-10)bits
#if 1
    lpusart->core_regs->LCR = lcr | USART_LCR_RX_TO_BIT_NUM_Msk;
#endif
    // don't check stopbits, tx use frac div
#if 1
    lpusart->core_regs->HCR = USART_HCR_DMA_EOR_MODE_Msk | \
                              USART_HCR_TX_USE_DIV_FRAC_Msk | \
                              EIGEN_VAL2FLD(USART_HCR_AUTO_CG, 0xFF);
#else
    lpusart->core_regs->HCR = USART_HCR_DMA_EOR_MODE_Msk | \
                              USART_HCR_TX_USE_DIV_FRAC_Msk | \
                              USART_HCR_RX_STOPBIT_CHK_Msk | \
                              EIGEN_VAL2FLD(USART_HCR_AUTO_CG, 0xFF);
#endif

    // LPUSART Baudrate
    if(ARM_DRIVER_OK != LPUSART_SetBaudrate(arg, aon_lcr, lpusart))
    {
        return ARM_USART_ERROR_BAUDRATE;
    }

    lpusart->info->frame_code = control;
    lpusart->info->flags |= LPUSART_FLAG_CONFIGURED;

    return ARM_DRIVER_OK;
}

ARM_USART_STATUS LPUSART_GetStatus(LPUSART_RESOURCES *lpusart)
{
    ARM_USART_STATUS status;

    status.tx_busy          = lpusart->info->xfer.send_active;
    status.rx_busy          = lpusart->info->rx_status.rx_busy;
    status.tx_underflow     = 0;
    status.rx_overflow      = lpusart->info->rx_status.rx_overflow;
    status.rx_break         = lpusart->info->rx_status.rx_break;
    status.rx_framing_error = lpusart->info->rx_status.rx_framing_error;
    status.rx_parity_error  = lpusart->info->rx_status.rx_parity_error;
    status.is_send_block    = (lpusart->dma_tx == NULL);
    return status;
}

int32_t LPUSART_SetModemControl(ARM_USART_MODEM_CONTROL control, LPUSART_RESOURCES *lpusart)
{
    if((lpusart->info->flags & LPUSART_FLAG_CONFIGURED) == 0U)
    {
        // USART is not configured
       return ARM_DRIVER_ERROR;
    }

    if(lpusart->info->frame_code & ARM_USART_FLOW_CONTROL_RTS)
    {
        if(control == ARM_USART_RTS_CLEAR) //Deactivate RTS, put rts pin to high level
        {
            lpusart->core_regs->FLOWCR &= ~USART_FLOWCR_RTS_Msk;
        }

        if(control == ARM_USART_RTS_SET) //Activate RTS, put rts pin to low level
        {
            lpusart->core_regs->FLOWCR |= USART_FLOWCR_RTS_Msk;
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

ARM_USART_MODEM_STATUS LPUSART_GetModemStatus(LPUSART_RESOURCES *lpusart)
{
    ARM_USART_MODEM_STATUS status = {0};

    if(lpusart->info->flags & LPUSART_FLAG_CONFIGURED)
    {
        status.cts = !(EIGEN_FLD2VAL(USART_FLOWCR_CTS, lpusart->core_regs->FLOWCR));
    }

    return status;
}

void LPUSART_WakeupIntHandler(void)
{
    slpManExtIntPreProcess(LpuartWakeup_IRQn);

    //extern void AonRegAPClrWIC(void);
    // WIC clear
    *(uint32_t*) 0x4f020140 = 1;
    NVIC_DisableIRQ(LpuartWakeup_IRQn);

#if LPUSART_DRIVER_DEBUG
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_wakeup_irq, P_SIG, "ISR:0x%x, LSR:0x%x, FSR:0x%x", LPUSART_CORE->ISR, LPUSART_CORE->LSR, LPUSART_CORE->FSR);
#endif
}

PLAT_PA_RAMCODE void LPUSART_IRQHandler(LPUSART_RESOURCES *lpusart)
{

    uint32_t event;
    uint32_t isr_reg;
    uint32_t i, current_cnt, total_cnt, left_to_recv, bytes_in_fifo;

IRQ_HANDLE:
    event = 0;

    LPUSART_INFO *info = lpusart->info;

    // Check interrupt source
    isr_reg = lpusart->core_regs->ISR;
    lpusart->core_regs->ICR = isr_reg;

#if LPUSART_DRIVER_DEBUG
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_irq_enter, P_DEBUG, "Enter lpuart irq, isr: 0x%x, fsr: 0x%x, rx_cnt:%d", isr_reg, lpusart->core_regs->FSR, info->xfer.rx_cnt);
#endif

    // RXFIFO overflow
    if(isr_reg & (USART_ISR_RXFIFO_OF_Msk | USART_ISR_LPUART_RXFIFO_OF_Msk))
    {
        lpusart->info->rx_status.rx_busy = 0;
        info->rx_status.rx_overflow = 1U;
        event |= ARM_USART_EVENT_RX_OVERFLOW;
    }

    // Break detected
    if(isr_reg & USART_ISR_RX_BREAK_DET_Msk)
    {
        lpusart->info->rx_status.rx_busy = 0;
        lpusart->info->rx_status.rx_break = 1;
        event |= ARM_USART_EVENT_RX_BREAK;
    }

    // Framing error
    if(isr_reg & (USART_ISR_RX_FRAME_ERR_Msk | USART_ISR_LPUART_RX_FRAME_ERR_Msk))
    {
        lpusart->info->rx_status.rx_busy = 0;
        lpusart->info->rx_status.rx_framing_error = 1;
        event |= ARM_USART_EVENT_RX_FRAMING_ERROR;
    }

    // Parity error
    if(isr_reg & (USART_ISR_RX_PARITY_ERR_Msk | USART_ISR_LPUART_PARITY_ERR_Msk))
    {
        lpusart->info->rx_status.rx_busy = 0;
        lpusart->info->rx_status.rx_parity_error = 1;
        event |= ARM_USART_EVENT_RX_PARITY_ERROR;
    }

    // TX transfer done
    if(isr_reg & USART_ISR_TX_TRANS_DONE_Msk)
    {
        info->xfer.tx_cnt = info->xfer.tx_num;
        info->xfer.send_active = 0U;

        event |= ARM_USART_EVENT_SEND_COMPLETE;

        lpusart->core_regs->IER &= ~USART_IER_TX_TRANS_DONE_Msk;
        lpusart->core_regs->MCR &= ~USART_MCR_TX_DMA_EN_Msk;

#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(1, 0);
#endif
    }

    // RX water level
    if(isr_reg & USART_ISR_RXFIFO_WL_Msk)
    {
        current_cnt = info->xfer.rx_cnt;
        total_cnt = info->xfer.rx_num;

        left_to_recv = total_cnt - current_cnt;

        bytes_in_fifo = lpusart->core_regs->FSR >> USART_FSR_RXFIFO_WL_Pos;

        // leave at least one byte in fifo to trigger timeout interrupt
        i = bytes_in_fifo - 1;

        if(i == 0)
            i = 1;

        i = MIN(i, left_to_recv);

        while(i--)
        {
            info->xfer.rx_buf[current_cnt++] = lpusart->core_regs->RDR;
        }

        // The clear action has no effect when enters irqHandler since the water level holds, let's clear twice here since we've moved data from FIFO
        lpusart->core_regs->ICR = USART_ICR_RXFIFO_WL_Msk;

        info->xfer.rx_cnt = current_cnt;

        if(current_cnt == total_cnt)
        {
            // Clear RX busy flag and set receive transfer complete event
            event |= ARM_USART_EVENT_RECEIVE_COMPLETE;

            lpusart->core_regs->IER &= ~(LPUSART_RX_COMMON_IRQ_ENABLE_MASK | USART_IER_RXFIFO_WL_Msk);

            info->rx_status.rx_busy = 0U;
        }

    }

    // RX timeout, only valid during auto baud
    if(isr_reg & USART_ISR_RX_TO_Msk)
    {
        // check the first received character's pattern(bit[0] shall be 1)
        lpusart->info->xfer.rx_dump_val = lpusart->core_regs->RDR;

#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_irq_autobaud_0, P_DEBUG, "Lpuart auto baud result, abdr: %d, divir: %d, baudrate: %d, rx_char: 0x%x", lpusart->aon_regs->ABDR, lpusart->core_regs->DIVIR, info->baudrate, lpusart->info->xfer.rx_dump_val);
#endif

        // flush rxfifo
        lpusart->core_regs->FCR0 |= USART_FCR0_RXFIFO_FLUSH_Msk;
        (void) lpusart->core_regs->FCR0;
        (void) lpusart->core_regs->FCR0;
        lpusart->core_regs->FCR0 &= ~USART_FCR0_RXFIFO_FLUSH_Msk;

        // pattern is right and no error happens
        if(lpusart->info->xfer.rx_dump_val & 0x1)
        {
            event = ARM_USART_EVENT_AUTO_BAUDRATE_DONE;

            // can switch to lpuart
            lpusart->aon_regs->DLR = lpusart->aon_regs->ABDR;

            // clear autobaud enable bit
            lpusart->aon_regs->CR1 &= ~LPUSARTAON_CR1_AUTO_BAUD_Msk;

        }
        else
        {
            // Either exceeding the max speed of lpuart or pattern is wrong, switch to normal uart in both cases, reset have to be performed for the latter case
            lpusart->core_regs->RXSR = 0;
            lpusart->aon_regs->CR0 &= ~LPUSARTAON_CR0_CLK_ENABLE_Msk;

        }

        // Restore DMA or IRQ enable setting when autobaud is done
        if(lpusart->dma_rx)
        {
            lpusart->core_regs->MCR |= USART_MCR_RX_DMA_EN_Msk;
            lpusart->core_regs->IER = LPUSART_RX_COMMON_IRQ_ENABLE_MASK;
        }
        else
        {
            lpusart->core_regs->IER = LPUSART_RX_COMMON_IRQ_ENABLE_MASK | USART_IER_RXFIFO_WL_Msk | USART_IER_RXFIFO_TO_Msk;
        }

    }

    // RXFIFO timeout
    if(isr_reg & USART_ISR_RXFIFO_TO_Msk)
    {
        current_cnt = info->xfer.rx_cnt;

        total_cnt = info->xfer.rx_num;

        bytes_in_fifo = lpusart->core_regs->FSR >> USART_FSR_RXFIFO_WL_Pos;

        left_to_recv = total_cnt - current_cnt;

        i = MIN(bytes_in_fifo, left_to_recv);

        // if still have space to recv
        if(left_to_recv > 0)
        {
            while(i--)
            {
                info->xfer.rx_buf[current_cnt++] = lpusart->core_regs->RDR;
            }
        }

        info->xfer.rx_cnt = current_cnt;

        // Check if required amount of data is received
        if(current_cnt == total_cnt)
        {
            // Clear RX busy flag and set receive transfer complete event
            event |= ARM_USART_EVENT_RECEIVE_COMPLETE;

            lpusart->core_regs->IER &= ~(LPUSART_RX_COMMON_IRQ_ENABLE_MASK);
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
        if(lpusart->core_regs->DIVIR != 0)
        {
            info->baudrate = ((GPR_getClockFreq(FCLK_UART1) << 4) + (lpusart->core_regs->DIVIR >> 1)) / (lpusart->core_regs->DIVIR);

            // Backup for sleep restore
            lpusart->core_regs->DIVR = lpusart->core_regs->DIVIR;

            // trigger rx_to timer counter in case of it hasn't started yet(lpuart autobaud detecting fails)
            lpusart->core_regs->TCR = USART_TCR_TOCNT_SWTRG_Msk;

            // threshold: 9600*(1+30%)
            if(info->baudrate > LPUSART_MAX_BAUDRATE)
            {
                lpusart->core_regs->RXSR = 0;
                lpusart->aon_regs->CR0 &= ~LPUSARTAON_CR0_CLK_ENABLE_Msk;
            }

        }

    }

    // check fifo number for case where start/end order is reversed(irq is blocked)
    // since dma request level is 16 and burst size is 8, there's always bytes left in fifo
    if(((isr_reg & USART_ISR_RX_START_Msk) == USART_ISR_RX_START_Msk) && ((EIGEN_FLD2VAL(USART_FSR_RXFIFO_WL, lpusart->core_regs->FSR)) != 0))
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(0, 1);
#endif
        lpusart->info->rx_status.rx_busy = 1;
    }

    if((info->cb_event != NULL) && (event != 0))
    {
        info->cb_event(event);

#ifdef PM_FEATURE_ENABLE
        if((event & ARM_USART_RX_EVENTS) != 0)
        {
            CHECK_TO_UNLOCK_SLEEP(0, 1);
        }
#endif

    }

    if(lpusart->core_regs->ISR != 0)
    {

#if LPUSART_DRIVER_DEBUG
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, lpuart_irq_exit, P_INFO, "Exit lpuart irq with pending irq, isr:0x%x, fsr_reg:0x%x, rx_cnt:%d", lpusart->core_regs->ISR, lpusart->core_regs->FSR, info->xfer.rx_cnt);
#endif
        goto IRQ_HANDLE;
    }

}

void LPUSART_DmaRxEvent(uint32_t event, LPUSART_RESOURCES *lpusart)
{
    LPUSART_INFO *info = lpusart->info;
    uint32_t dmaCurrentTargetAddress = DMA_getChannelCurrentTargetAddress(lpusart->dma_rx->instance, lpusart->dma_rx->channel, false);

    switch (event)
    {
        case DMA_EVENT_EOR:

            info->xfer.rx_cnt = dmaCurrentTargetAddress - (uint32_t)info->xfer.rx_buf;

#if USART_DEBUG
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, LPUSART_DmaRxEvent_0, P_INFO, "lpuart dma rx timeout, fsr:%x, rx_cnt:%d", lpusart->core_regs->FSR, info->xfer.rx_cnt);
#endif

            // start next recv
            DMA_startChannelNoVote(lpusart->dma_rx->instance, lpusart->dma_rx->channel);

            info->rx_status.rx_busy = 0;

            if(info->cb_event)
            {
                info->cb_event(ARM_USART_EVENT_RX_TIMEOUT);
            }
#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(0, 1);
#endif

            break;

        case DMA_EVENT_END:

#if USART_DEBUG
            ECPLAT_PRINTF(UNILOG_PLA_DRIVER, LPUSART_DmaRxEvent_1, P_INFO, "lpuart dma rx end, fsr:%x", lpusart->core_regs->FSR);
#endif
            //Disable all recv interrupt
            lpusart->core_regs->IER &= ~(LPUSART_RX_COMMON_IRQ_ENABLE_MASK);

            info->rx_status.rx_busy = 0;
            info->xfer.rx_cnt = info->xfer.rx_num;

            if(info->cb_event)
            {
                info->cb_event(ARM_USART_EVENT_RECEIVE_COMPLETE);
            }

#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(0, 1);
#endif

            break;
        case DMA_EVENT_ERROR:
        default:
            break;
    }
}
#if (RTE_UART1)
static int32_t                    LPUSART1_Initialize      (ARM_USART_SignalEvent_t cb_event)                        { return LPUSART_Initialize(cb_event, &LPUSART1_Resources); }
static int32_t                    LPUSART1_Uninitialize    (void)                                                    { return LPUSART_Uninitialize(&LPUSART1_Resources); }
static int32_t                    LPUSART1_PowerControl    (ARM_POWER_STATE state)                                   { return LPUSART_PowerControl(state, &LPUSART1_Resources); }
static int32_t                    LPUSART1_Send            (const void *data, uint32_t num)                          { return LPUSART_Send(data, num, &LPUSART1_Resources); }
static int32_t                    LPUSART1_Receive         (void *data, uint32_t num)                                { return LPUSART_Receive(data, num, &LPUSART1_Resources); }
static int32_t                    LPUSART1_Transfer        (const void *data_out, void *data_in, uint32_t num)       { return LPUSART_Transfer(data_out, data_in, num, &LPUSART1_Resources); }
static int32_t                    LPUSART1_SendPolling     (const void *data, uint32_t num)                          { return LPUSART_SendPolling (data, num, &LPUSART1_Resources); }
static uint32_t                   LPUSART1_GetTxCount      (void)                                                    { return LPUSART_GetTxCount(&LPUSART1_Resources); }
PLAT_PA_RAMCODE static uint32_t   LPUSART1_GetRxCount      (void)                                                    { return LPUSART_GetRxCount(&LPUSART1_Resources); }
static uint32_t                   LPUSART1_GetBaudRate     (void)                                                    { return LPUSART_GetBaudRate(&LPUSART1_Resources); }
static int32_t                    LPUSART1_Control         (uint32_t control, uint32_t arg)                          { return LPUSART_Control(control, arg, &LPUSART1_Resources); }
static ARM_USART_STATUS           LPUSART1_GetStatus       (void)                                                    { return LPUSART_GetStatus(&LPUSART1_Resources); }
static int32_t                    LPUSART1_SetModemControl (ARM_USART_MODEM_CONTROL control)                         { return LPUSART_SetModemControl(control, &LPUSART1_Resources); }
static ARM_USART_MODEM_STATUS     LPUSART1_GetModemStatus  (void)                                                    { return LPUSART_GetModemStatus(&LPUSART1_Resources); }
PLAT_PA_RAMCODE       void        LPUSART1_IRQHandler      (void)                                                    {        LPUSART_IRQHandler(&LPUSART1_Resources); }

#if (RTE_UART1_RX_IO_MODE == DMA_MODE)
void                              LPUSART1_DmaRxEvent(uint32_t event)                                                {        LPUSART_DmaRxEvent(event, &LPUSART1_Resources);}
#endif


ARM_DRIVER_USART Driver_LPUSART1 = {
    LPUSART_GetVersion,
    LPUSART_GetCapabilities,
    LPUSART1_Initialize,
    LPUSART1_Uninitialize,
    LPUSART1_PowerControl,
    LPUSART1_Send,
    LPUSART1_Receive,
    LPUSART1_Transfer,
    LPUSART1_GetTxCount,
    LPUSART1_GetRxCount,
    LPUSART1_Control,
    LPUSART1_GetStatus,
    LPUSART1_SetModemControl,
    LPUSART1_GetModemStatus,
    LPUSART1_GetBaudRate,
    LPUSART1_SendPolling
};

#endif

#if 0
#pragma GCC pop_options
#endif

#endif
