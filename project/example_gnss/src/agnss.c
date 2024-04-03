#include "luat_mobile.h"
#include "common_api.h"
#include "luat_network_adapter.h"
#include "luat_debug.h"
#include "luat_rtos.h"
#include "lbsLoc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "luat_fs.h"
#include "agnss.h"
#include "luat_rtc.h"
#include "luat_http.h"
#define LBSLOC_SERVER_UDP_IP "bs.openluat.com" // 基站定位网址
#define LBSLOC_SERVER_UDP_PORT 12411           // 端口
#define UART_ID 2

/// @brief 合宙IOT 项目productkey ，必须加上，否则定位失败
static char *productKey = "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX";
static luat_rtos_task_handle lbsloc_task_handle = NULL;
static uint8_t lbsloc_task_tatus = 0;

static bool ddddtoddmm(char *location, char *test)
{
    char *integer = NULL;
    char *fraction = NULL;
    char tmpstr[15] = {0};
    float tmp = 0;
    integer = strtok(location, ".");
    if (integer)
    {
        fraction = strtok(NULL, ".");
        sprintf(tmpstr, "0.%s", fraction == NULL ? "0" : fraction);
        tmp = atof(tmpstr) * 60;
        tmp = atoi(integer) * 100 + tmp;
        memset(tmpstr, 0x00, 15);
        sprintf(tmpstr, "%f", tmp);
        memcpy(test, tmpstr, strlen(tmpstr) + 1);
        return true;
    }
    return false;
}

/// @brief 把string 转换为BCD 编码
/// @param arr 字符串输入
/// @param len 长度
/// @param outPut 输出
/// @return
static uint8_t imeiToBcd(uint8_t *arr, uint8_t len, uint8_t *outPut)
{
    if (len % 2 != 0)
    {
        arr[len] = 0x0f;
    }

    uint8_t tmp = 0;

    for (uint8_t j = 0; j < len; j = j + 2)
    {
        outPut[tmp] = (arr[j] & 0x0f) << 4 | (arr[j + 1] & 0x0f);
        tmp++;
    }
    for (uint8_t i = 0; i < 8; i++)
    {
        outPut[i] = (outPut[i] % 0x10) * 0x10 + (outPut[i] - (outPut[i] % 0x10)) / 0x10;
    }
    return 0;
}

/// @brief BCD ->> str
/// @param pOutBuffer
/// @param pInBuffer
/// @param nInLen 长度
/// @return
static uint32_t location_service_bcd_to_str(uint8_t *pOutBuffer, uint8_t *pInBuffer, uint32_t nInLen)
{
    uint32_t len = 0;
    uint8_t ch;
    uint8_t *p = pOutBuffer;
    uint32_t i = 0;

    if (pOutBuffer == NULL || pInBuffer == NULL || nInLen == 0)
    {
        return 0;
    }

    for (i = 0; i < nInLen; i++)
    {
        ch = pInBuffer[i] & 0x0F;
        if (ch == 0x0F)
        {
            break;
        }
        *pOutBuffer++ = ch + '0';

        ch = (pInBuffer[i] >> 4) & 0x0F;
        if (ch == 0x0F)
        {
            break;
        }
        *pOutBuffer++ = ch + '0';
    }

    len = pOutBuffer - p;

    return len;
}

