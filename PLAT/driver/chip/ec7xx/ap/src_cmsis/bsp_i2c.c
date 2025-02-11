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
 * $Date:        13. July 2018
 * $Revision:    V2.0
 *
 * Driver:       Driver_I2C0, Driver_I2C1
 * Configured:   via RTE_Device.h configuration file
 * Project:      I2C Driver for AirM2M EC718
 * --------------------------------------------------------------------------
 * Use the following configuration settings in the middleware component
 * to connect to this driver.
 *
 *   Configuration Setting                 Value   I2C Interface
 *   ---------------------                 -----   -------------
 *   Connect to hardware via Driver_I2C# = 0       use I2C0
 *   Connect to hardware via Driver_I2C# = 1       use I2C1
 * -------------------------------------------------------------------------- */
/* History:
 *  Version 2.0
 *    - Initial CMSIS Driver API V2.0 release
 */

#include "bsp_i2c.h"
#include "slpman.h"
#include "sctdef.h"

#if 0
#pragma GCC push_options
#pragma GCC optimize("O0")
#endif

#define I2C_DEBUG  0
#if I2C_DEBUG
#define I2CDEBUG(...)     printf(__VA_ARGS__)
#else
#define I2CDEBUG(...)
#endif

#define ARM_I2C_DRV_VERSION    ARM_DRIVER_VERSION_MAJOR_MINOR(2, 0) /* driver version */

#if ((!RTE_I2C0) && (!RTE_I2C1))
#error "I2C not enabled in RTE_Device.h!"
#endif


/** \brief Internal used data structure */
typedef struct _i2c_database
{
    bool                            isInited;            /**< Whether i2c has been initialized */
    struct
    {
        uint32_t CR0;
        uint32_t TAR;
        uint32_t SAR;
        uint32_t SCLCNT[4];
        uint32_t IMR;
        uint32_t RXFCR;
        uint32_t TXFCR;
        uint32_t CR1;
        uint32_t SDAHOLD;
        uint32_t SDASETUP;
        uint32_t SPKSR;
        uint32_t STUCKTIMEOUT[2];
    } backup_registers;                                  /**< Backup registers for low power restore */
} i2c_database_t;

AP_PLAT_COMMON_BSS static i2c_database_t g_i2cDataBase[I2C_INSTANCE_NUM] = {0};

static I2C_TypeDef* const g_i2cBases[] = I2C_INSTANCE_ARRAY;

AP_PLAT_COMMON_DATA static ClockId_e g_i2cClocks[] = I2C_CLOCK_VECTOR;

static const ClockResetVector_t g_i2cResetVectors[] = I2C_RESET_VECTORS;

static void I2C_BackupRegs(uint32_t instance)
{
    g_i2cDataBase[instance].backup_registers.CR0 = g_i2cBases[instance]->CR0;
    g_i2cDataBase[instance].backup_registers.TAR = g_i2cBases[instance]->TAR;
    g_i2cDataBase[instance].backup_registers.SAR = g_i2cBases[instance]->SAR;
    g_i2cDataBase[instance].backup_registers.SCLCNT[0] = g_i2cBases[instance]->SCLCNT[0];
    g_i2cDataBase[instance].backup_registers.SCLCNT[1] = g_i2cBases[instance]->SCLCNT[1];
    g_i2cDataBase[instance].backup_registers.SCLCNT[2] = g_i2cBases[instance]->SCLCNT[2];
    g_i2cDataBase[instance].backup_registers.SCLCNT[3] = g_i2cBases[instance]->SCLCNT[3];
    g_i2cDataBase[instance].backup_registers.IMR = g_i2cBases[instance]->IMR;
    g_i2cDataBase[instance].backup_registers.RXFCR = g_i2cBases[instance]->RXFCR;
    g_i2cDataBase[instance].backup_registers.TXFCR = g_i2cBases[instance]->TXFCR;
    g_i2cDataBase[instance].backup_registers.CR1 = g_i2cBases[instance]->CR1;
    g_i2cDataBase[instance].backup_registers.SDAHOLD = g_i2cBases[instance]->SDAHOLD;
    g_i2cDataBase[instance].backup_registers.SDASETUP = g_i2cBases[instance]->SDASETUP;
    g_i2cDataBase[instance].backup_registers.SPKSR = g_i2cBases[instance]->SPKSR;
    g_i2cDataBase[instance].backup_registers.STUCKTIMEOUT[0] = g_i2cBases[instance]->STUCKTIMEOUT[0];
    g_i2cDataBase[instance].backup_registers.STUCKTIMEOUT[1] = g_i2cBases[instance]->STUCKTIMEOUT[1];

}

static void I2C_RestoreRegs(uint32_t instance)
{
    g_i2cBases[instance]->CR0 = g_i2cDataBase[instance].backup_registers.CR0;
    g_i2cBases[instance]->TAR = g_i2cDataBase[instance].backup_registers.TAR;
    g_i2cBases[instance]->SAR = g_i2cDataBase[instance].backup_registers.SAR;
    g_i2cBases[instance]->IMR = g_i2cDataBase[instance].backup_registers.IMR;
    g_i2cBases[instance]->RXFCR = g_i2cDataBase[instance].backup_registers.RXFCR;
    g_i2cBases[instance]->TXFCR = g_i2cDataBase[instance].backup_registers.TXFCR;

    g_i2cBases[instance]->SCLCNT[0] = g_i2cDataBase[instance].backup_registers.SCLCNT[0];
    g_i2cBases[instance]->SCLCNT[1] = g_i2cDataBase[instance].backup_registers.SCLCNT[1];
    g_i2cBases[instance]->SCLCNT[2] = g_i2cDataBase[instance].backup_registers.SCLCNT[2];
    g_i2cBases[instance]->SCLCNT[3] = g_i2cDataBase[instance].backup_registers.SCLCNT[3];
    g_i2cBases[instance]->SDAHOLD = g_i2cDataBase[instance].backup_registers.SDAHOLD;
    g_i2cBases[instance]->SDASETUP = g_i2cDataBase[instance].backup_registers.SDASETUP;
    g_i2cBases[instance]->SPKSR = g_i2cDataBase[instance].backup_registers.SPKSR;

    g_i2cBases[instance]->STUCKTIMEOUT[0] = g_i2cDataBase[instance].backup_registers.STUCKTIMEOUT[0];
    g_i2cBases[instance]->STUCKTIMEOUT[1] = g_i2cDataBase[instance].backup_registers.STUCKTIMEOUT[1];

    g_i2cBases[instance]->CR1 = g_i2cDataBase[instance].backup_registers.CR1;

}

