#include "luat_network_adapter.h"
#include "common_api.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_rtos.h"
#include "luat_mobile.h"

#include "libemqtt.h"
#include "luat_mqtt.h"
#include "luat_mcu.h"
#include "luat_uart.h"

// 是否开启SSL
#define MQTT_DEMO_SSL 			1
// 是否自动重连
#define MQTT_DEMO_AUTOCON 		1
// 上行时的QOS值
#define MQTT_DEMO_PUB_QOS 		1

// MQTT服务器信息, 仅支持MQTT3.1.1协议, 不支持 3.1和5.0协议, 不支持 mqtt over websocket
#if (MQTT_DEMO_SSL == 1)
#define MQTT_HOST    	"airtest.openluat.com"   				// MQTTS服务器的地址和端口号
#define MQTT_PORT		8883      
#define USERNAME    	"user"                 
#define PASSWORD    	"password"   
#else
#define MQTT_HOST    	"lbsmqtt.airm2m.com"   				// MQTT服务器的地址和端口号
#define MQTT_PORT		1884
#define USERNAME    	"username"                 
#define PASSWORD    	"password"   
#endif 

#define CLIENT_ID    	"123456789"         // 缺省ClientId, 本demo会用IMEI号填充
// MQTT的Client Id, 默认会用IMEI填充
static char mqtt_client_id[128] = {0};

// topic里的%s, 在本example中, 在上行时会先用imei格式化

// 订阅的主题及模板, downlink
static char mqtt_sub_topic_tmpl[] = "/sys/%s/sub";
static char mqtt_sub_topic[256]; // 可以自行填入值, 默认会用tmpl进行格式化
// 发布的主题, uplink
static char mqtt_pub_topic_tmpl[] = "/sys/%s/pub";
static char mqtt_pub_topic[256]; // 可以自行填入值, 默认会用tmpl进行格式化


// UART透传演示, 设置为-1可关闭
// MAIN UART ==> 1
// AUX  UART ==> 2
// 718P/718PV 有 UART3
#define MQTT_DEMO_UART_ID 		1
#define MQTT_DEMO_UART_BAUD 	115200

// MQTT控制器句柄
static luat_mqtt_ctrl_t *mymqtt;

#if (MQTT_DEMO_SSL == 1)
// MQTT支持的加密有:
// 单向无校验(无服务器证书)
// 单向有校验(需要服务器证书)
// 双向校验(需要服务器证书和客户端证书)

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

