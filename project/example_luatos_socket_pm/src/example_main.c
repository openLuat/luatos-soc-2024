#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_rtos.h"
#include "luat_mcu.h"
#include "luat_mobile.h"
#include "net_lwip.h"
#include "luat_pm.h"
#include "slpman.h"

// 请访问 https://netlab.luatos.com 获取新的端口号,之后修改remote_ip port再进行编译
const char remote_ip[] = "112.125.89.8";
const int port = 43982;

//心跳内容
const char heart_beat[] = "heart_beat!";
//心跳周期
const int heart_beat_timer = 5*60*1000;

//要进入的休眠等级
#define SLEEP_MODE_LEVEL LUAT_PM_POWER_MODE_HIGH_PERFORMANCE//500微安模式
//#define SLEEP_MODE_LEVEL LUAT_PM_POWER_MODE_BALANCED      //200微安模式

enum
{
    UPLOAD_TEST_CONNECT = 1,
    UPLOAD_TEST_TX_OK = 2,
    UPLOAD_TEST_ERROR,
};

static luat_rtos_task_handle g_s_task_handle;
static network_ctrl_t *g_s_network_ctrl;
static luat_rtos_task_handle g_s_upload_test_task_handle;
static int32_t luat_test_socket_callback(void *pdata, void *param)
{
    OS_EVENT *event = (OS_EVENT *)pdata;
    LUAT_DEBUG_PRINT("%x", event->ID);
    return 0;
}


static void luat_test_task(void *param)
{
    LUAT_DEBUG_PRINT("开始测试");
    //配置电源管理，日志打印会停止，USB会被断开
    luat_pm_set_power_mode(SLEEP_MODE_LEVEL, LUAT_PM_POWER_MODE_NORMAL);
    /* 
        出现异常后默认为死机重启
        demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
        如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
        但量产出货一定要设置为出现异常重启！！！！！！！！！1
    */
    luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET); 
    net_lwip_set_tcp_rx_cache(NW_ADAPTER_INDEX_LWIP_GPRS, 32); //为了下行测试才需要打开，对于不需要高速流量的应用不要打开
    g_s_network_ctrl = network_alloc_ctrl(NW_ADAPTER_INDEX_LWIP_GPRS);
    network_init_ctrl(g_s_network_ctrl, g_s_task_handle, luat_test_socket_callback, NULL);
    network_set_base_mode(g_s_network_ctrl, 1, 15000, 1, 300, 5, 9);
    g_s_network_ctrl->is_debug = 0;	//关闭debug

#if (SOCKET_TCP_SSL==1)
    network_init_tls(g_s_network_ctrl,0);
    //设置服务器端证书
    //network_set_client_cert(ctrl,cert, certLen,const key,keylen,pwd, pwdlen);
    //设置客户端证书
    //network_set_server_cert(network_ctrl_t *ctrl, const unsigned char *cert, size_t cert_len);
#endif
    uint8_t *tx_data = malloc(1024);
    uint8_t *rx_data = malloc(1024 * 8);
    uint32_t tx_len, rx_len, cnt;
    uint64_t uplink, downlink;
    int result;
    uint8_t is_break,is_timeout;
    cnt = 0;
    if (g_s_upload_test_task_handle) //上行测试时，需要延迟一段时间
    {
        luat_rtos_task_sleep(300000);
    }
    while(1)
    {
        //luat_meminfo_sys(&all, &now_free_block, &min_free_block);
        //LUAT_DEBUG_PRINT("meminfo %d,%d,%d",all,now_free_block,min_free_block);
        LUAT_DEBUG_PRINT("等待联网，超时时间60秒");
        result = network_wait_link_up(g_s_network_ctrl, 60000);
        if (result)
        {
            LUAT_DEBUG_PRINT("未联网，继续等待");
            continue;
        }

        result = network_connect(g_s_network_ctrl, remote_ip, sizeof(remote_ip) - 1, NULL, port, 30000);
        if (!result)
        {
            result = network_tx(g_s_network_ctrl, (const uint8_t*)heart_beat, sizeof(heart_beat) - 1, 0, NULL, 0, &tx_len, 15000);
            if (!result)
            {
                LUAT_DEBUG_PRINT("已联网，进入休眠模式");

                while(!result)
                {
                    result = network_wait_rx(g_s_network_ctrl, heart_beat_timer, &is_break, &is_timeout);
                    if (!result)
                    {
                        if (!is_timeout && !is_break)
                        {
                            do
                            {
                                result = network_rx(g_s_network_ctrl, rx_data, 1024 * 8, 0, NULL, NULL, &rx_len);
                                if (rx_len > 0)
                                {
                                    //回复一条数据，表示收到了
                                    result = network_tx(g_s_network_ctrl, (const uint8_t*)rx_data, rx_len, 0, NULL, 0, &tx_len, 15000);
                                }
                            }while(!result && rx_len > 0);
                        }
                        else if (is_timeout)
                        {
                            sprintf((char*)tx_data, "heart beat cnt %u", cnt);
                            result = network_tx(g_s_network_ctrl, (const uint8_t*)heart_beat, sizeof(heart_beat) - 1, 0, NULL, 0, &tx_len, 15000);
                            cnt++;
                        }
                    }
                }
            }
        }
        LUAT_DEBUG_PRINT("网络断开，15秒后重试");
        network_close(g_s_network_ctrl, 5000);
        luat_rtos_task_sleep(15000);
    }
}


static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
    if (LUAT_MOBILE_EVENT_NETIF == event)
    {
        if (LUAT_MOBILE_NETIF_LINK_ON == status)
        {
            // uint8_t is_ipv6;
            // luat_socket_check_ready(index, &is_ipv6);
            // if (is_ipv6)
            // {
            // 	g_s_server_ip = *net_lwip_get_ip6();
            // 	LUAT_DEBUG_PRINT("%s", ipaddr_ntoa(&g_s_server_ip));
            // }
        }
    }
}

static void luat_test_init(void)
{
    luat_mobile_event_register_handler(luatos_mobile_event_callback);

    luat_rtos_task_create(&g_s_task_handle, 2 * 1024, 90, "test", luat_test_task, NULL, 16);
    luat_mobile_set_default_pdn_ipv6(1);
//	luat_mobile_set_rrc_auto_release_time(2);
}

INIT_TASK_EXPORT(luat_test_init, "1");