#ifdef PM_FEATURE_ENABLE
/**
  \brief i2c initialization counter, for lower power callback register/de-register
 */
AP_PLAT_COMMON_BSS static uint32_t g_i2cInitCounter = 0;

/**
  \brief Bitmap of I2C working status,
         when all I2C instances are not working, we can vote to enter to low power state.
 */

AP_PLAT_COMMON_BSS static uint32_t g_i2cWorkingStatus = 0;

/**
  \fn        static void I2C_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, I2C hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void I2C_EnterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < I2C_INSTANCE_NUM; i++)
            {
                if(g_i2cDataBase[i].isInited == true)
                {
                    I2C_BackupRegs(i);
                }
            }
            break;
        default:
            break;
    }

}

/**
  \fn        static void I2C_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, I2C hareware is repowered, we restore user's configurations
             by aidding of the stored registers.

  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void I2C_ExitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < I2C_INSTANCE_NUM; i++)
            {
                if(g_i2cDataBase[i].isInited == true)
                {
                    GPR_clockEnable(g_i2cClocks[2*i]);
                    GPR_clockEnable(g_i2cClocks[2*i+1]);

                    I2C_RestoreRegs(i);

                }
            }
            break;

        default:
            break;
    }

}

#define  LOCK_SLEEP(instance)       do                                                                  \
                                    {                                                                   \
                                        g_i2cWorkingStatus |= (1U << instance);                         \
                                        slpManDrvVoteSleep(SLP_VOTE_I2C, SLP_ACTIVE_STATE);             \
                                    }                                                                   \
                                    while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)    do                                                                  \
                                            {                                                                   \
                                                g_i2cWorkingStatus &= ~(1U << instance);                        \
                                                if(g_i2cWorkingStatus == 0)                                     \
                                                    slpManDrvVoteSleep(SLP_VOTE_I2C, SLP_SLP1_STATE);           \
                                            }                                                                   \
                                            while(0)
#endif


#define I2C_FIFO_DEPTH               (16)

/* Driver Version */
static const ARM_DRIVER_VERSION DriverVersion =
{
    ARM_I2C_API_VERSION,
    ARM_I2C_DRV_VERSION
};

/* Driver Capabilities */
static const ARM_I2C_CAPABILITIES DriverCapabilities =
{
    1 /* supports 10-bit addressing */
};


#if (RTE_I2C0)

AP_PLAT_COMMON_BSS static I2C_CTRL I2C0_Ctrl = { 0 };
AP_PLAT_COMMON_DATA static PIN I2C0_pin_scl  = {RTE_I2C0_SCL_BIT,   RTE_I2C0_SCL_FUNC};
AP_PLAT_COMMON_DATA static PIN I2C0_pin_sda  = {RTE_I2C0_SDA_BIT,   RTE_I2C0_SDA_FUNC};

#if (RTE_I2C0_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif

#if (RTE_I2C0_IO_MODE == IRQ_MODE)
void I2C0_IRQHandler(void);

AP_PLAT_COMMON_DATA static I2C_IRQ I2C0_IRQ = {
                            PXIC0_I2C0_IRQn,
                            I2C0_IRQHandler
                          };

#endif

AP_PLAT_COMMON_DATA static I2C_RESOURCES I2C0_Resources =
{
    I2C0,
    {
      &I2C0_pin_scl,
      &I2C0_pin_sda,
    },

#if (RTE_I2C0_IO_MODE == IRQ_MODE)
    &I2C0_IRQ,
#else
    NULL,
#endif

    &I2C0_Ctrl
};
#endif

#if (RTE_I2C1)

AP_PLAT_COMMON_BSS static I2C_CTRL I2C1_Ctrl = { 0 };
AP_PLAT_COMMON_DATA static PIN I2C1_pin_scl  = {RTE_I2C1_SCL_BIT,   RTE_I2C1_SCL_FUNC};
AP_PLAT_COMMON_DATA static PIN I2C1_pin_sda  = {RTE_I2C1_SDA_BIT,   RTE_I2C1_SDA_FUNC};

#if (RTE_I2C1_IO_MODE == DMA_MODE)
#error "DMA mode is not supported"
#endif

#if (RTE_I2C1_IO_MODE == IRQ_MODE)

void I2C1_IRQHandler(void);

AP_PLAT_COMMON_DATA static I2C_IRQ I2C1_IRQ = {
                            PXIC0_I2C1_IRQn,
                            I2C1_IRQHandler
                          };

#endif

AP_PLAT_COMMON_DATA static I2C_RESOURCES I2C1_Resources =
{
    I2C1,
    {
      &I2C1_pin_scl,
      &I2C1_pin_sda,
    },

#if (RTE_I2C1_IO_MODE == IRQ_MODE)
    &I2C1_IRQ,
#else
    NULL,
#endif
    &I2C1_Ctrl
};
#endif

// Local Function
/**
  \fn          ARM_DRIVER_VERSION I2C_GetVersion(void)
  \brief       Get driver version.
  \return      \ref ARM_DRIVER_VERSION
*/
ARM_DRIVER_VERSION ARM_I2C_GetVersion(void)
{
    return DriverVersion;
}


