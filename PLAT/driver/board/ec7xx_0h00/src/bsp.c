/****************************************************************************
 *
 * Copy right:   2018 Copyrigths of AirM2M Ltd.
 * File name:    bsp.c
 * Description:
 * History:
 *
 ****************************************************************************/
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include "Driver_Common.h"
#include "clock.h"
#include "bsp.h"
#include "system_ec7xx.h"
#include DEBUG_LOG_HEADER_FILE
#include "ec_string.h"
#include "mem_map.h"
#ifdef FEATURE_CCIO_ENABLE
#include "uart_device.h"
#endif
#include "dbversion.h"
#include "clock.h"
#include "hal_adcproxy.h"
#include "apmu_external.h"
#include "slpman.h"

#define BSP_TEXT_SECTION     SECTION_DEF_IMPL(.sect_bsp_text)
#define BSP_RODATA_SECTION   SECTION_DEF_IMPL(.sect_bsp_rodata)
#define BSP_DATA_SECTION     SECTION_DEF_IMPL(.sect_bsp_data)
#define BSP_BSS_SECTION      SECTION_DEF_IMPL(.sect_bsp_bss)
#define FLASH_READ_GRANT_SIZE   (16)
#define FLASH_READ_MAX_DELAY    (10000)//10ms


extern ARM_DRIVER_USART Driver_USART0;
extern ARM_DRIVER_USART Driver_USART1;

BSP_BSS_SECTION ARM_DRIVER_USART *UsartPrintHandle = NULL;
BSP_BSS_SECTION ARM_DRIVER_USART *UsartUnilogHandle = NULL;
BSP_BSS_SECTION ARM_DRIVER_USART *UsartAtCmdHandle = NULL;

BSP_BSS_SECTION static uint8_t OSState = 0;     // OSState = 0 os not start, OSState = 1 os started
#ifdef __USER_CODE__
#else
BSP_BSS_SECTION static uint32_t gUartBaudrate[3]; // a copy for uart baud rate
#endif
extern void trimAdcSetGolbalVar(void);
#if defined CHIP_EC718 || defined CHIP_EC716
extern void trimLdoAIOVadjSetGolbalVar(void);
#endif
#if defined CHIP_EC718
extern void trimVadjVbatSenseSetGolbalVar(void);
#endif
extern void GPR_RmiErrCfg(bool en);
extern uint32_t GPR_RmiErrAddrGet(void);
extern uint8_t  FLASH_XIPRead(uint8_t* pData, uint32_t ReadAddr, uint32_t Size);
extern uint8_t ShareInfoAPGetCPBusyFlag( void );


void BSP_InitUartDriver(ARM_DRIVER_USART *drvHandler,
                        ARM_POWER_STATE powerMode,
                        uint32_t settings,
                        uint32_t baudRate,
                        ARM_USART_SignalEvent_t cb_event)
{
    if(drvHandler)
    {
        drvHandler->Initialize(cb_event);
        drvHandler->PowerControl(powerMode);
        drvHandler->Control(settings, baudRate);
    }
}

void BSP_DeinitUartDriver(ARM_DRIVER_USART *drvHandler)
{
    if(drvHandler)
    {
        drvHandler->PowerControl(ARM_POWER_OFF);
        drvHandler->Uninitialize();
    }
}

#if defined ( __GNUC__ )

/*
 *  retarget for _write implementation
 *  Parameter:      ch: character will be out
 */
int io_putchar(int ch)
{
    if (UsartPrintHandle != NULL)
        UsartPrintHandle->SendPolling((uint8_t*)&ch, 1);
    return 0;
}

/*
 *  retarget for _read implementation
 *  Parameter:      ch: character will be read
 */
int io_getchar()
{
    uint8_t ch = 0;
    if (UsartPrintHandle != NULL)
        UsartPrintHandle->Receive(&ch, 1);
    return (ch);
}


int fgetc(FILE *f)
{
    uint8_t ch = 0;
    if (UsartPrintHandle != NULL)
        UsartPrintHandle->Receive(&ch, 1);
    return (ch);
}

__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line) {
  printf("Assert, expr:%s, file: %s, line: %d\r\n", expr, file, line);
  while(1);
}


