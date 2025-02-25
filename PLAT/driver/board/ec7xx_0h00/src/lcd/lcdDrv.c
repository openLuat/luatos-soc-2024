#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "bsp_custom.h"
#include "lcdDrv.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif
#include "sctdef.h"
#include "slpman.h"

extern void lcdDrvDelay(uint32_t us);
extern void lspiCmdSend(uint8_t cmd,uint8_t *data,uint8_t allbytes);
AP_PLAT_COMMON_DATA static lspiDrvInterface_t *lcdDrv1  = &lspiDrvInterface2; 
AP_PLAT_COMMON_BSS static lspiErrCb lspiErrStatsFunc;
AP_PLAT_COMMON_BSS lcdDrvFunc_t* lcdDevHandle;
AP_PLAT_COMMON_BSS lcdIoCtrl_t lcdIoCtrlParam;


AP_PLAT_COMMON_DATA lcdDrvFunc_t* lcdDrvList[] = 
{
#if (LCD_ST7789_ENABLE == 1)
    &st7789Drv,
#elif (LCD_ST77903_ENABLE == 1)
    &st77903Drv,
#elif (LCD_AXS15231_ENABLE == 1)
    &axs15231Drv,
#elif (LCD_CO5300_ENABLE == 1)
    &co5300Drv,    
#endif
};

lcdDrvFunc_t* lcdOpen(uint32_t id, void* uspCb, void* dmaCb)
{       
    lcdDrvFunc_t *pDrvFunc = NULL;

    uint8_t drvListLen = sizeof(lcdDrvList) / sizeof(lcdDrvFunc_t*);
    if (drvListLen > 0)
    {
        for (uint8_t i = 0; i < drvListLen; i++) 
        {
            if (id == lcdDrvList[i]->id)
            {
                pDrvFunc = lcdDrvList[i];
                pDrvFunc->init(pDrvFunc, uspCb, dmaCb, pDrvFunc->freq, pDrvFunc->bpp); 
				lcdDevHandle = pDrvFunc;
                return pDrvFunc;
            }
        }
    }

    return NULL;
}

void lcdRegisterSlp1Cb(lcdSlp1Cb_fn cb)
{
    lcdSlp1CbFn = cb;
}


void lcdIoInit(bool isAonIO)
{   
    if (isAonIO)
    {
        slpManAONIOPowerOn();
    }
    
    PadConfig_t config;
    GpioPinConfig_t gpioCfg;
    PAD_getDefaultConfig(&config);
    
    // 1. rst pin init
    config.mux = LSPI_RST_PAD_ALT_FUNC;
    PAD_setPinConfig(LSPI_RST_GPIO_ADDR, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LSPI_RST_GPIO_INSTANCE, LSPI_RST_GPIO_PIN, &gpioCfg);

    // 2. backLight init
#if (BK_USE_GPIO == 1)
    config.mux = LCD_BK_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_BK_PAD_INDEX, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 0;
    GPIO_pinConfig(LCD_BK_GPIO_INSTANCE, LCD_BK_GPIO_PIN, &gpioCfg);
#elif (BK_USE_PWM == 1)
    lcdPwmBkInit();
#endif

	// 3. ldo pin init
#if (ENABLE_LDO == 1)
    config.mux = LCD_EN_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_EN_PAD_INDEX, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(LCD_EN_GPIO_INSTANCE, LCD_EN_GPIO_PIN, &gpioCfg);
#endif    

    // 4. te init
#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
    // 4.1 718 te init
#else
    // 4.2 719 te init
    config.mux = LCD_TE_PAD_ALT_FUNC;
    PAD_setPinConfig(LCD_TE_PAD_INDEX, &config);
#endif

    // test fill one frame need how much time
    #if 0
    config.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(27, &config);
    gpioCfg.pinDirection = GPIO_DIRECTION_OUTPUT;
    gpioCfg.misc.initOutput = 1;
    GPIO_pinConfig(0, 12, &gpioCfg);
    #endif
}


