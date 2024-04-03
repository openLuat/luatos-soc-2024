/**************************************************************************//**
 * @file
 * @brief
 * @version  V1.0.0
 * @date
 ******************************************************************************/
/*
 *
 *
 */


#include <stdlib.h>
#include <string.h>
#include "cmsis_os2.h"                  // ::CMSIS:RTOS2
#include "FreeRTOS.h"                   // ARM.FreeRTOS::RTOS:Core
#include "task.h"                       // ARM.FreeRTOS::RTOS:Core
#include "event_groups.h"               // ARM.FreeRTOS::RTOS:Event Groups
#include "semphr.h"                     // ARM.FreeRTOS::RTOS:Core
#include "bsp.h"
#include "Driver_USART.h"
#include DEBUG_LOG_HEADER_FILE
#include "ostask.h"
#include "exception_process.h"
#include "os_callback_hook.h"
//#include "TmuDriver.h"
#include "sys_record.h"


typedef enum OsHookTaskIdT  /* tasks sent to identified by their task ids */
{
    IDLE_TASK_ID = 16,
    TIME_TASK_ID = 17,
    APP_TASK_ID = 18,
    TCPIP_TASK_ID = 20,
    MQTT_RECV_TASK_ID = 27,
    MQTT_SEND_TASK_ID = 28,
    HTTP_SEND_TASK_ID = 34,
    HTTP_RECV_TASK_ID = 35,
    SIMBIP_TASK_ID = 37,
    MAIN_TASK_ID = 38,
    CIS_RECV_TASK_ID = 41,
    ABUP_FOTA_TASK_ID = 53, //abup_fota
    IPERF_C_TASK_ID = 54,   //iperf_client_thread
    IPERF_S_TASK_ID = 55,   //iperf_server_thread
    PING_TASK_ID = 56,      //ping_thread        // 56
    SLIPIF_TASK_ID = 57,    //slipif_loop
    SNMP_TASK_ID = 58,      //snmp_netconn
    ABUP_TIMER_TASK_ID = 59,
    AP_CON_TEST_TASK_ID = 62,
    AP_PER_TEST_TASK_ID = 63,
    AP_SCT_TEST_TASK_ID = 64,
    USR_APP_TASK_ID = 65,
    SLEEP_TASK_ID = 66,
    LOG_TEST_TASK_ID = 67,
    SWSEND_TEST_TASK_ID = 68,
    AT_ADC_TASK_ID = 69,
    DM_TASK_ID = 70,
    CP_MON_TASK_ID = 71,
    TEST_TASK_ID = 72,
    CTCC_REG_TASK_ID = 73,
    CT_DM_CONN_TASK_ID = 74,
    CUCC_REG_TASK_ID = 75,
    CUST_AT_DEMO_TASK_ID = 76,
    FILE_CLIENT_TASK_ID = 77,
    AP_TEST_TASK_ID = 78,
    CP_TEST_TASK_ID = 79,
    LFS_TASK_ID = 80,
    PWR_KEY_TASK_ID = 81,
    SCT_EEA_TASK_ID = 82,
    SCT_AES_TASK_ID = 83,
    SCT_CKS_TASK_ID = 84,
    CCIO_RX_TASK_ID = 85,
    CCIO_TX_TASK_ID = 86,
    ULDP_TASK_ID = 87,
    CMS_SOCK_MGR_TASK_ID = 88,
    DM_THREAD_TASK_ID = 89,
    CU_DM_TASK_ID = 90,
    SNTP_TASK_ID = 91,
    SSL_RECV_TASK_ID = 92,
    SSL_TASK_ID = 93,
    CMIOT_OTA_MON_TASK_ID = 94,
    CCIO_OPT_TX_TASK2_ID = 95,
    FOTAOS_TASK_ID = 96,
    MYTEST_TASK_ID = 97,
    IMS_ICM_TMP_TASK_ID = 98,
    IMS_UA_TMP_TASK_ID = 99,
    IMS_SIP_TMP_TASK_ID = 100,
    IMS_UTPT_TMP_TASK_ID = 101,
    IMS_MED_TMP_TASK_ID = 102,

    MAXIMUM_TASK_ID,
    UNKNOWN_TASK_ID = 0xFF,
} OsHookTaskId;

