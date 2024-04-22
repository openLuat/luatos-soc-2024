/*
 * 如果使用PSRAM做缓存，可以不开低速模式
 * 跑分时不能用PSRAM做缓存，必须开低速模式，否则RAM不够
 */

#include "common_api.h"

#include "luat_base.h"
#include "luat_rtos.h"
#include "luat_mcu.h"
#include "luat_spi.h"
#include "luat_debug.h"
#include "luat_gpio.h"
#include "sfud.h"

#include "lv_conf.h"
#include "lvgl.h"
#include "luat_lcd.h"
#include "luat_mem.h"


//如果不使用低功耗，或者硬件上已经有配了上下拉电阻，可以不使用AONIO
#define SPI_LCD_CS_PIN	HAL_GPIO_35
#define SPI_LCD_RST_PIN HAL_GPIO_36
#define SPI_LCD_BL_PIN HAL_GPIO_26	//不考虑低功耗的话，BL也可以省掉

#define SPI_LCD_W	240
#define SPI_LCD_H	320
#define SPI_LCD_X_OFFSET	0
#define SPI_LCD_Y_OFFSET	0

#define SPI_LCD_RAM_CACHE_MAX	(SPI_LCD_W * SPI_LCD_H)
#define LVGL_FLUSH_TIME	(30)
#define LVGL_FLUSH_BUF_LINE	(20) //buf开到20行大小，也可以自行修改
#define LVGL_FLUSH_WAIT_TIME (5)

#define SFUD_SPI	    SPI_ID0
#define SFUD_SPI_CS	    8

static luat_spi_device_t sfud_spi_dev = {
    .bus_id = SFUD_SPI,
    .spi_config.CPHA = 0,
    .spi_config.CPOL = 0,
    .spi_config.dataw = 8,
    .spi_config.bit_dict = 0,
    .spi_config.master = 1,
    .spi_config.mode = 0,
    .spi_config.bandrate = 25000000,
    .spi_config.cs = SFUD_SPI_CS
};
extern sfud_flash sfud_flash_tables[];

extern void luat_lcd_service_debug(void);

enum
{
	LVGL_FLUSH_EVENT = 1,
};

static luat_lcd_conf_t lcd_conf = {
    .port = LUAT_LCD_HW_ID_0,
    .opts = &lcd_opts_st7789,
	.pin_dc = 0xff,
    .pin_rst = SPI_LCD_RST_PIN,
    .pin_pwr = SPI_LCD_BL_PIN,
    .direction = 0,
    .w = SPI_LCD_W,
    .h = SPI_LCD_H,
    .xoffset = 0,
    .yoffset = 0,
	.interface_mode = LUAT_LCD_IM_4_WIRE_8_BIT_INTERFACE_I,
	.lcd_cs_pin = 0xff
};

typedef struct
{
	lv_disp_draw_buf_t draw_buf_dsc;
	lv_disp_drv_t disp_drv;
	luat_rtos_task_handle h_lvgl_task;
	luat_rtos_timer_t h_lvgl_timer;
	lv_color_t *draw_buf;
	uint8_t is_sleep;
	uint8_t wait_flush;
}lvgl_ctrl_t;

static lvgl_ctrl_t g_s_lvgl;

static LUAT_RT_RET_TYPE lvgl_flush_timer_cb(LUAT_RT_CB_PARAM)
{
	if (g_s_lvgl.wait_flush < 2)
	{
		g_s_lvgl.wait_flush++;
		luat_send_event_to_task(g_s_lvgl.h_lvgl_task, LVGL_FLUSH_EVENT, 0, 0, 0);
	}
}

static void lvgl_flush_cb(struct _lv_disp_drv_t * disp_drv, const lv_area_t * area, lv_color_t * color_p)
{
	uint32_t retry_cnt = 0;
	while ((luat_lcd_service_cache_len() >= SPI_LCD_RAM_CACHE_MAX))
	{
		retry_cnt++;
		luat_rtos_task_sleep(LVGL_FLUSH_WAIT_TIME);
		if (retry_cnt > 20)
		{
			LUAT_DEBUG_PRINT("too much wait");
			luat_lcd_service_debug();
		}
	}
	retry_cnt = 0;
    while (luat_lcd_service_draw(&lcd_conf, area->x1, area->y1, area->x2,  area->y2, color_p, 0))
    {
		retry_cnt++;
		luat_rtos_task_sleep(LVGL_FLUSH_WAIT_TIME);
		if (retry_cnt > 20)
		{
			LUAT_DEBUG_PRINT("too much wait no mem");
			luat_lcd_service_debug();
		}
    }
	lv_disp_flush_ready(disp_drv);
}