static int32_t luat_test_socket_callback(void *pdata, void *param)
{
    OS_EVENT *event = (OS_EVENT *)pdata;
    DBG("%x", event->ID);
    return 0;
}
static bool location_service_parse_response(struct am_location_service_rsp_data_t *response, uint8_t *latitude, uint8_t *longitude,
                                            uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint8_t loc[20] = {0};
    uint32_t len = 0;
    struct am_location_service_rsp_data_t locationServiceResponse;
    if (response == NULL || latitude == NULL || longitude == NULL || year == NULL || month == NULL || day == NULL || hour == NULL || minute == NULL || second == NULL)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: invalid parameter");
        return FALSE;
    }

    if (!(response->result == 0 || response->result == 0xFF))
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: result fail %d", response->result);
        return FALSE;
    }

    // latitude
    len = location_service_bcd_to_str(loc, response->latitude, AM_LOCATION_SERVICE_LOCATION_BCD_LEN);
    if (len <= 0)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: latitude fail");
        return FALSE;
    }
    strncat((char *)latitude, (char *)loc, 3);
    strncat((char *)latitude, ".", 2);
    strncat((char *)latitude, (char *)(loc + 3), len - 3);
    len = location_service_bcd_to_str(loc, response->longitude, AM_LOCATION_SERVICE_LOCATION_BCD_LEN);
    if (len <= 0)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: longitude fail");
        return FALSE;
    }
    strncat((char *)longitude, (char *)loc, 3);
    strncat((char *)longitude, (char *)".", 2);
    strncat((char *)longitude, (char *)(loc + 3), len - 3);
    *year = response->year + 2000;
    *month = response->month;
    *day = response->day;
    *hour = response->hour;
    *minute = response->minute;
    *second = response->second;

    return TRUE;
}