/**
  \fn          ARM_I2C_CAPABILITIES I2C_GetCapabilities(void)
  \brief       Get driver capabilities.
  \return      \ref ARM_I2C_CAPABILITIES
*/
ARM_I2C_CAPABILITIES I2C_GetCapabilities()
{
    return DriverCapabilities;
}

/**
  \fn          static uint32_t I2C_GetInstanceNumber(I2C_RESOURCES *i2c)
  \brief       Get instance number
  \param[in]   i2c       Pointer to I2C resources
  \returns     instance number
*/
static uint32_t I2C_GetInstanceNumber(I2C_RESOURCES *i2c)
{
    return ((uint32_t)i2c->reg - (uint32_t)I2C0) >> 12;
}

/**
  \fn          int32_t I2C_Initialize(ARM_I2C_SignalEvent_t cb_event,
                                      I2C_RESOURCES         *i2c)
  \brief       Initialize I2C Interface.
  \param[in]   cb_event  Pointer to \ref ARM_I2C_SignalEvent
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_Initialize(ARM_I2C_SignalEvent_t cb_event, I2C_RESOURCES *i2c)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif

    PadConfig_t padConfig;

    if(i2c->ctrl->flags & I2C_FLAG_INIT)
    {
        return ARM_DRIVER_OK;
    }

    // Configure I2C Pins

    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = i2c->pins.pin_scl->funcNum;
    PAD_setPinConfig(i2c->pins.pin_scl->pinNum, &padConfig);
    padConfig.mux = i2c->pins.pin_sda->funcNum;
    PAD_setPinConfig(i2c->pins.pin_sda->pinNum, &padConfig);

    // Reset Run-Time information structure
    memset(i2c->ctrl, 0, sizeof(I2C_CTRL));

    i2c->ctrl->cb_event = cb_event;

    i2c->ctrl->flags |= I2C_FLAG_INIT;

#ifdef PM_FEATURE_ENABLE

    instance = I2C_GetInstanceNumber(i2c);

    g_i2cDataBase[instance].isInited = true;

    g_i2cInitCounter++;

    if(g_i2cInitCounter == 1)
    {
        g_i2cWorkingStatus = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2C_MODULE, I2C_EnterLowPowerStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2C_MODULE, I2C_ExitLowPowerStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2C_Uninitialize(I2C_RESOURCES *i2c)
  \brief       De-initialize I2C Interface.
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_Uninitialize(I2C_RESOURCES *i2c)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;
#endif

    PadConfig_t padConfig;
    i2c->ctrl->flags = 0;
    i2c->ctrl->cb_event = NULL;

    // Unconfigure SCL and SDA pins
    PAD_getDefaultConfig(&padConfig);
    padConfig.mux = PAD_MUX_ALT0;

    PAD_setPinConfig(i2c->pins.pin_scl->pinNum, &padConfig);
    PAD_setPinConfig(i2c->pins.pin_sda->pinNum, &padConfig);
    // Input+pullup
    PAD_setPinPullConfig(i2c->pins.pin_scl->pinNum, PAD_INTERNAL_PULL_UP);
    PAD_setPinPullConfig(i2c->pins.pin_sda->pinNum, PAD_INTERNAL_PULL_UP);

#ifdef PM_FEATURE_ENABLE
    instance = I2C_GetInstanceNumber(i2c);
    g_i2cDataBase[instance].isInited = false;

    g_i2cInitCounter--;
    if(g_i2cInitCounter == 0)
    {
        g_i2cWorkingStatus = 0;
        slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_I2C_MODULE);
        slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_I2C_MODULE);

    }
#endif

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t I2C_PowerControl(ARM_POWER_STATE state,
                                        I2C_RESOURCES   *i2c)
  \brief       Control I2C Interface Power.
  \param[in]   state  Power state
  \param[in]   i2c    Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_PowerControl(ARM_POWER_STATE state, I2C_RESOURCES *i2c)
{
    uint32_t instance = I2C_GetInstanceNumber(i2c);

    switch (state)
    {
        case ARM_POWER_OFF:
            // I2C reset controller, including disable all I2C interrupts & clear fifo
            if(i2c->irq)
                XIC_DisableIRQ(i2c->irq->irq_num);

            // Disable I2C and other control bits
            i2c->reg->CR1 = 0;

            // Disable I2C power
            CLOCK_clockDisable(g_i2cClocks[instance*2]);
            CLOCK_clockDisable(g_i2cClocks[instance*2+1]);

            memset((void*)&i2c->ctrl->status, 0, sizeof(ARM_I2C_STATUS));

            i2c->ctrl->stalled = 0;
            i2c->ctrl->num    = 0;
            i2c->ctrl->flags  &= ~I2C_FLAG_POWER;
            break;

        case ARM_POWER_LOW:
            return ARM_DRIVER_ERROR_UNSUPPORTED;

        case ARM_POWER_FULL:
            if((i2c->ctrl->flags & I2C_FLAG_INIT) == 0U)
            {
                return ARM_DRIVER_ERROR;
            }
            if(i2c->ctrl->flags & I2C_FLAG_POWER)
            {
                return ARM_DRIVER_OK;
            }

            // Enable power to i2c clock
            CLOCK_clockEnable(g_i2cClocks[instance*2]);
            CLOCK_clockEnable(g_i2cClocks[instance*2+1]);

            // Enable I2C irq
            if(i2c->irq)
            {
                XIC_SetVector(i2c->irq->irq_num,i2c->irq->cb_irq);
                XIC_EnableIRQ(i2c->irq->irq_num);
                XIC_SuppressOvfIRQ(i2c->irq->irq_num);
            }

            i2c->ctrl->flags |= I2C_FLAG_POWER;
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

static void I2C_MasterReset(I2C_RESOURCES *i2c)
{
    uint32_t instance = I2C_GetInstanceNumber(i2c);

    // Disable irq
    i2c->reg->IMR = 0;

    I2C_BackupRegs(instance);

    GPR_swResetModule(&g_i2cResetVectors[instance]);

    I2C_RestoreRegs(instance);
}

/**
  \fn     static int32_t I2C_MasterCheckStatus(I2C_RESOURCES *i2c)
  \brief  Check status and clear any error if present
  \param  i2c         Pointer to I2C resources
  \return ARM_DRIVER_ERROR if any exception occurs, otherwise ARM_DRIVER_OK
  */
