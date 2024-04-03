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
#include "luat_base.h"
#include "luat_rtos.h"
#include "common_api.h"
#include "luat_camera.h"
#include "driver_usp.h"
#include "soc_image_decode.h"

typedef struct
{
	CBFuncEx_t callback;
	void *param;
	uint16_t image_w;
	uint16_t image_h;
	uint8_t is_init;
	uint8_t is_running;
	uint8_t is_scan_mode;
}luat_camera_ctrl_t;

#ifdef __LUATOS__
#include "luat_msgbus.h"
#include "luat_mem.h"
#include "luat_zbuff.h"
#include "luat_lcd.h"
extern luat_lcd_conf_t *l_lcd_get_default_conf(void);
enum
{
	LUAT_CAMERA_EVENT_FRAME_QRDECODE = USER_EVENT_ID_START + 100,
	LUAT_CAMERA_EVENT_FRAME_JPEG_ENCODE,
	LUAT_CAMERA_EVENT_FRAME_NEW,
	LUAT_CAMERA_EVENT_FRAME_END,
	LUAT_CAMERA_EVENT_FRAME_START,
	LUAT_CAMERA_EVENT_FRAME_ERROR,
	LUAT_CAMERA_EVENT_START,
	LUAT_CAMERA_EVENT_STOP,
};

typedef struct
{
	luat_spi_camera_t config;
	luat_rtos_task_handle task_handle;
	Buffer_Struct result_buffer;
	luat_zbuff_t *buff;
	const char *save_path;
	void *p_cache[2];
	void *raw_buffer;
	uint32_t jpeg_data_point;
	uint16_t image_w;
	uint16_t image_h;
	uint8_t cur_cache;
	uint8_t is_process_image;
	uint8_t double_buffer_mode;
	uint8_t rx_to_user;				//单缓冲区时，当前是否在接收数据到用户区
	uint8_t scan_mode;					//扫码模式
	uint8_t raw_mode;					//原始图像模式
	uint8_t capture_stage;				//拍照流程
	uint8_t scan_pause;				//扫码暂停
	uint8_t preview_on_off;
	uint8_t camera_id;
	uint8_t is_work;
}luat_camera_app_t;

static luat_camera_app_t luat_camera_app;

static int luat_image_decode_callback(void *pdata, void *param)
{
	uint32_t buffer_sn = (uint32_t)param;
	uint32_t len = (uint32_t)pdata;
	if (len)
	{
		if (len > luat_camera_app.result_buffer.MaxLen)
		{
			OS_ReInitBuffer(&luat_camera_app.result_buffer, len);
		}
		else if ((len < 1024) && (luat_camera_app.result_buffer.MaxLen >= 1024))
		{
			OS_ReInitBuffer(&luat_camera_app.result_buffer, len);
		}
		luat_camera_image_decode_get_result(luat_camera_app.result_buffer.Data);
	    rtos_msg_t msg = {0};
		msg.handler = l_camera_handler;
		msg.ptr = luat_camera_app.result_buffer.Data;
		msg.arg1 = 0;
		msg.arg2 = len;
		luat_msgbus_put(&msg, 1);
	}
	luat_camera_app.is_process_image = 0;
	return 0;
}