static void lbsloc_task(void *arg)
{
    lbsloc_task_tatus = 1;
    network_ctrl_t *g_s_network_ctrl = NULL;
    g_s_network_ctrl = network_alloc_ctrl(NW_ADAPTER_INDEX_LWIP_GPRS);
    network_init_ctrl(g_s_network_ctrl, luat_rtos_get_current_handle(), luat_test_socket_callback, NULL);
    network_set_base_mode(g_s_network_ctrl, 0, 15000, 0, 0, 0, 0);
    int result;
    while (1)
    {
        result = network_wait_link_up(g_s_network_ctrl, 5000);
        if (result)
        {
            continue;
        }
        else
        {
            break;
        }
    }
    struct am_location_service_rsp_data_t locationServiceResponse;
    uint8_t latitude[20] = {0};  // 经度
    uint8_t longitude[20] = {0}; // 维度
    uint16_t year = 0;           // 年
    uint8_t month = 0;           // 月
    uint8_t day = 0;             // 日
    uint8_t hour = 0;            // 小时
    uint8_t minute = 0;          // 分钟
    uint8_t second = 0;          // 秒
    uint8_t lbsLocReqBuf[176] = {0};
    memset(lbsLocReqBuf, 0, 176);
    uint8_t sendLen = 0;
    lbsLocReqBuf[sendLen++] = strlen(productKey);
    memcpy(&lbsLocReqBuf[sendLen], (UINT8 *)productKey, strlen(productKey));
    sendLen = sendLen + strlen(productKey);
    lbsLocReqBuf[sendLen++] = 0x28;
    CHAR imeiBuf[16];
    memset(imeiBuf, 0, sizeof(imeiBuf));
    luat_mobile_get_imei(0, imeiBuf, 16);
    uint8_t imeiBcdBuf[8] = {0};
    imeiToBcd((uint8_t *)imeiBuf, 15, imeiBcdBuf);
    memcpy(&lbsLocReqBuf[sendLen], imeiBcdBuf, 8);
    sendLen = sendLen + 8;
    CHAR muidBuf[64];
    memset(muidBuf, 0, sizeof(muidBuf));
    luat_mobile_get_muid(muidBuf, sizeof(muidBuf));
    lbsLocReqBuf[sendLen++] = strlen(muidBuf);
    memcpy(&lbsLocReqBuf[sendLen], (UINT8 *)muidBuf, strlen(muidBuf));
    sendLen = sendLen + strlen(muidBuf);
    luat_mobile_cell_info_t cell_info;
    memset(&cell_info, 0, sizeof(cell_info));
    luat_mobile_get_cell_info(&cell_info);
    lbsLocReqBuf[sendLen++] = 0x01;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.tac >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.tac & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.mcc >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.mcc & 0XFF;
    uint8_t mnc = cell_info.lte_service_info.mnc;
    if (mnc > 10)
    {
        CHAR buf[3] = {0};
        snprintf(buf, 3, "%02x", mnc);
        int ret1 = atoi(buf);
        lbsLocReqBuf[sendLen++] = ret1;
    }
    else
    {
        lbsLocReqBuf[sendLen++] = mnc;
    }
    int16_t sRssi;
    uint8_t retRssi;
    sRssi = cell_info.lte_service_info.rssi;
    if (sRssi <= -113)
    {
        retRssi = 0;
    }
    else if (sRssi < -52)
    {
        retRssi = (sRssi + 113) >> 1;
    }
    else
    {
        retRssi = 31;
    }
    lbsLocReqBuf[sendLen++] = retRssi;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 24) & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 16) & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.cid & 0xFF;

    uint32_t tx_len, rx_len;

    uint8_t is_break, is_timeout;

    result = network_connect(g_s_network_ctrl, LBSLOC_SERVER_UDP_IP, sizeof(LBSLOC_SERVER_UDP_IP), NULL, LBSLOC_SERVER_UDP_PORT, 5000);
    LUAT_DEBUG_PRINT("%d", result);
    if (!result)
    {
        result = network_tx(g_s_network_ctrl, lbsLocReqBuf, sendLen, 0, NULL, 0, &tx_len, 5000);
        if (!result)
        {
            result = network_wait_rx(g_s_network_ctrl, 15000, &is_break, &is_timeout);
            LUAT_DEBUG_PRINT("%d", result);
            if (!result)
            {
                if (!is_timeout && !is_break)
                {
                    result = network_rx(g_s_network_ctrl, &locationServiceResponse, sizeof(struct am_location_service_rsp_data_t), 0, NULL, NULL, &rx_len);
                    if (!result && (sizeof(struct am_location_service_rsp_data_t) == rx_len))
                    {
                        if (location_service_parse_response(&locationServiceResponse, latitude, longitude, &year, &month, &day, &hour, &minute, &second) == true)
                        {
                            LUAT_DEBUG_PRINT("rcv response, process success");
                            LUAT_DEBUG_PRINT("latitude:%s,longitude:%s,year:%d,month:%d,day:%d,hour:%d,minute:%d,second:%d\r\n", latitude, longitude, year, month, day, hour, minute, second);
                            char data_buf[60] = {0};
                            struct tm tblock = {0};
                            luat_rtc_get(&tblock);
                            snprintf(data_buf, 60, "$AIDTIME,%d,%d,%d,%d,%d,%d,000\r\n", tblock.tm_year + 1900, tblock.tm_mon + 1, tblock.tm_mday, tblock.tm_hour, tblock.tm_min, tblock.tm_sec);
                            LUAT_DEBUG_PRINT("AIDTIME %s", data_buf);
                            if (tblock.tm_year + 1900 > 2022)
                            {
                                luat_uart_write(UART_ID, data_buf, strlen(data_buf));
                            }
                            luat_rtos_task_sleep(200);
                            memset(data_buf, 0x00, 60);
                            char lat[20] = {0};
                            char lng[20] = {0};
                            ddddtoddmm(latitude, lat);
                            ddddtoddmm(longitude, lng);
                            snprintf(data_buf, 60, "$AIDPOS,%s,N,%s,E,1.0\r\n", lat, lng);
                            LUAT_DEBUG_PRINT("AIDPOS %s", data_buf);
                            luat_uart_write(UART_ID, data_buf, strlen(data_buf));
                        }
                        else
                        {
                            LUAT_DEBUG_PRINT("rcv response, but process fail");
                        }
                    }
                }
            }
            else
            {
                LUAT_DEBUG_PRINT("tx fil");
            }
        }
    }
    else
    {
        LUAT_DEBUG_PRINT("connect fil");
    }
    network_close(g_s_network_ctrl, 5000);
    network_release_ctrl(g_s_network_ctrl);
    memset(latitude, 0, 20);
    memset(longitude, 0, 20);
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    second = 0;
    LUAT_DEBUG_PRINT("socket quit");
    lbsloc_task_tatus = 0;
    luat_rtos_task_delete(luat_rtos_get_current_handle());
}

