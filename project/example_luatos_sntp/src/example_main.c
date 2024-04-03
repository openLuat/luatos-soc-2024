/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */


#include "common_api.h"
#include "luat_rtos.h"
#include "luat_mem.h"
#include "luat_debug.h"
#include "luat_mobile.h"
#include "luat_network_adapter.h"


const char *sntp_servers[] = {
    "ntp.aliyun.com",
    "ntp.ntsc.ac.cn",
    "time1.cloud.tencent.com"
};

#define DEMO_NTP_RESP_SIZE        (44)
#define DEMO_SNTP_SERVER_MAXNUM   (3)
#define DEMO_SNTP_PORT            (123)

luat_rtos_task_handle task_handle;
uint8_t g_linkup_status = 0;

static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{
            g_linkup_status = 1;
		}
        else
        {
            g_linkup_status = 0;
        }
	}
}


typedef struct sntp_msg {
  uint8_t  li_vn_mode;
  uint8_t  stratum;
  uint8_t  poll;
  uint8_t  precision;
  uint32_t root_delay;
  uint32_t root_dispersion;
  uint32_t reference_identifier;
  uint32_t reference_timestamp[2];
  uint32_t originate_timestamp[2];
  uint32_t receive_timestamp[2];
  uint32_t transmit_timestamp[2];
} sntp_msg_t;

static uint32_t ntptime2u32(const uint8_t* p, int plus) {
    if (plus && p[0] == 0 && p[1] == 0 && p[2] == 0 && p[3] == 0)  {
        return 0;
    }
    uint32_t t = ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) | ((uint32_t)p[2] << 8) | (uint32_t)p[3];
    if (plus == 0) {
        return t / 4295;
    }
    if (plus == 2) {
        return t;
    }
    
    if (t > 0x83AA7E80){
        t -= 0x83AA7E80;
    }else{
        t += 0x7C558180;
    }
    return t;
}

static void task(void *param)
{
    while(!g_linkup_status)
    {
        LUAT_DEBUG_PRINT("network not ready");
        luat_rtos_task_sleep(500);
    }
	network_ctrl_t *sntp_netc = network_alloc_ctrl(network_get_last_register_adapter());
	network_init_ctrl(sntp_netc, luat_get_current_task(), NULL, NULL);
	network_set_base_mode(sntp_netc, 0, 10000, 0, 0, 0, 0);
	network_set_local_port(sntp_netc, 0);
//	network_set_debug_onoff(sntp_netc, 1);
    sntp_msg_t smsg = {0};
    
    uint8_t is_break, is_timeout;
    uint8_t is_succ = 0;
    uint32_t tx_len, rx_len;
    int result;
    int retry = 0;
    while (1)
    {
        memset(&smsg, 0, sizeof(smsg));
        smsg.li_vn_mode = 0x1B;
        result = network_connect(sntp_netc, sntp_servers[retry], strlen(sntp_servers[retry]), NULL, DEMO_SNTP_PORT, 10000);
        LUAT_DEBUG_PRINT("%d", result);
        if (!result)
        {
            result = network_tx(sntp_netc, (const uint8_t*)&smsg, sizeof(smsg), 0, NULL, 0, &tx_len, 5000);
            if (!result)
            {
                result = network_wait_rx(sntp_netc, 5000, &is_break, &is_timeout);
                LUAT_DEBUG_PRINT("%d", result);
                if (!result)
                {
                    if (!is_timeout && !is_break)
                    {
                        result = network_rx(sntp_netc, (uint8_t*)&smsg, sizeof(smsg), 0, NULL, NULL, &rx_len);
                        if (result == 0 && rx_len >= DEMO_NTP_RESP_SIZE)
                        {
                            const uint8_t *p = ((const uint8_t *)&smsg)+40;
                            uint32_t time = ntptime2u32(p, 1);
                            LUAT_DEBUG_PRINT("ok result %ld", time);
                            is_succ = 1;
                        }
                        else
                        {
                        	LUAT_DEBUG_PRINT("%d,%d", result, rx_len);
                        }
                    }
                }
                else
                {
                    LUAT_DEBUG_PRINT("响应包不合法 result");
                }
            }
        }
        else
        {
            LUAT_DEBUG_PRINT("connect fail");
        }
        network_close(sntp_netc, 5000);
        retry ++;
        if (retry >= DEMO_SNTP_SERVER_MAXNUM || 1 == is_succ)
        {
            break;
        }
    }
    network_release_ctrl(sntp_netc);

    while (1)
    {
        luat_rtos_task_sleep(1000);
        LUAT_DEBUG_PRINT("ntp done");
    }
}



static void sntp_demo_init(void)
{
    luat_mobile_event_register_handler(luatos_mobile_event_callback);
	luat_rtos_task_create(&task_handle, 5 * 1024, 50, "task", task, NULL, 0);
}

//启动sntp_demo_init，启动位置任务1级
INIT_TASK_EXPORT(sntp_demo_init, "1");