static int luat_camera_irq_callback(void *pdata, void *param)
{
	uint8_t cur_cache = luat_camera_app.cur_cache;
	switch ((uint32_t)pdata)
	{
	case LUAT_CAMERA_FRAME_RX_DONE:
		if (luat_camera_app.scan_mode)
		{
			if (luat_camera_app.scan_pause)
			{
				if (luat_camera_app.rx_to_user)
				{
					luat_camera_app.rx_to_user = 0;
					CSPI_RxContinue(luat_camera_app.camera_id, 0);	//摄像头数据发送到底层，不传递给用户
				}
				luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_NEW, cur_cache, 0, 0, 0);
				return 0;
			}
			if (luat_camera_app.double_buffer_mode)
			{
				if (!luat_camera_app.rx_to_user)
				{
					luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_NEW, 0, 0, 0, 0);
					CSPI_RxContinue(luat_camera_app.camera_id, luat_camera_app.p_cache[0]);
					luat_camera_app.rx_to_user = 1;
					return 0;
				}
				//双缓冲模式下，扫码时允许1个解码，另一个接收数据
				if (!luat_camera_app.is_process_image)
				{
					luat_camera_app.is_process_image = 1;
					luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_QRDECODE, cur_cache, 0, 0, 0);
					luat_camera_app.cur_cache = !luat_camera_app.cur_cache;
					CSPI_RxContinue(luat_camera_app.camera_id, luat_camera_app.p_cache[luat_camera_app.cur_cache]);
				}
				else
				{
					CSPI_RxContinue(luat_camera_app.camera_id, luat_camera_app.p_cache[luat_camera_app.cur_cache]);
				}
			}
			else
			{
				if (!luat_camera_app.is_process_image)
				{
					if (luat_camera_app.rx_to_user)	//本次接收数据在用户区，则开始解码，并停止传递给用户区
					{
						luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_QRDECODE, 0, 0, 0, 0);
						CSPI_RxContinue(luat_camera_app.camera_id, 0);	//摄像头数据发送到底层，不传递给用户
						luat_camera_app.rx_to_user = 0;
						luat_camera_app.is_process_image = 1;
					}
					else	//本次接收数据不在用户区，则开始传递新的图像数据到用户区
					{
						CSPI_RxContinue(luat_camera_app.camera_id, luat_camera_app.p_cache[0]);
						luat_camera_app.rx_to_user = 1;
					}
				}
			}
		}
		else if (luat_camera_app.capture_stage && !luat_camera_app.is_process_image)
		{
			switch (luat_camera_app.capture_stage)
			{
			case 1:
				CSPI_RxContinue(luat_camera_app.camera_id, luat_camera_app.raw_mode?luat_camera_app.raw_buffer:luat_camera_app.p_cache[0]);
				luat_camera_app.capture_stage = 2;
				break;
			case 2:
				luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_JPEG_ENCODE, 0, 0, 0, 0);
				CSPI_RxContinue(luat_camera_app.camera_id, 0);	//摄像头数据发送到底层，不传递给用户
				luat_camera_app.is_process_image = 1;
				break;
			default:
				break;
			}
		}
		else
		{
			luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_NEW, cur_cache, 0, 0, 0);
		}
		break;
	case LUAT_CAMERA_FRAME_ERROR:
		luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_ERROR, cur_cache, 0, 0, 0);
		break;
	}
	return 0;
}
#endif

static luat_camera_ctrl_t g_s_camera[CSPI_MAX];

static int luat_camera_dummy_callback(void *pdata, void *param)
{
	int id = (int)param;
	return g_s_camera[id].callback(pdata, g_s_camera[id].param);
}


int luat_camera_setup(int id, luat_spi_camera_t *conf, void * callback, void *param)
{
#ifdef __LUATOS__
	if (id < 0 || id >= USP_ID2 || !conf ) return -ERROR_PARAM_INVALID;
	if (g_s_camera[id].is_init) return -ERROR_OPERATION_FAILED;
#else
	if (id < 0 || id >= USP_ID2 || !conf || !callback) return -ERROR_PARAM_INVALID;
	if (g_s_camera[id].is_init) return -ERROR_OPERATION_FAILED;
	g_s_camera[id].callback = callback;
	g_s_camera[id].param = param;
#endif
	if (id)
	{
		GPIO_IomuxEC7XX(18, 1, 1, 0);
		GPIO_IomuxEC7XX(19, 1, 1, 0);
		GPIO_IomuxEC7XX(21, 1, 1, 0);
		GPIO_IomuxEC7XX(22, 1, 1, 0);
	}
	else
	{
		GPIO_IomuxEC7XX(35, 1, 1, 0);
		GPIO_IomuxEC7XX(37, 1, 1, 0);
		GPIO_IomuxEC7XX(38, 1, 1, 0);
		GPIO_IomuxEC7XX(39, 1, 1, 0);
	}
	CSPI_Setup(id, conf->camera_speed, conf->spi_mode, conf->is_msb, conf->is_two_line_rx, conf->only_y, conf->seq_type, conf->rowScaleRatio, conf->colScaleRatio, conf->scaleBytes, conf->plat_param[0], conf->plat_param[1], conf->plat_param[2]);
	g_s_camera[id].is_running = 0;
	g_s_camera[id].is_init = 1;
	g_s_camera[id].image_w = conf->sensor_width;
	g_s_camera[id].image_h = conf->sensor_height;
#ifdef __LUATOS__
	luat_camera_app.camera_id = id;
	luat_camera_app.config = *conf;
	luat_camera_app.double_buffer_mode = ((conf->sensor_width * conf->sensor_height) <= 80000 );
	luat_camera_app.p_cache[0] = luat_heap_opt_malloc(LUAT_HEAP_PSRAM, luat_camera_app.config.sensor_width * luat_camera_app.config.sensor_height * 2);
	if (luat_camera_app.double_buffer_mode)
	{
		luat_camera_app.p_cache[1] = luat_heap_opt_malloc(LUAT_HEAP_PSRAM, luat_camera_app.config.sensor_width * luat_camera_app.config.sensor_height * 2);
	}
	else
	{
		luat_camera_app.p_cache[1] = luat_heap_opt_malloc(LUAT_HEAP_PSRAM, luat_camera_app.config.sensor_width * luat_camera_app.config.sensor_height / 2);
	}
#endif
	return id;
}

