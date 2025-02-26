#ifndef  _LCD_DRV_H
#define  _LCD_DRV_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include "ec7xx.h"
#include "bsp.h"
#include "lspi.h"
#include "disFormat.h"

#define LCD_COLOR565(r, g, b)       (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define RED                     (0x001f)
#define GREEN                   (0x07e0)
#define BLUE                    (0xf800)
#define WHITE                   (0xffff)
#define BLACK                   (0x0000)
#define YELLOW                  (0xffe0)
#define PURPLE                  (0x8010)
#define GOLDEN                  (0xFEA0)

#define SPI_3W_I                (0)
#define SPI_3W_II               (1)
#define SPI_4W_I                (2)
#define SPI_4W_II               (3)
#define MSPI_4W_II              (4)
#define INTERFACE_8080          (5)


#define LCD_RST_LOW    do {GPIO_pinWrite(LSPI_RST_GPIO_INSTANCE, 1 << LSPI_RST_GPIO_PIN, 0);}while(0)
#define LCD_RST_HIGH   do {GPIO_pinWrite(LSPI_RST_GPIO_INSTANCE, 1 << LSPI_RST_GPIO_PIN, 1 << LSPI_RST_GPIO_PIN);}while(0)



typedef struct
{
    uint8_t cmd;
    uint8_t len;
    uint8_t data[32];
}initLine_t;


typedef enum
{
    stopPreview     = 0,     
    startPreview    = 1,
}camPreviewStartStop_e;

typedef enum
{
    cbForCam        = 0,     
    cbForFill       = 1,
}uspCbRole_e;

typedef enum
{
	TE_RISE_EDGE	= 0,
	TE_FALL_EDGE	= 1,
}teEdgeSel_e;


typedef enum 
{
    /* @---> X
       |
       Y
    */
    DIS_DIR_LRTB,   /**< From left to right then from top to bottom, this consider as the original direction of the screen */

    /*  Y
        |
        @---> X
    */
    DIS_DIR_LRBT,   /**< From left to right then from bottom to top */

    /* X <---@
             |
             Y
    */
    DIS_DIR_RLTB,   /**< From right to left then from top to bottom */

    /*       Y
             |
       X <---@
    */
    DIS_DIR_RLBT,   /**< From right to left then from bottom to top */

    /* @---> Y
       |
       X
    */
    DIS_DIR_TBLR,   /**< From top to bottom then from left to right */

    /*  X
        |
        @---> Y
    */
    DIS_DIR_BTLR,   /**< From bottom to top then from left to right */

    /* Y <---@
             |
             X
    */
    DIS_DIR_TBRL,   /**< From top to bottom then from right to left */

    /*       X
             |
       Y <---@
    */
    DIS_DIR_BTRL,   /**< From bottom to top then from right to left */

    DIS_DIR_MAX,
    /* Another way to represent rotation with 3 bit*/
    DIS_MIRROR_X = 0x40, /**< Mirror X-axis */
    DIS_MIRROR_Y = 0x20, /**< Mirror Y-axis */
    DIS_SWAP_XY  = 0x80, /**< Swap XY axis */
} DisDirection_e;

typedef enum 
{
	LCD_POWER_OFF = 0,
	LCD_POWER_ON  = 1
}lcdPowerOnOff_e;

typedef enum
{
	CAM_PREVIEW_SET_AUTO 	= 0,
	CAM_PREVIEW_SET_MANUAL 	= 1
}lcdPreviewModeSel_e;

typedef struct
{
	uint16_t 			rowScaleFrac;
	uint16_t 			colScaleFrac;
	uint16_t 			tailorLeft;
	uint16_t 			tailorRight;
	uint16_t 			tailorTop;
	uint16_t 			tailorBottom;
}lcdPreviewManulItem_t;

typedef enum
{
	INT_PER_1_FRAME 	= 0,
	INT_PER_1_2_FRAME 	= 1,
	INT_PER_1_4_FRAME 	= 2,
	INT_PER_1_8_FRAME 	= 3,
	INT_PER_1_16_FRAME 	= 4,
}lcdRamWrIntSel_e;

typedef struct
{
	lcdPreviewModeSel_e 	previewModeSel;
	lcdPreviewManulItem_t	previewManulSet;

	uint8_t					ramWrIntEn;
	lcdRamWrIntSel_e		ramWrIntSel;
}lcdIoCtrl_t;

typedef struct _lcdDrvFunc_t lcdDrvFunc_t;
typedef void (*lspiErrCb)(uint32_t stats);

