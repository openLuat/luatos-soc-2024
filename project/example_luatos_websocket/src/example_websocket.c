#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"

#include "luat_websocket.h"

#define WS_URL  "ws://echo.airtun.air32.cn/ws/echo"

#define WS_SEND_TIME        15000   // 15s没有事件发送一次信息
#define WS_SEND_PAYLOAD     "{\"action\": \"echo\",\"device_id\": \"%s\"}"

static luat_rtos_task_handle g_s_task_handle;

static int luat_websocket_msg_cb(luat_websocket_ctrl_t *ctrl, int arg1, int arg2)
{
    luat_websocket_ctrl_t* websocket_ctrl = (luat_websocket_ctrl_t*)ctrl;
    luat_rtos_event_send(g_s_task_handle, arg1, arg2, 0, 0, 0);
	return 0;
}

static void luat_test_websocket_task(void *param)
{
    int ret = 0;
    luat_event_t event;
    luat_websocket_ctrl_t websocket_ctrl = {0};
    char websocket_send_payload[128] = {0};
    char mobile_imei[32] = {0};
    luat_mobile_get_imei(0, mobile_imei, sizeof(mobile_imei)-1);
    
    ret = luat_websocket_init(&websocket_ctrl, network_get_last_register_adapter());
    if (ret)
    {
		LUAT_DEBUG_PRINT("websocket init FAID ret %d", ret);
		goto error;
    }
    
    luat_websocket_set_cb(&websocket_ctrl,luat_websocket_msg_cb);

    luat_websocket_connopts_t opts = {.url = WS_URL,};

    network_set_ip_invaild(&websocket_ctrl.ip_addr);
    ret = luat_websocket_set_connopts(&websocket_ctrl, &opts);
	if (ret){
		luat_websocket_release_socket(&websocket_ctrl);
		goto error;
	}

    luat_websocket_autoreconn(&websocket_ctrl, 1,3000);

    ret = luat_websocket_connect(&websocket_ctrl);
	if (ret){
		LUAT_DEBUG_PRINT("socket connect ret=%d\n", ret);
		luat_websocket_close_socket(&websocket_ctrl);
		goto error;
	}

	luat_websocket_pkg_t pkg_send = {
		.FIN = 1,
		.OPT_CODE = 0x01,
		.plen = strlen(websocket_send_payload),
		.payload = websocket_send_payload,
    };
    luat_websocket_pkg_t pkg_recv = {0};
    while (true){
        ret = luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, WS_SEND_TIME);
        if (ret==0){
            switch (event.id){
                case WEBSOCKET_MSG_TIMER_PING:
                    luat_websocket_ping(&websocket_ctrl);
                    break;
                case WEBSOCKET_MSG_RECONNECT:
                    luat_websocket_reconnect(&websocket_ctrl);
                    break;
                case WEBSOCKET_MSG_PUBLISH:
                    LUAT_DEBUG_PRINT("WEBSOCKET_PUBLISH");
                    luat_websocket_payload((char *)event.param1, &pkg_recv, 64 * 1024);
                    LUAT_DEBUG_PRINT("payload:%s len:%d\nFIN:%d OPT_CODE:0x%02x",pkg_recv.payload,pkg_recv.plen,pkg_recv.FIN,pkg_recv.OPT_CODE);
                    luat_heap_free((char *)event.param1);
                    break;
                case WEBSOCKET_MSG_CONNACK:{
                    LUAT_DEBUG_PRINT("WEBSOCKET_CONNACK");
                    memset(websocket_send_payload,0,sizeof(websocket_send_payload));
                    sprintf(websocket_send_payload, WS_SEND_PAYLOAD, mobile_imei);
                    pkg_send.plen = strlen(websocket_send_payload);
                    luat_websocket_send_frame(&websocket_ctrl, &pkg_send);
                    break;
                }
                case WEBSOCKET_MSG_RELEASE:{
                    LUAT_DEBUG_PRINT("WEBSOCKET_RELEASE");
                    break;
                }
                case WEBSOCKET_MSG_SENT :{
                    LUAT_DEBUG_PRINT("WEBSOCKET_SENT");
                    break;
                }
                case WEBSOCKET_MSG_DISCONNECT : 
                    LUAT_DEBUG_PRINT("WEBSOCKET_DISCONNECT");
                    break;
                default:
                    LUAT_DEBUG_PRINT("error event:%d param1:%d", event.id,event.param1);
                    break;
            }
        }else{
            memset(websocket_send_payload,0,sizeof(websocket_send_payload));
            memcpy(websocket_send_payload,"{\"action\": \"echo\",\"msg\": \"hello luatos\"}",strlen("{\"action\": \"echo\",\"msg\": \"hello luatos\"}"));
            pkg_send.plen = strlen(websocket_send_payload);
            luat_websocket_send_frame(&websocket_ctrl, &pkg_send);
        }
    }
error:
    while (true)
    {
        luat_rtos_task_sleep(5000);
    }
}





static void luatos_mobile_event_callback(LUAT_MOBILE_EVENT_E event, uint8_t index, uint8_t status)
{
	if (LUAT_MOBILE_EVENT_NETIF == event)
	{
		if (LUAT_MOBILE_NETIF_LINK_ON == status)
		{

		}
	}
}

static void luat_test_init(void)
{
	luat_mobile_event_register_handler(luatos_mobile_event_callback);

	luat_rtos_task_create(&g_s_task_handle, 8 * 1024, 20, "test websocket", luat_test_websocket_task, NULL, 16);
}

INIT_TASK_EXPORT(luat_test_init, "1");

