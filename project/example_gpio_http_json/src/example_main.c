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
#include "luat_gpio.h"
#include "luat_debug.h"
#include "luat_mobile.h"
#include "luat_network_adapter.h"
#include "networkmgr.h"
#include "luat_http.h"
#include "luat_mem.h"
#include "luat_mcu.h"

#include "cJSON.h"

#define HTTP_TEST_IS_SYNC  1

typedef struct my_body {
	size_t len;
	size_t limit;
	uint8_t *data;
}my_body_t;

static luat_rtos_task_handle g_s_task_handle;
static luat_gpio_cfg_t key_fun_struct;

enum
{
	TEST_HTTP_GET_HEAD = 0,
	TEST_HTTP_GET_HEAD_DONE,
	TEST_HTTP_GET_DATA,
	TEST_HTTP_GET_DATA_DONE,
	TEST_HTTP_SEND_BODY_START,
	TEST_HTTP_SEND_BODY,
	TEST_GPIO_IRQ,
	TEST_HTTP_FAILED,
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
	uint8_t *head_data;
	uint8_t *body_data;
    if(status < 0) 
    {
		luat_rtos_event_send(g_s_task_handle, TEST_HTTP_FAILED, 0, 0, 0, 0);
		return;
    }
	switch(status)
	{
	case HTTP_STATE_GET_BODY:
		if (data)
		{
			body_data = luat_heap_malloc(len);
			memcpy(body_data, data, len);
			luat_rtos_event_send(g_s_task_handle, TEST_HTTP_GET_DATA, (uint32_t)body_data, len, 0, 0);
		}
		else
		{
			luat_rtos_event_send(g_s_task_handle, TEST_HTTP_GET_DATA_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_GET_HEAD:
		if (data)
		{
			head_data = luat_heap_malloc(len);
			memcpy(head_data, data, len);
			luat_rtos_event_send(g_s_task_handle, TEST_HTTP_GET_HEAD, (uint32_t)head_data, len, 0, 0);
		}
		else
		{
			luat_rtos_event_send(g_s_task_handle, TEST_HTTP_GET_HEAD_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_IDLE:
		break;
	case HTTP_STATE_SEND_BODY_START:
		//如果是POST，在这里发送POST的body数据，如果一次发送不完，可以在HTTP_STATE_SEND_BODY回调里继续发送
		luat_rtos_event_send(g_s_task_handle, TEST_HTTP_SEND_BODY_START, 0, 0, 0, 0);
		break;
	case HTTP_STATE_SEND_BODY:
		//如果是POST，可以在这里发送POST剩余的body数据
		luat_rtos_event_send(g_s_task_handle, TEST_HTTP_SEND_BODY, 0, 0, 0, 0);
		break;
	default:
		break;
	}
}



#if (HTTP_TEST_IS_SYNC == 0)
static void luat_test_http_async_task(void *param)
{
	luat_event_t event = {0};
	uint8_t is_end = 0;
	uint32_t done_len = 0;
	luat_http_ctrl_t *http = luat_http_client_create(luatos_http_cb, luat_rtos_get_current_handle(), -1);
	const char remote_domain[200];
	snprintf((char *)remote_domain, 200, "%s", "http://www.baidu.com");
    LUAT_DEBUG_PRINT("print url %s", remote_domain);
	luat_http_client_start(http, remote_domain, 0, 0, 1);
	while (!is_end)
	{
		luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case TEST_HTTP_GET_HEAD:
			luat_heap_free((char *)event.param1);
			break;
		case TEST_HTTP_GET_HEAD_DONE:
			// 在这里处理http响应头
			done_len = 0;
			LUAT_DEBUG_PRINT("status %d total %u", luat_http_client_get_status_code(http), http->total_len);
			break;
		case TEST_HTTP_GET_DATA:
			// 在这里处理用户数据
			done_len += event.param2;
			luat_heap_free((char *)event.param1);
			break;
		case TEST_HTTP_GET_DATA_DONE:
			is_end = 1;
			break;
		case TEST_HTTP_FAILED:
			is_end = 1;
			break;
		default:
			break;
		}
	}
	LUAT_DEBUG_PRINT("http test end, total count %d", done_len);
	luat_http_client_close(http);
	luat_http_client_destroy(&http);
	while(1)
	{
		luat_rtos_task_sleep(60000);
	}
}

#else

static int luat_test_http_get_sync(char *url, my_body_t* req_body, my_body_t* resp_body)
{
	luat_event_t event = {0};
	uint8_t is_end = 0;
	uint8_t is_error = 0;
	uint32_t head_len = 0;
	uint32_t done_len = 0;
	luat_http_ctrl_t *http = luat_http_client_create(luatos_http_cb, NULL, -1);
	http->timeout = 5000; // 5秒超时
	
	if (req_body && req_body->len > 0)
	{
		char tmp[16];
		sprintf_(tmp, "%d", req_body->len);
		luat_http_client_set_user_head(http, "Content-Type", "application/json");
		luat_http_client_set_user_head(http, "Content-Length", tmp);
	}

	luat_http_client_start(http, url, 1, 0, 0);
	while (!is_end)
	{
		luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case TEST_HTTP_GET_HEAD:
			luat_heap_free((char *)event.param1);
			break;
		case TEST_HTTP_GET_HEAD_DONE:
			// done_len = 0;
			// LUAT_DEBUG_PRINT("status %d total %u", luat_http_client_get_status_code(http), http->total_len);
			break;
		case TEST_HTTP_GET_DATA:
			// 在这里处理body数据
			if((done_len + event.param2) > resp_body->limit)
			{
				LUAT_DEBUG_PRINT("body len error");
				is_error = 1;
			}
			else
			{
				memcpy(resp_body->data + done_len, (void *)event.param1, event.param2);
			}
			done_len += event.param2;
			resp_body->len += event.param2;
			luat_heap_free((char *)event.param1);
			break;
		case TEST_HTTP_GET_DATA_DONE:
			is_end = 1;
			break;
		case TEST_HTTP_FAILED:
			is_error = 1;
			is_end = 1;
			LUAT_DEBUG_PRINT("http error");
			break;
		case TEST_HTTP_SEND_BODY_START:
			luat_http_client_post_body(http, req_body->data, req_body->len);
			break;
		default:
			break;
		}
	}
	luat_http_client_close(http);
	luat_http_client_destroy(&http);
	// if (is_error)
	// {
	// 	*headLen = 0;
	// 	*bodyLen = 0;
	// }
	// else
	// {
	// 	*headLen = head_len;
	// 	*bodyLen = done_len;
	// }
	// if (body) {
	// 	luat_mem_free(body);
	// 	body = NULL;
	// }
	return is_error;
}


static void luat_test_http_sync_task(void *param)
{
	luat_event_t event = {0};
	static char url[1024];
	int rc = 0;

	snprintf((char *)url, 1024, "%s", "http://httpbin.air32.cn/post");

	char *bodyBuf = NULL;
	#define BODY_BUFF_SIZE (16*1024)
	bodyBuf = luat_heap_malloc(BODY_BUFF_SIZE);
	my_body_t my_req = {0};
	my_body_t my_resp = {0};
	my_resp.data = bodyBuf;
	my_resp.limit = BODY_BUFF_SIZE;
	char imei[20] = {0};
	luat_mobile_get_imei(0, imei, 20);

	// 准备好要发送的数据
	cJSON *root;
	cJSON *fmt;

	while (1)
	{
		// 这里的逻辑是, 要么等60秒, 要么GPIO触发
		rc = luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, 60*1000);
		if (rc == 0 && event.id == TEST_GPIO_IRQ) {
			
			root = cJSON_CreateObject();
			cJSON_AddStringToObject(root, "serialNumber", imei);
			cJSON_AddNumberToObject(root, "hasElectric", event.param1);

			my_req.data = cJSON_Print(root);
			my_req.len = strlen(my_req.data);
			
			LUAT_DEBUG_PRINT("目标URL %s", url);
			LUAT_DEBUG_PRINT("发送数据 %s", my_req.data);
			int result = luat_test_http_get_sync(url, &my_req, &my_resp);
			if(!result)
			{
				LUAT_DEBUG_PRINT("http sync get success %.*s", my_resp.len, my_resp.data);
			}
			else
			{
				LUAT_DEBUG_PRINT("http sync get fail");
			}
			cJSON_free(root);
			luat_heap_free(my_req.data);
		}
	}
	luat_heap_free(bodyBuf);
	luat_rtos_task_delete(NULL);
}

#endif

static uint64_t last_irq; // 记录最后一次中断时间,防抖用的
void gpio_level_irq(void *data, void* args)
{
	uint64_t now = luat_mcu_tick64();
	if (now - last_irq < 1000000) { // 注意单位是us
		return;
	}
	last_irq = now;
	int pin = (int)data;
	int level = luat_gpio_get(pin);
	LUAT_DEBUG_PRINT("pin:%d, level:%d,", pin, level);
	luat_rtos_event_send(g_s_task_handle, TEST_GPIO_IRQ, level, 0, 0, 0);
}

void key_init(void)
{
    
    luat_gpio_set_default_cfg(&key_fun_struct);
	#ifdef CHIP_EC716
    key_fun_struct.pin=HAL_GPIO_8;
	// key_fun_struct.alt_fun=4;
	#else
    key_fun_struct.pin=HAL_GPIO_8;
	#endif
    key_fun_struct.pull=Luat_GPIO_PULLUP;
    key_fun_struct.mode=Luat_GPIO_IRQ;
    key_fun_struct.irq_type=LUAT_GPIO_BOTH_IRQ;
    key_fun_struct.irq_cb=(void*)gpio_level_irq;
    luat_gpio_open(&key_fun_struct);

	LUAT_DEBUG_PRINT("GPIO %d for key", key_fun_struct.pin);
}

static void luat_http_init(void)
{
	// 初始化一个GPIO来中断
	luat_mobile_event_register_handler(luatos_mobile_event_callback);
	luat_rtos_task_create(&g_s_task_handle, 16 * 1024,20, "sync test", luat_test_http_sync_task, NULL, 128);
	// 初始化一个GPIO来中断
	key_init();
}

INIT_TASK_EXPORT(luat_http_init, "1");
