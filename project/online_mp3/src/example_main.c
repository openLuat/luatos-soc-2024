/**
 * 通过HTTP在线播放MP3功能
 * HTTP下载并不是一个很好的流媒体播放方式，MP3格式也不是很好的流媒体播放格式
 * 本demo验证了边下边播的可行性
 * 尽量不要用HTTPS，也不要用阿里的文件服务（连接可靠性不够）
 */

#include "common_api.h"
#include "luat_rtos.h"
#include "luat_audio_play.h"
#include "luat_i2s.h"
#include "luat_audio.h"
#include "luat_multimedia.h"

#include "luat_gpio.h"
#include "luat_debug.h"
#include "luat_fs.h"
#include "luat_mem.h"
#include "luat_i2c.h"
#include "luat_pm.h"
#include "luat_mobile.h"
#include "luat_mcu.h"
#include "luat_network_adapter.h"
#include "luat_http.h"

#define MP3_BUFFER_LEN_MIN	(6 * 1024)	//MP3数据必须大于6KB才开始解码，需要根据实际情况确定
#define MP3_BUFFER_LEN_LOW	(12 * 1024)
#define MP3_BUFFER_LEN_HIGH	(24 * 1024)
#define PCM_BUFFER_LEN	(8000)

//AIR780EP音频扩展板配置，如果用的ES8311而且要低功耗的，不建议用LDO_CTL，换成AGPIO，不换AGPIO的话，需要看休眠演示，在唤醒后重新初始化codec
#define CODEC_PWR_PIN HAL_GPIO_16
#define CODEC_PWR_PIN_ALT_FUN	4
#ifndef CHIP_EC716
#define PA_PWR_PIN HAL_GPIO_25
#define PA_PWR_PIN_ALT_FUN	0
#else
#define PA_PWR_PIN HAL_GPIO_20
#define PA_PWR_PIN_ALT_FUN	0
#endif
#define PA_ON_LEVEL 1
#define PWR_ON_LEVEL 1

//#define LOW_POWER_TEST	//开启低功耗场景测试
//#define CODEC_NO_CTRL	//codec常开场景，音频扩展板的配置
//#define PA_NO_CTRL		//pa常开场景

#define AUDIO_SLEEP_MODE	LUAT_AUDIO_PM_STANDBY	//不追求低功耗的，可以用待机模式，其他模式也可以
#ifdef LOW_POWER_TEST
#undef AUDIO_SLEEP_MODE
#if defined PA_NO_CTRL
#define AUDIO_SLEEP_MODE  LUAT_AUDIO_PM_STANDBY	//PA无法控制的，大部分情况下应该用待机模式，除非硬件上解决了爆破音
#elif defined CODEC_NO_CTRL
#define AUDIO_SLEEP_MODE  LUAT_AUDIO_PM_SHUTDOWN //codec常开的情况下，可以让codec进入shutdown来省电
#else
#define AUDIO_SLEEP_MODE  LUAT_AUDIO_PM_POWER_OFF //一般情况用完全断电来省电
#endif
#endif

#define TEST_I2S_ID I2S_ID0

#define TEST_USE_ES8311	1
#define TEST_USE_TM8211 0

#define MULTIMEDIA_ID 	0	//多媒体id，用于区分不同多媒体硬件
#define TEST_VOL		70	// 测试音量调节
#define TEST_MIC_VOL	75	// 测试麦克风音量调节

#if (TEST_USE_ES8311 == 1)
#define PA_DELAY		200
#define PWR_ON_DELAY	10
#define PWR_SLEEP_DELAY	600
#else
#define PA_DELAY		100
#define PWR_ON_DELAY	0
#define PWR_SLEEP_DELAY	400
#endif

enum
{
	AUDIO_START = 1,
	AUDIO_NEED_DATA,
	MP3_HTTP_GET_HEAD_DONE,
	MP3_HTTP_GET_DATA,
	MP3_HTTP_GET_DATA_DONE,
	MP3_HTTP_FAILED,
	MP3_HTTP_QUIT,

	MP3_PLAY_IDLE = 0,
	MP3_PLAY_RUN,
	MP3_PLAY_NO_DECODE,
	MP3_PLAY_WAIT_DONE,
	MP3_PLAY_DONE,
	MP3_PLAY_ERROR,
};

