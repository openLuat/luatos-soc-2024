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
  *
 * $Date:        12. July 2018
 * $Revision:    V2.0
 *
 * Driver:       Driver_SPI0, Driver_SPI1, Driver_SPI2
 * Configured:   via RTE_Device.h configuration file
 * Project:      SPI (SSP used for SPI) Driver for AirM2M EC718
 * --------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                 Value   SPI Interface
 *   ---------------------                 -----   -------------
 *   Connect to hardware via Driver_SPI# = 0       use SPI0 (SSP0)
 *   Connect to hardware via Driver_SPI# = 1       use SPI1 (SSP1)
 * -------------------------------------------------------------------------- */

/* History:
 *  Version 2.0
 *    - Initial CMSIS Driver API V2.0 release
 */

#include "bsp_spi.h"
#include "slpman.h"

#define BSP_SPI_TEXT_SECTION     SECTION_DEF_IMPL(.sect_bsp_spi_text)
#define BSP_SPI_RODATA_SECTION   SECTION_DEF_IMPL(.sect_bsp_spi_rodata)
#define BSP_SPI_DATA_SECTION     SECTION_DEF_IMPL(.sect_bsp_spi_data)
#define BSP_SPI_BSS_SECTION      SECTION_DEF_IMPL(.sect_bsp_spi_bss)


#ifndef DISTANCE
#define DISTANCE(a,b)  ((a>b)?(a-b):(b-a))
#endif

#define SPI_RX_FIFO_TRIG_LVL      (8)
#define SPI_FIFO_DEPTH            (16)

#define ARM_SPI_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) // driver version

#if ((!RTE_SPI0) && (!RTE_SPI1))
#error "spi not enabled in RTE_Device.h!"
#endif

typedef struct
{
    uint32_t CR0;                                    /**< Control Register 0 */
    uint32_t CR1;                                    /**< Control Register 1 */
    uint32_t CPSR;                                   /**< Clock Prescale Register */
    uint32_t IMSC;                                   /**< Interrupt Mask Set or Clear Register */
    uint32_t DMACR;                                  /**< DMA Control Register */
    uint32_t DRLR;                                   /**< DMA Request Level Register */
    uint32_t DRRCR;                                  /**< DMA Rx Request Control Register */
    uint32_t DTRCR;                                  /**< DMA Tx Request Control Register */
    uint32_t RPTCR;                                  /**< Rx Path Timeout Register */
    uint32_t SRLR;                                   /**< SW request level register */
    uint32_t MPSR;                                   /**< Master RXD phase sel register */
} spi_backup_register_t;

#ifdef PM_FEATURE_ENABLE

/** \brief Internal used data structure */
typedef struct _spi_database
{
    bool                            isInited;            /**< Whether spi has been initialized */
    spi_backup_register_t           backup_registers;    /**< Backup registers for low power restore */
} spi_database_t;

BSP_SPI_BSS_SECTION static spi_database_t g_spiDataBase[SPI_INSTANCE_NUM] = {0};
#endif

BSP_SPI_RODATA_SECTION static SPI_TypeDef* const g_spiBases[] = SPI_INSTANCE_ARRAY;
BSP_SPI_RODATA_SECTION static const ClockId_e g_spiClocks[] = SPI_CLOCK_VECTOR;
BSP_SPI_RODATA_SECTION static const ClockResetVector_t g_spiResetVectors[] = SPI_RESET_VECTORS;

#ifdef PM_FEATURE_ENABLE
/**
  \brief spi initialization counter, for lower power callback register/de-register
 */
BSP_SPI_BSS_SECTION static uint32_t g_spiInitCounter = 0;

/**
  \brief Bitmap of SPI working status, each instance is assigned 2 bits representing tx and rx status,
         when all SPI instances are not working, we can vote to enter to low power state.
 */

BSP_SPI_BSS_SECTION static uint32_t g_spiWorkingStatus = 0;


/**
  \fn        static void SPI_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, SPI hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void SPI_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:
            for(i = 0; i < SPI_INSTANCE_NUM; i++)
            {
                if(g_spiDataBase[i].isInited == true)
                {
                    g_spiDataBase[i].backup_registers.CR0 = g_spiBases[i]->CR0;
                    g_spiDataBase[i].backup_registers.CR1 = g_spiBases[i]->CR1;
                    g_spiDataBase[i].backup_registers.CPSR = g_spiBases[i]->CPSR;
                    g_spiDataBase[i].backup_registers.IMSC = g_spiBases[i]->IMSC;
                    g_spiDataBase[i].backup_registers.DMACR = g_spiBases[i]->DMACR;
                    g_spiDataBase[i].backup_registers.DRLR = g_spiBases[i]->DRLR;
                    g_spiDataBase[i].backup_registers.DRRCR = g_spiBases[i]->DRRCR;
                    g_spiDataBase[i].backup_registers.DTRCR = g_spiBases[i]->DTRCR;
                    g_spiDataBase[i].backup_registers.RPTCR = g_spiBases[i]->RPTCR;
                    g_spiDataBase[i].backup_registers.SRLR = g_spiBases[i]->SRLR;
                    g_spiDataBase[i].backup_registers.MPSR = g_spiBases[i]->MPSR;
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
  \fn        static void SPI_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, SPI hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state

 */
static void SPI_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < SPI_INSTANCE_NUM; i++)
            {
                if(g_spiDataBase[i].isInited == true)
                {
                    GPR_clockEnable(g_spiClocks[2*i]);
                    GPR_clockEnable(g_spiClocks[2*i+1]);

                    g_spiBases[i]->CR0 = g_spiDataBase[i].backup_registers.CR0;
                    g_spiBases[i]->CPSR = g_spiDataBase[i].backup_registers.CPSR;
                    g_spiBases[i]->IMSC = g_spiDataBase[i].backup_registers.IMSC;
                    g_spiBases[i]->DMACR = g_spiDataBase[i].backup_registers.DMACR;
                    g_spiBases[i]->DRLR = g_spiDataBase[i].backup_registers.DRLR;
                    g_spiBases[i]->DRRCR = g_spiDataBase[i].backup_registers.DRRCR;
                    g_spiBases[i]->DTRCR = g_spiDataBase[i].backup_registers.DTRCR;
                    g_spiBases[i]->RPTCR = g_spiDataBase[i].backup_registers.RPTCR;
                    g_spiBases[i]->SRLR = g_spiDataBase[i].backup_registers.SRLR;
                    g_spiBases[i]->MPSR = g_spiDataBase[i].backup_registers.MPSR;

                    g_spiBases[i]->CR1 = g_spiDataBase[i].backup_registers.CR1;

                }
            }
            break;

        case SLPMAN_IDLE_STATE:         // add doze process

            break;

        default:
            break;
    }

}

#define  LOCK_SLEEP(instance)     do                                                                     \
                                          {                                                              \
                                              g_spiWorkingStatus |= (1U << instance);                    \
                                              slpManDrvVoteSleep(SLP_VOTE_SPI, SLP_ACTIVE_STATE);  \
                                          }                                                              \
                                          while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)      do                                                                     \
                                                      {                                                              \
                                                          g_spiWorkingStatus &= ~(1U << instance);                   \
                                                          if(g_spiWorkingStatus == 0)                                \
                                                              slpManDrvVoteSleep(SLP_VOTE_SPI, SLP_SLP1_STATE); \
                                                      }                                                              \
                                                      while(0)

#endif

extern uint32_t DMA_getChannelCount(DmaInstance_e instance, uint32_t channel);

// Driver Version
static const ARM_DRIVER_VERSION DriverVersion = {
    ARM_SPI_API_VERSION,
    ARM_SPI_DRV_VERSION
};

// Driver Capabilities
static const ARM_SPI_CAPABILITIES DriverCapabilities = {
    0, // Simplex Mode (Master and Slave)
    1, // TI Synchronous Serial Interface
    1, // Microwire Interface
    0  // Signal Mode Fault event
};

void SPI0_IRQHandler(void);
void SPI1_IRQHandler(void);

#if (RTE_SPI0)

BSP_SPI_BSS_SECTION static SPI_INFO  SPI0_Info = {0};
BSP_SPI_RODATA_SECTION static const PIN SPI0_pin_sclk  = {RTE_SPI0_SCLK_BIT,   RTE_SPI0_SCLK_FUNC};

#if RTE_SPI0_SSN_BIT
BSP_SPI_RODATA_SECTION static const PIN SPI0_pin_ssn   = {RTE_SPI0_SSN_BIT,    RTE_SPI0_SSN_FUNC};
#endif

