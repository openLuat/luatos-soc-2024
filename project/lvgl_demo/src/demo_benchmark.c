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

#include "lv_conf.h"
#include "lvgl.h"
#include "luat_lcd.h"
#include "luat_mem.h"

unsigned int luat_lv_benchmark_fps;
extern void luat_lcd_service_debug(void);
extern int luat_lcd_service_set_mem_type(uint8_t type);
#define SPI_LCD_FPS		(33)

//如果不使用低功耗，或者硬件上已经有配了上下拉电阻，可以不使用AONIO
#define SPI_LCD_CS_PIN	HAL_GPIO_24
#define SPI_LCD_RST_PIN HAL_GPIO_25
#define SPI_LCD_BL_PIN HAL_GPIO_26	//不考虑低功耗的话，BL也可以省掉

#define SPI_LCD_W	240
#define SPI_LCD_H	320
#define SPI_LCD_X_OFFSET	0
#define SPI_LCD_Y_OFFSET	0

#if LV_USE_DEMO_BENCHMARK
#define SPI_LCD_RAM_CACHE_MAX	(SPI_LCD_W * SPI_LCD_H * 2)
#define LVGL_FLUSH_TIME	(5)
#define LVGL_FLUSH_BUF_LINE	(SPI_LCD_H>>1) //buf开到1/2个屏幕
#define LVGL_FLUSH_WAIT_TIME (3)
#else
#define SPI_LCD_RAM_CACHE_MAX	(SPI_LCD_W * SPI_LCD_H)
#define LVGL_FLUSH_TIME	(30)
#define LVGL_FLUSH_BUF_LINE	(20) //buf开到20行大小，也可以自行修改
#define LVGL_FLUSH_WAIT_TIME (5)
#endif

enum
{
	LVGL_FLUSH_EVENT = 1,
};

static luat_lcd_conf_t lcd_conf = {
    .port = LUAT_LCD_HW_ID_0,
    .opts = &lcd_opts_gc9306x,
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



static void lvgl_task(void *param)
{
	luat_event_t event;
	luat_debug_set_fault_mode(LUAT_DEBUG_FAULT_HANG_RESET);
	lvgl_lcd_init();
	lv_init();
	lvgl_draw_init();
	luat_start_rtos_timer(g_s_lvgl.h_lvgl_timer, LVGL_FLUSH_TIME, 1);
#if LV_USE_DEMO_BENCHMARK
	luat_wdt_close();
	lv_demo_benchmark();
#endif
	while(1)
	{
		luat_wait_event_from_task(g_s_lvgl.h_lvgl_task, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if (g_s_lvgl.wait_flush) g_s_lvgl.wait_flush--;
		lv_timer_handler();
	}
}
#if LV_USE_DEMO_BENCHMARK
static void benchmark_task(void *param)
{
	luat_rtos_task_sleep(105000);
	while(!luat_lv_benchmark_fps)
	{
		luat_rtos_task_sleep(1000);
	}
	LUAT_DEBUG_PRINT("FPS %d", luat_lv_benchmark_fps);
	luat_rtos_task_delete(luat_get_current_task());
}
#endif
void lvgl_init(void)
{
#if LV_USE_DEMO_BENCHMARK
	luat_rtos_task_handle handle;
#endif
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
#if LV_USE_DEMO_BENCHMARK
	luat_lcd_service_set_mem_type(LUAT_HEAP_SRAM);	//跑分要用sram
	luat_rtos_task_create(&handle, 2048, 10, "test", benchmark_task, NULL, 16);
#endif

}

INIT_TASK_EXPORT(lvgl_init, "1");

