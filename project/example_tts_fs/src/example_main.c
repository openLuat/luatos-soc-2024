#include "common_api.h"
#include "luat_rtos.h"
#include "luat_audio_play.h"
#include "luat_audio_codec.h"
#include "luat_i2s.h"
#include "ivTTSSDKID_all.h"
#include "ivTTS.h"
#include "luat_gpio.h"
#include "luat_fs.h"
#include "luat_mem.h"
#include "luat_i2c.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_spi.h"
#include "luat_mcu.h"
#include "luat_wdt.h"
// #include "sfud.h"
#include "lfs.h"

#include "little_flash.h"

#define FLASH_SPI_ID SPI_ID0
#define FLASH_SPI_CS HAL_GPIO_8
#define FALSH_SPI_BR (51200000)

#ifndef CHIP_EC716
#define PA_PWR_PIN HAL_GPIO_25
#define PA_PWR_PIN_ALT_FUN	0
#else
#define PA_PWR_PIN HAL_GPIO_20
#define PA_PWR_PIN_ALT_FUN	0
#endif
#define CODEC_PWR_PIN HAL_GPIO_16
#define CODEC_PWR_PIN_ALT_FUN	4

// FLASH使能引脚

enum
{
	AUDIO_EVENT_PLAY_DONE = 1,
};

// extern sfud_flash sfud_flash_tables[];
// extern lfs_t* flash_lfs_sfud(sfud_flash* flash, size_t offset, size_t maxsize);

static luat_spi_device_t sfud_spi_dev = {
    .bus_id = FLASH_SPI_ID,
    .spi_config.CPHA = 0,
    .spi_config.CPOL = 0,
    .spi_config.dataw = 8,
    .spi_config.bit_dict = 0,
    .spi_config.master = 1,
    .spi_config.mode = 0,
    .spi_config.bandrate = FALSH_SPI_BR,
    .spi_config.cs = FLASH_SPI_CS
};
static luat_rtos_task_handle g_s_task_handle;

#define PA_ON_LEVEL 1
#define PWR_ON_LEVEL 1

#define LOW_POWER_TEST	//开启低功耗场景测试
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
static int i2s_cb(uint8_t id ,luat_i2s_event_t event, uint8_t *rx_data, uint32_t rx_len, void *param) {return 0;}
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
	.luat_i2s_event_callback = i2s_cb,
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
	.luat_i2s_event_callback = i2s_cb,
};

static const luat_audio_codec_conf_t luat_audio_codec_tm8211 = {
    .i2s_id = TEST_I2S_ID,
    .codec_opts = &codec_opts_tm8211,
};

static const luat_i2s_conf_t *i2s_conf = &luat_i2s_conf_tm8211;
static const luat_audio_codec_conf_t *codec_conf = &luat_audio_codec_tm8211;
#endif

