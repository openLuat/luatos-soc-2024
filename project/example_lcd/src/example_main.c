/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"

#include "luat_spi.h"
#include "luat_lcd.h"

#define LCD_SPI	    SPI_ID0

#define LCD_SPI_CS	12
#define LCD_DC	    14
#define LCD_RST	    3
#define LCD_PWR	    255

#define LCD_H   320
#define LCD_W   240

/**
 * 使用硬件LCD接口打开下面的注释
 */
//#define LCD_USE_HW_IF

#ifdef LCD_USE_HW_IF
static luat_lcd_conf_t lcd_conf = {
    .port = LUAT_LCD_HW_ID_0,
    .lcd_spi_device = NULL,
    .auto_flush = 1,
    .opts = &lcd_opts_gc9306x,
    .pin_dc = 0xff,
    .pin_rst = LCD_RST,
    .pin_pwr = LCD_PWR,
    .direction = 0,
    .w = LCD_W,
    .h = LCD_H,
    .xoffset = 0,
    .yoffset = 0,
    .interface_mode = LUAT_LCD_IM_4_WIRE_8_BIT_INTERFACE_I,
    .lcd_cs_pin = 0xff,	//注意不用的时候写0xff
};
#else
static luat_spi_device_t lcd_spi_dev = {
    .bus_id = LCD_SPI,
    .spi_config.CPHA = 0,
    .spi_config.CPOL = 0,
    .spi_config.dataw = 8,
    .spi_config.bit_dict = 0,
    .spi_config.master = 1,
    .spi_config.mode = 0,
    .spi_config.bandrate = 51000000,
    .spi_config.cs = LCD_SPI_CS
};

static luat_lcd_conf_t lcd_conf = {
    .port = LUAT_LCD_SPI_DEVICE,
    .lcd_spi_device = &lcd_spi_dev,
    .auto_flush = 1,
    .opts = &lcd_opts_st7789,
    .pin_dc = LCD_DC,
    .pin_rst = LCD_RST,
    .pin_pwr = LCD_PWR,
    .direction = 0,
    .w = LCD_W,
    .h = LCD_H,
    .xoffset = 0,
    .yoffset = 0
};
#endif
luat_rtos_task_handle lcd_task_handle;

static void task_test_lcd(void *param)
{
#ifdef LCD_USE_HW_IF
	luat_lcd_IF_init(&lcd_conf);
#else
    luat_spi_device_setup(&lcd_spi_dev);
#endif
    luat_lcd_init(&lcd_conf);
    luat_lcd_clear(&lcd_conf,LCD_WHITE);

    luat_lcd_draw_line(&lcd_conf,20,35,140,35,0x001F);
    luat_lcd_draw_rectangle(&lcd_conf,20,40,120,70,0xF800);
    luat_lcd_draw_circle(&lcd_conf,60,60,10,0x0CE0);

    while (1){
        luat_rtos_task_sleep(1000);
    }
}

static void task_demo_lcd(void)
{
    luat_rtos_task_create(&lcd_task_handle, 4096, 20, "lcd", task_test_lcd, NULL, 0);
}

INIT_TASK_EXPORT(task_demo_lcd,"1");