static int32_t I2C_MasterCheckStatus(I2C_RESOURCES *i2c, uint32_t *event)
{
    volatile uint32_t risr_reg = i2c->reg->RISR;

    if(risr_reg & I2C_RISR_SCL_STUCK_Msk)
    {
        i2c->ctrl->status.bus_error = 1;
        *event |= ARM_I2C_EVENT_BUS_ERROR;
        I2C_MasterReset(i2c);
        return ARM_DRIVER_ERROR;
    }

    // Have no hurt to clear isr for polling mode

    if(risr_reg & I2C_RISR_TX_ABRT_Msk)
    {
        volatile uint32_t str1_reg = i2c->reg->STR1;

        i2c->reg->ICR[5] |= 0x1;

        if(str1_reg & I2C_STR1_SDA_STUCK_Msk)
        {
            i2c->reg->CR1 |= I2C_CR1_SDA_RECOV_Msk;

            while(i2c->reg->CR1 & I2C_CR1_SDA_RECOV_Msk);

            // Can't recover
            if(i2c->reg->STR0 & I2C_STR0_SDA_RECOV_STATUS_Msk)
            {
                i2c->ctrl->status.bus_error = 1;
                I2C_MasterReset(i2c);
                *event |= ARM_I2C_EVENT_BUS_ERROR;
                return ARM_DRIVER_ERROR;
            }
            else
            {
                // SDK line shall be recovered now
                *event = ARM_I2C_EVENT_BUS_CLEAR;
                // Restart transaction
                i2c->ctrl->state = I2C_STATE_IDLE;
                return ARM_DRIVER_ERROR;
            }

        }

        if(str1_reg & I2C_STR1_ARBITRATATION_LOST_Msk)
        {
            i2c->ctrl->status.arbitration_lost = 1;
            *event |= ARM_I2C_EVENT_ARBITRATION_LOST;
        }

        if(str1_reg & (I2C_STR1_ADDR_NACK_Msk | I2C_STR1_GCALL_NACK_Msk))
        {
            i2c->ctrl->status.rx_nack = 1;
            *event |= ARM_I2C_EVENT_ADDRESS_NACK;
        }

        if(str1_reg & I2C_STR1_TXDATA_NACK_Msk)
        {
            i2c->ctrl->status.rx_nack = 1;
            *event |= ARM_I2C_EVENT_TRANSFER_INCOMPLETE;
        }


        i2c->reg->CR1 = 0;

        return ARM_DRIVER_ERROR;

    }

    return ARM_DRIVER_OK;
}

