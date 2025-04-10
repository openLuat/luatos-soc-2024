#include "luat_base.h"
#include "luat_lcd.h"
#include "luat_rtos.h"
#include "luat_camera.h"
#include "common_api.h"
#include "driver_usp.h"
#include "driver_gpio.h"
#include "cmsis_gcc.h"
#include "luat_mem.h"
typedef struct
{
	uint64_t wait_bytes;
	uint64_t done_bytes;
	luat_lcd_qspi_conf_t qspi_param;
	HANDLE task_handle;
	HANDLE camera_show_stop_sem;
	PV_Union auto_flush_ram;
	uint32_t auto_flush_ram_size;
	uint8_t mem_type;
	uint8_t direction;
	uint8_t qspi_auto_flush_inited;
	uint8_t need_record_frame_cnt;
}lcd_service_t;

static lcd_service_t g_s_lcd;

typedef struct
{
	luat_lcd_conf_t* conf;
	void *static_buf;
	int16_t x1;
	int16_t y1;
	int16_t x2;
	int16_t y2;
	uint32_t size;
	uint32_t data[0];
}lcd_service_draw_t;

enum
{
	SERVICE_LCD_DRAW = SERVICE_EVENT_ID_START + 1,
	SERVICE_LCD_INIT,
	SERVICE_LCD_SHOW_CAMERA,
	SERVICE_RUN_USER_API,
};

//#define __SWAP_RB(value) ((value & 0x07e0) | (value >> 11) | (value << 11))
#define __SWAP_RB(value) (value)

extern int LSPI_StopCameraPreview(uint8_t ID);
extern int LSPI_StartCameraPreview(uint8_t ID, uint16_t CameraW, uint16_t CameraH, uint16_t ImageW, uint16_t ImageH, uint16_t CutTopLine, uint16_t CutBottomLine, uint16_t CutLeftLine, uint16_t CutRightLine, uint16_t ScaleW, uint16_t ScaleH);

