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

#define I2C_ID            1	

#define FT6336_ADDR 	  0x38     	
  
/********************************FT6336 部分寄存器定义***************************/

#define FT6336_GET_FINGERNUM 	  0x02
#define FT6336_GET_LOC0 	      0x03     	
#define FT6336_GET_LOC1 	      0x09



//如果不使用低功耗，或者硬件上已经有配了上下拉电阻，可以不使用AONIO
#define SPI_LCD_RST_PIN HAL_GPIO_36
#define SPI_LCD_BL_PIN  HAL_GPIO_25	//不考虑低功耗的话，BL也可以省掉

#define TP_RST_PIN  HAL_GPIO_24
#define TP_INT_PIN  HAL_GPIO_22

#define SPI_LCD_W	320
#define SPI_LCD_H	480
#define SPI_LCD_X_OFFSET	0
#define SPI_LCD_Y_OFFSET	0


#define SPI_LCD_RAM_CACHE_MAX	(SPI_LCD_W * SPI_LCD_H)
#define LVGL_FLUSH_TIME	(30)
#define LVGL_FLUSH_BUF_LINE	(100) //buf开到100行大小，也可以自行修改，buf开得越大，演示越流畅，但剩余内存也越小
#define LVGL_FLUSH_WAIT_TIME (5)

enum
{
	LVGL_FLUSH_EVENT = 1,
};

static luat_lcd_conf_t lcd_conf = {
    .port = LUAT_LCD_HW_ID_0,
    .opts = &lcd_opts_nv3037,
	.pin_dc = 0xff,
    .pin_rst = SPI_LCD_RST_PIN,
    .pin_pwr = SPI_LCD_BL_PIN,
    .direction = 0,
    .w = SPI_LCD_W,
    .h = SPI_LCD_H,
    .xoffset = 0,
    .yoffset = 0,
	.interface_mode = LUAT_LCD_IM_2_DATA_LANE,
	.lcd_cs_pin = 0xff
};

typedef struct
{
	lv_disp_draw_buf_t draw_buf_dsc;
	lv_disp_drv_t disp_drv;
    lv_indev_drv_t indev_drv;
	luat_rtos_task_handle h_lvgl_task;
	luat_rtos_timer_t h_lvgl_timer;
	lv_color_t *draw_buf;
	uint8_t is_sleep;
	uint8_t wait_flush;
}lvgl_ctrl_t;

static lvgl_ctrl_t g_s_lvgl;



static bool is_pressed = false;
luat_rtos_task_handle tp_task_handle;
int16_t xy_pos[4] = {0};

void get_tp_data(int16_t* x,int16_t* y)
{
	*x=xy_pos[0];
	*y=xy_pos[1];
	is_pressed = false;
}

static void touchpad_read(lv_indev_drv_t * indev_drv, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;
    if(is_pressed) {
        get_tp_data(&last_x, &last_y);
        data->state = LV_INDEV_STATE_PR;
    }
    else {
        data->state = LV_INDEV_STATE_REL;
    }
    data->point.x = last_x;
    data->point.y = last_y;
}

;
static int tp_int_cb(int pin, void *args)
{
	if (pin == TP_INT_PIN)
	{
		luat_rtos_event_send(tp_task_handle, 0, 0, 0, 0, 0);
	}
}



int ft6336_read(uint8_t regAddr, uint8_t* buf,uint8_t len)
{
	uint8_t tx_buf[2] = {0};
	tx_buf[0] = regAddr; 
	luat_i2c_send(I2C_ID, FT6336_ADDR, tx_buf, 1, 1);
	luat_i2c_recv(I2C_ID, FT6336_ADDR, buf, len);
	return 1;
}


