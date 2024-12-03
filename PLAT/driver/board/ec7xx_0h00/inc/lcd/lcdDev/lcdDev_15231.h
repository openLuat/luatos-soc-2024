#ifndef  LCD_AXS15231_
#define  LCD_AXS15231_

#define     AXS15231_BPP              (16) // 12: 444;   16:565;   18:666
#define     AXS15231_WIDTH            (320)
#define     AXS15231_HEIGHT           (480) 
#define     AXS15231_FREQ             (51*1024*1024)
#define     AXS15231_INTERFACE        (SPI_3W_I)
#define     AXS15231_TIME_OF_FRAME    (149356) // us
#define     AXS15231_TE_CYCLE         (16742) // us
#define     AXS15231_TE_WAIT_TIME     (623)   // us
#define     AXS15231_X_OFFSET		  (0)
#define     AXS15231_Y_OFFSET		  (0)

#if (LCD_INTERFACE_8080 == 1)
#if (AXS15231_INTERFACE != INTERFACE_8080)
#error "Please choose 8080 interface for AXS15231_INTERFACE"
#endif
#elif (LCD_INTERFACE_SPI == 1)
#if ((AXS15231_INTERFACE != SPI_3W_I) && (AXS15231_INTERFACE != SPI_3W_II) && (AXS15231_INTERFACE != SPI_4W_I) && (AXS15231_INTERFACE != SPI_4W_II))
#error "Please choose SPI interface for AXS15231_INTERFACE"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (AXS15231_HEIGHT)
#define     LCD_WIDTH            (AXS15231_WIDTH)
#define     LCD_BPP_USE          (AXS15231_BPP)
#define     LCD_INTERFACE        (AXS15231_INTERFACE)
#define     LCD_FREQ			 (AXS15231_FREQ)
#define     LCD_X_OFFSET		 (AXS15231_X_OFFSET)
#define     LCD_Y_OFFSET		 (AXS15231_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_TIME_OF_FRAME    (AXS15231_TIME_OF_FRAME) // us
#define     LCD_TE_CYCLE         (AXS15231_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (AXS15231_TE_WAIT_TIME)  // us

#endif