static ivBool tts_read_data(
		  ivPointer		pParameter,			/* [in] user callback parameter */
		  ivPointer		pBuffer,			/* [out] read resource buffer */
		  ivResAddress	iPos,				/* [in] read start position */
ivResSize		nSize )			/* [in] read size */
{
	//DBG("%x", pParameter);
	int ret;
	ret = luat_fs_fseek(pParameter, iPos, SEEK_SET);
	if (ret < 0) return ivFalse;
	ret = luat_fs_fread(pBuffer, nSize, 1, pParameter);
	if (ret < 0) return ivFalse;
	return ivTrue;
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

static void audio_data_cb(uint8_t *data, uint32_t len, uint8_t bits, uint8_t channels)
{
	//这里可以对音频数据进行软件音量缩放，或者直接清空来静音
	//软件音量缩放参考HAL_I2sSrcAdjustVolumn
	//LUAT_DEBUG_PRINT("%x,%d,%d,%d", data, len, bits, channels);
}

static void print_fs_info(const char* dir_path)
{
    luat_fs_info_t fs_info = {0};

    luat_fs_info(dir_path, &fs_info);
    // 打印文件系统空间信息
    LUAT_DEBUG_PRINT("fs_info %s %d %d %d %d",
        fs_info.filesystem,
        fs_info.type,
        fs_info.total_block,
        fs_info.block_used,
        fs_info.block_size);
}

extern lfs_t* flash_lfs_lf(little_flash_t* flash, size_t offset, size_t maxsize);

static void demo_task(void *arg)
{
	static FILE* fp;
	uint8_t *temp;
	uint32_t write_len;
	uint32_t dummy_len;
	size_t total, alloc, peak;
	luat_event_t event;
	int re = -1;
    uint8_t data[8] = {0};
    luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG);
	luat_spi_device_setup(&sfud_spi_dev);

	/* SFUD */
    // sfud_flash_tables[0].luat_sfud.luat_spi = LUAT_TYPE_SPI_DEVICE;
    // sfud_flash_tables[0].luat_sfud.user_data = &sfud_spi_dev;

    // if (re = sfud_init()!=0){
    //     LUAT_DEBUG_PRINT("sfud_init error is %d\n", re);
    //     goto error;
    // }
    // const sfud_flash *flash = sfud_get_device_table();

	/* little flash */
	static little_flash_t lf_flash = {0};
    luat_spi_device_setup(&sfud_spi_dev);
    lf_flash.spi.user_data = &sfud_spi_dev;
    little_flash_init();
    little_flash_device_init(&lf_flash);

    luat_fs_init();

    // lfs_t* lfs = flash_lfs_sfud((sfud_flash *)flash, 0, 0);
	lfs_t* lfs = flash_lfs_lf(&lf_flash, 0, 0);

    if (lfs) {
	    luat_fs_conf_t conf = {
		    .busname = (char*)lfs,
		    .type = "lfs2",
		    .filesystem = "lfs2",
		    .mount_point = "/sfud",
	    };
	    int ret = luat_fs_mount(&conf);
        LUAT_DEBUG_PRINT("vfs mount %s ret %d", "/sfud", ret);
    }
    else {
        LUAT_DEBUG_PRINT("flash_lfs_sfud error");
        goto error;
    }
    print_fs_info("/sfud");
    // tts资源文件名称不限定，这里用tts.bin
    fp = luat_fs_fopen("/sfud/tts.bin", "r");
    if (!fp || luat_fs_fsize("/sfud/tts.bin") != 719278)
    {
    	LUAT_DEBUG_PRINT("init tts.bin");
    	fp = luat_fs_fopen("/sfud/tts.bin", "wb+");
    	temp = malloc(65536);
    	write_len = 0;
    	while(write_len < 719278)
    	{
    		dummy_len = 719278 - write_len;
    		if (dummy_len > 65536) dummy_len = 65536;
    		memcpy(temp, ivtts_16k + write_len, dummy_len);
    		luat_fs_fwrite(temp, dummy_len, 1, fp);
    		write_len += dummy_len;
    		luat_wdt_feed();
    	}
    	free(temp);
    	luat_fs_fclose(fp);
    	fp = luat_fs_fopen("/sfud/tts.bin", "r");
    	if (!fp || luat_fs_fsize("/sfud/tts.bin") != 719278)
    	{
    		LUAT_DEBUG_PRINT("init tts.bin failed");
    		goto error;
    	}
    }

	ivCStrA sdk_id = AISOUND_SDK_USERID_16K;
    luat_audio_play_global_init(audio_event_cb, audio_data_cb, NULL, luat_audio_play_tts_default_fun, NULL);
	luat_audio_play_tts_set_resource(fp, (void*)sdk_id, tts_read_data);

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

	char tts_string[] = "支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC";

    while(1)
    {
		luat_audio_play_tts_text(MULTIMEDIA_ID, tts_string, sizeof(tts_string));
		luat_rtos_event_recv(g_s_task_handle, AUDIO_EVENT_PLAY_DONE, &event, NULL, LUAT_WAIT_FOREVER);
		luat_meminfo_opt_sys(LUAT_HEAP_PSRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("psram total %u, used %u, max used %u", total, alloc, peak);
		luat_meminfo_opt_sys(LUAT_HEAP_SRAM, &total, &alloc, &peak);
		LUAT_DEBUG_PRINT("sram total %u, used %u, max used %u", total, alloc, peak);
		luat_rtos_task_sleep(2000);
    }
error:
	while (1)
	{
		luat_rtos_task_sleep(1000);
	}
}

static void test_audio_demo_init(void)
{
    luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
    gpio_cfg.mode = LUAT_GPIO_OUTPUT;
	// pa power ctrl init
	gpio_cfg.pin = PA_PWR_PIN;
	gpio_cfg.alt_fun = PA_PWR_PIN_ALT_FUN;
	luat_gpio_open(&gpio_cfg);

	// codec power ctrl init
	gpio_cfg.pin = CODEC_PWR_PIN;
	gpio_cfg.alt_fun = CODEC_PWR_PIN_ALT_FUN;
	luat_gpio_open(&gpio_cfg);

    // flash power init
//    gpio_cfg.pin = FLASH_VCC_PIN;
//    gpio_cfg.alt_fun = FLASH_VCC_PIN_ALT_FUN;
//    luat_gpio_open(&gpio_cfg);
//    luat_gpio_set(FLASH_VCC_PIN, 1);

	luat_rtos_task_create(&g_s_task_handle, 4096, 20, "test", demo_task, NULL, 0);
}

INIT_TASK_EXPORT(test_audio_demo_init, "1");
