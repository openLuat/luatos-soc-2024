#ifndef  LCD_CO5300_
#define  LCD_CO5300_

#define     CO5300_BPP              (16) // 16:565;   18:666   24:888
#define     CO5300_WIDTH            (460)
#define     CO5300_HEIGHT           (460) 
#define     CO5300_FREQ             (51*1024*1024)
#define     CO5300_INTERFACE        (MSPI_4W_II)
#define     CO5300_TE_CYCLE         (16327) // us
#define     CO5300_TE_WAIT_TIME     (425)   // us
#define     CO5300_X_OFFSET			(0xa)
#define     CO5300_Y_OFFSET			(0)

#if (ST8601_INTERFACE == MSPI_4W_II)
#if (LCD_INTERFACE_MSPI != 1)
#error "Please choose MSPI interface in RTE_Device.h"
#endif
#endif


#undef      LCD_WIDTH
#undef      LCD_HEIGHT
#define     LCD_HEIGHT           (CO5300_HEIGHT)
#define     LCD_WIDTH            (CO5300_WIDTH) 
#define     DEFAULT_INST         (0x2)
#define     LCD_BPP_USE          (CO5300_BPP)
#define     LCD_INTERFACE        (CO5300_INTERFACE)
#define     LCD_FREQ			 (CO5300_FREQ)
#define     LCD_X_OFFSET		 (CO5300_X_OFFSET)
#define     LCD_Y_OFFSET		 (CO5300_Y_OFFSET)

#define     LCD_PIXEL            (LCD_HEIGHT*LCD_WIDTH)
#define     LCD_TE_CYCLE         (CO5300_TE_CYCLE)      // us
#define     LCD_TE_WAIT_TIME     (CO5300_TE_WAIT_TIME)  // us

#endif
