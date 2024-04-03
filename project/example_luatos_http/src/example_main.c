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




#define HTTP_TEST_IS_SYNC  (0) // 在这里选择同步或异步http演示，测试少量数据时，可以使用同步演示方式获取数据，测试大量数据时，必须用异步演示方式获取数据

static luat_rtos_task_handle g_s_task_handle;

enum
{
	TEST_HTTP_GET_HEAD = 0,
	TEST_HTTP_GET_HEAD_DONE,
	TEST_HTTP_GET_DATA,
	TEST_HTTP_GET_DATA_DONE,
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
		luat_rtos_event_send(param, TEST_HTTP_FAILED, 0, 0, 0, 0);
		return;
    }
	switch(status)
	{
	case HTTP_STATE_GET_BODY:
		if (data)
		{
			body_data = luat_heap_malloc(len);
			memcpy(body_data, data, len);
			luat_rtos_event_send(param, TEST_HTTP_GET_DATA, (uint32_t)body_data, len, 0, 0);
		}
		else
		{
			luat_rtos_event_send(param, TEST_HTTP_GET_DATA_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_GET_HEAD:
		if (data)
		{
			head_data = luat_heap_malloc(len);
			memcpy(head_data, data, len);
			luat_rtos_event_send(param, TEST_HTTP_GET_HEAD, (uint32_t)head_data, len, 0, 0);
		}
		else
		{
			luat_rtos_event_send(param, TEST_HTTP_GET_HEAD_DONE, 0, 0, 0, 0);
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

static int luat_test_http_get_sync(char *url, char*headBuf, uint32_t headBufLen, char *bodyBuf, uint32_t bodyBufLen, uint32_t *headLen, uint32_t *bodyLen)
{
	luat_event_t event = {0};
	uint8_t is_end = 0;
	uint8_t is_error = 0;
	uint32_t head_len = 0;
	uint32_t done_len = 0;
	luat_http_ctrl_t *http = luat_http_client_create(luatos_http_cb, luat_rtos_get_current_handle(), -1);
	luat_http_client_start(http, url, 0, 0, 0);
	while (!is_end)
	{
		luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case TEST_HTTP_GET_HEAD:
			// 在这里处理head数据
			if(head_len + event.param2 > headBufLen)
			{
				LUAT_DEBUG_PRINT("head len error1");
				is_error = 1;
			}
			else
			{
				memcpy(headBuf + head_len, (void *)event.param1, event.param2);
			}
			head_len += event.param2;
			luat_heap_free((char *)event.param1);
			break;
		case TEST_HTTP_GET_HEAD_DONE:
			done_len = 0;
			LUAT_DEBUG_PRINT("status %d total %u", luat_http_client_get_status_code(http), http->total_len);
			break;
		case TEST_HTTP_GET_DATA:
			// 在这里处理body数据
			if((done_len + event.param2) > bodyBufLen)
			{
				LUAT_DEBUG_PRINT("body len error");
				is_error = 1;
			}
			else
			{
				memcpy(bodyBuf + done_len, (void *)event.param1, event.param2);
			}
			done_len += event.param2;
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
		default:
			break;
		}
	}
	luat_http_client_close(http);
	luat_http_client_destroy(&http);
	if (is_error)
	{
		*headLen = 0;
		*bodyLen = 0;
	}
	else
	{
		*headLen = head_len;
		*bodyLen = done_len;
	}
	return is_error;
}


static void luat_test_http_sync_task(void *param)
{
	char remote_domain[200];
	snprintf((char *)remote_domain, 200, "%s", "http://www.baidu.com");

	while (1)
	{
		char *headBuf = NULL;
		char *bodyBuf = NULL;
		uint32_t headDataLen = 0;
		uint32_t bodyDataLen = 0;
		headBuf = luat_heap_malloc(1024);
		bodyBuf = luat_heap_malloc(10 * 1024);
		int result = luat_test_http_get_sync(remote_domain, headBuf, 1024, bodyBuf, 10 * 1024, &headDataLen, &bodyDataLen);
		if(!result)
		{
			LUAT_DEBUG_PRINT("http sync get success %d, %d", headDataLen, bodyDataLen);
		}
		else
		{
			LUAT_DEBUG_PRINT("http sync get fail");
		}
		luat_heap_free(headBuf);
		luat_heap_free(bodyBuf);
		headBuf = NULL;
		bodyBuf = NULL;
		luat_rtos_task_sleep(60000);
	}
}

#endif

static void luat_test_init(void)
{
	luat_mobile_event_register_handler(luatos_mobile_event_callback);
#if (HTTP_TEST_IS_SYNC == 1)
	luat_rtos_task_create(&g_s_task_handle, 4 * 1024,20, "sync test", luat_test_http_sync_task, NULL, 16);
#else
	luat_rtos_task_create(&g_s_task_handle, 4 * 1024,20, "async test", luat_test_http_async_task, NULL, 16);
#endif
}

INIT_TASK_EXPORT(luat_test_init, "1");
