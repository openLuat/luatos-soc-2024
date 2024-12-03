#ifndef  LCD_SH8601_
#define  LCD_SH8601_

#define     SH8601_BPP              (16) // 16:565;   18:666   24:888
#define     SH8601_WIDTH            (466)
#define     SH8601_HEIGHT           (466) 
#define     SH8601_FREQ             (8*1024*1024)
#define     SH8601_INTERFACE        (MSPI_4W_II)
#define     SH8601_TIME_OF_FRAME    (167331) // us
#define     SH8601_TE_CYCLE         (16742) // us
#define     SH8601_TE_WAIT_TIME     (623)   // us
#define     SH8601_X_OFFSET		 	(0)
#define     SH8601_Y_OFFSET		 	(0)

#if (ST8601_INTERFACE == MSPI_4W_II)
#if (LCD_INTERFACE_MSPI != 1)
#error "Please choose MSPI interface in RTE_Device.h"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (SH8601_HEIGHT)
#define     LCD_WIDTH            (SH8601_WIDTH) 
#define     DEFAULT_INST         (0x2)
#define     LCD_BPP_USE          (SH8601_BPP)
#define     LCD_INTERFACE        (SH8601_INTERFACE)
#define     LCD_FREQ			 (SH8601_FREQ)
#define     LCD_X_OFFSET		 (SH8601_X_OFFSET)
#define     LCD_Y_OFFSET		 (SH8601_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_TIME_OF_FRAME    (SH8601_TIME_OF_FRAME) // us
#define     LCD_TE_CYCLE         (SH8601_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (SH8601_TE_WAIT_TIME)  // us

#endif
