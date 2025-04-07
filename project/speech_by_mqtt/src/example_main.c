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
//#define USE_AIR780EPVH
#ifdef USE_AIR780EPVH
#define CODEC_PWR_PIN HAL_GPIO_17
#define CODEC_PWR_PIN_ALT_FUN	4
#else
#define CODEC_PWR_PIN HAL_GPIO_16
#define CODEC_PWR_PIN_ALT_FUN	4
#endif
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
#define PCM_BLOCK_LEN	(AMR_ONE_FRAME_LEN)	//1帧amr-wb编码数据长度
#define PCM_ONE_FRAME_BLOCK_NUM	(5)	//1包数据包含5帧amr-wb编码
#define PCM_CACHE_FRAME_BEFORE_PLAY	(5)	//缓存5包数据后再开始播放
#define PCM_PLAY_FRAME_CNT	(4)	//4个播放缓冲区循环使用
static const uint8_t  amr_wb_byte_len[] = {17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 0, 0, 0, 0, 0, 0};

#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)

#include "audioCfg.h"   //struct AudioConfig_t
#include "mw_nvm_audio.h"
extern void ShareInfoAPGetCPAudioLogCtrl(AudioParaCfgLogControl_t *audioLogCfg);
extern void ShareInfoAPSetCPAudioLogCtrl(AudioParaCfgLogControl_t audioLogCfg);
void log_on(void)
{
	AudioParaCfgLogControl_t audioLogCfg = {0};
	AudioParaCfgCommon_t mAudioCfgCommon = {0};
	AecConfig_t    MwNvmAudioSphTxAEC;
	ecAudioCfgTlvStore *pMwNvmAudioCfg = NULL;
	AudioParaCfgLogControl_t MwNvmAudioLogCtrl;
	pMwNvmAudioCfg  = (ecAudioCfgTlvStore *)luat_heap_malloc(sizeof(ecAudioCfgTlvStore)+ sizeof(AudioParaSphEQBiquard_t)*EC_ADCFG_SPEECH_EQ_BIQUARD_NUMB*EC_ADCFG_SPEECH_TX_NUMB
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
	luat_heap_free(pMwNvmAudioCfg);
}


typedef struct
{
	uint16_t amr_save_data_len;
	uint8_t amr_save_data[64 * PCM_ONE_FRAME_BLOCK_NUM];	//1帧数据5个amr block，100ms
}amr_wb_save_struct;
typedef struct
{
	BSP_FifoStruct downlink_buffer;				//播放缓存fifo，一般存入下行数据，无下行数据时需要存入空白音
	Buffer_Struct downlink_cache;				//下行数据amr缓冲
	luat_rtos_task_handle speech_task_handle;
	luat_rtos_task_handle mqtt_task_handle;
	luat_rtos_task_handle key_task_handle;
	luat_rtos_task_handle led_task_handle;
	luat_rtos_timer_t download_loop_timer;		//下行检查，检查有没有数据delay的情况
	volatile uint32_t record_block_cnt;			//录音总amr block数据量，用于对端检查amr block完整性
	uint8_t *play_data_buffer;						//播放缓冲区
	volatile uint32_t last_play_cnt;						//上一次播放缓冲区，用于回声抑制
	volatile uint32_t current_play_cnt;					//当前播放缓冲区，用于解码数据存入下一个缓存
	amr_wb_save_struct amr_wb_save_buffer[8];	//amr录音上行缓存区，最多缓存8帧数据
	uint8_t audio_enable;						//允许audio功能使能
	uint8_t record_enable;						//允许录音
	uint8_t play_enable;						//允许播放
	uint8_t play_no_more_data;					//所有下行数据都播放完了
	uint8_t resync_flag;						//重新同步标志，没有新数据，且下一个数据delay超过200ms就需要重新同步了
	uint8_t codec_ready;						//es8311准备好了
	uint8_t codec_exsit;						//es8311存在
	uint8_t mqtt_ready;							//es8311存在
	uint8_t downlink_state;						//下行播放状态
	uint8_t amr_wb_save_buffer_point;			//上行数据在发送前保存地址
	uint8_t download_data_flag;					//下行是否正在进行，目前没什么用
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
	.cb_rx_len = AMR_ONE_FRAME_LEN * PCM_ONE_FRAME_BLOCK_NUM,

	.luat_i2s_event_callback = record_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_es8311 = {
    .i2c_id = 0,
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_es8311,
};

enum
{
	EVENT_AMR_RUN_ONCE = 1,		//amr编解码进行一次，回声消除要求编解码必须是同步进行
	EVENT_MQTT_MSG,				//MQTT除了上下行数据外其他消息
	EVENT_MQTT_DOWNLINK_DATA,	//MQTT下行数据，需要解码播放
	EVENT_MQTT_UPLINK_DATA,		//MQTT上行数据，已经编码过了
	EVENT_MQTT_CHECK_DOWNLINK,	//对讲过程中定时检查是否有网络卡顿情况
	EVENT_MQTT_FORCE_SYNC,		//下行数据异常后，重新开始同步数据
	EVENT_MQTT_FORCE_STOP,		//停止对讲流程
	EVENT_KEY_PRESS,
	EVENT_AMR_START,			//audio处理流程开始
	EVENT_AMR_RECORD_STOP,		//录音停止
	EVENT_AMR_PLAY_STOP,		//播放停止
	EVENT_KEY_PIN_IRQ,
	EVENT_LED_OFF_LINE,
	EVENT_LED_ON_LINE,
	EVENT_LED_DOWNLOAD_START,
	EVENT_LED_DOWNLOAD_END,
	EVENT_LED_UPLOAD_START,
	EVENT_LED_UPLOAD_END,

	PACKET_TYPE_BROADCAST_DATA = 0,	//MQTT广播包
	PACKET_TYPE_BROADCAST_END,			//MQTT 广播包没有后续了

	DL_STATE_IDLE = 0,				//对讲没有下行数据
	DL_STATE_WAIT_CACHE_OK,			//对讲播放前缓冲足够的下行数据
	DL_STATE_PLAY,					//对讲播放中
	DL_STATE_PLAY_WAIT_CACHE_OK,	//对讲播放中缓冲下行数据
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
			payload = luat_heap_malloc(len);
			memcpy(payload, ptr, len);
		}
		luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_DOWNLINK_DATA, (uint32_t)topic, (uint32_t)payload, len, 0);
	}
	return;
}