int luat_camera_set_image_w_h(int id, uint16_t w, uint16_t h)
{
	if (id < 0 || id >= USP_ID2 ) return -ERROR_PARAM_INVALID;
	g_s_camera[id].image_w = w;
	g_s_camera[id].image_h = h;
	return 0;
}

int luat_camera_image_decode_init(uint8_t type, void *stack, uint32_t stack_length, uint32_t priority)
{
	return soc_image_decode_init(stack, stack_length, priority, type);
}

int luat_camera_image_decode_once(uint8_t *data, uint16_t image_w, uint16_t image_h, uint32_t timeout, void *callback, void *param)
{
	return soc_image_decode_once(data, image_w, image_h, timeout, callback, param);
}

void luat_camera_image_decode_deinit(void)
{
	soc_image_decode_deinit();
}

int luat_camera_image_decode_get_result(uint8_t *buf)
{
	return soc_image_decode_get_result(buf);
}

int luat_camera_start(int id)
{
#ifdef __LUATOS__
	if (!luat_camera_app.task_handle || !g_s_camera[id].is_init) return -1;
	if (!g_s_camera[id].is_running)
	{
		CSPI_Rx(luat_camera_app.camera_id, NULL, luat_camera_app.config.sensor_width, luat_camera_app.config.sensor_height, luat_camera_irq_callback, id);
		g_s_camera[id].is_running = 1;
		luat_camera_app.is_work = 1;

	}
	if (luat_camera_app.scan_mode)
	{
		luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_START, 0, 0, 0, 0);
	}
	else
	{
		luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_FRAME_NEW, 0, 0, 0, 0);
	}
#else
	return -1;
#endif
}

int luat_camera_start_with_buffer(int id, void *buf)
{
	if (id < 0 || id >= USP_ID2) return -1;
	if (!g_s_camera[id].is_init || g_s_camera[id].is_running) return -ERROR_OPERATION_FAILED;
	CSPI_Rx(id, buf, g_s_camera[id].image_w, g_s_camera[id].image_h, luat_camera_dummy_callback, id);
	g_s_camera[id].is_running = 1;
	return 0;
}

void luat_camera_continue_with_buffer(int id, void *buf)
{
	CSPI_RxContinue(id, buf);
}

int luat_camera_stop(int id)
{
#ifdef __LUATOS__
	if (id < 0 || id >= USP_ID2) return -1;
	if (!g_s_camera[id].is_init || !g_s_camera[id].is_running) return -ERROR_OPERATION_FAILED;
	if (luat_camera_app.scan_mode)
	{
		luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_STOP, 0, 0, 0, 0);
	}
	luat_camera_app.raw_mode = 0;
#else
	return -1;
#endif
}

int luat_camera_pause(int id, uint8_t is_pause)
{
	return -1;
}