void __assert_func(const char *filename, int line, const char *assert_func, const char *expr)
{
    for(uint8_t i = 0; i<5; i++)
    {
        uniLogFlushOut();
        ECPLAT_PRINTF(UNILOG_PLA_DRIVER, assert_func_1, P_ERROR, "Assert, expr:%s, file: %s, line: %d\r\n", expr, filename, line);
    }
    while(1);
}



#elif defined (__CC_ARM)

/*
 *  retarget for printf implementation
 *  Parameter:      ch: character will be out
 *                  f:  not used
 */
int fputc(int ch, FILE *f)
{
    if (UsartPrintHandle != NULL)
        UsartPrintHandle->SendPolling((uint8_t*)&ch,1);
    return 0;
}

/*
 *  retarget for scanf implementation
 *  Parameter:      f:  not used
 */
int fgetc(FILE *f)
{
    uint8_t ch = 0;
    if (UsartPrintHandle != NULL)
        UsartPrintHandle->Receive(&ch,1);
    return (ch);
}

__attribute__((weak,noreturn))
void __aeabi_assert (const char *expr, const char *file, int line) {
  printf("Assert, expr:%s, file: %s, line: %d\r\n", expr, file, line);
  while(1);
}

#endif


#ifdef __USER_CODE__	//move to flash_code.c
#else
uint32_t GET_PMU_RAWFLASH_OFFSET(void)
{
	return FLASH_MEM_BACKUP_ADDR;
}
#endif



void setOSState(uint8_t state)
{
    OSState = state;
}

PLAT_PA_RAMCODE uint8_t getOSState(void)		//1 os started. 0 no OS or OS not started yet
{
    return OSState;
}


uint8_t* getBuildInfo(void)
{
    return (uint8_t *)BSP_HEADER;
}

uint8_t* getVersionInfo(void)
{
    return (uint8_t *)VERSION_INFO;
}

uint8_t* getATIVersionInfo(void)
{
    return (uint8_t *)ATI_VERSION_INFO;
}

//move here since this is an common and opensource place
uint8_t* getDebugDVersion(void)
{
    return (uint8_t*)DB_VERSION_UNIQ_ID;
}

extern void excepDump(uint32_t* start, uint32_t len);

void cpCoreRegsDumpCheckHook(void)
{
    excepDump((uint32_t*)0x4f000000, 40);
}

__attribute__ ((noinline)) __attribute__((weak)) uint32_t getUnilogUartPort(void)
{
    return UART_0_FOR_UNILOG;      // Swith to UART_x_FOR_UNILOG if need to use other uart for unilog
}

__attribute__ ((noinline)) void getUnilogRamLogBuff(uint32_t *addr, uint32_t *len)
{
#ifdef FEATURE_EXCEPTION_FLASH_DUMP_ENABLE
    ecGetUnilogDumpAddrAndLen(addr, len);
#else
    *addr = 0; // suggest to be 16 bytes aligned, note this area shall not be initialized in boot phase
    *len = 0;
#endif
}

#ifdef __USER_CODE__	//no need, move to bsp_custom.c
#else
void setUartBaudRate(uint8_t idx, uint32_t baudRate)
{
    gUartBaudrate[idx] = baudRate;

    ECPLAT_PRINTF(UNILOG_PMU, setUartBaudRate_1, P_WARNING, "Set BaudRate = %d, %d, %d", gUartBaudrate[0], gUartBaudrate[1], gUartBaudrate[2]);
}


void FLASH_appRead(uint8_t *pData, uint32_t ReadAddr, uint32_t Size)
{
    uint32_t numOfReadTrunk = 0, i=0;
    uint32_t waitLoop =0;

    numOfReadTrunk = (Size/FLASH_READ_GRANT_SIZE);
    if(Size <= FLASH_READ_GRANT_SIZE)
    {
        FLASH_XIPRead(pData, ReadAddr, Size);
    }
    else
    {
        for(i=0; i<numOfReadTrunk; i++)
        {
            while(ShareInfoAPGetCPBusyFlag() == 1)
            {
                delay_us(1);
                waitLoop ++;
                if(waitLoop >=FLASH_READ_MAX_DELAY)
                {
                    waitLoop = 0;
                    break;
                }
            }
            FLASH_XIPRead(pData+i*FLASH_READ_GRANT_SIZE, ReadAddr+i*FLASH_READ_GRANT_SIZE, FLASH_READ_GRANT_SIZE);

        }
        if((Size - i*FLASH_READ_GRANT_SIZE) != 0)
        {
            while(ShareInfoAPGetCPBusyFlag() == 1)
            {
                delay_us(1);
                waitLoop ++;
                if(waitLoop >=FLASH_READ_MAX_DELAY)
                {
                    waitLoop = 0;
                    break;
                }
            }
            FLASH_XIPRead(pData+i*FLASH_READ_GRANT_SIZE, ReadAddr+i*FLASH_READ_GRANT_SIZE, Size-i*FLASH_READ_GRANT_SIZE);
        }
    }
}