uint8_t ft6336_scan(int16_t *pos)
{
    uint8_t temp[8] = {0}; 
    uint8_t finger_num = 0;
    ft6336_read(FT6336_GET_FINGERNUM, &finger_num, 1);
    if(finger_num)
    {
        ft6336_read(FT6336_GET_LOC0, temp, 4);
        pos[0] = ((uint16_t)(temp[0] & 0x0F) << 8) + temp[1];
        pos[1] = (((uint16_t)(temp[2] & 0x0F) << 8) + temp[3]);
        if(finger_num > 1)
        {
            ft6336_read(FT6336_GET_LOC1, &temp[4], 4);
            pos[2] = ((uint16_t)(temp[4] & 0x0F) << 8) + temp[5];
            pos[3] = (((uint16_t)(temp[6] & 0x0F) << 8) + temp[7]);
        }
    }
	return finger_num;
}



static void tp_task(void *param)
{
	luat_gpio_cfg_t cfg = {0};
	cfg.pin = TP_RST_PIN;
	cfg.mode = LUAT_GPIO_OUTPUT;
	cfg.output_level = 1;
	luat_gpio_open(&cfg);
	cfg.pin = TP_INT_PIN;
	cfg.mode = LUAT_GPIO_IRQ;
	cfg.pull = LUAT_GPIO_PULLUP;
	cfg.irq_type = LUAT_GPIO_FALLING_IRQ;
	cfg.irq_cb = tp_int_cb;
	luat_mcu_iomux_ctrl(LUAT_MCU_PERIPHERAL_I2C, I2C_ID, 23, 2, 0);
    luat_mcu_iomux_ctrl(LUAT_MCU_PERIPHERAL_I2C, I2C_ID, 24, 2, 0);
	luat_gpio_open(&cfg);
	luat_gpio_set(TP_RST_PIN, 0);
	luat_rtos_task_sleep(10);
	luat_gpio_set(TP_RST_PIN, 1);
	luat_i2c_setup(I2C_ID, 1);
	luat_event_t event;
	while(1)
	{
		luat_rtos_event_recv(tp_task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if(ft6336_scan(xy_pos))
		{
			is_pressed = true;
		}
		else
		{
			is_pressed = false;
		}
	}

}

extern void luat_lcd_service_debug(void);



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


    lv_indev_drv_init(&g_s_lvgl.indev_drv);
	g_s_lvgl.indev_drv.type = LV_INDEV_TYPE_POINTER;
	g_s_lvgl.indev_drv.read_cb = touchpad_read;
	lv_indev_drv_register(&g_s_lvgl.indev_drv);
}



static void lvgl_task(void *param)
{
	luat_event_t event;
	lvgl_lcd_init();
	lv_init();
	lvgl_draw_init();
	luat_start_rtos_timer(g_s_lvgl.h_lvgl_timer, LVGL_FLUSH_TIME, 1);
	lv_demo_widgets();
	while(1)
	{
		luat_wait_event_from_task(g_s_lvgl.h_lvgl_task, 0, &event, NULL, LUAT_WAIT_FOREVER);
		if (g_s_lvgl.wait_flush) g_s_lvgl.wait_flush--;
		lv_timer_handler();
	}
}


void lvgl_init(void)
{
	luat_gpio_cfg_t gpio_cfg;
	luat_lcd_service_init(60);
    luat_rtos_task_create(&tp_task_handle, 2048, 20, "tp_task", tp_task, NULL, 16);
	luat_rtos_task_create(&g_s_lvgl.h_lvgl_task, 8192, 90, "lvgl", lvgl_task, NULL, 16);
	g_s_lvgl.h_lvgl_timer = luat_create_rtos_timer(lvgl_flush_timer_cb, NULL, NULL);
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = SPI_LCD_RST_PIN;
	gpio_cfg.mode = LUAT_GPIO_OUTPUT;
	gpio_cfg.output_level = 1;
	luat_gpio_open(&gpio_cfg);
	gpio_cfg.output_level = 0;
	gpio_cfg.pin = SPI_LCD_BL_PIN;
	luat_gpio_open(&gpio_cfg);
}

INIT_TASK_EXPORT(lvgl_init, "1");

