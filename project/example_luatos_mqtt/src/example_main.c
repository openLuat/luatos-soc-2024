#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"

#include "libemqtt.h"
#include "luat_mqtt.h"

#define MQTT_DEMO_SSL 			1
#define MQTT_DEMO_AUTOCON 		1
#define MQTT_DEMO_PUB_QOS 		1
#if (MQTT_DEMO_SSL == 1)
#define MQTT_HOST    	"airtest.openluat.com"   				// MQTTS服务器的地址和端口号
#define MQTT_PORT		8883
#define CLIENT_ID    	"123456789"          
#define USERNAME    	"user"                 
#define PASSWORD    	"password"   
#else
#define MQTT_HOST    	"lbsmqtt.airm2m.com"   				// MQTT服务器的地址和端口号
#define MQTT_PORT		1884
#define CLIENT_ID    	"123456789"         //替换自己的CLIENT_ID ,请看一下本.c代码的 183~190行,本demo 的CLIENT_ID 使用的是设备的imei号
#define USERNAME    	"username"                 
#define PASSWORD    	"password"   
#endif 

static char mqtt_sub_topic[] = "test_topic";
// static char mqtt_sub_topic1[] = "test_topic1";
// static char mqtt_sub_topic2[] = "test_topic2";
static char mqtt_pub_topic[] = "test_topic";
static char mqtt_send_payload[] = "hello mqtt_test!!!";

// static char mqtt_will_topic[] = "test_will";				// 测试遗嘱
// static char mqtt_will_payload[] = "hello, i was dead";