/**
  \fn           bool getCPWakeupType(void)
  \brief        wakeup cp in polling mode or int mode
                in polling mode interrupt mask for 700us at most which may cause uart fifo overflow @ 921600.
				in int mode, interrupt mask for less than 200us
  \returns      true: cp wakeup in int mode   false: cp wakeup in polling mode
*/
bool getCPWakeupType(void)          // true: cp wakeup in int mode   false: cp wakeup in polling mode
{
#if 0		// just an example for customer to enable int mode
    if((gUartBaudrate[0] == 921600) || (gUartBaudrate[1] == 921600) || (gUartBaudrate[2] == 921600))
    {
        return true;
    }
    else
    {
        return false;
    }
#else
    return false;
#endif
}
#endif

uint32_t getAPFlashLoadAddr(void)
{
    return AP_FLASH_LOAD_ADDR;
}

/**
  \fn           void apmuNeedSlpWaitTimeInWakeupFlow(bool *bUartDelay, bool *bSlp1ExtIntDelay)
  \brief        give pmu module information to restart slpWaitTime in pmu flow
  \param[out]    bUartDelay: give a result to pmu module whether we need restart sleep wait timer when, (1)sleep failed with uart pending in all sleep mode (2)receive uart in sleep1 wakeup flow
  \param[out]    bSlp1ExtIntDelay: give a result to pmu module whether we need restart sleep wait timer when, (1) wakeup from sleep1 with external int pending
                                   for sleep2/hib mode, restart slpWaitTime in BSP_CustomInit
  \returns      void
*/
void apmuNeedSlpWaitTimeInWakeupFlow(bool *bUartDelay, bool *bSlp1ExtIntDelay)
{
    uint32_t xic0_latch = XIC_LatchIRQ(APXIC_0);
    slpManWakeSrc_e wakeSrc = slpManGetWakeupSrc();

    *bUartDelay = true;
    *bSlp1ExtIntDelay = false;

    if((xic0_latch & (1<<(PXIC0_UART2_IRQn-32))) == 0)
    {
        if((xic0_latch & (1<<(PXIC0_UART1_IRQn-32))) == 0)
        {
            if((xic0_latch & (1<<(PXIC0_UART0_IRQn-32))) == 0)
            {
                *bUartDelay = false;
            }
        }
    }

    if((WAKEUP_FROM_PAD == wakeSrc) || (WAKEUP_FROM_LPUART == wakeSrc))
    {
        *bSlp1ExtIntDelay = true;
    }
}


#ifdef UINILOG_FEATURE_ENABLE
/**
  \fn           void logToolCommandHandle(uint8_t *atcmd_buffer, uint32_t len)
  \brief        handle downlink command sent from unilog tool EPAT
                if need to handle more command in future, add command-handler table
  \param[in]    event         UART event, note used in this function
  \param[in]    cmd_buffer    command received from UART
  \param[in]    len           command length
  \returns      void
*/
void logToolCommandHandle(uint32_t event, uint8_t *cmd_buffer, uint32_t len)
{
    (void)event;

    uint8_t * LogDbVserion=getDebugDVersion();

    if(ec_strnstr((const char *)cmd_buffer, "^logversion", len))
    {

        ECPLAT_PRINTF(UNILOG_PLA_INTERNAL_CMD, get_log_version, P_SIG, "LOGVERSION:%s",LogDbVserion);

    }
    else
    {

        ECPLAT_PRINTF(UNILOG_PLA_STRING, get_log_version_1, P_ERROR, "%s", "invalid command from EPAT");

    }

    return;

}


