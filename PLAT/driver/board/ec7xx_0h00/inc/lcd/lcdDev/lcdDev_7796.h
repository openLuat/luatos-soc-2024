#ifndef  LCD_ST7796_H
#define  LCD_ST7796_H

#define     LCD_ST7796
#define     LCD_NAME "ST7796"
#define     LCD_BPP  16

#ifndef     LCD_WIDTH_7796
#define     LCD_WIDTH_7796     320
#undef      LCD_WIDTH
#define     LCD_WIDTH      LCD_WIDTH_7796 
#endif

#ifndef     LCD_HEIGHT_7796
#define     LCD_HEIGHT_7796    480 
#undef      LCD_HEIGHT
#define     LCD_HEIGHT     LCD_HEIGHT_7796 
#endif

#endif