extern ecRecordNodeList ecRecordNodeLists[RECORD_LIST_LEN];
extern int currRecordNumb;
extern void TMU_APTimeReadOpen(UINT32 *sysTime);


char over_buf[64];

void vApplicationStackOverflowHook (TaskHandle_t xTask, signed char *pcTaskName)
{
    ECPLAT_PRINTF(UNILOG_PLA_STRING, StackOverflow, P_ERROR, "\r\n!!!error!!!..task:%s..stack.over.flow!!!\r\n", (uint8_t *)pcTaskName);
    sprintf(over_buf, "\r\n!!!error!!!..task:%s..stack.over.flow!!!\r\n",pcTaskName);

    if (UsartPrintHandle != NULL)
    {
        UsartPrintHandle->Send(over_buf, 64);
    }
    EC_ASSERT(0,0,0,0);
}

PLAT_FM_RAMCODE void ecTraceTaskSwitchOut(void)
{
#if (INCLUDE_xTaskGetCurrentTaskHandle == 1)
    vRecordParam recordParam;

    // 7-----If "UNILOG_PHY_SCHEDULE_MODULE" changed should adjust this value accordingly.
    // 0x41----If "PHY_SCHD_LOG_OSTASK_INFO" changed should adjust this value accordingly.
    //UINT32 swLogID = UNILOG_ID_CONSTRUCT(UNILOG_PHY_LOG, 7, 0x41);

    recordParam.ecRecordType = EC_RECORD_TYPE_TASK;
    recordParam.ecLogInOrOut = EC_RECORD_PRINT_OUT;

    vTracePrintSchedule(recordParam);

#endif
}

PLAT_FM_RAMCODE void ecTraceTaskSwitchIn(void)
{
#if(INCLUDE_xTaskGetCurrentTaskHandle == 1)
    vRecordParam recordParam;

    // 7-----If "UNILOG_PHY_SCHEDULE_MODULE" changed should adjust this value accordingly.
    // 0x41----If "PHY_SCHD_LOG_OSTASK_INFO" changed should adjust this value accordingly.
    //UINT32 swLogID = UNILOG_ID_CONSTRUCT(UNILOG_PHY_LOG, 7, 0x41);
    recordParam.ecRecordType = EC_RECORD_TYPE_TASK;
    recordParam.ecLogInOrOut = EC_RECORD_PRINT_IN;

    vTracePrintSchedule(recordParam);

    vTraceScheduleRecord(recordParam);

#endif
}