#if (MQTT_DEMO_SSL == 1)
static const char *testCaCrt = \
{
    \
    "-----BEGIN CERTIFICATE-----\r\n"
    "MIIDUTCCAjmgAwIBAgIJAPPYCjTmxdt/MA0GCSqGSIb3DQEBCwUAMD8xCzAJBgNV\r\n" \
    "BAYTAkNOMREwDwYDVQQIDAhoYW5nemhvdTEMMAoGA1UECgwDRU1RMQ8wDQYDVQQD\r\n" \
    "DAZSb290Q0EwHhcNMjAwNTA4MDgwNjUyWhcNMzAwNTA2MDgwNjUyWjA/MQswCQYD\r\n" \
    "VQQGEwJDTjERMA8GA1UECAwIaGFuZ3pob3UxDDAKBgNVBAoMA0VNUTEPMA0GA1UE\r\n" \
    "AwwGUm9vdENBMIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAzcgVLex1\r\n" \
    "EZ9ON64EX8v+wcSjzOZpiEOsAOuSXOEN3wb8FKUxCdsGrsJYB7a5VM/Jot25Mod2\r\n" \
    "juS3OBMg6r85k2TWjdxUoUs+HiUB/pP/ARaaW6VntpAEokpij/przWMPgJnBF3Ur\r\n" \
    "MjtbLayH9hGmpQrI5c2vmHQ2reRZnSFbY+2b8SXZ+3lZZgz9+BaQYWdQWfaUWEHZ\r\n" \
    "uDaNiViVO0OT8DRjCuiDp3yYDj3iLWbTA/gDL6Tf5XuHuEwcOQUrd+h0hyIphO8D\r\n" \
    "tsrsHZ14j4AWYLk1CPA6pq1HIUvEl2rANx2lVUNv+nt64K/Mr3RnVQd9s8bK+TXQ\r\n" \
    "KGHd2Lv/PALYuwIDAQABo1AwTjAdBgNVHQ4EFgQUGBmW+iDzxctWAWxmhgdlE8Pj\r\n" \
    "EbQwHwYDVR0jBBgwFoAUGBmW+iDzxctWAWxmhgdlE8PjEbQwDAYDVR0TBAUwAwEB\r\n" \
    "/zANBgkqhkiG9w0BAQsFAAOCAQEAGbhRUjpIred4cFAFJ7bbYD9hKu/yzWPWkMRa\r\n" \
    "ErlCKHmuYsYk+5d16JQhJaFy6MGXfLgo3KV2itl0d+OWNH0U9ULXcglTxy6+njo5\r\n" \
    "CFqdUBPwN1jxhzo9yteDMKF4+AHIxbvCAJa17qcwUKR5MKNvv09C6pvQDJLzid7y\r\n" \
    "E2dkgSuggik3oa0427KvctFf8uhOV94RvEDyqvT5+pgNYZ2Yfga9pD/jjpoHEUlo\r\n" \
    "88IGU8/wJCx3Ds2yc8+oBg/ynxG8f/HmCC1ET6EHHoe2jlo8FpU/SgGtghS1YL30\r\n" \
    "IWxNsPrUP+XsZpBJy/mvOhE5QXo6Y35zDqqj8tI7AGmAWu22jg==\r\n" \
    "-----END CERTIFICATE-----"
};
static const char *testclientCert = \
{
	\
	"-----BEGIN CERTIFICATE-----\r\n"
	"MIIDEzCCAfugAwIBAgIBATANBgkqhkiG9w0BAQsFADA/MQswCQYDVQQGEwJDTjER\r\n"
	"MA8GA1UECAwIaGFuZ3pob3UxDDAKBgNVBAoMA0VNUTEPMA0GA1UEAwwGUm9vdENB\r\n"
	"MB4XDTIwMDUwODA4MDY1N1oXDTMwMDUwNjA4MDY1N1owPzELMAkGA1UEBhMCQ04x\r\n"
	"ETAPBgNVBAgMCGhhbmd6aG91MQwwCgYDVQQKDANFTVExDzANBgNVBAMMBkNsaWVu\r\n"
	"dDCCASIwDQYJKoZIhvcNAQEBBQADggEPADCCAQoCggEBAMy4hoksKcZBDbY680u6\r\n"
	"TS25U51nuB1FBcGMlF9B/t057wPOlxF/OcmbxY5MwepS41JDGPgulE1V7fpsXkiW\r\n"
	"1LUimYV/tsqBfymIe0mlY7oORahKji7zKQ2UBIVFhdlvQxunlIDnw6F9popUgyHt\r\n"
	"dMhtlgZK8oqRwHxO5dbfoukYd6J/r+etS5q26sgVkf3C6dt0Td7B25H9qW+f7oLV\r\n"
	"PbcHYCa+i73u9670nrpXsC+Qc7Mygwa2Kq/jwU+ftyLQnOeW07DuzOwsziC/fQZa\r\n"
	"nbxR+8U9FNftgRcC3uP/JMKYUqsiRAuaDokARZxVTV5hUElfpO6z6/NItSDvvh3i\r\n"
	"eikCAwEAAaMaMBgwCQYDVR0TBAIwADALBgNVHQ8EBAMCBeAwDQYJKoZIhvcNAQEL\r\n"
	"BQADggEBABchYxKo0YMma7g1qDswJXsR5s56Czx/I+B41YcpMBMTrRqpUC0nHtLk\r\n"
	"M7/tZp592u/tT8gzEnQjZLKBAhFeZaR3aaKyknLqwiPqJIgg0pgsBGITrAK3Pv4z\r\n"
	"5/YvAJJKgTe5UdeTz6U4lvNEux/4juZ4pmqH4qSFJTOzQS7LmgSmNIdd072rwXBd\r\n"
	"UzcSHzsJgEMb88u/LDLjj1pQ7AtZ4Tta8JZTvcgBFmjB0QUi6fgkHY6oGat/W4kR\r\n"
	"jSRUBlMUbM/drr2PVzRc2dwbFIl3X+ZE6n5Sl3ZwRAC/s92JU6CPMRW02muVu6xl\r\n"
	"goraNgPISnrbpR6KjxLZkVembXzjNNc=\r\n" \
	"-----END CERTIFICATE-----"

};
static const char *testclientPk= \
{
	\
	"-----BEGIN RSA PRIVATE KEY-----\r\n"
	"MIIEpAIBAAKCAQEAzLiGiSwpxkENtjrzS7pNLblTnWe4HUUFwYyUX0H+3TnvA86X\r\n"
	"EX85yZvFjkzB6lLjUkMY+C6UTVXt+mxeSJbUtSKZhX+2yoF/KYh7SaVjug5FqEqO\r\n"
	"LvMpDZQEhUWF2W9DG6eUgOfDoX2milSDIe10yG2WBkryipHAfE7l1t+i6Rh3on+v\r\n"
	"561LmrbqyBWR/cLp23RN3sHbkf2pb5/ugtU9twdgJr6Lve73rvSeulewL5BzszKD\r\n"
	"BrYqr+PBT5+3ItCc55bTsO7M7CzOIL99BlqdvFH7xT0U1+2BFwLe4/8kwphSqyJE\r\n"
 	"C5oOiQBFnFVNXmFQSV+k7rPr80i1IO++HeJ6KQIDAQABAoIBAGWgvPjfuaU3qizq\r\n"
	"uti/FY07USz0zkuJdkANH6LiSjlchzDmn8wJ0pApCjuIE0PV/g9aS8z4opp5q/gD\r\n"
	"UBLM/a8mC/xf2EhTXOMrY7i9p/I3H5FZ4ZehEqIw9sWKK9YzC6dw26HabB2BGOnW\r\n"
	"5nozPSQ6cp2RGzJ7BIkxSZwPzPnVTgy3OAuPOiJytvK+hGLhsNaT+Y9bNDvplVT2\r\n"
	"ZwYTV8GlHZC+4b2wNROILm0O86v96O+Qd8nn3fXjGHbMsAnONBq10bZS16L4fvkH\r\n"
	"5G+W/1PeSXmtZFppdRRDxIW+DWcXK0D48WRliuxcV4eOOxI+a9N2ZJZZiNLQZGwg\r\n"
	"w3A8+mECgYEA8HuJFrlRvdoBe2U/EwUtG74dcyy30L4yEBnN5QscXmEEikhaQCfX\r\n"
	"Wm6EieMcIB/5I5TQmSw0cmBMeZjSXYoFdoI16/X6yMMuATdxpvhOZGdUGXxhAH+x\r\n"
	"xoTUavWZnEqW3fkUU71kT5E2f2i+0zoatFESXHeslJyz85aAYpP92H0CgYEA2e5A\r\n"
	"Yozt5eaA1Gyhd8SeptkEU4xPirNUnVQHStpMWUb1kzTNXrPmNWccQ7JpfpG6DcYl\r\n"
	"zUF6p6mlzY+zkMiyPQjwEJlhiHM2NlL1QS7td0R8ewgsFoyn8WsBI4RejWrEG9td\r\n"
	"EDniuIw+pBFkcWthnTLHwECHdzgquToyTMjrBB0CgYEA28tdGbrZXhcyAZEhHAZA\r\n"
	"Gzog+pKlkpEzeonLKIuGKzCrEKRecIK5jrqyQsCjhS0T7ZRnL4g6i0s+umiV5M5w\r\n"
	"fcc292pEA1h45L3DD6OlKplSQVTv55/OYS4oY3YEJtf5mfm8vWi9lQeY8sxOlQpn\r\n"
	"O+VZTdBHmTC8PGeTAgZXHZUCgYA6Tyv88lYowB7SN2qQgBQu8jvdGtqhcs/99GCr\r\n"
	"H3N0I69LPsKAR0QeH8OJPXBKhDUywESXAaEOwS5yrLNP1tMRz5Vj65YUCzeDG3kx\r\n"
	"gpvY4IMp7ArX0bSRvJ6mYSFnVxy3k174G3TVCfksrtagHioVBGQ7xUg5ltafjrms\r\n"
	"n8l55QKBgQDVzU8tQvBVqY8/1lnw11Vj4fkE/drZHJ5UkdC1eenOfSWhlSLfUJ8j\r\n"
 	"ds7vEWpRPPoVuPZYeR1y78cyxKe1GBx6Wa2lF5c7xjmiu0xbRnrxYeLolce9/ntp\r\n"
	"asClqpnHT8/VJYTD7Kqj0fouTTZf0zkig/y+2XERppd8k+pSKjUCPQ==\r\n" \
  	"-----END RSA PRIVATE KEY-----"

};
#endif