void lcdRegInit(uint32_t id)
{
    initLine_t *init = NULL;
    uint8_t drvListLen = sizeof(lcdDrvList) / sizeof(lcdDrvFunc_t*);
       
    if (drvListLen > 0)
    {
        for (uint8_t i = 0; i < drvListLen; i++) 
        {
            if (id == lcdDrvList[i]->id)
            {
                init = lcdDrvList[i]->initRegTbl;

                for (int j = 0; j < lcdDrvList[i]->initRegTblLen; j++)
                {
                    if (init->cmd == 0xff)
                    {
                        lcdDrvDelay(init->data[0]);
                    }
                    else
                    {
                        lspiCmdSend(init->cmd, init->data, init->len);
                    }
                    
                    init++;
                }
            }
        }
    }
}

int lcdClose(lcdDrvFunc_t *pdev)
{
    if (pdev == NULL) return 1;
    
    pdev->close(pdev);
    pdev = NULL;
    
    return 0;
}


int lcdDirection(lcdDrvFunc_t *lcd, DisDirection_e dir)
{
    if (lcd == NULL)
    {
        return -1;
    }

   return lcd->direction(lcd,dir); 
}

void camPreview(lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop)
{
    if (lcd == NULL)
    {
        return;
    }

    lcd->startStopPreview(lcd, previewStartStop);
}


void lcdDrawPoint(lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite)
{
    if (lcd == NULL)
    {
        return;
    }

    return lcd->drawPoint(lcd, x, y, dataWrite);
}



/**
 * \brief Fills an area of the LCD with the data from a buffer.
 *
 * This function fills an area of the LCD specified by the start and end coordinates
 * with the data from a buffer. The DMA trunk length parameter is not used in this
 * function and is mentioned in the comment as not being used.
 *
 * \param lcd A pointer to the LCD device structure.
 * \param sx The starting x-coordinate for the fill operation.
 * \param sy The starting y-coordinate for the fill operation.
 * \param ex The ending x-coordinate for the fill operation.
 * \param ey The ending y-coordinate for the fill operation.
 * \param buf A pointer to the buffer containing the data to be filled into the LCD.
 * \param dmaTrunkLength The length of the DMA trunk, which is not used in this function.
 *
 * \return Returns the result of the fill operation from the LCD driver.
 *         Returns 0xffffffff if the `lcd` pointer is `NULL`.
 */
uint32_t lcdSetWindow(lcdDrvFunc_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey)
{
    if (lcd == NULL)
    {
        return 0xffffffff;
    }

    return lcd->setWindow(lcd, sx, sy, ex, ey);
}
 
int lcdFill(lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t* buf)
{
    if (lcd == NULL)
    {
        return -1;
    }

    return lcd->fill(lcd, fillLen, buf);
}

// level: 0 ~ 100
void lcdBackLight(lcdDrvFunc_t *lcd, uint8_t level)
{
    if (lcd == NULL)
    {
        return;
    }
    return lcd->backLight(lcd, level);
}

void lcdPowerOnOff(lcdDrvFunc_t *lcd, lcdPowerOnOff_e onoff)
{
    if (lcd == NULL)
    {
        return;
    }
    return lcd->powerOnOff(lcd, onoff);
}


void lspiRstAndClearFifo()
{
    lspiCtrl.enable = 0;
    lcdDrv1->ctrl(LSPI_CTRL_CTRL, 0);
    
    GPR_swReset(RST_FCLK_USP2);
    LSPI2->DMACTL |= 1<<25; // clear tx fifo
    LSPI2->STAS   |= 1;
    
    lspiCtrl.enable = 1;
    lcdDrv1->ctrl(LSPI_CTRL_CTRL, 0);
}

void lspiRegisterErrStatsCb(lspiErrCb errCb)
{
    lspiErrStatsFunc = errCb;
}

void lspiCheckErrStats()
{
    if (!lspiErrStatsFunc)
    {
        return;
    }

    // check lspi error status and give cb to user
    uint32_t status = LSPI2->STAS;

    if ( (status | ICL_STATS_TX_UNDERRUN_RUN_Msk) ||
         (status | ICL_STATS_TX_DMA_ERR_Msk) ||
         (status | ICL_STATS_RX_OVERFLOW_Msk) ||
         (status | ICL_STATS_RX_DMA_ERR_Msk) ||
         (status | ICL_STATS_RX_FIFO_TIMEOUT_Msk) ||
         (status | ICL_STATS_FS_ERR_Msk) ||
         (status | ICL_STATS_CSPI_BUS_TIMEOUT_Msk) ||
         (status | ICL_STATS_RX_FIFO_TIMEOUT_Msk)
        ) 
    {   
        lspiErrStatsFunc(status);
    }        

    return;    
}

