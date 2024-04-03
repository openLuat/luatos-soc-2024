/**
 *****************************************************************************
 * @brief   Device Peripheral Access Layer Header File.
 *          This file contains all the peripheral register's definitions, bits
 *          definitions and memory mapping for EC716S chip.
 *
 * @file   ec716s.h
 * @author
 * @date   12/Nov/2018
 *****************************************************************************
 *
 * THE PRESENT FIRMWARE WHICH IS FOR GUIDANCE ONLY AIMS AT PROVIDING CUSTOMERS
 * WITH CODING INFORMATION REGARDING THEIR PRODUCTS IN ORDER FOR THEM TO SAVE
 * TIME. AS A RESULT, STMICROELECTRONICS SHALL NOT BE HELD LIABLE FOR ANY
 * DIRECT, INDIRECT OR CONSEQUENTIAL DAMAGES WITH RESPECT TO ANY CLAIMS ARISING
 * FROM THE CONTENT OF SUCH FIRMWARE AND/OR THE USE MADE BY CUSTOMERS OF THE
 * CODING INFORMATION CONTAINED HEREIN IN CONNECTION WITH THEIR PRODUCTS.
 *
 * <b>&copy; COPYRIGHT 2018 AirM2M Technologies Ltd.</b>
 *
 *****************************************************************************
 */

#ifndef EC716S_H
#define EC716S_H

