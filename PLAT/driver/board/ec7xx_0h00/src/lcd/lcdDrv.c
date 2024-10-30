#include <stdio.h>
#include <string.h>
#include "bsp.h"
#include "bsp_custom.h"
#include "lcdDrv.h"
#ifdef FEATURE_SUBSYS_SYSLOG_ENABLE
#include "syslog.h"
#endif

extern void lcdDrvDelay(uint32_t us);
extern void lspiCmdSend(uint8_t cmd,uint8_t *data,uint8_t allbytes);
static lspiDrvInterface_t *lcdDrv1  = &lspiDrvInterface2; 
static lspiErrCb lspiErrStatsFunc;

lcdDrvFunc_t* lcdDrvList[] = 
{
#if (LCD_ST7789_ENABLE == 1)
    &st7789Drv,
#elif (LCD_SH8601_ENABLE == 1)
    &sh8601Drv,
#elif (LCD_ST7571_ENABLE == 1)
    &st7571Drv,
#elif (LCD_ST7567_ENABLE == 1)
    &st7567Drv,
#elif (LCD_ST77903_ENABLE == 1)
    &st77903Drv,
#elif (LCD_GC9307_ENABLE == 1)
    &gc9307Drv,
#elif (LCD_AXS15231_ENABLE == 1)
    &axs15231Drv,
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
                return pDrvFunc;
            }
        }
    }

    return NULL;
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