static void prvLCD_Task(void* params)
{
	OS_EVENT event;
	uint32_t size;
	luat_lcd_api callback;
	lcd_service_draw_t *draw;
	luat_spi_camera_t *camera;
	luat_lcd_conf_t* lcd;
	camera_cut_info_t *cut;
	uint8_t *data;
	uint16_t x1,y1,w,h;
	uint8_t is_static_buf,dir;
	while(1)
	{
		get_event_from_task(g_s_lcd.task_handle, CORE_EVENT_ID_ANY, &event, NULL, 0xffffffff);
		switch(event.ID)
		{
		case SERVICE_LCD_DRAW:

			draw = (lcd_service_draw_t *)event.Param1;

			if (draw->static_buf)
			{
				data = (uint8_t *)draw->static_buf;
				is_static_buf = 1;
			}
			else
			{
				data = (uint8_t *)draw->data;
				is_static_buf = 0;
			}

			if (draw->conf->port != LUAT_LCD_HW_ID_0)
			{
				luat_lcd_set_address(draw->conf, draw->x1, draw->y1, draw->x2, draw->y2);
				uint32_t size_l = (draw->x2 - draw->x1 + 1) * (draw->y2 - draw->y1 + 1) * sizeof(luat_color_t);
				if(draw->conf->port == LUAT_LCD_SPI_DEVICE)
				{
					luat_spi_device_send((luat_spi_device_t*)(draw->conf->lcd_spi_device), data, size_l);
				}
				else
				{
					luat_spi_send(draw->conf->port, data, size_l);
				}
			}
			else
			{
				luat_lcd_IF_draw(draw->conf, draw->x1, draw->y1, draw->x2, draw->y2, data);
			}
			size = draw->size;
			luat_heap_free(draw);
			if (!is_static_buf)
			{
				g_s_lcd.done_bytes += size;
				if (g_s_lcd.done_bytes > g_s_lcd.wait_bytes)
				{
					g_s_lcd.done_bytes = g_s_lcd.wait_bytes;
				}
			}
			break;
		case SERVICE_LCD_SHOW_CAMERA:

			camera = (luat_spi_camera_t *)event.Param1;
			lcd = (luat_lcd_conf_t *)camera->lcd_conf;
			cut = (camera_cut_info_t *)event.Param2;
			if (lcd->opts->no_ram_mode)
			{
				if (cut)
				{
					LSPI_StartCameraPreview(USP_ID2, camera->sensor_width, camera->sensor_height,
							lcd->w, lcd->h, cut->top_cut_lines, cut->bottom_cut_lines, cut->left_cut_lines, cut->right_cut_lines,
							cut->w_scale, cut->h_scale);
				}
				else
				{
					LSPI_StartCameraPreview(USP_ID2, camera->sensor_width, camera->sensor_height,
							lcd->w, lcd->h, 0, 0, 0, 0, 0, 0);
				}
			}
			else
			{
				x1 = event.Param3;
				y1 = event.Param3 >> 16;
				if (lcd->opts->rb_swap)
				{
					dir = g_s_lcd.direction;
				}
				else
				{
					dir = g_s_lcd.direction|0x08;
				}
				GPIO_Output(lcd->lcd_cs_pin, 0);
				LSPI_WriteCmd(USP_ID2, 0x36, &dir, 1);
				GPIO_Output(lcd->lcd_cs_pin, 1);
				if (cut)
				{
					w = (camera->sensor_width / (cut->w_scale +1)) - (cut->left_cut_lines + cut->right_cut_lines);
					h = (camera->sensor_height / (cut->h_scale +1)) - (cut->top_cut_lines + cut->bottom_cut_lines);
					luat_lcd_set_address(lcd, x1, y1, x1+w-1, y1+h-1);
					LSPI_StartCameraPreview(USP_ID2, camera->sensor_width, camera->sensor_height,
							w, h, cut->top_cut_lines, cut->bottom_cut_lines, cut->left_cut_lines, cut->right_cut_lines,
							cut->w_scale, cut->h_scale);
				}
				else
				{
					luat_lcd_set_address(lcd, x1, y1, x1+lcd->w-1, y1+lcd->h-1);
					LSPI_StartCameraPreview(USP_ID2, camera->sensor_width, camera->sensor_height,
							lcd->w, lcd->h, 0, 0, 0, 0, 0, 0);
				}
			}
			OS_MutexLock(g_s_lcd.camera_show_stop_sem);
			LSPI_StopCameraPreview(USP_ID2);
			DBG("camera show stop!");
			if (lcd->opts->no_ram_mode)
			{

			}
			else
			{
				if (lcd->opts->rb_swap)
				{
					dir = g_s_lcd.direction|0x08;
				}
				else
				{
					dir = g_s_lcd.direction;
				}
				GPIO_Output(lcd->lcd_cs_pin, 0);
				LSPI_WriteCmd(USP_ID2, 0x36, &dir, 1);
				GPIO_Output(lcd->lcd_cs_pin, 1);
			}


			break;
		case SERVICE_LCD_INIT:
			luat_lcd_init((luat_lcd_conf_t *)event.Param1);
			break;
		case SERVICE_RUN_USER_API:
			callback = (luat_lcd_api)event.Param1;
			callback((void *)event.Param2, event.Param3);
			break;

		}
	}
}



void luat_lcd_service_init(uint32_t pro)
{
	if (!g_s_lcd.task_handle)
	{
		g_s_lcd.mem_type = LUAT_HEAP_AUTO;
		g_s_lcd.task_handle = create_event_task(prvLCD_Task, NULL, 4096, pro, 0, "lcdSer");
		g_s_lcd.camera_show_stop_sem = OS_MutexCreate();
	}
}



int luat_lcd_service_draw(luat_lcd_conf_t* conf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, luat_color_t *data, uint8_t is_static_buf)
{
	lcd_service_draw_t *draw;
	uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1) * sizeof(luat_color_t);
	if (is_static_buf)
	{
		draw = luat_heap_opt_malloc(g_s_lcd.mem_type, sizeof(lcd_service_draw_t));
		if (!draw) return -ERROR_NO_MEMORY;
		draw->static_buf = data;
	}
	else
	{
		draw = luat_heap_opt_malloc(g_s_lcd.mem_type, sizeof(lcd_service_draw_t) + size);
		if (!draw) return -ERROR_NO_MEMORY;
		memcpy(draw->data, data, size);
		draw->static_buf = 0;
		g_s_lcd.wait_bytes += size;
	}
	draw->conf = conf;
	draw->x1 = x1;
	draw->y1 = y1;
	draw->x2 = x2;
	draw->y2 = y2;
	draw->size = size;
	send_event_to_task(g_s_lcd.task_handle, NULL, SERVICE_LCD_DRAW, (uint32_t)draw, 0, 0, 0);

	return ERROR_NONE;
}

