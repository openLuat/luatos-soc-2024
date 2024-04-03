#ifndef _LCD_GC9307_H_
#define _LCD_GC9307_H_

#define LCD_GC9307
#define LCD_NAME "GC9307"
#define LCD_BPP  16

#ifndef LCD_WIDTH_9307
#define LCD_WIDTH_9307      (240)
#undef  LCD_WIDTH
#define LCD_WIDTH           LCD_WIDTH_9307 
#endif

#ifndef LCD_HEIGHT_9307
#define LCD_HEIGHT_9307     (280)
#undef  LCD_HEIGHT
#define LCD_HEIGHT          LCD_HEIGHT_9307 
#endif

#endif