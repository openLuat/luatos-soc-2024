// 全局变量，提供给获取基站定位信息

#include "common_api.h"
#include "sockets.h"
#include "dns.h"
#include "lwip/ip4_addr.h"
#include "netdb.h"
#include "luat_debug.h"
#include "luat_rtos.h"
#include "luat_mobile.h"
#include "lbs_service.h"
#include "FreeRTOS.h"
#include "task.h"
#include "luat_wlan.h"

#define DEMO_SERVER_UDP_IP "bs.openluat.com" // 基站定位网址
#define DEMO_SERVER_UDP_PORT 12411           // 端口

#define MOBILE_MNC (0)
#define UNICOM_MNC (1)
#define TELE_MNC (11)
#define IMSI_LEN (18)
#define MOBILE_NUM (6)
#define UNICOM_NUM (4)
#define TELE_NUM (4)

const char mobile[MOBILE_NUM][3] = {"00", "02", "04", "07", "08", "13"};
const char unicom[UNICOM_NUM][3] = {"01", "06", "09", "10"};
const char tele[TELE_NUM][3] = {"03", "05", "11", "12"};

static int8_t search_mnc(char *mnc)
{
    for (uint8_t i = 0; i < MOBILE_NUM; i++)
    {
        if (strncmp(mnc, mobile[i], 2) == 0)
        {
            return MOBILE_MNC;
        }
    }
    for (uint8_t i = 0; i < UNICOM_NUM; i++)
    {
        if (strncmp(mnc, unicom[i], 2) == 0)
        {
            return UNICOM_MNC;
        }
    }
    for (uint8_t i = 0; i < TELE_NUM; i++)
    {
        if (strncmp(mnc, tele[i], 2) == 0)
        {
            return TELE_MNC;
        }
    }
    return -1;
}
static bool ddddtoddmm(char *location, char *buf)
{
    char *integer = NULL;
    char *fraction = NULL;
    char tmpstr[15] = {0};
    float tmp = 0;
    integer = strtok(location, ".");
    if (integer)
    {
        fraction = strtok(NULL, ".");
        sprintf(tmpstr, "0.%d", atoi(fraction));
        tmp = atof(tmpstr) * 60;
        tmp = atoi(integer) * 100 + tmp;
        memset(tmpstr, 0x00, 15);
        sprintf(tmpstr, "%f", tmp);
        memcpy(buf, tmpstr, strlen(tmpstr) + 1);
        return true;
    }
    return false;
}
#define WIFI_LOC 0 // 是否开启wifi 定位
/// @brief 合宙IOT 项目productkey ，必须加上，否则定位失败
luat_location_service_result_t resultInd;
static uint8_t *productKey = "XXXXXXXXXXXXXXXXXXXXXXXXXXXX";
static luat_rtos_task_handle lbsloc_Init_handle;
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

static bool location_service_parse_response(struct am_location_service_rsp_data_t *response, uint8_t *latitude, uint8_t *longitude,
                                            uint16_t *year, uint8_t *month, uint8_t *day, uint8_t *hour, uint8_t *minute, uint8_t *second)
{
    uint8_t loc[20] = {0};
    uint32_t len = 0;

    if (response == NULL || latitude == NULL || longitude == NULL || year == NULL || month == NULL || day == NULL || hour == NULL || minute == NULL || second == NULL)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: invalid parameter\r\n");
        return FALSE;
    }

    if (!(response->result == 0 || response->result == 0xFF))
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: result fail %d\r\n", response->result);
        return FALSE;
    }

    // latitude
    len = location_service_bcd_to_str(loc, response->latitude, AM_LOCATION_SERVICE_LOCATION_BCD_LEN);
    if (len <= 0)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: latitude fail\r\n");
        return FALSE;
    }
    strncat((char *)latitude, (char *)loc, 3);
    strncat((char *)latitude, ".", 2);
    strncat((char *)latitude, (char *)(loc + 3), len - 3);
    len = location_service_bcd_to_str(loc, response->longitude, AM_LOCATION_SERVICE_LOCATION_BCD_LEN);
    if (len <= 0)
    {
        LUAT_DEBUG_PRINT("location_service_parse_response: longitude fail\r\n");
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
    memset(&resultInd, 0, sizeof(luat_location_service_result_t));
    resultInd.result = response->result;
    if (0 == response->result)
    {
        snprintf((CHAR *)resultInd.latitude, sizeof(resultInd.latitude), "%s", (CHAR *)latitude);
        snprintf((CHAR *)resultInd.longitude, sizeof(resultInd.longitude), "%s", (CHAR *)longitude);
        resultInd.year = year;
        resultInd.month = month;
        resultInd.day = day;
        resultInd.hour = hour;
        resultInd.minute = minute;
        resultInd.second = second;
    }
    return TRUE;
}