int luat_lcd_service_set_mem_type(uint8_t type)
{
	if (type < LUAT_HEAP_AUTO || type > LUAT_HEAP_PSRAM)
	{
		return -1;
	}
	g_s_lcd.mem_type = type;
	return 0;
}

uint32_t luat_lcd_service_cache_len(void)
{
	return (uint32_t)(g_s_lcd.wait_bytes - g_s_lcd.done_bytes);
}

void luat_lcd_service_debug(void)
{
	DBG("%llu, %llu", g_s_lcd.wait_bytes , g_s_lcd.done_bytes);
}

void luat_lcd_IF_init(luat_lcd_conf_t* conf)
{
#ifdef TYPE_EC718M
	switch(conf->interface_mode)
	{
	case LUAT_LCD_IM_QSPI_MODE:
		GPIO_IomuxEC7XX(40, 4, 0, 0);
		GPIO_IomuxEC7XX(41, 4, 0, 0);
		GPIO_IomuxEC7XX(43, 4, 0, 0);
		GPIO_IomuxEC7XX(44, 4, 0, 0);
		GPIO_IomuxEC7XX(16, 1, 0, 0);
		GPIO_IomuxEC7XX(17, 1, 0, 0);
		GPIO_IomuxEC7XX(13, 2, 1, 0);
		GPIO_IomuxEC7XX(14, 2, 1, 0);
		break;
	case LUAT_LCD_IM_8080_MODE:
		break;

	default:
		GPIO_IomuxEC7XX(40, 1, 0, 0);
		if (conf->lcd_cs_pin != 0xff)
		{

		}
		else
		{
			GPIO_IomuxEC7XX(41, 1, 0, 0);
		}
		GPIO_IomuxEC7XX(43, 1, 0, 0);
		GPIO_IomuxEC7XX(44, 2, 0, 0);
		break;
	}
	if (!conf->bus_speed)
	{
		conf->bus_speed = 51000000;
	}
	if (conf->bus_speed > 80216064)
	{
		conf->bus_speed = 80000000;
	}
	if (!conf->flush_rate)
	{
		conf->flush_rate = 630;
	}
	LSPI_Setup(USP_ID2, conf->bus_speed, conf->interface_mode, NULL, 0);
#else
	GPIO_IomuxEC7XX(40, 1, 0, 0);
	if (conf->lcd_cs_pin != 0xff)
	{

	}
	else
	{
		GPIO_IomuxEC7XX(41, 1, 0, 0);
	}
	GPIO_IomuxEC7XX(43, 1, 0, 0);
	GPIO_IomuxEC7XX(44, 2, 0, 0);
	LSPI_Setup(USP_ID2, 0, conf->interface_mode, NULL, 0);
#endif
	conf->opts->write_cmd_data = luat_lcd_IF_write_cmd_data;
	conf->opts->read_cmd_data = luat_lcd_IF_read_cmd_data;
	conf->opts->lcd_draw = luat_lcd_IF_draw;
	conf->opts->sleep_ctrl = luat_lcd_IF_sleep;
}
int luat_lcd_IF_write_cmd_data(luat_lcd_conf_t* conf,const uint8_t cmd, const uint8_t *data, uint8_t data_len)
{
	if (cmd == 0x36)
	{
		g_s_lcd.direction = data[0] & 0xf7;
//		DBG("dir %x %d", g_s_lcd.direction, conf->opts->rb_swap);
		uint8_t dir;
		if (conf->opts->rb_swap)
		{
			dir = g_s_lcd.direction | 0x08;
		}
		else
		{
			dir = g_s_lcd.direction;
		}
		GPIO_Output(conf->lcd_cs_pin, 0);
#ifdef TYPE_EC718M
		if (LUAT_LCD_IM_QSPI_MODE == conf->interface_mode)
		{
			LSPI_SetQspiLaneConfig(USP_ID2, 0, 0, g_s_lcd.qspi_param.write_1line_cmd);
		}
#endif
		int res = LSPI_WriteCmd(USP_ID2, cmd, &dir, 1);
		GPIO_Output(conf->lcd_cs_pin, 1);
		return res;
	}
	GPIO_Output(conf->lcd_cs_pin, 0);
#ifdef TYPE_EC718M
	if (LUAT_LCD_IM_QSPI_MODE == conf->interface_mode)
	{
		LSPI_SetQspiLaneConfig(USP_ID2, 0, 0, g_s_lcd.qspi_param.write_1line_cmd);
	}
#endif
	int res = LSPI_WriteCmd(USP_ID2, cmd, data, data_len);
	GPIO_Output(conf->lcd_cs_pin, 1);
	return res;
}