typedef struct
{
	BSP_FifoStruct mp3_data_buffer;
	Buffer_Struct mp3_decode_data_buffer;
	Buffer_Struct pcm_buffer;
	luat_http_ctrl_t *http;
	void *mp3_decoder;
	uint16_t *mp3_out_data;
	uint8_t *pcm_data;
	uint8_t i2s_cb_flag;	//每2次i2s回调给task发送一次填充新的数据请求
	uint8_t pcm_current_play_cnt;
	uint8_t mp3_play_state;
	uint8_t is_downloading;
}online_mp3_ctrl_t;
static luat_rtos_task_handle http_download_task_handle;
static luat_rtos_task_handle mp3_decode_task_handle;

static __USER_FUNC_IN_RAM__ uint8_t update_play_cnt(uint8_t org)
{
	org++;
	if (org > 2)
	{
		return 0;
	}
	return org;
}

static __USER_FUNC_IN_RAM__ int i2s_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param)
{
	online_mp3_ctrl_t *mp3p = (online_mp3_ctrl_t *)param;
	switch(event)
	{
	case LUAT_I2S_EVENT_RX_DONE:
		if (!mp3p->mp3_play_state) return 0;
		mp3p->i2s_cb_flag = !mp3p->i2s_cb_flag;
		if (!mp3p->i2s_cb_flag)
		{
			mp3p->pcm_current_play_cnt = update_play_cnt(mp3p->pcm_current_play_cnt);
			luat_rtos_event_send(mp3_decode_task_handle, AUDIO_NEED_DATA, (uint32_t)mp3p, 0, 0, 0);
		}
		break;
	case LUAT_I2S_EVENT_TRANSFER_DONE:
		break;
	default:
		break;
	}
	return 0;
}

#if (TEST_USE_ES8311 == 1)

