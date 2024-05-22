//项目配置必须是ec718p和使能denoise mode
//开启内置降噪功能后，会自动使能宏定义FEATURE_AMR_CP_ENABLE和FEATURE_VEM_CP_ENABLE
//demo使用的mqtt做为服务器协议
//已实现广播模式，即一个人说话，其他人全部都能接收到，按住BOOT，等NET灯亮，开始录音上行，松开BOOT，NET灯灭，结束录音，如果多个人同时上行，谁先上行，谁的话就被广播出来
//demo重点演示对音频数据的处理，网络部分处理需要用户根据实际情况进行优化
//上线后，led慢闪，录音时常亮，播放时快闪，录音优先于播放
//如果需要低功耗，需要把ES8311的控制引脚改为AGPIO，无数据交互时，LED灯关闭
#include "common_api.h"
#include "luat_gpio.h"
#include "luat_mobile.h"
#include "luat_i2c.h"
#include "luat_i2s.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_mem.h"
#include "luat_mcu.h"
#include "luat_pwm.h"

#include "luat_audio_play.h"
#include "luat_uart.h"

#include "luat_network_adapter.h"
#include "libemqtt.h"
#include "luat_mqtt.h"

//硬件相关宏定义
#define CODEC_PWR_PIN HAL_GPIO_16
#define CODEC_PWR_PIN_ALT_FUN	4
#define PA_PWR_PIN HAL_GPIO_25
#define PA_PWR_PIN_ALT_FUN	0
#define SPEECH_LED_PIN HAL_GPIO_27
#define SPEECH_LED_PWM	4
#define SPEECH_CTRL_PIN HAL_GPIO_0
#define SPEECH_CTRL_PIN_ALT_FUN	0

#define BUTTON_PRESS_LEVEL 1
#define PA_ON_LEVEL 1
#define PWR_ON_LEVEL 1

#define AUDIO_SLEEP_MODE	LUAT_AUDIO_PM_SHUTDOWN

#define TEST_I2S_ID I2S_ID0

#define MULTIMEDIA_ID 	0	//多媒体id，用于区分不同多媒体硬件
#define TEST_VOL		70	// 测试音量调节
#define TEST_MIC_VOL	75	// 测试麦克风音量调节

#define PA_DELAY		200
#define PWR_ON_DELAY	0
#define PWR_SLEEP_DELAY	600

#define VOICE_VOL   70
#define MIC_VOL     75

//#define LOW_POWER_ENABLE
#define AMR_ONE_FRAME_LEN	640
#define PCM_BLOCK_LEN	(AMR_ONE_FRAME_LEN)
#define PCM_ONE_FRAME_BLOCK_NUM	(5)	//1帧数据包含5个20ms
#define PCM_CACHE_FRAME_BEFORE_PLAY	(5)	//缓存N帧数据后再开始播放
static const uint8_t  amr_wb_byte_len[] = {17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 0, 0, 0, 0, 0, 0};
static const uint8_t amr_wb_zero_data[61] = {
		0x44, 0x11, 0x06, 0x30, 0x33, 0xbe, 0xce, 0xb3,
		0xa0, 0xd3, 0x12, 0x40, 0xeb, 0x50, 0x87, 0xb4,
		0xff, 0xd6, 0x42, 0x40, 0x18, 0x08, 0x1a, 0xe5,
		0x02, 0x22, 0x96, 0x11, 0x29, 0x48, 0x49, 0xcb,
		0x52, 0x22, 0x89, 0x06, 0x78, 0xc0, 0x08, 0x00,
		0xb1, 0x18, 0x8b, 0x83, 0x24, 0xc7, 0x58, 0x74,
		0xad, 0x19, 0x0d, 0xd3, 0xb0, 0x5b, 0x08, 0xa8,
		0xcb, 0xba, 0xaf, 0xf2, 0x58
};
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#if 0
#include "audioCfg.h"   //struct AudioConfig_t
#include "mw_nvm_audio.h"
static void log_on(void)
{
	AudioParaCfgLogControl_t audioLogCfg = {0};
	AudioParaCfgCommon_t mAudioCfgCommon = {0};
	AecConfig_t    MwNvmAudioSphTxAEC;
	ecAudioCfgTlvStore *pMwNvmAudioCfg = NULL;
	AudioParaCfgLogControl_t MwNvmAudioLogCtrl;
	pMwNvmAudioCfg  = (ecAudioCfgTlvStore *)malloc(sizeof(ecAudioCfgTlvStore)+ sizeof(AudioParaSphEQBiquard_t)*EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB*EC_ADCFG_SPEECH_TX_NUMB
					+ sizeof(AudioParaSphEQBiquard_t)*EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB*EC_ADCFG_SPEECH_RX_NUMB + sizeof(UINT16)*EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB*EC_ADCFG_SPEECH_RX_NUMB
					 + sizeof(UINT16)*EC_ADCFG_SPEECH_ANS_EQ_BAND_NUMB*EC_ADCFG_SPEECH_TX_NUMB);

	if (mwNvmAudioCfgRead(pMwNvmAudioCfg) == FALSE)
	{
		if (mwNvmAudioCfgRead(pMwNvmAudioCfg) == FALSE)
		{
			DBG("read config failed");
		}
	}
	mwNvmAudioCfgLogControlGet(&MwNvmAudioLogCtrl, pMwNvmAudioCfg);
	mwNvmAudioCfgSpeechGetTxAEC(&mAudioCfgCommon, &MwNvmAudioSphTxAEC, pMwNvmAudioCfg);
	if (!MwNvmAudioLogCtrl.TxBeforeVem)
	{
		MwNvmAudioLogCtrl.TxBeforeVem = 1;
		MwNvmAudioLogCtrl.TxAfterVem = 1;
		MwNvmAudioLogCtrl.RxBeforeVem = 1;
		MwNvmAudioLogCtrl.RxAfterVem = 1;
		MwNvmAudioLogCtrl.RxBeforeDecoder = 1;
		MwNvmAudioLogCtrl.TxAfterEncoder = 1;
		mwNvmAudioCfgLogControlSet(&MwNvmAudioLogCtrl, pMwNvmAudioCfg);
		DBG("log on");
	}

	audioLogCfg.TxBeforeVem = 1;
	audioLogCfg.TxAfterVem = 1;
	audioLogCfg.RxBeforeVem = 1;
	audioLogCfg.RxAfterVem = 1;
	audioLogCfg.RxBeforeDecoder = 1;
	audioLogCfg.TxAfterEncoder = 1;
	ShareInfoAPSetCPAudioLogCtrl(audioLogCfg);
	free(pMwNvmAudioCfg);
}
#endif