static void lvgl_lcd_init(void)
{
    luat_lcd_IF_init(&lcd_conf);
    luat_lcd_init(&lcd_conf);
}

static void lvgl_draw_init(void)
{
	g_s_lvgl.draw_buf = malloc(lcd_conf.w * LVGL_FLUSH_BUF_LINE * sizeof(lv_color_t));
	lv_disp_draw_buf_init(&g_s_lvgl.draw_buf_dsc, g_s_lvgl.draw_buf, NULL, lcd_conf.w * LVGL_FLUSH_BUF_LINE);   /*Initialize the display buffer*/
    lv_disp_drv_init(&g_s_lvgl.disp_drv);                    /*Basic initialization*/

    /*Set up the functions to access to your display*/

    /*Set the resolution of the display*/
    g_s_lvgl.disp_drv.hor_res = lcd_conf.w;
    g_s_lvgl.disp_drv.ver_res = lcd_conf.h;

    /*Used to copy the buffer's content to the display*/
    g_s_lvgl.disp_drv.flush_cb = lvgl_flush_cb;

    /*Set a display buffer*/
    g_s_lvgl.disp_drv.draw_buf = &g_s_lvgl.draw_buf_dsc;

    /*Finally register the driver*/
    lv_disp_drv_register(&g_s_lvgl.disp_drv);
}


const sfud_flash *lv_font_flash;

static void lvgl_task(void *param)
{
	luat_event_t event;
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);

    int re = -1;
    luat_spi_device_setup(&sfud_spi_dev);
    sfud_flash_tables[0].luat_sfud.luat_spi = LUAT_TYPE_SPI_DEVICE;
    sfud_flash_tables[0].luat_sfud.user_data = &sfud_spi_dev;

    if (re = sfud_init()!=0){
        LUAT_DEBUG_PRINT("sfud_init error is %d", re);
        goto error;
    }
    lv_font_flash = sfud_get_device_table();

if(0){//下载 font 到flash，也可自己使用烧录工具下载，这里支持演示，只有第一次下载需要支持烧录
	extern unsigned long hexLength;
	extern unsigned char hexData[];
    if (re = sfud_erase(lv_font_flash, 0, hexLength)!=0){
        LUAT_DEBUG_PRINT("sfud_erase error is %d", re);
    }
    if (re = sfud_write(lv_font_flash, 0, hexLength, hexData)!=0){
        LUAT_DEBUG_PRINT("sfud_write error is %d", re);
    }
	LUAT_DEBUG_PRINT("sfud_write font down");
}
	uint8_t data[8] = {0};
	if (re = sfud_read(lv_font_flash, 0, 8, data)!=0){
        LUAT_DEBUG_PRINT("sfud_read error is %d", re);
    }else{
        LUAT_DEBUG_PRINT("sfud_read 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x 0x%02x\n", data[0], data[1], data[2], data[3], data[4], data[5]);
    }
	
	lvgl_lcd_init();
	lv_init();
	lvgl_draw_init();
	luat_start_rtos_timer(g_s_lvgl.h_lvgl_timer, LVGL_FLUSH_TIME, 1);


	LV_FONT_DECLARE(myFont);
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(label, 150);
	lv_obj_set_style_text_font(label,&myFont,0);
    lv_label_set_text(label, "It is a circularly scrolling text for luatos spi_flash fonts. ");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

	while(1)
	{
		luat_wait_event_from_task(g_s_lvgl.h_lvgl_task, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if (g_s_lvgl.wait_flush) g_s_lvgl.wait_flush--;
		lv_timer_handler();
	}
error:
    while (1)
    {
        luat_rtos_task_sleep(1000);
    }
}

void lvgl_init(void)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_lcd_service_init(60);
	luat_rtos_task_create(&g_s_lvgl.h_lvgl_task, 8192, 90, "lvgl", lvgl_task, NULL, 16);
	g_s_lvgl.h_lvgl_timer = luat_create_rtos_timer(lvgl_flush_timer_cb, NULL, NULL);
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = SPI_LCD_CS_PIN;
	gpio_cfg.output_level = LUAT_GPIO_HIGH;
	luat_gpio_open(&gpio_cfg);
	gpio_cfg.pin = SPI_LCD_RST_PIN;
	luat_gpio_open(&gpio_cfg);
	gpio_cfg.pin = SPI_LCD_BL_PIN;
	luat_gpio_open(&gpio_cfg);
}

INIT_TASK_EXPORT(lvgl_init, "1");

