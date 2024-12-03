#ifndef  LCD_ST7789_
#define  LCD_ST7789_

#define     ST7789_BPP              (16) // 12: 444;   16:565;   18:666
#define     ST7789_WIDTH            (240)
#define     ST7789_HEIGHT           (320) 
#define     ST7789_FREQ             (51*1024*1024)
#define     ST7789_INTERFACE        (SPI_4W_II)
#define     ST7789_TIME_OF_FRAME    (149356) // us
#define     ST7789_TE_CYCLE         (16742) // us
#define     ST7789_TE_WAIT_TIME     (623)   // us
#define     ST7789_X_OFFSET		 	(0)
#define     ST7789_Y_OFFSET		 	(0)

#if (LCD_INTERFACE_8080 == 1)
#if (ST7789_INTERFACE != INTERFACE_8080)
#error "Please choose 8080 interface for ST7789_INTERFACE"
#endif
#elif (LCD_INTERFACE_SPI == 1)
#if ((ST7789_INTERFACE != SPI_3W_I) && (ST7789_INTERFACE != SPI_3W_II) && (ST7789_INTERFACE != SPI_4W_I) && (ST7789_INTERFACE != SPI_4W_II))
#error "Please choose SPI interface for ST7789_INTERFACE"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (ST7789_HEIGHT)
#define     LCD_WIDTH            (ST7789_WIDTH)
#define     LCD_BPP_USE          (ST7789_BPP)
#define     LCD_INTERFACE        (ST7789_INTERFACE)
#define     LCD_FREQ			 (ST7789_FREQ)
#define     LCD_X_OFFSET		 (ST7789_X_OFFSET)
#define     LCD_Y_OFFSET		 (ST7789_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_FREQ			 (ST7789_FREQ)
#define     LCD_TIME_OF_FRAME    (ST7789_TIME_OF_FRAME) // us
#define     LCD_TE_CYCLE         (ST7789_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (ST7789_TE_WAIT_TIME)  // us

#endif