typedef struct
{
	uint16_t amr_save_data_len;
	uint8_t amr_save_data[64 * PCM_ONE_FRAME_BLOCK_NUM];
}amr_wb_save_struct;
typedef struct
{
	BSP_FifoStruct downlink_buffer;
	Buffer_Struct downlink_cache;
	luat_rtos_task_handle speech_task_handle;
	luat_rtos_task_handle mqtt_task_handle;
	luat_rtos_task_handle key_task_handle;
	luat_rtos_task_handle led_task_handle;
	luat_rtos_timer_t download_loop_timer;
	luat_rtos_timer_t download_no_data_timer;
	volatile uint32_t record_block_cnt;
	amr_wb_save_struct amr_wb_save_buffer[8];
	uint8_t audio_enable;
	uint8_t record_enable;
	uint8_t play_enable;
	uint8_t play_no_more_data;
	uint8_t resync_flag;
	uint8_t codec_ready;
	uint8_t codec_exsit;
	uint8_t mqtt_ready;
	uint8_t downlink_state;
	uint8_t amr_wb_save_buffer_point;
	uint8_t download_data_flag;
	uint8_t amr_dummy[64];
	uint8_t amr_dummy_len;
}speech_ctrl_t;

#define MQTT_HOST    	"lbsmqtt.airm2m.com"   				// MQTT服务器的地址和端口号
#define MQTT_PORT		1884
static const char topic_global[] = "speech_demo/all";
static speech_ctrl_t speechc = {0};

static int record_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param);
static const luat_i2s_conf_t luat_i2s_conf_es8311 ={
	.id = TEST_I2S_ID,
	.mode = LUAT_I2S_MODE_MASTER,
	.channel_format = LUAT_I2S_CHANNEL_RIGHT,
	.standard = LUAT_I2S_MODE_LSB,
	.channel_bits = LUAT_I2S_BITS_16,
	.data_bits = LUAT_I2S_BITS_16,
	.is_full_duplex = 1,
	.cb_rx_len = AMR_ONE_FRAME_LEN,

	.luat_i2s_event_callback = record_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_es8311 = {
    .i2c_id = 0,
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_es8311,
};

enum
{
	EVENT_AMR_RUN_ONCE = 1,
	EVENT_MQTT_MSG,
	EVENT_MQTT_DOWNLINK_DATA,
	EVENT_MQTT_UPLINK_DATA,
	EVENT_MQTT_CHECK_DOWNLINK,
	EVENT_MQTT_FORCE_SYNC,
	EVENT_MQTT_FORCE_STOP,
	EVENT_KEY_PRESS,
	EVENT_AMR_START_TRUE_PLAY,
	EVENT_AMR_START,
	EVENT_AMR_RECORD_STOP,
	EVENT_AMR_PLAY_STOP,
	EVENT_KEY_PIN_IRQ,
	EVENT_LED_OFF_LINE,
	EVENT_LED_ON_LINE,
	EVENT_LED_DOWNLOAD_START,
	EVENT_LED_DOWNLOAD_END,
	EVENT_LED_UPLOAD_START,
	EVENT_LED_UPLOAD_END,

	PACKET_TYPE_BROADCAST_DATA = 0,
	PACKET_TYPE_BROADCAST_END,

	DL_STATE_IDLE = 0,
	DL_STATE_WAIT_CACHE_OK,
	DL_STATE_PLAY,
	DL_STATE_PLAY_WAIT_CACHE_OK,
};

static void luat_mqtt_cb(luat_mqtt_ctrl_t *luat_mqtt_ctrl, uint16_t event){
	int ret;
	if (event != MQTT_MSG_PUBLISH)
	{
		luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_MSG, event, 0, 0, 0);
	}
	else
	{
		const uint8_t* ptr;
		uint32_t len;
		uint8_t *topic = NULL;
		uint8_t *payload = NULL;
		len = mqtt_parse_pub_topic_ptr(luat_mqtt_ctrl->mqtt_packet_buffer, &ptr);
		topic = calloc(len + 1, 1);
		memcpy(topic, ptr, len);
		len = mqtt_parse_pub_msg_ptr(luat_mqtt_ctrl->mqtt_packet_buffer, &ptr);
		if (len)
		{
			payload = malloc(len);
			memcpy(payload, ptr, len);
		}
		luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_DOWNLINK_DATA, (uint32_t)topic, (uint32_t)payload, len, 0);
	}
	return;
}

