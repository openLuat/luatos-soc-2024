/*
 * soc_lcd.h
 *
 *  Created on: 2023年7月6日
 *      Author: Administrator
 */

#ifndef DRIVER_INCLUDE_SOC_LCD_H_
#define DRIVER_INCLUDE_SOC_LCD_H_
#include "common_api.h"
typedef struct
{
	uint8_t *Data;
	uint32_t Speed;
	uint32_t Size;
	uint16_t x1;
	uint16_t y1;
	uint16_t x2;
	uint16_t y2;
	uint16_t xoffset;
	uint16_t yoffset;
	uint8_t DCDelay;
	uint8_t SpiID;
	uint8_t Mode;
	uint8_t CSPin;
	uint8_t DCPin;
	uint8_t ColorMode;
}LCD_DrawStruct;

uint32_t LCD_DrawCacheLen(void);
void LCD_Draw(LCD_DrawStruct *Draw);
void LCD_DrawBlock(LCD_DrawStruct *Draw);
void LCD_CameraDraw(LCD_DrawStruct *Draw);
void LCD_ServiceInit(uint8_t Priority);
void LCD_RunUserAPI(CBDataFun_t CB, uint32_t data, uint32_t param, uint32_t timeout);
#endif /* DRIVER_INCLUDE_SOC_LCD_H_ */