int luat_lcd_IF_read_cmd_data(luat_lcd_conf_t* conf,const uint8_t cmd, uint8_t *data, uint8_t data_len, uint8_t dummy_bit)
{
#ifdef TYPE_EC718M
	GPIO_Output(conf->lcd_cs_pin, 0);
	int res = LSPI_ReadDataV2(USP_ID2, cmd, data, data_len, dummy_bit);
	GPIO_Output(conf->lcd_cs_pin, 1);
	return res;
#else
	return -1;
#endif
}

int luat_lcd_IF_draw(luat_lcd_conf_t* conf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, luat_color_t* color)
{
	if (conf->opts->no_ram_mode)
	{
		memcpy(g_s_lcd.auto_flush_ram.p, color, g_s_lcd.auto_flush_ram_size);
		return 0;
	}
	uint32_t size,w,h,points,i;
	PV_Union dummy;
	uint32_t temp[16];
	int res;
    uint8_t data_x[] = {(x1+conf->xoffset)>>8,x1+conf->xoffset,(x2+conf->xoffset)>>8,x2+conf->xoffset};
    luat_lcd_IF_write_cmd_data(conf,0x2a, data_x, 4);
    uint8_t data_y[] = {(y1+conf->yoffset)>>8,y1+conf->yoffset,(y2+conf->yoffset)>>8,y2+conf->yoffset};
    luat_lcd_IF_write_cmd_data(conf,0x2b, data_y, 4);
	w = x2 - x1 + 1;
	h = y2 - y1 + 1;
	size = w * h * sizeof(luat_color_t);
	points = w * h;
	if ((uint32_t)color & 0x03)
	{
		if (size <= sizeof(temp))
		{
			dummy.pu32 = temp;
			for(i = 0; i < points; i++)
			{
				dummy.pu16[i] = __SWAP_RB(color[i]);
			}
			GPIO_Output(conf->lcd_cs_pin, 0);
			LSPI_WriteImageData(USP_ID2, w, h, (uint32_t)temp, size, 1);
			GPIO_Output(conf->lcd_cs_pin, 1);

		}
		else
		{
			dummy.pu8 = (uint8_t *)luat_heap_malloc(size + 4);
			for(i = 0; i < points; i++)
			{
				dummy.pu16[i] = __SWAP_RB(color[i]);
			}
			GPIO_Output(conf->lcd_cs_pin, 0);
			LSPI_WriteImageData(USP_ID2, w, h, dummy.u32, size, 1);
			GPIO_Output(conf->lcd_cs_pin, 1);
			luat_heap_free(dummy.p);
		}
	}
	else
	{
		GPIO_Output(conf->lcd_cs_pin, 0);
		LSPI_WriteImageData(USP_ID2, w, h, (uint32_t)color, size, 1);
		GPIO_Output(conf->lcd_cs_pin, 1);
	}

	return 0;
}