void lbsloc_request(void)
{
    if (lbsloc_task_handle == NULL || lbsloc_task_tatus == 0)
        luat_rtos_task_create(&lbsloc_task_handle, 4 * 2048, 15, "lbsloc", lbsloc_task, NULL, NULL);
    else
        LUAT_DEBUG_PRINT("lbsloc task create fail");
}
#define EPH_HOST "http://download.openluat.com/9501-xingli/HXXT_GPS_BDS_AGNSS_DATA.dat"

static g_s_network_status = 0;
static luat_rtos_task_handle http_task_handle;

static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
    if (LUAT_MOBILE_EVENT_NETIF == event)
    {
        if (LUAT_MOBILE_NETIF_LINK_ON == status)
        {
            g_s_network_status = 1;
            luat_socket_check_ready(index, NULL);
        }
        else
        {
            g_s_network_status = 0;
        }
    }
}

enum
{
    EPH_HTTP_GET_HEAD_DONE = 1,
    EPH_HTTP_GET_DATA,
    EPH_HTTP_GET_DATA_DONE,
    EPH_HTTP_FAILED,
};

static void luatos_http_cb(int status, void *data, uint32_t len, void *param)
{
    uint8_t *eph_data;
    if (status < 0)
    {
        LUAT_DEBUG_PRINT("http failed! %d", status);
        luat_rtos_event_send(param, EPH_HTTP_FAILED, 0, 0, 0, 0);
        return;
    }
    switch (status)
    {
    case HTTP_STATE_GET_BODY:
        if (data)
        {
            eph_data = malloc(len);
            memcpy(eph_data, data, len);
            luat_rtos_event_send(param, EPH_HTTP_GET_DATA, eph_data, len, 0, 0);
        }
        else
        {
            luat_rtos_event_send(param, EPH_HTTP_GET_DATA_DONE, 0, 0, 0, 0);
        }
        break;
    case HTTP_STATE_GET_HEAD:
        if (data)
        {
            LUAT_DEBUG_PRINT("%s", data);
        }
        else
        {
            luat_rtos_event_send(param, EPH_HTTP_GET_HEAD_DONE, 0, 0, 0, 0);
        }
        break;
    case HTTP_STATE_IDLE:
        break;
    case HTTP_STATE_SEND_BODY_START:
        // 如果是POST，在这里发送POST的body数据，如果一次发送不完，可以在HTTP_STATE_SEND_BODY回调里继续发送
        break;
    case HTTP_STATE_SEND_BODY:
        // 如果是POST，可以在这里发送POST剩余的body数据
        break;
    default:
        break;
    }
}

static bool check_eph_time()
{
    if (luat_fs_fexist(EPH_TIME_FILE) == 0)
        return false;
    FILE *fp = luat_fs_fopen(EPH_TIME_FILE, "r");
    time_t history_time;
    luat_fs_fread((void *)&history_time, sizeof(time_t), 1, fp);
    luat_fs_fclose(fp);

    time_t now_time;
    time(&now_time);
    LUAT_DEBUG_PRINT("this is interval time %d", now_time - history_time);
    if ((now_time - history_time) > 4 * 60 * 60)
    {
        return false;
    }
    return true;
}

