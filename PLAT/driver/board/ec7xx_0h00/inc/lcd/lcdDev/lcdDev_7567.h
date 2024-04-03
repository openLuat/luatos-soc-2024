#ifndef  LCD_ST7567_H_
#define  LCD_ST7567_H_

#define     LCD_ST7567
#define     LCD_NAME    "ST7567"
#define     LCD_BPP     1

#ifndef     LCD_WIDTH_7567
#define     LCD_WIDTH_7567     128
#undef      LCD_WIDTH
#define     LCD_WIDTH      LCD_WIDTH_7567 
#endif

#ifndef     LCD_HEIGHT_7567
#define     LCD_HEIGHT_7567    64 
#undef      LCD_HEIGHT
#define     LCD_HEIGHT     LCD_HEIGHT_7567 
#endif

#endif