BSP_SPI_RODATA_SECTION static const PIN SPI0_pin_mosi  = {RTE_SPI0_MOSI_BIT,   RTE_SPI0_MOSI_FUNC};
BSP_SPI_RODATA_SECTION static const PIN SPI0_pin_miso  = {RTE_SPI0_MISO_BIT,   RTE_SPI0_MISO_FUNC};


#if (RTE_SPI0_IO_MODE == DMA_MODE)
void SPI0_DmaRxEvent(uint32_t event);

BSP_SPI_BSS_SECTION static DmaDescriptor_t __ALIGNED(16) SPI0_DMA_Descriptor;

BSP_SPI_DATA_SECTION static SPI_DMA SPI0_DMA = {
                            DMA_INSTANCE_MP,
                            -1,
                            RTE_SPI0_DMA_TX_REQID,
                            DMA_INSTANCE_MP,
                            -1,
                            RTE_SPI0_DMA_RX_REQID,
                            SPI0_DmaRxEvent,
                            &SPI0_DMA_Descriptor
                          };
#elif (RTE_SPI0_IO_MODE == IRQ_MODE)
#warning SPI irq mode is not suggested to use, use DMA_MODE instead
#endif

#if (RTE_SPI0_IO_MODE == IRQ_MODE) || (RTE_SPI0_IO_MODE == DMA_MODE)

BSP_SPI_RODATA_SECTION static SPI_IRQ SPI0_IRQ = {
                            PXIC0_SSP0_IRQn,
                            SPI0_IRQHandler
                          };
#endif

BSP_SPI_RODATA_SECTION static SPI_RESOURCES SPI0_Resources = {
    SPI0,
    {
       &SPI0_pin_sclk,
#if RTE_SPI0_SSN_BIT
       &SPI0_pin_ssn,
#else
        NULL,
#endif
       &SPI0_pin_mosi,
       &SPI0_pin_miso,
    },
#if (RTE_SPI0_IO_MODE == DMA_MODE)
    &SPI0_DMA,
#else
    NULL,
#endif

#if (RTE_SPI0_IO_MODE == IRQ_MODE) || (RTE_SPI0_IO_MODE == DMA_MODE)
    &SPI0_IRQ,
#else
    NULL,
#endif
    &SPI0_Info
};
#endif

#if (RTE_SPI1)

BSP_SPI_BSS_SECTION  static SPI_INFO  SPI1_Info = {0};
BSP_SPI_RODATA_SECTION static const PIN SPI1_pin_sclk  = {RTE_SPI1_SCLK_BIT,   RTE_SPI1_SCLK_FUNC};

#if RTE_SPI1_SSN_BIT
BSP_SPI_RODATA_SECTION static const PIN SPI1_pin_ssn   = {RTE_SPI1_SSN_BIT,    RTE_SPI1_SSN_FUNC};
#endif

BSP_SPI_RODATA_SECTION static const PIN SPI1_pin_mosi  = {RTE_SPI1_MOSI_BIT,   RTE_SPI1_MOSI_FUNC};
BSP_SPI_RODATA_SECTION static const PIN SPI1_pin_miso  = {RTE_SPI1_MISO_BIT,   RTE_SPI1_MISO_FUNC};

#if (RTE_SPI1_IO_MODE == DMA_MODE)
void SPI1_DmaRxEvent(uint32_t event);

BSP_SPI_BSS_SECTION static DmaDescriptor_t __ALIGNED(16) SPI1_DMA_Descriptor;

BSP_SPI_DATA_SECTION static SPI_DMA SPI1_DMA = {
                            DMA_INSTANCE_MP,
                            -1,
                            RTE_SPI1_DMA_TX_REQID,
                            DMA_INSTANCE_MP,
                            -1,
                            RTE_SPI1_DMA_RX_REQID,
                            SPI1_DmaRxEvent,
                            &SPI1_DMA_Descriptor
                          };
#elif (RTE_SPI0_IO_MODE == IRQ_MODE)
#warning SPI irq mode is not suggested to use, use DMA_MODE instead
#endif

#if (RTE_SPI1_IO_MODE == IRQ_MODE) || (RTE_SPI1_IO_MODE == DMA_MODE)

BSP_SPI_RODATA_SECTION static SPI_IRQ SPI1_IRQ = {
                            PXIC0_SSP1_IRQn,
                            SPI1_IRQHandler
                          };
#endif

BSP_SPI_RODATA_SECTION static SPI_RESOURCES SPI1_Resources = {
    SPI1,
    {
       &SPI1_pin_sclk,
#if RTE_SPI1_SSN_BIT
       &SPI1_pin_ssn,
#else
       NULL,
#endif
       &SPI1_pin_mosi,
       &SPI1_pin_miso,
    },
#if (RTE_SPI1_IO_MODE == DMA_MODE)
    &SPI1_DMA,
#else
    NULL,
#endif

#if (RTE_SPI1_IO_MODE == IRQ_MODE) || (RTE_SPI1_IO_MODE == DMA_MODE)
    &SPI1_IRQ,
#else
    NULL,
#endif
    &SPI1_Info
};

#endif

// Local Function

/**
  \fn          static uint32_t SPI_GetInstanceNumber(SPI_RESOURCES *spi)
  \brief       Get instance number
  \param[in]   spi       Pointer to SPI resources
  \returns     instance number
*/
PLAT_FM_RAMCODE static uint32_t SPI_GetInstanceNumber(SPI_RESOURCES *spi)
{
    return ((uint32_t)spi->reg - (uint32_t)SPI0) >> 12U;
}

/**
  \fn          ARM_DRIVER_VERSION SPI_GetVersion(void)
  \brief       Get SSP driver version.
  \return      \ref ARM_DRV_VERSION
*/
ARM_DRIVER_VERSION SPI_GetVersion(void)
{
    return DriverVersion;
}

/**
  \fn          ARM_SPI_CAPABILITIES SPI_GetCapabilities(void)
  \brief       Get driver capabilities.
  \return      \ref ARM_SPI_CAPABILITIES
*/
ARM_SPI_CAPABILITIES SPI_GetCapabilities(void)
{
    return DriverCapabilities;
}

/**
  \fn          static int32_t SPI_SetBusSpeed(uint32_t bps, SPI_RESOURCES *spi)
  \brief       Set bus speed
  \param[in]   bps       bus speed to set
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
static int32_t SPI_SetBusSpeed(uint32_t bps, SPI_RESOURCES *spi)
{
    uint32_t instance, spi_clock = 0;

    uint32_t cpsr, scr, best_cpsr = 2U, best_scr = 1U;
    uint32_t div = 0, best_div = 2U;

    if(bps == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    instance = SPI_GetInstanceNumber(spi);

#ifdef FPGA_TEST
    spi_clock = 102400000;
#else
    spi_clock = GPR_getClockFreq(g_spiClocks[instance*2+1]);
#endif
    // bps = clock_in / div where div = cpsr * scr

    // round to nearest even number
    div = spi_clock / bps;
    div += (div & 1U);

    if (div == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    /* divider range [2,512] = 2*[1,256] (cpsr = 2)
     * this range convers most cases, for 51.2M input clock,
     * the minimum generated bps can reach 51.2M/512 = 100K
     */
    else if (div <= 512U)
    {
        best_cpsr = 2U;
        best_scr = div >> 1U;
    }
    // divider range [512, 254*256]
    else
    {
        // for each cpsr, we can search scr from 1 to 256, however we can narrow down the range for symmetry nature
        for (cpsr = 4U; cpsr <= 254U; cpsr += 2U)
        {
            uint32_t scr_mid;
            // search upper triangular matrix
            uint32_t scr_left = cpsr >> 1U;
            uint32_t scr_right = 256U;

            // scr locates outside of current range [cpsr >> 1, 256]
            if (div >= (cpsr << 8U))
            {
                scr = 256U;
            }
            // scr locates in [cpsr >> 1, 256], then binary search
            else
            {
                while (scr_left <= scr_right)
                {
                    scr_mid = (scr_right + scr_left) >> 1U;
                    if (div > cpsr * scr_mid)
                        scr_left = scr_mid + 1U;
                    else
                        scr_right = scr_mid - 1U;
                }
                if (cpsr * scr_left - div < div - cpsr * scr_right)
                {
                    scr = scr_left;
                }
                else
                {
                    scr = scr_right;
                }

            }
            // update best_div value
            if (DISTANCE(div, best_div) >= DISTANCE(div, cpsr * scr))
            {
                best_scr = scr;
                best_cpsr = cpsr;
                best_div = best_cpsr * best_scr;
                if (best_div == div)
                    break;
            }
        }
    }

    spi->reg->CPSR = best_cpsr & SPI_CPSR_CPSDVSR_Msk;
    spi->reg->CR0 = ((spi->reg->CR0 & ~SPI_CR0_SCR_Msk) | EIGEN_VAL2FLD(SPI_CR0_SCR, best_scr - 1));

    spi->reg->RPTCR = 0x2;