static void ephemeris_get_task(void *param)
{
    luat_event_t event;
    int result, is_error, eph_download_end;
    uint32_t all, now_free_block, min_free_block, done_len;
    FILE *fp1 = NULL;
    while (1)
    {
        while (g_s_network_status != 1)
        {
            LUAT_DEBUG_PRINT("http wait network ready");
            luat_rtos_task_sleep(1000);
        }

        if (luat_fs_fexist(EPH_FILE_PATH) != 0 && check_eph_time() == true)
        {
            size_t size = luat_fs_fsize(EPH_FILE_PATH);
            uint8_t *data = NULL;
            data = (uint8_t *)luat_heap_malloc(size);
            FILE *fp1 = luat_fs_fopen(EPH_FILE_PATH, "r");
            luat_fs_fread(data, size, 1, fp1);
            luat_fs_fclose(fp1);
            for (size_t i = 0; i < size; i = i + 512)
            {
                if (i + 512 < size)
                    luat_uart_write(UART_ID, (uint8_t *)&data[i], 512);
                else
                    luat_uart_write(UART_ID, (uint8_t *)&data[i], size - i);
                luat_rtos_task_sleep(100);
                LUAT_DEBUG_PRINT("http wait network ready aaaaaa");
            }
            luat_heap_free(data);
            lbsloc_request();
            luat_rtos_task_sleep(4 * 60 * 60 * 1000);
        }
        else
        {
            is_error = 0;
            eph_download_end = 0;
            luat_http_ctrl_t *http = luat_http_client_create(luatos_http_cb, luat_rtos_get_current_handle(), -1);
            luat_http_client_start(http, EPH_HOST, 0, 0, 1);
            fp1 = luat_fs_fopen(EPH_FILE_PATH, "w+");
            while (!eph_download_end)
            {
                luat_rtos_event_recv(luat_rtos_get_current_handle(), 0, &event, NULL, LUAT_WAIT_FOREVER);
                DBG("this is result %d", event.id);
                switch (event.id)
                {
                case EPH_HTTP_GET_HEAD_DONE:
                    done_len = 0;
                    int status = luat_http_client_get_status_code(http);
                    if (status != 200 && status != 206)
                    {
                        is_error = 1;
                        eph_download_end = 1;
                    }
                    DBG("status %d total %u", status, http->total_len);

                    break;
                case EPH_HTTP_GET_DATA:
                    done_len += event.param2;
                    luat_fs_fwrite((uint8_t *)event.param1, event.param2, 1, fp1);
                    free(event.param1);
                    break;
                case EPH_HTTP_GET_DATA_DONE:
                    eph_download_end = 1;
                    break;
                case EPH_HTTP_FAILED:
                    is_error = 1;
                    eph_download_end = 1;
                    break;
                }
            }
            luat_fs_fclose(fp1);
            luat_http_client_close(http);
            luat_http_client_destroy(&http);
            if (eph_download_end && !is_error)
            {
                FILE *fp = luat_fs_fopen(EPH_TIME_FILE, "w+");
                time_t now_time;
                time(&now_time);
                luat_fs_fwrite((void *)&now_time, sizeof(time_t), 1, fp);
                luat_fs_fclose(fp);

                size_t size = luat_fs_fsize(EPH_FILE_PATH);
                uint8_t *data = NULL;
                data = (uint8_t *)luat_heap_malloc(size);
                FILE *fp1 = luat_fs_fopen(EPH_FILE_PATH, "r");
                luat_fs_fread(data, size, 1, fp1);
                luat_fs_fclose(fp1);
                for (size_t i = 0; i < size; i = i + 512)
                {
                    if (i + 512 < size)
                        luat_uart_write(UART_ID, (uint8_t *)&data[i], 512);
                    else
                        luat_uart_write(UART_ID, (uint8_t *)&data[i], size - i);
                    luat_rtos_task_sleep(100);
                }
                luat_heap_free(data);
                lbsloc_request();
                luat_rtos_task_sleep(4 * 60 * 60 * 1000);
            }
            else
            {
                LUAT_DEBUG_PRINT("download error");
                luat_fs_remove(EPH_FILE_PATH);
                luat_rtos_task_sleep(30 * 1000);            //星历获取失败，30s后重试
            }
        }
    }
}
void task_ephemeris(void)
{
    luat_rtos_task_create(&http_task_handle, 10 * 1024, 20, "http", ephemeris_get_task, NULL, 10);
}

void network_init(void)
{
    luat_mobile_event_register_handler(luatos_mobile_event_callback);
    net_lwip_init();
    net_lwip_register_adapter(NW_ADAPTER_INDEX_LWIP_GPRS);
    network_register_set_default(NW_ADAPTER_INDEX_LWIP_GPRS);
}