static luat_rtos_task_handle mqtt_task_handle;

static void luat_mqtt_cb(luat_mqtt_ctrl_t *luat_mqtt_ctrl, uint16_t event){
	int ret;
	switch (event)
	{
	case MQTT_MSG_CONNACK:{
		LUAT_DEBUG_PRINT("mqtt_connect ok");

		LUAT_DEBUG_PRINT("mqtt_subscribe");
		uint16_t msgid = 0;
		mqtt_subscribe(&(luat_mqtt_ctrl->broker), mqtt_sub_topic, &msgid, 1);

		LUAT_DEBUG_PRINT("publish");
		uint16_t message_id  = 0;
		mqtt_publish_with_qos(&(luat_mqtt_ctrl->broker), mqtt_pub_topic, mqtt_send_payload, strlen(mqtt_send_payload), 0, MQTT_DEMO_PUB_QOS, &message_id);
		break;
	}
	case MQTT_MSG_PUBLISH : {
		const uint8_t* ptr;
		uint16_t topic_len = mqtt_parse_pub_topic_ptr(luat_mqtt_ctrl->mqtt_packet_buffer, &ptr);
		LUAT_DEBUG_PRINT("pub_topic: %.*s",topic_len,ptr);
		uint16_t payload_len = mqtt_parse_pub_msg_ptr(luat_mqtt_ctrl->mqtt_packet_buffer, &ptr);
		LUAT_DEBUG_PRINT("pub_msg: %.*s",payload_len,ptr);
		break;
	}
	case MQTT_MSG_TCP_TX_DONE:
		//如果用QOS0发送，可以作为发送成功的初步判断依据
		if (0 == MQTT_DEMO_PUB_QOS)
		{
			LUAT_DEBUG_PRINT("publish send ok");
		}
		break;
	case MQTT_MSG_PUBACK : 
	case MQTT_MSG_PUBCOMP : {
		LUAT_DEBUG_PRINT("msg_id: %d",mqtt_parse_msg_id(luat_mqtt_ctrl->mqtt_packet_buffer));
		break;
	}
	case MQTT_MSG_RELEASE : {
		LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt release");
		break;
	}
	case MQTT_MSG_DISCONNECT : { // mqtt 断开(只要有断开就会上报,无论是否重连)
		LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt disconnect");
		break;
	}
	case MQTT_MSG_TIMER_PING : {
		luat_mqtt_ping(luat_mqtt_ctrl);
		break;
	}
	case MQTT_MSG_RECONNECT : {
if (MQTT_DEMO_AUTOCON == 1)
{
		luat_mqtt_reconnect(luat_mqtt_ctrl);
}
		break;
	}
	case MQTT_MSG_CLOSE : { // mqtt 关闭(不会再重连)  注意：一定注意和MQTT_MSG_DISCONNECT区别，如果要做手动重连处理推荐在这里 */
		LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt close");
if (MQTT_DEMO_AUTOCON == 0){
	ret = luat_mqtt_connect(luat_mqtt_ctrl);
	if (ret) {
		LUAT_DEBUG_PRINT("mqtt connect ret=%d\n", ret);
		luat_mqtt_close_socket(luat_mqtt_ctrl);
		return;
	}
}
		break;
	}
	default:
		break;
	}
	return;
}