static void lbsLoc_request_task(void *param)
{
    ip_addr_t remote_ip;
    char *txbuf;
    struct sockaddr_in name;
    socklen_t sockaddr_t_size = sizeof(name);
    int ret;
    int socket_id = -1;
    struct hostent dns_result;
    struct hostent *p_result;
    int h_errnop;
    struct am_location_service_rsp_data_t locationServiceResponse;
    uint8_t latitude[20] = {0};  // 经度
    uint8_t longitude[20] = {0}; // 维度
    uint16_t year = 0;           // 年
    uint8_t month = 0;           // 月
    uint8_t day = 0;             // 日
    uint8_t hour = 0;            // 小时
    uint8_t minute = 0;          // 分钟
    uint8_t second = 0;          // 秒
    char count = 0;
    uint8_t lac_total_num = 0x01;
    uint8_t lbsLocReqBuf[200] = {0};
    uint8_t sendLen = 0;
    lbsLocReqBuf[sendLen++] = strlen(productKey);
    memcpy(&lbsLocReqBuf[sendLen], (uint8_t *)productKey, 32);
    sendLen = sendLen + strlen(productKey);
#if WIFI_LOC
    lbsLocReqBuf[sendLen++] = 0x38;
#else
    lbsLocReqBuf[sendLen++] = 0x28;
#endif
    char imeiBuf[16] = {0};
    luat_mobile_get_imei(0, imeiBuf, 16);
    uint8_t imeiBcdBuf[8] = {0};
    imeiToBcd((uint8_t *)imeiBuf, 15, imeiBcdBuf);
    memcpy(&lbsLocReqBuf[sendLen], (uint8_t *)imeiBcdBuf, 8);
    sendLen = sendLen + 8;
    char muidBuf[64] = {0};
    luat_mobile_get_muid(muidBuf, sizeof(muidBuf));
    lbsLocReqBuf[sendLen++] = strlen(muidBuf);
    memcpy(&lbsLocReqBuf[sendLen], (uint8_t *)muidBuf, strlen(muidBuf));
    sendLen = sendLen + strlen(muidBuf);
    luat_mobile_cell_info_t cell_info = {0};
    ret = luat_mobile_get_cell_info_async(5);
    if (ret != 0)
    {
        LUAT_DEBUG_PRINT("cell_info_async false\r\n");
        goto quit;
    }
    luat_rtos_task_sleep(5000);
    ret = luat_mobile_get_last_notify_cell_info(&cell_info);
    // ret = luat_mobile_get_cell_info(&cell_info);//同步方式获取cell_info
    if (ret != 0)
    {
        LUAT_DEBUG_PRINT("get last notify cell_info false\r\n");
        goto quit;
    }

    uint8_t mnc = 0;

    uint8_t lac_num_index = sendLen++;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.tac >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.tac & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.mcc >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.mcc & 0XFF;

    int buf = 10;
    char imsi[IMSI_LEN + 1] = {0};
    int result = luat_mobile_get_imsi(0, imsi, IMSI_LEN + 1);
    if (result != -1)
    {
        char mncTmp[3] = {0};
        memcpy(mncTmp, imsi + 3, 2);
        int8_t mncSearchResult = search_mnc(mncTmp);
        if (mncSearchResult != -1)
        {
            lbsLocReqBuf[sendLen++] = mncSearchResult;
        }
        else
        {
            mnc = cell_info.lte_service_info.mnc & 0xFF;
            if (mnc > 10)
            {
                char buf[3] = {0};
                snprintf(buf, 3, "%02x", mnc);
                int mncRet = atoi(buf);
                lbsLocReqBuf[sendLen++] = mncRet;
            }
            else
            {
                lbsLocReqBuf[sendLen++] = mnc;
            }
        }
    }
    else
    {
        mnc = cell_info.lte_service_info.mnc & 0xFF;
        if (mnc > 10)
        {
            char buf[3] = {0};
            snprintf(buf, 3, "%02x", mnc);
            int mncRet = atoi(buf);
            lbsLocReqBuf[sendLen++] = mncRet;
        }
        else
        {
            lbsLocReqBuf[sendLen++] = mnc;
        }
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
    uint8_t rssi_index = sendLen++;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 24) & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 16) & 0xFF;
    lbsLocReqBuf[sendLen++] = (cell_info.lte_service_info.cid >> 8) & 0xFF;
    lbsLocReqBuf[sendLen++] = cell_info.lte_service_info.cid & 0xFF;

    uint8_t lac1Num = 0;
    uint8_t index = 0;
    uint16_t lte_tac_table[LUAT_MOBILE_CELL_MAX_NUM] = {0};
    for (uint8_t i = 0; i < cell_info.lte_neighbor_info_num; i++)
    {
        if (cell_info.lte_info[i].tac == cell_info.lte_service_info.tac)
        {
            int16_t rsrp = cell_info.lte_info[i].rsrp + 144;
            uint8_t ret_rssi = 0;
            if (rsrp > 0 && rsrp < 31)
            {
                ret_rssi = rsrp;
            }
            else if (rsrp > 31)
            {
                ret_rssi = 31;
            }
            else if (rsrp < 0)
            {
                ret_rssi = 0;
            }
            lbsLocReqBuf[sendLen++] = ret_rssi;
            lbsLocReqBuf[sendLen++] = (cell_info.lte_info[i].cid >> 24) & 0xFF;
            lbsLocReqBuf[sendLen++] = (cell_info.lte_info[i].cid >> 16) & 0xFF;
            lbsLocReqBuf[sendLen++] = (cell_info.lte_info[i].cid >> 8) & 0xFF;
            lbsLocReqBuf[sendLen++] = cell_info.lte_info[i].cid & 0xFF;
            lac1Num++;
        }
        else
        {
            lte_tac_table[index] = cell_info.lte_info[i].tac;
            index++;
        }
    }

    lbsLocReqBuf[rssi_index] = (lac1Num << 5) + retRssi;

    if (index > 0)
    {
        for (size_t i = 0; i < index - 1; i++)
        {
            for (size_t j = 0; j < index - i - 1; j++)
            {
                if (lte_tac_table[j] > lte_tac_table[j + 1])
                {
                    uint16_t temp = lte_tac_table[j];
                    lte_tac_table[j] = lte_tac_table[j + 1];
                    lte_tac_table[j + 1] = temp;
                }
            }
        }
        uint16_t temp = lte_tac_table[0];
        uint8_t lacxNum = 1;
        for (uint8_t i = 1; i < cell_info.lte_neighbor_info_num; i++)
        {
            if (temp == lte_tac_table[i])
            {
                lacxNum++;
            }
            else
            {
                uint8_t is_first_lac = 1;
                for (uint8_t j = 0; j < cell_info.lte_neighbor_info_num; j++)
                {
                    if (cell_info.lte_info[j].tac == temp)
                    {
                        int16_t rsrp = cell_info.lte_info[j].rsrp + 144;
                        uint8_t ret_rssi = 0;
                        if (rsrp > 0 && rsrp < 31)
                        {
                            ret_rssi = rsrp;
                        }
                        else if (rsrp > 31)
                        {
                            ret_rssi = 31;
                        }
                        else if (rsrp < 0)
                        {
                            ret_rssi = 0;
                        }
                        if (is_first_lac)
                        {
                            lac_total_num++;
                            is_first_lac = 0;
                            lbsLocReqBuf[sendLen++] = (cell_info.lte_info[j].tac >> 8) & 0xFF;
                            lbsLocReqBuf[sendLen++] = cell_info.lte_info[j].tac & 0xFF;
                            lbsLocReqBuf[sendLen++] = (cell_info.lte_info[j].mcc >> 8) & 0xFF;
                            lbsLocReqBuf[sendLen++] = cell_info.lte_info[j].mcc & 0XFF;

                            mnc = cell_info.lte_info[j].mnc & 0xFF;
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
                            lbsLocReqBuf[sendLen++] = (lacxNum - 1 << 5) + ret_rssi;
                        }
                        else
                        {
                            lbsLocReqBuf[sendLen++] = ret_rssi;
                        }
                        lbsLocReqBuf[sendLen++] = (cell_info.lte_info[j].cid >> 24) & 0xFF;
                        lbsLocReqBuf[sendLen++] = (cell_info.lte_info[j].cid >> 16) & 0xFF;
                        lbsLocReqBuf[sendLen++] = (cell_info.lte_info[j].cid >> 8) & 0xFF;
                        lbsLocReqBuf[sendLen++] = cell_info.lte_info[j].cid & 0xFF;
                    }
                }
                temp = lte_tac_table[i];
                if (temp == 0)
                    break;
                lacxNum = 1;
            }
        }
    }

    lbsLocReqBuf[lac_num_index] = lac_total_num;