//播放完成
static void end_broadcast_play(void)
{
	LUAT_DEBUG_PRINT("broadcast play end!");
	OS_DeInitBuffer(&speechc.downlink_cache);
	luat_rtos_timer_stop(speechc.download_loop_timer);
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
		//长时间没有数据就停止对讲
		luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_FORCE_STOP, 0, 0, 0, 0);
	}
}

static LUAT_RT_RET_TYPE download_loop(LUAT_RT_CB_PARAM)
{
	//定时检查一下下行数据是否及时接收了
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
						//检查一下播放数据顺序是否有异常，有异常的重新开始同步
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
					case DL_STATE_IDLE:	//准备同步数据
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
					case DL_STATE_WAIT_CACHE_OK:	//缓存PCM_CACHE_FRAME_BEFORE_PLAY帧数据后才能进行播放
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
					case DL_STATE_PLAY://播放中，接收数据，并缓存
						OS_WriteFifo(&speechc.downlink_buffer, (void *)(event.param2 + 20), event.param3 - 20);

						break;
					case DL_STATE_PLAY_WAIT_CACHE_OK://缓存PCM_CACHE_FRAME_BEFORE_PLAY帧数据后重新播放
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
			luat_heap_free((char *)event.param1);
			luat_heap_free((char *)event.param2);
			break;
		case EVENT_MQTT_CHECK_DOWNLINK:
			play_block_check_cnt_start++;
			//数据延迟达到2帧时（200ms），重新开始同步，为了播放正常进行，仍然需要填充空白帧
			if (play_block_cnt_last <= (play_block_check_cnt_start + 2))
			{
				LUAT_DEBUG_PRINT("broadcast play need resync! %u,%u", play_block_cnt_last, play_block_check_cnt_start);
				play_block_cnt_last = 0;
				cache_data_cnt = 0;
				speechc.downlink_state = DL_STATE_PLAY_WAIT_CACHE_OK;
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
			//强制重新同步，一般是接收数据出现异常
			LUAT_DEBUG_PRINT("broadcast play force resync! %u,%u", play_block_cnt_last, play_block_check_cnt_start);
			play_block_cnt_last = 0;
			cache_data_cnt = 0;
			speechc.downlink_state = DL_STATE_PLAY_WAIT_CACHE_OK;
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
		speechc.last_play_cnt = speechc.current_play_cnt;
		speechc.current_play_cnt = (speechc.current_play_cnt + 1) & 0x3;
		luat_rtos_event_send(speechc.speech_task_handle, EVENT_AMR_RUN_ONCE, (uint32_t)rx_data, rx_len, 0, 0);
		break;
	case LUAT_I2S_EVENT_TRANSFER_DONE:
		break;
	default:
		break;
	}
	return 0;
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
	speechc.play_data_buffer = luat_heap_malloc(AMR_ONE_FRAME_LEN * PCM_ONE_FRAME_BLOCK_NUM * PCM_PLAY_FRAME_CNT);
	luat_audio_play_global_init(NULL, NULL, NULL, NULL, NULL);
	luat_i2s_setup(&luat_i2s_conf_es8311);
	luat_i2s_set_user_data(TEST_I2S_ID, &speechc);
	luat_audio_set_bus_type(MULTIMEDIA_ID,LUAT_MULTIMEDIA_AUDIO_BUS_I2S);
	luat_audio_setup_codec(MULTIMEDIA_ID, &luat_audio_codec_es8311);					//设置音频codec
#ifndef USE_AIR780EPVH	//AIR780EPVH固定是I2C0不需要做兼容
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
#else
	luat_i2c_setup(I2C_ID0, 1);
#endif

	luat_audio_config_pa(MULTIMEDIA_ID, PA_PWR_PIN, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
	luat_audio_config_dac(MULTIMEDIA_ID, CODEC_PWR_PIN, PWR_ON_LEVEL, 0);//配置音频dac_power
	luat_audio_init(MULTIMEDIA_ID, TEST_VOL, TEST_MIC_VOL);		//初始化音频
	uint32_t decode_pos = 0;
	uint32_t i;
//	uint16_t pcm_dummy[PCM_BLOCK_LEN >> 1] = {0};
//	uint16_t pcm_buff[PCM_BLOCK_LEN >> 1];
	uint8_t amr_buff[64] = {0};
	uint8_t *ref_input;
	uint32_t ref_point;
	luat_event_t event;
	size_t total, alloc, peak;
	uint8_t out_len, temp_len, need_stop_record, need_stop_play, wait_stop_play;
	uint16_t *pcm_data;
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
			//20ms进行一次amr编解码，SDK必须先编码1个block,然后解码1个block，否则没有回声抑制效果
			if (!speechc.audio_enable)
			{
				break;
			}
			pcm_data = (uint16_t *)event.param1;
			decode_pos = (speechc.current_play_cnt + 1) & 0x03;
			for(i = 0; i < PCM_ONE_FRAME_BLOCK_NUM; i++)
			{
				//有下行的fifo数据，就取出解码
				if (OS_CheckFifoUsedSpace(&speechc.downlink_buffer))
				{
					OS_QueryFifo(&speechc.downlink_buffer, amr_buff, 1);//查询一下amr数据
					if ( ((amr_buff[0] >> 3) > 0x0f) || ((amr_buff[0] & 0x07) != 0x04) )	//无效的amr数据需要重新同步
					{
						LUAT_DEBUG_PRINT("ERROR AMR PARAM! %x", amr_buff[0]);
						speechc.resync_flag = 1;
						luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_FORCE_SYNC, PACKET_TYPE_BROADCAST_END, 0, 0, 0);
						goto ARM_ENCODE;
					}
					temp_len = amr_wb_byte_len[(amr_buff[0] >> 3)] + 1;
					OS_ReadFifo(&speechc.downlink_buffer, amr_buff, temp_len);//真正读出fifo数据
					luat_audio_inter_amr_decode((uint16_t *)&speechc.play_data_buffer[PCM_BLOCK_LEN * PCM_ONE_FRAME_BLOCK_NUM * decode_pos + i * PCM_BLOCK_LEN], amr_buff, &out_len);
				}
				else //没有下行fifo数据了
				{
					//如果需要停止，先让DMA缓冲区的数据放完再停止
					//DMA缓冲区数据播放完了，才能真正的停止播放
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
						//不应该出现的
						LUAT_DEBUG_ASSERT(0, "NO DATA");
					}
					else	//不需要停止播放，那就是解码空白数据
					{
						if (speechc.play_no_more_data)
						{
							need_stop_play = 1;
							speechc.play_no_more_data = 0;
						}
						memset(&speechc.play_data_buffer[PCM_BLOCK_LEN * PCM_ONE_FRAME_BLOCK_NUM * decode_pos + i * PCM_BLOCK_LEN], 0, PCM_BLOCK_LEN);
					}
				}
			}