int luat_camera_close(int id)
{
	if (id < 0 || id >= USP_ID2) return -1;
	if (!g_s_camera[id].is_init) return -ERROR_OPERATION_FAILED;
	CSPI_Stop(id);
	g_s_camera[id].is_init = 0;
	g_s_camera[id].is_running = 0;
	g_s_camera[id].callback = luat_camera_dummy_callback;
#ifdef __LUATOS__
	if (luat_camera_app.scan_mode)
	{
		luat_rtos_event_send(luat_camera_app.task_handle, LUAT_CAMERA_EVENT_STOP, 0, 0, 0, 0);
	}
	luat_heap_free(luat_camera_app.p_cache[0]);
	luat_heap_free(luat_camera_app.p_cache[1]);
	luat_camera_app.p_cache[0] = NULL;
	luat_camera_app.p_cache[1] = NULL;
	OS_DeInitBuffer(&luat_camera_app.result_buffer);
	luat_camera_app.is_work = 0;
#endif
	return 0;

}

#ifdef __LUATOS__

static void luat_camera_save_JPEG_data(void *cxt, void *data, int size)
{
	uint8_t *p = luat_camera_app.p_cache[1];
	memcpy(p + luat_camera_app.jpeg_data_point, data, size);
	luat_camera_app.jpeg_data_point += size;
}

static void luat_camera_task(void *param)
{
	luat_event_t event;
	rtos_msg_t msg = {0};
    HANDLE JPEGEncodeHandle = NULL;
    HANDLE fd;
    void *stack = NULL;
    uint8_t *ycb_cache;
    uint8_t *file_data;
    uint8_t *p_cache;
    uint32_t i,j,k;
    int result;
	while(1)
	{
		if (luat_camera_app.is_work)
		{
			result = luat_rtos_event_recv(luat_camera_app.task_handle, 0, &event, NULL, 1000);
			if (result && luat_camera_app.is_work)
			{
		        msg.handler = l_camera_handler;
		        msg.ptr = NULL;
		        msg.arg1 = 0;
		        msg.arg2 = 0;
		        luat_msgbus_put(&msg, 1);
				continue;
			}
		}
		else
		{
			luat_rtos_event_recv(luat_camera_app.task_handle, 0, &event, NULL, LUAT_WAIT_FOREVER);
		}
		switch(event.id)
		{
		case LUAT_CAMERA_EVENT_FRAME_QRDECODE:
			luat_camera_image_decode_once(luat_camera_app.p_cache[event.param1], luat_camera_app.config.sensor_width, luat_camera_app.config.sensor_height, 60, luat_image_decode_callback, event.param1);
			break;
		case LUAT_CAMERA_EVENT_FRAME_JPEG_ENCODE:
			if (luat_camera_app.raw_mode)
			{
				luat_camera_app.capture_stage = 0;
				luat_camera_app.raw_mode = 0;
		        msg.handler = l_camera_handler;
		        msg.ptr = NULL;
		        msg.arg1 = 0;
		        msg.arg2 = luat_camera_app.config.sensor_width * luat_camera_app.config.sensor_height * (luat_camera_app.config.only_y?1:2);
		        luat_msgbus_put(&msg, 1);
				luat_camera_app.is_process_image = 0;
				break;
			}
			p_cache = luat_camera_app.p_cache[0];
			luat_camera_app.jpeg_data_point = 0;
			JPEGEncodeHandle = jpeg_encode_init(luat_camera_save_JPEG_data, 0, 1, luat_camera_app.config.sensor_width, luat_camera_app.config.sensor_height, 3);
			ycb_cache = malloc(luat_camera_app.config.sensor_width * 8 * 3);
			for(i = 0; i < luat_camera_app.config.sensor_height; i+= 8)
			{
				if (luat_camera_app.config.only_y)
				{
					for(j = i * luat_camera_app.config.sensor_width , k = 0; j < (i + 8) * luat_camera_app.config.sensor_width; j+=2, k+=6)
					{
						ycb_cache[k] = p_cache[j];
						ycb_cache[k + 1] = 128;
						ycb_cache[k + 2] = 128;
						ycb_cache[k + 3] = p_cache[j + 1];
						ycb_cache[k + 4] = 128;
						ycb_cache[k + 5] = 128;
					}
				}
				else
				{
					for(j = i * luat_camera_app.config.sensor_width * 2, k = 0; j < (i + 8) * luat_camera_app.config.sensor_width * 2; j+=4, k+=6)
					{
						ycb_cache[k] = p_cache[j];
						ycb_cache[k + 1] = p_cache[j + 1];
						ycb_cache[k + 2] = p_cache[j + 3];
						ycb_cache[k + 3] = p_cache[j + 2];
						ycb_cache[k + 4] = p_cache[j + 1];
						ycb_cache[k + 5] = p_cache[j + 3];
					}
				}

				jpeg_encode_run(JPEGEncodeHandle, ycb_cache);
			}
			jpeg_encode_end(JPEGEncodeHandle);
			free(JPEGEncodeHandle);
			free(ycb_cache);
			if (luat_camera_app.buff)
			{
				if (luat_camera_app.buff->len < luat_camera_app.jpeg_data_point)
				{
					__zbuff_resize(luat_camera_app.buff, luat_camera_app.jpeg_data_point);
				}
				memcpy(luat_camera_app.buff->addr, luat_camera_app.p_cache[1], luat_camera_app.jpeg_data_point);
				luat_camera_app.buff->used = luat_camera_app.jpeg_data_point;
			}
			else
			{
				if (luat_camera_app.save_path)
				{
					fd = luat_fs_fopen(luat_camera_app.save_path, "w");
				}
				else
				{
					fd = luat_fs_fopen("/capture.jpg", "w");
				}
				if (fd)
				{
					luat_fs_fwrite(luat_camera_app.p_cache[1], luat_camera_app.jpeg_data_point, 1, fd);
					luat_fs_fclose(fd);
				}
			}
		    {
		        msg.handler = l_camera_handler;
		        msg.ptr = NULL;
		        msg.arg1 = 0;
		        msg.arg2 = luat_camera_app.jpeg_data_point;
		        luat_msgbus_put(&msg, 1);
			}
			luat_camera_app.is_process_image = 0;
			luat_camera_app.capture_stage = 0;
			break;
		case LUAT_CAMERA_EVENT_FRAME_NEW:
			break;
		case LUAT_CAMERA_EVENT_START:
			if (!stack)
			{
				stack = luat_heap_opt_malloc(LUAT_HEAP_AUTO, 250 * 1024);
				if (stack)
				{
					luat_camera_image_decode_init(0, stack, 250 * 1024, 10);
					luat_camera_app.scan_pause = 0;
				}
				else
				{
					DBG("no mem stack, decode init failed");
				}
			}
			break;
		case LUAT_CAMERA_EVENT_STOP:
			if (stack)
			{
				luat_camera_image_decode_deinit();
				luat_heap_free(stack);
				stack = NULL;
			}
			luat_camera_app.scan_pause = 1;
			break;
		case LUAT_CAMERA_EVENT_FRAME_ERROR:
	        msg.handler = l_camera_handler;
	        msg.ptr = NULL;
	        msg.arg1 = 0;
	        msg.arg2 = 0;
	        luat_msgbus_put(&msg, 1);
	        break;
		}
	}
}