void I2C_MasterStateMachine(I2C_RESOURCES *i2c, uint32_t* event)
{

#ifdef PM_FEATURE_ENABLE
    uint32_t  instance = I2C_GetInstanceNumber(i2c);
#endif

    int32_t ret;

    uint32_t i, txfifo_free, left;

    ret = I2C_MasterCheckStatus(i2c, event);

    // Error happens, terminate transfer
    if(ret != ARM_DRIVER_OK)
    {
        *event |= ARM_I2C_EVENT_TRANSFER_DONE;

        i2c->ctrl->status.busy = 0;

#ifdef PM_FEATURE_ENABLE
        CHECK_TO_UNLOCK_SLEEP(instance);
#endif
        return;
    }

    switch(i2c->ctrl->state)
    {
        case I2C_STATE_IDLE:

#ifdef PM_FEATURE_ENABLE
            LOCK_SLEEP(instance);
#endif
            // set slave(target) address
            if(i2c->ctrl->address & ARM_I2C_ADDRESS_10BIT)
            {
                i2c->reg->TAR = (i2c->ctrl->address & I2C_TAR_ADDRESS_Msk) | I2C_TAR_MASTER_10BIT_Msk;
            }
            else
            {
                i2c->reg->TAR = i2c->ctrl->address & I2C_TAR_ADDRESS_Msk;
            }

            if(i2c->ctrl->status.direction == 0)
            {
                i2c->ctrl->state = I2C_STATE_TXDATA;
            }
            else
            {
                i2c->ctrl->state = I2C_STATE_RXDATA;
                i2c->ctrl->cmd_cnt = 0;
            }

            i2c->reg->CR0 |= I2C_CR0_MASTER_CTRL_Msk | I2C_CR0_BUS_CLEAR_Msk;

            i2c->reg->CR1 = I2C_CR1_ENABLE_Msk;

            break;

        case I2C_STATE_TXDATA:

            if(i2c->ctrl->cnt < i2c->ctrl->num)
            {
                txfifo_free = I2C_FIFO_DEPTH - i2c->reg->TXFSR;
                left = i2c->ctrl->num - i2c->ctrl->cnt;

                if(left <= txfifo_free)
                {
                    i = left;

                    // disable water level interrupt now to get rid of XIC irq overflow
                    i2c->reg->IMR &= ~I2C_IMR_TX_WL_Msk;

                    // All data will be put into txfifo, move to END state to wait for 'the true ending'
                    i2c->ctrl->state = I2C_STATE_END;
                }
                else
                {
                    i = txfifo_free;
                }

                while(i--)
                {
                    if((i2c->ctrl->pending == false) && ((i2c->ctrl->cnt + 1) == i2c->ctrl->num))
                    {
                        i2c->reg->COMMAND = i2c->ctrl->data[i2c->ctrl->cnt++] | I2C_COMMAND_STOP_Msk;
                    }
                    else
                    {
                        i2c->reg->COMMAND = i2c->ctrl->data[i2c->ctrl->cnt++];
                    }
                }
            }

            break;

        case I2C_STATE_RXDATA:

            // Send cmd as more as possible and the differ between tx cnt and rx cnt shall be less than I2C_FIFO_DEPTH to get rid of rxfifo overflow
            while((i2c->reg->STR0 & I2C_STR0_TFNF_Msk) &&
                  (i2c->ctrl->num > i2c->ctrl->cmd_cnt) &&
                  ((i2c->ctrl->cmd_cnt - i2c->ctrl->cnt) < I2C_FIFO_DEPTH))
            {
                if((i2c->ctrl->pending == false) && ((i2c->ctrl->cmd_cnt + 1) == i2c->ctrl->num))
                {
                    i2c->reg->COMMAND = I2C_COMMAND_RWN_Msk | I2C_COMMAND_STOP_Msk;
                }
                else
                {
                    i2c->reg->COMMAND = I2C_COMMAND_RWN_Msk;
                }

                i2c->ctrl->cmd_cnt++;

            }

            while(i2c->reg->STR0 & I2C_STR0_RFNE_Msk)
            {
                i2c->ctrl->data[i2c->ctrl->cnt++] = i2c->reg->COMMAND;
            }

            if(i2c->ctrl->cmd_cnt == i2c->ctrl->num)
            {
                // disable tx water level interrupt now to get rid of interrupt storm
                // And enable rx water level interrupt to receive data for IRQ mode
                i2c->reg->IMR = (i2c->reg->IMR &~I2C_IMR_TX_WL_Msk) | I2C_IMR_RX_WL_Msk;

            }

            if(i2c->ctrl->cnt == i2c->ctrl->num)
            {
                i2c->ctrl->state = I2C_STATE_END;

                i2c->reg->IMR = (i2c->reg->IMR &~ I2C_IMR_RX_WL_Msk);
            }
            // Terminate rx loop in case of txfifo is empty and all data has been retrived from rxfifo and master is idle even the cnt is less than the required rx num
            else if((i2c->reg->STR0 & (I2C_STR0_TFE_Msk | I2C_STR0_MASTER_BUSY_Msk | I2C_STR0_RFNE_Msk)) == I2C_STR0_TFE_Msk)
            {
                i2c->ctrl->state = I2C_STATE_END;

                i2c->reg->IMR = (i2c->reg->IMR &~ I2C_IMR_RX_WL_Msk);
            }

            break;

        case I2C_STATE_END:

            if(i2c->reg->STR0 & I2C_STR0_TFE_Msk)
            {
            }
            else
            {
                break;
            }

            i2c->ctrl->state = I2C_STATE_IDLE;
            i2c->ctrl->status.busy = 0;

            // clear all interrupts first and disable i2c
            i2c->reg->IMR = 0;
            i2c->reg->ICR[5] |= 1;
            i2c->reg->ICR[0] |= 1;
            i2c->reg->CR1 = 0;

            *event = ARM_I2C_EVENT_TRANSFER_DONE;

#ifdef PM_FEATURE_ENABLE
            CHECK_TO_UNLOCK_SLEEP(instance);
#endif

            break;

        default:
            break;
    }

}


