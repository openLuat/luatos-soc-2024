

#include "minmea.h"
#include "common_api.h"
#include "luat_rtos.h"
#include "luat_debug.h"
#include "luat_uart.h"
#include "luat_gpio.h"
#include "agnss.h"
#define UART_ID 2
#define USE_780ETGG 0 //如果使用780ETGG 需要设置为1
#define USE_780EPVH 1 //如果使用780EPVH 需要设置为1
typedef struct
{
    char *gnss_data;
    size_t gnss_data_len;
}gnss_data_struct;


enum
{
	EVENT_NEW_GNSS_DATA = 1,
};

static luat_rtos_task_handle gnss_parse_task_handle;

static int libminmea_parse_data(const char *data, size_t len)
{
    size_t prev = 0;
    static char nmea_tmp_buff[86] = {0}; // nmea 最大长度82,含换行符
    for (size_t offset = 0; offset < len; offset++)
    {
        // \r == 0x0D  \n == 0x0A
        if (data[offset] == 0x0A)
        {
            // 最短也需要是 OK\r\n
            // 应该\r\n的
            // 太长了
            if (offset - prev < 3 || data[offset - 1] != 0x0D || offset - prev > 82)
            {
                prev = offset + 1;
                continue;
            }
            memcpy(nmea_tmp_buff, data + prev, offset - prev - 1);
            nmea_tmp_buff[offset - prev - 1] = 0x00;
            parse_nmea((const char *)nmea_tmp_buff);
            prev = offset + 1;
        }
    }
    return 0;
}

void luat_uart_recv_cb(int uart_id, uint32_t data_len)
{
	if (data_len)
	{
		luat_rtos_event_send(gnss_parse_task_handle, EVENT_NEW_GNSS_DATA, 0, 0, 0, 0);
	}
}

