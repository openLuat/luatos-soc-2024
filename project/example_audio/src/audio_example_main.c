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
//AIR780EP和AIR780EPV支持内置编解码amr，同时默认在编码时开启降噪功能，在build.bat里设置set DENOISE_FORCE=enable 或者 set CHIP_TARGET=ec718pv
//开启内置降噪功能后，会使能宏定义FEATURE_AMR_CP_ENABLE和FEATURE_VEM_CP_ENABLE
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

typedef struct
{
	uint8_t data[61 * RECORD_ONCE_LEN];
	uint16_t len;
}amr_data_backup_struct;

amr_data_backup_struct amr_play_data_backup[4];

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
	volatile uint32_t last_play_buf, cur_play_buf, cur_decode_buf, next_decode_buf;
	uint8_t *org_data, *pcm_data;
	uint8_t out_len;
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
	uint8_t dummy_data[640];
	uint16_t dummy_decode_pos, dummy_encode_len;
#endif
	HANDLE amr_encoder_handler;
	HANDLE amr_decoder_handler;
#ifdef LOW_POWER_TEST
	luat_mobile_set_flymode(0, 1);
	luat_pm_power_ctrl(LUAT_PM_POWER_USB, 0);	//没接USB的，只需要在开始的时候关闭一次USB就行了
	luat_pm_request(LUAT_PM_SLEEP_MODE_LIGHT);
#endif
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
//如果不需要播放文件/TTS，例如只需要录音（amr编码放到自己的task里），或者全双工对讲，可以不需要开启audio_play任务，初始化的调用如下代码，可以节省13KB ram
//	luat_audio_play_global_init_with_task_priority(NULL, NULL, NULL, NULL, NULL, 50)
#if defined FEATURE_IMS_ENABLE	//VOLTE固件不支持TTS
	luat_audio_play_global_init_with_task_priority(audio_event_cb, audio_data_cb, luat_audio_play_file_default_fun, NULL, NULL, 100);
#else
	luat_audio_play_global_init_with_task_priority(audio_event_cb, audio_data_cb, luat_audio_play_file_default_fun, luat_audio_play_tts_default_fun, NULL, 50);
	tts_config();
#endif

	luat_i2s_setup(i2s_conf);
	
	luat_audio_set_bus_type(MULTIMEDIA_ID,LUAT_MULTIMEDIA_AUDIO_BUS_I2S);	//设置音频总线类型
	luat_audio_setup_codec(MULTIMEDIA_ID, codec_conf);					//设置音频codec
#if (TEST_USE_ES8311==1)
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

#ifdef CALL_TEST
	uint8_t is_call_uplink_on = 0;
#else
	luat_audio_play_info_t mp3_info[4] = {0};
	luat_audio_play_info_t amr_info[5] = {0};
	download_file();
#endif
#ifdef CALL_TEST
	//luat_audio_pm_request(MULTIMEDIA_ID,LUAT_AUDIO_PM_SHUTDOWN);
    luat_rtos_event_recv(g_s_task_handle, VOLTE_EVENT_CALL_READY, &event, NULL, LUAT_WAIT_FOREVER);
    //luat_mobile_make_call(MULTIMEDIA_ID, "xxxxxxxxxx", 11);