static void end_broadcast_play(void)
{
	LUAT_DEBUG_PRINT("broadcast play end!");
	OS_DeInitBuffer(&speechc.downlink_cache);
	luat_rtos_timer_stop(speechc.download_loop_timer);
	luat_rtos_timer_stop(speechc.download_no_data_timer);
	speechc.downlink_state = DL_STATE_IDLE;
	speechc.play_no_more_data = 1;
	luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_PLAY_STOP, 0, 0, 0, 0);
	luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_DOWNLOAD_END, 0, 0, 0, 0);

}

static LUAT_RT_RET_TYPE download_timeout(LUAT_RT_CB_PARAM)
{
	if (speechc.download_data_flag)
	{
		speechc.download_data_flag = 0;
	}
	else
	{
		luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_FORCE_STOP, 0, 0, 0, 0);
	}
}

static LUAT_RT_RET_TYPE download_loop(LUAT_RT_CB_PARAM)
{
	luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_CHECK_DOWNLINK, 0, 0, 0, 0);
}

static void mqtt_task(void *param)
{
	luat_event_t event;
	Buffer_Struct uplink;
	OS_InitBuffer(&uplink, 1024);
	OS_InitBuffer(&speechc.downlink_cache, 4096);
	int ret = -1;
	char *client_id;
	uint32_t play_block_cnt = 0;
	uint32_t play_block_cnt_last = 0;
	uint32_t play_block_check_cnt_start = 0;
	int i;
	uint16_t msgid = 0;
	uint8_t cache_data_cnt = 0;
	char remote_client[16];
	char clientId[16] = {0};
	luat_rtos_timer_create(&speechc.download_loop_timer);
	luat_rtos_timer_create(&speechc.download_no_data_timer);
	luat_mqtt_ctrl_t *luat_mqtt_ctrl = (luat_mqtt_ctrl_t *)luat_heap_malloc(sizeof(luat_mqtt_ctrl_t));
	luat_mqtt_init(luat_mqtt_ctrl, NW_ADAPTER_INDEX_LWIP_GPRS);

	luat_mqtt_ctrl->ip_addr.type = 0xff;
	luat_mqtt_connopts_t opts = {0};
	opts.host = MQTT_HOST;
	opts.port = MQTT_PORT;
	luat_mqtt_set_connopts(luat_mqtt_ctrl, &opts);
	while (1)
	{
		if (luat_mobile_get_imsi(0, clientId, sizeof(clientId)-1) <= 0)
		{
			luat_rtos_task_sleep(500);
		}
		else
		{
			LUAT_DEBUG_PRINT("use id %s", clientId);
			break;
		}
	}
	mqtt_init(&(luat_mqtt_ctrl->broker), clientId);
	mqtt_init_auth(&(luat_mqtt_ctrl->broker), "user", "password");

	luat_mqtt_ctrl->broker.clean_session = 1;
	luat_mqtt_ctrl->keepalive = 240;


	luat_mqtt_ctrl->reconnect = 1;
	luat_mqtt_ctrl->reconnect_time = 3000;

	luat_mqtt_set_cb(luat_mqtt_ctrl,luat_mqtt_cb);
	//luat_mqtt_ctrl->netc->is_debug = 1;
	luat_mqtt_connect(luat_mqtt_ctrl);

	while(1){
		luat_rtos_event_recv(speechc.mqtt_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case EVENT_MQTT_DOWNLINK_DATA:
			if (memcmp(topic_global, (char *)event.param1, sizeof(topic_global) - 1))
			{
				LUAT_DEBUG_PRINT("topic %s", (char *)event.param1);
			}
			else
			{
				client_id = (char *)event.param2;
				if (!memcmp(client_id, clientId, 15))
				{
					//LUAT_DEBUG_PRINT("self msg");
					goto RX_DATA_DONE;
				}
				memcpy(&play_block_cnt, client_id + 16, 4);
				if (PACKET_TYPE_BROADCAST_END == client_id[15])
				{
					if (speechc.downlink_state)
					{
						DBG("remote upload stop!");
						end_broadcast_play();
					}
				}
				else
				{
					if (speechc.downlink_state)
					{
						if (memcmp(client_id, remote_client, 15))
						{
							LUAT_DEBUG_PRINT("not continue msg");
							goto RX_DATA_DONE;
						}
						if ((play_block_cnt_last + 1) != play_block_cnt)
						{
							LUAT_DEBUG_PRINT("packet cnt resync %u,%u", play_block_cnt_last, play_block_cnt);
							play_block_cnt_last = play_block_cnt;
							play_block_check_cnt_start = play_block_cnt_last;
							speechc.downlink_cache.Pos = 0;
							OS_BufferWrite(&speechc.downlink_cache, (void *)(event.param2 + 20), event.param3 - 20);
							cache_data_cnt = 1;
							if (DL_STATE_PLAY_WAIT_CACHE_OK != speechc.downlink_state)
							{
								speechc.downlink_state = (DL_STATE_PLAY == speechc.downlink_state)?DL_STATE_PLAY_WAIT_CACHE_OK:DL_STATE_WAIT_CACHE_OK;
							}
							LUAT_DEBUG_PRINT("downlink state %d", speechc.downlink_state);
							goto RX_DATA_DONE;
						}
						else
						{
							play_block_cnt_last = play_block_cnt;
						}

					}
					speechc.download_data_flag = 1;
					switch(speechc.downlink_state)
					{
					case DL_STATE_IDLE:
						LUAT_DEBUG_PRINT("find first packet");
						play_block_cnt_last = play_block_cnt;
						OS_ReInitBuffer(&speechc.downlink_cache, 1024);
						OS_BufferWrite(&speechc.downlink_cache, (void *)(event.param2 + 20), event.param3 - 20);
						memcpy(remote_client, client_id, 15);
						cache_data_cnt = 1;
						play_block_check_cnt_start = play_block_cnt_last;
						speechc.downlink_state = DL_STATE_WAIT_CACHE_OK;
						luat_rtos_timer_start(speechc.download_loop_timer, 3000, 1, download_timeout, NULL);
						luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_DOWNLOAD_START, 0, 0, 0, 0);
						break;
					case DL_STATE_WAIT_CACHE_OK:
						OS_BufferWrite(&speechc.downlink_cache, (void *)(event.param2 + 20), event.param3 - 20);
						cache_data_cnt++;
						if (cache_data_cnt >= PCM_CACHE_FRAME_BEFORE_PLAY)
						{
							cache_data_cnt = 0;
							speechc.downlink_state = DL_STATE_PLAY;
							LUAT_DEBUG_PRINT("broadcast play start!, %u", play_block_check_cnt_start);
							OS_WriteFifo(&speechc.downlink_buffer, speechc.downlink_cache.Data, speechc.downlink_cache.Pos);
							speechc.play_enable = 1;
							speechc.downlink_cache.Pos = 0;
							luat_rtos_timer_start(speechc.download_loop_timer, PCM_ONE_FRAME_BLOCK_NUM * 20, 1, download_loop, NULL);
							luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_START, 0, 0, 0, 0);
						}
						break;
					case DL_STATE_PLAY:
						OS_WriteFifo(&speechc.downlink_buffer, (void *)(event.param2 + 20), event.param3 - 20);

						break;
					case DL_STATE_PLAY_WAIT_CACHE_OK:
						OS_BufferWrite(&speechc.downlink_cache, (void *)(event.param2 + 20), event.param3 - 20);
						cache_data_cnt++;
						if (cache_data_cnt >= PCM_CACHE_FRAME_BEFORE_PLAY)
						{
							cache_data_cnt = 0;
							LUAT_DEBUG_PRINT("broadcast replay start! %u", play_block_check_cnt_start);
							speechc.downlink_state = DL_STATE_PLAY;
							OS_WriteFifo(&speechc.downlink_buffer, speechc.downlink_cache.Data, speechc.downlink_cache.Pos);
						}
						break;
					}
				}
			}
RX_DATA_DONE:
			free((char *)event.param1);
			free((char *)event.param2);
			break;
		case EVENT_MQTT_CHECK_DOWNLINK:
			if (DL_STATE_PLAY_WAIT_CACHE_OK == speechc.downlink_state)
			{
				LUAT_DEBUG_PRINT("add blank data");
				for(i = 0; i < PCM_ONE_FRAME_BLOCK_NUM; i++)
				{
					OS_WriteFifo(&speechc.downlink_buffer, speechc.amr_dummy, 61);
				}
				break;
			}
			play_block_check_cnt_start++;
			if (play_block_cnt_last <= (play_block_check_cnt_start + 2))
			{
				LUAT_DEBUG_PRINT("broadcast play need resync! %u,%u", play_block_cnt_last, play_block_check_cnt_start);
				play_block_cnt_last = 0;
				cache_data_cnt = 0;
				speechc.downlink_state = DL_STATE_PLAY_WAIT_CACHE_OK;
				for(i = 0; i < PCM_ONE_FRAME_BLOCK_NUM; i++)
				{
					OS_WriteFifo(&speechc.downlink_buffer, speechc.amr_dummy, 61);
				}
			}
			break;
		case EVENT_MQTT_UPLINK_DATA:
			if (speechc.mqtt_ready)
			{
				clientId[15] = event.param1;
				uplink.Pos = 0;
				OS_BufferWrite(&uplink, clientId, 16);

				if (PACKET_TYPE_BROADCAST_DATA == event.param1)
				{
					OS_BufferWrite(&uplink, &speechc.record_block_cnt, 4);
					OS_BufferWrite(&uplink, speechc.amr_wb_save_buffer[event.param2].amr_save_data, speechc.amr_wb_save_buffer[event.param2].amr_save_data_len);
					speechc.record_block_cnt++;
					//LUAT_DEBUG_PRINT("upload buffer %d, len %d", event.param2, speechc.amr_wb_save_buffer[event.param2].amr_save_data_len);
					speechc.amr_wb_save_buffer[event.param2].amr_save_data_len = 0;
				}
				mqtt_publish(&(luat_mqtt_ctrl->broker), topic_global, uplink.Data, uplink.Pos, 0);
			}
			else
			{
				//LUAT_DEBUG_PRINT("upload buffer %d, len %d", event.param2, speechc.amr_wb_save_buffer[event.param2].amr_save_data_len);
				speechc.amr_wb_save_buffer[event.param2].amr_save_data_len = 0;
			}
			break;
		case EVENT_MQTT_FORCE_SYNC:
			LUAT_DEBUG_PRINT("broadcast play force resync! %u,%u", play_block_cnt_last, play_block_check_cnt_start);
			play_block_cnt_last = 0;
			cache_data_cnt = 0;
			speechc.downlink_state = DL_STATE_PLAY_WAIT_CACHE_OK;
			for(i = 0; i < PCM_ONE_FRAME_BLOCK_NUM; i++)
			{
				OS_WriteFifo(&speechc.downlink_buffer, speechc.amr_dummy, 61);
			}
			speechc.resync_flag = 0;
			break;
		case EVENT_MQTT_FORCE_STOP:
			LUAT_DEBUG_PRINT("broadcast long time no data!");
			end_broadcast_play();
			break;
		case EVENT_MQTT_MSG:
			switch(event.param1)
			{
			case MQTT_MSG_TCP_TX_DONE:
				//如果用QOS0发送，可以作为发送成功的初步判断依据
				break;
			case MQTT_MSG_CONNACK:

				if(luat_mqtt_ctrl->mqtt_packet_buffer[3] != 0x00){
					LUAT_DEBUG_PRINT("CONACK 0x%02x",luat_mqtt_ctrl->mqtt_packet_buffer[3]);
					luat_mqtt_ctrl->error_state = luat_mqtt_ctrl->mqtt_packet_buffer[3];
	                luat_mqtt_close_socket(luat_mqtt_ctrl);
	                break;
	            }
				LUAT_DEBUG_PRINT("mqtt_connect ok");
				LUAT_DEBUG_PRINT("mqtt_subscribe start");
				mqtt_subscribe(&(luat_mqtt_ctrl->broker), "speech_demo/#", &msgid, 0);
				msgid++;
				break;
			case MQTT_MSG_SUBACK:
				if(luat_mqtt_ctrl->mqtt_packet_buffer[4] > 0x02){
					LUAT_DEBUG_PRINT("SUBACK 0x%02x",luat_mqtt_ctrl->mqtt_packet_buffer[4]);
	                luat_mqtt_close_socket(luat_mqtt_ctrl);
	                break;
	            }
				LUAT_DEBUG_PRINT("mqtt_subscribe ok");
				OS_ReInitBuffer(&uplink, 1024);
				speechc.mqtt_ready = 1;
				luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_ON_LINE, 0, 0, 0, 0);
				break;
			case MQTT_MSG_DISCONNECT:
				LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt disconnect");
				end_broadcast_play();
				luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_OFF_LINE, 0, 0, 0, 0);
				break;
			case MQTT_MSG_TIMER_PING:
				luat_mqtt_ping(luat_mqtt_ctrl);
				break;
			case MQTT_MSG_RECONNECT:
				luat_mqtt_reconnect(luat_mqtt_ctrl);
				break;
			case MQTT_MSG_CLOSE :
				LUAT_DEBUG_PRINT("luat_mqtt_cb mqtt close");
				break;
			}
			break;
		}
	}
}


