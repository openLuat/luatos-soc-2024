#ifndef  LCD_ST77903_
#define  LCD_ST77903_

#define     ST77903_BPP              (16) // 16:565;   18:666   24:888
#define     ST77903_WIDTH            (400)
#define     ST77903_HEIGHT           (400) 
#define     ST77903_FREQ             (8*1024*1024)
#define     ST77903_INTERFACE        (MSPI_4W_II)
#define     BIST_TEST                (0)
#define     ST77903_TIME_OF_FRAME    (149356) // us
#define     ST77903_TE_CYCLE         (16742) // us
#define     ST77903_TE_WAIT_TIME     (623)   // us
#define     ST77903_X_OFFSET		 (0)
#define     ST77903_Y_OFFSET		 (0)

#if (ST77903_INTERFACE == MSPI_4W_II)
#if (LCD_INTERFACE_MSPI != 1)
#error "Please choose MSPI interface in RTE_Device.h"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (ST77903_HEIGHT)
#define     LCD_WIDTH            (ST77903_WIDTH)
#define     DEFAULT_INST         (0xde)
#define     LCD_BPP_USE          (ST77903_BPP)
#define     LCD_INTERFACE        (ST77903_INTERFACE)
#define     LCD_FREQ			 (ST77903_FREQ)
#define     LCD_X_OFFSET		 (ST77903_X_OFFSET)
#define     LCD_Y_OFFSET		 (ST77903_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_TIME_OF_FRAME    (ST77903_TIME_OF_FRAME) // us
#define     LCD_TE_CYCLE         (ST77903_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (ST77903_TE_WAIT_TIME)  // us


#endif
