#include "common_api.h"
#include "luat_rtos.h"
#include "luat_audio_play.h"
#include "luat_i2s.h"
#include "luat_audio.h"
#include "luat_multimedia.h"
#include "ivTTSSDKID_all.h"
#include "ivTTS.h"
#include "amr_alipay_data.h"
#include "amr_2_data.h"
#include "amr_10_data.h"
#include "amr_yuan_data.h"
#include "power_audio.h"
#include "luat_gpio.h"
#include "luat_debug.h"
#include "luat_fs.h"
#include "luat_mem.h"
#include "luat_i2c.h"
#include "luat_pm.h"
#include "luat_mobile.h"
#include "luat_mcu.h"

#include "interf_enc.h"
#include "interf_dec.h"
#include "dec_if.h"
//只是为了让固件更大，没有其他作用
#if (defined TYPE_EC718U) || (defined TYPE_EC718UM) || (defined TYPE_EC718HM)
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

#define TEST_USE_ES8311	0
#define TEST_USE_TM8211 1

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

#if defined FEATURE_IMS_ENABLE	//VOLTE固件才支持通话测试
#define CALL_TEST		//通话测试会关闭掉其他测试，防止冲突
#endif

#define RECORD_ONCE_LEN	10	   //单声道 8K录音单次10个编码块，总共200ms回调 320B 20ms，amr编码要求，20ms一个块
#define RECORD_TIME	(5)	//设置5秒录音，只要ram够，当然可以更长
#define TEST_MAX_TIME	100		//单次测试时间，如果是0就是无限长，单位是录音回调次数

int record_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param);

static HANDLE g_s_amr_encoder_handler;
static uint32_t g_s_record_time;
static Buffer_Struct g_s_amr_rom_file;
static uint8_t g_s_test_only_record = 0;
//如果amr编解码用内部编码，来电时暂停双向对讲测试，会有冲突
static uint8_t g_s_speech_not_volte_test;
static uint8_t g_s_speech_test_while_call_in;

#if (TEST_USE_ES8311 == 1)

static const luat_i2s_conf_t luat_i2s_conf_es8311 ={
	.id = TEST_I2S_ID,
	.mode = LUAT_I2S_MODE_MASTER,
	.channel_format = LUAT_I2S_CHANNEL_RIGHT,
	.standard = LUAT_I2S_MODE_LSB,
	.channel_bits = LUAT_I2S_BITS_16,
	.data_bits = LUAT_I2S_BITS_16,
	.is_full_duplex = 1,
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)	//内部amr支持16K编码，演示一下
	.cb_rx_len = 640 * RECORD_ONCE_LEN,
#else
	.cb_rx_len = 320 * RECORD_ONCE_LEN,
#endif
	.luat_i2s_event_callback = record_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_es8311 = {
    .i2c_id = 0,
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_es8311,
};

static const luat_i2s_conf_t *i2s_conf = &luat_i2s_conf_es8311;
static const luat_audio_codec_conf_t *codec_conf = &luat_audio_codec_es8311;
#else