#endif

    while(1)
    {
#ifdef CALL_TEST
		//带ES8311+780EPV的演示VOLTE通话
    	luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if (TEST_USE_ES8311)
		{
			switch(event.id)
			{
			case VOLTE_EVENT_PLAY_TONE:
				if (!event.param1)
				{
					is_call_uplink_on = 0;
					luat_audio_speech_stop(MULTIMEDIA_ID);
#ifdef LOW_POWER_TEST
					luat_audio_pm_request(MULTIMEDIA_ID,AUDIO_SLEEP_MODE);
#else
					luat_audio_pm_request(MULTIMEDIA_ID,LUAT_AUDIO_PM_SHUTDOWN);	//LUAT_AUDIO_PM_POWER_OFF也可以
#endif
				}
				luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
				LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
				luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
				LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
				break;
			case VOLTE_EVENT_RECORD_VOICE_START:
				is_call_uplink_on = 1;
				luat_audio_speech(MULTIMEDIA_ID, 0, event.param1, NULL, 0, 1);
				break;
			case VOLTE_EVENT_RECORD_VOICE_UPLOAD:
				if (is_call_uplink_on)
				{
					luat_mobile_speech_upload((uint8_t *)event.param1, event.param2);
				}
				break;
			case VOLTE_EVENT_PLAY_VOICE:
				luat_audio_speech(MULTIMEDIA_ID, 1, event.param3, (uint8_t *)event.param1, event.param2, 1);
				break;
			case VOLTE_EVENT_HANGUP:
				luat_mobile_hangup_call(MULTIMEDIA_ID);
				break;
			}
		}
#else
		mp3_info[0].path = "test1.mp3";
		mp3_info[1].path = "test2.mp3";
		mp3_info[2].path = "test3.mp3";
		mp3_info[3].path = "test4.mp3";
		amr_info[0].path = NULL;
		amr_info[0].address = (uint32_t)amr_alipay_data;
		amr_info[0].rom_data_len = sizeof(amr_alipay_data);
		amr_info[1].path = NULL;
		amr_info[1].address = (uint32_t)amr_2_data;
		amr_info[1].rom_data_len = sizeof(amr_2_data);
		amr_info[2].path = NULL;
		amr_info[2].address = (uint32_t)amr_10_data;
		amr_info[2].rom_data_len = sizeof(amr_10_data);
		amr_info[3].path = NULL;
		amr_info[3].address = (uint32_t)amr_2_data;
		amr_info[3].rom_data_len = sizeof(amr_2_data);
		amr_info[4].path = NULL;
		amr_info[4].address = (uint32_t)amr_yuan_data;
		amr_info[4].rom_data_len = sizeof(amr_yuan_data);
		luat_audio_play_multi_files(MULTIMEDIA_ID, mp3_info, 4);
		luat_rtos_event_recv(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, &event, NULL, LUAT_WAIT_FOREVER);
		luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
		luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
		luat_rtos_task_sleep(2000);
		luat_audio_play_multi_files(MULTIMEDIA_ID, amr_info, 5);
		luat_rtos_event_recv(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, &event, NULL, LUAT_WAIT_FOREVER);
		luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
		luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
		luat_rtos_task_sleep(2000);

#if defined FEATURE_IMS_ENABLE	//VOLTE固件不支持TTS
#else
		luat_audio_play_tts_text(MULTIMEDIA_ID, tts_string, sizeof(tts_string));
		luat_rtos_event_recv(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, &event, NULL, LUAT_WAIT_FOREVER);
		luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
		luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
		luat_rtos_task_sleep(2000);
#endif

		//带ES8311的演示录音后再放音，录音15秒
		if (TEST_USE_ES8311)
		{
			LUAT_DEBUG_PRINT("record test start");
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifdef LOW_POWER_TEST
			//内部AMR解码需要确保不在飞行模式，低功耗测试时强制进入了，所以这里需要先退出
			luat_mobile_set_flymode(0, 0);
			luat_rtos_task_sleep(1000);
#endif
			luat_audio_inter_amr_init(0, 7);
#else
			g_s_amr_encoder_handler = Encoder_Interface_init(0);
#endif
			g_s_record_time = 0;
			g_s_amr_rom_file.Pos = 0;
			OS_BufferWrite(&g_s_amr_rom_file, "#!AMR\n", 6);
			g_s_test_only_record = 1;
			luat_audio_record_and_play(MULTIMEDIA_ID, 8000, NULL, 3200, 2); //放音buffer填NULL，就是喇叭静音
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
			luat_rtos_task_sleep((RECORD_TIME + 1) * 2000);
#else
			luat_rtos_task_sleep((RECORD_TIME + 1) * 1000);
#endif
			g_s_test_only_record = 0;
			amr_info[0].address = (uint32_t)g_s_amr_rom_file.Data;
			amr_info[0].rom_data_len = g_s_amr_rom_file.Pos;
			luat_audio_play_multi_files(MULTIMEDIA_ID, amr_info, 1);
			luat_rtos_event_recv(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, &event, NULL, LUAT_WAIT_FOREVER);
			luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
			LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
			luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
			LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
			luat_rtos_task_sleep(2000);
		}
		//带ES8311的演示双向对讲，默认演示20秒
		if (TEST_USE_ES8311)
		{
			LUAT_DEBUG_PRINT("speech test start");
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
#ifdef LOW_POWER_TEST
			//内部AMR解码需要确保不在飞行模式，低功耗测试时强制进入了，所以这里需要先退出
			luat_mobile_set_flymode(0, 0);
			luat_rtos_task_sleep(1000);
#endif
			luat_audio_inter_amr_init(1, 8);	//
			buff = luat_heap_opt_zalloc(LUAT_HEAP_SRAM, RECORD_ONCE_LEN * 640 * 4);
			amr_buff = luat_heap_opt_zalloc(LUAT_HEAP_SRAM, RECORD_ONCE_LEN * 640);
#else
			amr_encoder_handler = Encoder_Interface_init(0);
			amr_decoder_handler = Decoder_Interface_init();
			buff = luat_heap_opt_zalloc(LUAT_HEAP_SRAM, RECORD_ONCE_LEN * 320 * 4);
			amr_buff = luat_heap_opt_zalloc(LUAT_HEAP_SRAM, RECORD_ONCE_LEN * 320);
#endif

			cur_play_buf = 0;
			cur_decode_buf = 0;
			next_decode_buf = 0;
			g_s_speech_not_volte_test = 1;
			g_s_test_only_record = 0;
			run_cnt = 0;
			speech_test = 1;
			luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
			LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
			luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
			LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
#if defined (FEATURE_AMR_CP_ENABLE) || defined (FEATURE_VEM_CP_ENABLE)
			//内部amr编解码可以用16K，这里演示16K
			luat_audio_record_and_play(MULTIMEDIA_ID, 16000, buff, 640 * RECORD_ONCE_LEN, 4);
			while (speech_test)
			{
				luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
				if (event.id == VOLTE_EVENT_RECORD_VOICE_UPLOAD)
				{
					if (g_s_speech_test_while_call_in)
					{
						continue;
					}
					org_data = (uint8_t *)event.param1;
					start_tick = luat_mcu_tick64_ms();
					done_len = 0;
					last_play_buf = cur_play_buf;
					cur_play_buf = (cur_play_buf + 1) & 3;
					dummy_decode_pos = 0;
					LUAT_DEBUG_PRINT("load play buf %d data len %d", last_play_buf, amr_play_data_backup[last_play_buf].len);
					for(i = 0; i < (RECORD_ONCE_LEN); i++)
					{
						//用录音时间段内保存的放音数据作为回声消除的数据源
						if (amr_play_data_backup[last_play_buf].len)
						{
							luat_audio_inter_amr_decode((uint16_t *)dummy_data, &amr_play_data_backup[last_play_buf].data[dummy_decode_pos], &out_len);
							dummy_decode_pos += out_len;
						}
						luat_audio_inter_amr_encode((uint16_t *)&org_data[i * 640], &amr_buff[done_len], &out_len);
						done_len += out_len;
					}
					amr_play_data_backup[last_play_buf].len = 0;
					dummy_encode_len = done_len;
					cur_decode_buf = (cur_play_buf + 1) & 3;
					pcm_data = &buff[cur_decode_buf * 640 * (RECORD_ONCE_LEN)];

					memcpy(amr_play_data_backup[cur_decode_buf].data, amr_buff, dummy_encode_len);
					amr_play_data_backup[cur_decode_buf].len = dummy_encode_len;
					LUAT_DEBUG_PRINT("save play buf %d data len %d", cur_decode_buf, amr_play_data_backup[cur_decode_buf].len);

					done_len = 0;
					for(i = 0; i < (RECORD_ONCE_LEN); i++)
					{
						luat_audio_inter_amr_decode((uint16_t *)&pcm_data[640 * i], &amr_buff[done_len], &out_len);
						done_len += out_len;
					}
					next_decode_buf = (cur_decode_buf + 1) & 3;
					memset(&buff[next_decode_buf * 640 * (RECORD_ONCE_LEN)], 0, 640 * (RECORD_ONCE_LEN));

					end_tick = luat_mcu_tick64_ms();
					LUAT_DEBUG_PRINT("take time %ums %d,%d,%d", (uint32_t)(end_tick - start_tick), cur_play_buf, cur_decode_buf, next_decode_buf);
					if (!TEST_MAX_TIME)
					{
						luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
						LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
						luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
						LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
					}
					if (TEST_MAX_TIME)
					{
						if (run_cnt++ > TEST_MAX_TIME)
						{
							speech_test = 0;
							LUAT_DEBUG_PRINT("test stop");
							luat_audio_record_stop(MULTIMEDIA_ID);
							luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
							luat_heap_free(buff);
							buff = NULL;
							luat_heap_free(amr_buff);
							amr_buff = NULL;
							luat_audio_inter_amr_deinit();
#ifdef LOW_POWER_TEST
							luat_mobile_set_flymode(0, 1);
#endif
							luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
							LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
							luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
							LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
						}
					}
				}
			}
			g_s_speech_not_volte_test = 0;
#else
			luat_audio_record_and_play(MULTIMEDIA_ID, 8000, buff, 320 * RECORD_ONCE_LEN, 4);
			while (speech_test)
			{
				luat_rtos_event_recv(g_s_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
				if (event.id == VOLTE_EVENT_RECORD_VOICE_UPLOAD)
				{
					org_data = (uint8_t *)event.param1;
					start_tick = luat_mcu_tick64_ms();
					done_len = 0;
					cur_play_buf = (cur_play_buf + 1) & 3;
					for(i = 0; i < RECORD_ONCE_LEN; i++)
					{
						out_len = Encoder_Interface_Encode(amr_encoder_handler, MR122, (uint16_t *)&org_data[i * 320], &amr_buff[done_len], 0);
						done_len += out_len;
					}

					cur_decode_buf = (cur_play_buf + 1) & 3;
					pcm_data = &buff[cur_decode_buf * 320 * RECORD_ONCE_LEN];
					done_len = 0;
					for(i = 0; i < RECORD_ONCE_LEN; i++)
					{
						out_len = g_s_amr_nb_sizes[(amr_buff[done_len] >> 3) & 0x0f];
						Decoder_Interface_Decode(amr_decoder_handler, &amr_buff[done_len], (uint16_t *)&pcm_data[320 * i], 0);
						done_len += 1+out_len;
					}
					next_decode_buf = (cur_decode_buf + 1) & 3;
					memset(&buff[next_decode_buf * 320 * RECORD_ONCE_LEN], 0, 320 * RECORD_ONCE_LEN);

					end_tick = luat_mcu_tick64_ms();
					LUAT_DEBUG_PRINT("take time %ums %d,%d,%d", (uint32_t)(end_tick - start_tick), cur_play_buf, cur_decode_buf, next_decode_buf);
					if (!TEST_MAX_TIME)
					{
						luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
						LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
						luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
						LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
					}
					if (TEST_MAX_TIME)
					{
						if (run_cnt++ > TEST_MAX_TIME)
						{
							speech_test = 0;
							LUAT_DEBUG_PRINT("test stop");
							luat_audio_record_stop(MULTIMEDIA_ID);
							luat_audio_pm_request(MULTIMEDIA_ID, AUDIO_SLEEP_MODE);
							luat_heap_opt_free(LUAT_HEAP_AUTO, buff);
							buff = NULL;
							luat_heap_opt_free(LUAT_HEAP_AUTO, amr_buff);
							amr_buff = NULL;
							Encoder_Interface_exit(amr_encoder_handler);
							amr_encoder_handler = NULL;
							Decoder_Interface_exit(amr_decoder_handler);
							amr_decoder_handler = NULL;
							luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
							LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
							luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
							LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
						}
					}
				}
			}
#endif
		}
		//演示audio休眠

		LUAT_DEBUG_PRINT("audio shutdown 20 second");
#ifdef LOW_POWER_TEST
		luat_audio_pm_request(MULTIMEDIA_ID,AUDIO_SLEEP_MODE);
#else
		luat_audio_pm_request(MULTIMEDIA_ID,LUAT_AUDIO_PM_SHUTDOWN);	//LUAT_AUDIO_PM_POWER_OFF也可以
#endif
		luat_rtos_task_sleep(20000);

#endif
    }
}
extern void audio_play_set_ram_type(LUAT_HEAP_TYPE_E Type);
//自动接听相关
static uint8_t g_s_ring_cnt;