int luat_camera_init(luat_camera_conf_t *conf)
{
	if (!luat_camera_app.task_handle) luat_rtos_task_create(&luat_camera_app.task_handle, 6 * 1024, 10, "camera", luat_camera_task, NULL, 64);
	return 0;
}

int luat_camera_capture_in_ram(int id, uint8_t quality, void *buffer)
{
	if (!luat_camera_app.scan_mode && !luat_camera_app.capture_stage && !luat_camera_app.is_process_image)
	{
		luat_camera_app.buff = buffer;
		luat_camera_app.capture_stage = 1;
		return 0;
	}
	else
	{
		DBG("device busy!");
	}
	return -1;
}

int luat_camera_capture(int id, uint8_t quality, const char *path)
{
	if (!luat_camera_app.scan_mode && !luat_camera_app.capture_stage && !luat_camera_app.is_process_image)
	{
		luat_camera_app.buff = NULL;
		if (path)
		{
			luat_camera_app.buff = NULL;
			if (luat_camera_app.save_path)
			{
				free(luat_camera_app.save_path);
			}
			luat_camera_app.save_path = calloc(1, strlen(path) + 1);
			strcpy(luat_camera_app.save_path, path);
			DBG("save file in %s", luat_camera_app.save_path);
		}
		luat_camera_app.capture_stage = 1;
		return 0;
	}
	else
	{
		DBG("device busy!");
	}
	return -1;
}

