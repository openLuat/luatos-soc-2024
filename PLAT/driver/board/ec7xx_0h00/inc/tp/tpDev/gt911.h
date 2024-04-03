/****************************************************************************
 *
 * Copy right:   2020-, Copyrigths of AirM2M Ltd.
 * File name:    GT911.h
 * Description:  EC7xx touchpanel driver file
 * History:      Rev1.0   2023-09-18
 *
 ****************************************************************************/

#ifndef _TP_GT911_H
#define _TP_GT911_H

#include "ec7xx.h"
#include "Driver_Common.h"
 #ifdef __cplusplus
 extern "C" {
#endif


//I2C读写命令	
// #define GT911_ADDR 		0X5D     	
#define GT911_ADDR 	  0x14    //由初始化INT输出状态决定    	 
/********************************GT911部分寄存器定义***************************/
#define GT_CTRL_REG 	0X8040   	//GT911控制寄存器
#define GT_CFGS_REG 	0X8047   	//配置版本
#define GT_X_MAX_LOW 	0X8048   	//X轴低字节
#define GT_X_MAX_HOW 	0X8049   	//X轴高字节
#define GT_Y_MAX_LOW 	0X804A   	//Y轴低字节
#define GT_Y_MAX_HOW 	0X804B   	//Y轴高字节
#define GT_TOUCH_NUM  0X804C      //输出触摸点数1--10


#define GT_CHECK_REG 	0X80FF   	//GT911校验和寄存器
#define GT_PID_REG 		0X8140   	//GT911产品ID寄存器

#define GT_GSTID_REG 	0X814E   	//当前检测到的触摸情况
#define GT_TP1_REG 		0X8150  	//第一个触摸点数据地址
#define GT_TP2_REG 		0X8158		//第二个触摸点数据地址
#define GT_TP3_REG 		0X8160		//第三个触摸点数据地址
#define GT_TP4_REG 		0X8168		//第四个触摸点数据地址
#define GT_TP5_REG 		0X8170		//第五个触摸点数据地址

/*******************************************************************************
 * API
 ******************************************************************************/



#ifdef __cplusplus
}
#endif

#endif
