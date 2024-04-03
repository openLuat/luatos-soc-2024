#ifndef  BL_BSP_H
#define  BL_BSP_H

#ifdef __cplusplus
extern "C" {
#endif

#include "bsp.h"
#if 0
typedef enum
{
    SelSerUart0PrintType = 0,
    SelSerUart1PrintType = 1,
    SelSerUart2PrintType = 2,

    SelSerVCom0PrintType = 10,
    SelSerVCom1PrintType = 11,
    SelSerVCom2PrintType = 12,
}SelSerialPrintType;
#endif

typedef enum
{
    URCUart1PrintType = 1,

    URCVCom0PrintType = 4,
    URCVCom1PrintType = 5,
    URCVCom2PrintType = 6,
}URCSelPrintType;


void BSP_Init(void);
void BSP_DeInit(void);
void BL_CustomInit(void);
void BL_CustomDeInit(void);
void CleanBootRomResouce(void);
void bl_clk_check(void);
void SystemNormalBootInit(void);
uint8_t SystemWakeUpBootInit(void);
void BSP_QSPI_Set_Clk_50M(void);
void BSP_WdtInit(void);

void SelNormalOrURCPrint(uint32_t Sel);
/*ONLY for FOTA case!!!!!!, and only when SRAM is not enough
  e.g. if FOTA and USB URC en, USB mem may in CSMB
  call en API before USB init
  access CSMB need power on CP, but SW has no method
  to power off CP. for FOTA case, system will reboot 
  !!!!!!!  Do not use the first two words on CSMB  !!!!!!!!!
  The base address of CSMB start from 0x200000, use from 0x200008
  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!  
  */
void BSP_apAccCmsbEn(void);
void BSP_apAccCmsbDisEn(void);

/*
 *  URC set enable and baudrate value config
 *  Parameter:   enable uart1/vcom0-2 init for urc, 0-disable, 1-enable
 *  Parameter:   baudrate for UART1(URC)
 *  Parameter:   serial type(uart1/vcom0-2)
 *  Parameter:   index of uart(1) or vcom(0-2)
 */
void BSP_URCSetCfg(uint8_t enable, uint32_t baud, URCSelPrintType serlType, uint8_t serlIdx);

/*
 *  WDT wdt timecnt value config
 *  Parameter:   Value - timecnt, timeout period equal to Value *20/32768 second
 */

void BSP_WdtTimeCntCfg(uint32_t Value);


#ifdef __cplusplus
}
#endif

#endif /* BL_BSP_H */