static void mobile_event_cb(uint8_t event, uint8_t index, uint8_t status)
{

	char number[64];
	switch (event)
	{
	case LUAT_MOBILE_EVENT_PDP:
		if (7 == index && (LUAT_MOBILE_PDP_DEACTIVED == status))
		{
			//如果amr编解码用内部编码，来电时暂停双向对讲测试，会有冲突，这里才能恢复
			if (g_s_speech_not_volte_test && g_s_speech_test_while_call_in)
			{
				g_s_speech_test_while_call_in = 0;
				luat_audio_inter_amr_init(1, 8);
				LUAT_DEBUG_PRINT("speech test go on");
			}

		}
		break;
	case LUAT_MOBILE_EVENT_IMS_REGISTER_STATUS:
		LUAT_DEBUG_PRINT("ims reg state %d", status);
		break;
	case LUAT_MOBILE_EVENT_CC:
		switch(status)
		{
		case LUAT_MOBILE_CC_READY:
			LUAT_DEBUG_PRINT("volte ready!");
			luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_CALL_READY, 0, 0, 0, 0);
			break;
		case LUAT_MOBILE_CC_INCOMINGCALL:
			LUAT_DEBUG_PRINT("ring!");
			g_s_ring_cnt++;
			if (g_s_ring_cnt >= 2)
			{
				luat_mobile_answer_call(0);
				g_s_ring_cnt = 0;
			}
			break;
		case LUAT_MOBILE_CC_CALL_NUMBER:
			luat_mobile_get_last_call_num(number, sizeof(number));
			LUAT_DEBUG_PRINT("incoming call %s", number);
			break;
		case LUAT_MOBILE_CC_CONNECTED_NUMBER:
			luat_mobile_get_last_call_num(number, sizeof(number));
			LUAT_DEBUG_PRINT("connected call %s", number);
			break;
		case LUAT_MOBILE_CC_CONNECTED:
			LUAT_DEBUG_PRINT("call connected");
			break;
		case LUAT_MOBILE_CC_DISCONNECTED:
			LUAT_DEBUG_PRINT("call disconnected");
			break;
		case LUAT_MOBILE_CC_SPEECH_START:
			LUAT_DEBUG_PRINT("now speech type %d", index);
			luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_RECORD_VOICE_START, index + 1, 0, 0, 0);
			break;
		case LUAT_MOBILE_CC_MAKE_CALL_OK:
			LUAT_DEBUG_PRINT("call OK");
			break;
		case LUAT_MOBILE_CC_MAKE_CALL_FAILED:
			LUAT_DEBUG_PRINT("call FAILED, result %d", index);
			break;
		case LUAT_MOBILE_CC_ANSWER_CALL_DONE:
			LUAT_DEBUG_PRINT("answer call result %d", index);
			break;
		case LUAT_MOBILE_CC_HANGUP_CALL_DONE:
			LUAT_DEBUG_PRINT("hangup call result %d", index);
			break;
		case LUAT_MOBILE_CC_LIST_CALL_RESULT:
			break;
		case LUAT_MOBILE_CC_PLAY:
			LUAT_DEBUG_PRINT("play %d", index);
			//如果amr编解码用内部编码，来电时暂停双向对讲测试，会有冲突
			if (index && g_s_speech_not_volte_test && !g_s_speech_test_while_call_in)
			{
				g_s_speech_test_while_call_in = 1;
				luat_audio_inter_amr_deinit();
				LUAT_DEBUG_PRINT("speech test pause");
			}
			if (!index)	//不对本地铃声做控制，只对关闭通话做控制
			{
				luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_PLAY_TONE, index, 0, 0, 0);
			}

		}
		break;
	default:
		LUAT_DEBUG_PRINT("event %d, index %d status %d", event, index, status);
		break;
	}

}

void mobile_voice_data_input(uint8_t *input, uint32_t len, uint32_t sample_rate, uint8_t bits)
{
	luat_rtos_event_send(g_s_task_handle, VOLTE_EVENT_PLAY_VOICE, (uint32_t)input, len, sample_rate, 0);

}

static void test_audio_demo_init(void)
{
	luat_mobile_event_register_handler(mobile_event_cb);
	luat_mobile_speech_init(MULTIMEDIA_ID,mobile_voice_data_input);
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

	// 当前仅EC718p/EC718pv支持这个demo
	#if defined TYPE_EC718P
	luat_rtos_task_create(&g_s_task_handle, 8192, 100, "test", demo_task, NULL, 0);
//	audio_play_set_ram_type(LUAT_HEAP_SRAM);		//打开后消耗RAM较多的地方将使用SRAM，否则使用AUTO模式
	#endif

}

INIT_TASK_EXPORT(test_audio_demo_init, "1");
