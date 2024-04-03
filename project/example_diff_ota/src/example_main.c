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

/**
 *
 * 如果使用差分升级，luatools需要2.2.15及以上版本，进入选项及工具->SOC差分/整包升级包制作工具，
 * 旧版固件选择当前模块烧录的SOC包（不是binpkg包），新版固件选择要升级的SOC包（不是binpkg包），用户标识可以填版本号，也可以不填，看自己的代码对升级版本是否有控制，如果用合宙IOT升级服务，则必须填写版本号！！！
 * 选好输出路径，点生成，在输出目录下生成相应的xxx.sota文件为升级包，放在服务器上下载即可升级
 *
 *
 * PS：和全量升级代码是一样的，只是项目的配置，ota包生成有区别

*/

#include "common_api.h"

#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_flash.h"
#include "luat_crypto.h"
#include "luat_fota.h"
#include "mem_map.h"
#include "luat_ymodem.h"
#include "luat_uart.h"
#include "luat_mobile.h"
#include "luat_network_adapter.h"
#include "networkmgr.h"
#include "luat_http.h"
#include "luat_mem.h"

//#define OTA_BY_HTTP		//OTA文件通过自定义HTTP服务器下载
//#define OTA_BY_IOT		//OTA文件通过合宙IOT服务器下载
 #define OTA_BY_USB			//OTA文件通过USB虚拟串口下载，打开串口工具时需要勾选DTR或者RTS，否则模块发不出同步码
#define UART_ID LUAT_VUART_ID_0
luat_rtos_task_handle g_s_task_handle;
luat_rtos_task_handle g_s_version_print_task_handle;
#define PROJECT_VERSION  "1.0.1"                  					//使用合宙iot升级的话此字段必须存在，并且强制固定格式为x.x.x, x可以为任意的数字
#ifdef OTA_BY_USB
enum
{
	OTA_NEW_DATA = USER_EVENT_ID_START+1,
};
static void luat_uart_recv_cb(int uart_id, uint32_t data_len)
{
	luat_rtos_event_send(g_s_task_handle, OTA_NEW_DATA, 0, 0, 0, 0);
}
static void luat_ymodem_cb(uint8_t *data, uint32_t data_len)
{
	if (data)
	{
		if (data_len)
		{
			LUAT_DEBUG_PRINT("get data %u", data_len);
			luat_fota_write(data, data_len);
		}
		else
		{
			LUAT_DEBUG_PRINT("file name %s, but no need", data);
		}
	}
	else
	{
		if (data_len)
		{
			LUAT_DEBUG_PRINT("file len %u", data_len);
		}
		else
		{
			LUAT_DEBUG_PRINT("end");
			if (!luat_fota_done())
			{
				LUAT_DEBUG_PRINT("OTA包写入完成，重启模块!");
				luat_os_reboot(0);
			}
		}
	}
}

#else


/*
    注意事项！！！！！！！！！！！！！

    重要说明！！！！！！！！！！！！！！
    设备如果升级的话，设备运行的版本必须和差分文件时所选的旧版固件一致
    举例，用1.0.0和3.0.0差分生成的差分包，必须也只能给运行1.0.0软件的设备用
*/

/* 
    第一种升级方式 ！！！！！！！！！！
    这种方式通过设备自身上报的版本名称修改，不需要特别对设备和升级包做版本管理                  用合宙iot平台升级时，推荐使用此种升级方式

    PROJECT_VERSION:用于区分不同软件版本，同时也用于区分固件差分基础版本

    假设：
        现在有两批模块在运行不同的基础版本，一个版本号为1.0.0，另一个版本号为2.0.0
        现在这两个版本都需要升级到3.0.0，则需要做两个差分包，一个是从1.0.0升级到3.0.0，另一个是从2.0.0升级到3.0.0
        
        但是因为合宙IOT是通过firmware来区分不同版本固件，只要请求时firmware相同，版本号比设备运行的要高，就会下发升级文件

        所以需要对firmware字段做一点小小的操作，将PROJECT_VERSION加入字段中来区分基础版本不同的差分包

        添加字段前的fireware字段
                从1.0.0升级到3.0.0生成的firmware字段为TEST_FOTA_CSDK_EC7XX
                从2.0.0升级到3.0.0生成的firmware字段为TEST_FOTA_CSDK_EC7XX

        添加字段后的fireware字段
                从1.0.0升级到3.0.0生成的firmware字段为1.0.0_TEST_FOTA_CSDK_EC7XX
                从2.0.0升级到3.0.0生成的firmware字段为2.0.0_TEST_FOTA_CSDK_EC7XX

        这样操作后，当两个升级包放上去，1.0.0就算被放进了2.0.0_TEST_FOTA_CSDK_EC7XX的升级列表里，也会因为自身上报的字段和所在升级列表的固件名称不一致而被服务器拒绝升级
*/