#if WIFI_LOC
    luat_wifiscan_set_info_t wifiscan_set_info;
    luat_wifisacn_get_info_t wifiscan_get_info;
    wifiscan_set_info.maxTimeOut = 10000;
    wifiscan_set_info.round = 1;
    wifiscan_set_info.maxBssidNum = 10;
    wifiscan_set_info.scanTimeOut = 5;
    wifiscan_set_info.wifiPriority = LUAT_WIFISCAN_DATA_PERFERRD;
    wifiscan_set_info.channelCount = 1;
    wifiscan_set_info.channelRecLen = 280;
    wifiscan_set_info.channelId[0] = 0;
    ret = luat_get_wifiscan_cell_info(&wifiscan_set_info, &wifiscan_get_info);
    if (ret != 0)
    {
        LUAT_DEBUG_PRINT("get wifiscan cell info false\r\n");
        goto quit;
    }
    else
    {
        if (wifiscan_get_info.bssidNum > 0)
        {
            lbsLocReqBuf[sendLen++] = wifiscan_get_info.bssidNum;
            for (int i = 0; i < wifiscan_get_info.bssidNum; i++)
            {
                for (int j = 0; j < 6; j++)
                {
                    lbsLocReqBuf[sendLen++] = wifiscan_get_info.bssid[i][j];
                }
                lbsLocReqBuf[sendLen++] = wifiscan_get_info.rssi[i] + 255;
            }
        }
        else
        {
            LUAT_DEBUG_PRINT("get wifiscan cell info wifiscan_get_info.bssidNum %d\r\n", wifiscan_get_info.bssidNum);
            //goto quit;
        }
    }
