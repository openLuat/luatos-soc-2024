 /****************************************************************************
  *
  * Copy right:   2017-, Copyrigths of AirM2M Ltd.
  * File name:    pad.c
  * Description:  EC718 pad driver source file
  * History:      Rev1.0   2018-11-14
  *
  ****************************************************************************/

#include "pad.h"
#include "clock.h"
#include "slpman.h"
#include "sctdef.h"

#define PAD_TEXT_SECTION     SECTION_DEF_IMPL(.sect_pad_text)
#define PAD_RODATA_SECTION   SECTION_DEF_IMPL(.sect_pad_rodata)
#define PAD_DATA_SECTION     SECTION_DEF_IMPL(.sect_pad_data)
#define PAD_BSS_SECTION      SECTION_DEF_IMPL(.sect_pad_bss)



#ifdef PM_FEATURE_ENABLE

PAD_BSS_SECTION static uint32_t gPadBackupRegs[PAD_ADDR_MAX_NUM] = {0};

/**
  \fn        void PAD_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Backup pad configurations before sleep.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
void PAD_enterLowPowerStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            GPR_clockEnable(PCLK_PAD);

            for(i = 0; i < PAD_ADDR_MAX_NUM; i++)
            {
                gPadBackupRegs[i] = PAD->PCR[i];
            }

            GPR_clockDisable(PCLK_PAD);

            break;
        default:
            break;
    }

}

/**
 \fn        void PAD_exitLowPowerStateRestore(void* pdata, slpManLpState state)
 \brief     Restore PAD configurations after exit from sleep.
 \param[in] pdata pointer to user data, not used now
 \param[in] state low power state
 */
void PAD_exitLowPowerStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;

    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            GPR_clockEnable(PCLK_PAD);

            for(i = 0; i < PAD_ADDR_MAX_NUM; i++)
            {
                PAD->PCR[i] = gPadBackupRegs[i];
            }

            GPR_clockDisable(PCLK_PAD);

            break;

        default:
            break;
    }

}
#endif

PLAT_BL_CIRAM_FLASH_TEXT void PAD_driverInit(void)
{
#ifdef PM_FEATURE_ENABLE
    slpManRegisterPredefinedBackupCb(SLP_CALLBACK_PAD_MODULE, PAD_enterLowPowerStatePrepare, NULL);
    slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_PAD_MODULE, PAD_exitLowPowerStateRestore, NULL);
#endif
}

void PAD_driverDeInit(void)
{
#ifdef PM_FEATURE_ENABLE
    slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_PAD_MODULE);
    slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_PAD_MODULE);
#endif
}

void PAD_getDefaultConfig(PadConfig_t *config)
{
    ASSERT(config);

    config->mux = PAD_MUX_ALT0;
    config->inputControl = PAD_INPUT_CONTROL_AUTO;
    config->inputForceDisable = 0;
    config->outputForceDisable = 0;
    config->outputControl = PAD_OUTPUT_CONTROL_AUTO;
    config->swOutputEnable = 0;
    config->swOutputValue = 0;
    config->pullSelect = PAD_PULL_AUTO;
    config->pullUpEnable = PAD_PULL_UP_DISABLE;
    config->pullDownEnable = PAD_PULL_DOWN_DISABLE;
#if defined CHIP_EC718
    config->driveStrength = PAD_DRIVE_STRENGTH_HIGH;
#elif defined CHIP_EC716
    config->slewRate = PAD_SLEW_RATE_HIGH;
    config->schmittTriggerEnable = 0;
    config->driveStrength = PAD_DRIVE_STRENGTH_7;
#else
	#error "unknow chip name"
#endif
}

void PAD_setInputOutputDisable(uint32_t paddr)
{
    uint32_t tmp;
    ASSERT(paddr < PAD_ADDR_MAX_NUM);

    CLOCK_clockEnable(PCLK_PAD);

    tmp = PAD->PCR[paddr];
    tmp = tmp & (~(PAD_PCR_PULL_UP_ENABLE_Msk | PAD_PCR_PULL_DOWN_ENABLE_Msk));         // disable pull up, disable pull down
    tmp = tmp | PAD_PCR_INPUT_FORCE_DISABLE_Msk | PAD_PCR_OUTPUT_FORCE_DISABLE_Msk;     // input disable and output disable
    tmp = tmp & ~PAD_PCR_DRIVE_STRENGTH_Msk;                                            // driver strength = 0
    PAD->PCR[paddr] = tmp;

    CLOCK_clockDisable(PCLK_PAD);
}


void PAD_setPinConfig(uint32_t paddr, const PadConfig_t *config)
{
    ASSERT(config);
    ASSERT(paddr < PAD_ADDR_MAX_NUM);

    CLOCK_clockEnable(PCLK_PAD);
    PAD->PCR[paddr] = *((const uint32_t *)config);
    CLOCK_clockDisable(PCLK_PAD);
}

void PAD_setPinMux(uint32_t paddr, PadMux_e mux)
{
    ASSERT(paddr < PAD_ADDR_MAX_NUM);

    CLOCK_clockEnable(PCLK_PAD);
    PAD->PCR[paddr] = (PAD->PCR[paddr] & ~PAD_PCR_MUX_Msk) | EIGEN_VAL2FLD(PAD_PCR_MUX, mux);
    CLOCK_clockDisable(PCLK_PAD);

}

void PAD_setPinPullConfig(uint32_t paddr, PadPullConfig_e config)
{
    ASSERT(paddr < PAD_ADDR_MAX_NUM);

    CLOCK_clockEnable(PCLK_PAD);

    switch(config)
    {
        case PAD_INTERNAL_PULL_UP:
            PAD->PCR[paddr] = (PAD->PCR[paddr] & ~PAD_PCR_PULL_DOWN_ENABLE_Msk) | PAD_PCR_PULL_UP_ENABLE_Msk | PAD_PCR_PULL_SELECT_Msk;
            break;
        case PAD_INTERNAL_PULL_DOWN:
            PAD->PCR[paddr] = (PAD->PCR[paddr] & ~PAD_PCR_PULL_UP_ENABLE_Msk) | PAD_PCR_PULL_DOWN_ENABLE_Msk | PAD_PCR_PULL_SELECT_Msk;
            break;
        case PAD_AUTO_PULL:
            PAD->PCR[paddr] = (PAD->PCR[paddr] & ~PAD_PCR_PULL_SELECT_Msk);
            break;
        default:
            break;
    }

    CLOCK_clockDisable(PCLK_PAD);

}

