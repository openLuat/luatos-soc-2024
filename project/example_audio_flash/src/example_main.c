#include "common_api.h"
#include "luat_rtos.h"
#include "luat_audio_play.h"
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
#include "sfud.h"
#include "soc_spi.h"
#include "driver_gpio.h"
// 这里定义TTS数据在Flash上的起始地址
// 因为很多demo都喜欢用0开始演示flash读写
// 导致TTS的数据无意中就破坏掉了

#define FLASH_TTS_ADDR (64 * 1024)

#define FLASH_SPI_ID SPI_ID0
#define FLASH_SPI_CS HAL_GPIO_8
#define FALSH_SPI_BR (51200000)

#define PA_PWR_PIN HAL_GPIO_25
#define PA_PWR_PIN_ALT_FUN	0
#define CODEC_PWR_PIN HAL_GPIO_16
#define CODEC_PWR_PIN_ALT_FUN	4

// FLASH使能引脚
#define FLASH_VCC_PIN HAL_GPIO_26
#define FLASH_VCC_PIN_ALT_FUN   0

extern sfud_flash sfud_flash_tables[];

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

static ivBool tts_read_data(
		  ivPointer		pParameter,			/* [in] user callback parameter */
		  ivPointer		pBuffer,			/* [out] read resource buffer */
		  ivResAddress	iPos,				/* [in] read start position */
ivResSize		nSize )			/* [in] read size */
{
	iPos += FLASH_TTS_ADDR;
	GPIO_FastOutput(FLASH_SPI_CS, 0);
	char cmd[4] = {0x03, iPos >> 16, (iPos >> 8) & 0xFF, iPos & 0xFF};
	SPI_FastTransfer(FLASH_SPI_ID, cmd, cmd, 4);
	if (nSize >= 4096) {
		SPI_BlockTransfer(FLASH_SPI_ID, pBuffer, pBuffer, nSize);
	}
	else {
		SPI_FastTransfer(FLASH_SPI_ID, pBuffer, pBuffer, nSize);
	}
	GPIO_FastOutput(FLASH_SPI_CS, 1);
	return ivTrue;
}

static HANDLE g_s_delay_timer;

void app_pa_on(LUAT_RT_CB_PARAM)
{
	luat_gpio_set(PA_PWR_PIN, 1);
}

void codec_ctrl(uint8_t onoff)
{
	if (1 == onoff)
	    luat_gpio_set(CODEC_PWR_PIN, 1);
	else
	    luat_gpio_set(CODEC_PWR_PIN, 0);
}

void audio_event_cb(uint32_t event, void *param)
{
	LUAT_DEBUG_PRINT("%d", event);
	switch(event)
	{
	case LUAT_MULTIMEDIA_CB_AUDIO_DECODE_START:
		codec_ctrl(1);
		luat_audio_play_write_blank_raw(0, 6, 1);
		break;
	case LUAT_MULTIMEDIA_CB_AUDIO_OUTPUT_START:
		luat_rtos_timer_start(g_s_delay_timer, 200, 0, app_pa_on, NULL);
		break;
	case LUAT_MULTIMEDIA_CB_TTS_INIT:
		break;
	case LUAT_MULTIMEDIA_CB_TTS_DONE:
		if (!luat_audio_play_get_last_error(0))
		{
			luat_audio_play_write_blank_raw(0, 1, 0);
		}
		break;
	case LUAT_MULTIMEDIA_CB_AUDIO_DONE:
		luat_rtos_timer_stop(g_s_delay_timer);
		LUAT_DEBUG_PRINT("audio play done, result=%d!", luat_audio_play_get_last_error(0));
        luat_gpio_set(PA_PWR_PIN, 0);
		codec_ctrl(0);
		break;
	}
}

void audio_data_cb(uint8_t *data, uint32_t len, uint8_t bits, uint8_t channels)
{
	//这里可以对音频数据进行软件音量缩放，或者直接清空来静音
	//软件音量缩放参考HAL_I2sSrcAdjustVolumn
	//LUAT_DEBUG_PRINT("%x,%d,%d,%d", data, len, bits, channels);
}


static void play_channel_config(void)
{
	luat_i2s_conf_t conf = {0};
	conf.id = 0;
	// TM8211
	conf.standard = LUAT_I2S_MODE_MSB;
	conf.channel_format = LUAT_I2S_CHANNEL_STEREO;
	conf.channel_bits = LUAT_I2S_BITS_16;
	conf.data_bits = LUAT_I2S_BITS_16;
	luat_i2s_setup(&conf);
}


static void demo_task(void *arg)
{
	int re = -1;
    uint8_t data[8] = {0};
	luat_spi_device_setup(&sfud_spi_dev);
    sfud_flash_tables[0].luat_sfud.luat_spi = LUAT_TYPE_SPI_DEVICE;
    sfud_flash_tables[0].luat_sfud.user_data = &sfud_spi_dev;

    if (re = sfud_init()!=0){
        LUAT_DEBUG_PRINT("sfud_init error is %d\n", re);
    }
    const sfud_flash *flash = sfud_get_device_table();
    // 第一次刷数据到spi flash才需要开启
#if 1
    if (re = sfud_erase(flash, FLASH_TTS_ADDR, 719278)!=0){
        LUAT_DEBUG_PRINT("sfud_erase error is %d\n", re);
    }
    if (re = sfud_write(flash, FLASH_TTS_ADDR, 719278, ivtts_16k)!=0){
        LUAT_DEBUG_PRINT("sfud_write error is %d\n", re);
    }
	LUAT_DEBUG_PRINT("sfud_write ivtts_16k down\n");
	if (re = sfud_read(flash, FLASH_TTS_ADDR, 8, data)!=0){
        LUAT_DEBUG_PRINT("sfud_read error is %d\n", re);
    }else{
        LUAT_DEBUG_PRINT("sfud_read 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    }
#else
	 //校验数据
	 uint8_t *tmp = malloc(4096);
	 for (size_t i = 0; i < 719278; i+=4096)
	 {
	 	 sfud_read(flash, i+FLASH_TTS_ADDR, 4096, tmp);
	 	 if (memcmp(tmp, ivtts_16k + i, 4096)) {
	 	 	LUAT_DEBUG_PRINT("flash NOT match, %x", i);
	 	 	break;
	 	 }
	 }
#endif
    play_channel_config();
	ivCStrA sdk_id = AISOUND_SDK_USERID_16K;
	char tts_string[] = "支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC,支付宝到账123.45元,微信收款9876.12元ABC";
	luat_audio_play_info_t info[5];

	luat_rtos_timer_create(&g_s_delay_timer);
    luat_audio_play_global_init(audio_event_cb, audio_data_cb, NULL, luat_audio_play_tts_default_fun, NULL);
	// 外部flash版本
	luat_audio_play_tts_set_resource(NULL, (void*)sdk_id, tts_read_data);
    while(1)
    {
    	luat_audio_play_tts_text(0, tts_string, sizeof(tts_string));
    	luat_rtos_task_sleep(35000);
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
    gpio_cfg.pin = FLASH_VCC_PIN;
    gpio_cfg.alt_fun = FLASH_VCC_PIN_ALT_FUN;
    luat_gpio_open(&gpio_cfg);
    luat_gpio_set(FLASH_VCC_PIN, 1);

	luat_rtos_task_handle task_handle;
	luat_rtos_task_create(&task_handle, 2048, 20, "test", demo_task, NULL, 0);
}

INIT_TASK_EXPORT(test_audio_demo_init, "1");