static __USER_FUNC_IN_RAM__ int record_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param)
{

	if (!speechc.audio_enable) return 0;
	switch(event)
	{
	case LUAT_I2S_EVENT_RX_DONE:
		luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_RUN_ONCE, (uint32_t)rx_data, rx_len, 0, 0);
		break;
	case LUAT_I2S_EVENT_TRANSFER_DONE:
		break;
	default:
		break;
	}
	return 0;
}

static LUAT_RT_RET_TYPE start_play_callback(LUAT_RT_CB_PARAM)
{
	luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_START_TRUE_PLAY, 0, 0, 0, 0);
}

static void start_broadcast_record(void)
{
	LUAT_DEBUG_PRINT("record start!");
	speechc.record_block_cnt = 0;
	speechc.amr_wb_save_buffer_point = 0;
	speechc.amr_wb_save_buffer[0].amr_save_data_len = 0;
	luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_UPLOAD_START, 0, 0, 0, 0);
}
static void speech_task(void *arg)
{
	uint64_t check_time = 0;
	uint64_t amr_times = 0;
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	OS_InitFifo(&speechc.downlink_buffer, luat_heap_malloc(1 << 14), 14);
	luat_audio_play_global_init(NULL, NULL, NULL, NULL, NULL);
	luat_i2s_setup(&luat_i2s_conf_es8311);
	luat_i2s_set_user_data(TEST_I2S_ID, &speechc);
	luat_audio_set_bus_type(MULTIMEDIA_ID,LUAT_MULTIMEDIA_AUDIO_BUS_I2S);
	luat_audio_setup_codec(MULTIMEDIA_ID, &luat_audio_codec_es8311);					//设置音频codec
#if 1
	//自适应I2C，适配多种开发板，实际产品不需要，保留1个就够了
	luat_i2c_setup(I2C_ID0, 1);
	luat_i2c_setup(I2C_ID1, 1);
	uint8_t reg = 0xfd;
	luat_gpio_set(CODEC_PWR_PIN, 1);
	luat_rtos_task_sleep(10);

	if (!luat_i2c_send(I2C_ID0, 0x18, &reg, 1, 1))
	{
		LUAT_DEBUG_PRINT("ES8311 use i2c0");
	}
	else if (!luat_i2c_send(I2C_ID1, 0x18, &reg, 1, 1))
	{
		LUAT_DEBUG_PRINT("ES8311 use i2c1");
		luat_audio_conf_t* audio_conf = luat_audio_get_config(MULTIMEDIA_ID);
		audio_conf->codec_conf.i2c_id = I2C_ID1;
		speechc.codec_exsit = 1;
	}
	else
	{
		LUAT_DEBUG_PRINT("NO ES8311!!!");
		while (1) {luat_rtos_task_sleep(1000000);}
	}
#endif

	luat_audio_config_pa(MULTIMEDIA_ID, PA_PWR_PIN, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
	luat_audio_config_dac(MULTIMEDIA_ID, CODEC_PWR_PIN, PWR_ON_LEVEL, 0);//配置音频dac_power
	luat_audio_init(MULTIMEDIA_ID, TEST_VOL, TEST_MIC_VOL);		//初始化音频

	luat_rtos_timer_t play_start_timer = luat_create_rtos_timer(start_play_callback, NULL, NULL);

	uint16_t pcm_dummy[PCM_BLOCK_LEN >> 1] = {0};

	uint16_t pcm_buff[PCM_BLOCK_LEN >> 1];
	uint8_t amr_buff[64] = {0};
	luat_event_t event;
	uint8_t play_delay = 2;
	size_t total, alloc, peak;
	uint8_t out_len, temp_len, record_cnt, need_stop_record, need_stop_play, wait_stop_play;
	uint16_t *pcm_data;
	//准备空白数据
//	luat_audio_inter_amr_init(1, 8);	//7是NB最高音质，WB最高是8
//	luat_audio_inter_amr_encode(pcm_dummy, speechc.amr_dummy, &speechc.amr_dummy_len);
//	luat_audio_inter_amr_deinit();
//	DBG("%d", speechc.amr_dummy_len);
//	for(int i = 0; i < speechc.amr_dummy_len; i+= 8)
//	{
//		DBG("0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x, 0x%02x", speechc.amr_dummy[i], speechc.amr_dummy[i+1], speechc.amr_dummy[i+2], speechc.amr_dummy[i+3], speechc.amr_dummy[i+4], speechc.amr_dummy[i+5], speechc.amr_dummy[i+6], speechc.amr_dummy[i+7]);
//	}
	memcpy(speechc.amr_dummy, amr_wb_zero_data, 61);
	record_cnt = 0;
	need_stop_record = 0;
	need_stop_play = 0;
	wait_stop_play = 0;
	temp_len = 0;
	while (1)
	{
		luat_rtos_event_recv(speechc.speech_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		switch(event.id)
		{
		case EVENT_AMR_RUN_ONCE:
			if (!speechc.audio_enable)
			{
				break;
			}
			pcm_data = (uint16_t *)event.param1;
			if (play_delay)
			{
				play_delay--;
				if (!play_delay)	//2fps解码后，再delay一段时间真正开始播放
				{
					luat_start_rtos_timer(play_start_timer, 13, 0);
				}

			}
			if (OS_CheckFifoUsedSpace(&speechc.downlink_buffer))
			{
				OS_QueryFifo(&speechc.downlink_buffer, amr_buff, 1);
				if ( ((amr_buff[0] >> 3) > 0x0f) || ((amr_buff[0] & 0x07) != 0x04) )
				{
					LUAT_DEBUG_PRINT("ERROR AMR PARAM! %x", amr_buff[0]);
					speechc.resync_flag = 1;
					luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_FORCE_SYNC, PACKET_TYPE_BROADCAST_END, 0, 0, 0);
					goto ARM_ENCODE;
				}
				temp_len = amr_wb_byte_len[(amr_buff[0] >> 3)] + 1;
				OS_ReadFifo(&speechc.downlink_buffer, amr_buff, temp_len);
				luat_audio_inter_amr_decode(pcm_buff, amr_buff, &out_len);
				goto ARM_ENCODE;
			}
			else
			{
				//如果需要停止，先让缓冲区的数据放完再停止
				if (wait_stop_play)
				{
					wait_stop_play = 0;
					speechc.play_enable = 0;
					LUAT_DEBUG_PRINT("play stop!");
				}
				else if (need_stop_play)
				{
					wait_stop_play = 1;
					need_stop_play = 0;
				}
				else if (speechc.play_enable && !speechc.play_no_more_data && !speechc.resync_flag)
				{
					LUAT_DEBUG_ASSERT(0, "NO DATA");
				}
				else
				{
					if (speechc.play_no_more_data)
					{
						need_stop_play = 1;
						speechc.play_no_more_data = 0;
					}
					luat_audio_inter_amr_decode(pcm_buff, speechc.amr_dummy, &out_len);
				}
			}
ARM_ENCODE:
			if (speechc.record_enable)
			{
				if (!speechc.mqtt_ready)
				{
					need_stop_record = 1;
				}
				luat_audio_inter_amr_encode(pcm_data, amr_buff, &out_len);
				record_cnt++;
				memcpy(speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data + speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len, amr_buff, out_len);
				speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len += out_len;
				if (record_cnt >= PCM_ONE_FRAME_BLOCK_NUM)
				{
					record_cnt = 0;
					luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_UPLINK_DATA, PACKET_TYPE_BROADCAST_DATA, speechc.amr_wb_save_buffer_point, 0, 0);
					//LUAT_DEBUG_PRINT("upload buffer %d", speechc.amr_wb_save_buffer_point);
					speechc.amr_wb_save_buffer_point = (speechc.amr_wb_save_buffer_point + 1) & 7;
					if (speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len)
					{
						LUAT_DEBUG_PRINT("record buffer %d is full!", speechc.amr_wb_save_buffer_point);
					}
					speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len = 0;
					if (need_stop_record)
					{
						LUAT_DEBUG_PRINT("upload stop!");
						luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_UPLINK_DATA, PACKET_TYPE_BROADCAST_END, 0, 0, 0);
						need_stop_record = 0;
						speechc.record_enable = 0;
						luat_rtos_event_send(speechc.led_task_handle, EVENT_LED_UPLOAD_END, PACKET_TYPE_BROADCAST_END, 0, 0, 0);
						luat_gpio_ctrl(SPEECH_CTRL_PIN, LUAT_GPIO_CMD_SET_IRQ_MODE, LUAT_GPIO_RISING_IRQ);
					}
				}
			}
			if (!speechc.record_enable && !speechc.play_enable)
			{
				LUAT_DEBUG_PRINT("audio stop!");
				speechc.audio_enable = 0;
				luat_audio_record_stop(MULTIMEDIA_ID);
				luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
				luat_audio_inter_amr_deinit();
				speechc.downlink_buffer.WPoint = 0;
				speechc.downlink_buffer.RPoint = 0;
				luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
				LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
				luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
				LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
			}
			else
			{
				amr_times++;
				if (luat_mcu_tick64_ms() >= (check_time + (amr_times + 1) * 20))
				{
					LUAT_DEBUG_PRINT("amr codec timeout! %llu", amr_times);
				}
			}

			break;
		case EVENT_AMR_START_TRUE_PLAY:
			check_time = luat_mcu_tick64_ms();
			amr_times = 0;
			luat_audio_record_and_play(MULTIMEDIA_ID, 16000, luat_audio_inter_amr_pcm_address(), 640, 3);
			//luat_audio_record_and_play(MULTIMEDIA_ID, 8000, luat_audio_inter_amr_pcm_address(), 320, 6);
			LUAT_DEBUG_PRINT("true play start!");
			break;
		case EVENT_AMR_START:
			if (!speechc.audio_enable)
			{
				speechc.audio_enable = 1;
				luat_audio_pm_request(MULTIMEDIA_ID, LUAT_AUDIO_PM_RESUME);
				luat_audio_inter_amr_init(1, 8);
				play_delay = 2;
				record_cnt = 0;
				luat_audio_record_and_play(MULTIMEDIA_ID, 16000, NULL, 640, 3);
				//luat_audio_record_and_play(MULTIMEDIA_ID, 8000, NULL, 320, 6);
				check_time = luat_mcu_tick64_ms();
				amr_times = 0;
				if (speechc.record_enable)
				{
					start_broadcast_record();
				}

			}

			break;
		case EVENT_AMR_RECORD_STOP:
			if (speechc.record_enable)
			{
				need_stop_record = 1;
				LUAT_DEBUG_PRINT("record require stop!");
			}
			break;
		case EVENT_AMR_PLAY_STOP:
			if (speechc.play_enable)
			{
				speechc.play_no_more_data = 1;
				LUAT_DEBUG_PRINT("play require stop!");
			}
			break;
		}
	}
}