int luat_lcd_IF_sleep(luat_lcd_conf_t* conf, uint8_t on_off)
{
	if (on_off)
	{
		if (conf->opts->sleep_cmd && conf->opts->sleep_cmd != 0xff)
		{
			lcd_write_cmd_data(conf,conf->opts->sleep_cmd, NULL, 0);
		}
		else if (!conf->opts->sleep_cmd)
		{
			lcd_write_cmd_data(conf,LUAT_LCD_DEFAULT_SLEEP, NULL, 0);
		}
		if (LUAT_LCD_IM_QSPI_MODE == conf->interface_mode)
		{
			luat_lcd_qspi_auto_flush_on_off(conf, 0);
		}
		LSPI_Sleep(USP_ID2, 1);
	}
	else
	{
		if (conf->opts->wakeup_cmd && conf->opts->wakeup_cmd != 0xff)
		{
			lcd_write_cmd_data(conf,conf->opts->wakeup_cmd, NULL, 0);
		}
		else if (!conf->opts->sleep_cmd)
		{
			lcd_write_cmd_data(conf,LUAT_LCD_DEFAULT_WAKEUP, NULL, 0);
		}
		LSPI_Sleep(USP_ID2, 0);
		if (LUAT_LCD_IM_QSPI_MODE == conf->interface_mode)
		{
			luat_lcd_qspi_auto_flush_on_off(conf, 1);
		}
	}
	return 0;
}

int luat_lcd_init_in_service(luat_lcd_conf_t* conf)
{
	return send_event_to_task(g_s_lcd.task_handle, NULL, SERVICE_LCD_INIT, (uint32_t)conf, 0, 0, 0);
}

int luat_lcd_show_camera_in_service(void *camera_info, camera_cut_info_t *cut_info, uint16_t start_x, uint16_t start_y)
{

	PV_Union uPV;
	uPV.u16[0] = start_x;
	uPV.u16[1] = start_y;
	if (cut_info)
	{
		camera_cut_info_t *cut = luat_heap_malloc(sizeof(camera_cut_info_t));
		memcpy(cut, cut_info, sizeof(camera_cut_info_t));
		return send_event_to_task(g_s_lcd.task_handle, NULL, SERVICE_LCD_SHOW_CAMERA, (uint32_t)camera_info, (uint32_t)cut, uPV.u32, 0);
	}
	else
	{
		return send_event_to_task(g_s_lcd.task_handle, NULL, SERVICE_LCD_SHOW_CAMERA, (uint32_t)camera_info, 0, uPV.u32, 0);
	}
}

int luat_lcd_stop_show_camera(void)
{
	if (!g_s_lcd.camera_show_stop_sem) return -1;
	OS_MutexRelease(g_s_lcd.camera_show_stop_sem);
	return 0;
}

int luat_lcd_run_api_in_service(luat_lcd_api api, void *param, uint32_t param_len)
{
	return send_event_to_task(g_s_lcd.task_handle, NULL, SERVICE_RUN_USER_API, (uint32_t)api, (uint32_t)param, param_len, 0);
}
#ifdef TYPE_EC718M
int luat_lcd_qspi_config(luat_lcd_conf_t* conf, luat_lcd_qspi_conf_t *qspi_config)
{
	g_s_lcd.qspi_param = *qspi_config;
	g_s_lcd.auto_flush_ram_size = conf->w * conf->h * ((conf->bpp <= 16)?2:4);
	g_s_lcd.auto_flush_ram.p = luat_heap_opt_zalloc(LUAT_HEAP_AUTO, g_s_lcd.auto_flush_ram_size);
	return 0;
}
int luat_lcd_qspi_auto_flush_on_off(luat_lcd_conf_t* conf, uint8_t on_off)
{
	if (on_off)
	{
		if (!g_s_lcd.qspi_auto_flush_inited)
		{
			uint32_t div = (612*1024*1024 / conf->bus_speed);
			conf->bus_speed = 612*1024*1024 / div;
			uint32_t vlt = 0;
			uint32_t csht = 3;
			switch(div)
			{
			case 8:
				csht = 8;
				break;
			case 9:
				csht = 5;
				break;
			default:
				csht = 3;
				break;
			}
			vlt = conf->h + conf->vbp + conf->vfp + conf->vspw;
//			DBG("%u, %u, %d", conf->bus_speed, vlt, conf->flush_rate);
			vlt = (conf->bus_speed * 10) / (conf->flush_rate * vlt);
//			DBG("%u, %u, %d", conf->bus_speed, vlt, csht);
			LSPI_SetQspiLaneConfig(USP_ID2, 0, 2, g_s_lcd.qspi_param.write_4line_cmd);
			LSPI_QspiAutoFlushConfigAndRun(USP_ID2, g_s_lcd.auto_flush_ram.p, conf->w, conf->h,
					g_s_lcd.qspi_param.vsync_reg,
					g_s_lcd.qspi_param.hsync_reg,
					g_s_lcd.qspi_param.hsync_cmd,
					conf->vbp,
					conf->vfp,
					vlt, csht, g_s_lcd.need_record_frame_cnt);
		}
		else
		{
			LSPI_AutoFlushReStart(USP_ID2, g_s_lcd.auto_flush_ram.p, g_s_lcd.need_record_frame_cnt);
		}
	}
	else
	{
		if (g_s_lcd.qspi_auto_flush_inited)
		{
			LSPI_AutoFlushStop(USP_ID2);
		}
	}
	return 0;
}
#endif
#ifdef LUAT_USE_LCD_CUSTOM_DRAW