ARM_ENCODE:
			if (speechc.record_enable)
			{
				//编码一次数据
				if (!speechc.mqtt_ready)
				{
					need_stop_record = 1;
				}
				if (speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len)
				{
					LUAT_DEBUG_PRINT("record buffer %d is full!,clear it", speechc.amr_wb_save_buffer_point);
					speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len = 0;
				}
				ref_point = speechc.last_play_cnt;
				LUAT_DEBUG_PRINT("%d,%d", ref_point, speechc.current_play_cnt);
				for(i = 0; i < PCM_ONE_FRAME_BLOCK_NUM; i++)
				{
					ref_input = &speechc.play_data_buffer[PCM_BLOCK_LEN * PCM_ONE_FRAME_BLOCK_NUM * ref_point + i * PCM_BLOCK_LEN];
					//录音时刻对应的放音数据作为回声消除的参考数据输入，可以完美消除回声
					luat_audio_inter_amr_encode_with_ref(&pcm_data[i * PCM_BLOCK_LEN >> 1], amr_buff, &out_len, ref_input);
					memcpy(speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data + speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len, amr_buff, out_len);
					speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len += out_len;
				}
				//数据保存到上行缓存区，交给MQTT
				luat_rtos_event_send(speechc.mqtt_task_handle, EVENT_MQTT_UPLINK_DATA, PACKET_TYPE_BROADCAST_DATA, speechc.amr_wb_save_buffer_point, 0, 0);
				//LUAT_DEBUG_PRINT("upload buffer %d", speechc.amr_wb_save_buffer_point);
				speechc.amr_wb_save_buffer_point = (speechc.amr_wb_save_buffer_point + 1) & 7;
				if (speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len)
				{
					LUAT_DEBUG_PRINT("record buffer %d is full!", speechc.amr_wb_save_buffer_point);
				}
				speechc.amr_wb_save_buffer[speechc.amr_wb_save_buffer_point].amr_save_data_len = 0;
				//如果有停止录音的请求，让MQTT上行一次终止包
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
			//既没有播放也没有录音，就直接停止audio
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
				if (luat_mcu_tick64_ms() >= (check_time + (amr_times + 1) * 20 * PCM_ONE_FRAME_BLOCK_NUM))
				{
					LUAT_DEBUG_PRINT("amr codec timeout! %llu", amr_times);
				}
			}
			break;
		case EVENT_AMR_START:
			if (!speechc.audio_enable)
			{
				speechc.audio_enable = 1;
				luat_audio_pm_request(MULTIMEDIA_ID, LUAT_AUDIO_PM_RESUME);
				luat_audio_inter_amr_init(1, 8);
				memset(speechc.play_data_buffer, 0, PCM_ONE_FRAME_BLOCK_NUM * PCM_BLOCK_LEN * PCM_PLAY_FRAME_CNT);
				speechc.last_play_cnt = 0;
				speechc.current_play_cnt = 0;
				luat_audio_record_and_play(MULTIMEDIA_ID, 16000, speechc.play_data_buffer, PCM_ONE_FRAME_BLOCK_NUM * PCM_BLOCK_LEN, PCM_PLAY_FRAME_CNT);
				//luat_audio_record_and_play(MULTIMEDIA_ID, speechc.play_data_buffer, PCM_ONE_FRAME_BLOCK_NUM * PCM_BLOCK_LEN, PCM_PLAY_FRAME_CNT);
				check_time = luat_mcu_tick64_ms();
				amr_times = 0;
				if (speechc.record_enable)//已经请求录音了，那么就开始录音了
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