int luat_camera_work_mode(id, mode)
{
	if (g_s_camera[id].is_init)
	{
		DBG("device busy!");
		return -1;
	}
	switch(mode)
	{
	case LUAT_CAMERA_MODE_AUTO:
		luat_camera_app.scan_mode = 0;
		break;
	case LUAT_CAMERA_MODE_SCAN:
		luat_camera_app.scan_mode = 1;
		break;
	}
	return 0;
}

int luat_camera_preview(int id, uint8_t on_off)
{
	luat_camera_app.config.lcd_conf = l_lcd_get_default_conf();
	if (luat_camera_app.config.lcd_conf)
	{
		if (!on_off)
		{
			if (luat_camera_app.preview_on_off)
			{
				luat_lcd_stop_show_camera();
				luat_camera_app.preview_on_off = 0;
			}
			else
			{
				DBG("preview already off");
			}
			return 0;
		}
		if (luat_camera_app.preview_on_off)
		{
			DBG("preview already on");
			return -1;
		}
		if ((luat_camera_app.config.sensor_width == luat_camera_app.config.lcd_conf->w) && (luat_camera_app.config.sensor_height == luat_camera_app.config.lcd_conf->h))
		{
			luat_lcd_show_camera_in_service(&luat_camera_app.config, 0, 0, 0);
		}
		else
		{
			uint16_t x,y;
			camera_cut_info_t cut = {0};
			if (luat_camera_app.config.sensor_width >= luat_camera_app.config.lcd_conf->w)
			{
				x = 0;
				cut.left_cut_lines = (luat_camera_app.config.sensor_width - luat_camera_app.config.lcd_conf->w) >> 1;
				cut.right_cut_lines = cut.left_cut_lines;
			}
			else
			{
				x = (luat_camera_app.config.lcd_conf->w - luat_camera_app.config.sensor_width) >> 1;
			}
			if (luat_camera_app.config.sensor_height >= luat_camera_app.config.lcd_conf->h)
			{
				y = 0;
				cut.top_cut_lines = (luat_camera_app.config.sensor_height - luat_camera_app.config.lcd_conf->h) >> 1;
				cut.bottom_cut_lines = cut.top_cut_lines;
			}
			else
			{
				y = (luat_camera_app.config.lcd_conf->h - luat_camera_app.config.sensor_height) >> 1;
			}
			//DBG("%d,%d,%d,%d,%d,%d",x,y,cut.top_cut_lines,cut.bottom_cut_lines,cut.left_cut_lines,cut.right_cut_lines);
			luat_lcd_show_camera_in_service(&luat_camera_app.config, &cut, x, y);
		}
		luat_camera_app.preview_on_off = 1;
		return 0;
	}
	else
	{
		return -1;
	}

}
int luat_camera_get_raw_start(int id, int w, int h, uint8_t *data, uint32_t max_len)
{
	if (id < 0 || id >= USP_ID2) return -1;
	if (!g_s_camera[id].is_init || !g_s_camera[id].is_running || luat_camera_app.scan_mode || luat_camera_app.capture_stage || luat_camera_app.is_process_image) return -ERROR_OPERATION_FAILED;
	luat_camera_app.raw_buffer = data;
	luat_camera_app.raw_mode = 1;
	luat_camera_app.capture_stage = 1;
	return 0;
}
int luat_camera_get_raw_again(int id)
{
	if (id < 0 || id >= USP_ID2) return -1;
	if (!g_s_camera[id].is_init || !g_s_camera[id].is_running || luat_camera_app.scan_mode || luat_camera_app.capture_stage || luat_camera_app.is_process_image || !luat_camera_app.raw_buffer) return -ERROR_OPERATION_FAILED;
	luat_camera_app.raw_mode = 1;
	luat_camera_app.capture_stage = 1;
	return 0;
}
#else
int luat_camera_init(luat_camera_conf_t *conf) {return -1;}
int luat_camera_capture(int id, uint8_t quality, const char *path) {return -1;}
int luat_camera_work_mode(id, mode){return -1;}
#endif