//void ecTraceTaskCreate(TCB_t *pxNewTCB);
void ecTraceTaskCreate(StaticTask_t *pxTCB)
{
#if(INCLUDE_xTaskGetCurrentTaskHandle == 1)
    const char * task_name;

    task_name =(char *)&(pxTCB->ucDummy7[ 0 ]); //pcTaskGetTaskName(pxTCB);

    if(strcmp(task_name, "CcmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CCM_TASK_ID);
    }
    else if(strcmp(task_name, "CemmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, NAS_TASK_ID);
    }
    else if(strcmp(task_name, "CerrcTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CERRC_TASK_ID);
    }
    else if(strcmp(task_name, "CeupTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CEUP_TASK_ID);
    }
    else if(strcmp(task_name, "UiccCtrlTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UICC_CTRL_TASK_ID);
    }
    else if(strcmp(task_name, "UiccDrvTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, UICC_DRV_TASK_ID);
    }
    else if(strcmp(task_name, "CmsTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CMS_TASK_ID);
    }
    else if(strcmp(task_name, "IDLE") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IDLE_TASK_ID);
    }
    else if(strcmp(task_name, "Tmr Svc") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TIME_TASK_ID);
    }
    else if(strcmp(task_name, "mainTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MAIN_TASK_ID);
    }
    else if(strcmp(task_name, "SimBipTak") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SIMBIP_TASK_ID);
    }
    else if(strcmp(task_name, "cpMonTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CP_MON_TASK_ID);
    }
    else if(strcmp(task_name, "ApTestTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, AP_TEST_TASK_ID);
    }
    else if(strcmp(task_name, "usrapp") == 0)
    {
        vTaskSetTaskNumber(pxTCB, USR_APP_TASK_ID);
    }
    else if(strcmp(task_name, "atAdc") == 0)
    {
        vTaskSetTaskNumber(pxTCB, AT_ADC_TASK_ID);
    }
    else if(strcmp(task_name, "custAtDemo") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CUST_AT_DEMO_TASK_ID);
    }
    else if(strcmp(task_name, "fileClient") == 0)
    {
        vTaskSetTaskNumber(pxTCB, FILE_CLIENT_TASK_ID);
    }
    else if(strcmp(task_name, "httpRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, HTTP_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "httpSend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, HTTP_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "mqttRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MQTT_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "mqttSend") == 0)
    {
        vTaskSetTaskNumber(pxTCB, MQTT_SEND_TASK_ID);
    }
    else if(strcmp(task_name, "CcioRxTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CCIO_RX_TASK_ID);
    }
    else if(strcmp(task_name, "CcioTxTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CCIO_TX_TASK_ID);
    }
    else if(strcmp(task_name, "Ccio2TxTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CCIO_OPT_TX_TASK2_ID);
    }
    else if(strcmp(task_name, "dmConnTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, DM_TASK_ID);
    }
    else if(strcmp(task_name, "ctccAutoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CTCC_REG_TASK_ID);
    }
    else if(strcmp(task_name, "ctdmConn") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CT_DM_CONN_TASK_ID);
    }
    else if(strcmp(task_name, "cuccAutoReg") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CUCC_REG_TASK_ID);
    }
    else if(strcmp(task_name, "pwrKeyTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PWR_KEY_TASK_ID);
    }
    else if(strcmp(task_name, "iperf_c") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IPERF_C_TASK_ID);
    }
    else if(strcmp(task_name, "iperf_s") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IPERF_S_TASK_ID);
    }
    else if(strcmp(task_name, "lfs") == 0)
    {
        vTaskSetTaskNumber(pxTCB, LFS_TASK_ID);
    }
    else if(strcmp(task_name, "appTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, APP_TASK_ID);
    }
    else if(strcmp(task_name, "ping_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, PING_TASK_ID);
    }
    else if(strcmp(task_name, "slipif_loop") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SLIPIF_TASK_ID);
    }
    else if(strcmp(task_name, "snmp_netconn") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SNMP_TASK_ID);
    }
    else if(strcmp(task_name, "tcpip_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TCPIP_TASK_ID);
    }
    else if(strcmp(task_name, "cis_recv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CIS_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "testThread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, TEST_TASK_ID);
    }
    else if(strcmp(task_name, "dm_task_thread") == 0)
    {
        vTaskSetTaskNumber(pxTCB, DM_THREAD_TASK_ID);
    }
    else if(strcmp(task_name, "cms_sock_mgr") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CMS_SOCK_MGR_TASK_ID);
    }
    else if(strcmp(task_name, "sslRecv") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SSL_RECV_TASK_ID);
    }
    else if(strcmp(task_name, "sslTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SSL_TASK_ID);
    }
    else if(strcmp(task_name, "CmiotOtaMonitor") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CMIOT_OTA_MON_TASK_ID);
    }
    else if(strcmp(task_name, "sntp_task") == 0)
    {
        vTaskSetTaskNumber(pxTCB, SNTP_TASK_ID);
    }
    else if(strcmp(task_name, "cuDmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, CU_DM_TASK_ID);
    }
    else if(strcmp(task_name, "FotaosTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, FOTAOS_TASK_ID);
    }
    else if(strcmp(task_name, "IcmTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IMS_ICM_TMP_TASK_ID);
    }
    else if(strcmp(task_name, "UaTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IMS_UA_TMP_TASK_ID);
    }
    else if(strcmp(task_name, "SipTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IMS_SIP_TMP_TASK_ID);
    }
    else if(strcmp(task_name, "UtptTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IMS_UTPT_TMP_TASK_ID);
    }
    else if(strcmp(task_name, "MedTask") == 0)
    {
        vTaskSetTaskNumber(pxTCB, IMS_MED_TMP_TASK_ID);
    }
    else
    {
        vTaskSetTaskNumber(pxTCB, UNKNOWN_TASK_ID);
    }
#endif

}