int parse_nmea(const char *gnssdata)
{
    switch (minmea_sentence_id(gnssdata, false))
    {
    case MINMEA_SENTENCE_RMC:
    {
        struct minmea_sentence_rmc frame;
        if (minmea_parse_rmc(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxRMC: raw coordinates and speed: (%d/%d,%d/%d) %d/%d",
                             frame.latitude.value, frame.latitude.scale,
                             frame.longitude.value, frame.longitude.scale,
                             frame.speed.value, frame.speed.scale);
            LUAT_DEBUG_PRINT("$xxRMC fixed-point coordinates and speed scaled to three decimal places: (%d,%d) %d",
                             minmea_rescale(&frame.latitude, 1000),
                             minmea_rescale(&frame.longitude, 1000),
                             minmea_rescale(&frame.speed, 1000));
            LUAT_DEBUG_PRINT("$xxRMC floating point degree coordinates and speed: (%f,%f) %f",
                             minmea_tocoord(&frame.latitude),
                             minmea_tocoord(&frame.longitude),
                             minmea_tofloat(&frame.speed));
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxRMC sentence is not parsed");
        }
    }
    break;
    case MINMEA_SENTENCE_GGA:
    {
        struct minmea_sentence_gga frame;
        if (minmea_parse_gga(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxGGA: fix quality: %d", frame.fix_quality);
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxGGA sentence is not parsed");
        }
    }
    break;
    case MINMEA_SENTENCE_GST:
    {
        struct minmea_sentence_gst frame;
        if (minmea_parse_gst(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxGST: raw latitude,longitude and altitude error deviation: (%d/%d,%d/%d,%d/%d)",
                             frame.latitude_error_deviation.value, frame.latitude_error_deviation.scale,
                             frame.longitude_error_deviation.value, frame.longitude_error_deviation.scale,
                             frame.altitude_error_deviation.value, frame.altitude_error_deviation.scale);
            LUAT_DEBUG_PRINT("$xxGST fixed point latitude,longitude and altitude error deviation"
                                           " scaled to one decimal place: (%d,%d,%d)",
                             minmea_rescale(&frame.latitude_error_deviation, 10),
                             minmea_rescale(&frame.longitude_error_deviation, 10),
                             minmea_rescale(&frame.altitude_error_deviation, 10));
            LUAT_DEBUG_PRINT("$xxGST floating point degree latitude, longitude and altitude error deviation: (%f,%f,%f)",
                             minmea_tofloat(&frame.latitude_error_deviation),
                             minmea_tofloat(&frame.longitude_error_deviation),
                             minmea_tofloat(&frame.altitude_error_deviation));
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxGST sentence is not parsed");
        }
    }
    break;
    case MINMEA_SENTENCE_GSV:
    {
        struct minmea_sentence_gsv frame;
        if (minmea_parse_gsv(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxGSV: message %d of %d\n", frame.msg_nr, frame.total_msgs);
            LUAT_DEBUG_PRINT("$xxGSV: satellites in view: %d\n", frame.total_sats);
            for (int i = 0; i < 4; i++)
                LUAT_DEBUG_PRINT("$xxGSV: sat nr %d, elevation: %d, azimuth: %d, CN: %d db",
                                 frame.sats[i].nr,
                                 frame.sats[i].elevation,
                                 frame.sats[i].azimuth,
                                 frame.sats[i].snr);
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxGSV sentence is not parsed");
        }
    }
    break;
    case MINMEA_SENTENCE_VTG:
    {
        struct minmea_sentence_vtg frame;
        if (minmea_parse_vtg(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxVTG: true track degrees = %f",
                             minmea_tofloat(&frame.true_track_degrees));
            LUAT_DEBUG_PRINT("        magnetic track degrees = %f",
                             minmea_tofloat(&frame.magnetic_track_degrees));
            LUAT_DEBUG_PRINT("        speed knots = %f",
                             minmea_tofloat(&frame.speed_knots));
            LUAT_DEBUG_PRINT("        speed kph = %f",
                             minmea_tofloat(&frame.speed_kph));
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxVTG sentence is not parsed");
        }
    }
    break;
    case MINMEA_SENTENCE_ZDA:
    {
        struct minmea_sentence_zda frame;
        if (minmea_parse_zda(&frame, gnssdata))
        {
            LUAT_DEBUG_PRINT("$xxZDA: %d:%d:%d %02d.%02d.%d UTC%+03d:%02d",
                             frame.time.hours,
                             frame.time.minutes,
                             frame.time.seconds,
                             frame.date.day,
                             frame.date.month,
                             frame.date.year,
                             frame.hour_offset,
                             frame.minute_offset);
        }
        else
        {
            LUAT_DEBUG_PRINT("$xxZDA sentence is not parsed");
        }
    }
    break;
    case MINMEA_INVALID:
    {
        LUAT_DEBUG_PRINT("$xxxxx sentence is not valid");
    }
    break;
    default:
    {
        LUAT_DEBUG_PRINT("$xxxxx sentence is not parsed");
    }
    break;
    }
    return 0;
}
static void gnss_parse_task(void *param)
{
    luat_uart_t uart = {
        .id = UART_ID,
#if USE_780EPVH
		.baud_rate = 9600,
#else
        .baud_rate = 115200,
#endif
        .data_bits = 8,
        .stop_bits = 1,
		.bufsz = 4096,
        .parity = 0};
    luat_uart_setup(&uart);


    luat_uart_ctrl(UART_ID, LUAT_UART_SET_RECV_CALLBACK, luat_uart_recv_cb);
    luat_mcu_xtal_ref_output(1, 0);
#if USE_780ETGG
    luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = HAL_GPIO_16;
	luat_gpio_open(&gpio_cfg);
    luat_gpio_set(HAL_GPIO_16,1);	//GNSS VCC-BAK
	gpio_cfg.pin = HAL_GPIO_13;
	luat_gpio_open(&gpio_cfg);
    luat_gpio_set(HAL_GPIO_13,1);	//GNSS VCC
#endif
#if USE_780EPVH
    luat_gpio_cfg_t gpio_cfg;
	luat_gpio_set_default_cfg(&gpio_cfg);
	gpio_cfg.pin = HAL_GPIO_23;
	luat_gpio_open(&gpio_cfg);
    luat_gpio_set(HAL_GPIO_23,1);	//GNSS VCC-BAK
	gpio_cfg.pin = HAL_GPIO_17;
	gpio_cfg.alt_fun = 4;
	luat_gpio_open(&gpio_cfg);
    luat_gpio_set(HAL_GPIO_17,1);	//GNSS VCC
#endif
    luat_event_t event;
    Buffer_Struct nmea_data_buffer;
    uint32_t read_len, i;
    uint8_t temp[128];
    uint8_t new_line_flag = 0;
    OS_InitBuffer(&nmea_data_buffer, 4096);
    while (1)
    {
    	luat_rtos_event_recv(gnss_parse_task_handle, EVENT_NEW_GNSS_DATA, &event, NULL, LUAT_WAIT_FOREVER);
    	read_len = luat_uart_read(UART_ID, temp, 128);
    	if (read_len)
    	{
    		do
    		{
    			OS_BufferWrite(&nmea_data_buffer, temp, read_len);
    			read_len = luat_uart_read(UART_ID, temp, 128);
    		}while(read_len > 0);
    		do
    		{
    			new_line_flag = 0;
    			for (i = 0; i < nmea_data_buffer.Pos; i++)
    			{
    				if ('\n' == nmea_data_buffer.Data[i])
    				{
    					if (i >= 6 && ('\r' == nmea_data_buffer.Data[i-1]))
    					{
        					new_line_flag = 1;
        					nmea_data_buffer.Data[i] = 0;
        					LUAT_DEBUG_PRINT("gnssdata:\n %s", nmea_data_buffer.Data);
        					parse_nmea((const char *)nmea_data_buffer.Data);
    					}
    					OS_BufferRemove(&nmea_data_buffer, i+1);
    					break;
    				}
    			}
    		}while(new_line_flag);
    	}

    }
}
static void task_gnss_init(void)
{
    luat_rtos_task_create(&gnss_parse_task_handle, 1024 * 8, 30, "gnss_parse", gnss_parse_task, NULL, 16);
    task_ephemeris();
}

extern void network_init(void);
INIT_TASK_EXPORT(task_gnss_init, "1");
INIT_TASK_EXPORT(network_init, "1");