/**
  \fn          int32_t I2Cx_MasterTransmit(uint32_t       addr,
                                           const uint8_t *data,
                                           uint32_t       num,
                                           bool           xfer_pending,
                                           I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[in]   data          Pointer to buffer with data to transmit to I2C Slave
  \param[in]   num           Number of data bytes to transmit
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_MasterTransmit(uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c)
{
    uint32_t event = 0;

    if(!data || !num )
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(!(i2c->ctrl->flags & I2C_FLAG_SETUP))
    {
        return ARM_DRIVER_ERROR;
    }

    if(i2c->ctrl->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    // Set control variables
    i2c->ctrl->address = addr;
    i2c->ctrl->pending = xfer_pending;
    i2c->ctrl->data = (uint8_t *)data;
    i2c->ctrl->num  = num;
    i2c->ctrl->cnt  = 0;

    // Update driver status
    i2c->ctrl->status.busy = 1;
    // 0 :slave / 1 :master
    i2c->ctrl->status.mode = 1;
    // 0 :tx  / 1 :rx
    i2c->ctrl->status.direction = 0;
    i2c->ctrl->status.arbitration_lost = 0;
    i2c->ctrl->status.bus_error = 0;
    i2c->ctrl->status.rx_nack = 0;
    i2c->ctrl->state = I2C_STATE_IDLE;

    I2CDEBUG("transmit, num-%d, misr-%0x%x\r\n", num, i2c->reg->MISR);

    if(i2c->irq)
    {
        i2c->reg->TXFCR = 8;

        // clear all interrupts first
        i2c->reg->IMR = 0;

        i2c->reg->ICR[5] |= 1;
        i2c->reg->ICR[0] |= 1;

        uint32_t mask = SaveAndSetIRQMask();

        // stop_det indicates the end of tx if pending is false and master_on_hold is the case for true condition
        i2c->reg->IMR = I2C_IMR_TX_WL_Msk | \
                        I2C_IMR_TX_ABRT_Msk | \
                        I2C_IMR_STOP_DET_Msk | \
                        I2C_IMR_MASTER_ON_HOLD_Msk | \
                        I2C_IMR_SCL_STUCK_Msk;

        // Trigger state machine to run
        I2C_MasterStateMachine(i2c, &event);

        RestoreIRQMask(mask);

        return ARM_DRIVER_OK;
    }
    else
    {
        i2c->reg->TXFCR = 0;
        i2c->reg->IMR = 0;

        do
        {
            I2C_MasterStateMachine(i2c, &event);
        }
        while((event == 0) || ((event & ARM_I2C_EVENT_BUS_CLEAR) == ARM_I2C_EVENT_BUS_CLEAR));

        i2c->ctrl->status.busy = 0;

        if( (i2c->ctrl->status.arbitration_lost == 1) || \
            (i2c->ctrl->status.bus_error == 1) || \
            (i2c->ctrl->status.rx_nack == 1))
        {
            return ARM_DRIVER_ERROR;
        }
        else
        {
            return ARM_DRIVER_OK;
        }

    }

}

/**
  \fn          int32_t I2Cx_MasterReceive(uint32_t       addr,
                                          uint8_t       *data,
                                          uint32_t       num,
                                          bool           xfer_pending,
                                          I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[out]  data          Pointer to buffer for data to receive from I2C Slave
  \param[in]   num           Number of data bytes to receive
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_MasterReceive(uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending, I2C_RESOURCES *i2c)
{
    uint32_t event = 0;

    if(!data || !num)
    {
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(!(i2c->ctrl->flags & I2C_FLAG_SETUP))
    {
        return ARM_DRIVER_ERROR;
    }

    if(i2c->ctrl->status.busy)
    {
        return ARM_DRIVER_ERROR_BUSY;
    }

    // Set control variables
    i2c->ctrl->address = addr;
    i2c->ctrl->pending = xfer_pending;
    i2c->ctrl->data = (uint8_t *)data;
    i2c->ctrl->num  = num;
    i2c->ctrl->cnt  = 0;

    // Update driver status
    i2c->ctrl->status.busy = 1;
    // 0 :slave / 1 :master
    i2c->ctrl->status.mode = 1;
    // 0 :tx  / 1 :rx
    i2c->ctrl->status.direction = 1;
    i2c->ctrl->status.arbitration_lost = 0;
    i2c->ctrl->status.bus_error = 0;
    i2c->ctrl->status.rx_nack = 0;
    i2c->ctrl->state = I2C_STATE_IDLE;

    I2CDEBUG("receive, num-%d, misr-%0x%x\r\n", num, i2c->reg->MISR);

    if(i2c->irq)
    {
        i2c->reg->TXFCR = 8;
        i2c->reg->RXFCR = 1;

        // clear all interrupts first
        i2c->reg->IMR = 0;

        i2c->reg->ICR[5] |= 1;
        i2c->reg->ICR[0] |= 1;

        uint32_t mask = SaveAndSetIRQMask();

        // stop_det indicates the end of rx if pending is false and master_on_hold is the case for true condition
        i2c->reg->IMR = I2C_IMR_TX_WL_Msk | \
                        I2C_IMR_TX_ABRT_Msk | \
                        I2C_IMR_STOP_DET_Msk | \
                        I2C_IMR_MASTER_ON_HOLD_Msk | \
                        I2C_IMR_SCL_STUCK_Msk;

        // Trigger state machine to run
        I2C_MasterStateMachine(i2c, &event);

        RestoreIRQMask(mask);

        return ARM_DRIVER_OK;
    }
    else
    {
        i2c->reg->TXFCR = 0;

        do
        {
            I2C_MasterStateMachine(i2c, &event);
        }
        while((event == 0) || ((event & ARM_I2C_EVENT_BUS_CLEAR) == ARM_I2C_EVENT_BUS_CLEAR));

        i2c->ctrl->status.busy = 0;

        if( (i2c->ctrl->status.arbitration_lost == 1) || \
            (i2c->ctrl->status.bus_error == 1) || \
            (i2c->ctrl->status.rx_nack == 1))
        {
            return ARM_DRIVER_ERROR;
        }
        else
        {
            return ARM_DRIVER_OK;
        }

    }

}

/**
  \fn          int32_t I2Cx_SlaveTransmit(const uint8_t *data,
                                          uint32_t       num,
                                          I2C_RESOURCES *i2c)
  \brief       Start transmitting data as I2C Slave.
  \param[in]   data  Pointer to buffer with data to transmit to I2C Master
  \param[in]   num   Number of data bytes to transmit
  \param[in]   i2c   Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_SlaveTransmit(const uint8_t *data, uint32_t num, I2C_RESOURCES *i2c)
{
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_MasterReceive(uint32_t       addr,
                                          uint8_t       *data,
                                          uint32_t       num,
                                          bool           xfer_pending,
                                          I2C_RESOURCES *i2c)
  \brief       Start receiving data as I2C Master.
  \param[in]   addr          Slave address (7-bit or 10-bit)
  \param[out]  data          Pointer to buffer for data to receive from I2C Slave
  \param[in]   num           Number of data bytes to receive
  \param[in]   xfer_pending  Transfer operation is pending - Stop condition will not be generated
  \param[in]   i2c           Pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_SlaveReceive(uint8_t *data, uint32_t num, I2C_RESOURCES *i2c)
{
    if(!data || !num)
    {
        /* Invalid parameters */
        return ARM_DRIVER_ERROR_PARAMETER;
    }

    if(i2c->ctrl->status.busy)
    {
        /* Transfer operation in progress, Master stalled or Slave transmit stalled */
        return ARM_DRIVER_ERROR_BUSY;
    }

    /* Set control variables */
    i2c->ctrl->data  = data;
    i2c->ctrl->num   = num;
    i2c->ctrl->cnt    = 0;

    /* Update driver status */
    i2c->ctrl->status.general_call = 0;
    i2c->ctrl->status.bus_error    = 0;

    // Update driver status
    i2c->ctrl->status.busy = 1;
    // 0 :slave / 1 :master
    i2c->ctrl->status.mode = 0;
    // 0 :tx  / 1 :rx
    i2c->ctrl->status.direction = 1;

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t I2Cx_GetDataCount(I2C_RESOURCES *i2c)
  \brief       Get transferred data count.
  \return      number of data bytes transferred; -1 when Slave is not addressed by Master
