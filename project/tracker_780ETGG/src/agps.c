#include "common_api.h"
#include "luat_network_adapter.h"
#include "networkmgr.h"
#include "luat_http.h"
#include "luat_mobile.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_fs.h"
#include "luat_rtc.h"
#include "gpsmsg.h"
#include "osasys.h"

#define EPH_HOST "http://download.openluat.com/9501-xingli/HXXT_GPS_BDS_AGNSS_DATA.dat"
#define HTTP_RECV_BUF_SIZE      (5300)
#define HTTP_HEAD_BUF_SIZE      (1200)

#define  EPH_TIME_FILE "/ephTime.txt"
#define  EPH_DATA_FILE "/ephData.bin"
#define  EPH_UPDATE_INTERVAL 4*3600

static luat_rtos_semaphore_t g_http_task_semaphore_handle;
HANDLE gps_update_timer = NULL;
extern nmea_msg gpsx;


luat_rtos_task_handle agps_task_handle;
enum //HTTP 枚举回调
{
	TEST_HTTP_GET_HEAD = 0,
	TEST_HTTP_GET_HEAD_DONE,
	TEST_HTTP_GET_DATA,
	TEST_HTTP_GET_DATA_DONE,
	TEST_HTTP_FAILED,
};
//http 回调
static void luatos_http_cb(int status, void *data, uint32_t len, void *param)
{
	uint8_t *head_data;
	uint8_t *body_data;
	// time_t ntime;
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
			// localtime(&ntime);
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
			// localtime(&ntime);
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
//使用HTTP 同步的方法去更新星历文件
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
		luat_rtos_event_recv(agps_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
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
static void update_eph(void)
{
    FILE* fp1 = NULL;
    FILE* fp2 = NULL;
    char *headBuf = NULL;
	char *bodyBuf = NULL;
	uint32_t headDataLen = 0;
	uint32_t bodyDataLen = 0;
	headBuf = luat_heap_malloc(HTTP_HEAD_BUF_SIZE);
	bodyBuf = luat_heap_malloc(HTTP_RECV_BUF_SIZE);
    while (!network_service_is_connect())
    {
        luat_rtos_task_sleep(1000);
    }
    int result = luat_test_http_get_sync(EPH_HOST, headBuf, HTTP_HEAD_BUF_SIZE, bodyBuf, HTTP_RECV_BUF_SIZE, &headDataLen, &bodyDataLen);
    if (result == 0)
    {
		LUAT_DEBUG_PRINT("星历更新完成");
        fp1 = luat_fs_fopen(EPH_DATA_FILE,"w+");
        fp2 = luat_fs_fopen(EPH_TIME_FILE,"w+");
        utc_timer_value_t *timeutc = OsaSystemTimeReadUtc();
        luat_fs_fwrite((uint8_t *)&timeutc->UTCsecs, sizeof(timeutc->UTCsecs), 1, fp2);
        luat_fs_fclose(fp2);
        agps_start_timer();

        luat_fs_fwrite((uint8_t *)bodyBuf, bodyDataLen, 1, fp1);

        luat_heap_free(headBuf); 
        luat_heap_free(bodyBuf); 
        luat_fs_fclose(fp1);
        
        fp1 = luat_fs_fopen(EPH_DATA_FILE, "r");
        size_t size = luat_fs_fsize(EPH_DATA_FILE);
        int8_t *data = NULL;
        data = (uint8_t *)luat_heap_malloc(size);
        luat_fs_fread(data, size, 1, fp1);
        for (size_t i = 0; i < size; i = i + 512)
        {
                if (i + 512 < size)
                    gps_writedata((uint8_t *)&data[i], 512);
                else
                    gps_writedata((uint8_t *)&data[i], size - i);
                luat_rtos_task_sleep(100);
            
           
        }
        luat_fs_fclose(fp1);
        luat_heap_free(data);
    }
    else
    {
        LUAT_DEBUG_PRINT("http client connect error");
    }
}

static bool check_eph(int timeutc,int lstimeutc)
{   
    if((timeutc - lstimeutc) >= EPH_UPDATE_INTERVAL)
        return true;
    else
        return false;
}

static void luat_http_callback(void *param)
{
	luat_rtos_semaphore_release(g_http_task_semaphore_handle);
}

void agps_start_timer(void)
{
    luat_rtos_timer_start(gps_update_timer, EPH_UPDATE_INTERVAL * 1000, 0, luat_http_callback, NULL);  
}

static void agps_https_task(void *param)
{
    int lstimeutc = 0;
    size_t size = 0;
    uint8_t *data = NULL;
    if(gpsx.gpssta == 0)
    {
        FILE* fp = luat_fs_fopen(EPH_TIME_FILE, "r");
        if(fp)
        {
            luat_fs_fread(&lstimeutc, sizeof(lstimeutc), 1, fp);
            utc_timer_value_t *timeutc = OsaSystemTimeReadUtc();
            LUAT_DEBUG_PRINT("UTC TIME old %d now %d",lstimeutc,timeutc->UTCsecs);
            if(check_eph(timeutc->UTCsecs,lstimeutc))
            {
                update_eph();
            }
            else
            {
                FILE* fp1 = luat_fs_fopen(EPH_DATA_FILE, "r");
                size = luat_fs_fsize(EPH_DATA_FILE);
                data = (uint8_t *)luat_heap_malloc(size);
                luat_fs_fread(data, size, 1, fp1);
                for (size_t i = 0; i < size; i = i + 512)
                {
                    if (i + 512 < size)
                        gps_writedata((uint8_t *)&data[i], 512);
                    else
                        gps_writedata((uint8_t *)&data[i], size - i);
                    luat_rtos_task_sleep(100);
                }
                luat_fs_fclose(fp1);
                luat_heap_free(data);
            }
            luat_fs_fclose(fp);
        }
        else
        {
            update_eph();
        }
    }
    while(1)
	{
        luat_rtos_semaphore_take(g_http_task_semaphore_handle, LUAT_WAIT_FOREVER);
		update_eph();
	}
	luat_rtos_task_delete(agps_task_handle);
}

void agps_service_init(void)//init GPS服务
{
    // https所需的栈空间会大很多
    luat_rtos_timer_create(&gps_update_timer);
    luat_rtos_semaphore_create(&g_http_task_semaphore_handle, 1);

	luat_rtos_task_create(&agps_task_handle, 2*1024, 30, "agps", agps_https_task, NULL, 0);
}