/* 
    第二种升级方式 ！！！！！！！！！！
    这种方式可以在合宙iot平台统计升级成功的设备数量，但是需要用户自身对设备和升级包做版本管理     用合宙iot平台升级时，不推荐使用此种升级方式

    PROJECT_VERSION:用于区分不同软件版本

    假设：
        现在有两批模块在运行不同的基础版本，一个版本号为1.0.0，另一个版本号为2.0.0
        现在这两个版本都需要升级到3.0.0，则需要做两个差分包，一个是从1.0.0升级到3.0.0，另一个是从2.0.0升级到3.0.0

        合宙IOT通过firmware来区分不同版本固件，只要请求时firmware相同，版本号比设备运行的要高，就会下发升级文件
        
        从1.0.0升级到3.0.0生成的firmware字段为TEST_FOTA_CSDK_EC7XX
        从2.0.0升级到3.0.0生成的firmware字段为TEST_FOTA_CSDK_EC7XX

        如果将运行1.0.0的设备imei放进了2.0.0-3.0.0的升级列表中，因为设备上报的firmware字段相同，服务器也会将2.0.0-3.0.0的差分软件下发给运行1.0.0软件的设备
        所以用户必须自身做好设备版本区分，否则会一直请求错误的差分包，导致流量损耗
*/


/* 
    第二种升级方式相对于第一种方式，多了一个合宙iot平台统计升级成功设备数量的功能，但需要用户自身做好设备、软件版本管理
    
    总体来说弊大于利

    推荐使用第一种方式进行升级，此demo也默认使用第一种方式进行升级演示！！！！！！
*/



#ifdef OTA_BY_IOT

#define OTA_URL "http://iot.openluat.com"
#define PROJECT_KEY "XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"  	//修改为自己iot上面的PRODUCT_KEY，这里是一个错误的，使用合宙iot升级的话此字段必须存在
#define PROJECT_NAME "TEST_FOTA"                  					//使用合宙iot升级的话此字段必须存在，可以任意修改，但和升级包的必须一致

#endif

#ifdef OTA_BY_HTTP

#define OTA_URL        "http://XXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXXX"	//使用自定义url升级的话修改此字段即可

#endif

enum
{
	OTA_HTTP_GET_HEAD_DONE = 1,
	OTA_HTTP_GET_DATA,
	OTA_HTTP_GET_DATA_DONE,
	OTA_HTTP_FAILED,
};

static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{

		}
	}
}