void imageRotateColor(uint8_t* src, uint32_t width, uint32_t height, uint8_t* dst, uint8_t bpp)
{
	int i, j, bytePerLine, tmp, i2;

	bytePerLine = width * bpp / 8;
	for (i = 0; i < width; i++)
	{
		i2 = i*bpp / 8;
		for (j = 0; j < height; j++)
		{
			tmp 	= (height - 1 - j) * bytePerLine + i2;
			*dst++ 	= src[tmp];
			*dst++ 	= src[tmp+1];
		}
	}
}

void imageRotateGray(uint8_t* src, uint32_t width, uint32_t height, uint8_t* dst)
{
	for (int i= 0; i < width; i++)
	{
		for (int j = 0; j < height; j++)
		{
			dst[i*height + j] = src[(height-1-j) * width + i];
		}
	}
}

#define RANGE_LIMIT(x) (x > 255 ? 255 : (x < 0 ? 0 : x))
void yuv422ToRgb565_2(const void* inbuf, void* outbuf, int width, int height)
{
	int rows, cols;
	int y, u, v, r, g, b;
	unsigned char *yuv_buf;
	unsigned short *rgb_buf;
	int y_pos,u_pos,v_pos;

	yuv_buf = (unsigned char *)inbuf;
	rgb_buf = (unsigned short *)outbuf;

	y_pos = 0;
	u_pos = 1;
	v_pos = 3;

	for (rows = 0; rows < height; rows++)
	{
		for (cols = 0; cols < width; cols++) 
		{
			y = yuv_buf[y_pos];
			u = yuv_buf[u_pos] - 128;
			v = yuv_buf[v_pos] - 128;

			// R = Y + 1.402*(V-128)
			// G = Y - 0.34414*(U-128)
			// B = Y + 1.772*(U-128)
			r = RANGE_LIMIT(y + v + ((v * 103) >> 8));
			g = RANGE_LIMIT(y - ((u * 88) >> 8) - ((v * 183) >> 8));
			b = RANGE_LIMIT(y + u + ((u * 198) >> 8));

			*rgb_buf++ = (((r & 0xf8) << 8) | ((g & 0xfc) << 3) | ((b & 0xf8) >> 3));

			y_pos += 2;

			if (cols & 0x01) 
			{
				u_pos += 4;
				v_pos += 4;
			}
		}
	}
}

#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
void calTe(uint32_t totalBytes, uint16_t sy)
{   
    //uint32_t timeStampApp= 0;
    uint32_t teRunTimeMs = 0;
    uint16_t yte = 0;
    //uint32_t controllerBytesPerMs = 480*320*2/28;
    uint16_t waitTimeMs = 0;
    
    // cal te
#if 0
    timeStampApp = TIMER_getCount(0);
    
    if (timeStampApp < timeStampTe)
    {
        teRunTimeMs = (26000 - timeStampTe + timeStampApp) / 1000;
    }
    else
    {
        teRunTimeMs = (timeStampApp - timeStampTe) / 1000;
    }
#else
#if (BK_USE_PWM == 1)
        teRunTimeMs = millis();
#endif        
#endif
    if (teRunTimeMs > 16)
    {
        teRunTimeMs = 0;
    }
    
    yte = teRunTimeMs * 40 - 1;

    

    if (teRunTimeMs > 6)
    {
        if (sy > yte)
        {
            waitTimeMs = (sy-yte)/40;
            if (waitTimeMs == 0)
            {
                waitTimeMs = 1; // at least 1ms
            }
            
#ifdef FEATURE_OS_ENABLE
            osDelay(waitTimeMs);
#endif            
            dmaStartStop(true);
        }
        else
        {
            if (sy >= (480/2))
            {
                dmaStartStop(true);
            }
            else
            {
                waitTimeMs = 16 - teRunTimeMs + sy/40; // +1
#ifdef FEATURE_OS_ENABLE                
                osDelay(waitTimeMs);
#endif                
                dmaStartStop(true);
            }
        }
    }
    else
    {
        if (sy > yte)
        {
            waitTimeMs = (sy-yte)/40;           
            if (waitTimeMs == 0)
            {
                waitTimeMs = 1; // at least 1ms
            }

#ifdef FEATURE_OS_ENABLE            
            osDelay(waitTimeMs);
#endif            
            dmaStartStop(true);
        }
        else
        {
            if (totalBytes > ((16-teRunTimeMs+12)*40*320*2))
            {
                // must wait te irq
                //osDelay(16-teRunTimeMs);
                //osEventFlagsWait(lcdEvtHandle, 0x4, osFlagsWaitAll, osWaitForever); // in te gpio isr release
                dmaStartStop(true);
            }
            else
            {
                dmaStartStop(true);
            }
        }
    }
}

