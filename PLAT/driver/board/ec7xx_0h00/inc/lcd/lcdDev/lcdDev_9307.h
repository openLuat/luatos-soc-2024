#ifndef  LCD_GC9307_
#define  LCD_GC9307_

#define     GC9307_BPP              (16) // 12: 444;   16:565;   18:666
#define     GC9307_WIDTH            (240)
#define     GC9307_HEIGHT           (320) 
#define     GC9307_FREQ             (51*1024*1024)
#define     GC9307_INTERFACE        (SPI_4W_I)
#define     GC9307_TIME_OF_FRAME    (149356) // us
#define     GC9307_TE_CYCLE         (16742) // us
#define     GC9307_TE_WAIT_TIME     (623)   // us
#define     GC9307_X_OFFSET		 	(0)
#define     GC9307_Y_OFFSET		 	(0)

#if (LCD_INTERFACE_8080 == 1)
#if (GC9307_INTERFACE != INTERFACE_8080)
#error "Please choose 8080 interface for GC9307_INTERFACE"
#endif
#elif (LCD_INTERFACE_SPI == 1)
#if ((GC9307_INTERFACE != SPI_3W_I) && (GC9307_INTERFACE != SPI_3W_II) && (GC9307_INTERFACE != SPI_4W_I) && (GC9307_INTERFACE != SPI_4W_II))
#error "Please choose SPI interface for GC9307_INTERFACE"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (GC9307_HEIGHT)
#define     LCD_WIDTH            (GC9307_WIDTH)
#define     LCD_BPP_USE          (GC9307_BPP)
#define     LCD_INTERFACE        (GC9307_INTERFACE)
#define     LCD_X_OFFSET		 (GC9307_X_OFFSET)
#define     LCD_Y_OFFSET		 (GC9307_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_TIME_OF_FRAME    (GC9307_TIME_OF_FRAME) // us
#define     LCD_TE_CYCLE         (GC9307_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (GC9307_TE_WAIT_TIME)  // us

#endif