static void luat_mqtt_task(void *param)
{
	int ret = -1;
	luat_mqtt_ctrl_t *luat_mqtt_ctrl = (luat_mqtt_ctrl_t *)luat_heap_malloc(sizeof(luat_mqtt_ctrl_t));
	ret = luat_mqtt_init(luat_mqtt_ctrl, NW_ADAPTER_INDEX_LWIP_GPRS);
	if (ret) {
		LUAT_DEBUG_PRINT("mqtt init FAID ret %d", ret);
		return;
	}
	luat_mqtt_ctrl->ip_addr.type = 0xff;
	luat_mqtt_connopts_t opts = {0};

#if (MQTT_DEMO_SSL == 1)
	opts.is_tls = 1;
	opts.server_cert = testCaCrt;
	opts.server_cert_len = strlen(testCaCrt);
	opts.client_cert = testclientCert;
	opts.client_cert_len = strlen(testclientCert);
	opts.client_key = testclientPk;
	opts.client_key_len = strlen(testclientPk);
#else
	opts.is_tls = 0;
#endif 
	opts.host = MQTT_HOST;
	opts.port = MQTT_PORT;
	ret = luat_mqtt_set_connopts(luat_mqtt_ctrl, &opts);

	char clientId[16] = {0};
	ret = luat_mobile_get_imei(0, clientId, sizeof(clientId)-1);
	if(ret <= 0){
		LUAT_DEBUG_PRINT("imei get fail");
		mqtt_init(&(luat_mqtt_ctrl->broker), CLIENT_ID);
	}
	else
		mqtt_init(&(luat_mqtt_ctrl->broker), clientId);

	mqtt_init_auth(&(luat_mqtt_ctrl->broker), USERNAME, PASSWORD);

	// luat_mqtt_ctrl->netc->is_debug = 1;// debug信息
	luat_mqtt_ctrl->broker.clean_session = 1;
	luat_mqtt_ctrl->keepalive = 240;

if (MQTT_DEMO_AUTOCON == 1)
{
	luat_mqtt_ctrl->reconnect = 1;
	luat_mqtt_ctrl->reconnect_time = 3000;
}

	// luat_mqtt_set_will(luat_mqtt_ctrl, mqtt_will_topic, mqtt_will_payload, strlen(mqtt_will_payload), 0, 0); // 测试遗嘱
	
	luat_mqtt_set_cb(luat_mqtt_ctrl,luat_mqtt_cb);
	luat_mqtt_ctrl->netc->is_debug = 1;
	LUAT_DEBUG_PRINT("mqtt_connect");
	ret = luat_mqtt_connect(luat_mqtt_ctrl);
	if (ret) {
		LUAT_DEBUG_PRINT("mqtt connect ret=%d\n", ret);
		luat_mqtt_close_socket(luat_mqtt_ctrl);
		return;
	}
	LUAT_DEBUG_PRINT("wait mqtt_state ...");

	while(1){
		if (luat_mqtt_state_get(luat_mqtt_ctrl) == MQTT_STATE_READY){
			uint16_t message_id = 0;
			mqtt_publish_with_qos(&(luat_mqtt_ctrl->broker), mqtt_pub_topic, mqtt_send_payload, strlen(mqtt_send_payload), 0, MQTT_DEMO_PUB_QOS, &message_id);
		}
		luat_rtos_task_sleep(5000);
	}
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

static void luat_libemqtt_init(void)
{
	luat_mobile_set_sim_detect_sim0_first();
	luat_mobile_event_register_handler(luatos_mobile_event_callback);

	luat_rtos_task_create(&mqtt_task_handle, 2 * 1024, 10, "libemqtt", luat_mqtt_task, NULL, 16);
}

INIT_TASK_EXPORT(luat_libemqtt_init, "1");