static void luatos_http_cb(int status, void *data, uint32_t len, void *param)
{
	uint8_t *ota_data = NULL;
    if(status < 0) 
    {
        LUAT_DEBUG_PRINT("http failed! %d", status);
		luat_rtos_event_send(param, OTA_HTTP_FAILED, 0, 0, 0, 0);
		return;
    }
	switch(status)
	{
	case HTTP_STATE_GET_BODY:
		if (data)
		{
			ota_data = malloc(len);
			memcpy(ota_data, data, len);
			luat_rtos_event_send(param, OTA_HTTP_GET_DATA, (uint32_t)ota_data, len, 0, 0);
		}
		else
		{
			luat_rtos_event_send(param, OTA_HTTP_GET_DATA_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_GET_HEAD:
		if (data)
		{
			LUAT_DEBUG_PRINT("%s", data);
		}
		else
		{
			luat_rtos_event_send(param, OTA_HTTP_GET_HEAD_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_IDLE:
		break;
	case HTTP_STATE_SEND_BODY_START:
		//如果是POST，在这里发送POST的body数据，如果一次发送不完，可以在HTTP_STATE_SEND_BODY回调里继续发送
		break;
	case HTTP_STATE_SEND_BODY:
		//如果是POST，可以在这里发送POST剩余的body数据
		break;
	default:
		break;
	}
}


#endif

static void luat_test_task(void *param)
{
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
#ifdef OTA_BY_USB
	uint8_t temp[128];
    luat_uart_t uart = {
        .id = UART_ID,
        .baud_rate = 3000000,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
    LUAT_DEBUG_PRINT("setup result %d", luat_uart_setup(&uart));
    LUAT_DEBUG_PRINT("ctrl result %d", luat_uart_ctrl(UART_ID, LUAT_UART_SET_RECV_CALLBACK, luat_uart_recv_cb));
    void *ymodem_handler = luat_ymodem_create_handler(luat_ymodem_cb);
    luat_event_t event;
    uint32_t read_len;
    uint8_t ack, flag, file_ok, all_done;
    int result;
    luat_fota_init(0,0,NULL,NULL,0);
    while(1)
    {
    	if (luat_rtos_event_recv(g_s_task_handle, OTA_NEW_DATA, &event, NULL, 250))
    	{
    		DBG("!");
    		luat_ymodem_reset(ymodem_handler);
    		luat_uart_write(UART_ID, "C", 1);
    	}
    	else
    	{
    		read_len = luat_uart_read(UART_ID, temp, 128);
    		while(read_len > 0)
    		{
    			result = luat_ymodem_receive(ymodem_handler, temp, read_len, &ack, &flag, &file_ok, &all_done);
    			if (result)
    			{
    				LUAT_DEBUG_PRINT("%d,%d,%d", result, ack, flag);
    			}
    			if (ack)
    			{
    				luat_uart_write(UART_ID, &ack, 1);
    			}
    			if (flag)
    			{
    				luat_uart_write(UART_ID, &flag, 1);
    			}
    			read_len = luat_uart_read(UART_ID, temp, 128);
    		}

    	}
    }

#endif
#if defined (OTA_BY_IOT) || defined (OTA_BY_HTTP)
	extern const char *soc_get_sdk_type(void);
	luat_event_t event;
	int result, is_error;
	uint8_t is_end = 0;
	/* 
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
	size_t all,now_free_block,min_free_block,done_len;
	luat_http_ctrl_t *http = luat_http_client_create(luatos_http_cb, luat_rtos_get_current_handle(), -1);
	char remote_domain[200];
#ifdef OTA_BY_IOT
	char imei[16] = {0};
	luat_mobile_get_imei(0, imei, 15);
	// 第一种升级方式
	snprintf(remote_domain, 200, "%s/api/site/firmware_upgrade?project_key=%s&imei=%s&device_key=&firmware_name=%s_%s_%s_%s&version=%s", OTA_URL, PROJECT_KEY, imei, PROJECT_VERSION, PROJECT_NAME, soc_get_sdk_type(), "EC7XX", PROJECT_VERSION);

	// 第二种升级方式
 	// snprintf(remote_domain, 200, "%s/api/site/firmware_upgrade?project_key=%s&imei=%s&device_key=&firmware_name=%s_%s_%s&version=%s", OTA_URL, PROJECT_KEY, imei, PROJECT_NAME, soc_get_sdk_type(), "EC7XX", PROJECT_VERSION);
#endif

#ifdef OTA_BY_HTTP
	snprintf(remote_domain, 200, "%s", OTA_URL);
#endif
	
    LUAT_DEBUG_PRINT("print url %s", remote_domain);
	luat_fota_init(0,0,NULL,NULL,0);
	luat_http_client_start(http, remote_domain, 0, 0, 1);
	while (!is_end)
	{
		luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case OTA_HTTP_GET_HEAD_DONE:
			done_len = 0;
			LUAT_DEBUG_PRINT("status %d total %u", luat_http_client_get_status_code(http), http->total_len);
			break;
		case OTA_HTTP_GET_DATA:
			done_len += event.param2;
			result = luat_fota_write((uint8_t *)event.param1, event.param2);
			luat_heap_free((uint8_t *)event.param1);
			break;
		case OTA_HTTP_GET_DATA_DONE:
			is_end = 1;
			break;
		case OTA_HTTP_FAILED:
			is_end = 1;
			break;
		default:
			break;
		}
	}

	is_error = luat_fota_done();
	luat_http_client_close(http);
	luat_http_client_destroy(&http);
	luat_rtos_task_sleep(1000);
    if(is_error != 0)
    {
		LUAT_DEBUG_PRINT("OTA 测试失败");
    }
	else
	{
		LUAT_DEBUG_PRINT("OTA包写入完成，重启模块!");
    	luat_os_reboot(0);
	}
	luat_meminfo_sys(&all, &now_free_block, &min_free_block);
	LUAT_DEBUG_PRINT("meminfo %d,%d,%d",all,now_free_block,min_free_block);
	while(1)
	{
		luat_rtos_task_sleep(60000);
	}
#endif
}

static void version_print_task(void *args)
{
	while (1)
	{
		luat_rtos_task_sleep(5000);
		LUAT_DEBUG_PRINT("version: %s", PROJECT_VERSION);
	}
}

static void luat_test_init(void)
{
#if defined (OTA_BY_IOT) || defined (OTA_BY_HTTP)
	luat_mobile_event_register_handler(luatos_mobile_event_callback);
#endif
	luat_rtos_task_create(&g_s_task_handle, 4 * 1024, 50, "test", luat_test_task, NULL, 0);
	luat_rtos_task_create(&g_s_version_print_task_handle, 4 * 1024, 20, "test2", version_print_task, NULL, 0);
}

INIT_TASK_EXPORT(luat_test_init, "1");



