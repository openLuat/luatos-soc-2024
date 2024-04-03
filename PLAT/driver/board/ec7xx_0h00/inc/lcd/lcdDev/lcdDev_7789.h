#ifndef  LCD_ST7789_H_
#define  LCD_ST7789_H_

#define     LCD_ST7789
#define     LCD_NAME "ST7789"
#define     LCD_BPP  16

#ifndef     LCD_WIDTH_7789
#define     LCD_WIDTH_7789     240
#undef      LCD_WIDTH
#define     LCD_WIDTH      LCD_WIDTH_7789 
#endif

#ifndef     LCD_HEIGHT_7789
#define     LCD_HEIGHT_7789    320 
#undef      LCD_HEIGHT
#define     LCD_HEIGHT     LCD_HEIGHT_7789 
#endif

#endif