#if 0
    // Enable master delay sampling
    spi->reg->MPSR = SPI_MPSR_VALUE_Msk;
#endif
    spi->info->bus_speed = spi_clock / (best_cpsr * best_scr);

    return ARM_DRIVER_OK;

}

/**
  \fn          static int32_t SPI_SetSlaveRxTimeout(uint32_t bps, SPI_RESOURCES *spi)
  \brief       Set rx timeout based on bps
  \param[in]   bps       bus speed
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
static int32_t SPI_SetSlaveRxTimeout(uint32_t bps, SPI_RESOURCES *spi)
{
    uint32_t instance, spi_clock = 0;

    uint32_t cpsr, timeout_cycles;
    uint32_t div = 0;

    if(bps == 0)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    instance = SPI_GetInstanceNumber(spi);

#ifdef FPGA_TEST
    spi_clock = 102400000;
#else
    spi_clock = GPR_getClockFreq(g_spiClocks[instance*2+1]);
#endif

    /*
        The frequency of rx timeout counter is spi_clock/(cpsr/2), timeout counter clears on every edge of input clock.
        Minimum rx timeout requirement:
        timeout > T_clkin/2, that's, timeout_cycles*(cpsr/2)/spi_clock > (1/bps)/2

        Some manipulations,
        timeout_cycles*cpsr > spi_clock/bps

        In fact, we shall set timeout to be much longer, such as, 16 bits duriation, in such case

        timeout_cycles*cpsr = 32*floor(spi_clock/bps)

    */

    div = spi_clock / bps;

    if((div == 0) || (div > (8*254)))
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }
    else
    {
        div <<= 5;

        // for each cpsr, search timeout_cycles from 1 to 256
        for(cpsr = 2; cpsr <= 254; cpsr += 2)
        {
            // cell(N/M) = floor((N-1)/M) + 1 (0<M<=N)
            timeout_cycles = (div - 1) / cpsr + 1;

            if(timeout_cycles > 256)
            {
                continue;
            }
            else
            {
                break;
            }

        }

        if(cpsr == 256)
        {
            return ARM_DRIVER_ERROR_PARAMETER;
        }
        else
        {
            spi->reg->CPSR = cpsr & SPI_CPSR_CPSDVSR_Msk;
            spi->reg->RPTCR = (timeout_cycles - 1) & SPI_RPTCR_VALUE_Msk;
        }
    }

    return ARM_DRIVER_OK;

}