int luat_lcd_draw_no_block(luat_lcd_conf_t* conf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, luat_color_t* color, uint8_t last_flush)
{
	uint32_t retry_cnt = 0;
	while ((luat_lcd_service_cache_len() >= (x2-x1+1)*(y2-y1+1))){
		retry_cnt++;
		luat_rtos_task_sleep(1);
		if (retry_cnt > 20)
		{
			DBG("too much wait");
			luat_lcd_service_debug();
		}
	}
	retry_cnt = 0;
    while (luat_lcd_service_draw(conf, x1, y1, x2, y2, color, 0)){
		retry_cnt++;
		luat_rtos_task_sleep(1);
		if (retry_cnt > 20)
		{
			DBG("too much wait no mem");
			luat_lcd_service_debug();
			return -1;
		}
    }
	return 0;
}
int luat_lcd_flush(luat_lcd_conf_t* conf) {
    if (conf->buff == NULL) {
        return 0;
    }
    //LLOGD("luat_lcd_flush range %d %d", conf->flush_y_min, conf->flush_y_max);
    if (conf->flush_y_max < conf->flush_y_min) {
        // 没有需要刷新的内容,直接跳过
        //LLOGD("luat_lcd_flush no need");
        return 0;
    }
	const char* tmp = (const char*)(conf->buff + conf->flush_y_min * conf->w);
	if(conf->is_init_done) {
		luat_lcd_draw_no_block(conf, 0, conf->flush_y_min, conf->w - 1, conf->flush_y_max, tmp, 0);
	} else {
		uint32_t size = conf->w * (conf->flush_y_max - conf->flush_y_min + 1) * 2;
    	luat_lcd_set_address(conf, 0, conf->flush_y_min, conf->w - 1, conf->flush_y_max);
    	const char* tmp = (const char*)(conf->buff + conf->flush_y_min * conf->w);
		if (conf->port == LUAT_LCD_SPI_DEVICE){
			luat_spi_device_send((luat_spi_device_t*)(conf->lcd_spi_device), tmp, size);
		}else{
			luat_spi_send(conf->port, tmp, size);
		}
	}

    

    // 重置为不需要刷新的状态
    conf->flush_y_max = 0;
    conf->flush_y_min = conf->h;
    
    return 0;
}