static void luat_mqtt_cb(luat_mqtt_ctrl_t *ctrl, uint16_t event){
	int ret = 0;
	const uint8_t* ptr = NULL;
	uint16_t msgid = 0;
	uint16_t message_id  = 0;
	char mqtt_send_payload[128] = {0};
	switch (event)
	{
	case MQTT_MSG_CONNACK:{
		// 连接成功, 鉴权完成
		LUAT_DEBUG_PRINT("mqtt_connect ok");

		// 订阅Topic
		LUAT_DEBUG_PRINT("mqtt_subscribe %s", mqtt_sub_topic);
		mqtt_subscribe(&(ctrl->broker), mqtt_sub_topic, &msgid, 1);

		// 数据上行
		char iccid[32] = {0};
		char imsi[32] = {0};
		luat_mobile_get_iccid(index, iccid, sizeof(iccid));
		luat_mobile_get_imsi(index, imsi, sizeof(imsi));
		sprintf(mqtt_send_payload, "{\"iccid\":\"%s\", \"imsi\":\"%s\"}", iccid, imsi);
		LUAT_DEBUG_PRINT("publish %s data %s", mqtt_pub_topic, mqtt_send_payload);
		mqtt_publish_with_qos(&(ctrl->broker), mqtt_pub_topic, mqtt_send_payload, strlen(mqtt_send_payload), 1, MQTT_DEMO_PUB_QOS, &message_id);
		break;
	}
	case MQTT_MSG_PUBLISH : {
		// 数据下行
		uint16_t topic_len = mqtt_parse_pub_topic_ptr(ctrl->mqtt_packet_buffer, &ptr);
		LUAT_DEBUG_PRINT("downlink topic: %.*s",topic_len,ptr);
		uint16_t payload_len = mqtt_parse_pub_msg_ptr(ctrl->mqtt_packet_buffer, &ptr);
		LUAT_DEBUG_PRINT("downlink payload: %.*s",payload_len,ptr);
		// 这里添加自己的业务逻辑, 下面是演示, 直接透传到UART
		// 将MQTT数据透传到UART
		#if MQTT_DEMO_UART_ID > 0
		luat_uart_write(MQTT_DEMO_UART_ID, ptr, payload_len);
		#endif
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
		LUAT_DEBUG_PRINT("uplink ack msg_id: %d",mqtt_parse_msg_id(ctrl->mqtt_packet_buffer));
		break;
	}
	case MQTT_MSG_RELEASE : {
		LUAT_DEBUG_PRINT("mqtt release msg!!");
		break;
	}
	case MQTT_MSG_DISCONNECT : { // mqtt 断开(只要有断开就会上报,无论是否重连)
		LUAT_DEBUG_PRINT("mqtt disconnect msg!!");
		break;
	}
	case MQTT_MSG_TIMER_PING : {
		luat_mqtt_ping(ctrl);
		break;
	}
	case MQTT_MSG_RECONNECT : {
		if (MQTT_DEMO_AUTOCON == 1)
		{
			luat_mqtt_reconnect(ctrl);
		}
		break;
	}
	case MQTT_MSG_CLOSE : { // mqtt 关闭(不会再重连)  注意：一定注意和MQTT_MSG_DISCONNECT区别，如果要做手动重连处理推荐在这里 */
		LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt close");
		if (MQTT_DEMO_AUTOCON == 0){
			ret = luat_mqtt_connect(ctrl);
			if (ret) {
				LUAT_DEBUG_PRINT("mqtt connect ret=%d\n", ret);
				luat_mqtt_close_socket(ctrl);
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

// MQTT主任务
static void mqtt_task(void *param)
{
	int ret = -1;
	// 对mqtt的信息进行初始化

	// 首先是client id
	if (mqtt_client_id[0] == 0) {
		ret = luat_mobile_get_imei(0, mqtt_client_id, sizeof(mqtt_client_id)-1);
		if(ret <= 0){
			LUAT_DEBUG_PRINT("imei get fail!!!!!");
			memcpy(mqtt_client_id, CLIENT_ID, strlen(CLIENT_ID));
		}
	}

	// 然后是topic
	if (mqtt_sub_topic[0] == 0) {
		sprintf(mqtt_sub_topic, mqtt_sub_topic_tmpl, mqtt_client_id);
	}
	if (mqtt_pub_topic[0] == 0) {
		sprintf(mqtt_pub_topic, mqtt_pub_topic_tmpl, mqtt_client_id);
	}

	// 打印mqtt三元组及topic信息
	LUAT_DEBUG_PRINT("MQTT_HOST:         %s", MQTT_HOST);
	LUAT_DEBUG_PRINT("MQTT_PORT:         %d", MQTT_PORT);
	LUAT_DEBUG_PRINT("MQTT_CLIENT_ID:    %s", mqtt_client_id);
	LUAT_DEBUG_PRINT("mqtt_sub_topic:    %s", mqtt_sub_topic);
	LUAT_DEBUG_PRINT("mqtt_pub_topic:    %s", mqtt_pub_topic);


	mymqtt = (luat_mqtt_ctrl_t *)luat_heap_malloc(sizeof(luat_mqtt_ctrl_t));
	ret = luat_mqtt_init(mymqtt, NW_ADAPTER_INDEX_LWIP_GPRS);
	if (ret) {
		LUAT_DEBUG_PRINT("mqtt init FAID ret %d", ret);
		return;
	}
	mymqtt->ip_addr.type = 0xff;
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
	ret = luat_mqtt_set_connopts(mymqtt, &opts);

	mqtt_init(&(mymqtt->broker), mqtt_client_id);
	mqtt_init_auth(&(mymqtt->broker), USERNAME, PASSWORD);

	// luat_mqtt_ctrl->netc->is_debug = 1;   // 开启底层debug信息
	mymqtt->keepalive = 240;         // MQTT PING 间隔
	mymqtt->broker.clean_session = 1;

	if (MQTT_DEMO_AUTOCON == 1) // 自动重连
	{
		mymqtt->reconnect = 1;
		mymqtt->reconnect_time = 3000; // 延迟3秒重连
	}

	// 测试遗嘱
	// luat_mqtt_set_will(luat_mqtt_ctrl, mqtt_will_topic, mqtt_will_payload, strlen(mqtt_will_payload), 0, 0); 
	
	luat_mqtt_set_cb(mymqtt,luat_mqtt_cb);
	LUAT_DEBUG_PRINT("mqtt_connect");
	ret = luat_mqtt_connect(mymqtt);
	if (ret) {
		// 发起连接, 非阻塞的
		LUAT_DEBUG_PRINT("mqtt connect ret=%d\n", ret);
		luat_mqtt_close_socket(mymqtt);
		return;
	}
	LUAT_DEBUG_PRINT("wait mqtt_state ...");

	uint16_t message_id = 0;
	char mqtt_send_payload[256] = {0};
	while(1){
		if (luat_mqtt_state_get(mymqtt) == MQTT_STATE_READY){
			sprintf(mqtt_send_payload, "{\"ticks\":%u}", luat_mcu_ticks());
			ret = mqtt_publish_with_qos(&(mymqtt->broker), mqtt_pub_topic, mqtt_send_payload, strlen(mqtt_send_payload), 1, MQTT_DEMO_PUB_QOS, &message_id);
			LUAT_DEBUG_PRINT("uplink topic %s data %s msgid %d ret %d", mqtt_pub_topic, mqtt_send_payload, message_id, ret);
		}
		luat_rtos_task_sleep(5000);
	}

	// task结束, 删除自身
	luat_rtos_task_delete(mqtt_task_handle);
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

#if MQTT_DEMO_UART_ID > 0
static void uart_recv_cb(uint8_t *data, uint32_t len);
#endif

static void my_mqtt_init(void)
{
	luat_mobile_set_sim_detect_sim0_first();
	luat_mobile_event_register_handler(luatos_mobile_event_callback);

	luat_rtos_task_create(&mqtt_task_handle, 8 * 1024, 10, "mqtt_task", mqtt_task, NULL, 16);

	#if MQTT_DEMO_UART_ID > 0
	luat_uart_t uart = {
        .id = MQTT_DEMO_UART_ID,
        .baud_rate = MQTT_DEMO_UART_BAUD,
        .data_bits = 8,
        .stop_bits = 1,
        .parity    = 0
    };
	LUAT_DEBUG_PRINT("初始化MQTT-UART透传");
	LUAT_DEBUG_PRINT("uart setup result %d", luat_uart_setup(&uart));
    LUAT_DEBUG_PRINT("uart ctrl result %d", luat_uart_ctrl(MQTT_DEMO_UART_ID, LUAT_UART_SET_RECV_CALLBACK, uart_recv_cb));
	#endif
}

// 接收UART数据, 透传到MQTT
#if MQTT_DEMO_UART_ID > 0
static char uart_rxbuff[2048];
static void uart_recv_cb(uint8_t *data, uint32_t len) {
	int ready = mymqtt != NULL && luat_mqtt_state_get(mymqtt) == MQTT_STATE_READY;
	int message_id = 0;
	int ret = 0;
    while (1) {
        len = luat_uart_read(MQTT_DEMO_UART_ID, uart_rxbuff, 2048);
        if (len <= 0) {
            break;
        }
		if (ready != MQTT_STATE_READY) {
			LUAT_DEBUG_PRINT("MQTT连接未就绪, 丢弃数据 %d %d", MQTT_DEMO_UART_ID, len);
		}
		else {
			ret = mqtt_publish_with_qos(&(mymqtt->broker), mqtt_pub_topic, uart_rxbuff, len, 1, MQTT_DEMO_PUB_QOS, &message_id);
			LUAT_DEBUG_PRINT("uplink topic %s len %d msgid %d ret %d", mqtt_pub_topic, len, message_id, ret);
		}
    }
}
#endif

INIT_TASK_EXPORT(my_mqtt_init, "1");