#else // chip 719
void calTe(teEdgeSel_e teEdge, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye)
{
	uint32_t tlspi=0, te=0, tw=0, tlcd=0, ta=0, tgaps=0, tgape=0, pos0=0, pos1=0;
	xs += LCD_X_OFFSET;
	xe += LCD_X_OFFSET;
	ys += LCD_Y_OFFSET;
	ye += LCD_Y_OFFSET;

#if (LCD_INTERFACE_SPI == 1)	
	tlspi = 24 * (LCD_FREQ / (51*1024*1024)) * (LCD_PIXEL / (320*240)) * 1000; // us
#elif (LCD_INTERFACE_MSPI == 1)
	if (LCD_FREQ == 51*1024*1024)
	{
		tlspi = 16600; // 51M, 16.6ms
	}
	else if (LCD_FREQ == 6*1024*1024)
	{
		tlspi = 148000; // 6M, 148ms
	}
#elif (LCD_INTERFACE_8080 == 1)	

#endif

	te 	  = LCD_TE_CYCLE;
	tw 	  = LCD_TE_WAIT_TIME;
	tlcd  = te - tw;
	ta	  = (ye - ys + 1) * (xe - xs + 1) / LCD_PIXEL * tlspi ;
	tgaps = ys * tlcd / LCD_HEIGHT;
	tgape = ye * tlcd / LCD_HEIGHT;

	if (ta < (tgape - tgaps))
	{
		pos1 = tgaps;
		pos0 = tgape - ta;
	}
	else if (((tgape - tgaps) <= ta) && (ta <= tgape))
	{
		pos1 = tgape - ta;
		pos0 = tgaps;
	}
	else if ((tgape < ta) && (ta <= tgape + tw))
	{
		pos1 = 0;
		pos0 = tgaps;
	}
	else if (((tgape + tw) < ta) && (ta <= (tgape + tw + tlcd - tgaps)))
	{
		pos0 = tgaps;
		pos1 = tlcd + tgape + tw - ta;
	}
	else if	(ta > (tgape + tw + tlcd - tgaps))
	{
		EC_ASSERT(0,0,0,0);
	}

	lspiTeParam0.lspiTeEn 		= 1;
	lspiTeParam0.lspiTeEdgeSel 	= teEdge;
	lspiTeParam0.lspiTePos0     = pos0*LCD_FREQ/1000000;
	lspiTeParam1.lspiTePos1     = pos1*LCD_FREQ/1000000;
    lcdDrv1->ctrl(LSPI_TE_CTRL, 0);
	
}
#endif

void lcdIoCtrl(lcdDrvFunc_t *lcd, lcdIoCtrl_t ioCtrl)
{
    if (lcd == NULL)
    {
        return;
    }

	// 1. preview window ctrl
	lcdIoCtrlParam = ioCtrl;

    
#if defined TYPE_EC718M
	// 2. int ctrl
	lspiIntCtrl.ramWrIntCtrlEn   = ioCtrl.ramWrIntEn;
	lspiIntCtrl.ramWrIntCtrl     = ioCtrl.ramWrIntSel;
    //lcdDrv1->ctrl(LSPI_CTRL_INT_CTRL, 0);
#endif    
}

void lcdConfigReg(lcdDrvFunc_t *lcd, uint8_t cmd, uint8_t *data, uint8_t dataLen)
{
	lspiCmdSend(cmd, data, dataLen);
}

