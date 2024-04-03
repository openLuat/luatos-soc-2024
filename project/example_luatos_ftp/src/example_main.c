#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"
#include "luat_ftp.h"
#include "luat_fs.h"


#define FTP_HOST    		"121.43.224.154"
#define FTP_PORT			21
#define FTP_USERNAME    	"ftp_user"
#define FTP_PASSWORD		"3QujbiMG"


#define FTP_QUEUE_SIZE		32

typedef struct{
	FTP_SUCCESS_STATE_e event;
	uint8_t *Data;
} ftp_queue_t;

static luat_rtos_task_handle ftp_task_handle;
static luat_rtos_queue_t ftp_queue_handle;

static void luat_ftp_cb(luat_ftp_ctrl_t *luat_ftp_ctrl, FTP_SUCCESS_STATE_e event){
	ftp_queue_t ftp_queue = {.event = event};
	if (event == FTP_SUCCESS_DATE){
		ftp_queue.Data = luat_heap_malloc(luat_ftp_ctrl->result_buffer.Pos+1);
		memset(ftp_queue.Data, 0, luat_ftp_ctrl->result_buffer.Pos+1);
		memcpy(ftp_queue.Data , luat_ftp_ctrl->result_buffer.Data, luat_ftp_ctrl->result_buffer.Pos);
	}
	luat_rtos_queue_send(ftp_queue_handle, &ftp_queue, NULL, 0);
}

static void luat_ftp_task(void *param)
{
	int result;
	ftp_queue_t ftp_queue;

	luat_fs_init(); // 必须先初始化文件系统    

	luat_rtos_queue_create(&ftp_queue_handle, FTP_QUEUE_SIZE, sizeof(ftp_queue_t));

	result = luat_ftp_login(network_get_last_register_adapter(),FTP_HOST,FTP_PORT,FTP_USERNAME,FTP_PASSWORD,NULL,luat_ftp_cb);
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event != FTP_SUCCESS_NO_DATE){
		goto error;
	}
	result = luat_ftp_command("NOOP");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}

	result = luat_ftp_command("SYST");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("TYPE I");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("PWD");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("MKD QWER");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("CWD /QWER");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("CDUP");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("RMD QWER");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
	result = luat_ftp_command("LIST");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}

	luat_ftp_pull("/122224.txt","/122224.txt");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event != FTP_SUCCESS_NO_DATE){
		goto error;
	}

	result = luat_ftp_command("DELE /12222.txt");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}

	luat_ftp_push("/122224.txt","/12222.txt");
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event != FTP_SUCCESS_NO_DATE){
		goto error;
	}

	result = luat_ftp_close();
	luat_rtos_queue_recv(ftp_queue_handle, &ftp_queue, NULL, LUAT_WAIT_FOREVER);
	if (ftp_queue.event == FTP_SUCCESS_DATE){
		LUAT_DEBUG_PRINT("ftp result_buffer:%s",ftp_queue.Data);
		luat_heap_free(ftp_queue.Data);
	}else{
		goto error;
	}
error:
	LUAT_DEBUG_PRINT("ftp error event:%d",ftp_queue.event);
	luat_ftp_release();
	luat_rtos_task_delete(ftp_task_handle);
}

static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{
			LUAT_DEBUG_PRINT("luatos_mobile_event_callback  link ...");
			//luat_socket_check_ready(index, NULL);
		}
        else if(LUAT_MOBILE_NETIF_LINK_OFF == status || LUAT_MOBILE_NETIF_LINK_OOS == status)
        {
            LUAT_DEBUG_PRINT("luatos_mobile_event_callback  error ...");
        }
	}
}

static void luat_ftp_init(void)
{
	luat_mobile_set_sim_detect_sim0_first();
	luat_mobile_event_register_handler(luatos_mobile_event_callback);

	luat_rtos_task_create(&ftp_task_handle, 2 * 1024, 10, "ftp", luat_ftp_task, NULL, 16);
}

INIT_TASK_EXPORT(luat_ftp_init, "1");