/**
 * unilog entity is removed for the reason of BSP small image.
 * for more implementation details, pls refer to
 * gCustSerlEntity[CUST_ENTITY_UNILOG] in ccio_provider.c
 */
#ifdef __USER_CODE__ //no need, move to am_uart.c
#else
/*
 *  set unilog uart port
 *  Parameter:      port: for unilog
 *  Parameter:      baudrate: uart baudrate
 */
 void SetUnilogUart(usart_port_t port, uint32_t baudrate, bool startRecv)
{
    ARM_POWER_STATE      powerMode = ARM_POWER_FULL;
    uint32_t           ctrlSetting = ARM_USART_MODE_ASYNCHRONOUS | ARM_USART_DATA_BITS_8 | \
                                     ARM_USART_PARITY_NONE       | ARM_USART_STOP_BITS_1 | \
                                     ARM_USART_FLOW_CONTROL_NONE;

    if (port == PORT_USART_0)
    {
#if (RTE_UART0)
        UsartUnilogHandle = &CREATE_SYMBOL(Driver_USART, 0);
#endif
    }
    else if (port == PORT_USART_1)
    {
#if (RTE_UART1)
        UsartUnilogHandle = &CREATE_SYMBOL(Driver_USART, 1);
#endif
    }

    if (UsartUnilogHandle == NULL) return;

#ifdef FEATURE_CCIO_ENABLE
    UartDevConf_t  uartDevConf;
    UartHwConf_t   *uartHwConf = &uartDevConf.hwConf;

    memset(&uartDevConf, 0, sizeof(UartDevConf_t));

    uartHwConf->powerMode   = powerMode;
    uartHwConf->ctrlSetting = ctrlSetting;
    uartHwConf->baudRate    = baudrate;

    uartDevConf.drvHandler = UsartUnilogHandle;
    uartDevConf.mainUsage  = CSIO_DT_DIAG;
    uartDevConf.speedType  = CCIO_ST_HIGH;
    uartDevConf.rbufFlags  = CUST_RBUF_FOR_DIAG;
    if(startRecv)
    {
        uartDevConf.bmCreateFlag = CCIO_TASK_FLAG_RX;
    }
    else
    {
        uartDevConf.bmCreateFlag = CCIO_TASK_FLAG_NONE;
    }

    uartDevCreate(port, &uartDevConf);
#else
    BSP_InitUartDriver(UsartUnilogHandle, powerMode, ctrlSetting, baudrate, NULL);
#endif
}
#endif


void GPR_rmiErrDetectIsr(void)
{
    volatile uint32_t rmiErrAddr;
    rmiErrAddr = GPR_RmiErrAddrGet();

    ECPLAT_PRINTF(UNILOG_PMU, GPR_rmiErrDetectIsr_1, P_WARNING, "Rmi Err Detect Address=0x%x", rmiErrAddr);
}


void GPR_rmiErrDetectInit(void)
{
    XIC_SetVector(PXIC0_RMIIF_TOERR_IRQn, &GPR_rmiErrDetectIsr);
    XIC_EnableIRQ(PXIC0_RMIIF_TOERR_IRQn);
    GPR_RmiErrCfg(true);
}


void FlushUnilogOutput(void)
{
    uniLogFlushOut();

    if(UsartUnilogHandle == NULL)
        return;
    UsartUnilogHandle->Control(ARM_USART_CONTROL_FLUSH_TX, 0);
}
#endif

void BSP_CommonInit(void)
{
    SystemCoreClockUpdate();

    PAD_driverInit();

    GPR_initialize();

    trimAdcSetGolbalVar();
    
    #if defined CHIP_EC718 || defined CHIP_EC716
    trimLdoAIOVadjSetGolbalVar();
    #endif

	#if defined CHIP_EC718
	trimVadjVbatSenseSetGolbalVar();
	#endif
	
    apmuInit();

    //interrupt config
    IC_PowupInit();

    if(apmuGetAPBootFlag() == 0)            // power on
    {
        apmuSetCPFastBoot(false);           // set cp fast boot in case of cp dap wakeup
    }

    cpADCInit();        // enable adc ref output, need stable time

    GPR_rmiErrDetectInit();

    BOOT_TIMESTAMP_SET(1, 3);

}