*/
int32_t I2C_GetDataCount(I2C_RESOURCES *i2c)
{
    return (i2c->ctrl->cnt);
}

/**
  \fn          int32_t I2C_GetClockFreq(I2C_RESOURCES *i2c)
  \brief       Get i2c clock.
  \return      value of i2c clock
*/
int32_t I2C_GetClockFreq(I2C_RESOURCES *i2c)
{
    uint32_t instance = I2C_GetInstanceNumber(i2c);

    return GPR_getClockFreq(g_i2cClocks[instance*2 + 1]);
}

/**
  \fn          int32_t I2Cx_Control(uint32_t       control,
                                    uint32_t       arg,
                                    I2C_RESOURCES *i2c)
  \brief       Control I2C Interface.
  \param[in]   control  operation
  \param[in]   arg      argument of operation (optional)
  \param[in]   i2c      pointer to I2C resources
  \return      \ref execution_status
*/
int32_t I2C_Control(uint32_t control, uint32_t arg, I2C_RESOURCES *i2c)
{
    uint32_t clk;

    uint32_t hcnt = 0, lcnt = 0;

    if(!(i2c->ctrl->flags & I2C_FLAG_POWER))
    {
        return ARM_DRIVER_ERROR;
    }
    switch(control)
    {
        case ARM_I2C_OWN_ADDRESS:
            // General call enable
#if 0
            if(arg & ARM_I2C_ADDRESS_GC)
            {
                i2c->reg->SAR |= I2C_SAR_GENERAL_CALL_EN_Msk;
            }
            else
            {
                i2c->reg->SAR &= (~I2C_SAR_GENERAL_CALL_EN_Msk);
            }
            // Slave address mode 0: 7-bit mode 1: 10-bit mode
            if(arg & ARM_I2C_ADDRESS_10BIT)
            {
                i2c->reg->SAR |= I2C_SAR_SLAVE_ADDR_MODE_Msk;
                val = arg & 0x3FF;
            }
            else
            {
                i2c->reg->SAR &= (~I2C_SAR_SLAVE_ADDR_MODE_Msk);
                val = arg & 0x7F;
            }
            // Slave address
            i2c->reg->SAR |= (val << 1);
            // Enable slave address
            i2c->reg->SAR |= I2C_SAR_SLAVE_ADDR_EN_Msk;
#endif
            break;

        case ARM_I2C_BUS_SPEED:
            clk = I2C_GetClockFreq(i2c);

            // tHIGH = (HCNT + SPKSR) / Fclk
            // tLOW  = (LCNT + 1) / Fclk - tf

            switch(arg)
            {
                case ARM_I2C_BUS_SPEED_STANDARD:

                    // spec: tLOW(min) = 4.7us, tHIGH(min) = 4us, tf = 0.3us, tr = 1us
                    // 100kHz
                    // setting: tLOW = 5us, tHIGH = 4.5us, spike len = 4 clk

                    i2c->reg->SPKSR = 4;

                    hcnt = (4 * clk + clk * 5 / 10) / 1000000 - i2c->reg->SPKSR;
                    lcnt = (5 * clk + clk * 3 / 10) / 1000000 - 1;

                    i2c->reg->SCLCNT[0] = hcnt;
                    i2c->reg->SCLCNT[1] = lcnt;

                    i2c->reg->CR0 = ((i2c->reg->CR0 &~ I2C_CR0_SS_Msk) | EIGEN_VAL2FLD(I2C_CR0_SS, 1));

                    break;
                case ARM_I2C_BUS_SPEED_FAST:

                    // spec: tLOW(min) = 1.3us, tHIGH(min) = 0.6us, tf = 0.3us, tr = 0.3us
                    // 400kHz
                    // setting: tLOW = 1.3us, tHIGH(min) = 1us, spike len = 4 clk

                    i2c->reg->SPKSR = 4;

                    hcnt = clk / 1000000 - i2c->reg->SPKSR;
                    lcnt = (clk + clk * 6 / 10) / 1000000 - 1;

                    i2c->reg->SCLCNT[2] = hcnt;
                    i2c->reg->SCLCNT[3] = lcnt;

                    i2c->reg->CR0 = ((i2c->reg->CR0 &~ I2C_CR0_SS_Msk) | EIGEN_VAL2FLD(I2C_CR0_SS, 2));

                    break;
                case ARM_I2C_BUS_SPEED_FAST_PLUS:

                    // spec: tLOW(min) = 0.5us, tHIGH(min) = 0.26us, tf = 0.12us, tr = 0.12us
                    // 1MHz
                    // setting: tLOW = 0.5us, tHIGH = 0.3us, spike len = 1 clk

                    i2c->reg->SPKSR = 1;

                    hcnt = (clk * 3 / 10) / 1000000 - i2c->reg->SPKSR;
                    lcnt = (clk * 62 / 100) / 1000000 - 1;

                    i2c->reg->SCLCNT[2] = hcnt;
                    i2c->reg->SCLCNT[3] = lcnt;

                    i2c->reg->CR0 = ((i2c->reg->CR0 &~ I2C_CR0_SS_Msk) | EIGEN_VAL2FLD(I2C_CR0_SS, 2));


                    break;
                default:
                    return ARM_DRIVER_ERROR_UNSUPPORTED;
            }

            // Setup SCL/SDA line(0->SCL,1->SDA) stuck(hold low) timeout cycles
            #if 1
            i2c->reg->STUCKTIMEOUT[0] = clk * 2; // 2s
            i2c->reg->STUCKTIMEOUT[1] = clk * 2;
            #endif
            // Speed configured
            i2c->ctrl->flags |= I2C_FLAG_SETUP;
            break;

        case ARM_I2C_BUS_CLEAR:

            break;

        case ARM_I2C_ABORT_TRANSFER:
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn          ARM_I2C_STATUS I2Cx_GetStatus(I2C_RESOURCES *i2c)
  \brief       Get I2C status.
  \param[in]   i2c      pointer to I2C resources
  \return      I2C status \ref ARM_I2C_STATUS
*/
ARM_I2C_STATUS I2C_GetStatus(I2C_RESOURCES *i2c)
{
    return (i2c->ctrl->status);
}


/**
  \fn          void I2Cx_IRQHandler(I2C_RESOURCES *i2c)
  \brief       I2C Event Interrupt handler.
  \param[in]   i2c  Pointer to I2C resources
*/
void I2C_IRQHandler(I2C_RESOURCES *i2c)
{
    uint32_t event = 0, imr;

    I2CDEBUG("risr-0x%x, misr-0x%x, imr-0x%x\r\n", i2c->reg->RISR, i2c->reg->MISR, i2c->reg->IMR);
    I2CDEBUG("cmd_cnt-%d, cnt-%d, tx_lvl-%d, rx_lvl-%d\n", i2c->ctrl->cmd_cnt, i2c->ctrl->cnt, i2c->reg->TXFSR, i2c->reg->RXFSR);

    I2C_MasterStateMachine(i2c, &event);

    if(i2c->ctrl->cb_event && event)
    {
        i2c->ctrl->cb_event(event);
    }

    imr = i2c->reg->IMR;
    i2c->reg->IMR = 0;
    i2c->reg->IMR = imr;

}

#if (RTE_I2C0)
static int32_t I2C0_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return I2C_Initialize(cb_event, &I2C0_Resources);
}
static int32_t I2C0_Uninitialize(void)
{
    return I2C_Uninitialize(&I2C0_Resources);
}
static int32_t I2C0_PowerControl(ARM_POWER_STATE state)
{
    return I2C_PowerControl(state, &I2C0_Resources);
}
static int32_t I2C0_MasterTransmit(uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending)
{
    return I2C_MasterTransmit(addr, data, num, xfer_pending, &I2C0_Resources);
}
static int32_t I2C0_MasterReceive(uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending)
{
    return I2C_MasterReceive(addr, data, num, xfer_pending, &I2C0_Resources);
}
static int32_t I2C0_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return I2C_SlaveTransmit(data, num, &I2C0_Resources);
}
static int32_t I2C0_SlaveReceive(uint8_t *data, uint32_t num)
{
    return I2C_SlaveReceive(data, num, &I2C0_Resources);
}
static int32_t I2C0_GetDataCount(void)
{
    return I2C_GetDataCount(&I2C0_Resources);
}
static int32_t I2C0_Control(uint32_t control, uint32_t arg)
{
    return I2C_Control(control, arg, &I2C0_Resources);
}
static ARM_I2C_STATUS I2C0_GetStatus(void)
{
    return I2C_GetStatus(&I2C0_Resources);
}
void I2C0_IRQHandler(void)
{
    I2C_IRQHandler(&I2C0_Resources);
}

AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C0 =
{
    ARM_I2C_GetVersion,
    I2C_GetCapabilities,
    I2C0_Initialize,
    I2C0_Uninitialize,
    I2C0_PowerControl,
    I2C0_MasterTransmit,
    I2C0_MasterReceive,
    I2C0_SlaveTransmit,
    I2C0_SlaveReceive,
    I2C0_GetDataCount,
    I2C0_Control,
    I2C0_GetStatus
};

#endif

#if (RTE_I2C1)

static int32_t I2C1_Initialize(ARM_I2C_SignalEvent_t cb_event)
{
    return I2C_Initialize(cb_event, &I2C1_Resources);
}
static int32_t I2C1_Uninitialize(void)
{
    return I2C_Uninitialize(&I2C1_Resources);
}
static int32_t I2C1_PowerControl(ARM_POWER_STATE state)
{
    return I2C_PowerControl(state, &I2C1_Resources);
}
static int32_t I2C1_MasterTransmit(uint32_t addr, const uint8_t *data, uint32_t num, bool xfer_pending)
{
    return I2C_MasterTransmit(addr, data, num, xfer_pending, &I2C1_Resources);
}
static int32_t I2C1_MasterReceive(uint32_t addr, uint8_t *data, uint32_t num, bool xfer_pending)
{
    return I2C_MasterReceive(addr, data, num, xfer_pending, &I2C1_Resources);
}
static int32_t I2C1_SlaveTransmit(const uint8_t *data, uint32_t num)
{
    return I2C_SlaveTransmit(data, num, &I2C1_Resources);
}
static int32_t I2C1_SlaveReceive(uint8_t *data, uint32_t num)
{
    return I2C_SlaveReceive(data, num, &I2C1_Resources);
}
static int32_t I2C1_GetDataCount (void)
{
    return I2C_GetDataCount(&I2C1_Resources);
}
static int32_t I2C1_Control(uint32_t control, uint32_t arg)
{
    return I2C_Control(control, arg, &I2C1_Resources);
}
static ARM_I2C_STATUS I2C1_GetStatus(void)
{
    return I2C_GetStatus(&I2C1_Resources);
}
void I2C1_IRQHandler(void)
{
    I2C_IRQHandler(&I2C1_Resources);
}

// End I2C Interface

AP_PLAT_COMMON_DATA ARM_DRIVER_I2C Driver_I2C1 =
{
    ARM_I2C_GetVersion,
    I2C_GetCapabilities,
    I2C1_Initialize,
    I2C1_Uninitialize,
    I2C1_PowerControl,
    I2C1_MasterTransmit,
    I2C1_MasterReceive,
    I2C1_SlaveTransmit,
    I2C1_SlaveReceive,
    I2C1_GetDataCount,
    I2C1_Control,
    I2C1_GetStatus
};
#endif

#if 0
#pragma GCC pop_options
#endif