static int button_irq(void *data, void* args)
{
	luat_gpio_ctrl(SPEECH_CTRL_PIN, LUAT_GPIO_CMD_SET_IRQ_MODE, LUAT_GPIO_NO_IRQ);
	luat_rtos_event_send(speechc.key_task_handle, EVENT_KEY_PIN_IRQ, 0, 0, 0, 0);
	return 0;
}

static void key_task(void *p)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = SPEECH_CTRL_PIN;
	gpio_cfg.alt_fun = SPEECH_CTRL_PIN_ALT_FUN;
	gpio_cfg.mode = LUAT_GPIO_IRQ;
	gpio_cfg.irq_type = LUAT_GPIO_RISING_IRQ;
	gpio_cfg.pull = LUAT_GPIO_PULLDOWN;
	gpio_cfg.irq_cb = button_irq;
	luat_gpio_open(&gpio_cfg);
	luat_event_t event;
	while(1)
	{
WAIT_PRESS:
		luat_rtos_event_recv(speechc.key_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if (BUTTON_PRESS_LEVEL == luat_gpio_get(SPEECH_CTRL_PIN))
		{
			luat_rtos_task_sleep(20);
			if (BUTTON_PRESS_LEVEL == luat_gpio_get(SPEECH_CTRL_PIN))
			{
				for(int i = 0; i < 2; i++)
				{
					luat_rtos_task_sleep(100);
					if (BUTTON_PRESS_LEVEL != luat_gpio_get(SPEECH_CTRL_PIN))
					{
						LUAT_DEBUG_PRINT("short press");
						luat_gpio_ctrl(SPEECH_CTRL_PIN, LUAT_GPIO_CMD_SET_IRQ_MODE, LUAT_GPIO_RISING_IRQ);
						goto WAIT_PRESS;
					}
				}
				LUAT_DEBUG_PRINT("long press start");
				while(1)
				{
					luat_rtos_task_sleep(100);
					if (BUTTON_PRESS_LEVEL != luat_gpio_get(SPEECH_CTRL_PIN))
					{
						LUAT_DEBUG_PRINT("long press end");
						luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_RECORD_STOP, 0, 0, 0, 0);
						goto WAIT_PRESS;
					}
					else
					{

						if (speechc.codec_exsit && speechc.mqtt_ready)
						{
							if (!speechc.audio_enable)
							{
								speechc.record_enable = 1;
								luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_START, 0, 0, 0, 0);
							}
							else
							{
								if (!speechc.record_enable)
								{
									speechc.record_enable = 1;
									start_broadcast_record();
								}
							}
						}
					}
				}
			}

		}
	}
}

