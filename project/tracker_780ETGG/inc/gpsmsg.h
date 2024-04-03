#ifndef GPSMAG_H
#define GPSMAG_H

//UTC时间信息
typedef struct
{										    
 	uint16_t year;	//年份
	uint8_t month;	//月份
	uint8_t date;	//日期
	uint8_t hour; 	//小时
	uint8_t min; 	//分钟
	uint8_t sec; 	//秒钟
}nmea_utc_time;

//NMEA 0183 协议解析后数据存放结构体
typedef struct  
{										    
	nmea_utc_time utc;			    //UTC时间
	uint32_t latitude;				//纬度 分扩大100000倍,实际要除以100000
	uint8_t nshemi;					//北纬/南纬,N:北纬;S:南纬				  
	uint32_t longitude;			    //经度 分扩大100000倍,实际要除以100000
	uint8_t ewhemi;					//东经/西经,E:东经;W:西经
	uint8_t gpssta;					//GPS状态:0,未定位;1,定位成功			  
	int altitude;			 	    //海拔高度,放大了10倍,实际除以10.单位:0.1m	 
	uint16_t speed;					//地面速率,放大了1000倍,实际除以1000.单位:0.001公里/小时	
    uint16_t course;                //方位角
    uint8_t posslnum;				//用于定位的GPS卫星数,0~12.
}nmea_msg;


void gps_service_init(void);
void agps_service_init(void);
void agps_start_timer(void);

#endif