static const luat_i2s_conf_t luat_i2s_conf_es8311 ={
	.id = TEST_I2S_ID,
	.mode = LUAT_I2S_MODE_MASTER,
	.channel_format = LUAT_I2S_CHANNEL_RIGHT,
	.standard = LUAT_I2S_MODE_LSB,
	.channel_bits = LUAT_I2S_BITS_16,
	.data_bits = LUAT_I2S_BITS_16,
	.is_full_duplex = 1,
	.cb_rx_len = PCM_BUFFER_LEN,

	.luat_i2s_event_callback = i2s_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_es8311 = {
    .i2c_id = 0,
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_es8311,
};

static const luat_i2s_conf_t *i2s_conf = &luat_i2s_conf_es8311;
static const luat_audio_codec_conf_t *codec_conf = &luat_audio_codec_es8311;
#endif
#if (TEST_USE_TM8211 == 1)

static const luat_i2s_conf_t luat_i2s_conf_tm8211 =
{
	.id = TEST_I2S_ID,
	.mode = LUAT_I2S_MODE_MASTER,
	.channel_format = LUAT_I2S_CHANNEL_STEREO,
	.standard = LUAT_I2S_MODE_MSB,
	.channel_bits = LUAT_I2S_BITS_16,
	.data_bits = LUAT_I2S_BITS_16,
	.is_full_duplex = 1,
	.cb_rx_len = PCM_BUFFER_LEN,
	.luat_i2s_event_callback = i2s_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_tm8211 = {
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_tm8211,
};

static const luat_i2s_conf_t *i2s_conf = &luat_i2s_conf_tm8211;
static const luat_audio_codec_conf_t *codec_conf = &luat_audio_codec_tm8211;
#endif

static void run_mp3_decode(online_mp3_ctrl_t *mp3p, uint8_t channel_num)
{
	uint32_t mp3_data_len, out_len, hz, used, read_len, i;
	PV_Union uPV;
	int mp3_decode_result;
	mp3_data_len = OS_CheckFifoUsedSpace(&mp3p->mp3_data_buffer);
	while (((mp3_data_len >= 1792) || (!mp3p->is_downloading && (mp3_data_len || mp3p->mp3_decode_data_buffer.Pos))) && mp3p->pcm_buffer.Pos < (2 * PCM_BUFFER_LEN))
	{
		read_len = mp3p->mp3_decode_data_buffer.MaxLen - mp3p->mp3_decode_data_buffer.Pos;
		mp3p->mp3_decode_data_buffer.Pos += OS_ReadFifo(&mp3p->mp3_data_buffer, mp3p->mp3_decode_data_buffer.Data + mp3p->mp3_decode_data_buffer.Pos, read_len);
		mp3_decode_result = mp3_decoder_get_data(mp3p->mp3_decoder, mp3p->mp3_decode_data_buffer.Data, mp3p->mp3_decode_data_buffer.Pos,
				(int16_t *)mp3p->mp3_out_data, &out_len, &hz, &used);
		if (out_len)
		{
			if (i2s_conf->channel_format < LUAT_I2S_CHANNEL_STEREO)
			{
				if (channel_num < 2)
				{
					OS_BufferWrite(&mp3p->pcm_buffer, mp3p->mp3_out_data, out_len);
				}
				else
				{
					//立体声解码，单声道播放，数据量减半
					uPV.pu8 = mp3p->pcm_buffer.Data + mp3p->pcm_buffer.Pos;
					for(i = 0; i < (out_len>>2); i++)
					{
						uPV.pu16[i] = mp3p->mp3_out_data[i * 2];
					}
					mp3p->pcm_buffer.Pos += (out_len >> 1);
				}
			}
			else
			{
				if (channel_num < 2)
				{
					//单声道解码，立体声播放，数据量2倍
					uPV.pu8 = mp3p->pcm_buffer.Data + mp3p->pcm_buffer.Pos;
					for(i = 0; i < (out_len>>1); i++)
					{
						uPV.pu16[2 * i] = mp3p->mp3_out_data[i * 2];
						uPV.pu16[2 * i + 1] = mp3p->mp3_out_data[i * 2];
					}
					mp3p->pcm_buffer.Pos += (out_len << 1);
				}
				else
				{
					OS_BufferWrite(&mp3p->pcm_buffer, mp3p->mp3_out_data, out_len);
				}
			}
		}
		OS_BufferRemove(&mp3p->mp3_decode_data_buffer, used);
		mp3_data_len = OS_CheckFifoUsedSpace(&mp3p->mp3_data_buffer);
	}
	if (!mp3p->is_downloading )
	{
		if (!mp3_data_len && !mp3p->mp3_decode_data_buffer.Pos)
		{
			mp3p->mp3_play_state = MP3_PLAY_NO_DECODE;
			LUAT_DEBUG_PRINT("mp3 decode finish");
		}
	}
}

static void luatos_http_cb(int status, void *data, uint32_t len, void *param)
{
	online_mp3_ctrl_t *mp3p = (online_mp3_ctrl_t *)param;
	if (status < 0)
	{
		LUAT_DEBUG_PRINT("http failed! %d", status);
		luat_rtos_event_send(param, MP3_HTTP_FAILED, 0, 0, 0, 0);
		return;
	}
	uint32_t start, end, total;
	switch(status)
	{
	case HTTP_STATE_GET_BODY:
		if (data)
		{
			OS_WriteFifo(&mp3p->mp3_data_buffer, data, len);
			if (OS_CheckFifoUsedSpace(&mp3p->mp3_data_buffer) > MP3_BUFFER_LEN_HIGH)
			{
//				LUAT_DEBUG_PRINT("http pause");
				luat_http_client_pause(mp3p->http, 1);
			}
			luat_rtos_event_send(http_download_task_handle, MP3_HTTP_GET_DATA, 0, 0, 0, 0);
		}
		else
		{
			luat_rtos_event_send(http_download_task_handle, MP3_HTTP_GET_DATA_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_GET_HEAD:
		if (data)
		{
			LUAT_DEBUG_PRINT("%s", data);
		}
		else
		{
			luat_rtos_event_send(http_download_task_handle, MP3_HTTP_GET_HEAD_DONE, 0, 0, 0, 0);
		}
		break;
	case HTTP_STATE_IDLE:
		break;
//	case HTTP_STATE_SEND_BODY_START:
//		break;
//	case HTTP_STATE_SEND_BODY:
//		break;
	default:
		break;
	}
}

static void online_mp3_download(void *param)
{
	luat_event_t event;
	online_mp3_ctrl_t mp3c;
	/*
		出现异常后默认为死机重启
		demo这里设置为LUAT_DEBUG_FAULT_HANG_RESET出现异常后尝试上传死机信息给PC工具，上传成功或者超时后重启
		如果为了方便调试，可以设置为LUAT_DEBUG_FAULT_HANG，出现异常后死机不重启
		但量产出货一定要设置为出现异常重启！！！！！！！！！1
	*/
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	uint32_t all,now_free_block,min_free_block;
	luat_audio_play_global_init(NULL, NULL, NULL, NULL, NULL);
	luat_i2s_setup(i2s_conf);
	luat_i2s_set_user_data(TEST_I2S_ID, &mp3c);
	luat_audio_set_bus_type(MULTIMEDIA_ID,LUAT_MULTIMEDIA_AUDIO_BUS_I2S);	//设置音频总线类型
	luat_audio_setup_codec(MULTIMEDIA_ID, codec_conf);					//设置音频codec
#if (TEST_USE_ES8311==1)
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
	}
	else
	{
		LUAT_DEBUG_PRINT("NO ES8311!!!");
		while (1) {luat_rtos_task_sleep(1000000);}
	}
#endif
#ifdef PA_NO_CTRL
	luat_audio_config_pa(MULTIMEDIA_ID, 0xff, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
#else
	luat_audio_config_pa(MULTIMEDIA_ID, PA_PWR_PIN, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
#endif
	luat_audio_config_dac(MULTIMEDIA_ID, CODEC_PWR_PIN, PWR_ON_LEVEL, 0);//配置音频dac_power
	luat_audio_init(MULTIMEDIA_ID, TEST_VOL, TEST_MIC_VOL);		//初始化音频

	luat_http_ctrl_t *http_client = luat_http_client_create(luatos_http_cb, &mp3c, -1);
	mp3c.http = http_client;
	mp3c.mp3_decoder = mp3_decoder_create();
	const char remote_domain[] = "http://www.air32.cn/test_44K.mp3";
    //const char remote_domain[] = "https://cloridge-hardware.oss-cn-shanghai.aliyuncs.com/music/test1.mp3";
	uint32_t start, i;
	uint8_t get_mp3_head = 0;
	uint8_t is_error;
	uint8_t mp3_head_data[12];
	uint8_t mp3_head_len = 0;

	luat_http_client_base_config(http_client, 5000, 0, 3);
	//https下载额外打开ssl配置
	//luat_http_client_ssl_config(g_s_http_client, 0, NULL, 0, NULL, 0, NULL, 0, NULL, 0);
	//根据实际情况，可以把LUAT_HEAP_SRAM换成LUAT_HEAP_PSRAM来节省SRAM使用
	OS_InitFifo(&mp3c.mp3_data_buffer, luat_heap_opt_malloc(LUAT_HEAP_SRAM, 1 << 15), 15);//32K缓存MP3数据空间
	mp3c.pcm_data = luat_heap_opt_malloc(LUAT_HEAP_SRAM, 6 * PCM_BUFFER_LEN);	//PCM loop播放cache 48K, 这样44.1K 立体声 MP3大约有270ms的播放时间，每90ms更新数据
	OS_InitBuffer(&mp3c.pcm_buffer, PCM_BUFFER_LEN * 2 + 4608);	//PCM临时数据缓存 16K+4608
	OS_InitBuffer(&mp3c.mp3_decode_data_buffer, 2000);
	mp3c.mp3_out_data = luat_heap_malloc(1152 * 4);
	while(1)
	{
		luat_meminfo_sys(&all, &now_free_block, &min_free_block);
		LUAT_DEBUG_PRINT("meminfo %d,%d,%d",all,now_free_block,min_free_block);
		get_mp3_head = 0;
		is_error = 0;
		OS_DeleteFifo(&mp3c.mp3_data_buffer, mp3c.mp3_data_buffer.Size);
		luat_http_client_clear(http_client);
		luat_http_client_set_user_head(http_client, "Range", "bytes=0-11");
		luat_http_client_start(http_client, remote_domain, 0, 0, 1);

		luat_audio_record_and_play(MULTIMEDIA_ID, 8000, NULL, 1024, 6);
		luat_audio_pa(MULTIMEDIA_ID, 1, PA_DELAY);
		while (!get_mp3_head)
		{
			luat_rtos_event_recv(http_download_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
			switch(event.id)
			{
			case MP3_HTTP_GET_HEAD_DONE:
				LUAT_DEBUG_PRINT("status %d ", luat_http_client_get_status_code(http_client));
				break;
			case MP3_HTTP_GET_DATA:
				break;
			case MP3_HTTP_GET_DATA_DONE:
				get_mp3_head = 1;
				break;
			case MP3_HTTP_FAILED:
				get_mp3_head = 1;
				mp3_head_len = 0;
				break;
			}
		}
		luat_http_client_close(http_client);
		luat_http_client_clear(http_client);

		mp3_head_len = OS_CheckFifoUsedSpace(&mp3c.mp3_data_buffer);
		if (get_mp3_head && mp3_head_len >= 12)
		{
			memcpy(mp3_head_data, mp3c.mp3_data_buffer.Data, mp3_head_len);
			if (!memcmp(mp3_head_data, "ID3", 3) || (mp3_head_data[0] == 0xff))
			{
				start = 0;
				if (mp3_head_data[0] != 0xff)
				{
					//跳过无用的数据
					for(i = 0; i < 4; i++)
					{
						start <<= 7;
						start |= mp3_head_data[6 + i] & 0x7f;
					}
				}
			}
			else
			{
				LUAT_DEBUG_PRINT("不是MP3文件，退出");
				LUAT_DEBUG_PRINT("%.*s", mp3_head_len, mp3_head_data);
				goto MP3_PLAY_END;
			}
		}
		else
		{
			LUAT_DEBUG_PRINT("下载MP3文件头失败");
			goto MP3_PLAY_END;
		}
		//播放相关数据清空，状态还原
		OS_DeleteFifo(&mp3c.mp3_data_buffer, mp3c.mp3_data_buffer.Size);
		OS_BufferRemove(&mp3c.pcm_buffer, mp3c.pcm_buffer.MaxLen);
		memset(mp3c.pcm_data, 0, 6 * PCM_BUFFER_LEN);
		mp3c.mp3_play_state = MP3_PLAY_IDLE;
		mp3c.i2s_cb_flag = 0;
		mp3c.pcm_current_play_cnt = 0;

		luat_http_client_set_get_offset(http_client, start);
		luat_http_client_start(http_client, remote_domain, 0, 0, 1);
		mp3c.is_downloading = 1;
		is_error = 0;
		while(!is_error && (mp3c.mp3_play_state < MP3_PLAY_DONE))
		{
			luat_rtos_event_recv(http_download_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
			switch(event.id)
			{

			case MP3_HTTP_GET_HEAD_DONE:
				LUAT_DEBUG_PRINT("status %d", luat_http_client_get_status_code(http_client));
				if (luat_http_client_get_status_code(http_client) != 206)
				{
					mp3c.is_downloading = 0;
					is_error = 1;
				}
				break;
			case MP3_HTTP_GET_DATA:
				if (!mp3c.mp3_play_state)
				{
					if (OS_CheckFifoUsedSpace(&mp3c.mp3_data_buffer) > MP3_BUFFER_LEN_MIN)
					{
						luat_rtos_event_send(mp3_decode_task_handle, AUDIO_START, (uint32_t)&mp3c, 0, 0, 0);
					}
				}
				break;
			case MP3_HTTP_GET_DATA_DONE:
				LUAT_DEBUG_PRINT("mp3 download finish!");
				mp3c.is_downloading = 0;
				break;
			case MP3_HTTP_FAILED:
				LUAT_DEBUG_PRINT("mp3 download fail!");
				mp3c.is_downloading = 0;
				is_error = 1;
				break;
			case MP3_HTTP_QUIT:
				goto MP3_PLAY_END;
				break;
			}
		}
MP3_PLAY_END:
		luat_http_client_close(http_client);
		mp3c.is_downloading = 0;
		LUAT_DEBUG_PRINT("本次MP3下载结束，60秒后再次演示");
		if (is_error)
		{
			LUAT_DEBUG_PRINT("本次MP3边下边播发生错误");
		}
		luat_audio_record_stop(MULTIMEDIA_ID);
		luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
		luat_meminfo_sys(&all, &now_free_block, &min_free_block);
		LUAT_DEBUG_PRINT("meminfo %d,%d,%d",all,now_free_block,min_free_block);
		luat_rtos_task_sleep(60000);
	}
}

static void online_mp3_decode(void *p)
{
	luat_event_t event;
	online_mp3_ctrl_t *mp3p;
	uint32_t mp3_decode_buffer_cnt, sample_rate;
	int mp3_decode_result;
	uint8_t num_channels;
	while(1)
	{
		luat_rtos_event_recv(mp3_decode_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		mp3p = (online_mp3_ctrl_t *)event.param1;

//				LUAT_DEBUG_PRINT("%d", mp3_decode_buffer_cnt);
		if (AUDIO_NEED_DATA == event.id)
		{
			mp3_decode_buffer_cnt = update_play_cnt(mp3p->pcm_current_play_cnt);
			switch (mp3p->mp3_play_state)
			{
			case MP3_PLAY_IDLE:
				break;
			case MP3_PLAY_RUN:
				if (mp3p->pcm_buffer.Pos >= 2 * PCM_BUFFER_LEN)
				{
					memcpy(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, mp3p->pcm_buffer.Data, 2 * PCM_BUFFER_LEN);
					OS_BufferRemove(&mp3p->pcm_buffer, 2 * PCM_BUFFER_LEN);
				}
				else
				{
					DBG("no data");
					memset(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, 0, 2 * PCM_BUFFER_LEN);
				}
				run_mp3_decode(mp3p, num_channels);
				if (mp3p->is_downloading)
				{
					if (OS_CheckFifoUsedSpace(&mp3p->mp3_data_buffer) < MP3_BUFFER_LEN_LOW)
					{
						if (mp3p->http->is_pause)
						{
//							LUAT_DEBUG_PRINT("http recovery");
							luat_http_client_pause(mp3p->http, 0);
						}
					}
				}
				break;
			case MP3_PLAY_NO_DECODE:
				if (mp3p->pcm_buffer.Pos >= 2 * PCM_BUFFER_LEN)
				{
					memcpy(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, mp3p->pcm_buffer.Data, 2 * PCM_BUFFER_LEN);
					OS_BufferRemove(&mp3p->pcm_buffer, 2 * PCM_BUFFER_LEN);
				}
				else if (mp3p->pcm_buffer.Pos)
				{
					memset(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, 0, 2 * PCM_BUFFER_LEN);
					memcpy(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, mp3p->pcm_buffer.Data, mp3p->pcm_buffer.Pos);
					OS_BufferRemove(&mp3p->pcm_buffer, mp3p->pcm_buffer.Pos);
				}
				else
				{
					mp3p->mp3_play_state = MP3_PLAY_WAIT_DONE;
					memset(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, 0, 2 * PCM_BUFFER_LEN);
				}
				break;
			case MP3_PLAY_WAIT_DONE:
				mp3p->mp3_play_state = MP3_PLAY_DONE;
				memset(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, 0, 2 * PCM_BUFFER_LEN);
				LUAT_DEBUG_PRINT("mp3 play succ!!!");
				luat_rtos_event_send(http_download_task_handle, MP3_HTTP_QUIT, 0, 0, 0, 0);
				break;
			default:
				memset(mp3p->pcm_data + mp3_decode_buffer_cnt * 2 * PCM_BUFFER_LEN, 0, 2 * PCM_BUFFER_LEN);
				break;
			}
		}
		else
		{
			if (!mp3p->mp3_play_state)
			{
				mp3_decoder_init(mp3p->mp3_decoder);
				mp3_decode_result = mp3_decoder_get_info(mp3p->mp3_decoder, mp3p->mp3_data_buffer.Data, OS_CheckFifoUsedSpace(&mp3p->mp3_data_buffer), &sample_rate, &num_channels);
				if (mp3_decode_result)
				{
					mp3_decoder_init(mp3p->mp3_decoder);
					LUAT_DEBUG_PRINT("mp3 %d,%d play start", sample_rate, num_channels);
					run_mp3_decode(mp3p, num_channels);
					luat_audio_record_and_play(MULTIMEDIA_ID, sample_rate, mp3p->pcm_data, PCM_BUFFER_LEN, 6);
					mp3p->mp3_play_state = MP3_PLAY_RUN;
				}
				else
				{
					LUAT_DEBUG_PRINT("mp3 decode fail!");
					mp3p->mp3_play_state = MP3_PLAY_ERROR;
					luat_rtos_event_send(http_download_task_handle, MP3_HTTP_QUIT, 0, 0, 0, 0);
				}
			}
		}

	}

}

static void luat_test_init(void)
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
	luat_rtos_task_create(&http_download_task_handle, 4 * 1024, 50, "http_dl", online_mp3_download, NULL, 0);
	luat_rtos_task_create(&mp3_decode_task_handle, 10 * 1024, 100, "decode", online_mp3_decode, NULL, 64);
}

INIT_TASK_EXPORT(luat_test_init, "1");