static const luat_i2s_conf_t luat_i2s_conf_tm8211 =
{
	.id = TEST_I2S_ID,
	.mode = LUAT_I2S_MODE_MASTER,
	.channel_format = LUAT_I2S_CHANNEL_STEREO,
	.standard = LUAT_I2S_MODE_MSB,
	.channel_bits = LUAT_I2S_BITS_16,
	.data_bits = LUAT_I2S_BITS_16,
	.is_full_duplex = 0,
	.luat_i2s_event_callback = record_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_tm8211 = {
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_tm8211,
};

static const luat_i2s_conf_t *i2s_conf = &luat_i2s_conf_tm8211;
static const luat_audio_codec_conf_t *codec_conf = &luat_audio_codec_tm8211;
#endif

extern void download_file();

enum
{
	VOLTE_EVENT_PLAY_TONE = 1,
	VOLTE_EVENT_RECORD_VOICE_START,
	VOLTE_EVENT_RECORD_VOICE_UPLOAD,
	VOLTE_EVENT_PLAY_VOICE,
	VOLTE_EVENT_HANGUP,
	VOLTE_EVENT_CALL_READY,
	AUDIO_EVENT_PLAY_DONE,
};
static const int g_s_amr_nb_sizes[] = { 12, 13, 15, 17, 19, 20, 26, 31, 5, 6, 5, 5, 0, 0, 0, 0 };
static const uint8_t  amr_wb_byte_len[] = {17, 23, 32, 36, 40, 46, 50, 58, 60, 5, 0, 0, 0, 0, 0, 0};

static luat_rtos_task_handle g_s_task_handle;

static void record_encode_amr(uint8_t *data, uint32_t len)
{
	uint8_t outbuf[64];
	int16_t *pcm = (int16_t *)data;
	uint32_t total_len = len >> 1;
	uint32_t done_len = 0;
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
	uint8_t out_len;
#else
	int out_len;
#endif
	while ((total_len - done_len) >= 160)
	{
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
		luat_audio_inter_amr_encode(&pcm[done_len], outbuf, &out_len);
#else
		out_len = Encoder_Interface_Encode(g_s_amr_encoder_handler, MR122, &pcm[done_len], outbuf, 0);
#endif
		if (out_len <= 0)
		{
			LUAT_DEBUG_PRINT("encode error in %d,result %d", done_len, out_len);
		}
		else
		{
			OS_BufferWrite(&g_s_amr_rom_file, outbuf, out_len);
		}
		done_len += 160;
	}
}

static void record_stop_encode_amr(uint8_t *data, uint32_t len)
{
	luat_audio_record_stop(MULTIMEDIA_ID);
	luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
	luat_audio_inter_amr_deinit();
#ifdef LOW_POWER_TEST
	luat_mobile_set_flymode(0, 1);
#endif
#else

	Encoder_Interface_exit(g_s_amr_encoder_handler);
	g_s_amr_encoder_handler = NULL;
#endif
	LUAT_DEBUG_PRINT("amr encode stop");
}

__USER_FUNC_IN_RAM__ int record_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param)
{
	switch(event)
	{
	case LUAT_I2S_EVENT_RX_DONE:
		if (g_s_test_only_record)
		{
			soc_call_function_in_audio(record_encode_amr, (uint32_t)rx_data, rx_len, LUAT_WAIT_FOREVER);
			g_s_record_time++;

			if (g_s_record_time >= (RECORD_TIME * 5))	//8K 5秒 16K 10秒
			{
				soc_call_function_in_audio(record_stop_encode_amr, 0, 0, LUAT_WAIT_FOREVER);
			}
		}
		else
		{
			luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_RECORD_VOICE_UPLOAD, (uint32_t)rx_data, rx_len, 0, 0);
		}

		break;
	case LUAT_I2S_EVENT_TRANSFER_DONE:
		break;
	default:
		break;
	}
	return 0;
}

void audio_event_cb(uint32_t event, void *param)
{
	LUAT_DEBUG_PRINT("%d", event);
	switch(event)
	{
	case LUAT_MULTIMEDIA_CB_AUDIO_DECODE_START:
		luat_audio_check_ready(MULTIMEDIA_ID);
		break;
	case LUAT_MULTIMEDIA_CB_AUDIO_OUTPUT_START:
		break;
	case LUAT_MULTIMEDIA_CB_TTS_INIT:
		break;
	case LUAT_MULTIMEDIA_CB_DECODE_DONE:
	case LUAT_MULTIMEDIA_CB_TTS_DONE:
		if (!luat_audio_play_get_last_error(MULTIMEDIA_ID))
		{
			luat_audio_play_write_blank_raw(MULTIMEDIA_ID, 1, 0);
		}
		break;
	case LUAT_MULTIMEDIA_CB_AUDIO_DONE:
		LUAT_DEBUG_PRINT("audio play done, result=%d!", luat_audio_play_get_last_error(MULTIMEDIA_ID));
		luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
		//通知一下用户task播放完成了
		luat_rtos_event_send(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, luat_audio_play_get_last_error(MULTIMEDIA_ID), 0, 0, 0);
		break;
	}
}

void HAL_I2sSrcAdjustVolumn(int16_t* srcBuf, uint32_t srcTotalNum, uint16_t volScale)
{
	int integer = volScale / 10;
	int decimal = volScale % 10;
	int scale = 0;
	int32_t tmp = 0;
	uint32_t totalNum = srcTotalNum;
	uint32_t step = 0;
	
	while (totalNum)
	{
		if (volScale < 10)
		{
			tmp = ((*(srcBuf + step)) * (256 * integer + 26 * decimal)) >> 8;
		}
		else
		{
			scale = (256 * integer + 26 * decimal) >> 8;
			tmp = (*(srcBuf + step)) * scale;
		}
		
		if (tmp > 32767)
		{
			tmp = 32767;
		}
		else if (tmp < -32768)
		{
			tmp = -32768;
		}
			
		*(srcBuf + step) = (int16_t)tmp;
		step += 1;
		totalNum -= 2;
	}
}