#ifdef __cplusplus
 extern "C" {
#endif
#include "commontypedef.h"

/* Start of section using anonymous unions and disabling warnings  */
#if   defined (__CC_ARM)
  #pragma push
  #pragma anon_unions
#elif defined (__ICCARM__)
  #pragma language=extended
#elif defined(__ARMCC_VERSION) && (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic push
  #pragma clang diagnostic ignored "-Wc11-extensions"
  #pragma clang diagnostic ignored "-Wreserved-id-macro"
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning 586
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

//#include "AP_Arm_Common.h"
/** @addtogroup Interrupt_number_definiton Interrupt Number Definition
  * @{
  */

#ifndef APIRQ_NUMBER_DEFINED
#define APIRQ_NUMBER_DEFINED

typedef enum IRQn
{
    /*  Cortex-M3 Processor Exceptions Numbers */
    NonMaskableInt_IRQn             = -14,      /**<  2 Non Maskable Interrupt */
    HardFault_IRQn                  = -13,      /**<  3 HardFault Interrupt */
    MemoryManagement_IRQn           = -12,      /**<  4 Memory Management Interrupt */
    BusFault_IRQn                   = -11,      /**<  5 Bus Fault Interrupt */
    UsageFault_IRQn                 = -10,      /**<  6 Usage Fault Interrupt */
    SVCall_IRQn                     =  -5,      /**< 11 SV Call Interrupt */
    DebugMonitor_IRQn               =  -4,      /**< 12 Debug Monitor Interrupt */
    PendSV_IRQn                     =  -2,      /**< 14 Pend SV Interrupt */
    SysTick_IRQn                    =  -1,      /**< 15 System Tick Interrupt */

    /* ARMCM3 Specific Interrupt Numbers  */
    PadWakeup0_IRQn                 =   0,      /**< Pad Wakeup0 Interrupt */
    PadWakeup1_IRQn                 =   1,      /**< Pad Wakeup1 Interrupt */
    PadWakeup2_IRQn                 =   2,      /**< Pad Wakeup2 Interrupt */
    PadWakeup3_IRQn                 =   3,      /**< Pad Wakeup3 Interrupt */
    PadWakeup4_IRQn                 =   4,      /**< Pad Wakeup4 Interrupt */
    PadWakeup5_IRQn                 =   5,      /**< Pad Wakeup5 Interrupt */
    LpuartWakeup_IRQn               =   6,      /**< Lpuart Wakeup Interrupt */
    LpusbWakeup_IRQn                =   7,      /**< Lpusb Wakeup Interrupt */
    PwrkeyWakeup_IRQn               =   8,      /**< Power Key Wakeup Interrupt */
    ChrgpadWakeup_IRQn              =   9,      /**< Charge Pad Wakeup Interrupt */
    RtcWakeup_IRQn                  =   10,     /**< RTC Wakeup Interrupt */
    Usb_IRQn                        =   11,     /**< USB Interrupt */
    TmuSystk_IRQn                   =   12,     /**< TMU Systk Interrupt */
    APXIC0_Normal_IRQn              =   13,     /**< AP XIC0 Interrupt */
    APXIC1_Normal_IRQn              =   14,     /**< AP XIC1 Interrupt */
    APXIC2_Normal_IRQn              =   15,     /**< AP XIC2 Interrupt */
    APXIC3_Normal_IRQn              =   16,     /**< AP XIC3 Interrupt */

    /* AP XIC0 for IPC/APB  */
    PXIC0_OVF_IRQn                  =   32,     /**< SW or HW Error Interrupt */
    PXIC0_USP2_IRQn                 =   33,     /**< USP2 Interrupt */
    PXIC0_USP1_IRQn                 =   34,     /**< USP1 Interrupt */
    PXIC0_USP0_IRQn                 =   35,     /**< USP0 Interrupt */
    PXIC0_SSP1_IRQn                 =   36,     /**< SSP1 Interrupt */
    PXIC0_SSP0_IRQn                 =   37,     /**< SSP0 Interrupt */
    PXIC0_UART3_IRQn                =   38,     /**< UART3 Interrupt */
    PXIC0_UART2_IRQn                =   39,     /**< UART2 Interrupt */
    PXIC0_UART1_IRQn                =   40,     /**< UART1 Interrupt */
    PXIC0_UART0_IRQn                =   41,     /**< UART0 Interrupt */
    PXIC0_TIMER5_IRQn               =   42,     /**< TIMER5 Interrupt */
    PXIC0_TIMER4_IRQn               =   43,     /**< TIMER4 Interrupt */
    PXIC0_TIMER3_IRQn               =   44,     /**< TIMER3 Interrupt */
    PXIC0_TIMER2_IRQn               =   45,     /**< TIMER2 Interrupt */
    PXIC0_TIMER1_IRQn               =   46,     /**< TIMER1 Interrupt */
    PXIC0_TIMER0_IRQn               =   47,     /**< TIMER0 Interrupt */
    PXIC0_KPC_DIRECT_IRQn           =   48,     /**< KPC DIRECT Interrupt */
    PXIC0_KPC_KEYPAD_IRQn           =   49,     /**< KPC KEYPAD Interrupt */
    PXIC0_ONEW_IRQn                 =   50,     /**< ONEW Interrupt */
    PXIC0_I2C1_IRQn                 =   51,     /**< I2C1 Interrupt */
    PXIC0_I2C0_IRQn                 =   52,     /**< I2C0 Interrupt */
    PXIC0_USIM1_IRQn                =   53,     /**< USIM1 Interrupt */
    PXIC0_USIM0_IRQn                =   54,     /**< USIM0 Interrupt */
    PXIC0_UTFC_ERR_IRQn             =   55,     /**< UTFC ERR Interrupt */
    PXIC0_PM_VBAT_LOW_IRQn          =   56,     /**< PM VBAT LOW Interrupt */
    PXIC0_WDG_CP_IRQn               =   57,     /**< WDG CP Interrupt */
    PXIC0_LOCKUP_CP_IRQn            =   58,     /**< LOCKUP CP Interrupt */
    PXIC0_RMIIF_TOERR_IRQn          =   59,     /**< RMIIF TOERR Interrupt */
    PXIC0_IPC_Merge_IRQn            =   60,     /**< IPC MERGE Interrupt */
    PXIC0_IPC_Alone1_IRQn           =   61,     /**< IPC ALONE1 Interrupt */
    PXIC0_IPC_Alone0_IRQn           =   62,     /**< IPC ALONE0 Interrupt */
    PXIC0_Sipc_IRQn                 =   63,     /**< IPC SIPC Interrupt */


    /* AP XIC1 for APB/AHB  */
    PXIC1_OVF_IRQn                  =   64,     /**< SW or HW Error Interrupt */
    PXIC1_CPMU_SLEEP_END_IRQn       =   65,     /**< CPMU SLEEP END Interrupt */
    PXIC1_CPMU_WAKEUP_END_IRQn      =   66,     /**< CPMU WAKEUP END Interrupt */
    PXIC1_CPMU_ASSIST_REQ_IRQn      =   67,     /**< CPMU ASSIST REQ Interrupt */
    PXIC1_CP_CPU_RST_REQ_IRQn       =   68,     /**< CP CPU RST REQ Interrupt */
    PXIC1_SCT_CH0_IRQn              =   69,     /**< SCT CH0 Interrupt */
    PXIC1_SCT_CH1_IRQn              =   70,     /**< SCT CH1 Interrupt */
    PXIC1_SCT_CH2_IRQn              =   71,     /**< SCT CH2 Interrupt */
    PXIC1_SCT_CH3_IRQn              =   72,     /**< SCT CH3 Interrupt */
    PXIC1_SCT_CH4_IRQn              =   73,     /**< SCT CH4 Interrupt */
    PXIC1_SCT_CH5_IRQn              =   74,     /**< SCT CH5 Interrupt */
    PXIC1_SCT_ERR_IRQn              =   75,     /**< SCT ERR Interrupt */
    PXIC1_TMU_BASECNT_CP_RT_IRQn    =   76,     /**< TMU BASECNT CP RT Interrupt */
    PXIC1_TMU_BASECNT_CP_BC_IRQn    =   77,     /**< TMU BASECNT CP BC Interrupt */
    PXIC1_TMU_BASECNT_AP_BC_IRQn    =   78,     /**< TMU BASECNT AP BC Interrupt */
    PXIC1_TMU_BASECNT_BC_LD_IRQn    =   79,     /**< TMU BASECNT BC LD Interrupt */
    PXIC1_TMU_CALCAL_LTCH_IRQn      =   80,     /**< TMU CALCAL LTCH Interrupt */
    PXIC1_UPMU_FINISH_IRQn          =   81,     /**< UPMU FINISH Interrupt */
    PXIC1_FUSE_FAIL_IRQn            =   82,     /**< FUSE FAIL Interrupt */
    PXIC1_FUSE_BURN_IRQn            =   83,     /**< FUSE BURN Interrupt */
    PXIC1_FUSE_READ_IRQn            =   84,     /**< FUSE READ Interrupt */
    PXIC1_DMA_AP_IRQn               =   85,     /**< DMA AP Interrupt */
    PXIC1_DMA_CP_IRQn               =   86,     /**< DMA CP Interrupt */
    PXIC1_DMA_MP_IRQn               =   87,     /**< DMA MP Interrupt */
    PXIC1_GPIO_IRQn                 =   88,     /**< GPIO Interrupt */
    PXIC1_TMU_SYSTK_IRQn            =   89,     /**< TMU SYSTK Interrupt */
    PXIC1_PSRAM_IRQn                =   90,     /**< PSRAM Interrupt */
    PXIC1_FLASH_IRQn                =   91,     /**< FLASH Interrupt */
    PXIC1_ULOG_ERR_IRQn             =   92,     /**< ULOG ERR Interrupt */
    PXIC1_ULOG_IRQn                 =   93,     /**< ULOG Interrupt */
    PXIC1_THM_HI_IRQn               =   94,     /**< THM HI Interrupt */
    PXIC1_AUXADC_IRQn               =   95,     /**< AUXADC Interrupt */

} IRQn_Type;

#endif

/**
  * @}
  */ /* end of group Interrupt_number_definiton */

/** @addtogroup Cortex_core_configuration Processor and Core Peripherals configuration
  * @{
  */

#define __CM3_REV                                0x0201U                        /**< Core revision r2p1 */
#define __MPU_PRESENT                            1U                             /**< MPU present */
#define __VTOR_PRESENT                           1U                             /**< VTOR present or not */
#define __NVIC_PRIO_BITS                         3U                             /**< Number of Bits used for Priority Levels */
#define __Vendor_SysTickConfig                   0U                             /**< Set to 1 if different SysTick Config is used */


#include "core_cm3.h"

#include "system_ec7xx.h"

/**
  * @}
  */ /* end of group Cortex_core_configuration */

/** @addtogroup Device_peripheral_access Device Peripheral Access
  * @{
  */


/** @addtogroup BASE_ADDR Peripheral instance base addresses
  * @{
  */

/**
  * @brief AP XIC base address
  *
  */
#define APXIC_BASE_ADDR                          ((unsigned int)0x40020000)

#define APXIC0_BASE_ADDR                         (APXIC_BASE_ADDR+0x0000)           /**< APXIC0(manage APB/IPC interrupt) base address */
#define APXIC1_BASE_ADDR                         (APXIC_BASE_ADDR+0x1000)           /**< APXIC1(manage AHB/APB interupt) base address */
#define APXIC2_BASE_ADDR                         (APXIC_BASE_ADDR+0x2000)           /**< APXIC2(manage ULDP interupt) base address */
#define APXIC3_BASE_ADDR                         (APXIC_BASE_ADDR+0x3000)           /**< APXIC3(manage USB interupt) base address */

/**
  * @brief AP APB base address
  *
  */
#define APB_AP_PERIPH_BASE                       ((unsigned int)0x4C000000)         /**< APB AP peripheral base address */
#define RMI_AP_PERIPH_BASE                       ((unsigned int)0x40000000)         /**< RMI AP peripheral base address */

/**
  * @brief CP APB base address
  *
  */
#define APB_CP_PERIPH_BASE                       ((unsigned int)0x5C000000)         /**< APB CP peripheral base address */
#define RMI_CP_PERIPH_BASE                       ((unsigned int)0x50000000)         /**< RMI CP peripheral base address */

/**
  * @brief MP APB base address
  *
  */
#define APB_MP_PERIPH_BASE                       ((unsigned int)0x4D000000)         /**< APB MP peripheral base address */


/**
  * @brief GP APB base address
  *
  */
#define APB_GP_PERIPH_BASE                       ((unsigned int)0x4F000000)         /**< APB GP peripheral base address */


/**
  * @brief RMI AP Address
  *
  */
#define RMI_GPIO_BASE_ADDR                        (RMI_AP_PERIPH_BASE + 0x040000)    /**< RMI GPIO base address */


/**
  * @brief APB AP Address
  *
  */
#define AP_WDG_BASE_ADDR                         (APB_AP_PERIPH_BASE + 0x010000)    /**< AP WDG base address */
#define AP_TIMER0_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x020000)    /**< AP TIMER0 base address */
#define AP_TIMER1_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x021000)    /**< AP TIMER1 base address */
#define AP_TIMER2_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x022000)    /**< AP TIMER2 base address */
#define AP_TIMER3_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x023000)    /**< AP TIMER3 base address */
#define AP_TIMER4_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x024000)    /**< AP TIMER4 base address */
#define AP_TIMER5_BASE_ADDR                      (APB_AP_PERIPH_BASE + 0x025000)    /**< AP TIMER5 base address */
#define AP_IPC_BASE_ADDR                         (APB_AP_PERIPH_BASE + 0x030000)    /**< AP IPC base address */
#define AP_USIM0_BASE_ADDR                       (APB_AP_PERIPH_BASE + 0x040000)    /**< AP USIM0 base address */
#define AP_USIM1_BASE_ADDR                       (APB_AP_PERIPH_BASE + 0x041000)    /**< AP USIM1 base address */
#define AP_KPC_BASE_ADDR                         (APB_AP_PERIPH_BASE + 0x050000)    /**< AP KPC base address */
#define AP_ONEWIRE_BASE_ADDR                     (APB_AP_PERIPH_BASE + 0x060000)    /**< AP ONEWIRE base address */


/**
  * @brief APB GP Address
  *
  */
#define GP_GPR_BASE_ADDR                         (APB_GP_PERIPH_BASE + 0x000000)    /**< GP GPR base address */
#define GP_GPR_SC_BASE_ADDR                      (APB_GP_PERIPH_BASE + 0x000000)    /**< GP GPR SC base address */
#define GP_SIPC_BASE_ADDR                        (APB_GP_PERIPH_BASE + 0x010000)    /**< GP SIPC base address */
#define GP_AON_BASE_ADDR                         (APB_GP_PERIPH_BASE + 0x020000)    /**< GP AON base address */
#define GP_CPMU_BASE_ADDR                        (APB_GP_PERIPH_BASE + 0x030000)    /**< GP CPMU base address */
#define GP_PMDIG_BASE_ADDR                       (APB_GP_PERIPH_BASE + 0x040000)    /**< GP RFDIG base address */
#define GP_RFDIG_BASE_ADDR                       (APB_GP_PERIPH_BASE + 0x050000)    /**< GP RFDIG base address */
#define GP_PAD_BASE_ADDR                         (APB_GP_PERIPH_BASE + 0x060000)    /**< GP PAD base address */
#define GP_TMU_BASE_ADDR                         (APB_GP_PERIPH_BASE + 0x070000)    /**< GP TMU base address */
#define GP_FUSE_BASE_ADDR                        (APB_GP_PERIPH_BASE + 0x080000)    /**< GP FUSE base address */
#define GP_TRNG_BASE_ADDR                        (APB_GP_PERIPH_BASE + 0x090000)    /**< GP TRNG base address */
#define GP_USB_BASE_ADDR                         (APB_GP_PERIPH_BASE + 0x0A0000)    /**< GP USB base address */


/**
  * @brief APB MP Address
  *
  */
#define MP_UART0_BASE_ADDR                       (APB_MP_PERIPH_BASE + 0x010000)    /**< MP UART0 base address */
#define MP_UART1_BASE_ADDR                       (APB_MP_PERIPH_BASE + 0x011000)    /**< MP UART1 base address */
#define MP_UART2_BASE_ADDR                       (APB_MP_PERIPH_BASE + 0x012000)    /**< MP UART2 base address */
#define MP_UART3_BASE_ADDR                       (APB_MP_PERIPH_BASE + 0x013000)    /**< MP UART2 base address */
#define MP_I2C0_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x020000)    /**< MP I2C0 base address */
#define MP_I2C1_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x021000)    /**< MP I2C1 base address */
#define MP_SSP0_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x030000)    /**< MP SSP0 base address */
#define MP_SSP1_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x031000)    /**< MP SSP1 base address */
#define MP_USP0_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x040000)    /**< MP USP0 base address */
#define MP_USP1_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x041000)    /**< MP USP1 base address */
#define MP_USP2_BASE_ADDR                        (APB_MP_PERIPH_BASE + 0x042000)    /**< MP USP1 base address */
#define MP_DMA_BASE_ADDR                         (APB_MP_PERIPH_BASE + 0x1F0000)    /**< MP DMA base address */

#define LPUSARTAON_BASE_ADDR                     (GP_AON_BASE_ADDR + 0x000154)      /**< LPUSART AON base address */

/**
  * @}
  */ /* end of group BASE_ADDR */


/** @addtogroup ADC ADC(Analog-to-Digital converter)
  * @{
  */

/**
  * @brief ADC register layout typedef
  *
  */
typedef struct {
    __IO uint32_t CTRL;                          /**< ADC control Register,                         offset: 0x0 */
    __IO uint32_t CFG;                           /**< ADC configuration Register,                   offset: 0x4 */
    __IO uint32_t AIOCFG;                        /**< AIO configuration Register,                   offset: 0x8 */
    __I  uint32_t STATUS;                        /**< ADC status Register,                          offset: 0xC */
    __I  uint32_t RESULT;                        /**< ADC data result Register,                     offset: 0x10 */
} ADC_TypeDef;

/** @name CTRL - ADC_CTRL register */
/** @{ */
#define ADC_CTRL_RSTN_Pos                        (0)
#define ADC_CTRL_RSTN_Msk                        (0x1UL << ADC_CTRL_RSTN_Pos)

#define ADC_CTRL_EN_Pos                          (1)
#define ADC_CTRL_EN_Msk                          (0x1UL << ADC_CTRL_EN_Pos)

#define ADC_CTRL_LDO_EN_Pos                      (2)
#define ADC_CTRL_LDO_EN_Msk                      (0x1UL << ADC_CTRL_LDO_EN_Pos)
/** @} */

/** @name CFG - ADC_CFG register */
/** @{ */
#define ADC_CFG_WAIT_CTRL_Pos                    (1)
#define ADC_CFG_WAIT_CTRL_Msk                    (0x3UL << ADC_CFG_WAIT_CTRL_Pos)

#define ADC_CFG_OFFSET_CTRL_Pos                  (3)
#define ADC_CFG_OFFSET_CTRL_Msk                  (0x7UL << ADC_CFG_OFFSET_CTRL_Pos)

#define ADC_CFG_SAMPLE_AVG_Pos                   (6)
#define ADC_CFG_SAMPLE_AVG_Msk                   (0x1UL << ADC_CFG_SAMPLE_AVG_Pos)

#define ADC_CFG_CLKIN_DIV_Pos                    (7)
#define ADC_CFG_CLKIN_DIV_Msk                    (0x3UL << ADC_CFG_CLKIN_DIV_Pos)

#define ADC_CFG_VREF_BS_Pos                      (9)
#define ADC_CFG_VREF_BS_Msk                      (0x1UL << ADC_CFG_VREF_BS_Pos)

#define ADC_CFG_VREF_SEL_Pos                     (10)
#define ADC_CFG_VREF_SEL_Msk                     (0x7UL << ADC_CFG_VREF_SEL_Pos)

#define ADC_CFG_LDO_SEL_Pos                      (13)
#define ADC_CFG_LDO_SEL_Msk                      (0x7UL << ADC_CFG_LDO_SEL_Pos)

#define ADC_CFG_IBIAS_SEL_Pos                    (16)
#define ADC_CFG_IBIAS_SEL_Msk                    (0x3UL << ADC_CFG_IBIAS_SEL_Pos)
/** @} */

/** @name AIOCFG - ADC_AIOCFG register */
/** @{ */
#define ADC_AIOCFG_THM_SEL_Pos                   (0)
#define ADC_AIOCFG_THM_SEL_Msk                   (0x3UL << ADC_AIOCFG_THM_SEL_Pos)

#define ADC_AIOCFG_VBATSEN_RDIV_Pos              (2)
#define ADC_AIOCFG_VBATSEN_RDIV_Msk              (0x7UL << ADC_AIOCFG_VBATSEN_RDIV_Pos)

#define ADC_AIOCFG_RDIV_Pos                      (5)
#define ADC_AIOCFG_RDIV_Msk                      (0x7UL << ADC_AIOCFG_RDIV_Pos)

#define ADC_AIOCFG_RDIV_BYP_Pos                  (8)
#define ADC_AIOCFG_RDIV_BYP_Msk                  (0x1UL << ADC_AIOCFG_RDIV_BYP_Pos)

#define ADC_AIOCFG_THM_EN_Pos                    (9)
#define ADC_AIOCFG_THM_EN_Msk                    (0x1UL << ADC_AIOCFG_THM_EN_Pos)

#define ADC_AIOCFG_BATSENS_EN_Pos                (10)
#define ADC_AIOCFG_BATSENS_EN_Msk                (0x1UL << ADC_AIOCFG_BATSENS_EN_Pos)

#define ADC_AIOCFG_AIO4_EN_Pos                   (11)
#define ADC_AIOCFG_AIO4_EN_Msk                   (0x1UL << ADC_AIOCFG_AIO4_EN_Pos)

#define ADC_AIOCFG_AIO3_EN_Pos                   (12)
#define ADC_AIOCFG_AIO3_EN_Msk                   (0x1UL << ADC_AIOCFG_AIO3_EN_Pos)

#define ADC_AIOCFG_AIO2_EN_Pos                   (13)
#define ADC_AIOCFG_AIO2_EN_Msk                   (0x1UL << ADC_AIOCFG_AIO2_EN_Pos)

#define ADC_AIOCFG_AIO1_EN_Pos                   (14)
#define ADC_AIOCFG_AIO1_EN_Msk                   (0x1UL << ADC_AIOCFG_AIO1_EN_Pos)

#define ADC_AIOCFG_AIO4_NO_DIVR_EN_Pos           (15)
#define ADC_AIOCFG_AIO4_NO_DIVR_EN_Msk           (0x1UL << ADC_AIOCFG_AIO4_NO_DIVR_EN_Pos)

#define ADC_AIOCFG_AIO3_NO_DIVR_EN_Pos           (16)
#define ADC_AIOCFG_AIO3_NO_DIVR_EN_Msk           (0x1UL << ADC_AIOCFG_AIO3_NO_DIVR_EN_Pos)

#define ADC_AIOCFG_AIO2_NO_DIVR_EN_Pos           (17)
#define ADC_AIOCFG_AIO2_NO_DIVR_EN_Msk           (0x1UL << ADC_AIOCFG_AIO2_NO_DIVR_EN_Pos)

#define ADC_AIOCFG_AIO1_NO_DIVR_EN_Pos           (18)
#define ADC_AIOCFG_AIO1_NO_DIVR_EN_Msk           (0x1UL << ADC_AIOCFG_AIO1_NO_DIVR_EN_Pos)

#define ADC_AIOCFG_VREF2AIO1_EN_Pos              (19)
#define ADC_AIOCFG_VREF2AIO1_EN_Msk              (0x1UL << ADC_AIOCFG_VREF2AIO1_EN_Pos)
/** @} */

/** @name STATUS - ADC_STATUS register */
/** @{ */
#define ADC_STATUS_DATA_VALID_Pos                (0)
#define ADC_STATUS_DATA_VALID_Msk                (0x1UL << ADC_STATUS_DATA_VALID_Pos)
/** @} */

/** @name RESULT - ADC_RESULT register */
/** @{ */
#define ADC_RESULT_DATA_Pos                      (0)
#define ADC_RESULT_DATA_Msk                      (0xFFFUL << ADC_RESULT_DATA_Pos)
/** @} */

/** Peripheral ADC base pointer */
#define ADC_BASE_ADDR                            (GP_RFDIG_BASE_ADDR + 0x600)
#define ADC                                      ((ADC_TypeDef *)ADC_BASE_ADDR)

/**
  * @}
  */ /* end of group ADC */


/** @addtogroup GPIO GPIO
  * @{
  */

/**
  * @brief GPIO register layout typedef
  *
  */
typedef struct {
    __IO uint32_t DATA;                          /**< Data Input Register,                offset: 0x0 */
    __IO uint32_t DATAOUT;                       /**< Data Output Register,               offset: 0x4 */
         uint32_t RESERVED_0[2];
    __IO uint32_t OUTENSET;                      /**< Output Enable Set Register,         offset: 0x10 */
    __IO uint32_t OUTENCLR;                      /**< Output Enable Clear Register,       offset: 0x14 */
         uint32_t RESERVED_1[2];
    __IO uint32_t INTENSET;                      /**< Interrupt Enable Set Register,      offset: 0x20 */
    __IO uint32_t INTENCLR;                      /**< Interrupt Enable Clear Register,    offset: 0x24 */
    __IO uint32_t INTTYPESET;                    /**< Interrupt Type set Register,        offset: 0x28 */
    __IO uint32_t INTTYPECLR;                    /**< Interrupt Type Clear Register,      offset: 0x2C */
    __IO uint32_t INTPOLSET;                     /**< Interrupt Polarity Set Register,    offset: 0x30 */
    __IO uint32_t INTPOLCLR;                     /**< Interrupt Polarity Clear Register,  offset: 0x34 */
    __IO uint32_t INTSTATUS;                     /**< Interrupt Status Register,          offset: 0x38 */
    __IO uint32_t INTEDGESET;                    /**< Interrupt Edge Set Register,        offset: 0x3C */
    __IO uint32_t INTEDGECLR;                    /**< Interrupt Edge Clear Register,      offset: 0x40 */
         uint32_t RESERVED_3[239];
    __IO uint32_t MASKLOWBYTE[256];              /**< Lower Eight Bits Masked Access Register, array offset: 0x400, array step: 0x4 */
    __IO uint32_t MASKHIGHBYTE[256];             /**< Higher Eight Bits Masked Access Register, array offset: 0x800, array step: 0x4 */
} GPIO_TypeDef;

/** @name DATA - GPIO_DATA register */
/** @{ */
#define GPIO_DATA_DVAL_Pos                       (0)
#define GPIO_DATA_DVAL_Msk                       (0xFFFFUL << GPIO_DATA_DVAL_Pos)
/** @} */

/** @name DATAOUT - GPIO_DATAOUT register */
/** @{ */
#define GPIO_DATAOUT_DOVAL_Pos                   (0)
#define GPIO_DATAOUT_DOVAL_Msk                   (0xFFFFUL << GPIO_DATAOUT_DOVAL_Pos)
/** @} */

/** @name OUTENSET - GPIO_OUTENSET register */
/** @{ */
#define GPIO_OUTENSET_OEN_Pos                    (0)
#define GPIO_OUTENSET_OEN_Msk                    (0xFFFFUL << GPIO_OUTENSET_OEN_Pos)
/** @} */

/** @name OUTENCLR - GPIO_OUTENCLR register */
/** @{ */
#define GPIO_OUTENCLR_OCLR_Pos                   (0)
#define GPIO_OUTENCLR_OCLR_Msk                   (0xFFFFUL << GPIO_OUTENCLR_OCLR_Pos)
/** @} */

/** @name INTENSET - GPIO_INTENSET register */
/** @{ */
#define GPIO_INTENSET_INEN_Pos                   (0)
#define GPIO_INTENSET_INEN_Msk                   (0xFFFFUL << GPIO_INTENSET_INEN_Pos)
/** @} */

/** @name INTENCLR - GPIO_INTENCLR register */
/** @{ */
#define GPIO_INTENCLR_INCLR_Pos                  (0)
#define GPIO_INTENCLR_INCLR_Msk                  (0xFFFFUL << GPIO_INTENCLR_INCLR_Pos)
/** @} */

/** @name INTTYPESET - GPIO_INTTYPESET register */
/** @{ */
#define GPIO_INTTYPESET_INTYPE_Pos               (0)
#define GPIO_INTTYPESET_INTYPE_Msk               (0xFFFFUL << GPIO_INTTYPESET_INTYPE_Pos)
/** @} */

/** @name INTTYPECLR - GPIO_INTTYPECLR register */
/** @{ */
#define GPIO_INTTYPECLR_INTYPECLR_Pos            (0)
#define GPIO_INTTYPECLR_INTYPECLR_Msk            (0xFFFFUL << GPIO_INTTYPECLR_INTYPECLR_Pos)
/** @} */

/** @name INTPOLSET - GPIO_INTPOLSET register */
/** @{ */
#define GPIO_INTPOLSET_INPOL_Pos                 (0)
#define GPIO_INTPOLSET_INPOL_Msk                 (0xFFFFUL << GPIO_INTPOLSET_INPOL_Pos)
/** @} */

/** @name INTPOLCLR - GPIO_INTPOLCLR register */
/** @{ */
#define GPIO_INTPOLCLR_INPOLCLR_Pos              (0)
#define GPIO_INTPOLCLR_INPOLCLR_Msk              (0xFFFFUL << GPIO_INTPOLCLR_INPOLCLR_Pos)
/** @} */

/** @name INTSTATUS - GPIO_INTSTATUS register */
/** @{ */
#define GPIO_INTSTATUS_INSTAT_Pos                (0)
#define GPIO_INTSTATUS_INSTAT_Msk                (0xFFFFUL << GPIO_INTSTATUS_INSTAT_Pos)
/** @} */

/** @name INTEDGESET - GPIO_INTEDGESET register */
/** @{ */
#define GPIO_INTEDGESET_INEDGE_Pos               (0)
#define GPIO_INTEDGESET_INEDGE_Msk               (0xFFFFUL << GPIO_INTEDGESET_INEDGE_Pos)
/** @} */

/** @name INTEDGECLR - GPIO_INTEDGECLR register */
/** @{ */
#define GPIO_INTEDGECLR_INEDGECLR_Pos            (0)
#define GPIO_INTEDGECLR_INEDGECLR_Msk            (0xFFFFUL << GPIO_INTEDGECLR_INEDGECLR_Pos)
/** @} */

/** @name MASKLOWBYTE - GPIO_MASKLOWBYTE N register */
/** @{ */
#define GPIO_MASKLOWBYTE_MASK_Pos                (0)
#define GPIO_MASKLOWBYTE_MASK_Msk                (0xFFFFUL << GPIO_MASKLOWBYTE_MASK_Pos)
/** @} */

/** @name MASKHIGHBYTE - GPIO_MASKHIGHBYTE N register */
/** @{ */
#define GPIO_MASKHIGHBYTE_MASK_Pos               (0)
#define GPIO_MASKHIGHBYTE_MASK_Msk               (0xFFFFUL << GPIO_MASKHIGHBYTE_MASK_Pos)
/** @} */

/** Peripheral GPIO0 base address */
#define GPIO0_BASE_ADDR                          (RMI_GPIO_BASE_ADDR + 0x0000)

/** Peripheral GPIO0 base pointer */
#define GPIO_0                                   ((GPIO_TypeDef *) GPIO0_BASE_ADDR)

/** Peripheral GPIO1 base address */
#define GPIO1_BASE_ADDR                          (RMI_GPIO_BASE_ADDR + 0x1000)

/** Peripheral GPIO1 base pointer */
#define GPIO_1                                   ((GPIO_TypeDef *) GPIO1_BASE_ADDR)

/** @brief GPIO peripheral instance number */
#define GPIO_INSTANCE_NUM                        (2)

/**
  * @}
  */ /* end of group ADC */


/** @addtogroup I2C I2C
  * @{
  */

/**
  * @brief I2C register layout typedef
  *
  */
typedef struct {
    __IO uint32_t CR0;                           /**< offset: 0x0*/
    __IO uint32_t TAR;                           /**< offset: 0x4*/
    __IO uint32_t SAR;                           /**< offset: 0x8*/
         uint32_t RESERVED_0;
    __IO uint32_t COMMAND;                       /**< offset: 0x10*/
    __IO uint32_t SCLCNT[4];                     /**< array offset: 0x14, array step: 0x4*/
         uint32_t RESERVED_1[2];
    __I  uint32_t MISR;                          /**< offset: 0x2C*/
    __IO uint32_t IMR;                           /**< offset: 0x30*/
    __I  uint32_t RISR;                          /**< offset: 0x34*/
    __IO uint32_t RXFCR;                         /**< offset: 0x38*/
    __IO uint32_t TXFCR;                         /**< offset: 0x3C*/
    __IO uint32_t ICR[11];                       /**< array offset: 0x40, array step: 0x4*/
    __IO uint32_t CR1;                           /**< offset: 0x6C*/
    __I  uint32_t STR0;                          /**< offset: 0x70*/
    __I  uint32_t TXFSR;                         /**< offset: 0x74*/
    __I  uint32_t RXFSR;                         /**< offset: 0x78*/
    __IO uint32_t SDAHOLD;                       /**< offset: 0x7C*/
    __I  uint32_t STR1;                          /**< offset: 0x80*/
         uint32_t RESERVED_2[4];
    __IO uint32_t SDASETUP;                      /**< offset: 0x94*/
    __IO uint32_t ACKGC;                         /**< offset: 0x98*/
    __I  uint32_t STR2;                          /**< offset: 0x9C*/
    __IO uint32_t SPKSR;                         /**< offset: 0xA0*/
         uint32_t RESERVED_3[2];
    __IO uint32_t STUCKTIMEOUT[2];               /**< array offset: 0xAC, array step: 0x4*/
    __IO uint32_t CLRSCLSTUCKDET;                /**< offset: 0xB4*/
} I2C_TypeDef;


/** @name CR0 - I2C_CR0 register */
/** @{ */
#define I2C_CR0_MASTER_CTRL_Pos                  (0)
#define I2C_CR0_MASTER_CTRL_Msk                  (0x1UL << I2C_CR0_MASTER_CTRL_Pos)

#define I2C_CR0_SS_Pos                           (1)
#define I2C_CR0_SS_Msk                           (0x3UL << I2C_CR0_SS_Pos)

#define I2C_CR0_SLAVE_10BIT_Pos                  (3)
#define I2C_CR0_SLAVE_10BIT_Msk                  (0x1UL << I2C_CR0_SLAVE_10BIT_Pos)

#define I2C_CR0_REP_START_Pos                    (5)
#define I2C_CR0_REP_START_Msk                    (0x1UL << I2C_CR0_REP_START_Pos)

#define I2C_CR0_SLAVE_CTRL_Pos                   (6)
#define I2C_CR0_SLAVE_CTRL_Msk                   (0x3UL << I2C_CR0_SLAVE_CTRL_Pos)

#define I2C_CR0_STOP_DET_CTRL_Pos                (10)
#define I2C_CR0_STOP_DET_CTRL_Msk                (0x1UL << I2C_CR0_STOP_DET_CTRL_Pos)

#define I2C_CR0_BUS_CLEAR_Pos                    (11)
#define I2C_CR0_BUS_CLEAR_Msk                    (0x1UL << I2C_CR0_BUS_CLEAR_Pos)
/** @} */

/** @name TAR - I2C_TAR register */
/** @{ */
#define I2C_TAR_ADDRESS_Pos                      (0)
#define I2C_TAR_ADDRESS_Msk                      (0x3FFUL << I2C_TAR_ADDRESS_Pos)

#define I2C_TAR_ISSUE_GC_START_Pos               (10)
#define I2C_TAR_ISSUE_GC_START_Msk               (0x3UL << I2C_TAR_ISSUE_GC_START_Pos)

#define I2C_TAR_MASTER_10BIT_Pos                 (12)
#define I2C_TAR_MASTER_10BIT_Msk                 (0x1UL << I2C_TAR_MASTER_10BIT_Pos)

/** @} */

/** @name SAR - I2C_SAR register */
/** @{ */
#define I2C_SAR_ADDRESS_Pos                      (0)
#define I2C_SAR_ADDRESS_Msk                      (0x3FFUL << I2C_SAR_ADDRESS_Pos)
/** @} */

/** @name COMMAND - I2C_COMMAND register */
/** @{ */
#define I2C_COMMAND_TRXD_Pos                     (0)
#define I2C_COMMAND_TRXD_Msk                     (0xFFUL << I2C_COMMAND_TRXD_Pos)

#define I2C_COMMAND_RWN_Pos                      (8)
#define I2C_COMMAND_RWN_Msk                      (0x1UL << I2C_COMMAND_RWN_Pos)

#define I2C_COMMAND_STOP_Pos                     (9)
#define I2C_COMMAND_STOP_Msk                     (0x1UL << I2C_COMMAND_STOP_Pos)

#define I2C_COMMAND_REP_START_Pos                (10)
#define I2C_COMMAND_REP_START_Msk                (0x1UL << I2C_COMMAND_REP_START_Pos)
/** @} */

/** @name SCLCNT - I2C_SCLCNT N register */
/** @{ */
#define I2C_SCLCNT_CNT_Pos                       (0)
#define I2C_SCLCNT_CNT_Msk                       (0xFFFFUL << I2C_SCLCNT_CNT_Pos)
/** @} */


/** @name MISR - I2C_MISR register */
/** @{ */
#define I2C_MISR_RX_UNDER_Pos                    (0)
#define I2C_MISR_RX_UNDER_Msk                    (0x1UL << I2C_MISR_RX_UNDER_Pos)

#define I2C_MISR_RX_OVER_Pos                     (1)
#define I2C_MISR_RX_OVER_Msk                     (0x1UL << I2C_MISR_RX_OVER_Pos)

#define I2C_MISR_RX_WL_Pos                       (2)
#define I2C_MISR_RX_WL_Msk                       (0x1UL << I2C_MISR_RX_WL_Pos)

#define I2C_MISR_TX_OVER_Pos                     (3)
#define I2C_MISR_TX_OVER_Msk                     (0x1UL << I2C_MISR_TX_OVER_Pos)

#define I2C_MISR_TX_WL_Pos                       (4)
#define I2C_MISR_TX_WL_Msk                       (0x1UL << I2C_MISR_TX_WL_Pos)

#define I2C_MISR_TX_ABRT_Pos                     (6)
#define I2C_MISR_TX_ABRT_Msk                     (0x1UL << I2C_MISR_TX_ABRT_Pos)

#define I2C_MISR_ACTIVITY_Pos                    (8)
#define I2C_MISR_ACTIVITY_Msk                    (0x1UL << I2C_MISR_ACTIVITY_Pos)

#define I2C_MISR_STOP_DET_Pos                    (9)
#define I2C_MISR_STOP_DET_Msk                    (0x1UL << I2C_MISR_STOP_DET_Pos)

#define I2C_MISR_START_DET_Pos                   (10)
#define I2C_MISR_START_DET_Msk                   (0x1UL << I2C_MISR_START_DET_Pos)

#define I2C_MISR_GEN_CALL_Pos                    (11)
#define I2C_MISR_GEN_CALL_Msk                    (0x1UL << I2C_MISR_GEN_CALL_Pos)

#define I2C_MISR_MASTER_ON_HOLD_Pos              (13)
#define I2C_MISR_MASTER_ON_HOLD_Msk              (0x1UL << I2C_MISR_MASTER_ON_HOLD_Pos)

#define I2C_MISR_SCL_STUCK_Pos                   (14)
#define I2C_MISR_SCL_STUCK_Msk                   (0x1UL << I2C_MISR_SCL_STUCK_Pos)
/** @} */

/** @name IMR - I2C_IMR register */
/** @{ */
#define I2C_IMR_RX_UNDER_Pos                     (0)
#define I2C_IMR_RX_UNDER_Msk                     (0x1UL << I2C_IMR_RX_UNDER_Pos)

#define I2C_IMR_RX_OVER_Pos                      (1)
#define I2C_IMR_RX_OVER_Msk                      (0x1UL << I2C_IMR_RX_OVER_Pos)

#define I2C_IMR_RX_WL_Pos                        (2)
#define I2C_IMR_RX_WL_Msk                        (0x1UL << I2C_IMR_RX_WL_Pos)

#define I2C_IMR_TX_OVER_Pos                      (3)
#define I2C_IMR_TX_OVER_Msk                      (0x1UL << I2C_IMR_TX_OVER_Pos)

#define I2C_IMR_TX_WL_Pos                        (4)
#define I2C_IMR_TX_WL_Msk                        (0x1UL << I2C_IMR_TX_WL_Pos)

#define I2C_IMR_TX_ABRT_Pos                      (6)
#define I2C_IMR_TX_ABRT_Msk                      (0x1UL << I2C_IMR_TX_ABRT_Pos)

#define I2C_IMR_ACTIVITY_Pos                     (8)
#define I2C_IMR_ACTIVITY_Msk                     (0x1UL << I2C_IMR_ACTIVITY_Pos)

#define I2C_IMR_STOP_DET_Pos                     (9)
#define I2C_IMR_STOP_DET_Msk                     (0x1UL << I2C_IMR_STOP_DET_Pos)

#define I2C_IMR_START_DET_Pos                    (10)
#define I2C_IMR_START_DET_Msk                    (0x1UL << I2C_IMR_START_DET_Pos)

#define I2C_IMR_GEN_CALL_Pos                     (11)
#define I2C_IMR_GEN_CALL_Msk                     (0x1UL << I2C_IMR_GEN_CALL_Pos)

#define I2C_IMR_MASTER_ON_HOLD_Pos               (13)
#define I2C_IMR_MASTER_ON_HOLD_Msk               (0x1UL << I2C_IMR_MASTER_ON_HOLD_Pos)

#define I2C_IMR_SCL_STUCK_Pos                    (14)
#define I2C_IMR_SCL_STUCK_Msk                    (0x1UL << I2C_IMR_SCL_STUCK_Pos)
/** @} */

/** @name RISR - I2C_RISR register */
/** @{ */
#define I2C_RISR_RX_UNDER_Pos                    (0)
#define I2C_RISR_RX_UNDER_Msk                    (0x1UL << I2C_RISR_RX_UNDER_Pos)

#define I2C_RISR_RX_OVER_Pos                     (1)
#define I2C_RISR_RX_OVER_Msk                     (0x1UL << I2C_RISR_RX_OVER_Pos)

#define I2C_RISR_RX_WL_Pos                       (2)
#define I2C_RISR_RX_WL_Msk                       (0x1UL << I2C_RISR_RX_WL_Pos)

#define I2C_RISR_TX_OVER_Pos                     (3)
#define I2C_RISR_TX_OVER_Msk                     (0x1UL << I2C_RISR_TX_OVER_Pos)

#define I2C_RISR_TX_WL_Pos                       (4)
#define I2C_RISR_TX_WL_Msk                       (0x1UL << I2C_RISR_TX_WL_Pos)

#define I2C_RISR_TX_ABRT_Pos                     (6)
#define I2C_RISR_TX_ABRT_Msk                     (0x1UL << I2C_RISR_TX_ABRT_Pos)

#define I2C_RISR_ACTIVITY_Pos                    (8)
#define I2C_RISR_ACTIVITY_Msk                    (0x1UL << I2C_RISR_ACTIVITY_Pos)

#define I2C_RISR_STOP_DET_Pos                    (9)
#define I2C_RISR_STOP_DET_Msk                    (0x1UL << I2C_RISR_STOP_DET_Pos)

#define I2C_RISR_START_DET_Pos                   (10)
#define I2C_RISR_START_DET_Msk                   (0x1UL << I2C_RISR_START_DET_Pos)

#define I2C_RISR_GEN_CALL_Pos                    (11)
#define I2C_RISR_GEN_CALL_Msk                    (0x1UL << I2C_RISR_GEN_CALL_Pos)

#define I2C_RISR_MASTER_ON_HOLD_Pos              (13)
#define I2C_RISR_MASTER_ON_HOLD_Msk              (0x1UL << I2C_RISR_MASTER_ON_HOLD_Pos)

#define I2C_RISR_SCL_STUCK_Pos                   (14)
#define I2C_RISR_SCL_STUCK_Msk                   (0x1UL << I2C_RISR_SCL_STUCK_Pos)
/** @} */

/** @name RXFCR - I2C_RXFCR register */
/** @{ */
#define I2C_RXFCR_TH_Pos                         (0)
#define I2C_RXFCR_TH_Msk                         (0xFFUL << I2C_RXFCR_TH_Pos)
/** @} */

/** @name TXFCR - I2C_TXFCR register */
/** @{ */
#define I2C_TXFCR_TH_Pos                         (0)
#define I2C_TXFCR_TH_Msk                         (0xFFUL << I2C_TXFCR_TH_Pos)
/** @} */

/** @name ICR - I2C_ICR N register */
/** @{ */
#define I2C_ICR_CLR_Pos                          (0)
#define I2C_ICR_CLR_Msk                          (0x1UL << I2C_ICR_CLR_Pos)
/** @} */

/** @name CR1 - I2C_CR1 register */
/** @{ */
#define I2C_CR1_ENABLE_Pos                       (0)
#define I2C_CR1_ENABLE_Msk                       (0x1UL << I2C_CR1_ENABLE_Pos)

#define I2C_CR1_ABORT_Pos                        (1)
#define I2C_CR1_ABORT_Msk                        (0x1UL << I2C_CR1_ABORT_Pos)

#define I2C_CR1_TX_SUSPEND_Pos                   (2)
#define I2C_CR1_TX_SUSPEND_Msk                   (0x1UL << I2C_CR1_TX_SUSPEND_Pos)

#define I2C_CR1_SDA_RECOV_Pos                    (3)
#define I2C_CR1_SDA_RECOV_Msk                    (0x1UL << I2C_CR1_SDA_RECOV_Pos)
/** @} */

/** @name STR0 - I2C_STR0 register */
/** @{ */
#define I2C_STR0_TFNF_Pos                        (1)
#define I2C_STR0_TFNF_Msk                        (0x1UL << I2C_STR0_TFNF_Pos)

#define I2C_STR0_TFE_Pos                         (2)
#define I2C_STR0_TFE_Msk                         (0x1UL << I2C_STR0_TFE_Pos)

#define I2C_STR0_RFNE_Pos                        (3)
#define I2C_STR0_RFNE_Msk                        (0x1UL << I2C_STR0_RFNE_Pos)

#define I2C_STR0_RFF_Pos                         (4)
#define I2C_STR0_RFF_Msk                         (0x1UL << I2C_STR0_RFF_Pos)

#define I2C_STR0_MASTER_BUSY_Pos                 (5)
#define I2C_STR0_MASTER_BUSY_Msk                 (0x1UL << I2C_STR0_MASTER_BUSY_Pos)

#define I2C_STR0_SLAVE_BUSY_Pos                  (6)
#define I2C_STR0_SALVE_BUSY_Msk                  (0x1UL << I2C_STR0_SLAVE_BUSY_Pos)

#define I2C_STR0_MASTER_HOLD_BUS_Pos             (7)
#define I2C_STR0_MASTER_HOLD_BUS_Msk             (0x1UL << I2C_STR0_MASTER_HOLD_BUS_Pos)

#define I2C_STR0_SLAVE_HOLD_BUS_Pos              (9)
#define I2C_STR0_SLAVE_HOLD_BUS_Msk              (0x1UL << I2C_STR0_SLAVE_HOLD_BUS_Pos)

#define I2C_STR0_SDA_RECOV_STATUS_Pos            (11)
#define I2C_STR0_SDA_RECOV_STATUS_Msk            (0x1UL << I2C_STR0_SDA_RECOV_STATUS_Pos)
/** @} */

/** @name TXFSR - I2C_TXFSR register */
/** @{ */
#define I2C_TXFSR_VALUE_Pos                      (0)
#define I2C_TXFSR_VALUE_Msk                      (0xFFUL << I2C_TXFSR_VALUE_Pos)
/** @} */

/** @name RXFSR - I2C_RXFSR register */
/** @{ */
#define I2C_RXFSR_VALUE_Pos                      (0)
#define I2C_RXFSR_VALUE_Msk                      (0xFFUL << I2C_RXFSR_VALUE_Pos)
/** @} */

/** @name SDAHOLD - I2C_SDAHOLD register */
/** @{ */
#define I2C_SDAHOLD_TX_Pos                       (0)
#define I2C_SDAHOLD_TX_Msk                       (0xFFFFUL << I2C_SDAHOLD_TX_Pos)

#define I2C_SDAHOLD_RX_Pos                       (16)
#define I2C_SDAHOLD_RX_Msk                       (0xFFUL << I2C_SDAHOLD_RX_Pos)
/** @} */

/** @name STR1 - I2C_STR1 register */
/** @{ */
#define I2C_STR1_ADDR_NACK_Pos                   (0)
#define I2C_STR1_ADDR_NACK_Msk                   (0x7UL << I2C_STR1_ADDR_NACK_Pos)

#define I2C_STR1_TXDATA_NACK_Pos                 (3)
#define I2C_STR1_TXDATA_NACK_Msk                 (0x1UL << I2C_STR1_TXDATA_NACK_Pos)

#define I2C_STR1_GCALL_NACK_Pos                  (4)
#define I2C_STR1_GCALL_NACK_Msk                  (0x1UL << I2C_STR1_GCALL_NACK_Pos)

#define I2C_STR1_SBYTE_ACKDET_Pos                (7)
#define I2C_STR1_SBYTE_ACKDET_Msk                (0x1UL << I2C_STR1_SBYTE_ACKDET_Pos)

#define I2C_STR1_SBYTE_NORSTRT_Pos               (9)
#define I2C_STR1_SBYTE_NORSTRT_Msk               (0x1UL << I2C_STR1_SBYTE_NORSTRT_Pos)

#define I2C_STR1_10B_RD_NORSTRT_Pos              (10)
#define I2C_STR1_10B_RD_NORSTRT_Msk              (0x1UL << I2C_STR1_10B_RD_NORSTRT_Pos)

#define I2C_STR1_ARBITRATATION_LOST_Pos          (12)
#define I2C_STR1_ARBITRATATION_LOST_Msk          (0x1UL << I2C_STR1_ARBITRATATION_LOST_Pos)

#define I2C_STR1_USER_ABRT_Pos                   (16)
#define I2C_STR1_USER_ABRT_Msk                   (0x1UL << I2C_STR1_USER_ABRT_Pos)

#define I2C_STR1_SDA_STUCK_Pos                   (17)
#define I2C_STR1_SDA_STUCK_Msk                   (0x1UL << I2C_STR1_SDA_STUCK_Pos)

/** @} */

/** @name SDASETUP - I2C_SDASETUP register */
/** @{ */
#define I2C_SDASETUP_VALUE_Pos                   (0)
#define I2C_SDASETUP_VALUE_Msk                   (0xFFUL << I2C_SDASETUP_VALUE_Pos)
/** @} */

/** @name ACKGC - I2C_ACKGC register */
/** @{ */
#define I2C_ACKGC_VALUE_Pos                      (0)
#define I2C_ACKGC_VALUE_Msk                      (0x1UL << I2C_ACKGC_VALUE_Pos)
/** @} */

/** @name STR2 - I2C_STR2 register */
/** @{ */
#define I2C_STR2_ENABLE_Pos                      (0)
#define I2C_STR2_ENABLE_Msk                      (0x1UL << I2C_STR2_ENABLE_Pos)
/** @} */

/** @name SPKSR - I2C_SPKSR register */
/** @{ */
#define I2C_SPKSR_VALUE_Pos                      (0)
#define I2C_SPKSR_VALUE_Msk                      (0xFFUL << I2C_SPKSR_VALUE_Pos)
/** @} */

/** @name STUCKTIMEOUT - I2C_STUCKTIMEOUT N register */
/** @{ */
#define I2C_STUCKTIMEOUT_VALUE_Pos               (0)
#define I2C_STUCKTIMEOUT_VALUE_Msk               (0xFFFFFFFFUL << I2C_STUCKTIMEOUT_VALUE_Pos)
/** @} */

/** Peripheral I2C0 base pointer */
#define I2C0                                     ((I2C_TypeDef *)MP_I2C0_BASE_ADDR)

/** Peripheral I2C1 base pointer */
#define I2C1                                     ((I2C_TypeDef *)MP_I2C1_BASE_ADDR)

/** @brief I2C peripheral instance number */
#define I2C_INSTANCE_NUM                         (2)

/** @brief I2C peripheral instance array */
#define I2C_INSTANCE_ARRAY                       {I2C0, I2C1}
/**
  * @}
  */ /* end of group I2C */

/** @addtogroup I2S I2S
  * @{
  */

/**
  * @brief I2S register layout typedef
  *
  */
typedef struct {
    __IO uint32_t DFMT;                           /**< Data Format Register,                offset: 0x0 */
    __IO uint32_t SLOTCTL;                        /**< Slot Control Register,               offset: 0x4 */
    __IO uint32_t CLKCTL;                         /**< Clock Control Register,              offset: 0x8 */
    __IO uint32_t DMACTL;                         /**< DMA Control Register,                offset: 0xC */
    __IO uint32_t INTCTL;                         /**< Interrupt Control Register,          offset: 0x10 */
    __IO uint32_t TIMEOUTCTL;                     /**< Timeout Control Register,            offset: 0x14 */
    __IO uint32_t STAS;                           /**< Status Register,                     offset: 0x18 */
    __IO uint32_t RFIFO;                          /**< Rx Buffer Register,                  offset: 0x1c */
    __IO uint32_t TFIFO;                          /**< Tx Buffer Register,                  offset: 0x20 */
    __IO uint32_t I2SCTL;                         /**< I2S Control Register,                offset: 0x24 */
         uint32_t RESERVED_1[46];
    __IO uint32_t I2SBUSSEL;                      /**< i2s bus select                       offset: 0xe0 */
} I2S_TypeDef;

/** @name DFMT - I2S_DFMT register */
/** @{ */
#define I2S_DFMT_SLAVE_MODE_EN_Pos               (0)
#define I2S_DFMT_SLAVE_MODE_EN_Msk               (0x1UL << I2S_DFMT_SLAVE_MODE_EN_Pos)

#define I2S_DFMT_SLOT_SIZE_Pos                   (1)
#define I2S_DFMT_SLOT_SIZE_Msk                   (0x1FUL << I2S_DFMT_SLOT_SIZE_Pos)

#define I2S_DFMT_WORD_SIZE_Pos                   (6)
#define I2S_DFMT_WORD_SIZE_Msk                   (0x1FUL << I2S_DFMT_WORD_SIZE_Pos)

#define I2S_DFMT_ALIGN_MODE_Pos                  (11)
#define I2S_DFMT_ALIGN_MODE_Msk                  (0x1UL << I2S_DFMT_ALIGN_MODE_Pos)

#define I2S_DFMT_ENDIAN_MODE_Pos                 (12)
#define I2S_DFMT_ENDIAN_MODE_Msk                 (0x1UL << I2S_DFMT_ENDIAN_MODE_Pos)

#define I2S_DFMT_DATA_DLY_Pos                    (13)
#define I2S_DFMT_DATA_DLY_Msk                    (0x3UL << I2S_DFMT_DATA_DLY_Pos)

#define I2S_DFMT_TX_PAD_Pos                      (15)
#define I2S_DFMT_TX_PAD_Msk                      (0x3UL << I2S_DFMT_TX_PAD_Pos)

#define I2S_DFMT_RX_SIGN_EXT_Pos                 (17)
#define I2S_DFMT_RX_SIGN_EXT_Msk                 (0x1UL << I2S_DFMT_RX_SIGN_EXT_Pos)

#define I2S_DFMT_TX_PACK_Pos                     (18)
#define I2S_DFMT_TX_PACK_Msk                     (0x3UL << I2S_DFMT_TX_PACK_Pos)

#define I2S_DFMT_RX_PACK_Pos                     (20)
#define I2S_DFMT_RX_PACK_Msk                     (0x3UL << I2S_DFMT_RX_PACK_Pos)

#define I2S_DFMT_TX_FIFO_ENDIAN_MODE_Pos         (22)
#define I2S_DFMT_TX_FIFO_ENDIAN_MODE_Msk         (0x1UL << I2S_DFMT_TX_FIFO_ENDIAN_MODE_Pos)

#define I2S_DFMT_RX_FIFO_ENDIAN_MODE_Pos         (23)
#define I2S_DFMT_RX_FIFO_ENDIAN_MODE_Msk         (0x1UL << I2S_DFMT_RX_FIFO_ENDIAN_MODE_Pos)
/** @} */

/** @name SLOTCTL - I2S_SLOTCTL register */
/** @{ */
#define I2S_SLOTCTL_SLOT_EN_Pos                  (0)
#define I2S_SLOTCTL_SLOT_EN_Msk                  (0xFFUL << I2S_SLOTCTL_SLOT_EN_Pos)

#define I2S_SLOTCTL_SLOT_NUM_Pos                 (8)
#define I2S_SLOTCTL_SLOT_NUM_Msk                 (0x7UL << I2S_SLOTCTL_SLOT_NUM_Pos)
/** @} */

/** @name CLKCTL - I2S_CLKCTL register */
/** @{ */
#define I2S_CLKCTL_BCLK_POLARITY_Pos             (0)
#define I2S_CLKCTL_BCLK_POLARITY_Msk             (0x1UL << I2S_CLKCTL_BCLK_POLARITY_Pos)

#define I2S_CLKCTL_FS_POLARITY_Pos               (1)
#define I2S_CLKCTL_FS_POLARITY_Msk               (0x1UL << I2S_CLKCTL_FS_POLARITY_Pos)

#define I2S_CLKCTL_FS_WIDTH_Pos                  (2)
#define I2S_CLKCTL_FS_WIDTH_Msk                  (0x3FUL << I2S_CLKCTL_FS_WIDTH_Pos)
/** @} */

/** @name DMA_CTRL - I2S_DMA_CTRL register */
/** @{ */
#define I2S_DMA_CTRL_RX_DMA_REQ_EN_Pos           (0)
#define I2S_DMA_CTRL_RX_DMA_REQ_EN_Msk           (0x1UL << I2S_DMA_CTRL_RX_DMA_REQ_EN_Pos)

#define I2S_DMA_CTRL_TX_DMA_REQ_EN_Pos           (1)
#define I2S_DMA_CTRL_TX_DMA_REQ_EN_Msk           (0x1UL << I2S_DMA_CTRL_TX_DMA_REQ_EN_Pos)

#define I2S_DMA_CTRL_RX_DMA_TIMEOUT_EN_Pos       (2)
#define I2S_DMA_CTRL_RX_DMA_TIMEOUT_EN_Msk       (0x1UL << I2S_DMA_CTRL_RX_DMA_TIMEOUT_EN_Pos)

#define I2S_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Pos     (3)
#define I2S_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Msk     (0x1FUL << I2S_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Pos)

#define I2S_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Pos  (8)
#define I2S_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Msk  (0xFUL << I2S_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Pos)

#define I2S_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Pos  (12)
#define I2S_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Msk  (0xFUL << I2S_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Pos)

#define I2S_DMA_CTRL_RX_DMA_THRESHOLD_Pos        (16)
#define I2S_DMA_CTRL_RX_DMA_THRESHOLD_Msk        (0xFUL << I2S_DMA_CTRL_RX_DMA_THRESHOLD_Pos)

#define I2S_DMA_CTRL_TX_DMA_THRESHOLD_Pos        (20)
#define I2S_DMA_CTRL_TX_DMA_THRESHOLD_Msk        (0xFUL << I2S_DMA_CTRL_TX_DMA_THRESHOLD_Pos)

#define I2S_DMA_CTRL_RX_FIFO_FLUSH_Pos           (24)
#define I2S_DMA_CTRL_RX_FIFO_FLUSH_Msk           (0x1UL << I2S_DMA_CTRL_RX_FIFO_FLUSH_Pos)

#define I2S_DMA_CTRL_TX_FIFO_FLUSH_Pos           (25)
#define I2S_DMA_CTRL_TX_FIFO_FLUSH_Msk           (0x1UL << I2S_DMA_CTRL_TX_FIFO_FLUSH_Pos)
/** @} */


/** @name INT_CTRL - I2S_INT_CTRL register */
/** @{ */
#define I2S_INT_CTRL_TX_UNDERRUN_INT_EN_Pos      (0)
#define I2S_INT_CTRL_TX_UNDERRUN_INT_EN_Msk      (0x1UL << I2S_INT_CTRL_TX_UNDERRUN_INT_EN_Pos)

#define I2S_INT_CTRL_TX_DMA_ERR_INT_EN_Pos       (1)
#define I2S_INT_CTRL_TX_DMA_ERR_INT_EN_Msk       (0x1UL << I2S_INT_CTRL_TX_DMA_ERR_INT_EN_Pos)

#define I2S_INT_CTRL_TX_DAT_INT_EN_Pos           (2)
#define I2S_INT_CTRL_TX_DAT_INT_EN_Msk           (0x1UL << I2S_INT_CTRL_TX_DAT_INT_EN_Pos)

#define I2S_INT_CTRL_RX_OVERFLOW_INT_EN_Pos      (3)
#define I2S_INT_CTRL_RX_OVERFLOW_INT_EN_Msk      (0x1UL << I2S_INT_CTRL_RX_OVERFLOW_INT_EN_Pos)

#define I2S_INT_CTRL_RX_DMA_ERR_INT_EN_Pos       (4)
#define I2S_INT_CTRL_RX_DMA_ERR_INT_EN_Msk       (0x1UL << I2S_INT_CTRL_RX_DMA_ERR_INT_EN_Pos)

#define I2S_INT_CTRL_RX_DAT_INT_EN_Pos           (5)
#define I2S_INT_CTRL_RX_DAT_INT_EN_Msk           (0x1UL << I2S_INT_CTRL_RX_DAT_INT_EN_Pos)

#define I2S_INT_CTRL_RX_TIMEOUT_INT_EN_Pos       (6)
#define I2S_INT_CTRL_RX_TIMEOUT_INT_EN_Msk       (0x1UL << I2S_INT_CTRL_RX_TIMEOUT_INT_EN_Pos)

#define I2S_INT_CTRL_FS_ERR_INT_EN_Pos           (7)
#define I2S_INT_CTRL_FS_ERR_INT_EN_Msk           (0x1UL << I2S_INT_CTRL_FS_ERR_INT_EN_Pos)

#define I2S_INT_CTRL_FRAME_START_INT_EN_Pos      (8)
#define I2S_INT_CTRL_FRAME_START_INT_EN_Msk      (0x1UL << I2S_INT_CTRL_FRAME_START_INT_EN_Pos)

#define I2S_INT_CTRL_FRAME_END_INT_EN_Pos        (9)
#define I2S_INT_CTRL_FRAME_END_INT_EN_Msk        (0x1UL << I2S_INT_CTRL_FRAME_END_INT_EN_Pos)

#define I2S_INT_CTRL_CSPI_BUS_TIMEOUT_INT_EN_Pos (10)
#define I2S_INT_CTRL_CSPI_BUS_TIMEOUT_INT_EN_Msk (0x1UL << I2S_INT_CTRL_CSPI_BUS_TIMEOUT_INT_EN_Pos)

#define I2S_INT_CTRL_RSVD1_Pos                   (11)
#define I2S_INT_CTRL_RSVD1_Msk                   (0x1UL << I2S_INT_CTRL_RSVD1_Pos)

#define I2S_INT_CTRL_RSVD2_Pos                   (12)
#define I2S_INT_CTRL_RSVD2_Msk                   (0x1UL << I2S_INT_CTRL_RSVD2_Pos)

#define I2S_INT_CTRL_RSVD3_Pos                   (13)
#define I2S_INT_CTRL_RSVD3_Msk                   (0x1UL << I2S_INT_CTRL_RSVD3_Pos)

#define I2S_INT_CTRL_RSVD4_Pos                   (14)
#define I2S_INT_CTRL_RSVD4_Msk                   (0x1UL << I2S_INT_CTRL_RSVD4_Pos)

#define I2S_INT_CTRL_RSVD5_Pos                   (15)
#define I2S_INT_CTRL_RSVD5_Msk                   (0x1UL << I2S_INT_CTRL_RSVD5_Pos)

#define I2S_INT_CTRL_TX_INT_THREASHOLD_Pos       (16)
#define I2S_INT_CTRL_TX_INT_THREASHOLD_Msk       (0xFUL << I2S_INT_CTRL_TX_INT_THREASHOLD_Pos)

#define I2S_INT_CTRL_RX_INT_THREASHOLD_Pos       (20)
#define I2S_INT_CTRL_RX_INT_THREASHOLD_Msk       (0xFUL << I2S_INT_CTRL_RX_INT_THREASHOLD_Pos)
/** @} */

/** @name TIMEOUT_CYCLE - I2S_TIMEOUT_CYCLE register */
/** @{ */
#define I2S_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Pos   (0)
#define I2S_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Msk   (0xFFFFFFUL << I2S_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Pos)
/** @} */



/** @name STATS_CTRL - I2S_STATS_CTRL register */
/** @{ */
#define I2S_STATS_CTRL_TX_UNDERRUN_RUN_Pos       (0)
#define I2S_STATS_CTRL_TX_UNDERRUN_RUN_Msk       (0x1UL << I2S_STATS_CTRL_TX_UNDERRUN_RUN_Pos)

#define I2S_STATS_CTRL_TX_DMA_ERR_Pos            (1)
#define I2S_STATS_CTRL_TX_DMA_ERR_Msk            (0x1UL << I2S_STATS_CTRL_TX_DMA_ERR_Pos)

#define I2S_STATS_CTRL_TX_DAT_RDY_Pos            (2)
#define I2S_STATS_CTRL_TX_DAT_RDY_Msk            (0x1UL << I2S_STATS_CTRL_TX_DAT_RDY_Pos)

#define I2S_STATS_CTRL_RX_OVERFLOW_Pos           (3)
#define I2S_STATS_CTRL_RX_OVERFLOW_Msk           (0x1UL << I2S_STATS_CTRL_RX_OVERFLOW_Pos)

#define I2S_STATS_CTRL_RX_DMA_ERR_Pos            (4)
#define I2S_STATS_CTRL_RX_DMA_ERR_Msk            (0x1UL << I2S_STATS_CTRL_RX_DMA_ERR_Pos)

#define I2S_STATS_CTRL_RX_DAT_RDY_Pos            (5)
#define I2S_STATS_CTRL_RX_DAT_RDY_Msk            (0x1UL << I2S_STATS_CTRL_RX_DAT_RDY_Pos)

#define I2S_STATS_CTRL_RX_FIFO_TIMEOUT_Pos       (6)
#define I2S_STATS_CTRL_RX_FIFO_TIMEOUT_Msk       (0x1UL << I2S_STATS_CTRL_RX_FIFO_TIMEOUT_Pos)

#define I2S_STATS_CTRL_FS_ERR_Pos                (7)
#define I2S_STATS_CTRL_FS_ERR_Msk                (0xFUL << I2S_STATS_CTRL_FS_ERR_Pos)

#define I2S_STATS_CTRL_FRAME_START_Pos           (11)
#define I2S_STATS_CTRL_FRAME_START_Msk           (0x1UL << I2S_STATS_CTRL_FRAME_START_Pos)

#define I2S_STATS_CTRL_FRAME_END_Pos             (12)
#define I2S_STATS_CTRL_FRAME_END_Msk             (0x1UL << I2S_STATS_CTRL_FRAME_END_Pos)

#define I2S_STATS_CTRL_TX_FIFO_LEVEL_Pos         (13)
#define I2S_STATS_CTRL_TX_FIFO_LEVEL_Msk         (0x3FUL << I2S_STATS_CTRL_TX_FIFO_LEVEL_Pos)

#define I2S_STATS_CTRL_RX_FIFO_LEVEL_Pos         (19)
#define I2S_STATS_CTRL_RX_FIFO_LEVEL_Msk         (0x3FUL << I2S_STATS_CTRL_RX_FIFO_LEVEL_Pos)

#define I2S_STATS_CTRL_CSPI_BUS_TIMEOUT_Pos      (25)
#define I2S_STATS_CTRL_CSPI_BUS_TIMEOUT_Msk      (0x1UL << I2S_STATS_CTRL_CSPI_BUS_TIMEOUT_Pos)
/** @} */


/** @name RFIFO - I2S_RFIFO register */
/** @{ */
#define I2S_RFIFO_DATA_Pos                       (0)
#define I2S_RFIFO_DATA_Msk                       (0xFFFFFFFFUL << I2S_RFIFO_DATA_Pos)
/** @} */

/** @name TFIFO - I2S_TFIFO register */
/** @{ */
#define I2S_TFIFO_DAT_Pos                        (0)
#define I2S_TFIFO_DAT_Msk                        (0xFFFFFFFFUL << I2S_TFIFO_DAT_Pos)
/** @} */

/** @name I2SCTL - I2S_CTL register */
/** @{ */
#define I2S_CTL_MODE_Pos                         (0)
#define I2S_CTL_MODE_Msk                         (0x3UL << I2S_CTL_MODE_Pos)
/** @} */

/** Peripheral I2S0 base pointer */
#define I2S0                                     ((I2S_TypeDef *)MP_USP0_BASE_ADDR)

/** Peripheral I2S1 base pointer */
#define I2S1                                     ((I2S_TypeDef *)MP_USP1_BASE_ADDR)

/** @brief SPI peripheral instance number */
#define I2S_INSTANCE_NUM                         (2)


/**
  * @}
  */ /* end of group I2S */

  /** @addtogroup CSPI CSPI
  * @{
  */

/**
  * @brief CSPI register layout typedef
  *
  */
typedef struct {
    __IO uint32_t DFMT;                           /**< Data Format Register,                offset: 0x0 */
    __IO uint32_t SLOTCTL;                        /**< Slot Control Register,               offset: 0x4 */
    __IO uint32_t CLKCTL;                         /**< Clock Control Register,              offset: 0x8 */
    __IO uint32_t DMACTL;                         /**< DMA Control Register,                offset: 0xC */
    __IO uint32_t INTCTL;                         /**< Interrupt Control Register,          offset: 0x10 */
    __IO uint32_t TIMEOUTCTL;                     /**< Timeout Control Register,            offset: 0x14 */
    __IO uint32_t STAS;                           /**< Status Register,                     offset: 0x18 */
    __IO uint32_t RFIFO;                          /**< Rx Buffer Register,                  offset: 0x1c */
    __IO uint32_t TFIFO;                          /**< Tx Buffer Register,                  offset: 0x20 */
    __IO uint32_t RSVD;                           /**< Reserved,                            offset: 0x24 */
    __IO uint32_t CSPICTL;                        /**< Camera SPI Control Register,         offset: 0x28 */
    __IO uint32_t CCTL;                           /**< Auto Cg Control Register,            offset: 0x2c */
    __IO uint32_t CSPIINFO0;                      /**< Cspi Frame info0 Register,           offset: 0x30 */
    __IO uint32_t CSPIINFO1;                      /**< Cspi Frame info1 Register,           offset: 0x34 */
    __IO uint32_t CSPIDBG;                        /**< Cspi Debug Register,                 offset: 0x38 */
    __IO uint32_t CSPINIT;                        /**< Cspi Init Register,                  offset: 0x3c */
    __IO uint32_t CLSP;                           /**< Cspi Line Start Register,            offset: 0x40 */
    __IO uint32_t CDATP;                          /**< Cspi Data Packet Register,           offset: 0x44 */
    __IO uint32_t CLINFO;                         /**< Cspi Line Info Register,             offset: 0x48 */
    __IO uint32_t CBCTRL;                         /**< Cspi binary ctrl                     offset: 0x4c */
    __IO uint32_t CSPIPROCLSPI;                   /**< Cspi frame proc lspi                 offset: 0x50 */
    __IO uint32_t CSPIQUARTILE;                   /**< Cspi OTSU quartile                   offset: 0x54 */
    __IO uint32_t CSPIYADJ;                       /**< Cspi y Adjustment                    offset: 0x58 */
    __IO uint32_t RSVD2;
    __IO uint32_t RSVD3[18];                      /**< For lspi                                          */
    __IO uint32_t RSVD4[10];                      /**< Reserved                                          */
    __IO uint32_t CSPIDLYCTRL;                    /**< Cspi delay ctrl                      offset: 0xD0 */
    __IO uint32_t RSVD6[3];                       /**< Reserved                                          */
    __IO uint32_t I2SBUSSEL;                      /**< Cspi bus select                      offset: 0xe0 */
    __IO uint32_t RSVD5[3];                       /**< Reserved                                          */
    __IO uint32_t HISTOBUFCTRL;                   /**< Histogram buf ctrl                   offset: 0xf0 */
} CSPI_TypeDef;

/** @name DFMT - CSPI_DFMT register */
/** @{ */
#define CSPI_DFMT_SLAVE_MODE_EN_Pos               (0)
#define CSPI_DFMT_SLAVE_MODE_EN_Msk               (0x1UL << CSPI_DFMT_SLAVE_MODE_EN_Pos)

#define CSPI_DFMT_SLOT_SIZE_Pos                   (1)
#define CSPI_DFMT_SLOT_SIZE_Msk                   (0x1FUL << CSPI_DFMT_SLOT_SIZE_Pos)

#define CSPI_DFMT_WORD_SIZE_Pos                   (6)
#define CSPI_DFMT_WORD_SIZE_Msk                   (0x1FUL << CSPI_DFMT_WORD_SIZE_Pos)

#define CSPI_DFMT_ALIGN_MODE_Pos                  (11)
#define CSPI_DFMT_ALIGN_MODE_Msk                  (0x1UL << CSPI_DFMT_ALIGN_MODE_Pos)

#define CSPI_DFMT_ENDIAN_MODE_Pos                 (12)
#define CSPI_DFMT_ENDIAN_MODE_Msk                 (0x1UL << CSPI_DFMT_ENDIAN_MODE_Pos)

#define CSPI_DFMT_DATA_DLY_Pos                    (13)
#define CSPI_DFMT_DATA_DLY_Msk                    (0x3UL << CSPI_DFMT_DATA_DLY_Pos)

#define CSPI_DFMT_TX_PAD_Pos                      (15)
#define CSPI_DFMT_TX_PAD_Msk                      (0x3UL << CSPI_DFMT_TX_PAD_Pos)

#define CSPI_DFMT_RX_SIGN_EXT_Pos                 (17)
#define CSPI_DFMT_RX_SIGN_EXT_Msk                 (0x1UL << CSPI_DFMT_RX_SIGN_EXT_Pos)

#define CSPI_DFMT_TX_PACK_Pos                     (18)
#define CSPI_DFMT_TX_PACK_Msk                     (0x3UL << CSPI_DFMT_TX_PACK_Pos)

#define CSPI_DFMT_RX_PACK_Pos                     (20)
#define CSPI_DFMT_RX_PACK_Msk                     (0x3UL << CSPI_DFMT_RX_PACK_Pos)

#define CSPI_DFMT_TX_FIFO_ENDIAN_MODE_Pos         (22)
#define CSPI_DFMT_TX_FIFO_ENDIAN_MODE_Msk         (0x1UL << CSPI_DFMT_TX_FIFO_ENDIAN_MODE_Pos)

#define CSPI_DFMT_RX_FIFO_ENDIAN_MODE_Pos         (23)
#define CSPI_DFMT_RX_FIFO_ENDIAN_MODE_Msk         (0x1UL << CSPI_DFMT_RX_FIFO_ENDIAN_MODE_Pos)

#define CSPI_DFMT_EOR_MODE_Pos                    (24)
#define CSPI_DFMT_EOR_MODE_Msk                    (0x1UL << CSPI_DFMT_EOR_MODE_Pos)

/** @} */

/** @name SLOTCTL - CSPI_SLOTCTL register */
/** @{ */
#define CSPI_SLOTCTL_SLOT_EN_Pos                  (0)
#define CSPI_SLOTCTL_SLOT_EN_Msk                  (0xFFUL << CSPI_SLOTCTL_SLOT_EN_Pos)

#define CSPI_SLOTCTL_SLOT_NUM_Pos                 (8)
#define CSPI_SLOTCTL_SLOT_NUM_Msk                 (0x7UL << CSPI_SLOTCTL_SLOT_NUM_Pos)
/** @} */

/** @name CLKCTL - CSPI_CLKCTL register */
/** @{ */
#define CSPI_CLKCTL_BCLK_POLARITY_Pos             (0)
#define CSPI_CLKCTL_BCLK_POLARITY_Msk             (0x1UL << CSPI_CLKCTL_BCLK_POLARITY_Pos)

#define CSPI_CLKCTL_FS_POLARITY_Pos               (1)
#define CSPI_CLKCTL_FS_POLARITY_Msk               (0x1UL << CSPI_CLKCTL_FS_POLARITY_Pos)

#define CSPI_CLKCTL_FS_WIDTH_Pos                  (2)
#define CSPI_CLKCTL_FS_WIDTH_Msk                  (0x3FUL << CSPI_CLKCTL_FS_WIDTH_Pos)
/** @} */

/** @name DMA_CTRL - CSPI_DMA_CTRL register */
/** @{ */
#define CSPI_DMA_CTRL_RX_DMA_REQ_EN_Pos           (0)
#define CSPI_DMA_CTRL_RX_DMA_REQ_EN_Msk           (0x1UL << CSPI_DMA_CTRL_RX_DMA_REQ_EN_Pos)

#define CSPI_DMA_CTRL_TX_DMA_REQ_EN_Pos           (1)
#define CSPI_DMA_CTRL_TX_DMA_REQ_EN_Msk           (0x1UL << CSPI_DMA_CTRL_TX_DMA_REQ_EN_Pos)

#define CSPI_DMA_CTRL_RX_DMA_TIMEOUT_EN_Pos       (2)
#define CSPI_DMA_CTRL_RX_DMA_TIMEOUT_EN_Msk       (0x1UL << CSPI_DMA_CTRL_RX_DMA_TIMEOUT_EN_Pos)

#define CSPI_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Pos     (3)
#define CSPI_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Msk     (0x1FUL << CSPI_DMA_CTRL_DMA_WORK_WAIT_CYCLE_Pos)

#define CSPI_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Pos  (8)
#define CSPI_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Msk  (0xFUL << CSPI_DMA_CTRL_RX_DMA_BURST_SIZE_SUB1_Pos)

#define CSPI_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Pos  (12)
#define CSPI_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Msk  (0xFUL << CSPI_DMA_CTRL_TX_DMA_BURST_SIZE_SUB1_Pos)

#define CSPI_DMA_CTRL_RX_DMA_THRESHOLD_Pos        (16)
#define CSPI_DMA_CTRL_RX_DMA_THRESHOLD_Msk        (0xFUL << CSPI_DMA_CTRL_RX_DMA_THRESHOLD_Pos)

#define CSPI_DMA_CTRL_TX_DMA_THRESHOLD_Pos        (20)
#define CSPI_DMA_CTRL_TX_DMA_THRESHOLD_Msk        (0xFUL << CSPI_DMA_CTRL_TX_DMA_THRESHOLD_Pos)

#define CSPI_DMA_CTRL_RX_FIFO_FLUSH_Pos           (24)
#define CSPI_DMA_CTRL_RX_FIFO_FLUSH_Msk           (0x1UL << CSPI_DMA_CTRL_RX_FIFO_FLUSH_Pos)

#define CSPI_DMA_CTRL_TX_FIFO_FLUSH_Pos           (25)
#define CSPI_DMA_CTRL_TX_FIFO_FLUSH_Msk           (0x1UL << CSPI_DMA_CTRL_TX_FIFO_FLUSH_Pos)
/** @} */


/** @name INT_CTRL - CSPI_INT_CTRL register */
/** @{ */
#define CSPI_INT_CTRL_TX_UNDERRUN_INT_EN_Pos      (0)
#define CSPI_INT_CTRL_TX_UNDERRUN_INT_EN_Msk      (0x1UL << CSPI_INT_CTRL_TX_UNDERRUN_INT_EN_Pos)

#define CSPI_INT_CTRL_TX_DMA_ERR_INT_EN_Pos       (1)
#define CSPI_INT_CTRL_TX_DMA_ERR_INT_EN_Msk       (0x1UL << CSPI_INT_CTRL_TX_DMA_ERR_INT_EN_Pos)

#define CSPI_INT_CTRL_TX_DAT_INT_EN_Pos           (2)
#define CSPI_INT_CTRL_TX_DAT_INT_EN_Msk           (0x1UL << CSPI_INT_CTRL_TX_DAT_INT_EN_Pos)

#define CSPI_INT_CTRL_RX_OVERFLOW_INT_EN_Pos      (3)
#define CSPI_INT_CTRL_RX_OVERFLOW_INT_EN_Msk      (0x1UL << CSPI_INT_CTRL_RX_OVERFLOW_INT_EN_Pos)

#define CSPI_INT_CTRL_RX_DMA_ERR_INT_EN_Pos       (4)
#define CSPI_INT_CTRL_RX_DMA_ERR_INT_EN_Msk       (0x1UL << CSPI_INT_CTRL_RX_DMA_ERR_INT_EN_Pos)

#define CSPI_INT_CTRL_RX_DAT_INT_EN_Pos           (5)
#define CSPI_INT_CTRL_RX_DAT_INT_EN_Msk           (0x1UL << CSPI_INT_CTRL_RX_DAT_INT_EN_Pos)

#define CSPI_INT_CTRL_RX_TIMEOUT_INT_EN_Pos       (6)
#define CSPI_INT_CTRL_RX_TIMEOUT_INT_EN_Msk       (0x1UL << CSPI_INT_CTRL_RX_TIMEOUT_INT_EN_Pos)

#define CSPI_INT_CTRL_FS_ERR_INT_EN_Pos           (7)
#define CSPI_INT_CTRL_FS_ERR_INT_EN_Msk           (0x1UL << CSPI_INT_CTRL_FS_ERR_INT_EN_Pos)

#define CSPI_INT_CTRL_FRAME_START_INT_EN_Pos      (8)
#define CSPI_INT_CTRL_FRAME_START_INT_EN_Msk      (0x1UL << CSPI_INT_CTRL_FRAME_START_INT_EN_Pos)

#define CSPI_INT_CTRL_FRAME_END_INT_EN_Pos        (9)
#define CSPI_INT_CTRL_FRAME_END_INT_EN_Msk        (0x1UL << CSPI_INT_CTRL_FRAME_END_INT_EN_Pos)

#define CSPI_INT_CTRL_BUS_TIMEOUT_INT_EN_Pos      (10)
#define CSPI_INT_CTRL_BUS_TIMEOUT_INT_EN_Msk      (0x1UL << CSPI_INT_CTRL_BUS_TIMEOUT_INT_EN_Pos)

#define CSPI_INT_CTRL_OTSU_END_EN_Pos             (15)
#define CSPI_INT_CTRL_OTSU_END_EN_Msk             (0x1UL << CSPI_INT_CTRL_OTSU_END_EN_Pos)

#define CSPI_INT_CTRL_TX_INT_THREASHOLD_Pos       (17)
#define CSPI_INT_CTRL_TX_INT_THREASHOLD_Msk       (0xFUL << CSPI_INT_CTRL_TX_INT_THREASHOLD_Pos)

#define CSPI_INT_CTRL_RX_INT_THREASHOLD_Pos       (21)
#define CSPI_INT_CTRL_RX_INT_THREASHOLD_Msk       (0xFUL << CSPI_INT_CTRL_RX_INT_THREASHOLD_Pos)
/** @} */

/** @name TIMEOUT_CYCLE - CSPI_TIMEOUT_CYCLE register */
/** @{ */
#define CSPI_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Pos   (0)
#define CSPI_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Msk   (0xFFFFFFUL << CSPI_TIMEOUT_CYCLE_RX_TIMEOUT_CYCLE_Pos)
/** @} */

/** @name STATS_CTRL -CSPI_STATS_CTRL register */
/** @{ */
#define CSPI_STATS_CTRL_TX_UNDERRUN_RUN_Pos       (0)
#define CSPI_STATS_CTRL_TX_UNDERRUN_RUN_Msk       (0x1UL << CSPI_STATS_CTRL_TX_UNDERRUN_RUN_Pos)

#define CSPI_STATS_CTRL_TX_DMA_ERR_Pos            (1)
#define CSPI_STATS_CTRL_TX_DMA_ERR_Msk            (0x1UL << CSPI_STATS_CTRL_TX_DMA_ERR_Pos)

#define CSPI_STATS_CTRL_TX_DAT_RDY_Pos            (2)
#define CSPI_STATS_CTRL_TX_DAT_RDY_Msk            (0x1UL << CSPI_STATS_CTRL_TX_DAT_RDY_Pos)

#define CSPI_STATS_CTRL_RX_OVERFLOW_Pos           (3)
#define CSPI_STATS_CTRL_RX_OVERFLOW_Msk           (0x1UL << CSPI_STATS_CTRL_RX_OVERFLOW_Pos)

#define CSPI_STATS_CTRL_RX_DMA_ERR_Pos            (4)
#define CSPI_STATS_CTRL_RX_DMA_ERR_Msk            (0x1UL << CSPI_STATS_CTRL_RX_DMA_ERR_Pos)

#define CSPI_STATS_CTRL_RX_DAT_RDY_Pos            (5)
#define CSPI_STATS_CTRL_RX_DAT_RDY_Msk            (0x1UL << CSPI_STATS_CTRL_RX_DAT_RDY_Pos)

#define CSPI_STATS_CTRL_RX_FIFO_TIMEOUT_Pos       (6)
#define CSPI_STATS_CTRL_RX_FIFO_TIMEOUT_Msk       (0x1UL << CSPI_STATS_CTRL_RX_FIFO_TIMEOUT_Pos)

#define CSPI_STATS_CTRL_FS_ERR_Pos                (7)
#define CSPI_STATS_CTRL_FS_ERR_Msk                (0xFUL << CSPI_STATS_CTRL_FS_ERR_Pos)

#define CSPI_STATS_CTRL_FRAME_START_Pos           (11)
#define CSPI_STATS_CTRL_FRAME_START_Msk           (0x1UL << CSPI_STATS_CTRL_FRAME_START_Pos)

#define CSPI_STATS_CTRL_FRAME_END_Pos             (12)
#define CSPI_STATS_CTRL_FRAME_END_Msk             (0x1UL << CSPI_STATS_CTRL_FRAME_END_Pos)

#define CSPI_STATS_CTRL_TX_FIFO_LEVEL_Pos         (13)
#define CSPI_STATS_CTRL_TX_FIFO_LEVEL_Msk         (0x3FUL << CSPI_STATS_CTRL_TX_FIFO_LEVEL_Pos)

#define CSPI_STATS_CTRL_RX_FIFO_LEVEL_Pos         (19)
#define CSPI_STATS_CTRL_RX_FIFO_LEVEL_Msk         (0x3FUL << CSPI_STATS_CTRL_RX_FIFO_LEVEL_Pos)

#define CSPI_STATS_CTRL_CSPI_BUS_TIMEOUT_Pos      (25)
#define CSPI_STATS_CTRL_CSPI_BUS_TIMEOUT_Msk      (0x1UL << CSPI_STATS_CTRL_CSPI_BUS_TIMEOUT_Pos)

#define CSPI_STATS_CTRL_OTSU_END_Pos              (29)
#define CSPI_STATS_CTRL_OTSU_END_Msk              (0x1UL << CSPI_STATS_CTRL_OTSU_END_Pos)

/** @} */


/** @name RFIFO - CSPI_RFIFO register */
/** @{ */
#define CSPI_RFIFO_DATA_Pos                       (0)
#define CSPI_RFIFO_DATA_Msk                       (0xFFFFFFFFUL << CSPI_RFIFO_DATA_Pos)
/** @} */

/** @name TFIFO - CSPI_TFIFO register */
/** @{ */
#define CSPI_TFIFO_DAT_Pos                        (0)
#define CSPI_TFIFO_DAT_Msk                        (0xFFFFFFFFUL << CSPI_TFIFO_DAT_Pos)
/** @} */

/** @name SPI_CTRL - CAMERA_SPI_CTRL register */
/** @{ */
#define CSPI_ENABLE_Pos                           (0)
#define CSPI_ENABLE_Msk                           (0x1UL << CSPI_ENABLE_Pos)

#define CSPI_CS_EN_Pos                            (1)
#define CSPI_CS_EN_Msk                            (0x1UL << CSPI_CS_EN_Pos)

#define CSPI_RXD_WID_Pos                          (2)
#define CSPI_RXD_WID_Msk                          (0x1UL << CSPI_RXD_WID_Pos)

#define CSPI_RXD_SEQ_Pos                          (3)
#define CSPI_RXD_SEQ_Msk                          (0x1UL << CSPI_RXD_SEQ_Pos)

#define CSPI_CPOL_Pos                             (4)
#define CSPI_CPOL_Msk                             (0x1UL << CSPI_CPOL_Pos)

#define CSPI_CPHA_Pos                             (5)
#define CSPI_CPHA_Msk                             (0x1UL << CSPI_CPHA_Pos)

#define CSPI_FRAME_PROC_EN_Pos                    (6)
#define CSPI_FRAME_PROC_EN_Msk                    (0x1UL << CSPI_FRAME_PROC_EN_Pos)

#define CSPI_FILL_Y_ONLY_Pos                      (7)
#define CSPI_FILL_Y_ONLY_Msk                      (0x1UL << CSPI_FILL_Y_ONLY_Pos)

#define CSPI_HW_INIT_EN_Pos                       (8)
#define CSPI_HW_INIT_EN_Msk                       (0x1UL << CSPI_HW_INIT_EN_Pos)

#define CSPI_LS_CHECK_EN_Pos                      (9)
#define CSPI_LS_CHECK_EN_Msk                      (0x1UL << CSPI_LS_CHECK_EN_Pos)

#define CSPI_DP_CHECK_EN_Pos                      (10)
#define CSPI_DP_CHECK_EN_Msk                      (0x1UL << CSPI_DP_CHECK_EN_Pos)

#define CSPI_FRAME_PROC_INIT_EN_Pos               (11)
#define CSPI_FRAME_PROC_INIT_EN_Msk               (0x1UL << CSPI_FRAME_PROC_INIT_EN_Pos)

#define CSPI_ROW_SCALE_RATIO_Pos                  (12)
#define CSPI_ROW_SCALE_RATIO_Msk                  (0xFUL << CSPI_ROW_SCALE_RATIO_Pos)

#define CSPI_COL_SCALE_RATIO_Pos                  (16)
#define CSPI_COL_SCALE_RATIO_Msk                  (0xFUL << CSPI_COL_SCALE_RATIO_Pos)

#define CSPI_SCALE_BYTES_Pos                      (20)
#define CSPI_SCALE_BYTES_Msk                      (0x3UL << CSPI_SCALE_BYTES_Pos)

#define CSPI_OTSU_DUMMY_LINE_CNT_Pos              (22)
#define CSPI_OTSU_DUMMY_LINE_CNT_Msk              (0x3FUL << CSPI_OTSU_DUMMY_LINE_CNT_Pos)

#define CSPI_OTSU_EN_Pos                          (28)
#define CSPI_OTSU_EN_Msk                          (0x1UL << CSPI_OTSU_EN_Pos)

#define CSPI_OTSU_EN_CLR_EN_Pos                   (29)
#define CSPI_OTSU_EN_CLR_EN_Msk                   (0x1UL << CSPI_OTSU_EN_CLR_EN_Pos)

#define CSPI_OTSU_CAL_Y_ADJ_EN_Pos                (30)
#define CSPI_OTSU_CAL_Y_ADJ_EN_Msk                (0x1UL << CSPI_OTSU_CAL_Y_ADJ_EN_Pos)

#define CSPI_OTSU_DDR_MODE_Pos                    (31)
#define CSPI_OTSU_DDR_MODE_Msk                    (0x1UL << CSPI_OTSU_DDR_MODE_Pos)


/** @} */

/** @name CG_CTRL - AUTO_CG_CTRL register */
/** @{ */
#define AUTO_CG_CTRL_AUTOCG_EN_Pos                (0)
#define AUTO_CG_CTRL_AUTOCG_EN_Msk                (0x1UL << AUTO_CG_CTRL_AUTOCG_EN_Pos)
/** @} */

/** @name FRAME_INFO0 - CSPI_FRAME_INFO0 register */
/** @{ */
#define CSPI_FRAME_INFO0_BUS_TIMEOUT_CYCLE_Pos    (0)
#define CSPI_FRAME_INFO0_BUS_TIMEOUT_CYCLE_Msk    (0xFFFFFFUL << CSPI_FRAME_INFO0_BUS_TIMEOUT_CYCLE_Pos)

#define CSPI_FRAME_INFO0_DATA_ID_Pos              (24)
#define CSPI_FRAME_INFO0_DATA_ID_Msk              (0xFFUL << CSPI_FRAME_INFO0_DATA_ID_Pos)
/** @} */

/** @name FRAME_INFO1 - CSPI_FRAME_INFO1 register */
/** @{ */
#define CSPI_FRAME_INFO1_IMAGE_HEIGHT_Pos         (0)
#define CSPI_FRAME_INFO1_IMAGE_HEIGHT_Msk         (0xFFFFUL << CSPI_FRAME_INFO1_IMAGE_HEIGHT_Pos)

#define CSPI_FRAME_INFO1_IMAGE_WIDTH_Pos          (16)
#define CSPI_FRAME_INFO1_IMAGE_WIDTH_Msk          (0xFFFFUL << CSPI_FRAME_INFO1_IMAGE_WIDTH_Pos)
/** @} */

/** @name DEBUG_INFO - CSPI_DEBUG_INFO register */
/** @{ */
#define CSPI_DEBUG_INFO_CSPI_DEBUG_LINE_CNT_Pos   (0)
#define CSPI_DEBUG_INFO_CSPI_DEBUG_LINE_CNT_Msk   (0xFFFFUL << CSPI_DEBUG_INFO_CSPI_DEBUG_LINE_CNT_Pos)

#define CSPI_DEBUG_INFO_CSPI_CS_Pos               (16)
#define CSPI_DEBUG_INFO_CSPI_CS_Msk               (0x7UL << CSPI_DEBUG_INFO_CSPI_CS_Pos)
/** @} */

/** @name INIT - CSPI_INIT register */
/** @{ */
#define CSPI_INIT_CSPI_INIT_Pos                   (0)
#define CSPI_INIT_CSPI_INIT_Msk                   (0x1UL << CSPI_INIT_CSPI_INIT_Pos)
/** @} */

/** @name LS_PACKET - CSPI_LS_PACKET register */
/** @{ */
#define CSPI_LS_PACKET_LS_PACKET_Pos              (0)
#define CSPI_LS_PACKET_LS_PACKET_Msk              (0xFFFFFFFFUL << CSPI_LS_PACKET_LS_PACKET_Pos)
/** @} */

/** @name DAT_PACKET - CSPI_DAT_PACKET register */
/** @{ */
#define CSPI_DAT_PACKET_DAT_PACKET_Pos            (0)
#define CSPI_DAT_PACKET_DAT_PACKET_Msk            (0xFFFFFFFFUL << CSPI_DAT_PACKET_DAT_PACKET_Pos)
/** @} */

/** @name LINE_INFO - CSPI_LINE_INFO register */
/** @{ */
#define CSPI_LINE_INFO_DP_SIZE_Pos                (0)
#define CSPI_LINE_INFO_DP_SIZE_Msk                (0xFFFFUL << CSPI_LINE_INFO_DP_SIZE_Pos)

#define CSPI_LINE_INFO_LINE_ID_Pos                (16)
#define CSPI_LINE_INFO_LINE_ID_Msk                (0xFFFFUL << CSPI_LINE_INFO_LINE_ID_Pos)
/** @} */

/** @name BINARY_CTRL - CSPI_BINARY_CTRL register */
/** @{ */
#define CSPI_BINARY_THRESHOLD_INUSE_Pos           (0)
#define CSPI_BINARY_THRESHOLD_INUSE_Msk           (0xFFUL << CSPI_BINARY_THRESHOLD_INUSE_Pos)

#define CSPI_OTSU_THRESHOLD_Pos                   (8)
#define CSPI_OTSU_THRESHOLD_Msk                   (0xFFUL << CSPI_OTSU_THRESHOLD_Pos)

#define CSPI_BINARY_THRESHOLD_Pos                 (16)
#define CSPI_BINARY_THRESHOLD_Msk                 (0xFFUL << CSPI_BINARY_THRESHOLD_Pos)

#define CSPI_BINARY_THRESHOLD_SEL_Pos             (24)
#define CSPI_BINARY_THRESHOLD_SEL_Msk             (0x1UL << CSPI_BINARY_THRESHOLD_SEL_Pos)

#define CSPI_OUT_SW_CTRL_Pos                      (25)
#define CSPI_OUT_SW_CTRL_Msk                      (0x3UL << CSPI_OUT_SW_CTRL_Pos)

#define CSPI_OUT_SW_CLR_EN_Pos                    (27)
#define CSPI_OUT_SW_CLR_EN_Msk                    (0x1UL << CSPI_OUT_SW_CLR_EN_Pos)

/** @} */

/** @name BUS_SEL - I2S_BUS_SEL register */
/** @{ */
#define I2S_BUS_EN_Pos                            (0)
#define I2S_BUS_EN_Msk                            (0x1UL << I2S_BUS_EN_Pos)

#define CSPI_BUS_EN_Pos                           (1)
#define CSPI_BUS_EN_Msk                           (0x1UL << CSPI_BUS_EN_Pos)

#define LSPI_BUS_EN_Pos                           (2)
#define LSPI_BUS_EN_Msk                           (0x1UL << LSPI_BUS_EN_Pos)
/** @} */

/** @name HISTOBUFCTRL - HISTOGRAM_BUF_CTRL register */
/** @{ */
#define HISTOGRAM_BUF_SWMODE_Pos                  (0)
#define HISTOGRAM_BUF_SWMODE_Msk                  (0x1UL << HISTOGRAM_BUF_SWMODE_Pos)

/** @} */


/** Peripheral CSPI0 base pointer */
#define CSPI0                                     ((CSPI_TypeDef *)MP_USP0_BASE_ADDR)

/** Peripheral CSPI1 base pointer */
#define CSPI1                                     ((CSPI_TypeDef *)MP_USP1_BASE_ADDR)

/** Peripheral CSPI2 base pointer */
#define CSPI2                                     ((CSPI_TypeDef *)MP_USP2_BASE_ADDR)

/** @brief SPI peripheral instance number */
#define CSPI_INSTANCE_NUM                         (3)


/**
  * @}
  */ /* end of group CSPI */

/** @addtogroup LSPI LSPI
* @{
*/


/**
  * @brief LSPI register layout typedef
  *
  */
typedef struct {
    __IO uint32_t DFMT;
    __IO uint32_t RSVD1[2];
    __IO uint32_t DMACTL;
    __IO uint32_t INTCTL;                         /**< Interrupt Control Register,          offset: 0x10 */
    __IO uint32_t RSVD2;
    __IO uint32_t STAS;                           /**< Status Register,                     offset: 0x18 */
    __IO uint32_t RFIFO;                          /**< RFIFO,                               offset: 0x1C */
    __IO uint32_t TFIFO;                          /**< TFIFO,                               offset: 0x20 */
    __IO uint32_t RSVD7;
    __IO uint32_t CSPICTL;
    __IO uint32_t RSVD4[13];
    __IO uint32_t LSPI_CTRL;                      /**< LSPI control                         offset: 0x60 */
    __IO uint32_t LSPI_CCTRL;                     /**< LSPI command control                 offset: 0x64 */
    __IO uint32_t LSPI_CADDR;                     /**< LSPI command addr                    offset: 0x68 */
    __IO uint32_t LSPI_STAT;                      /**< LSPI status                          offset: 0x6c */
    __IO uint32_t LSPI_RAMWLEN;                   /**< LSPI ram write len                   offset: 0x70 */
    __IO uint32_t LSPFINFO;                       /**< LSPI frame info                      offset: 0x74 */
    __IO uint32_t LSPTINFO0;                      /**< LSPI tailor info0                    offset: 0x78 */
    __IO uint32_t LSPTINFO;                       /**< LSPI tailor info                     offset: 0x7c */
    __IO uint32_t LSPSINFO;                       /**< LSPI scale info                      offset: 0x80 */
    __IO uint32_t LSPIQUARTCTRL;                  /**< LSPI quartile ctrl                   offset: 0x84 */
    __IO uint32_t LSPIQUARTUSE;                   /**< LSPI quartile inuse                  offset: 0x88 */
    __IO uint32_t LSPIYADJ;                       /**< LSPI Y adj                           offset: 0x8c */
    __IO uint32_t LSPIYADJUSE;                    /**< LSPI Y adj inuse                     offset: 0x90 */
    __IO uint32_t LSPIGPCMD0;                     /**< LSPI gray page cmd0                  offset: 0x94 */
    __IO uint32_t LSPIGPCMD1;                     /**< LSPI gray page cmd1                  offset: 0x98 */
    __IO uint32_t LSPFINFO0;                      /**< LSPI frame info out                  offset: 0x9c */
    __IO uint32_t YUV2RGBINFO0;                   /**< YUV to RGB info0                     offset: 0xa0 */
    __IO uint32_t YUV2RGBINFO1;                   /**< YUV to RGB info1                     offset: 0xa4 */
    __IO uint32_t RSVD5[14];                      /**< Reserved                                          */
    __IO uint32_t I2SBUSSEL;                      /**< LSPI bus select                      offset: 0xe0 */
    __IO uint32_t RSVD6[4];                       /**< Reserved                                          */
} LSPI_TypeDef;

/** @name DFMT - CSPI_DFMT register */
/** @{ */
#define LSPI_DFMT_SLAVE_MODE_EN_Pos               (0)
#define LSPI_DFMT_SLAVE_MODE_EN_Msk               (0x1UL << LSPI_DFMT_SLAVE_MODE_EN_Pos)

#define LSPI_DFMT_SLOT_SIZE_Pos                   (1)
#define LSPI_DFMT_SLOT_SIZE_Msk                   (0x1FUL << LSPI_DFMT_SLOT_SIZE_Pos)

#define LSPI_DFMT_WORD_SIZE_Pos                   (6)
#define LSPI_DFMT_WORD_SIZE_Msk                   (0x1FUL << LSPI_DFMT_WORD_SIZE_Pos)

#define LSPI_DFMT_ALIGN_MODE_Pos                  (11)
#define LSPI_DFMT_ALIGN_MODE_Msk                  (0x1UL << LSPI_DFMT_ALIGN_MODE_Pos)

#define LSPI_DFMT_ENDIAN_MODE_Pos                 (12)
#define LSPI_DFMT_ENDIAN_MODE_Msk                 (0x1UL << LSPI_DFMT_ENDIAN_MODE_Pos)

#define LSPI_DFMT_DATA_DLY_Pos                    (13)
#define LSPI_DFMT_DATA_DLY_Msk                    (0x3UL << LSPI_DFMT_DATA_DLY_Pos)

#define LSPI_DFMT_TX_PAD_Pos                      (15)
#define LSPI_DFMT_TX_PAD_Msk                      (0x3UL << LSPI_DFMT_TX_PAD_Pos)

#define LSPI_DFMT_RX_SIGN_EXT_Pos                 (17)
#define LSPI_DFMT_RX_SIGN_EXT_Msk                 (0x1UL << LSPI_DFMT_RX_SIGN_EXT_Pos)

#define LSPI_DFMT_TX_PACK_Pos                     (18)
#define LSPI_DFMT_TX_PACK_Msk                     (0x3UL << LSPI_DFMT_TX_PACK_Pos)

#define LSPI_DFMT_RX_PACK_Pos                     (20)
#define LSPI_DFMT_RX_PACK_Msk                     (0x3UL << LSPI_DFMT_RX_PACK_Pos)

#define LSPI_DFMT_TX_FIFO_ENDIAN_MODE_Pos         (22)
#define LSPI_DFMT_TX_FIFO_ENDIAN_MODE_Msk         (0x1UL << LSPI_DFMT_TX_FIFO_ENDIAN_MODE_Pos)

#define LSPI_DFMT_RX_FIFO_ENDIAN_MODE_Pos         (23)
#define LSPI_DFMT_RX_FIFO_ENDIAN_MODE_Msk         (0x1UL << LSPI_DFMT_RX_FIFO_ENDIAN_MODE_Pos)

#define LSPI_DFMT_EOR_MODE_Pos                    (24)
#define LSPI_DFMT_EOR_MODE_Msk                    (0x1UL << LSPI_DFMT_EOR_MODE_Pos)

/** @} */


/** @name INT_CTRL - CSPI_INT_CTRL register */
/** @{ */
#define LSPI_RAM_WR_BREAK_INT_EN_Pos                (11)
#define LSPI_RAM_WR_BREAK_INT_EN_Msk                (0x1UL << LSPI_RAM_WR_BREAK_INT_EN_Pos)

#define LSPI_RAM_WR_FRAME_START_EN_Pos              (12)
#define LSPI_RAM_WR_FRAME_START_EN_Msk              (0x1UL << LSPI_RAM_WR_FRAME_START_EN_Pos)

#define LSPI_RAM_WR_FRAME_END_EN_Pos                (13)
#define LSPI_RAM_WR_FRAME_END_EN_Msk                (0x1UL << LSPI_RAM_WR_FRAME_END_EN_Pos)

#define LSPI_COMMAND_END_EN_Pos                     (14)
#define LSPI_COMMAND_END_EN_Msk                     (0x1UL << LSPI_COMMAND_END_EN_Pos)

#define LSPI_RAM_WR_END_EN_Pos                      (16)
#define LSPI_RAM_WR_END_EN_Msk                      (0x1UL << LSPI_RAM_WR_END_EN_Pos)

/** @} */

  /** @name STATS_CTRL -CSPI_STATS_CTRL register */
  /** @{ */

#define LSPI_STATS_RAM_WR_BREAK_Pos                 (26)
#define LSPI_STATS_RAM_WR_BREAK_Msk                 (0x1UL << LSPI_STATS_RAM_WR_BREAK_Pos)

#define LSPI_STATS_RAM_WR_FRAME_START_Pos           (27)
#define LSPI_STATS_RAM_WR_FRAME_START_Msk           (0x1UL << LSPI_STATS_RAM_WR_FRAME_START_Pos)

#define LSPI_STATS_RAM_WR_FRAME_END_Pos             (28)
#define LSPI_STATS_RAM_WR_FRAME_END_Msk             (0x1UL << LSPI_STATS_RAM_WR_FRAME_END_Pos)

#define LSPI_STATS_COMMAND_END_Pos                  (30)
#define LSPI_STATS_COMMAND_END_Msk                  (0x1UL << LSPI_STATS_COMMAND_END_Pos)

#define LSPI_STATS_RAM_WR_END_Pos                   (31)
#define LSPI_STATS_RAM_WR_END_Msk                   (0x1UL << LSPI_STATS_RAM_WR_END_Pos)

  /** @} */

  /** @name CTRL - LSPI_CTRL register */
  /** @{ */
#define LSPI_CTRL_ENABLE_Pos                        (0)
#define LSPI_CTRL_ENABLE_Msk                        (0x1UL << LSPI_CTRL_ENABLE_Pos)

#define LSPI_CTRL_2DATA_LANE_Pos                    (1)
#define LSPI_CTRL_2DATA_LANE_Msk                    (0x1UL << LSPI_CTRL_2DATA_LANE_Pos)

#define LSPI_CTRL_4LINE_Pos                         (2)
#define LSPI_CTRL_4LINE_Msk                         (0x1UL << LSPI_CTRL_4LINE_Pos)

#define LSPI_CTRL_DAT_SRC_Pos                       (3)
#define LSPI_CTRL_DAT_SRC_Msk                       (0x1UL << LSPI_CTRL_DAT_SRC_Pos)

#define LSPI_CTRL_COLOR_MODE_IN_Pos                 (4)
#define LSPI_CTRL_COLOR_MODE_IN_Msk                 (0x3UL << LSPI_CTRL_COLOR_MODE_IN_Pos)

#define LSPI_CTRL_COLOR_MODE_OUT_Pos                (6)
#define LSPI_CTRL_COLOR_MODE_OUT_Msk                (0x7UL << LSPI_CTRL_COLOR_MODE_OUT_Pos)

#define LSPI_CTRL_Y_ADJ_EN_Pos                      (9)
#define LSPI_CTRL_Y_ADJ_EN_Msk                      (0x1UL << LSPI_CTRL_Y_ADJ_EN_Pos)

#define LSPI_CTRL_Y_ADJ_SEL_Pos                     (10)
#define LSPI_CTRL_Y_ADJ_SEL_Msk                     (0x1UL << LSPI_CTRL_Y_ADJ_SEL_Pos)

#define LSPI_CTRL_Y_ADJ_BOUND_Pos                   (11)
#define LSPI_CTRL_Y_ADJ_BOUND_Msk                   (0x1FFFFUL << LSPI_CTRL_Y_ADJ_BOUND_Pos)

#define LSPI_CTRL_DC_DUMMY_Pos                      (28)
#define LSPI_CTRL_DC_DUMMY_Msk                      (0x1UL << LSPI_CTRL_DC_DUMMY_Pos)

#define LSPI_CTRL_BUS_TYPE_Pos                      (29)
#define LSPI_CTRL_BUS_TYPE_Msk                      (0x1UL << LSPI_CTRL_BUS_TYPE_Pos)
  /** @} */

  /** @name COMMAND_CTRL - LSPI_COMMAND_CTRL register */
  /** @{ */
#define LSPI_COMMAND_CTRL_WR_RDN_Pos                (0)
#define LSPI_COMMAND_CTRL_WR_RDN_Msk                (0x1UL << LSPI_COMMAND_CTRL_WR_RDN_Pos)

#define LSPI_COMMAND_CTRL_RAM_WR_Pos                (1)
#define LSPI_COMMAND_CTRL_RAM_WR_Msk                (0x1UL << LSPI_COMMAND_CTRL_RAM_WR_Pos)

#define LSPI_COMMAND_CTRL_RDAT_DUMMY_CYCLE_Pos      (2)
#define LSPI_COMMAND_CTRL_RDAT_DUMMY_CYCLE_Msk      (0x3FUL << LSPI_COMMAND_CTRL_RDAT_DUMMY_CYCLE_Pos)

#define LSPI_COMMAND_CTRL_DATA_LEN_Pos              (8)
#define LSPI_COMMAND_CTRL_DATA_LEN_Msk              (0x3FFFFUL << LSPI_COMMAND_CTRL_DATA_LEN_Pos)

#define LSPI_COMMAND_CTRL_INIT_Pos                  (26)
#define LSPI_COMMAND_CTRL_INIT_Msk                  (0x1UL << LSPI_COMMAND_CTRL_INIT_Pos)
  /** @} */

  /** @name COMMAND_ADDR - LSPI_COMMAND_ADDR register */
  /** @{ */
#define LSPI_COMMAND_ADDR_ADDR_Pos                  (0)
#define LSPI_COMMAND_ADDR_ADDR_Msk                  (0xFFUL << LSPI_COMMAND_ADDR_ADDR_Pos)
  /** @} */

  /** @name STATS - LSPI_STATS register */
  /** @{ */
#define LSPI_STATS_IDLE_Pos                         (0)
#define LSPI_STATS_IDLE_Msk                         (0x1UL << LSPI_STATS_IDLE_Pos)
  /** @} */

  /** @name RAMWR_LEN - LSPI_RAMWR_LEN register */
  /** @{ */
#define LSPI_RAMWR_LEN_RAM_WR_LEN_Pos               (0)
#define LSPI_RAMWR_LEN_RAM_WR_LEN_Msk               (0x3FFFFUL << LSPI_RAMWR_LEN_RAM_WR_LEN_Pos)
  /** @} */

  /** @name FRAME_INFO - LSPI_FRAME_INFO register */
  /** @{ */
#define LSPI_FRAME_INFO_FRAME_HEIGHT_Pos            (0)
#define LSPI_FRAME_INFO_FRAME_HEIGHT_Msk            (0xFFFFUL << LSPI_FRAME_INFO_FRAME_HEIGHT_Pos)

#define LSPI_FRAME_INFO_FRAME_WIDTH_Pos             (16)
#define LSPI_FRAME_INFO_FRAME_WIDTH_Msk             (0xFFFFUL << LSPI_FRAME_INFO_FRAME_WIDTH_Pos)
  /** @} */

  /** @name TAILOR_INFO0 - LSPI_TAILOR_INFO0 register */
  /** @{ */
#define LSPI_TAILOR_INFO0_TAILOR_BOTTOM_Pos         (0)
#define LSPI_TAILOR_INFO0_TAILOR_BOTTOM_Msk         (0x3FFUL << LSPI_TAILOR_INFO0_TAILOR_BOTTOM_Pos)

#define LSPI_TAILOR_INFO0_TAILOR_TOP_Pos            (10)
#define LSPI_TAILOR_INFO0_TAILOR_TOP_Msk            (0x3FFUL << LSPI_TAILOR_INFO0_TAILOR_TOP_Pos)
  /** @} */

  /** @name TAILOR_INFO - LSPI_TAILOR_INFO register */
  /** @{ */
#define LSPI_TAILOR_INFO_TAILOR_LEFT_Pos            (0)
#define LSPI_TAILOR_INFO_TAILOR_LEFT_Msk            (0x3FFUL << LSPI_TAILOR_INFO_TAILOR_LEFT_Pos)

#define LSPI_TAILOR_INFO_TAILOR_RIGHT_Pos           (10)
#define LSPI_TAILOR_INFO_TAILOR_RIGHT_Msk           (0x3FFUL << LSPI_TAILOR_INFO_TAILOR_RIGHT_Pos)
  /** @} */

  /** @name SCALE_INFO - LSPI_SCALE_INFO register */
  /** @{ */
#define LSPI_SCALE_INFO_ROW_SCALE_FRAC_Pos          (0)
#define LSPI_SCALE_INFO_ROW_SCALE_FRAC_Msk          (0x7FUL << LSPI_SCALE_INFO_ROW_SCALE_FRAC_Pos)

#define LSPI_SCALE_INFO_COL_SCALE_FRAC_Pos          (7)
#define LSPI_SCALE_INFO_COL_SCALE_FRAC_Msk          (0x7FUL << LSPI_SCALE_INFO_COL_SCALE_FRAC_Pos)
  /** @} */

  /** @name QUARTILE_CTRL - LSPI_QUARTILE_CTRL register */
  /** @{ */
#define LSPI_QUARTILE_CTRL_GRAY_CTRL_Pos            (0)
#define LSPI_QUARTILE_CTRL_GRAY_CTRL_Msk            (0x3UL << LSPI_QUARTILE_CTRL_GRAY_CTRL_Pos)

#define LSPI_QUARTILE_CTRL_QUARTILE_SEL_Pos         (2)
#define LSPI_QUARTILE_CTRL_QUARTILE_SEL_Msk         (0x1UL << LSPI_QUARTILE_CTRL_QUARTILE_SEL_Pos)

#define LSPI_QUARTILE_CTRL_QUARTILE1_Pos            (3)
#define LSPI_QUARTILE_CTRL_QUARTILE1_Msk            (0xFFUL << LSPI_QUARTILE_CTRL_QUARTILE1_Pos)

#define LSPI_QUARTILE_CTRL_QUARTILE2_Pos            (11)
#define LSPI_QUARTILE_CTRL_QUARTILE2_Msk            (0xFFUL << LSPI_QUARTILE_CTRL_QUARTILE2_Pos)

#define LSPI_QUARTILE_CTRL_QUARTILE3_Pos            (19)
#define LSPI_QUARTILE_CTRL_QUARTILE3_Msk            (0xFFUL << LSPI_QUARTILE_CTRL_QUARTILE3_Pos)
  /** @} */

  /** @name QUARTILE_INUSE - LSPI_QUARTILE_INUSE register */
  /** @{ */
#define LSPI_QUARTILE_INUSE_QUARTILE1_INUSE_Pos     (0)
#define LSPI_QUARTILE_INUSE_QUARTILE1_INUSE_Msk     (0xFFUL << LSPI_QUARTILE_INUSE_QUARTILE1_INUSE_Pos)

#define LSPI_QUARTILE_INUSE_QUARTILE2_INUSE_Pos     (8)
#define LSPI_QUARTILE_INUSE_QUARTILE2_INUSE_Msk     (0xFFUL << LSPI_QUARTILE_INUSE_QUARTILE2_INUSE_Pos)

#define LSPI_QUARTILE_INUSE_QUARTILE3_INUSE_Pos     (16)
#define LSPI_QUARTILE_INUSE_QUARTILE3_INUSE_Msk     (0xFFUL << LSPI_QUARTILE_INUSE_QUARTILE3_INUSE_Pos)
  /** @} */

  /** @name Y_ADJ - LSPI_Y_ADJ register */
  /** @{ */
#define LSPI_Y_ADJ_YADJ_Y_MIN_Pos                   (0)
#define LSPI_Y_ADJ_YADJ_Y_MIN_Msk                   (0xFFUL << LSPI_Y_ADJ_YADJ_Y_MIN_Pos)

#define LSPI_Y_ADJ_YADJ_Y_MAX_Pos                   (8)
#define LSPI_Y_ADJ_YADJ_Y_MAX_Msk                   (0xFFUL << LSPI_Y_ADJ_YADJ_Y_MAX_Pos)

#define LSPI_Y_ADJ_YADJ_STRECH_Pos                  (16)
#define LSPI_Y_ADJ_YADJ_STRECH_Msk                  (0xFFUL << LSPI_Y_ADJ_YADJ_STRECH_Pos)

#define LSPI_Y_ADJ_YADJ_STRECH_FWL_Pos              (24)
#define LSPI_Y_ADJ_YADJ_STRECH_FWL_Msk              (0xFFUL << LSPI_Y_ADJ_YADJ_STRECH_FWL_Pos)
  /** @} */

  /** @name Y_ADJ_INUSE - LSPI_Y_ADJ_INUSE register */
  /** @{ */
#define LSPI_Y_ADJ_INUSE_YADJ_Y_MIN_INUSE_Pos       (0)
#define LSPI_Y_ADJ_INUSE_YADJ_Y_MIN_INUSE_Msk       (0xFFUL << LSPI_Y_ADJ_INUSE_YADJ_Y_MIN_INUSE_Pos)

#define LSPI_Y_ADJ_INUSE_YADJ_Y_MAX_INUSE_Pos       (8)
#define LSPI_Y_ADJ_INUSE_YADJ_Y_MAX_INUSE_Msk       (0xFFUL << LSPI_Y_ADJ_INUSE_YADJ_Y_MAX_INUSE_Pos)

#define LSPI_Y_ADJ_INUSE_YADJ_STRECH_INUSE_Pos      (16)
#define LSPI_Y_ADJ_INUSE_YADJ_STRECH_INUSE_Msk      (0xFFUL << LSPI_Y_ADJ_INUSE_YADJ_STRECH_INUSE_Pos)

#define LSPI_Y_ADJ_INUSE_YADJ_STRECH_FWL_INUSE_Pos  (24)
#define LSPI_Y_ADJ_INUSE_YADJ_STRECH_FWL_INUSE_Msk  (0xFFUL << LSPI_Y_ADJ_INUSE_YADJ_STRECH_FWL_INUSE_Pos)
  /** @} */

  /** @name GRAY_PAGE_CMD0 - LSPI_GRAY_PAGE_CMD0 register */
  /** @{ */
#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD_Pos            (0)
#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD_Msk            (0xFFUL << LSPI_GRAY_PAGE_CMD0_PAGE_CMD_Pos)

#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD0_Pos           (8)
#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD0_Msk           (0xFFFFUL << LSPI_GRAY_PAGE_CMD0_PAGE_CMD0_Pos)

#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD01_BYTE_NUM_Pos (24)
#define LSPI_GRAY_PAGE_CMD0_PAGE_CMD01_BYTE_NUM_Msk (0xFUL << LSPI_GRAY_PAGE_CMD0_PAGE_CMD01_BYTE_NUM_Pos)
  /** @} */

  /** @name GRAY_PAGE_CMD1 - LSPI_GRAY_PAGE_CMD1 register */
  /** @{ */
#define LSPI_GRAY_PAGE_CMD1_PAGE_CMD1_Pos           (0)
#define LSPI_GRAY_PAGE_CMD1_PAGE_CMD1_Msk           (0xFFFFFFFFUL << LSPI_GRAY_PAGE_CMD1_PAGE_CMD1_Pos)
  /** @} */

  /** @name FRAME_INFO_OUT - LSPI_FRAME_INFO_OUT register */
  /** @{ */
#define LSPI_FRAME_INFO_OUT_FRAME_HEIGHT_OUT_Pos    (0)
#define LSPI_FRAME_INFO_OUT_FRAME_HEIGHT_OUT_Msk    (0x3FFUL << LSPI_FRAME_INFO_OUT_FRAME_HEIGHT_OUT_Pos)

#define LSPI_FRAME_INFO_OUT_FRAME_WIDTH_OUT_Pos     (10)
#define LSPI_FRAME_INFO_OUT_FRAME_WIDTH_OUT_Msk     (0x3FFUL << LSPI_FRAME_INFO_OUT_FRAME_WIDTH_OUT_Pos)
  /** @} */

  /** @name YUV2RGB_INFO0 - LSPI_YUV2RGB_INFO0 register */
  /** @{ */
#define LSPI_YUV2RGB_INFO0_YUV2RGB_Y_OFST_Pos       (0)
#define LSPI_YUV2RGB_INFO0_YUV2RGB_Y_OFST_Msk       (0xFFUL << LSPI_YUV2RGB_INFO0_YUV2RGB_Y_OFST_Pos)

#define LSPI_YUV2RGB_INFO0_YUV2RGB_C0_Pos           (8)
#define LSPI_YUV2RGB_INFO0_YUV2RGB_C0_Msk           (0x3FFUL << LSPI_YUV2RGB_INFO0_YUV2RGB_C0_Pos)

#define LSPI_YUV2RGB_INFO0_YUV2RGB_C1_Pos           (18)
#define LSPI_YUV2RGB_INFO0_YUV2RGB_C1_Msk           (0x3FFUL << LSPI_YUV2RGB_INFO0_YUV2RGB_C1_Pos)
  /** @} */

  /** @name YUV2RGB_INFO1 - LSPI_YUV2RGB_INFO1 register */
  /** @{ */
#define LSPI_YUV2RGB_INFO1_YUV2RGB_C2_Pos           (0)
#define LSPI_YUV2RGB_INFO1_YUV2RGB_C2_Msk           (0x3FFUL << LSPI_YUV2RGB_INFO1_YUV2RGB_C2_Pos)

#define LSPI_YUV2RGB_INFO1_YUV2RGB_C3_Pos           (10)
#define LSPI_YUV2RGB_INFO1_YUV2RGB_C3_Msk           (0x3FFUL << LSPI_YUV2RGB_INFO1_YUV2RGB_C3_Pos)

#define LSPI_YUV2RGB_INFO1_YUV2RGB_C4_Pos           (20)
#define LSPI_YUV2RGB_INFO1_YUV2RGB_C4_Msk           (0x3FFUL << LSPI_YUV2RGB_INFO1_YUV2RGB_C4_Pos)
  /** @} */

  /** @name BUS_SEL - I2S_BUS_SEL register */
  /** @{ */
#define I2S_BUS_SEL_I2S_BUS_EN_Pos                  (0)
#define I2S_BUS_SEL_I2S_BUS_EN_Msk                  (0x1UL << I2S_BUS_SEL_I2S_BUS_EN_Pos)

#define I2S_BUS_SEL_CSPI_BUS_EN_Pos                 (1)
#define I2S_BUS_SEL_CSPI_BUS_EN_Msk                 (0x1UL << I2S_BUS_SEL_CSPI_BUS_EN_Pos)

#define I2S_BUS_SEL_LSPI_BUS_EN_Pos                 (2)
#define I2S_BUS_SEL_LSPI_BUS_EN_Msk                 (0x1UL << I2S_BUS_SEL_LSPI_BUS_EN_Pos)
  /** @} */



  /** Peripheral LSPI1 base pointer */
#define LSPI1                                       ((LSPI_TypeDef *)MP_USP1_BASE_ADDR)

  /** Peripheral LSPI2 base pointer */
#define LSPI2                                       ((LSPI_TypeDef *)MP_USP2_BASE_ADDR)

  /** @brief SPI peripheral instance number */
#define LSPI_INSTANCE_NUM                           (2)



  /**
  * @}
  */ /* end of group LSPI */

/** @addtogroup KPC KPC
  * @{
  */

/**
  * @brief KPC register layout typedef
  *
  */
typedef struct {
    __IO uint32_t DEBCTL;                           /**< Debounce Control Register,              offset: 0x0 */
    __IO uint32_t KPCTL;                            /**< Keypad Control Register,                offset: 0x4 */
    __IO uint32_t DICTL;                            /**< Direct Input Control Register,          offset: 0x8 */
    __IO uint32_t KPENCTL;                          /**< Keypad Enable Register,                 offset: 0xC */
    __IO uint32_t DIENCTL;                          /**< Direct Input Enable Register,           offset: 0x10 */
    __IO uint32_t AUTOCG;                           /**< Auto Gate Enable Register,              offset: 0x14 */
    __IO uint32_t CLRCTL;                           /**< Direct Input Clear Control Register,    offset: 0x18 */
    __I  uint32_t KPSTAT;                           /**< Keypad Status Register,                 offset: 0x1C */
    __I  uint32_t DISTAT;                           /**< Direct Input Status Register,           offset: 0x20 */
} KPC_TypeDef;

/** @name DEBCTL - KPC_DEBCTL register */
/** @{ */
#define KPC_DEBCTL_DEBOUNCER_DEPTH_Pos           (0)
#define KPC_DEBCTL_DEBOUNCER_DEPTH_Msk           (0xFUL << KPC_DEBCTL_DEBOUNCER_DEPTH_Pos)

#define KPC_DEBCTL_DEBOUNCER_TO0_THRD_Pos        (4)
#define KPC_DEBCTL_DEBOUNCER_TO0_THRD_Msk        (0xFUL << KPC_DEBCTL_DEBOUNCER_TO0_THRD_Pos)

#define KPC_DEBCTL_DEBOUNCER_TO1_THRD_Pos        (8)
#define KPC_DEBCTL_DEBOUNCER_TO1_THRD_Msk        (0xFUL << KPC_DEBCTL_DEBOUNCER_TO1_THRD_Pos)

#define KPC_DEBCTL_DEBOUNCER_TO_MCLK_RATIO_Pos   (12)
#define KPC_DEBCTL_DEBOUNCER_TO_MCLK_RATIO_Msk   (0xFUL << KPC_DEBCTL_DEBOUNCER_TO_MCLK_RATIO_Pos)
/** @} */

/** @name KPCTL - KPC_KPCTL register */
/** @{ */
#define KPC_KPCTL_POLARITY_Pos                   (0)
#define KPC_KPCTL_POLARITY_Msk                   (0x1UL << KPC_KPCTL_POLARITY_Pos)

#define KPC_KPCTL_ROW_VLD_BITMAP_Pos             (1)
#define KPC_KPCTL_ROW_VLD_BITMAP_Msk             (0x1FUL << KPC_KPCTL_ROW_VLD_BITMAP_Pos)

#define KPC_KPCTL_COL_VLD_BITMAP_Pos             (6)
#define KPC_KPCTL_COL_VLD_BITMAP_Msk             (0x1FUL << KPC_KPCTL_COL_VLD_BITMAP_Pos)

#define KPC_KPCTL_SCAN_TO_DEBOUNCE_RATIO_Pos     (11)
#define KPC_KPCTL_SCAN_TO_DEBOUNCE_RATIO_Msk     (0x7UL << KPC_KPCTL_SCAN_TO_DEBOUNCE_RATIO_Pos)
/** @} */

/** @name DICTL - KPC_DICTL register */
/** @{ */
#define KPC_DICTL_INT_MODE_Pos                   (0)
#define KPC_DICTL_INT_MODE_Msk                   (0x3UL << KPC_DICTL_INT_MODE_Pos)

#define KPC_DICTL_INT_EN_Pos                     (2)
#define KPC_DICTL_INT_EN_Msk                     (0x3FFUL << KPC_DICTL_INT_EN_Pos)
/** @} */

/** @name KPENCTL - KPC_KPENCTL register */
/** @{ */
#define KPC_KPENCTL_ENABLE_Pos                   (0)
#define KPC_KPENCTL_ENABLE_Msk                   (0x1UL << KPC_KPENCTL_ENABLE_Pos)

#define KPC_KPENCTL_RELEASE_DET_EN_Pos           (1)
#define KPC_KPENCTL_RELEASE_DET_EN_Msk           (0x1FFFFFFUL << KPC_KPENCTL_RELEASE_DET_EN_Pos)

#define KPC_KPENCTL_PULL_EN_Pos                  (26)
#define KPC_KPENCTL_PULL_EN_Msk                  (0x1UL << KPC_KPENCTL_PULL_EN_Pos)

#define KPC_KPENCTL_WORK_MODE_Pos                (27)
#define KPC_KPENCTL_WORK_MODE_Msk                (0x1UL << KPC_KPENCTL_WORK_MODE_Pos)
/** @} */

/** @name DIENCTL - KPC_DIENCTL register */
/** @{ */
#define KPC_DIENCTL_ENABLE_Pos                   (0)
#define KPC_DIENCTL_ENABLE_Msk                   (0x1UL << KPC_DIENCTL_ENABLE_Pos)
/** @} */

/** @name AUTOCG - KPC_AUTOCG register */
/** @{ */
#define KPC_AUTOCG_ENABLE_Pos                    (0)
#define KPC_AUTOCG_ENABLE_Msk                    (0x1UL << KPC_AUTOCG_ENABLE_Pos)
/** @} */

/** @name CLRCTL - KPC_CLRCTL register */
/** @{ */
#define KPC_CLRCTL_INPUT_INT_CLR_Pos             (0)
#define KPC_CLRCTL_INPUT_INT_CLR_Msk             (0x3FFUL << KPC_CLRCTL_INPUT_INT_CLR_Pos)

#define KPC_CLRCTL_DEBOUNCER_CLR_Pos             (10)
#define KPC_CLRCTL_DEBOUNCER_CLR_Msk             (0x3FFUL << KPC_CLRCTL_DEBOUNCER_CLR_Pos)
/** @} */

/** @name KPSTAT - KPC_KPSTAT register */
/** @{ */
#define KPC_KPSTAT_STATUS_Pos                    (0)
#define KPC_KPSTAT_STATUS_Msk                    (0x1FFFFFFUL << KPC_KPSTAT_STATUS_Pos)
/** @} */

/** @name DISTAT - KPC_DISTAT register */
/** @{ */
#define KPC_DISTAT_INPUT_INT_NEG_STATUS_Pos      (0)
#define KPC_DISTAT_INPUT_INT_NEG_STATUS_Msk      (0x3FFUL << KPC_DISTAT_INPUT_INT_NEG_STATUS_Pos)

#define KPC_DISTAT_INPUT_INT_POS_STATUS_Pos      (10)
#define KPC_DISTAT_INPUT_INT_POS_STATUS_Msk      (0x3FFUL << KPC_DISTAT_INPUT_INT_POS_STATUS_Pos)

#define KPC_DISTAT_INPUT_STATUS_Pos              (20)
#define KPC_DISTAT_INPUT_STATUS_Msk              (0x3FFUL << KPC_DISTAT_INPUT_STATUS_Pos)
/** @} */

/** Peripheral KPC base pointer */
#define KPC                                      ((KPC_TypeDef *) AP_KPC_BASE_ADDR)

/**
  * @}
  */ /* end of group KPC */

/** @addtogroup LPUSART LPUSART
  * @{
  */

/**
  * @brief LPUSART CORE part register layout typedef
  *
  */
typedef struct _USART_TypeDef LPUSARTCORE_TypeDef;

/**
  * @brief LPUSART AON part register layout typedef
  *
  */
typedef struct {
    __IO uint32_t CR0;                           /**< Control Register 0,                       offset: 0x0 */
         uint32_t RESERVED_0[138];
    __IO uint32_t CR1;                           /**< Control Register 1,                       offset: 0x0 */
    __IO uint32_t DLR;                           /**< Divisor Latch Register,                   offset: 0x4 */
    __IO uint32_t LCR;                           /**< Line Control Register,                    offset: 0x8 */
    __IO uint32_t SCR;                           /**< Stop Control Register,                    offset: 0xC */
    __I  uint32_t FSR;                           /**< FIFO Status Register,                     offset: 0x10 */
    __I  uint32_t IIR;                           /**< Interrupt Identification Register,        offset: 0x14 */
    __I  uint32_t SR;                            /**< Status Register,                          offset: 0x18 */
    __I  uint32_t ABSR;                          /**< Auto Baud Status Register,                offset: 0x1C */
    __I  uint32_t ABDR;                          /**< Auto Baud Divisor Register,               offset: 0x20 */
} LPUSARTAON_TypeDef;

/** @name CR0 - LPUSARTAON_CR0 register */
/** @{ */
#define LPUSARTAON_CR0_RX_ENABLE_Pos             (0)
#define LPUSARTAON_CR0_RX_ENABLE_Msk             (0x1UL << LPUSARTAON_CR0_RX_ENABLE_Pos)

#define LPUSARTAON_CR0_CLK_ENABLE_Pos            (1)
#define LPUSARTAON_CR0_CLK_ENABLE_Msk            (0x1UL << LPUSARTAON_CR0_CLK_ENABLE_Pos)
/** @} */

/** @name CR1 - LPUSARTAON_CR1 register */
/** @{ */
#define LPUSARTAON_CR1_ENABLE_Pos                (0)
#define LPUSARTAON_CR1_ENABLE_Msk                (0x1UL << LPUSARTAON_CR1_ENABLE_Pos)

#define LPUSARTAON_CR1_ACG_EN_Pos                (1)
#define LPUSARTAON_CR1_ACG_EN_Msk                (0x1UL << LPUSARTAON_CR1_ACG_EN_Pos)

#define LPUSARTAON_CR1_AUTO_ADJ_Pos              (2)
#define LPUSARTAON_CR1_AUTO_ADJ_Msk              (0x1UL << LPUSARTAON_CR1_AUTO_ADJ_Pos)

#define LPUSARTAON_CR1_AUTO_BAUD_Pos             (3)
#define LPUSARTAON_CR1_AUTO_BAUD_Msk             (0x1UL << LPUSARTAON_CR1_AUTO_BAUD_Pos)

#define LPUSARTAON_CR1_BREAK_MASK_Pos            (4)
#define LPUSARTAON_CR1_BREAK_MASK_Msk            (0x1UL << LPUSARTAON_CR1_BREAK_MASK_Pos)
/** @} */

/** @name DLR - LPUSARTAON_DLR register */
/** @{ */
#define LPUSARTAON_DLR_DIVISOR_Pos               (0)
#define LPUSARTAON_DLR_DIVISOR_Msk               (0xFFFUL << LPUSARTAON_DLR_DIVISOR_Pos)
/** @} */

/** @name LCR - LPUSARTAON_LCR register */
/** @{ */
#define LPUSARTAON_LCR_CHAR_LEN_Pos              (0)
#define LPUSARTAON_LCR_CHAR_LEN_Msk              (0x3UL << LPUSARTAON_LCR_CHAR_LEN_Pos)

#define LPUSARTAON_LCR_STOPBIT_DETECT_EN_Pos     (2)
#define LPUSARTAON_LCR_STOPBIT_DETECT_EN_Msk     (0x1UL << LPUSARTAON_LCR_STOPBIT_DETECT_EN_Pos)

#define LPUSARTAON_LCR_PARITY_EN_Pos             (3)
#define LPUSARTAON_LCR_PARITY_EN_Msk             (0x1UL << LPUSARTAON_LCR_PARITY_EN_Pos)

#define LPUSARTAON_LCR_EVEN_PARITY_Pos           (4)
#define LPUSARTAON_LCR_EVEN_PARITY_Msk           (0x1UL << LPUSARTAON_LCR_EVEN_PARITY_Pos)
/** @} */

/** @name SCR - LPUSARTAON_SCR register */
/** @{ */
#define LPUSARTAON_SCR_STOP_SW_SET_Pos           (0)
#define LPUSARTAON_SCR_STOP_SW_SET_Msk           (0x1UL << LPUSARTAON_SCR_STOP_SW_SET_Pos)

#define LPUSARTAON_SCR_STOP_SW_CLR_Pos           (1)
#define LPUSARTAON_SCR_STOP_SW_CLR_Msk           (0x1UL << LPUSARTAON_SCR_STOP_SW_CLR_Pos)

#define LPUSARTAON_SCR_STOP_FLAG_Pos             (3)
#define LPUSARTAON_SCR_STOP_FLAG_Msk             (0x1UL << LPUSARTAON_SCR_STOP_FLAG_Pos)
/** @} */

/** @name FSR - LPUSARTAON_FSR register */
/** @{ */
#define LPUSARTAON_FSR_RXFIFO_EMPTY_Pos          (1)
#define LPUSARTAON_FSR_RXFIFO_EMPTY_Msk          (0x1UL << LPUSARTAON_FSR_RXFIFO_EMPTY_Pos)

#define LPUSARTAON_FSR_RXFIFO_FULL_Pos           (2)
#define LPUSARTAON_FSR_RXFIFO_FULL_Msk           (0x1UL << LPUSARTAON_FSR_RXFIFO_FULL_Pos)

#define LPUSARTAON_FSR_RX_BUSY_Pos               (3)
#define LPUSARTAON_FSR_RX_BUSY_Msk               (0x1UL << LPUSARTAON_FSR_RX_BUSY_Pos)

#define LPUSARTAON_FSR_RXFIFO_NUM_Pos            (8)
#define LPUSARTAON_FSR_RXFIFO_NUM_Msk            (0xFUL << LPUSARTAON_FSR_RXFIFO_NUM_Pos)
/** @} */

/** @name IIR - LPUSARTAON_IIR register */
/** @{ */
#define LPUSARTAON_IIR_RXFIFO_OVERRUN_Pos        (1)
#define LPUSARTAON_IIR_RXFIFO_OVERRUN_Msk        (0x1UL << LPUSARTAON_IIR_RXFIFO_OVERRUN_Pos)

#define LPUSARTAON_IIR_PARITY_ERR_Pos            (2)
#define LPUSARTAON_IIR_PARITY_ERR_Msk            (0x1UL << LPUSARTAON_IIR_PARITY_ERR_Pos)

#define LPUSARTAON_IIR_FRAME_ERR_Pos             (3)
#define LPUSARTAON_IIR_FRAME_ERR_Msk             (0x1UL << LPUSARTAON_IIR_FRAME_ERR_Pos)
/** @} */

/** @name LSR - LPUSARTAON_LSR register */
/** @{ */
#define LPUSARTAON_LSR_STATUS_Pos                (0)
#define LPUSARTAON_LSR_STATUS_Msk                (0xFFFFFFFFUL << LPUSARTAON_LSR_STATUS_Pos)
/** @} */

/** @name ABSR - LPUSARTAON_ABSR register */
/** @{ */
#define LPUSARTAON_ABSR_DONE_Pos                 (0)
#define LPUSARTAON_ABSR_DONE_Msk                 (0x1UL << LPUSARTAON_ABSR_DONE_Pos)

#define LPUSARTAON_ABSR_BREAK_Pos                (1)
#define LPUSARTAON_ABSR_BREAK_Msk                (0x1UL << LPUSARTAON_ABSR_BREAK_Pos)
/** @} */

/** @name ABDR - LPUSARTAON_ABDR register */
/** @{ */
#define LPUSARTAON_ABDR_VALUE_Pos                (0)
#define LPUSARTAON_ABDR_VALUE_Msk                (0xFFFUL << LPUSARTAON_ABDR_VALUE_Pos)
/** @} */

/** Peripheral LPUART base pointer */
#define LPUSART_AON                              ((LPUSARTAON_TypeDef *) LPUSARTAON_BASE_ADDR)
#define LPUSART_CORE                             ((LPUSARTCORE_TypeDef *) MP_UART1_BASE_ADDR)
/**
  * @}
  */ /* end of group LPUSART */

/** @addtogroup OWC OWC
  * @{
  */

/**
  * @brief OWC register layout typedef
  *
  */
typedef struct {
    __IO uint32_t ECR;                           /**< Enable Control Register,              offset: 0x0 */
    __IO uint32_t CDR;                           /**< Clock Divider Register,               offset: 0x4 */
    __IO uint32_t IOR;                           /**< IO Control/Status Register,           offset: 0x8 */
    __IO uint32_t DFR;                           /**< Data Format Register,                 offset: 0xC */
    __IO uint32_t OCR;                           /**< Operation Command Register,           offset: 0x10 */
    __IO uint32_t TBR;                           /**< Transmit Buffer Register,             offset: 0x14 */
    __IO uint32_t RBR;                           /**< Receive Buffer Register,              offset: 0x18 */
    __IO uint32_t IER;                           /**< Interrupt Enable Register,            offset: 0x1C */
    __IO uint32_t IIR;                           /**< Interrupt Identification Register,    offset: 0x20 */
    __IO uint32_t CSR;                           /**< Control Status Register,              offset: 0x24 */
    __IO uint32_t RTCR;                          /**< Reset Timing Control Register,        offset: 0x28 */
    __IO uint32_t ATCR;                          /**< Access Timing Control Register,       offset: 0x2C */
} OWC_TypeDef;


/** @name ECR - OWC_ECR register */
/** @{ */
#define OWC_ECR_ENABLE_Pos                       (0)
#define OWC_ECR_ENABLE_Msk                       (0x1UL << OWC_ECR_ENABLE_Pos)

#define OWC_ECR_CLK_EN_Pos                       (1)
#define OWC_ECR_CLK_EN_Msk                       (0x1UL << OWC_ECR_CLK_EN_Pos)

#define OWC_ECR_AUTO_CGEN_Pos                    (2)
#define OWC_ECR_AUTO_CGEN_Msk                    (0x1UL << OWC_ECR_AUTO_CGEN_Pos)

#define OWC_ECR_RXD_MJR_Pos                      (3)
#define OWC_ECR_RXD_MJR_Msk                      (0x1UL << OWC_ECR_RXD_MJR_Pos)
/** @} */

/** @name CDR - OWC_CDR register */
/** @{ */
#define OWC_CDR_CLKUS_DIVIDER_Pos                (0)
#define OWC_CDR_CLKUS_DIVIDER_Msk                (0xFFUL << OWC_CDR_CLKUS_DIVIDER_Pos)
/** @} */

/** @name IOR - OWC_IOR register */
/** @{ */
#define OWC_IOR_SWMODE_EN_Pos                    (0)
#define OWC_IOR_SWMODE_EN_Msk                    (0x1UL << OWC_IOR_SWMODE_EN_Pos)

#define OWC_IOR_SWOEN_Pos                        (1)
#define OWC_IOR_SWOEN_Msk                        (0x1UL << OWC_IOR_SWOEN_Pos)

#define OWC_IOR_SWOUT_Pos                        (2)
#define OWC_IOR_SWOUT_Msk                        (0x1UL << OWC_IOR_SWOUT_Pos)

#define OWC_IOR_SWIN_Pos                         (3)
#define OWC_IOR_SWIN_Msk                         (0x1UL << OWC_IOR_SWIN_Pos)

#define OWC_IOR_SWIN_SYNC_Pos                    (4)
#define OWC_IOR_SWIN_SYNC_Msk                    (0x1UL << OWC_IOR_SWIN_SYNC_Pos)
/** @} */

/** @name DFR - OWC_DFR register */
/** @{ */
#define OWC_DFR_MODE_Pos                         (0)
#define OWC_DFR_MODE_Msk                         (0x1UL << OWC_DFR_MODE_Pos)

#define OWC_DFR_BYTE_ENDIAN_Pos                  (1)
#define OWC_DFR_BYTE_ENDIAN_Msk                  (0x1UL << OWC_DFR_BYTE_ENDIAN_Pos)

#define OWC_DFR_POLARITY_Pos                     (2)
#define OWC_DFR_POLARITY_Msk                     (0x1UL << OWC_DFR_POLARITY_Pos)
/** @} */

/** @name OCR - OWC_OCR register */
/** @{ */
#define OWC_OCR_FLUSH_Pos                        (0)
#define OWC_OCR_FLUSH_Msk                        (0x1UL << OWC_OCR_FLUSH_Pos)

#define OWC_OCR_RESET_Pos                        (1)
#define OWC_OCR_RESET_Msk                        (0x1UL << OWC_OCR_RESET_Pos)

#define OWC_OCR_WRITE_Pos                        (2)
#define OWC_OCR_WRITE_Msk                        (0x1UL << OWC_OCR_WRITE_Pos)

#define OWC_OCR_READ_Pos                         (3)
#define OWC_OCR_READ_Msk                         (0x1UL << OWC_OCR_READ_Pos)
/** @} */

/** @name TBR - OWC_TBR register */
/** @{ */
#define OWC_TBR_TX_BUF_Pos                       (0)
#define OWC_TBR_TX_BUF_Msk                       (0xFFUL << OWC_TBR_TX_BUF_Pos)
/** @} */

/** @name RBR - OWC_RBR register */
/** @{ */
#define OWC_RBR_RX_BUF_Pos                       (0)
#define OWC_RBR_RX_BUF_Msk                       (0xFFUL << OWC_RBR_RX_BUF_Pos)
/** @} */

/** @name IER - OWC_IER register */
/** @{ */
#define OWC_IER_RESET_Pos                        (1)
#define OWC_IER_RESET_Msk                        (0x1UL << OWC_IER_RESET_Pos)

#define OWC_IER_RESET_PD_Pos                     (2)
#define OWC_IER_RESET_PD_Msk                     (0x1UL << OWC_IER_RESET_PD_Pos)

#define OWC_IER_WRITE_Pos                        (3)
#define OWC_IER_WRITE_Msk                        (0x1UL << OWC_IER_WRITE_Pos)

#define OWC_IER_READ_Pos                         (4)
#define OWC_IER_READ_Msk                         (0x1UL << OWC_IER_READ_Pos)
/** @} */

/** @name IIR - OWC_IIR register */
/** @{ */
#define OWC_IIR_INT_CLR_Pos                      (0)
#define OWC_IIR_INT_CLR_Msk                      (0x1UL << OWC_IIR_INT_CLR_Pos)

#define OWC_IIR_RESET_Pos                        (1)
#define OWC_IIR_RESET_Msk                        (0x1UL << OWC_IIR_RESET_Pos)

#define OWC_IIR_RESET_PD_Pos                     (2)
#define OWC_IIR_RESET_PD_Msk                     (0x1UL << OWC_IIR_RESET_PD_Pos)

#define OWC_IIR_WRITE_Pos                        (3)
#define OWC_IIR_WRITE_Msk                        (0x1UL << OWC_IIR_WRITE_Pos)

#define OWC_IIR_READ_Pos                         (4)
#define OWC_IIR_READ_Msk                         (0x1UL << OWC_IIR_READ_Pos)

#define OWC_IIR_RESET_PD_RES_Pos                 (7)
#define OWC_IIR_RESET_PD_RES_Msk                 (0x1UL << OWC_IIR_RESET_PD_RES_Pos)
/** @} */

/** @name CSR - OWC_CSR register */
/** @{ */
#define OWC_CSR_SFTREG_Pos                       (0)
#define OWC_CSR_SFTREG_Msk                       (0xFFUL << OWC_CSR_SFTREG_Pos)

#define OWC_CSR_SFTCNT_Pos                       (8)
#define OWC_CSR_SFTCNT_Msk                       (0x7UL << OWC_CSR_SFTCNT_Pos)

#define OWC_CSR_FSM_Pos                          (12)
#define OWC_CSR_FSM_Msk                          (0x7UL << OWC_CSR_FSM_Pos)

#define OWC_CSR_USCNT_Pos                        (16)
#define OWC_CSR_USCNT_Msk                        (0x3FFUL << OWC_CSR_USCNT_Pos)

#define OWC_CSR_MODE_Pos                         (28)
#define OWC_CSR_MODE_Msk                         (0x3UL << OWC_CSR_MODE_Pos)

#define OWC_CSR_USCLK_ENABLE_Pos                 (31)
#define OWC_CSR_USCLK_ENABLE_Msk                 (0x1UL << OWC_CSR_USCLK_ENABLE_Pos)
/** @} */

/** @name RTCR - OWC_RTCR register */
/** @{ */
#define OWC_RTCR_SEND_DIV10_Pos                  (0)
#define OWC_RTCR_SEND_DIV10_Msk                  (0x7FUL << OWC_RTCR_SEND_DIV10_Pos)

#define OWC_RTCR_WAIT_DIV10_Pos                  (8)
#define OWC_RTCR_WAIT_DIV10_Msk                  (0x7FUL << OWC_RTCR_WAIT_DIV10_Pos)

#define OWC_RTCR_RDDLY_MIN_Pos                   (16)
#define OWC_RTCR_RDDLY_MIN_Msk                   (0x3FUL << OWC_RTCR_RDDLY_MIN_Pos)

#define OWC_RTCR_RDDLY_MAX_DIV10_Pos             (24)
#define OWC_RTCR_RDDLY_MAX_DIV10_Msk             (0x1FUL << OWC_RTCR_RDDLY_MAX_DIV10_Pos)
/** @} */

/** @name ATCR - OWC_ATCR register */
/** @{ */
#define OWC_ATCR_RECO_Pos                        (0)
#define OWC_ATCR_RECO_Msk                        (0x7UL << OWC_ATCR_RECO_Pos)

#define OWC_ATCR_SLOT_DIV10_Pos                  (8)
#define OWC_ATCR_SLOT_DIV10_Msk                  (0xFUL << OWC_ATCR_SLOT_DIV10_Pos)

#define OWC_ATCR_START_Pos                       (12)
#define OWC_ATCR_START_Msk                       (0x7UL << OWC_ATCR_START_Pos)

#define OWC_ATCR_WRDLY_Pos                       (16)
#define OWC_ATCR_WRDLY_Msk                       (0x1FUL << OWC_ATCR_WRDLY_Pos)

#define OWC_ATCR_RDDLY_Pos                       (24)
#define OWC_ATCR_RDDLY_Msk                       (0x1FUL << OWC_ATCR_RDDLY_Pos)
/** @} */

/** Peripheral OWC base pointer */
#define OWC                                      ((OWC_TypeDef *) AP_ONEWIRE_BASE_ADDR)

/**
  * @}
  */ /* end of group OWC */


/** @addtogroup PAD PAD
  * @{
  */

/** max number of addr in PAD */
#define PAD_ADDR_MAX_NUM                          (35U)

/**
  * @brief PAD register layout typedef
  *
  */
typedef struct {
    __IO uint32_t PCR[PAD_ADDR_MAX_NUM];          /**< PAD Control Register n, array offset: 0x0, array step:0x4 */
} PAD_TypeDef;

/** @name PCR - PAD_PCR register */
/** @{ */
#define PAD_PCR_MUX_Pos                                    (4)
#define PAD_PCR_MUX_Msk                                    (0x7UL << PAD_PCR_MUX_Pos)

#define PAD_PCR_PULL_UP_ENABLE_Pos                         (8)
#define PAD_PCR_PULL_UP_ENABLE_Msk                         (0x1UL << PAD_PCR_PULL_UP_ENABLE_Pos)

#define PAD_PCR_PULL_DOWN_ENABLE_Pos                       (9)
#define PAD_PCR_PULL_DOWN_ENABLE_Msk                       (0x1UL << PAD_PCR_PULL_DOWN_ENABLE_Pos)

#define PAD_PCR_PULL_SELECT_Pos                            (10)
#define PAD_PCR_PULL_SELECT_Msk                            (0x1UL << PAD_PCR_PULL_SELECT_Pos)

#define PAD_PCR_INPUT_FORCE_DISABLE_Pos                    (11)
#define PAD_PCR_INPUT_FORCE_DISABLE_Msk                    (0x1UL << PAD_PCR_INPUT_FORCE_DISABLE_Pos)

#define PAD_PCR_INPUT_CONTROL_Pos                          (13)
#define PAD_PCR_INPUT_CONTROL_Msk                          (0x1UL << PAD_PCR_INPUT_CONTROL_Pos)

#define PAD_PCR_OUTPUT_FORCE_DISABLE_Pos                   (15)
#define PAD_PCR_OUTPUT_FORCE_DISABLE_Msk                   (0x1UL << PAD_PCR_OUTPUT_FORCE_DISABLE_Pos)

#define PAD_PCR_DRIVE_STRENGTH_Pos                         (17)
#define PAD_PCR_DRIVE_STRENGTH_Msk                         (0x7UL << PAD_PCR_DRIVE_STRENGTH_Pos)

/** @} */


/** Peripheral PAD base pointer */
#define PAD                                      ((PAD_TypeDef *) GP_PAD_BASE_ADDR)

/**
  * @}
  */ /* end of group PAD */

/** @addtogroup SPI SPI
  * @{
  */

/**
  * @brief SPI register layout typedef
  *
  */
typedef struct {
    __IO uint32_t CR0;                           /**< Control Register 0,                     offset: 0x0 */
    __IO uint32_t CR1;                           /**< Control Register 1,                     offset: 0x4 */
    __IO uint32_t DR;                            /**< Data Register,                          offset: 0x8 */
    __IO uint32_t SR;                            /**< Status Register,                        offset: 0xC */
    __IO uint32_t CPSR;                          /**< Clock Prescale Register,                offset: 0x10 */
    __IO uint32_t IMSC;                          /**< Interrupt Mask Set or Clear Register,   offset: 0x14 */
    __IO uint32_t RIS;                           /**< Raw Interrupt Status Register,          offset: 0x18 */
    __IO uint32_t MIS;                           /**< Masked Interrupt Status Register,       offset: 0x1C */
    __IO uint32_t ICR;                           /**< Interrupt Clear Register,               offset: 0x20 */
    __IO uint32_t DMACR;                         /**< DMA Control Register,                   offset: 0x24 */
    __IO uint32_t DRLR;                          /**< DMA Request Level Register,             offset: 0x28 */
    __IO uint32_t DRRCR;                         /**< DMA Rx Request Control Register,        offset: 0x2C */
    __IO uint32_t DTRCR;                         /**< DMA Tx Request Control Register,        offset: 0x30 */
    __IO uint32_t RPTCR;                         /**< Rx Path Timeout Register,               offset: 0x34 */
    __IO uint32_t SRLR;                          /**< SW request level register,              offset: 0x38 */
    __IO uint32_t MPSR;                          /**< Master RXD phase sel register,          offset: 0x3C */
} SPI_TypeDef;

/** @name CR0 - SPI_CR0 register */
/** @{ */
#define SPI_CR0_DSS_Pos                          (0)
#define SPI_CR0_DSS_Msk                          (0xFUL << SPI_CR0_DSS_Pos)

#define SPI_CR0_FRF_Pos                          (4)
#define SPI_CR0_FRF_Msk                          (0x3UL << SPI_CR0_FRF_Pos)

#define SPI_CR0_SPO_Pos                          (6)
#define SPI_CR0_SPO_Msk                          (0x1UL << SPI_CR0_SPO_Pos)

#define SPI_CR0_SPH_Pos                          (7)
#define SPI_CR0_SPH_Msk                          (0x1UL << SPI_CR0_SPH_Pos)

#define SPI_CR0_SCR_Pos                          (8)
#define SPI_CR0_SCR_Msk                          (0xFFUL << SPI_CR0_SCR_Pos)
/** @} */

/** @name CR1 - SPI_CR1 register */
/** @{ */
#define SPI_CR1_LBM_Pos                          (0)
#define SPI_CR1_LBM_Msk                          (0x1UL << SPI_CR1_LBM_Pos)

#define SPI_CR1_SSE_Pos                          (1)
#define SPI_CR1_SSE_Msk                          (0x1UL << SPI_CR1_SSE_Pos)

#define SPI_CR1_MS_Pos                           (2)
#define SPI_CR1_MS_Msk                           (0x1UL << SPI_CR1_MS_Pos)

#define SPI_CR1_SOD_Pos                          (3)
#define SPI_CR1_SOD_Msk                          (0x1UL << SPI_CR1_SOD_Pos)
/** @} */

/** @name DR - SPI_DR register */
/** @{ */
#define SPI_DR_DATA_Pos                          (0)
#define SPI_DR_DATA_Msk                          (0xFFFFUL << SPI_DR_DATA_Pos)
/** @} */

/** @name SR - SPI_SR register */
/** @{ */
#define SPI_SR_TFE_Pos                           (0)
#define SPI_SR_TFE_Msk                           (0x1UL << SPI_SR_TFE_Pos)

#define SPI_SR_TNF_Pos                           (1)
#define SPI_SR_TNF_Msk                           (0x1UL << SPI_SR_TNF_Pos)

#define SPI_SR_RNE_Pos                           (2)
#define SPI_SR_RNE_Msk                           (0x1UL << SPI_SR_RNE_Pos)

#define SPI_SR_RFF_Pos                           (3)
#define SPI_SR_RFF_Msk                           (0x1UL << SPI_SR_RFF_Pos)

#define SPI_SR_BSY_Pos                           (4)
#define SPI_SR_BSY_Msk                           (0x1UL << SPI_SR_BSY_Pos)
/** @} */

/** @name CPSR - SPI_CPSR register */
/** @{ */
#define SPI_CPSR_CPSDVSR_Pos                     (0)
#define SPI_CPSR_CPSDVSR_Msk                     (0xFFUL << SPI_CPSR_CPSDVSR_Pos)
/** @} */

/** @name IMSC - SPI_IMSC register */
/** @{ */
#define SPI_IMSC_RORIM_Pos                       (0)
#define SPI_IMSC_RORIM_Msk                       (0x1UL << SPI_IMSC_RORIM_Pos)

#define SPI_IMSC_RTIM_Pos                        (1)
#define SPI_IMSC_RTIM_Msk                        (0x1UL << SPI_IMSC_RTIM_Pos)

#define SPI_IMSC_RXIM_Pos                        (2)
#define SPI_IMSC_RXIM_Msk                        (0x1UL << SPI_IMSC_RXIM_Pos)

#define SPI_IMSC_TXIM_Pos                        (3)
#define SPI_IMSC_TXIM_Msk                        (0x1UL << SPI_IMSC_TXIM_Pos)
/** @} */

/** @name RIS - SPI_RIS register */
/** @{ */
#define SPI_RIS_RORRIS_Pos                       (0)
#define SPI_RIS_RORRIS_Msk                       (0x1UL << SPI_RIS_RORRIS_Pos)

#define SPI_RIS_RTRIS_Pos                        (1)
#define SPI_RIS_RTRIS_Msk                        (0x1UL << SPI_RIS_RTRIS_Pos)

#define SPI_RIS_RXRIS_Pos                        (2)
#define SPI_RIS_RXRIS_Msk                        (0x1UL << SPI_RIS_RXRIS_Pos)

#define SPI_RIS_TXRIS_Pos                        (3)
#define SPI_RIS_TXRIS_Msk                        (0x1UL << SPI_RIS_TXRIS_Pos)
/** @} */

/** @name MIS - SPI_MIS register */
/** @{ */
#define SPI_MIS_RORMIS_Pos                       (0)
#define SPI_MIS_RORMIS_Msk                       (0x1UL << SPI_MIS_RORMIS_Pos)

#define SPI_MIS_RTMIS_Pos                        (1)
#define SPI_MIS_RTMIS_Msk                        (0x1UL << SPI_MIS_RTMIS_Pos)

#define SPI_MIS_RXMIS_Pos                        (2)
#define SPI_MIS_RXMIS_Msk                        (0x1UL << SPI_MIS_RXMIS_Pos)

#define SPI_MIS_TXMIS_Pos                        (3)
#define SPI_MIS_TXMIS_Msk                        (0x1UL << SPI_MIS_TXMIS_Pos)
/** @} */

/** @name ICR - SPI_ICR register */
/** @{ */
#define SPI_ICR_RORIC_Pos                        (0)
#define SPI_ICR_RORIC_Msk                        (0x1UL << SPI_ICR_RORIC_Pos)

#define SPI_ICR_RTIC_Pos                         (1)
#define SPI_ICR_RTIC_Msk                         (0x1UL << SPI_ICR_RTIC_Pos)
/** @} */

/** @name DMACR - SPI_DMACR register */
/** @{ */
#define SPI_DMACR_RXDMAE_Pos                     (0)
#define SPI_DMACR_RXDMAE_Msk                     (0x1UL << SPI_DMACR_RXDMAE_Pos)

#define SPI_DMACR_TXDMAE_Pos                     (1)
#define SPI_DMACR_TXDMAE_Msk                     (0x1UL << SPI_DMACR_TXDMAE_Pos)
/** @} */

/** @name DRLR - SPI_DRLR register */
/** @{ */
#define SPI_DRLR_RX_REQ_LVL_Pos                  (0)
#define SPI_DRLR_RX_REQ_LVL_Msk                  (0x1FUL << SPI_DRLR_RX_REQ_LVL_Pos)

#define SPI_DRLR_TX_REQ_LVL_Pos                  (8)
#define SPI_DRLR_TX_REQ_LVL_Msk                  (0x1FUL << SPI_DRLR_TX_REQ_LVL_Pos)
/** @} */

/** @name DRRCR - SPI_DRRCR register */
/** @{ */
#define SPI_DRRCR_RX_WAIT_CYCLE_Pos              (0)
#define SPI_DRRCR_RX_WAIT_CYCLE_Msk              (0x1FUL << SPI_DRRCR_RX_WAIT_CYCLE_Pos)

#define SPI_DRRCR_READ_DEPTH_ONE_BURST_Pos       (8)
#define SPI_DRRCR_READ_DEPTH_ONE_BURST_Msk       (0xFUL << SPI_DRRCR_READ_DEPTH_ONE_BURST_Pos)

#define SPI_DRRCR_RX_REQ_MODE_Pos                (13)
#define SPI_DRRCR_RX_REQ_MODE_Msk                (0x1UL << SPI_DRRCR_RX_REQ_MODE_Pos)

#define SPI_DRRCR_RX_TO_REQ_EN_Pos               (14)
#define SPI_DRRCR_RX_TO_REQ_EN_Msk               (0x1UL << SPI_DRRCR_RX_TO_REQ_EN_Pos)

#define SPI_DRRCR_RX_EOR_MODE_Pos                (15)
#define SPI_DRRCR_RX_EOR_MODE_Msk                (0x1UL << SPI_DRRCR_RX_EOR_MODE_Pos)
/** @} */

/** @name DTRCR - SPI_DTRCR register */
/** @{ */
#define SPI_DTRCR_TX_WAIT_CYCLE_Pos              (0)
#define SPI_DTRCR_TX_WAIT_CYCLE_Msk              (0x1FUL << SPI_DTRCR_TX_WAIT_CYCLE_Pos)

#define SPI_DTRCR_WRITE_DEPTH_ONE_BURST_Pos      (8)
#define SPI_DTRCR_WRITE_DEPTH_ONE_BURST_Msk      (0xFUL << SPI_DTRCR_WRITE_DEPTH_ONE_BURST_Pos)

#define SPI_DTRCR_TX_REQ_MODE_Pos                (13)
#define SPI_DTRCR_TX_REQ_MODE_Msk                (0x1UL << SPI_DTRCR_TX_REQ_MODE_Pos)
/** @} */

/** @name RPTCR - SPI_RPTCR register */
/** @{ */
#define SPI_RPTCR_VALUE_Pos                      (0)
#define SPI_RPTCR_VALUE_Msk                      (0xFFUL << SPI_RPTCR_VALUE_Pos)
/** @} */

/** @name SRLR - SPI_SRLR register */
/** @{ */
#define SPI_SRLR_RX_REQ_LVL_Pos                  (0)
#define SPI_SRLR_RX_REQ_LVL_Msk                  (0x1FUL << SPI_SRLR_RX_REQ_LVL_Pos)

#define SPI_SRLR_TX_REQ_LVL_Pos                  (8)
#define SPI_SRLR_TX_REQ_LVL_Msk                  (0x1FUL << SPI_SRLR_TX_REQ_LVL_Pos)
/** @} */

/** @name MPSR - SPI_MPSR register */
/** @{ */
#define SPI_MPSR_VALUE_Pos                       (0)
#define SPI_MPSR_VALUE_Msk                       (0x1UL << SPI_MPSR_VALUE_Pos)
/** @} */


/** Peripheral SPI0 base pointer */
#define SPI0                                     ((SPI_TypeDef *)MP_SSP0_BASE_ADDR)

/** @brief SPI peripheral instance number */
#define SPI_INSTANCE_NUM                         (1)

/** @brief SPI peripheral instance array */
#define SPI_INSTANCE_ARRAY                       {SPI0}

/**
  * @}
  */ /* end of group SPI */

/** @addtogroup TIMER Timer
  * @{
  */

/**
  * @brief TIMER register layout typedef
  *
  */
typedef struct {
    __IO uint32_t TCCR;                          /**< Timer Clock Control Register,                offset: 0x0 */
    __IO uint32_t TCTLR;                         /**< Timer Control Register,                      offset: 0x4 */
    __IO uint32_t TSR;                           /**< Timer Status Register,                       offset: 0x8 */
    __IO uint32_t TIVR;                          /**< Timer Init Value Register,                   offset: 0xC */
    __IO uint32_t TMR[3];                        /**< Timer Match N Register, array offset: 0x10, array step: 0x4 */
    __I  uint32_t TCR;                           /**< Timer Counter Register,                      offset: 0x1C */
    __IO uint32_t TCLR;                          /**< Timer Counter Latch Register,                offset: 0x20 */
    __I  uint32_t TACR;                          /**< Timer Actual value of Counter Register       offset: 0x24 */
    __I  uint32_t TCVR;                          /**< Timer Capture Value Register                 offset: 0x28 */
    __I  uint32_t TCER;                          /**< Timer Capture Edge Register                  offset: 0x2C */
} TIMER_TypeDef;

/** @name TCCR - TIMER_TCCR register */
/** @{ */
#define TIMER_TCCR_ENABLE_Pos                    (0)
#define TIMER_TCCR_ENABLE_Msk                    (0x1UL << TIMER_TCCR_ENABLE_Pos)
/** @} */

/** @name TCTLR - TIMER_TCTLR register */
/** @{ */
#define TIMER_TCTLR_MODE_Pos                     (0)
#define TIMER_TCTLR_MODE_Msk                     (0x1UL << TIMER_TCTLR_MODE_Pos)

#define TIMER_TCTLR_MCS_Pos                      (1)
#define TIMER_TCTLR_MCS_Msk                      (0x3UL << TIMER_TCTLR_MCS_Pos)

#define TIMER_TCTLR_IE_0_Pos                     (3)
#define TIMER_TCTLR_IE_0_Msk                     (0x1UL << TIMER_TCTLR_IE_0_Pos)

#define TIMER_TCTLR_IE_1_Pos                     (4)
#define TIMER_TCTLR_IE_1_Msk                     (0x1UL << TIMER_TCTLR_IE_1_Pos)

#define TIMER_TCTLR_IE_2_Pos                     (5)
#define TIMER_TCTLR_IE_2_Msk                     (0x1UL << TIMER_TCTLR_IE_1_Pos)

#define TIMER_TCTLR_IT_0_Pos                     (6)
#define TIMER_TCTLR_IT_0_Msk                     (0x1UL << TIMER_TCTLR_IT_0_Pos)

#define TIMER_TCTLR_IT_1_Pos                     (7)
#define TIMER_TCTLR_IT_1_Msk                     (0x1UL << TIMER_TCTLR_IT_1_Pos)

#define TIMER_TCTLR_IT_2_Pos                     (8)
#define TIMER_TCTLR_IT_2_Msk                     (0x1UL << TIMER_TCTLR_IT_2_Pos)

#define TIMER_TCTLR_PWMOUT_Pos                   (9)
#define TIMER_TCTLR_PWMOUT_Msk                   (0x1UL << TIMER_TCTLR_PWMOUT_Pos)

#define TIMER_TCTLR_CAPTURE_EN_Pos               (10)
#define TIMER_TCTLR_CAPTURE_EN_Msk               (0x1UL << TIMER_TCTLR_CAPTURE_EN_Pos)

#define TIMER_TCTLR_CAPTURE_IT_Pos               (11)
#define TIMER_TCTLR_CAPTURE_IT_Msk               (0x1UL << TIMER_TCTLR_CAPTURE_IT_Pos)

#define TIMER_TCTLR_EDGE_TYPE_Pos                (12)
#define TIMER_TCTLR_EDGE_TYPE_Msk                (0x3UL << TIMER_TCTLR_EDGE_TYPE_Pos)

#define TIMER_TCTLR_PWM_STOP_VALUE_Pos           (14)
#define TIMER_TCTLR_PWM_STOP_VALUE_Msk           (0x3UL << TIMER_TCTLR_PWM_STOP_VALUE_Pos)
/** @} */

/** @name TSR - TIMER_TSR register */
/** @{ */
#define TIMER_TSR_ICLR_0_Pos                     (0)
#define TIMER_TSR_ICLR_0_Msk                     (0x1UL << TIMER_TSR_ICLR_0_Pos)

#define TIMER_TSR_ICLR_1_Pos                     (1)
#define TIMER_TSR_ICLR_1_Msk                     (0x1UL << TIMER_TSR_ICLR_1_Pos)

#define TIMER_TSR_ICLR_2_Pos                     (2)
#define TIMER_TSR_ICLR_2_Msk                     (0x1UL << TIMER_TSR_ICLR_2_Pos)

#define TIMER_TSR_CAPTURE_Pos                    (3)
#define TIMER_TSR_CAPTURE_Msk                    (0x1UL << TIMER_TSR_CAPTURE_Pos)
/** @} */

/** @name TIVR - TIMER_TIVR register */
/** @{ */
#define TIMER_TIVR_VALUE_Pos                     (0)
#define TIMER_TIVR_VALUE_Msk                     (0xFFFFFFFFUL << TIMER_TIVR_VALUE_Pos)
/** @} */

/** @name TMR - TIMER_TMR N register */
/** @{ */
#define TIMER_TMR_MATCH_Pos                      (0)
#define TIMER_TMR_MATCH_Msk                      (0xFFFFFFFFUL << TIMER_TMR_MATCH_Pos)
/** @} */

/** @name TCR - TIMER_TCR register */
/** @{ */
#define TIMER_TCR_VALUE_Pos                      (0)
#define TIMER_TCR_VALUE_Msk                      (0xFFFFFFFFUL << TIMER_TCR_VALUE_Pos)
/** @} */

/** @name TCLR - TIMER_TCLR register */
/** @{ */
#define TIMER_TCLR_LATCH_Pos                      (0)
#define TIMER_TCLR_LATCH_Msk                      (0x1UL << TIMER_TCLR_LATCH_Pos)
/** @} */

/** @name TACR - TIMER_TACR register */
/** @{ */
#define TIMER_TACR_VALUE_Pos                      (0)
#define TIMER_TACR_VALUE_Msk                      (0xFFFFFFFFUL << TIMER_TACR_VALUE_Pos)
/** @} */

/** @name TCVR - TIMER_TCVR register */
/** @{ */
#define TIMER_TCVR_VALUE_Pos                      (0)
#define TIMER_TCVR_VALUE_Msk                      (0xFFFFFFFFUL << TIMER_TCVR_VALUE_Pos)
/** @} */

/** @name TCER - TIMER_TCER register */
/** @{ */
#define TIMER_TCER_VALUE_Pos                      (0)
#define TIMER_TCER_VALUE_Msk                      (0xFFFFFFFFUL << TIMER_TCER_VALUE_Pos)
/** @} */

/** @brief TIMER peripheral instance number */
#define TIMER_INSTANCE_NUM                       (6)

/**
  * @}
  */ /* end of group TIMER */


/** @addtogroup USART USART
  * @{
  */

/**
  * @brief USART register layout typedef
  *
  */
typedef struct _USART_TypeDef {
    __IO uint32_t ENR;                           /**< Usart Enable Register,              offset: 0x0*/
    __IO uint32_t DIVR;                          /**< Usart Divder Register,              offset: 0x4*/
    __IO uint32_t LCR;                           /**< Usart Line Control Register,        offset: 0x8*/
    __IO uint32_t RXSR;                          /**< Usart Rx Select Register,           offset: 0xC*/
    __IO uint32_t FCR0;                          /**< Usart Fifo Control 0 Register,      offset: 0x10*/
    __IO uint32_t MCR;                           /**< Usart Mode Control Register,        offset: 0x14*/
    __IO uint32_t ABSR;                          /**< Usart AutoBaud Set Register,        offset: 0x18*/
    __IO uint32_t ABCR;                          /**< Usart AutoBaud Clear Register,      offset: 0x1C*/
    __IO uint32_t BSR;                           /**< Usart Break Set Register,           offset: 0x20*/
    __IO uint32_t ISCR;                          /**< Usart Irda Sir Register,            offset: 0x24*/
    __IO uint32_t IER;                           /**< Usart Interrupt Enable Register,    offset: 0x28*/
    __IO uint32_t ISR;                           /**< Usart Interrupt Status Register,    offset: 0x2C*/
    __IO uint32_t ICR;                           /**< Usart Interrupt Clear Register,     offset: 0x30*/
    __IO uint32_t FLOWCR;                        /**< Usart Flow Control Register,        offset: 0x34*/
    __IO uint32_t TDR;                           /**< Usart Tx Data Register,             offset: 0x38*/
    __IO uint32_t RDR;                           /**< Usart Rx Data Register,             offset: 0x3C*/
    __IO uint32_t HCR;                           /**< Usart Hw Control Register,          offset: 0x40*/
    __IO uint32_t LSR;                           /**< Usart Line status Register,         offset: 0x44*/
    __I  uint32_t DIVIR;                         /**< Usart divder Inner Register,        offset: 0x48*/
    __I  uint32_t FSR;                           /**< Usart Fifo Status Register,         offset: 0x4C*/
    __IO uint32_t TCR;                           /**< Usart Timeout Control Register,     offset: 0x50*/
    __I  uint32_t DEBUGR;                        /**< Usart Debug Register,               offset: 0x54*/
    __IO uint32_t FCR1;                          /**< Usart Fifo Control 1 Register,      offset: 0x58*/
} USART_TypeDef;

/** @name ENR - USART_ENR register */
/** @{ */
#define USART_ENR_TX_EN_Pos                      (0)
#define USART_ENR_TX_EN_Msk                      (0x1UL << USART_ENR_TX_EN_Pos)

#define USART_ENR_RX_EN_Pos                      (1)
#define USART_ENR_RX_EN_Msk                      (0x1UL << USART_ENR_RX_EN_Pos)
/** @} */

/** @name DIVR - USART_DIVR register */
/** @{ */
#define USART_DIVR_FRAC_Pos                      (0)
#define USART_DIVR_FRAC_Msk                      (0xFUL << USART_DIVR_FRAC_Pos)

#define USART_DIVR_INT_Pos                       (4)
#define USART_DIVR_INT_Msk                       (0x3FFFFUL << USART_DIVR_INT_Pos)
/** @} */

/** @name LCR - USART_LCR register */
/** @{ */
#define USART_LCR_CHAR_LEN_Pos                   (0)
#define USART_LCR_CHAR_LEN_Msk                   (0x3UL << USART_LCR_CHAR_LEN_Pos)

#define USART_LCR_STOP_BIT_NUM_Pos               (2)
#define USART_LCR_STOP_BIT_NUM_Msk               (0x3UL << USART_LCR_STOP_BIT_NUM_Pos)

#define USART_LCR_PARITY_EN_Pos                  (4)
#define USART_LCR_PARITY_EN_Msk                  (0x1UL << USART_LCR_PARITY_EN_Pos)

#define USART_LCR_EVEN_PARITY_Pos                (5)
#define USART_LCR_EVEN_PARITY_Msk                (0x1UL << USART_LCR_EVEN_PARITY_Pos)

#define USART_LCR_STICKY_PARITY_Pos              (6)
#define USART_LCR_STICKY_PARITY_Msk              (0x1UL << USART_LCR_STICKY_PARITY_Pos)

#define USART_LCR_RX_TO_BIT_NUM_Pos              (7)
#define USART_LCR_RX_TO_BIT_NUM_Msk              (0x3FUL << USART_LCR_RX_TO_BIT_NUM_Pos)

#define USART_LCR_TX_BREAK_LEN_DEC10_Pos         (13)
#define USART_LCR_TX_BREAK_LEN_DEC10_Msk         (0x7UL << USART_LCR_TX_BREAK_LEN_DEC10_Pos)
/** @} */

/** @name RXSR - USART_RXSR register */
/** @{ */
#define USART_RXSR_SEL_Pos                       (0)
#define USART_RXSR_SEL_Msk                       (0x1UL << USART_RXSR_SEL_Pos)
/** @} */

/** @name FCR0 - USART_FCR0 register */
/** @{ */
#define USART_FCR0_TXFIFO_TH_Pos                  (0)
#define USART_FCR0_TXFIFO_TH_Msk                  (0x1FUL << USART_FCR0_TXFIFO_TH_Pos)

#define USART_FCR0_RXFIFO_TH_Pos                  (5)
#define USART_FCR0_RXFIFO_TH_Msk                  (0x7FUL << USART_FCR0_RXFIFO_TH_Pos)

#define USART_FCR0_TXFIFO_FLUSH_Pos               (12)
#define USART_FCR0_TXFIFO_FLUSH_Msk               (0x1UL << USART_FCR0_TXFIFO_FLUSH_Pos)

#define USART_FCR0_RXFIFO_FLUSH_Pos               (13)
#define USART_FCR0_RXFIFO_FLUSH_Msk               (0x1UL << USART_FCR0_RXFIFO_FLUSH_Pos)

#define USART_FCR0_RXFIFO_TO_BIT_NUM_Pos          (14)
#define USART_FCR0_RXFIFO_TO_BIT_NUM_Msk          (0x3FUL << USART_FCR0_RXFIFO_TO_BIT_NUM_Pos)

#define USART_FCR0_RXFIFO_TO_DMA_REQ_EN_Pos       (20)
#define USART_FCR0_RXFIFO_TO_DMA_REQ_EN_Msk       (0x1UL << USART_FCR0_RXFIFO_TO_DMA_REQ_EN_Pos)
/** @} */

/** @name MCR - USART_MCR register */
/** @{ */
#define USART_MCR_TX_DMA_EN_Pos                  (0)
#define USART_MCR_TX_DMA_EN_Msk                  (0x1UL << USART_MCR_TX_DMA_EN_Pos)

#define USART_MCR_RX_DMA_EN_Pos                  (1)
#define USART_MCR_RX_DMA_EN_Msk                  (0x1UL << USART_MCR_RX_DMA_EN_Pos)

#define USART_MCR_AUTO_FLOW_RTS_EN_Pos           (2)
#define USART_MCR_AUTO_FLOW_RTS_EN_Msk           (0x1UL << USART_MCR_AUTO_FLOW_RTS_EN_Pos)

#define USART_MCR_AUTO_FLOW_CTS_EN_Pos           (3)
#define USART_MCR_AUTO_FLOW_CTS_EN_Msk           (0x1UL << USART_MCR_AUTO_FLOW_CTS_EN_Pos)

#define USART_MCR_LOOPBACK_MODE_Pos              (4)
#define USART_MCR_LOOPBACK_MODE_Msk              (0x1UL << USART_MCR_LOOPBACK_MODE_Pos)

#define USART_MCR_TX_SIR_MODE_Pos                (5)
#define USART_MCR_TX_SIR_MODE_Msk                (0x1UL << USART_MCR_TX_SIR_MODE_Pos)

#define USART_MCR_RX_SIR_MODE_Pos                (6)
#define USART_MCR_RX_SIR_MODE_Msk                (0x1UL << USART_MCR_RX_SIR_MODE_Pos)

#define USART_MCR_TX_POLARITY_Pos                (7)
#define USART_MCR_TX_POLARITY_Msk                (0x1UL << USART_MCR_TX_POLARITY_Pos)

#define USART_MCR_RX_POLARITY_Pos                (8)
#define USART_MCR_RX_POLARITY_Msk                (0x1UL << USART_MCR_RX_POLARITY_Pos)

#define USART_MCR_AUTOBAUD_WAIT_IDLE_EN_Pos      (9)
#define USART_MCR_AUTOBAUD_WAIT_IDLE_EN_Msk      (0x1UL << USART_MCR_AUTOBAUD_WAIT_IDLE_EN_Pos)
/** @} */

/** @name ABSR - USART_ABSR register */
/** @{ */
#define USART_ABSR_SET_Pos                       (0)
#define USART_ABSR_SET_Msk                       (0x1UL << USART_ABSR_SET_Pos)
/** @} */

/** @name ABCR - USART_ABCR register */
/** @{ */
#define USART_ABCR_CLR_Pos                       (0)
#define USART_ABCR_CLR_Msk                       (0x1UL << USART_ABCR_CLR_Pos)
/** @} */

/** @name BSR - USART_BSR register */
/** @{ */
#define USART_BSR_SET_Pos                        (0)
#define USART_BSR_SET_Msk                        (0x1UL << USART_BSR_SET_Pos)
/** @} */

/** @name ISCR - USART_ISCR register */
/** @{ */
#define USART_ISCR_SIR_LOW_POWER_Pos             (0)
#define USART_ISCR_SIR_LOW_POWER_Msk             (0x1UL << USART_ISCR_SIR_LOW_POWER_Pos)

#define USART_ISCR_DIV_SIR_Pos                   (1)
#define USART_ISCR_DIV_SIR_Msk                   (0x1UL << USART_ISCR_DIV_SIR_Pos)
/** @} */

/** @name IER - USART_IER register */
/** @{ */
#define USART_IER_TXFIFO_WL_Pos                  (0)
#define USART_IER_TXFIFO_WL_Msk                  (0x1UL << USART_IER_TXFIFO_WL_Pos)

#define USART_IER_TX_TRANS_DONE_Pos              (1)
#define USART_IER_TX_TRANS_DONE_Msk              (0x1UL << USART_IER_TX_TRANS_DONE_Pos)

#define USART_IER_RXFIFO_WL_Pos                  (2)
#define USART_IER_RXFIFO_WL_Msk                  (0x1UL << USART_IER_RXFIFO_WL_Pos)

#define USART_IER_RXFIFO_TO_Pos                  (3)
#define USART_IER_RXFIFO_TO_Msk                  (0x1UL << USART_IER_RXFIFO_TO_Pos)

#define USART_IER_RX_START_Pos                   (4)
#define USART_IER_RX_START_Msk                   (0x1UL << USART_IER_RX_START_Pos)

#define USART_IER_RX_TO_Pos                      (5)
#define USART_IER_RX_TO_Msk                      (0x1UL << USART_IER_RX_TO_Pos)

#define USART_IER_TX_BREAK_DONE_Pos              (6)
#define USART_IER_TX_BREAK_DONE_Msk              (0x1UL << USART_IER_TX_BREAK_DONE_Pos)

#define USART_IER_RX_BREAK_DET_Pos               (7)
#define USART_IER_RX_BREAK_DET_Msk               (0x1UL << USART_IER_RX_BREAK_DET_Pos)

#define USART_IER_CTS_TOGGLE_Pos                 (8)
#define USART_IER_CTS_TOGGLE_Msk                 (0x1UL << USART_IER_CTS_TOGGLE_Pos)

#define USART_IER_DCD_TOGGLE_Pos                 (9)
#define USART_IER_DCD_TOGGLE_Msk                 (0x1UL << USART_IER_DCD_TOGGLE_Pos)

#define USART_IER_AUTOBAUD_DONE_Pos              (10)
#define USART_IER_AUTOBAUD_DONE_Msk              (0x1UL << USART_IER_AUTOBAUD_DONE_Pos)

#define USART_IER_TXFIFO_OF_Pos                  (11)
#define USART_IER_TXFIFO_OF_Msk                  (0x1UL << USART_IER_TXFIFO_OF_Pos)

#define USART_IER_RXFIFO_OF_Pos                  (12)
#define USART_IER_RXFIFO_OF_Msk                  (0x1UL << USART_IER_RXFIFO_OF_Pos)

#define USART_IER_RXFIFO_UF_Pos                  (13)
#define USART_IER_RXFIFO_UF_Msk                  (0x1UL << USART_IER_RXFIFO_UF_Pos)

#define USART_IER_RX_FRAME_ERR_Pos               (14)
#define USART_IER_RX_FRAME_ERR_Msk               (0x1UL << USART_IER_RX_FRAME_ERR_Pos)

#define USART_IER_RX_PARITY_ERR_Pos              (15)
#define USART_IER_RX_PARITY_ERR_Msk              (0x1UL << USART_IER_RX_PARITY_ERR_Pos)

#define USART_IER_LPUART_RX_FRAME_ERR_Pos        (16)
#define USART_IER_LPUART_RX_FRAME_ERR_Msk        (0x1UL << USART_IER_LPUART_RX_FRAME_ERR_Pos)

#define USART_IER_LPUART_PARITY_ERR_Pos          (17)
#define USART_IER_LPUART_PARITY_ERR_Msk          (0x1UL << USART_IER_LPUART_PARITY_ERR_Pos)

#define USART_IER_LPUART_RXFIFO_OF_Pos           (18)
#define USART_IER_LPUART_RXFIFO_OF_Msk           (0x1UL << USART_IER_LPUART_RXFIFO_OF_Pos)

#define USART_IER_LPUART_AUTOBAUD_DONE_Pos       (19)
#define USART_IER_LPUART_AUTOBAUD_DONE_Msk       (0x1UL << USART_IER_LPUART_AUTOBAUD_DONE_Pos)
/** @} */

/** @name ISR - USART_ISR register */
/** @{ */
#define USART_ISR_TXFIFO_WL_Pos                  (0)
#define USART_ISR_TXFIFO_WL_Msk                  (0x1UL << USART_ISR_TXFIFO_WL_Pos)

#define USART_ISR_TX_TRANS_DONE_Pos              (1)
#define USART_ISR_TX_TRANS_DONE_Msk              (0x1UL << USART_ISR_TX_TRANS_DONE_Pos)

#define USART_ISR_RXFIFO_WL_Pos                  (2)
#define USART_ISR_RXFIFO_WL_Msk                  (0x1UL << USART_ISR_RXFIFO_WL_Pos)

#define USART_ISR_RXFIFO_TO_Pos                  (3)
#define USART_ISR_RXFIFO_TO_Msk                  (0x1UL << USART_ISR_RXFIFO_TO_Pos)

#define USART_ISR_RX_START_Pos                   (4)
#define USART_ISR_RX_START_Msk                   (0x1UL << USART_ISR_RX_START_Pos)

#define USART_ISR_RX_TO_Pos                      (5)
#define USART_ISR_RX_TO_Msk                      (0x1UL << USART_ISR_RX_TO_Pos)

#define USART_ISR_TX_BREAK_DONE_Pos              (6)
#define USART_ISR_TX_BREAK_DONE_Msk              (0x1UL << USART_ISR_TX_BREAK_DONE_Pos)

#define USART_ISR_RX_BREAK_DET_Pos               (7)
#define USART_ISR_RX_BREAK_DET_Msk               (0x1UL << USART_ISR_RX_BREAK_DET_Pos)

#define USART_ISR_CTS_TOGGLE_Pos                 (8)
#define USART_ISR_CTS_TOGGLE_Msk                 (0x1UL << USART_ISR_CTS_TOGGLE_Pos)

#define USART_ISR_DCD_TOGGLE_Pos                 (9)
#define USART_ISR_DCD_TOGGLE_Msk                 (0x1UL << USART_ISR_DCD_TOGGLE_Pos)

#define USART_ISR_AUTOBAUD_DONE_Pos              (10)
#define USART_ISR_AUTOBAUD_DONE_Msk              (0x1UL << USART_ISR_AUTOBAUD_DONE_Pos)

#define USART_ISR_TXFIFO_OF_Pos                  (11)
#define USART_ISR_TXFIFO_OF_Msk                  (0x1UL << USART_ISR_TXFIFO_OF_Pos)

#define USART_ISR_RXFIFO_OF_Pos                  (12)
#define USART_ISR_RXFIFO_OF_Msk                  (0x1UL << USART_ISR_RXFIFO_OF_Pos)

#define USART_ISR_RXFIFO_UF_Pos                  (13)
#define USART_ISR_RXFIFO_UF_Msk                  (0x1UL << USART_ISR_RXFIFO_UF_Pos)

#define USART_ISR_RX_FRAME_ERR_Pos               (14)
#define USART_ISR_RX_FRAME_ERR_Msk               (0x1UL << USART_ISR_RX_FRAME_ERR_Pos)

#define USART_ISR_RX_PARITY_ERR_Pos              (15)
#define USART_ISR_RX_PARITY_ERR_Msk              (0x1UL << USART_ISR_RX_PARITY_ERR_Pos)

#define USART_ISR_LPUART_RX_FRAME_ERR_Pos        (16)
#define USART_ISR_LPUART_RX_FRAME_ERR_Msk        (0x1UL << USART_ISR_LPUART_RX_FRAME_ERR_Pos)

#define USART_ISR_LPUART_PARITY_ERR_Pos          (17)
#define USART_ISR_LPUART_PARITY_ERR_Msk          (0x1UL << USART_ISR_LPUART_PARITY_ERR_Pos)

#define USART_ISR_LPUART_RXFIFO_OF_Pos           (18)
#define USART_ISR_LPUART_RXFIFO_OF_Msk           (0x1UL << USART_ISR_LPUART_RXFIFO_OF_Pos)

#define USART_ISR_LPUART_AUTOBAUD_DONE_Pos       (19)
#define USART_ISR_LPUART_AUTOBAUD_DONE_Msk       (0x1UL << USART_ISR_LPUART_AUTOBAUD_DONE_Pos)
/** @} */

/** @name ICR - USART_ICR register */
/** @{ */
#define USART_ICR_TXFIFO_WL_Pos                  (0)
#define USART_ICR_TXFIFO_WL_Msk                  (0x1UL << USART_ICR_TXFIFO_WL_Pos)

#define USART_ICR_TX_TRANS_DONE_Pos              (1)
#define USART_ICR_TX_TRANS_DONE_Msk              (0x1UL << USART_ICR_TX_TRANS_DONE_Pos)

#define USART_ICR_RXFIFO_WL_Pos                  (2)
#define USART_ICR_RXFIFO_WL_Msk                  (0x1UL << USART_ICR_RXFIFO_WL_Pos)

#define USART_ICR_RXFIFO_TO_Pos                  (3)
#define USART_ICR_RXFIFO_TO_Msk                  (0x1UL << USART_ICR_RXFIFO_TO_Pos)

#define USART_ICR_RX_START_Pos                   (4)
#define USART_ICR_RX_START_Msk                   (0x1UL << USART_ICR_RX_START_Pos)

#define USART_ICR_RX_TO_Pos                      (5)
#define USART_ICR_RX_TO_Msk                      (0x1UL << USART_ICR_RX_TO_Pos)

#define USART_ICR_TX_BREAK_DONE_Pos              (6)
#define USART_ICR_TX_BREAK_DONE_Msk              (0x1UL << USART_ICR_TX_BREAK_DONE_Pos)

#define USART_ICR_RX_BREAK_DET_Pos               (7)
#define USART_ICR_RX_BREAK_DET_Msk               (0x1UL << USART_ICR_RX_BREAK_DET_Pos)

#define USART_ICR_CTS_TOGGLE_Pos                 (8)
#define USART_ICR_CTS_TOGGLE_Msk                 (0x1UL << USART_ICR_CTS_TOGGLE_Pos)

#define USART_ICR_DCD_TOGGLE_Pos                 (9)
#define USART_ICR_DCD_TOGGLE_Msk                 (0x1UL << USART_ICR_DCD_TOGGLE_Pos)

#define USART_ICR_AUTOBAUD_DONE_Pos              (10)
#define USART_ICR_AUTOBAUD_DONE_Msk              (0x1UL << USART_ICR_AUTOBAUD_DONE_Pos)

#define USART_ICR_TXFIFO_OF_Pos                  (11)
#define USART_ICR_TXFIFO_OF_Msk                  (0x1UL << USART_ICR_TXFIFO_OF_Pos)

#define USART_ICR_RXFIFO_OF_Pos                  (12)
#define USART_ICR_RXFIFO_OF_Msk                  (0x1UL << USART_ICR_RXFIFO_OF_Pos)

#define USART_ICR_RXFIFO_UF_Pos                  (13)
#define USART_ICR_RXFIFO_UF_Msk                  (0x1UL << USART_ICR_RXFIFO_UF_Pos)

#define USART_ICR_RX_FRAME_ERR_Pos               (14)
#define USART_ICR_RX_FRAME_ERR_Msk               (0x1UL << USART_ICR_RX_FRAME_ERR_Pos)

#define USART_ICR_RX_PARITY_ERR_Pos              (15)
#define USART_ICR_RX_PARITY_ERR_Msk              (0x1UL << USART_ICR_RX_PARITY_ERR_Pos)

#define USART_ICR_LPUART_RX_FRAME_ERR_Pos        (16)
#define USART_ICR_LPUART_RX_FRAME_ERR_Msk        (0x1UL << USART_ICR_LPUART_RX_FRAME_ERR_Pos)

#define USART_ICR_LPUART_PARITY_ERR_Pos          (17)
#define USART_ICR_LPUART_PARITY_ERR_Msk          (0x1UL << USART_ICR_LPUART_PARITY_ERR_Pos)

#define USART_ICR_LPUART_RXFIFO_OF_Pos           (18)
#define USART_ICR_LPUART_RXFIFO_OF_Msk           (0x1UL << USART_ICR_LPUART_RXFIFO_OF_Pos)

#define USART_ICR_LPUART_AUTOBAUD_DONE_Pos       (19)
#define USART_ICR_LPUART_AUTOBAUD_DONE_Msk       (0x1UL << USART_ICR_LPUART_AUTOBAUD_DONE_Pos)
/** @} */

/** @name FLOWCR - USART_FLOWCR register */
/** @{ */
#define USART_FLOWCR_RTS_Pos                     (0)
#define USART_FLOWCR_RTS_Msk                     (0x1UL << USART_FLOWCR_RTS_Pos)

#define USART_FLOWCR_DTR_Pos                     (1)
#define USART_FLOWCR_DTR_Msk                     (0x1UL << USART_FLOWCR_DTR_Pos)

#define USART_FLOWCR_CTS_Pos                     (2)
#define USART_FLOWCR_CTS_Msk                     (0x1UL << USART_FLOWCR_CTS_Pos)

#define USART_FLOWCR_DCD_Pos                     (3)
#define USART_FLOWCR_DCD_Msk                     (0x1UL << USART_FLOWCR_DCD_Pos)

#define USART_FLOWCR_RTS_TH_Pos                  (4)
#define USART_FLOWCR_RTS_TH_Msk                  (0x7FUL << USART_FLOWCR_RTS_TH_Pos)
/** @} */

/** @name TDR - USART_TDR register */
/** @{ */
#define USART_TDR_DATA_Pos                       (0)
#define USART_TDR_DATA_Msk                       (0xFFUL << USART_TDR_DATA_Pos)
/** @} */

/** @name RDR - USART_RDR register */
/** @{ */
#define USART_RDR_DATA_Pos                       (0)
#define USART_RDR_DATA_Msk                       (0xFFUL << USART_RDR_DATA_Pos)
/** @} */

/** @name HCR - USART_HCR register */
/** @{ */
#define USART_HCR_GLITCH_FILTER_Pos              (0)
#define USART_HCR_GLITCH_FILTER_Msk              (0x1UL << USART_HCR_GLITCH_FILTER_Pos)

#define USART_HCR_RX_STOPBIT_CHK_Pos             (1)
#define USART_HCR_RX_STOPBIT_CHK_Msk             (0x1UL << USART_HCR_RX_STOPBIT_CHK_Pos)

#define USART_HCR_TX_USE_DIV_FRAC_Pos            (2)
#define USART_HCR_TX_USE_DIV_FRAC_Msk            (0x1UL << USART_HCR_TX_USE_DIV_FRAC_Pos)

#define USART_HCR_AUTO_CG_Pos                    (3)
#define USART_HCR_AUTO_CG_Msk                    (0x1FFUL << USART_HCR_AUTO_CG_Pos)

#define USART_HCR_DMA_EOR_MODE_Pos               (12)
#define USART_HCR_DMA_EOR_MODE_Msk               (0x1UL << USART_HCR_DMA_EOR_MODE_Pos)
/** @} */

/** @name LSR - USART_LSR register */
/** @{ */
#define USART_LSR_TX_BUSY_Pos                    (0)
#define USART_LSR_TX_BUSY_Msk                    (0x1UL << USART_LSR_TX_BUSY_Pos)

#define USART_LSR_RX_BUSY_Pos                    (1)
#define USART_LSR_RX_BUSY_Msk                    (0x1UL << USART_LSR_RX_BUSY_Pos)

#define USART_LSR_BREAK_SENDING_Pos              (2)
#define USART_LSR_BREAK_SENDING_Msk              (0x1UL << USART_LSR_BREAK_SENDING_Pos)

#define USART_LSR_AUTOBAUD_WORKING_Pos           (3)
#define USART_LSR_AUTOBAUD_WORKING_Msk           (0x1UL << USART_LSR_AUTOBAUD_WORKING_Pos)

#define USART_LSR_RXD_ST_Pos                     (4)
#define USART_LSR_RXD_ST_Msk                     (0x1UL << USART_LSR_RXD_ST_Pos)

#define USART_LSR_LUAC_RXFIFO_EMPTY_Pos          (5)
#define USART_LSR_LUAC_RXFIFO_EMPTY_Msk          (0x1UL << USART_LSR_LUAC_RXFIFO_EMPTY_Pos)

#define USART_LSR_LUAC_RXFIFO_FULL_Pos           (6)
#define USART_LSR_LUAC_RXFIFO_FULL_Msk           (0x1UL << USART_LSR_LUAC_RXFIFO_FULL_Pos)

#define USART_LSR_LUAC_STOPFLG_Pos               (7)
#define USART_LSR_LUAC_STOPFLG_Msk               (0x1UL << USART_LSR_LUAC_STOPFLG_Pos)

#define USART_LSR_LUAC_RX_BUSY_Pos               (8)
#define USART_LSR_LUAC_RX_BUSY_Msk               (0x1UL << USART_LSR_LUAC_RX_BUSY_Pos)
/** @} */

/** @name DIVIR - USART_DIVIR register */
/** @{ */
#define USART_DIVIR_VALUE_Pos                    (0)
#define USART_DIVIR_VALUE_Msk                    (0x3FFFFFUL << USART_DIVIR_VALUE_Pos)
/** @} */

/** @name FSR - USART_FSR register */
/** @{ */
#define USART_FSR_TXFIFO_WL_Pos                  (0)
#define USART_FSR_TXFIFO_WL_Msk                  (0x3FUL << USART_FSR_TXFIFO_WL_Pos)

#define USART_FSR_RXFIFO_WL_Pos                  (16)
#define USART_FSR_RXFIFO_WL_Msk                  (0xFFUL << USART_FSR_RXFIFO_WL_Pos)
/** @} */

/** @name TCR - USART_TCR register */
/** @{ */
#define USART_TCR_TOCNT_SWTRG_Pos                (0)
#define USART_TCR_TOCNT_SWTRG_Msk                (0x1UL << USART_TCR_TOCNT_SWTRG_Pos)

#define USART_TCR_TOCNT_SWCLR_Pos                (1)
#define USART_TCR_TOCNT_SWCLR_Msk                (0x1UL << USART_TCR_TOCNT_SWCLR_Pos)
/** @} */

/** @name DEBUGR - USART_DEBUGR register */
/** @{ */
#define USART_DEBUGR_VALUE_Pos                   (0)
#define USART_DEBUGR_VALUE_Msk                   (0xFFFFFFFFUL << USART_DEBUGR_VALUE_Pos)
/** @} */


/** @name FCR1 - USART_FCR1 register */
/** @{ */
#define USART_FCR1_TXFIFO_INT_TH_Pos              (0)
#define USART_FCR1_TXFIFO_INT_TH_Msk              (0x1FUL << USART_FCR1_TXFIFO_INT_TH_Pos)

#define USART_FCR1_RXFIFO_INT_TH_Pos              (5)
#define USART_FCR1_RXFIFO_INT_TH_Msk              (0x7FUL << USART_FCR1_RXFIFO_INT_TH_Pos)
/** @} */

static __FORCEINLINE uint32_t UART_readLSR(volatile uint32_t *lsr)
{
    // write to latch
    *lsr = 1;
    return *lsr;
}
/** Peripheral USART_0 base pointer */
#define USART_0                                  ((USART_TypeDef *)MP_UART0_BASE_ADDR)

/** Peripheral USART_1 base pointer */
#define USART_1                                  ((USART_TypeDef *)MP_UART1_BASE_ADDR)

/** Peripheral USART_2 base pointer */
#define USART_2                                  ((USART_TypeDef *)MP_UART2_BASE_ADDR)

/** @brief USART peripheral instance number */
#define USART_INSTANCE_NUM                       (3)

/** @brief USART peripheral instance array */
#define USART_INSTANCE_ARRAY                     {USART_0, USART_1, USART_2}

/**
  * @}
  */ /* end of group USART */

/** @addtogroup WDT Watchdog
  * @{
  */

/**
  * @brief WDT register layout typedef
  *
  */
typedef struct {
    __IO uint32_t CTRL;                          /**< WDT Control Register,               offset: 0x0 */
    __IO uint32_t TOVR;                          /**< WDT Timerout Value Register,        offset: 0x4 */
    __O  uint32_t CCR;                           /**< WDT Counter Clear Register,         offset: 0x8 */
    __O  uint32_t ICR;                           /**< WDT Interrupt Clear Register,       offset: 0xC */
    __O  uint32_t LOCK;                          /**< WDT Lock Register,                  offset: 0x10 */
    __I  uint32_t STAT;                          /**< WDT Status Register,                offset: 0x14 */
} WDT_TypeDef;

/** @name CTRL - WDT_CTRL register */
/** @{ */
#define WDT_CTRL_ENABLE_Pos                      (0)
#define WDT_CTRL_ENABLE_Msk                      (0x1UL << WDT_CTRL_ENABLE_Pos)

#define WDT_CTRL_MODE_Pos                        (1)
#define WDT_CTRL_MODE_Msk                        (0x1UL << WDT_CTRL_MODE_Pos)
/** @} */

/** @name TOVR - WDT_TOVR register */
/** @{ */
#define WDT_TOVR_VAL_Pos                         (0)
#define WDT_TOVR_VAL_Msk                         (0xFFFFUL << WDT_TOVR_VAL_Pos)
/** @} */

/** @name CCR - WDT_CCR register */
/** @{ */
#define WDT_CCR_CNT_CLR_Pos                      (0)
#define WDT_CCR_CNT_CLR_Msk                      (0x1UL << WDT_CCR_CNT_CLR_Pos)
/** @} */

/** @name ICR - WDT_ICR register */
/** @{ */
#define WDT_ICR_ICLR_Pos                         (0)
#define WDT_ICR_ICLR_Msk                         (0x1UL << WDT_ICR_ICLR_Pos)
/** @} */

/** @name LOCK - WDT_LOCK register */
/** @{ */
#define WDT_LOCK_LOCK_VAL_Pos                    (0)
#define WDT_LOCK_LOCK_VAL_Msk                    (0xFFFFUL << WDT_LOCK_LOCK_VAL_Pos)
/** @} */

/** @name STAT - WDT_STAT register */
/** @{ */
#define WDT_STAT_CV_Pos                          (0)
#define WDT_STAT_CV_Msk                          (0xFFFFUL << WDT_STAT_CV_Pos)

#define WDT_STAT_ISTAT_Pos                       (16)
#define WDT_STAT_ISTAT_Msk                       (0x1UL << WDT_STAT_ISTAT_Pos)
/** @} */

/** Peripheral WDT base pointer */
#define WDT                                      ((WDT_TypeDef *)AP_WDG_BASE_ADDR)

/**
  * @}
  */ /* end of group WDT */


/** @addtogroup oneWire
  * @{
  */

/**
  * @brief oneWire register layout typedef
  *
  */
typedef struct {
    __IO  uint32_t ECR;                          /**< OW Enable Control Register,                                offset: 0x0  */
    __IO  uint32_t CDR;                          /**< OW Clock Divider Register,                                 offset: 0x4  */
    __IO  uint32_t IOR;                          /**< OW IO Control/Status Register,                             offset: 0x8  */
    __IO  uint32_t DFR;                          /**< OW Data Format Register,                                   offset: 0xC  */
    __IO  uint32_t OCR;                          /**< OW Operation Cmd Register,                                 offset: 0x10 */
    __IO  uint32_t TBR;                          /**< OW Transmit Buffer Register,                               offset: 0x14 */
    __IO  uint32_t RBR;                          /**< OW Receive Buffer Register,                                offset: 0x18 */
    __IO  uint32_t IER;                          /**< OW Interrupt Enable Register,                              offset: 0x1C */
    __IO  uint32_t IIR;                          /**< OW Interrupt Identification Register,                      offset: 0x20 */
    __IO  uint32_t CSR;                          /**< OW Control Status Register,                                offset: 0x24 */
    __IO  uint32_t RTCR;                         /**< OW Reset Timing Control Register,                          offset: 0x28 */
    __IO  uint32_t ATCR;                         /**< OW Access(write/read) Timing Control Register,             offset: 0x2C */
} OW_TypeDef;

/** @name ECR - OW_ECR register */
/** @{ */
#define OW_ECR_ENABLE_Pos                        (0)
#define OW_ECR_ENABLE_Msk                        (0x1UL << OW_ECR_ENABLE_Pos)

#define OW_ECR_CLK_EN_Pos                        (1)
#define OW_ECR_CLK_EN_Msk                        (0x1UL << OW_ECR_CLK_EN_Pos)

#define OW_ECR_AUTO_CGEN_Pos                     (2)
#define OW_ECR_AUTO_CGEN_Msk                     (0x1UL << OW_ECR_AUTO_CGEN_Pos)

#define OW_ECR_RXD_MJR_Pos                       (3)
#define OW_ECR_RXD_MJR_Msk                       (0x1UL << OW_ECR_RXD_MJR_Pos)
/** @} */

/** @name CDR - OW_CDR register */
/** @{ */
#define OW_CDR_CLKUS_DIV_SUB1_Pos                (0)
#define OW_CDR_CLKUS_DIV_SUB1_Msk                (0xFFUL << OW_CDR_CLKUS_DIV_SUB1_Pos)
/** @} */

/** @name IOR - OW_IOR register */
/** @{ */
#define OW_IOR_IO_SWMODE_Pos                     (0)
#define OW_IOR_IO_SWMODE_Msk                     (0x1UL << OW_IOR_IO_SWMODE_Pos)

#define OW_IOR_IO_SWOEN_Pos                      (1)
#define OW_IOR_IO_SWOEN_Msk                      (0x1UL << OW_IOR_IO_SWOEN_Pos)

#define OW_IOR_IO_SWOUT_Pos                      (2)
#define OW_IOR_IO_SWOUT_Msk                      (0x1UL << OW_IOR_IO_SWOUT_Pos)

#define OW_IOR_IO_SWIN_Pos                       (3)
#define OW_IOR_IO_SWIN_Msk                       (0x1UL << OW_IOR_IO_SWIN_Pos)

#define OW_IOR_IO_SWIN_SYNC_Pos                  (4)
#define OW_IOR_IO_SWIN_SYNC_Msk                  (0x1UL << OW_IOR_IO_SWIN_SYNC_Pos)
/** @} */

/** @name DFR - OW_DFR register */
/** @{ */
#define OW_DFR_MODE_BYTE_Pos                     (0)
#define OW_DFR_MODE_BYTE_Msk                     (0x1UL << OW_DFR_MODE_BYTE_Pos)

#define OW_DFR_MODE_ENDIAN_Pos                   (1)
#define OW_DFR_MODE_ENDIAN_Msk                   (0x1UL << OW_DFR_MODE_ENDIAN_Pos)

#define OW_DFR_MODE_POLARITY_Pos                 (2)
#define OW_DFR_MODE_POLARITY_Msk                 (0x1UL << OW_DFR_MODE_POLARITY_Pos)
/** @} */

/** @name OCR - OW_OCR register */
/** @{ */
#define OW_OCR_CMD_FLUSH_Pos                     (0)
#define OW_OCR_CMD_FLUSH_Msk                     (0x1UL << OW_OCR_CMD_FLUSH_Pos)

#define OW_OCR_CMD_RESET_Pos                     (1)
#define OW_OCR_CMD_RESET_Msk                     (0x1UL << OW_OCR_CMD_RESET_Pos)

#define OW_OCR_CMD_WRITE_Pos                     (2)
#define OW_OCR_CMD_WRITE_Msk                     (0x1UL << OW_OCR_CMD_WRITE_Pos)

#define OW_OCR_CMD_READ_Pos                      (3)
#define OW_OCR_CMD_READ_Msk                      (0x1UL << OW_OCR_CMD_READ_Pos)
/** @} */

/** @name TBR - OW_TBR register */
/** @{ */
#define OW_TBR_TX_BUF_Pos                        (0)
#define OW_TBR_TX_BUF_Msk                        (0xFFUL << OW_TBR_TX_BUF_Pos)
/** @} */

/** @name RBR - OW_RBR register */
/** @{ */
#define OW_RBR_RX_BUF_Pos                        (0)
#define OW_RBR_RX_BUF_Msk                        (0xFFUL << OW_RBR_RX_BUF_Pos)
/** @} */

/** @name IER - OW_IER register */
/** @{ */
#define OW_IER_INTEN_RESET_Pos                   (1)
#define OW_IER_INTEN_RESET_Msk                   (0x1UL << OW_IER_INTEN_RESET_Pos)

#define OW_IER_INTEN_RESET_PD_Pos                (2)
#define OW_IER_INTEN_RESET_PD_Msk                (0x1UL << OW_IER_INTEN_RESET_PD_Pos)

#define OW_IER_INTEN_WRITE_Pos                   (3)
#define OW_IER_INTEN_WRITE_Msk                   (0x1UL << OW_IER_INTEN_WRITE_Pos)

#define OW_IER_INTEN_READ_Pos                    (4)
#define OW_IER_INTEN_READ_Msk                    (0x1UL << OW_IER_INTEN_READ_Pos)
/** @} */

/** @name IIR - OW_IIR register */
/** @{ */
#define OW_IIR_INT_CLR_Pos                       (0)
#define OW_IIR_INT_CLR_Msk                       (0x1UL << OW_IIR_INT_CLR_Pos)

#define OW_IIR_INT_RESET_Pos                     (1)
#define OW_IIR_INT_RESET_Msk                     (0x1UL << OW_IIR_INT_RESET_Pos)

#define OW_IIR_INT_RESET_PD_Pos                  (2)
#define OW_IIR_INT_RESET_PD_Msk                  (0x1UL << OW_IIR_INT_RESET_PD_Pos)

#define OW_IIR_INT_WRITE_Pos                     (3)
#define OW_IIR_INT_WRITE_Msk                     (0x1UL << OW_IIR_INT_WRITE_Pos)

#define OW_IIR_INT_READ_Pos                      (4)
#define OW_IIR_INT_READ_Msk                      (0x1UL << OW_IIR_INT_READ_Pos)

#define OW_IIR_RESET_PD_RES_Pos                  (7)
#define OW_IIR_RESET_PD_RES_Msk                  (0x1UL << OW_IIR_RESET_PD_RES_Pos)
/** @} */

/** @name CSR - OW_CSR register */
/** @{ */
#define OW_CSR_STATUS_SFTREG_Pos                 (0)
#define OW_CSR_STATUS_SFTREG_Msk                 (0xFFUL << OW_CSR_STATUS_SFTREG_Pos)

#define OW_CSR_STATUS_SFTCNT_Pos                 (8)
#define OW_CSR_STATUS_SFTCNT_Msk                 (0x7UL << OW_CSR_STATUS_SFTCNT_Pos)

#define OW_CSR_STATUS_FSM_Pos                    (12)
#define OW_CSR_STATUS_FSM_Msk                    (0x7UL << OW_CSR_STATUS_FSM_Pos)

#define OW_CSR_STATUS_USCNT_Pos                  (16)
#define OW_CSR_STATUS_USCNT_Msk                  (0x3FFUL << OW_CSR_STATUS_USCNT_Pos)

#define OW_CSR_STATUS_MODE_Pos                   (28)
#define OW_CSR_STATUS_MODE_Msk                   (0x3UL << OW_CSR_STATUS_MODE_Pos)

#define OW_CSR_STATUS_USCLK_ENABLE_Pos           (31)
#define OW_CSR_STATUS_USCLK_ENABLE_Msk           (0x1UL << OW_CSR_STATUS_USCLK_ENABLE_Pos)
/** @} */

/** @name RTCR - OW_RTCR register */
/** @{ */
#define OW_RTCR_RESET_SEND_DIV10_Pos             (0)
#define OW_RTCR_RESET_SEND_DIV10_Msk             (0x7FUL << OW_RTCR_RESET_SEND_DIV10_Pos)

#define OW_RTCR_RESET_WAIT_DIV10_Pos             (8)
#define OW_RTCR_RESET_WAIT_DIV10_Msk             (0x7FUL << OW_RTCR_RESET_WAIT_DIV10_Pos)

#define OW_RTCR_RESET_RDDLY_MIN_Pos              (16)
#define OW_RTCR_RESET_RDDLY_MIN_Msk              (0x3FUL << OW_RTCR_RESET_RDDLY_MIN_Pos)

#define OW_RTCR_RESET_RDDLY_MAX_DIV10_Pos        (24)
#define OW_RTCR_RESET_RDDLY_MAX_DIV10_Msk        (0x1FUL << OW_RTCR_RESET_RDDLY_MAX_DIV10_Pos)

/** @} */

/** @name ATCR - OW_ATCR register */
/** @{ */
#define OW_ATCR_WRRD_RECO_Pos                    (0)
#define OW_ATCR_WRRD_RECO_Msk                    (0x7UL << OW_ATCR_WRRD_RECO_Pos)

#define OW_ATCR_WRRD_SLOT_DIV10_Pos              (8)
#define OW_ATCR_WRRD_SLOT_DIV10_Msk              (0xFUL << OW_ATCR_WRRD_SLOT_DIV10_Pos)

#define OW_ATCR_WRRD_START_Pos                   (12)
#define OW_ATCR_WRRD_START_Msk                   (0x7UL << OW_ATCR_WRRD_START_Pos)

#define OW_ATCR_WRRD_WRDLY_Pos                   (16)
#define OW_ATCR_WRRD_WRDLY_Msk                   (0x1FUL << OW_ATCR_WRRD_WRDLY_Pos)

#define OW_ATCR_WRRD_RDDLY_Pos                   (24)
#define OW_ATCR_WRRD_RDDLY_Msk                   (0x1FUL << OW_ATCR_WRRD_RDDLY_Pos)

/** @} */


/** Peripheral OW base pointer */
#define OW                                      ((OW_TypeDef *)AP_ONEWIRE_BASE_ADDR)

/**
  * @}
  */ /* end of group OW */


/** @addtogroup XIC XIC(external interrupt controller)
  * @{
  */



/**
  * @brief XIC register layout typedef
  *
  */
typedef struct {
    __IO uint32_t LATCHIRQ;                      /**< Latch IRQ Register,                           offset: 0x0 */
    __IO uint32_t IRQSTATUS;                     /**< IRQ Status Register,                          offset: 0x4 */
    __IO uint32_t MASK;                          /**< IRQ Mask Register,                            offset: 0x8 */
    __IO uint32_t PEND;                          /**< Pend Register,                                offset: 0xC */
    __IO uint32_t SWGENIRQ;                      /**< Software Generat IRQ Register,                offset: 0x10 */
    __IO uint32_t CLRIRQ;                        /**< IRQ Clear Register,                           offset: 0x14 */
    __IO uint32_t CLROVF;                        /**< Clear IRQ Overflow Register,                  offset: 0x18 */
    __IO uint32_t OVFSTATUS;                     /**< IRQ Overflow Status Register,                 offset: 0x1C */
} XIC_TypeDef;

/** AP XIC_0 for IPC/APB base pointer */
#define APXIC_0                                    ((XIC_TypeDef *)APXIC0_BASE_ADDR)

/** AP XIC_1 for APB/AHB base pointer */
#define APXIC_1                                    ((XIC_TypeDef *)APXIC1_BASE_ADDR)

/** AP ULDP XIC_2 base pointer */
#define APXIC_2                                    ((XIC_TypeDef *)APXIC2_BASE_ADDR)

/** AP USB XIC_3 base pointer */
#define APXIC_3                                    ((XIC_TypeDef *)APXIC3_BASE_ADDR)


/**
  * @}
  */ /* end of group XIC */
/** @addtogroup DMA DMA
  * @{
  */

/** @brief Number of DMA channel */
#define DMA_NUMBER_OF_HW_CHANNEL_SUPPORTED       (8U)

/** @brief List of DMA request sources */
typedef enum
{
    DMA_MEMORY_TO_MEMORY = -1,                     /**< Dummy for memory to memory transfer */
    DMA_REQUEST_USART0_TX = 3,                     /**< USART0 TX                           */
    DMA_REQUEST_USART0_RX,                         /**< USART0 RX                           */
    DMA_REQUEST_USART1_TX,                         /**< USART1 TX                           */
    DMA_REQUEST_USART1_RX,                         /**< USART1 RX                           */
    DMA_REQUEST_USART2_TX,                         /**< USART2 TX                           */
    DMA_REQUEST_USART2_RX,                         /**< USART2 RX                           */
    DMA_REQUEST_USART3_TX,                         /**< USART3 TX                           */
    DMA_REQUEST_USART3_RX,                         /**< USART3 RX                           */
    DMA_REQUEST_SPI0_TX = 15,                      /**< SPI0 TX                             */
    DMA_REQUEST_SPI0_RX,                           /**< SPI0 RX                             */
    DMA_REQUEST_SPI1_TX,                           /**< SPI1 TX                             */
    DMA_REQUEST_SPI1_RX,                           /**< SPI1 RX                             */
    DMA_REQUEST_USP0_TX,                           /**< USP0 TX                             */
    DMA_REQUEST_USP0_RX,                           /**< USP0 RX                             */
    DMA_REQUEST_USP1_TX,                           /**< USP1 TX                             */
    DMA_REQUEST_USP1_RX,                           /**< USP1 RX                             */
    DMA_REQUEST_USP2_TX,                           /**< USP2 TX                             */
    DMA_REQUEST_USP2_RX,                           /**< USP2 RX                             */

    DMA_REQUEST_USIM0_TX = 3,                      /**< USIM0 TX                            */
    DMA_REQUEST_USIM0_RX,                          /**< USIM0 RX                            */
    DMA_REQUEST_USIM1_TX,                          /**< USIM1 TX                            */
    DMA_REQUEST_USIM1_RX,                          /**< USIM1 RX                            */

    DMA_REQUEST_FLASH_TX = 44,                     /**< FLASH TX                            */
    DMA_REQUEST_FLASH_RX,                          /**< FLASH RX                            */
    DMA_REQUEST_PSRAM_TX,
    DMA_REQUEST_PSRAM_RX,
    DMA_REQUEST_UNILOG_TX,                         /**< UNILOG TX                           */
} DmaRequestSource_e;

/** @brief DMA channel mapping */
#define UNILOG_TX_CHANNEL                        (0)

/**
  * @}
  */ /* end of group DMA */


/*  End of section using anonymous unions and disabling warnings   */
#if   defined (__CC_ARM)
  #pragma pop
#elif defined (__ICCARM__)
  /* leave anonymous unions enabled */
#elif (__ARMCC_VERSION >= 6010050)
  #pragma clang diagnostic pop
#elif defined (__GNUC__)
  /* anonymous unions are enabled by default */
#elif defined (__TMS470__)
  /* anonymous unions are enabled by default */
#elif defined (__TASKING__)
  #pragma warning restore
#elif defined (__CSMC__)
  /* anonymous unions are enabled by default */
#else
  #warning Not supported compiler type
#endif

/** @addtogroup Bit_Field_Access_Macros Macros for use with bit field definitions (xxx_Pos, xxx_Msk)
  * @{
  */

/**
 * @brief   Mask and shift a bit field value for use in a register bit range
 *
 * @param[in] field  Name of the register bit field
 * @param[in] value  Value of the bit field
 * @return           Masked and shifted value
 */
#define EIGEN_VAL2FLD(field, value)    (((value) << field ## _Pos) & field ## _Msk)

/**
 * @brief     Mask and shift a register value to extract a bit filed value
 *
 * @param[in] field  Name of the register bit field
 * @param[in] value  Value of register
 * @return           Masked and shifted bit field value
*/
#define EIGEN_FLD2VAL(field, value)    (((value) & field ## _Msk) >> field ## _Pos)

/**
  * @}
  */ /* end of group Bit_Field_Access_Macros */

/**
  * @}
  */ /* end of group Device_peripheral_access */

#ifdef __cplusplus
}
#endif

#endif /* EC716S_H */