/**
  \fn          int32_t SPI_Initialize(ARM_SPI_SignalEvent_t cb_event, SPI_RESOURCES *spi)
  \brief       Initialize SPI Interface.
  \param[in]   cb_event  Pointer to \ref ARM_SPI_SignalEvent
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_Initialize(ARM_SPI_SignalEvent_t cb_event, SPI_RESOURCES *spi)
{
    int32_t returnCode;

#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif

    PadConfig_t config;

    if (spi->info->flags & SPI_FLAG_INITIALIZED)
        return ARM_DRIVER_OK;

#ifdef PM_FEATURE_ENABLE
        instance = SPI_GetInstanceNumber(spi);
        g_spiDataBase[instance].isInited = true;
#endif

    // Initialize SPI PINS
    PAD_getDefaultConfig(&config);
    config.mux = spi->pins.pin_sclk->funcNum;
    PAD_setPinConfig(spi->pins.pin_sclk->pinNum, &config);

    if(spi->pins.pin_ssn)
    {
        config.mux = spi->pins.pin_ssn->funcNum;
        PAD_setPinConfig(spi->pins.pin_ssn->pinNum, &config);
    }

    config.mux = spi->pins.pin_mosi->funcNum;
    PAD_setPinConfig(spi->pins.pin_mosi->pinNum, &config);
    config.mux = spi->pins.pin_miso->funcNum;
    PAD_setPinConfig(spi->pins.pin_miso->pinNum, &config);

    // Initialize SPI run-time resources
    spi->info->cb_event          = cb_event;

    spi->info->status.busy       = 0;
    spi->info->status.data_lost  = 0;

    // Clear transfer information
    memset(&(spi->info->xfer), 0, sizeof(SPI_TRANSFER_INFO));

    spi->info->mode              = 0;

    // Configure DMA if necessary
    if (spi->dma)
    {
        returnCode = DMA_openChannel(spi->dma->tx_instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            spi->dma->tx_ch = returnCode;

        DMA_setChannelRequestSource(spi->dma->tx_instance, spi->dma->tx_ch, (DmaRequestSource_e)spi->dma->tx_req);

        returnCode = DMA_openChannel(spi->dma->rx_instance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
            return ARM_DRIVER_ERROR;
        else
            spi->dma->rx_ch = returnCode;

        DMA_setChannelRequestSource(spi->dma->rx_instance, spi->dma->rx_ch, (DmaRequestSource_e)spi->dma->rx_req);
        DMA_rigisterChannelCallback(spi->dma->rx_instance, spi->dma->rx_ch, spi->dma->rx_callback);
    }

    spi->info->flags = SPI_FLAG_INITIALIZED;  // SPI is initialized

#ifdef PM_FEATURE_ENABLE
    g_spiInitCounter++;

    if(g_spiInitCounter == 1U)
    {
        g_spiWorkingStatus = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_SPI_MODULE, SPI_EnterLowPowerStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_SPI_MODULE, SPI_ExitLowPowerStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;

}


/**
  \fn          int32_t SPI_Uninitialize(SPI_RESOURCES *ssp)
  \brief       De-initialize SPI Interface.
  \param[in]   spi  Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_Uninitialize(SPI_RESOURCES *spi)
{
    if(spi->dma)
    {
        DMA_closeChannel(spi->dma->tx_instance, spi->dma->tx_ch);
        DMA_closeChannel(spi->dma->rx_instance, spi->dma->rx_ch);
    }

#ifdef PM_FEATURE_ENABLE
    uint32_t instance;

    instance = SPI_GetInstanceNumber(spi);

    g_spiDataBase[instance].isInited = false;

    g_spiInitCounter--;

    if(g_spiInitCounter == 0)
    {
        g_spiWorkingStatus = 0;
        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_SPI_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_SPI_MODULE);
    }
#endif

    spi->info->flags = 0;                    // SPI is uninitialized

    return ARM_DRIVER_OK;
}

static void SPI_DefValueTxDmaConfig(SPI_RESOURCES *spi)
{
    DmaTransferConfig_t dmaTxConfig;
    DmaExtraConfig_t extraConfig;

    // Configure tx DMA and start it
    dmaTxConfig.flowControl = DMA_FLOW_CONTROL_TARGET;
    dmaTxConfig.dataWidth = (DmaDataWidth_e)spi->info->data_width;
    dmaTxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_NONE;
    dmaTxConfig.sourceAddress = (void *)&spi->info->xfer.def_val;
    dmaTxConfig.targetAddress = (void *)&(spi->reg->DR);
    dmaTxConfig.totalLength   = SPI_FIFO_DEPTH * spi->info->data_width;
    dmaTxConfig.burstSize = DMA_BURST_4_BYTES;

    extraConfig.stopDecriptorFetch       = false;
    extraConfig.enableStartInterrupt     = false;
    extraConfig.enableEndInterrupt       = false;
    extraConfig.nextDesriptorAddress     = (void *)(spi->dma->descriptor);

    DMA_buildDescriptor(spi->dma->descriptor, &dmaTxConfig, &extraConfig);

    DMA_resetChannel(spi->dma->tx_instance, spi->dma->tx_ch);

}

static void SPI_swReset(uint32_t instance)
{
    spi_backup_register_t           backup_registers;

    backup_registers.CR0 = g_spiBases[instance]->CR0;
    backup_registers.CR1 = g_spiBases[instance]->CR1;
    backup_registers.CPSR = g_spiBases[instance]->CPSR;
    backup_registers.IMSC = g_spiBases[instance]->IMSC;
    backup_registers.DMACR = g_spiBases[instance]->DMACR;
    backup_registers.DRLR = g_spiBases[instance]->DRLR;
    backup_registers.DRRCR = g_spiBases[instance]->DRRCR;
    backup_registers.DTRCR = g_spiBases[instance]->DTRCR;
    backup_registers.RPTCR = g_spiBases[instance]->RPTCR;
    backup_registers.SRLR = g_spiBases[instance]->SRLR;
    backup_registers.MPSR = g_spiBases[instance]->MPSR;

    GPR_swResetModule(&g_spiResetVectors[instance]);

    g_spiBases[instance]->CR0 = backup_registers.CR0;
    g_spiBases[instance]->CPSR = backup_registers.CPSR;
    g_spiBases[instance]->IMSC = backup_registers.IMSC;
    g_spiBases[instance]->DMACR = backup_registers.DMACR;
    g_spiBases[instance]->DRLR = backup_registers.DRLR;
    g_spiBases[instance]->DRRCR = backup_registers.DRRCR;
    g_spiBases[instance]->DTRCR = backup_registers.DTRCR;
    g_spiBases[instance]->RPTCR = backup_registers.RPTCR;
    g_spiBases[instance]->SRLR = backup_registers.SRLR;
    g_spiBases[instance]->MPSR = backup_registers.MPSR;

    g_spiBases[instance]->CR1 = backup_registers.CR1;

}

/**
  \fn          int32_t SPI_PowerControl(ARM_POWER_STATE state, SPI_RESOURCES *spi)
  \brief       Control SPI Interface Power.
  \param[in]   state  Power state
  \param[in]   spi    Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_PowerControl(ARM_POWER_STATE state, SPI_RESOURCES *spi)
{
    uint32_t instance = SPI_GetInstanceNumber(spi);

    switch (state)
    {
        case ARM_POWER_OFF:

            if(spi->irq)
            {
                // Disable SPI IRQ
                XIC_DisableIRQ(spi->irq->irq_num);

                // Clear pending SPI interrupts
                XIC_ClearPendingIRQ(spi->irq->irq_num);
            }

            // DMA disable
            if(spi->dma)
            {
                DMA_stopChannel(spi->dma->tx_instance, spi->dma->tx_ch, true);
                DMA_stopChannel(spi->dma->rx_instance, spi->dma->rx_ch, true);
            }

            // Reset register values
            spi->reg->IMSC  = 0;
            spi->reg->DMACR = 0;
            spi->reg->CR0   = 0;
            spi->reg->CR1   = 0;
            spi->reg->CPSR  = 0;
            spi->reg->ICR   = (SPI_ICR_RTIC_Msk | SPI_ICR_RORIC_Msk);

            // Disable SPI clock
            CLOCK_clockDisable(g_spiClocks[instance*2]);
            CLOCK_clockDisable(g_spiClocks[instance*2+1]);

            // Clear SPI run-time resources
            spi->info->status.busy       = 0;
            spi->info->status.data_lost  = 0;

            // Clear transfer information
            memset(&(spi->info->xfer), 0, sizeof(SPI_TRANSFER_INFO));

            spi->info->mode  = 0;

            spi->info->flags &= ~SPI_FLAG_POWERED;
            break;

        case ARM_POWER_FULL:
            if ((spi->info->flags & SPI_FLAG_INITIALIZED) == 0)
                return ARM_DRIVER_ERROR;

            if (spi->info->flags & SPI_FLAG_POWERED)
                return ARM_DRIVER_OK;

            // Enable spi clock
            CLOCK_clockEnable(g_spiClocks[instance*2]);
            CLOCK_clockEnable(g_spiClocks[instance*2+1]);

            GPR_swResetModule(&g_spiResetVectors[instance]);

            // Clear interrupts
            spi->reg->IMSC = 0;
            spi->reg->ICR  = (SPI_ICR_RTIC_Msk | SPI_ICR_RORIC_Msk);

            // setup DMA setting registers
            spi->reg->DRLR = EIGEN_VAL2FLD(SPI_DRLR_TX_REQ_LVL, 8) | EIGEN_VAL2FLD(SPI_DRLR_RX_REQ_LVL, 9);

            spi->reg->DRRCR = EIGEN_VAL2FLD(SPI_DRRCR_RX_WAIT_CYCLE, 0x7) | \
                              EIGEN_VAL2FLD(SPI_DRRCR_READ_DEPTH_ONE_BURST, 0x7) | \
                              SPI_DRRCR_RX_REQ_MODE_Msk | SPI_DRRCR_RX_TO_REQ_EN_Msk | SPI_DRRCR_RX_EOR_MODE_Msk;

            spi->reg->DTRCR = EIGEN_VAL2FLD(SPI_DTRCR_TX_WAIT_CYCLE, 0x7) | \
                              EIGEN_VAL2FLD(SPI_DTRCR_WRITE_DEPTH_ONE_BURST, 0x7);

            // setup FIFO interrupt trigger level
            if(spi->dma)
            {
                // We can lock sleep upon receiving one data for slave receiving transcation
                spi->reg->SRLR = EIGEN_VAL2FLD(SPI_SRLR_TX_REQ_LVL, 8) | EIGEN_VAL2FLD(SPI_SRLR_RX_REQ_LVL, 1);
            }
            else if(spi->irq)
            {
                spi->reg->SRLR = EIGEN_VAL2FLD(SPI_SRLR_TX_REQ_LVL, 8) | EIGEN_VAL2FLD(SPI_SRLR_RX_REQ_LVL, 9);
            }

            // Reset SPI run-time resources
            spi->info->status.busy       = 0;
            spi->info->status.data_lost  = 0;

            // Set power flag
            spi->info->flags |= SPI_FLAG_POWERED;

            // Clear RX FIFO
            while((spi->reg->SR & SPI_SR_RNE_Msk) != 0)
            {
                spi->reg->DR;
            }

            // Enable spi interrupt
            if(spi->irq)
            {
                XIC_SetVector(spi->irq->irq_num, spi->irq->cb_irq);
                XIC_EnableIRQ(spi->irq->irq_num);
                XIC_SuppressOvfIRQ(spi->irq->irq_num);
            }

            break;
        case ARM_POWER_LOW:
        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t SPI_Send(const void *data, uint32_t num, SPI_RESOURCES *spi)
  \brief       Start sending data to SPI transmitter.
  \param[in]   data  Pointer to buffer with data to send to SPI transmitter
  \param[in]   num   Number of data items to send
  \param[in]   ssp   Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_Send(const void *data, uint32_t num, SPI_RESOURCES *spi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif

    uint8_t data_width;

    if ((data == NULL) || (num == 0))
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return ARM_DRIVER_ERROR;

    if (spi->info->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

#ifdef PM_FEATURE_ENABLE
    instance = SPI_GetInstanceNumber(spi);
#endif

    // set busy flag
    spi->info->status.busy       = 1U;
    spi->info->status.data_lost  = 0;

    spi->info->xfer.rx_buf   = NULL;
    spi->info->xfer.tx_buf   = (uint8_t *)data;
    spi->info->xfer.tx_cnt   = 0;
    spi->info->xfer.rx_cnt   = 0;
    spi->info->xfer.num      = num;

    // whether occupys 2 bytes
    data_width = spi->info->data_width;

    // dma mode
    if(spi->dma)
    {
        DmaTransferConfig_t dmaTxConfig, dmaRxConfig;

#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif

        // Configure tx DMA and start it
        dmaTxConfig.flowControl = DMA_FLOW_CONTROL_TARGET;
        dmaTxConfig.dataWidth = (DmaDataWidth_e)data_width;
        dmaTxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_SOURCE;
        dmaTxConfig.sourceAddress = (void *)data;
        dmaTxConfig.targetAddress = (void *)&(spi->reg->DR);
        dmaTxConfig.totalLength   = num * data_width;
        dmaTxConfig.burstSize = (data_width == 2)? DMA_BURST_16_BYTES : DMA_BURST_8_BYTES;

        DMA_transferSetup(spi->dma->tx_instance, spi->dma->tx_ch, &dmaTxConfig);
        DMA_startChannel(spi->dma->tx_instance, spi->dma->tx_ch);

        // retrieve data from RX FIFO to get rid of overflow

        dmaRxConfig.flowControl = DMA_FLOW_CONTROL_SOURCE;
        dmaRxConfig.dataWidth = (DmaDataWidth_e)data_width;
        dmaRxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_NONE;
        dmaRxConfig.sourceAddress = (void *)&(spi->reg->DR);
        dmaRxConfig.targetAddress = (void *)&spi->info->xfer.dump_val;
        dmaRxConfig.totalLength   = num * data_width;
        dmaRxConfig.burstSize = DMA_BURST_4_BYTES;

        DMA_transferSetup(spi->dma->rx_instance, spi->dma->rx_ch, &dmaRxConfig);
        DMA_enableChannelInterrupts(spi->dma->rx_instance, spi->dma->rx_ch, DMA_END_INTERRUPT_ENABLE | DMA_EOR_INTERRUPT_ENABLE);
        DMA_startChannel(spi->dma->rx_instance, spi->dma->rx_ch);

        // Enable DMA
        spi->reg->DMACR = (SPI_DMACR_TXDMAE_Msk | SPI_DMACR_RXDMAE_Msk);
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

    }
    // irq mode
    else if(spi->irq)
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif
        spi->reg->IMSC = SPI_IMSC_TXIM_Msk | SPI_IMSC_RXIM_Msk | SPI_IMSC_RTIM_Msk | SPI_IMSC_RORIM_Msk;
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;
    }
    // polling mode
    else
    {
        SPI_INFO *info = spi->info;

        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

        while(info->xfer.num > info->xfer.rx_cnt)
        {
            /* tx flow, the differ between tx_cnt and rx_cnt shall be less than SPI_FIFO_DEPTH(16) to get rid of rx fifo overflow,
               for the max number of tx chunk is 16+1(fifo_depth + shift_register)
            */
            if((spi->reg->SR & SPI_SR_TNF_Msk) &&
               (info->xfer.tx_cnt < info->xfer.num) &&
               ((info->xfer.tx_cnt - info->xfer.rx_cnt) < SPI_FIFO_DEPTH))
            {
                if(data_width == 2U)
                    spi->reg->DR = *((uint16_t *)(info->xfer.tx_buf + (info->xfer.tx_cnt << 1U)));
                else
                    spi->reg->DR = info->xfer.tx_buf[info->xfer.tx_cnt];
                info->xfer.tx_cnt++;
            }

            // rx flow, retrieve data from RX FIFO to get rid of overflow
            while(spi->reg->SR & SPI_SR_RNE_Msk)
            {
                (void)spi->reg->DR;
                info->xfer.rx_cnt++;
            }
        }

        info->status.busy = 0;
    }

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t SPI_Receive(void *data, uint32_t num, SPI_RESOURCES *spi)
  \brief       Start receiving data from SPI receiver.
  \param[out]  data  Pointer to buffer for data to receive from SPI receiver
  \param[in]   num   Number of data items to receive
  \param[in]   ssp   Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_Receive(void *data, uint32_t num, SPI_RESOURCES *spi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif

    uint8_t data_width;
    SPI_INFO *info = spi->info;

    if ((data == NULL) || (num == 0))
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return ARM_DRIVER_ERROR;

    if (spi->info->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

#ifdef PM_FEATURE_ENABLE
    instance = SPI_GetInstanceNumber(spi);
#endif

    // set busy flag
    info->status.busy       = 1U;
    info->status.data_lost  = 0;

    info->xfer.rx_buf   = (uint8_t *)data;
    info->xfer.tx_buf   = NULL;
    info->xfer.tx_cnt   = 0;
    info->xfer.rx_cnt   = 0;
    info->xfer.num      = num;

    // whether occupys 2 bytes
    data_width = info->data_width;

    // dma mode
    if(spi->dma)
    {
        DmaTransferConfig_t dmaTxConfig, dmaRxConfig;

        // Configure rx DMA and start it
        dmaRxConfig.flowControl = DMA_FLOW_CONTROL_SOURCE;
        dmaRxConfig.dataWidth = (DmaDataWidth_e)data_width;
        dmaRxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_TARGET;
        dmaRxConfig.sourceAddress = (void *)&(spi->reg->DR);
        dmaRxConfig.targetAddress = (void *)data;
        dmaRxConfig.totalLength   = num * data_width;
        dmaRxConfig.burstSize = (data_width == 2)? DMA_BURST_16_BYTES : DMA_BURST_8_BYTES;

        DMA_transferSetup(spi->dma->rx_instance, spi->dma->rx_ch, &dmaRxConfig);
        DMA_enableChannelInterrupts(spi->dma->rx_instance, spi->dma->rx_ch, DMA_END_INTERRUPT_ENABLE | DMA_EOR_INTERRUPT_ENABLE);
        DMA_startChannel(spi->dma->rx_instance, spi->dma->rx_ch);

        spi->reg->IMSC = SPI_IMSC_RORIM_Msk;

        if((info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER)
        {
#ifdef PM_FEATURE_ENABLE
            LOCK_SLEEP(instance);
#endif

            // Configure tx DMA and start it
            dmaTxConfig.flowControl = DMA_FLOW_CONTROL_TARGET;
            dmaTxConfig.dataWidth = (DmaDataWidth_e)data_width;
            dmaTxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_NONE;
            dmaTxConfig.sourceAddress = (void *)&spi->info->xfer.def_val;
            dmaTxConfig.targetAddress = (void *)&(spi->reg->DR);
            dmaTxConfig.totalLength   = num * data_width;
            dmaTxConfig.burstSize = DMA_BURST_4_BYTES;

            DMA_transferSetup(spi->dma->tx_instance, spi->dma->tx_ch, &dmaTxConfig);
        }
        else
        {
            // Enable DMA
            // For slave mode, we don't know when the transaction will start, so we don't lock sleep here, we'll do this upon first data reception.
            DMA_loadChannelFirstDescriptor(spi->dma->tx_instance, spi->dma->tx_ch, (void *)spi->dma->descriptor);
            spi->reg->IMSC |= SPI_IMSC_RXIM_Msk; // trigger for sleep vote
        }

        DMA_startChannel(spi->dma->tx_instance, spi->dma->tx_ch);
        spi->reg->DMACR = SPI_DMACR_TXDMAE_Msk | SPI_DMACR_RXDMAE_Msk;
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;
    }
    // ird mode
    else if(spi->irq)
    {
        spi->reg->IMSC = SPI_IMSC_TXIM_Msk | SPI_IMSC_RXIM_Msk | SPI_IMSC_RTIM_Msk | SPI_IMSC_RORIM_Msk;
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;
    }
    // polling mode
    else
    {
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

        while(info->xfer.num > info->xfer.rx_cnt)
        {
            /* tx flow, the differ between tx_cnt and rx_cnt shall be less than SPI_FIFO_DEPTH(16) to get rid of rx fifo overflow,
               for the max number of tx chunk is 16+1(fifo_depth + shift_register)
            */
            if((spi->reg->SR & SPI_SR_TNF_Msk) &&
               (info->xfer.tx_cnt < info->xfer.num) &&
               ((info->xfer.tx_cnt - info->xfer.rx_cnt) < SPI_FIFO_DEPTH))
            {
                spi->reg->DR = info->xfer.def_val;
                info->xfer.tx_cnt++;
            }

            // rx flow
            while(spi->reg->SR & SPI_SR_RNE_Msk)
            {
                if(data_width == 2U)
                    *((uint16_t *)(info->xfer.rx_buf + (info->xfer.rx_cnt << 1U))) = spi->reg->DR;
                else
                    info->xfer.rx_buf[info->xfer.rx_cnt] = (uint8_t)(spi->reg->DR);

                info->xfer.rx_cnt++;
            }
        }


        spi->info->status.busy = 0;

    }
    return ARM_DRIVER_OK;

}


/**
  \fn          int32_t SPI_Transfer(const void          *data_out,
                                          void          *data_in,
                                          uint32_t       num,
                                          SPI_RESOURCES *spi)
  \brief       Start sending/receiving data to/from SPI transmitter/receiver.
  \param[in]   data_out  Pointer to buffer with data to send to SPI transmitter
  \param[out]  data_in   Pointer to buffer for data to receive from SPI receiver
  \param[in]   num       Number of data items to transfer
  \param[in]   ssp       Pointer to SPI resources
  \return      \ref execution_status
*/
int32_t SPI_Transfer(const void *data_out, void *data_in, uint32_t num, SPI_RESOURCES *spi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif
    uint8_t data_width;

    if ((data_out == NULL) || (data_in == NULL) || (num == 0))
        return ARM_DRIVER_ERROR_PARAMETER;

    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return ARM_DRIVER_ERROR;

    if (spi->info->status.busy)
        return ARM_DRIVER_ERROR_BUSY;

#ifdef PM_FEATURE_ENABLE
    instance = SPI_GetInstanceNumber(spi);
#endif

    // set busy flag
    spi->info->status.busy       = 1U;
    spi->info->status.data_lost  = 0;

    spi->info->xfer.rx_buf   = (uint8_t *)data_in;
    spi->info->xfer.tx_buf   = (uint8_t *)data_out;
    spi->info->xfer.tx_cnt   = 0;
    spi->info->xfer.rx_cnt   = 0;
    spi->info->xfer.num      = num;

    // whether occupys 2 bytes
    data_width = spi->info->data_width;

    if(spi->dma)
    {
        DmaTransferConfig_t dmaTxConfig, dmaRxConfig;

#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif

        // Configure tx DMA and start it
        dmaTxConfig.flowControl = DMA_FLOW_CONTROL_TARGET;
        dmaTxConfig.dataWidth = (DmaDataWidth_e)data_width;
        dmaTxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_SOURCE;
        dmaTxConfig.sourceAddress = (void *)data_out;
        dmaTxConfig.targetAddress = (void *)&(spi->reg->DR);
        dmaTxConfig.totalLength   = num * data_width;
        dmaTxConfig.burstSize = (data_width == 2)? DMA_BURST_16_BYTES : DMA_BURST_8_BYTES;

        DMA_transferSetup(spi->dma->tx_instance, spi->dma->tx_ch, &dmaTxConfig);
        DMA_startChannel(spi->dma->tx_instance, spi->dma->tx_ch);

        // Configure rx DMA and start it
        dmaRxConfig.flowControl = DMA_FLOW_CONTROL_SOURCE;
        dmaRxConfig.dataWidth = (DmaDataWidth_e)data_width;
        dmaRxConfig.addressIncrement = DMA_ADDRESS_INCREMENT_TARGET;
        dmaRxConfig.sourceAddress = (void *)&(spi->reg->DR);
        dmaRxConfig.targetAddress = (void *)data_in;
        dmaRxConfig.totalLength   = num * data_width;
        dmaRxConfig.burstSize = (data_width == 2)? DMA_BURST_16_BYTES : DMA_BURST_8_BYTES;

        DMA_transferSetup(spi->dma->rx_instance, spi->dma->rx_ch, &dmaRxConfig);
        DMA_enableChannelInterrupts(spi->dma->rx_instance, spi->dma->rx_ch, DMA_END_INTERRUPT_ENABLE | DMA_EOR_INTERRUPT_ENABLE);
        DMA_startChannel(spi->dma->rx_instance, spi->dma->rx_ch);

        spi->reg->IMSC = SPI_IMSC_RORIM_Msk;

        // Enable DMA
        spi->reg->DMACR = (SPI_DMACR_TXDMAE_Msk | SPI_DMACR_RXDMAE_Msk);
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

    }
    else if(spi->irq)
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif
        spi->reg->IMSC = SPI_IMSC_TXIM_Msk | SPI_IMSC_RXIM_Msk | SPI_IMSC_RTIM_Msk | SPI_IMSC_RORIM_Msk;
        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

    }
    else
    {
        SPI_INFO *info = spi->info;

        spi->reg->CR1 |= SPI_CR1_SSE_Msk;

        while(info->xfer.num > info->xfer.rx_cnt)
        {
            /* tx flow, the differ between tx_cnt and rx_cnt shall be less than SPI_FIFO_DEPTH(16) to get rid of rx fifo overflow,
               for the max number of tx chunk is 16+1(fifo_depth + shift_register)
            */
            if((spi->reg->SR & SPI_SR_TNF_Msk) &&
               (info->xfer.tx_cnt < info->xfer.num) &&
               ((info->xfer.tx_cnt - info->xfer.rx_cnt) < SPI_FIFO_DEPTH))
            {
                if(data_width == 2U)
                    spi->reg->DR = *((uint16_t *)(info->xfer.tx_buf + (info->xfer.tx_cnt << 1U)));
                else
                    spi->reg->DR = info->xfer.tx_buf[info->xfer.tx_cnt];
                info->xfer.tx_cnt++;
            }

            // rx flow
            while(spi->reg->SR & SPI_SR_RNE_Msk)
            {
                if(data_width == 2U)
                    *((uint16_t *)(info->xfer.rx_buf + (info->xfer.rx_cnt << 1U))) = spi->reg->DR;
                else
                    info->xfer.rx_buf[info->xfer.rx_cnt] = (uint8_t)(spi->reg->DR);

                info->xfer.rx_cnt++;
            }
        }

        info->status.busy = 0;

    }
    return ARM_DRIVER_OK;

}


/**
  \fn          uint32_t SPI_GetDataCount(SPI_RESOURCES *spi)
  \brief       Get transferred data count.
  \param[in]   spi  Pointer to SPI resources
  \return      number of data items transferred
*/
uint32_t SPI_GetDataCount(SPI_RESOURCES *spi)
{
    if (!(spi->info->flags & SPI_FLAG_CONFIGURED))
        return 0;
    return spi->info->xfer.rx_cnt;
}


/**
  \fn          int32_t SPI_Control(uint32_t control, uint32_t arg, SPI_RESOURCES *spi)
  \brief       Control SPI Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   spi      Pointer to SPI resources
  \return      common \ref execution_status and driver specific \ref spi_execution_status
*/
int32_t SPI_Control(uint32_t control, uint32_t arg, SPI_RESOURCES *spi)
{
    int32_t ret;
    uint32_t dataBits;
    PadConfig_t padConfig;
    GpioPinConfig_t gpioConfig;

    uint32_t instance = SPI_GetInstanceNumber(spi);

    if(!(spi->info->flags & SPI_FLAG_POWERED))
        return ARM_DRIVER_ERROR;

    if((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_ABORT_TRANSFER)
    {
        // abort SPI transfer
        // Disable SPI and SPI interrupts
        spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;
        spi->reg->IMSC = 0;
        spi->reg->ICR = SPI_ICR_RTIC_Msk | SPI_ICR_RORIC_Msk;

        if(spi->info->status.busy)
        {
            // If DMA mode, disable DMA channel
            if(spi->dma)
            {
                DMA_stopChannel(spi->dma->tx_instance, spi->dma->tx_ch, true);
                DMA_stopChannel(spi->dma->rx_instance, spi->dma->rx_ch, true);
            }
        }

        // clear SPI run-time resources
        spi->info->status.busy = 0;
        memset(&(spi->info->xfer), 0, sizeof(SPI_TRANSFER_INFO));

#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(instance);
#endif

        return ARM_DRIVER_OK;
    }

    if((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_PURGE_COMM)
    {
        SPI_swReset(instance);
    }

    if(spi->info->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    switch(control & ARM_SPI_CONTROL_Msk)
    {
        // SPI Inactive
        case ARM_SPI_MODE_INACTIVE:

            // Disable SPI
            spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;
            // Disable interrupts
            spi->reg->IMSC = 0;
            spi->info->mode = ((spi->info->mode & ~ARM_SPI_CONTROL_Msk) | ARM_SPI_MODE_INACTIVE);
            spi->info->flags &= ~SPI_FLAG_CONFIGURED;

            return ARM_DRIVER_OK;

        // SPI Slave  (Output on MISO, Input on MOSI)
        case ARM_SPI_MODE_SLAVE:

            // Disable SPI first
            spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;

            ret = SPI_SetSlaveRxTimeout(arg, spi);

            if(ret != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;

            // Set slave mode and Enable SPI
            spi->reg->CR1 |= SPI_CR1_MS_Msk;

            spi->info->mode = ((spi->info->mode & ~ARM_SPI_CONTROL_Msk) | ARM_SPI_MODE_SLAVE);
            spi->info->flags |= SPI_FLAG_CONFIGURED;

            break;

        // SPI Master (Output/Input on MOSI); arg = Bus Speed in bps
        case ARM_SPI_MODE_MASTER_SIMPLEX:

        // SPI Slave  (Output/Input on MISO)
        case ARM_SPI_MODE_SLAVE_SIMPLEX:
            return ARM_SPI_ERROR_MODE;

        // SPI Master (Output on MOSI, Input on MISO); arg = Bus Speed in bps
        case ARM_SPI_MODE_MASTER:

        // Set Bus Speed in bps; arg = value
        case ARM_SPI_SET_BUS_SPEED:

            if((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER)
            {
                // Disable SPI first
                spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;

                // Disable interrupts
                spi->reg->IMSC = 0;

                // Set master mode
                spi->reg->CR1 = spi->reg->CR1 & ~SPI_CR1_MS_Msk;

                spi->info->mode = ((spi->info->mode & ~ARM_SPI_CONTROL_Msk) | ARM_SPI_MODE_MASTER);
                spi->info->flags |= SPI_FLAG_CONFIGURED;

            }

            ret = SPI_SetBusSpeed(arg, spi);

            if(ret != ARM_DRIVER_OK)
                return ARM_DRIVER_ERROR;

            if((control & ARM_SPI_CONTROL_Msk) == ARM_SPI_SET_BUS_SPEED)
            {
                return ARM_DRIVER_OK;
            }
            break;

        // Get Bus Speed in bps
        case ARM_SPI_GET_BUS_SPEED:
            return spi->info->bus_speed;

        // Set default Transmit value; arg = value
        case ARM_SPI_SET_DEFAULT_TX_VALUE:
            spi->info->xfer.def_val = (arg & 0xffff) | ((arg & 0xffff) << 16);
            return ARM_DRIVER_OK;

        // Control Slave Select; arg = 0:inactive, 1:active
        case ARM_SPI_CONTROL_SS:
            if(SPI0 == spi->reg)
            {
                GPIO_pinWrite(RTE_SPI0_SSN_GPIO_INSTANCE, 1 << RTE_SPI0_SSN_GPIO_INDEX, arg << RTE_SPI0_SSN_GPIO_INDEX);
            }
#if (RTE_SPI1)
            else if(SPI1 == spi->reg)
            {
                GPIO_pinWrite(RTE_SPI1_SSN_GPIO_INSTANCE, 1 << RTE_SPI1_SSN_GPIO_INDEX, arg << RTE_SPI1_SSN_GPIO_INDEX);
            }
#endif
            return ARM_DRIVER_OK;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

    }

    // SPI slave select mode for master
    if((spi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_MASTER)
    {
        switch(control & ARM_SPI_SS_MASTER_MODE_Msk)
        {
            // SPI Slave Select when Master: Not used(default)
            case ARM_SPI_SS_MASTER_UNUSED:
                spi->info->mode = (spi->info->mode & ~ARM_SPI_SS_MASTER_MODE_Msk) | ARM_SPI_SS_MASTER_UNUSED;
                break;

            case ARM_SPI_SS_MASTER_SW:

                PAD_getDefaultConfig(&padConfig);
                padConfig.mux = PAD_MUX_ALT0;
                PAD_setPinConfig(spi->pins.pin_ssn->pinNum, &padConfig);

                gpioConfig.pinDirection = GPIO_DIRECTION_OUTPUT;
                gpioConfig.misc.initOutput = 1U;

                if(SPI0 == spi->reg)
                {
                    GPIO_pinConfig(RTE_SPI0_SSN_GPIO_INSTANCE, RTE_SPI0_SSN_GPIO_INDEX, &gpioConfig);
                }
#if (RTE_SPI1)
                else if(SPI1 == spi->reg)
                {
                    GPIO_pinConfig(RTE_SPI1_SSN_GPIO_INSTANCE, RTE_SPI1_SSN_GPIO_INDEX, &gpioConfig);
                }
#endif
                spi->info->mode = (spi->info->mode & ~ARM_SPI_SS_MASTER_MODE_Msk) | ARM_SPI_SS_MASTER_SW;

                break;
            case ARM_SPI_SS_MASTER_HW_OUTPUT:

                PAD_getDefaultConfig(&padConfig);
                padConfig.mux = spi->pins.pin_ssn->funcNum;
                PAD_setPinConfig(spi->pins.pin_ssn->pinNum, &padConfig);

                spi->info->mode = (spi->info->mode & ~ARM_SPI_SS_MASTER_MODE_Msk) | ARM_SPI_SS_MASTER_HW_OUTPUT;

                break;
            case ARM_SPI_SS_MASTER_HW_INPUT:
                return ARM_SPI_ERROR_SS_MODE;
            default:
                break;
        }
    }

    // SPI slave select mode for slave
    if((spi->info->mode & ARM_SPI_CONTROL_Msk) == ARM_SPI_MODE_SLAVE)
    {
        switch(control & ARM_SPI_SS_SLAVE_MODE_Msk)
        {
            case ARM_SPI_SS_SLAVE_HW:
                spi->info->mode &= ~ARM_SPI_SS_SLAVE_MODE_Msk;
                spi->info->mode |= ARM_SPI_SS_SLAVE_HW;
                break;
            case ARM_SPI_SS_SLAVE_SW:
                return ARM_SPI_ERROR_SS_MODE;
            default:
                break;
        }
    }

    // set SPI frame format
    switch(control & ARM_SPI_FRAME_FORMAT_Msk)
    {
        case ARM_SPI_CPOL0_CPHA0:
            spi->reg->CR0 &= ~(SPI_CR0_FRF_Msk | SPI_CR0_SPO_Msk | SPI_CR0_SPH_Msk);
            break;
        case ARM_SPI_CPOL0_CPHA1:
            spi->reg->CR0 = (spi->reg->CR0 & ~(SPI_CR0_FRF_Msk | SPI_CR0_SPO_Msk)) | SPI_CR0_SPH_Msk;
            break;
        case ARM_SPI_CPOL1_CPHA0:
            spi->reg->CR0 = (spi->reg->CR0 & ~(SPI_CR0_FRF_Msk | SPI_CR0_SPH_Msk)) | SPI_CR0_SPO_Msk;
            break;
        case ARM_SPI_CPOL1_CPHA1:
            spi->reg->CR0 = (spi->reg->CR0 & ~SPI_CR0_FRF_Msk) | SPI_CR0_SPO_Msk | SPI_CR0_SPH_Msk;
            break;
        case ARM_SPI_TI_SSI:
            spi->reg->CR0 = (spi->reg->CR0 & ~SPI_CR0_FRF_Msk) | (1U << SPI_CR0_FRF_Pos);
            break;
        case ARM_SPI_MICROWIRE:
            spi->reg->CR0 = (spi->reg->CR0 & ~SPI_CR0_FRF_Msk) | (2U << SPI_CR0_FRF_Pos);
        default:
            return ARM_SPI_ERROR_FRAME_FORMAT;
    }
    // set number of data bits
    dataBits = ((control & ARM_SPI_DATA_BITS_Msk) >> ARM_SPI_DATA_BITS_Pos);

    if((dataBits >= 4U) && (dataBits <= 16U))
    {
        spi->reg->CR0 = (spi->reg->CR0 & ~SPI_CR0_DSS_Msk) | (dataBits - 1U);
        spi->info->data_width = (dataBits > 8U) ? 2U : 1U;
    }
    else
        return ARM_SPI_ERROR_DATA_BITS;

    // set SPI bit order
    if((control & ARM_SPI_BIT_ORDER_Msk) == ARM_SPI_LSB_MSB)
        return ARM_SPI_ERROR_BIT_ORDER;

    if(spi->dma)
        SPI_DefValueTxDmaConfig(spi);
    return ARM_DRIVER_OK;
}


/**
  \fn          ARM_SPI_STATUS SPI_GetStatus(SPI_RESOURCES *spi)
  \brief       Get SPI status.
  \param[in]   spi  Pointer to SPI resources
  \return      SPI status \ref ARM_SPI_STATUS
*/
ARM_SPI_STATUS SPI_GetStatus(SPI_RESOURCES *spi)
{
    ARM_SPI_STATUS status;
    status.busy       = spi->info->status.busy;
    status.data_lost  = spi->info->status.data_lost;
    return (status);
}


/**
  \fn          void SPI_IRQHandler(SPI_RESOURCES *spi)
  \brief       SPI Interrupt handler.
  \param[in]   spi Pointer to SPI resources
*/
void SPI_IRQHandler(SPI_RESOURCES *spi)
{
    uint32_t mis, data_width, imsc;
    SPI_INFO *info = spi->info;

#ifdef PM_FEATURE_ENABLE
    uint32_t instance = SPI_GetInstanceNumber(spi);
#endif

    // whether occupys 2 bytes
    data_width = info->data_width;

    mis = spi->reg->MIS;
    spi->reg->ICR = mis & (SPI_ICR_RTIC_Msk | SPI_ICR_RORIC_Msk);

    if(mis == 0)
    {
        return;
    }

    if(mis & SPI_MIS_RORMIS_Msk)
    {
        // Handle errors
        // Overrun flag is set
        info->status.data_lost = 1U;
        if(info->cb_event)
            info->cb_event(ARM_SPI_EVENT_DATA_LOST);

#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(instance);
#endif

        return;

    }

    // slave DMA receive
    if(spi->dma && (mis & SPI_MIS_RXMIS_Msk))
    {
        spi->reg->IMSC &= ~SPI_IMSC_RXIM_Msk;

#ifdef PM_FEATURE_ENABLE
        // check fifo number for case where start/end order is reversed(irq is blocked)
        if(spi->reg->SR & SPI_SR_RNE_Msk)
        {
            LOCK_SLEEP(instance);
        }
#endif
        return;
    }
    else
    {
#ifdef PM_FEATURE_ENABLE
        LOCK_SLEEP(instance);
#endif
    }

    while((spi->reg->SR & SPI_SR_TNF_Msk) &&
          (info->xfer.num > info->xfer.tx_cnt) &&
          ((info->xfer.tx_cnt - info->xfer.rx_cnt) < SPI_FIFO_DEPTH))
    {
        if(info->xfer.tx_buf)
        {
            if(data_width == 2U)
                spi->reg->DR = *((uint16_t *)(info->xfer.tx_buf + (info->xfer.tx_cnt << 1U)));
            else
                spi->reg->DR = info->xfer.tx_buf[info->xfer.tx_cnt];
        }
        else
        {
            spi->reg->DR = info->xfer.def_val;
        }
        info->xfer.tx_cnt++;
    }

    // Disable TX interrupt to get rid of interrupt storm
    if(info->xfer.num == info->xfer.tx_cnt)
    {
        spi->reg->IMSC &= ~SPI_IMSC_TXIM_Msk;
    }

    while((spi->reg->SR & SPI_SR_RNE_Msk) && (info->xfer.rx_cnt < info->xfer.num))
    {
        // Send only
        if(info->xfer.rx_buf == NULL)
        {
            (void)spi->reg->DR;
        }
        else
        {
            if(data_width == 2U)
                *((uint16_t *)(info->xfer.rx_buf + (info->xfer.rx_cnt << 1U))) = spi->reg->DR;
            else
                info->xfer.rx_buf[info->xfer.rx_cnt] = (uint8_t)(spi->reg->DR);
        }

        info->xfer.rx_cnt++;
    }



    //---------------------------------- complete criteria -------------------------------------------
    //                                    irq mode
    // tx                      rx_cnt = num && tx_cnt = num
    // rx                      rx_cnt = num && tx_cnt = num
    // tx&rx                   rx_cnt = num && tx_cnt = num
    //------------------------------------------------------------------------------------------------

    if((info->xfer.rx_cnt == info->xfer.num) || (mis & SPI_MIS_RTMIS_Msk))
    {
        // disable interrupts no matter what kind of trasaction is
        spi->reg->IMSC = 0;
        spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;
        info->status.busy = 0;
        if(info->cb_event)
        {
            info->cb_event((info->xfer.rx_cnt == info->xfer.num) ? ARM_SPI_EVENT_TRANSFER_COMPLETE : ARM_SPI_EVENT_RX_TIMEOUT);
        }
#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(instance);
#endif
    }

    // re-trigger ic controller in case there's pending irq
    imsc = spi->reg->IMSC;
    spi->reg->IMSC = 0;
    spi->reg->IMSC =imsc;
}

/**
  \fn          void SPI_DmaRxEvent(uint32_t event, SPI_RESOURCES *spi)
  \brief       SPI DMA Rx Event handler.
  \param[in]   event DMA Rx Event
  \param[in]   spi   Pointer to SPI resources
*/
void SPI_DmaRxEvent(uint32_t event, SPI_RESOURCES *spi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance = SPI_GetInstanceNumber(spi);
#endif

    switch(event)
    {
        case DMA_EVENT_EOR:
        case DMA_EVENT_END:

            spi->reg->CR1 &= ~SPI_CR1_SSE_Msk;

            DMA_stopChannel(spi->dma->tx_instance, spi->dma->tx_ch, true);
            DMA_stopChannel(spi->dma->rx_instance, spi->dma->rx_ch, true);

            spi->info->xfer.rx_cnt = DMA_getChannelCount(spi->dma->rx_instance, spi->dma->rx_ch);
            spi->info->xfer.tx_cnt = spi->info->xfer.rx_cnt;

            spi->reg->DMACR = 0;
            spi->info->status.busy = 0;

            if(spi->info->cb_event)
            {
                spi->info->cb_event((event == DMA_EVENT_END) ? ARM_SPI_EVENT_TRANSFER_COMPLETE : ARM_SPI_EVENT_RX_TIMEOUT);
            }
#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(instance);
#endif

            break;
        case DMA_EVENT_ERROR:
        default:
            break;
    }
}


#if (RTE_SPI0)

static int32_t SPI0_Initialize(ARM_SPI_SignalEvent_t pSignalEvent)
{
    return SPI_Initialize(pSignalEvent, &SPI0_Resources);
}
static int32_t SPI0_Uninitialize(void)
{
    return SPI_Uninitialize(&SPI0_Resources);
}
static int32_t SPI0_PowerControl(ARM_POWER_STATE state)
{
    return SPI_PowerControl(state, &SPI0_Resources);
}
static int32_t SPI0_Send(const void *data, uint32_t num)
{
    return SPI_Send(data, num, &SPI0_Resources);
}
static int32_t SPI0_Receive(void *data, uint32_t num)
{
    return SPI_Receive(data, num, &SPI0_Resources);
}
static int32_t SPI0_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return SPI_Transfer(data_out, data_in, num, &SPI0_Resources);
}
static uint32_t SPI0_GetDataCount(void)
{
    return SPI_GetDataCount(&SPI0_Resources);
}
static int32_t SPI0_Control(uint32_t control, uint32_t arg)
{
    return SPI_Control(control, arg, &SPI0_Resources);
}
static ARM_SPI_STATUS SPI0_GetStatus(void)
{
    return SPI_GetStatus(&SPI0_Resources);
}
void SPI0_IRQHandler(void)
{
    SPI_IRQHandler(&SPI0_Resources);
}

void SPI0_DmaRxEvent(uint32_t event)
{
    SPI_DmaRxEvent(event, &SPI0_Resources);
}

// SPI0 Driver Control Block
ARM_DRIVER_SPI Driver_SPI0 = {
    SPI_GetVersion,
    SPI_GetCapabilities,
    SPI0_Initialize,
    SPI0_Uninitialize,
    SPI0_PowerControl,
    SPI0_Send,
    SPI0_Receive,
    SPI0_Transfer,
    SPI0_GetDataCount,
    SPI0_Control,
    SPI0_GetStatus
};

#endif

#if (RTE_SPI1)

static int32_t SPI1_Initialize(ARM_SPI_SignalEvent_t pSignalEvent)
{
    return SPI_Initialize(pSignalEvent, &SPI1_Resources);
}
static int32_t SPI1_Uninitialize(void)
{
    return SPI_Uninitialize(&SPI1_Resources);
}
static int32_t SPI1_PowerControl(ARM_POWER_STATE state)
{
    return SPI_PowerControl(state, &SPI1_Resources);
}
static int32_t SPI1_Send(const void *data, uint32_t num)
{
    return SPI_Send(data, num, &SPI1_Resources);
}
static int32_t SPI1_Receive(void *data, uint32_t num)
{
    return SPI_Receive(data, num, &SPI1_Resources);
}
static int32_t SPI1_Transfer(const void *data_out, void *data_in, uint32_t num)
{
    return SPI_Transfer(data_out, data_in, num, &SPI1_Resources);
}
static uint32_t SPI1_GetDataCount(void)
{
    return SPI_GetDataCount(&SPI1_Resources);
}
static int32_t SPI1_Control(uint32_t control, uint32_t arg)
{
    return SPI_Control(control, arg, &SPI1_Resources);
}
static ARM_SPI_STATUS SPI1_GetStatus(void)
{
    return SPI_GetStatus(&SPI1_Resources);
}
void SPI1_IRQHandler(void)
{
    SPI_IRQHandler(&SPI1_Resources);
}

void SPI1_DmaRxEvent(uint32_t event)
{
    SPI_DmaRxEvent(event, &SPI1_Resources);
}

// SPI1 Driver Control Block
ARM_DRIVER_SPI Driver_SPI1 = {
    SPI_GetVersion,
    SPI_GetCapabilities,
    SPI1_Initialize,
    SPI1_Uninitialize,
    SPI1_PowerControl,
    SPI1_Send,
    SPI1_Receive,
    SPI1_Transfer,
    SPI1_GetDataCount,
    SPI1_Control,
    SPI1_GetStatus
};

#endif