typedef struct _lcdDrvFunc_t
{
    uint32_t    id;
    uint16_t    width;
    uint16_t    height;
    uint32_t    freq;
    uint8_t     bpp;    
    initLine_t  *initRegTbl;
    uint32_t    initRegTblLen;
    uint8_t     dir; // vertical: 0;     horizontal: 1;  

    int         (*init)                 (lcdDrvFunc_t *lcd, void* uspCb, void* dmaCb, uint32_t freq, uint8_t bpp);
    void        (*drawPoint)            (lcdDrvFunc_t *lcd, uint16_t x, uint16_t y, uint32_t dataWrite);
    uint32_t    (*setWindow)            (lcdDrvFunc_t *lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey);
    int         (*fill)                 (lcdDrvFunc_t *lcd, uint32_t fillLen, uint8_t *buf);
    void        (*backLight)            (lcdDrvFunc_t *lcd, uint8_t  level);
    void        (*powerOnOff)           (lcdDrvFunc_t *lcd, lcdPowerOnOff_e  onoff);
    void        (*startStop)            (lcdDrvFunc_t *lcd, bool startOrStop);
    void        (*startStopPreview)     (lcdDrvFunc_t *lcd, camPreviewStartStop_e previewStartStop);
    void        (*uspIrq4CamCb)         (lcdDrvFunc_t *lcd);
    void        (*uspIrq4FillCb)        (lcdDrvFunc_t *lcd);
    void        (*registerUspIrqCb)     (lcdDrvFunc_t *lcd, uspCbRole_e who);
    void        (*unregisterUspIrqCb)   (lcdDrvFunc_t *lcd, uspCbRole_e who);
    int         (*direction)            (lcdDrvFunc_t *lcd, DisDirection_e dir);
    int         (*close)                (lcdDrvFunc_t *lcd);
}lcdDrvFunc_t;


#if (LCD_ST7789_ENABLE == 1)
#include "lcdDev_7789.h"
extern lcdDrvFunc_t st7789Drv;
#elif (LCD_SH8601_ENABLE == 1)
#include "lcdDev_8601.h"
extern lcdDrvFunc_t sh8601Drv;
#elif (LCD_ST7571_ENABLE == 1)
#include "lcdDev_7571.h"
extern lcdDrvFunc_t st7571Drv;
#elif (LCD_ST7567_ENABLE == 1)
#include "lcdDev_7567.h"
extern lcdDrvFunc_t st7567Drv;
#elif (LCD_ST77903_ENABLE == 1)
#include "lcdDev_77903.h"
extern lcdDrvFunc_t st77903Drv;
#elif (LCD_GC9307_ENABLE == 1)
#include "lcdDev_9307.h"
extern lcdDrvFunc_t gc9307Drv;
#elif (LCD_AXS15231_ENABLE == 1)
#include "lcdDev_15231.h"
extern lcdDrvFunc_t axs15231Drv;
#elif (LCD_CO5300_ENABLE == 1)
#include "lcdDev_5300.h"
extern lcdDrvFunc_t co5300Drv;

#endif


void            lcdRegInit(uint32_t id);
lcdDrvFunc_t*   lcdOpen(uint32_t id, void* uspCb, void* dmaCb);
int             lcdClose(lcdDrvFunc_t *pdrv);
void 			lcdIoInit(bool isAonIO);
void 			lcdRegisterSlp1Cb(lcdSlp1Cb_fn cb);
int             lcdDirection(lcdDrvFunc_t *pdrv, DisDirection_e dir);
int             lcdFill(lcdDrvFunc_t *pdrv, uint32_t fillLen, uint8_t* buf);
void            lcdDrawPoint(lcdDrvFunc_t *pdrv, uint16_t x, uint16_t y, uint32_t dataWrite);
void            camPreview(lcdDrvFunc_t *pdrv, camPreviewStartStop_e previewStartStop);
void            lcdBackLight(lcdDrvFunc_t *pdrv, uint8_t level);
uint32_t        lcdSetWindow(lcdDrvFunc_t *pdrv, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);
void            lspiRstAndClearFifo();
void            lspiRegisterErrStatsCb(lspiErrCb errCb);
void            lspiCheckErrStats();
void 			imageRotateColor(uint8_t* src, uint32_t width, uint32_t height, uint8_t* dst, uint8_t bpp);
void 			imageRotateGray(uint8_t* src, uint32_t width, uint32_t height, uint8_t* dst);
void 			yuv422ToRgb565_2(const void* inbuf, void* outbuf, int width, int height);
void 			lcdIoCtrl(lcdDrvFunc_t *lcd, lcdIoCtrl_t ioCtrl);
void 			lcdConfigReg(lcdDrvFunc_t *lcd, uint8_t cmd, uint8_t *data, uint8_t dataLen);
#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
void    calTe(uint32_t totalBytes, uint16_t sy);
#else // 719
void calTe(teEdgeSel_e teEdge, uint16_t xs, uint16_t ys, uint16_t xe, uint16_t ye);
#endif

#ifdef __cplusplus
}
#endif
#endif
