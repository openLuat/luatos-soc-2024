#ifndef  LCD_NV3037_H_
#define  LCD_NV3037_H_

#define     LCD_NV3037
#define     LCD_NAME "NV3037"
#define     LCD_BPP  16

#ifndef     LCD_WIDTH_3037
#define     LCD_WIDTH_3037     320
#undef      LCD_WIDTH
#define     LCD_WIDTH      LCD_WIDTH_3037 
#endif

#ifndef     LCD_HEIGHT_3037
#define     LCD_HEIGHT_3037    480 
#undef      LCD_HEIGHT
#define     LCD_HEIGHT     LCD_HEIGHT_3037 
#endif

#ifndef LCD_SPI_DATA_LANE
#define LCD_SPI_DATA_LANE   2 
#endif
#endif