int luat_lcd_draw(luat_lcd_conf_t* conf, int16_t x1, int16_t y1, int16_t x2, int16_t y2, luat_color_t* color) {
    if (x1 >= conf->w || y1 >= conf->h || x2 < 0 || y2 < 0 || x2 < x1 || y2 < y1) {
        // LLOGE("out of lcd buff range %d %d %d %d", x1, y1, x2, y2);
        // LLOGE("out of lcd buff range %d %d %d %d %d", x1 >= conf->w, y1 >= conf->h, y2 < 0, x2 < x1, y2 < y1);
        return 0;
    }
    if (y2 >= conf->h) {
        y2 = conf->h - 1;
    }

	if (conf->is_init_done) {
		return	luat_lcd_draw_no_block(conf, x1, y1, x2, y2, color, 0);
	}

    if (conf->opts->lcd_draw)
    	return conf->opts->lcd_draw(conf, x1, y1, x2, y2, color);
    // 直接刷屏模式
    if (conf->buff == NULL) {
        // 常规数据, 整体传输
        if (x1 >= 0 && y1 >= 0 && x2 <= conf->w && y2 <= conf->h) {
            uint32_t size = (x2 - x1 + 1) * (y2 - y1 + 1);
            // LLOGD("draw %dx%d %dx%d %d", x1, y1, x2, y2, size);
            luat_lcd_set_address(conf, x1, y1, x2, y2);
	        if (conf->port == LUAT_LCD_SPI_DEVICE){
		        luat_spi_device_send((luat_spi_device_t*)(conf->lcd_spi_device), (const char*)color, size* sizeof(luat_color_t));
	        }else{
		        luat_spi_send(conf->port, (const char*)color, size * sizeof(luat_color_t));
	        }
        }
        // 超出边界的数据, 按行传输
        else {
            int line_size = (x2 - x1 + 1);
            // LLOGD("want draw %dx%d %dx%d %d", x1, y1, x2, y2, line_size);
            luat_color_t* ptr = (luat_color_t*)color;
            for (int i = y1; i <= y2; i++)
            {
                if (i < 0) {
                    ptr += line_size;
                    continue;
                }
                luat_color_t* line = ptr;
                int lsize = line_size;
                int tmp_x1 = x1;
                int tmp_x2 = x2;
                if (x1 < 0) {
                    line += ( - x1);
                    lsize += (x1);
                    tmp_x1 = 0;
                }
                if (x2 > conf->w) {
                    lsize -= (x2 - conf->w);
                    tmp_x2 = conf->w;
                }
                // LLOGD("action draw %dx%d %dx%d %d", tmp_x1, i, tmp_x2, i, lsize);
				if (conf->is_init_done) {
					return	luat_lcd_draw_no_block(conf, x1, y1, x2, y2, color, 0);
				}
                luat_lcd_set_address(conf, tmp_x1, i, tmp_x2, i);
	            if (conf->port == LUAT_LCD_SPI_DEVICE){
		            luat_spi_device_send((luat_spi_device_t*)(conf->lcd_spi_device), (const char*)line, lsize * sizeof(luat_color_t));
	            }else{
		            luat_spi_send(conf->port, (const char*)line, lsize * sizeof(luat_color_t));
	            }
                ptr += line_size;
            }
            
            // TODO
            // LLOGD("超出边界,特殊处理");
        }
        return 0;
    }
    // buff模式
    int16_t x_end = x2 >= conf->w?  (conf->w - 1):x2;
    luat_color_t* dst = (conf->buff);
    size_t lsize = (x2 - x1 + 1);
    for (int16_t x = x1; x <= x2; x++)
    {
        if (x < 0 || x >= conf->w)
            continue;
        for (int16_t y = y1; y <= y2; y++)
        {
            if (y < 0 || y >= conf->h)
                continue;
            memcpy((char*)(dst + (conf->w * y + x)), (char*)(color + (lsize * (y-y1) + (x-x1))), sizeof(luat_color_t));
        }
    }
    // 存储需要刷新的区域
    if (y1 < conf->flush_y_min) {
        if (y1 >= 0)
            conf->flush_y_min = y1;
        else
            conf->flush_y_min = 0;
    }
    if (y2 > conf->flush_y_max) {
        conf->flush_y_max = y2;
    }
    return 0;
}

static void lcd_service_task_init(void)
{
	luat_lcd_service_init(60);
}
INIT_TASK_EXPORT(lcd_service_task_init, "1");
#else
#ifdef LUAT_USE_LCD_SERVICE
static void lcd_service_task_init(void)
{
	luat_lcd_service_init(90);
}
INIT_TASK_EXPORT(lcd_service_task_init, "1");
#endif
#endif