void audio_data_cb(uint8_t *data, uint32_t len, uint8_t bits, uint8_t channels)
{
	//这里可以对音频数据进行软件音量缩放，或者直接清空来静音
	//软件音量缩放参考HAL_I2sSrcAdjustVolumn
	//LUAT_DEBUG_PRINT("%x,%d,%d,%d", data, len, bits, channels);
}

static void tts_config(void)
{
	// 8K英文
	// ivCStrA sdk_id = AISOUND_SDK_USERID_8K_ENG;
	// luat_audio_play_tts_set_resource(ivtts_8k_eng, sdk_id, NULL);

	// 16K英文
	// ivCStrA sdk_id = AISOUND_SDK_USERID_16K_ENG;
	// luat_audio_play_tts_set_resource(ivtts_16k_eng, sdk_id, NULL);

	// 8K中文
	// ivCStrA sdk_id = AISOUND_SDK_USERID_8K;
	// luat_audio_play_tts_set_resource(ivtts_8k, sdk_id, NULL);

	// 16K中文
	ivCStrA sdk_id = AISOUND_SDK_USERID_16K;
	luat_audio_play_tts_set_resource((void*)ivtts_16k, (void*)sdk_id, NULL);
}


static void demo_task(void *arg)
{
	uint8_t *buff;
	uint8_t *amr_buff;
	luat_event_t event;
	size_t total, alloc, peak;
	uint64_t start_tick, end_tick;
	uint32_t i, done_len, run_cnt, speech_test;
	volatile uint32_t cur_play_buf, cur_decode_buf, next_decode_buf;
	uint8_t *org_data, *pcm_data;
	uint8_t out_len;

	HANDLE amr_encoder_handler;
	HANDLE amr_decoder_handler;
#ifdef LOW_POWER_TEST
	luat_mobile_set_flymode(0, 1);
	luat_pm_power_ctrl(LUAT_PM_POWER_USB, 0);	//没接USB的，只需要在开始的时候关闭一次USB就行了
	luat_pm_request(LUAT_PM_SLEEP_MODE_LIGHT);
#endif
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);

	luat_audio_play_global_init_with_task_priority(audio_event_cb, audio_data_cb, luat_audio_play_file_default_fun, luat_audio_play_tts_default_fun, NULL, 50);
	tts_config();

	luat_i2s_setup(i2s_conf);
	
	luat_audio_set_bus_type(MULTIMEDIA_ID,LUAT_MULTIMEDIA_AUDIO_BUS_I2S);	//设置音频总线类型
	luat_audio_setup_codec(MULTIMEDIA_ID, codec_conf);					//设置音频codec

	uint8_t reg = 0xfd;
	luat_rtos_task_sleep(10);
#if (TEST_USE_ES8311==1)
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
	}
#endif
#ifdef PA_NO_CTRL
	luat_audio_config_pa(MULTIMEDIA_ID, 0xff, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
#else
	luat_audio_config_pa(MULTIMEDIA_ID, PA_PWR_PIN, PA_ON_LEVEL, PWR_SLEEP_DELAY, PA_DELAY);//配置音频pa
#endif
	luat_audio_config_dac(MULTIMEDIA_ID, CODEC_PWR_PIN, PWR_ON_LEVEL, 0);//配置音频dac_power
	luat_audio_init(MULTIMEDIA_ID, TEST_VOL, TEST_MIC_VOL);		//初始化音频
#ifdef PA_NO_CTRL
	luat_gpio_set(CODEC_PWR_PIN, 1);
	luat_gpio_set(PA_PWR_PIN, 1);
#endif
#if defined FEATURE_IMS_ENABLE	//VOLTE固件不支持TTS
#else
	// 中文测试用下面的
	char tts_string[] = "支付宝到账123.45元,微信收款9876.12元ABC";
	// 英文测试用下面的
	// char tts_string[] = "hello world, now test once";
#endif

    while (1) {luat_rtos_task_sleep(2000);}

}
extern void audio_play_set_ram_type(LUAT_HEAP_TYPE_E Type);
//自动接听相关

void mobile_voice_data_input(uint8_t *input, uint32_t len, uint32_t sample_rate, uint8_t bits)
{
	luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_PLAY_VOICE, (uint32_t)input, len, sample_rate, 0);

}

static void test_audio_demo_init(void)
{
	luat_mobile_speech_init(MULTIMEDIA_ID,mobile_voice_data_input);
	luat_rtos_task_create(&g_s_task_handle, 8192, 100, "test", demo_task, NULL, 0);
//	audio_play_set_ram_type(LUAT_HEAP_SRAM);		//打开后消耗RAM较多的地方将使用SRAM，否则使用AUTO模式

}

INIT_TASK_EXPORT(test_audio_demo_init, "1");

#endif