#endif
    txbuf = malloc(128);
    ret = lwip_gethostbyname_r(DEMO_SERVER_UDP_IP, &dns_result, txbuf, 128, &p_result, &h_errnop);
    if (!ret)
    {
        remote_ip = *((ip_addr_t *)dns_result.h_addr_list[0]);
        free(txbuf);
    }
    else
    {
        free(txbuf);
        LUAT_DEBUG_PRINT("dns fail\r\n");
        goto quit;
    }

    socket_id = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    struct timeval timeout;
    timeout.tv_sec = 15;
    timeout.tv_usec = 0;
    setsockopt(socket_id, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
    memset(&name, 0, sizeof(name));
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = remote_ip.u_addr.ip4.addr;
    name.sin_port = htons(DEMO_SERVER_UDP_PORT);
    while (count < 3)
    {
        ret = sendto(socket_id, lbsLocReqBuf, sendLen, 0, (const struct sockaddr *)&name, sockaddr_t_size);
        if (ret == sendLen)
        {
            LUAT_DEBUG_PRINT("lbsLocSendRequest send  success\r\n");
            ret = recv(socket_id, &locationServiceResponse, sizeof(struct am_location_service_rsp_data_t), 0);
            if (ret > 0)
            {
                LUAT_DEBUG_PRINT("location_service_task: rcv response result %d\r\n", locationServiceResponse.result);
                switch (locationServiceResponse.result)
                {
                case LBSLOC_SUCCESS:
                case WIFILOC_SUCCESS:
                    if (sizeof(struct am_location_service_rsp_data_t) == ret)
                    {
                        if (location_service_parse_response(&locationServiceResponse, latitude, longitude, &year, &month, &day, &hour, &minute, &second) == TRUE)
                        {
                            LUAT_DEBUG_PRINT("LbsLoc_result %d,latitude:%s,longitude:%s,year:%d,month:%d,day:%d,hour:%d,minute:%d,second:%d\r\n", locationServiceResponse.result, latitude, longitude, year, month, day, hour, minute, second);
                            char data_buf[60] = {0};
                            memset(data_buf, 0x00, 60);
                            char lat[20] = {0};
                            char lng[20] = {0};
                            ddddtoddmm(latitude, lat);
                            ddddtoddmm(longitude, lng);
                            snprintf(data_buf, 60, "$AIDPOS,%s,N,%s,E,1.0\r\n", lat, lng);
                            LUAT_DEBUG_PRINT("location_service_task: AIDPOS %s", data_buf);
                            gps_writedata((uint8_t *)data_buf, strlen(data_buf));
                            goto quit;
                        }
                        else
                        {
                            LUAT_DEBUG_PRINT("location_service_task: rcv response, but process fail\r\n");
                        }
                    }
                    break;
                case UNKNOWN_LOCATION:
                    LUAT_DEBUG_PRINT("基站数据库查询不到所有小区的位置信息\r\n");
                    LUAT_DEBUG_PRINT("在电脑浏览器中打开http://bs.openluat.com/，根据此条打印中的小区信息，手动查找小区位置, mcc: %x, mnc: %x, lac: %d, ci: %d\r\n", cell_info.lte_service_info.mcc, cell_info.lte_service_info.mnc, cell_info.lte_service_info.tac, cell_info.lte_service_info.cid);
                    LUAT_DEBUG_PRINT("如果手动可以查到位置，则服务器存在BUG，直接向技术人员反映问题\r\n");
                    LUAT_DEBUG_PRINT("如果手动无法查到位置，则基站数据库还没有收录当前设备的小区位置信息，向技术人员反馈，我们会尽快收录\r\n");
                    break;
                case PERMISSION_ERROR:
                    LUAT_DEBUG_PRINT("权限错误，请联系官方人员尝试定位问题 %d\r\n", locationServiceResponse.result);
                    break;
                case UNKNOWN_ERROR:
                    LUAT_DEBUG_PRINT("未知错误，请联系官方人员尝试定位问题");
                    break;
                default:
                    break;
                }
            }
            else
            {
                LUAT_DEBUG_PRINT("location_service_task: rcv fail %d", ret);
            }
        }
        else
        {
            LUAT_DEBUG_PRINT("lbsLocSendRequest send lbsLoc request fail\r\n");
        }
        count++;
    }
quit:
    memset(latitude, 0, 20);
    memset(longitude, 0, 20);
    year = 0;
    month = 0;
    day = 0;
    hour = 0;
    minute = 0;
    second = 0;
    LUAT_DEBUG_PRINT("lbsLoc request quit\r\n");
    close(socket_id);
    socket_id = -1;
    luat_rtos_task_delete(lbsloc_Init_handle);
}

// void lbsloc_Init(void)
// {

// }
void luat_lbs_task_init()
{
    if (lbsloc_Init_handle==NULL)
    {
       luat_rtos_task_create(&lbsloc_Init_handle, 4 * 2048, 80, "lbsloc_Init", lbsLoc_request_task, NULL, NULL);
    }  // am_location_service_init(0, 1, 1);
}

void luat_lbs_get_info(void *arg)
{
    luat_location_service_result_t *info = (luat_location_service_result_t *)arg;

    info->result = resultInd.result;

    memcpy(info->latitude, resultInd.latitude, sizeof(resultInd.latitude));
    memcpy(info->longitude, resultInd.longitude, sizeof(resultInd.longitude));

    info->year = resultInd.year;
    info->month = resultInd.month;
    info->day = resultInd.day;
    info->hour = resultInd.hour;
    info->minute = resultInd.minute;
    info->second = resultInd.second;
}