static void led_task(void *p)
{
	uint8_t channel = 4;
	uint8_t is_online = 0;
	uint8_t is_download = 0;
	uint8_t is_upload = 0;
	luat_event_t event;
	while(1)
	{

		luat_rtos_event_recv(speechc.led_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		LUAT_DEBUG_PRINT("led require %d", event.id - EVENT_LED_OFF_LINE);
		switch(event.id)
		{
		case EVENT_LED_OFF_LINE:
			is_online = 0;
			is_download = 0;
			luat_pwm_close(channel);
			break;
		case EVENT_LED_ON_LINE:
			is_online = 1;
			is_download = 0;
#ifdef LOW_POWER_ENABLE
			luat_pwm_close(channel);
#else
			luat_pwm_open(channel, 1, 100, 0);
#endif
			break;
		case EVENT_LED_DOWNLOAD_START:
			is_download = 1;
			if (!is_upload && is_online)
			{
				luat_pwm_open(channel, 10, 500, 0);
			}
			break;
		case EVENT_LED_DOWNLOAD_END:
			is_download = 0;
			if (is_upload)
			{
				break;
			}
			if (is_online)
			{
#ifdef LOW_POWER_ENABLE
				luat_pwm_close(channel);
#else
				luat_pwm_open(channel, 1, 100, 0);
#endif
			}
			else
			{
				luat_pwm_close(channel);
			}
			break;
		case EVENT_LED_UPLOAD_START:
			is_upload = 1;
			luat_pwm_open(channel, 10000, 1000, 0);
			break;
		case EVENT_LED_UPLOAD_END:
			is_upload = 0;
			if (is_download && is_online)
			{
				luat_pwm_open(channel, 10, 500, 0);
			}
			else if (is_online)
			{
#ifdef LOW_POWER_ENABLE
				luat_pwm_close(channel);
#else
				luat_pwm_open(channel, 1, 100, 0);
#endif
			}
			else
			{
				luat_pwm_close(channel);
			}
			break;
		}

	}
}

static void speech_demo_init(void)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);

	// pa power ctrl init
	gpio_cfg.pin = PA_PWR_PIN;
	gpio_cfg.alt_fun = PA_PWR_PIN_ALT_FUN;
	luat_gpio_open(&gpio_cfg);

	// codec power ctrl init
	gpio_cfg.pin = CODEC_PWR_PIN;
	gpio_cfg.alt_fun = CODEC_PWR_PIN_ALT_FUN;
	luat_gpio_open(&gpio_cfg);

	luat_rtos_task_create(&speechc.speech_task_handle, 4 * 1024, 100, "speech", speech_task, NULL, 64);
	luat_rtos_task_create(&speechc.key_task_handle, 2 * 1024, 10, "key", key_task, NULL, 16);
	luat_rtos_task_create(&speechc.led_task_handle, 2 * 1024, 10, "led", led_task, NULL, 16);
	luat_rtos_task_create(&speechc.mqtt_task_handle, 4 * 1024, 50, "mqtt", mqtt_task, NULL, 0);

}
#else
static void speech_demo_init(void)
{
	LUAT_DEBUG_PRINT("没有使能denoise mode，无法演示");
}
#endif
INIT_TASK_EXPORT(speech_demo_init, "1");

