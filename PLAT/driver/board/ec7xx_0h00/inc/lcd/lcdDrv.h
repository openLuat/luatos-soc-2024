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

#define SUPPORT_LCD_NUM             (3)
#define INSTALL_LCD_NUM             (1)
#define LCD_COLOR565(r, g, b)       (((r & 0xF8) << 8) | ((g & 0xFC) << 3) | ((b & 0xF8) >> 3))

#define RED                     (0x001f)//(0xf800)
#define GREEN                   (0x07e0)
#define BLUE                    (0xf800)//(0x001f)
#define WHITE                   (0xffff)
#define BLACK                   (0x0000)
#define YELLOW                  (0xffe0)
#define PURPLE                  (0x8010)
#define GOLDEN                  (0xFEA0)

typedef void (*lspiCbEvent_fn) (void);            ///< lspi callback event.


typedef struct
{
    uint32_t        pinInstance : 8;
    uint32_t        pinNum      : 8;
    uint32_t        padAddr     : 8;
    uint32_t        func        : 8;
}lcdPinInfo_t;

typedef struct
{
    lcdPinInfo_t    cs;
    lcdPinInfo_t    ds;
    lcdPinInfo_t    rst;
    lcdPinInfo_t    clk;
    lcdPinInfo_t    mosi;
    lcdPinInfo_t    miso;
}spiPin_t;


typedef struct
{
    uint16_t        id;
    uint32_t        width;
    uint32_t        height;
}lcdDrvPra_t;

typedef enum _lcdBusType_e
{
    BUS_LSPI,
    BUS_SPI,
    BUS_I2C,        ///< OLED will use
}lcdBusType_e;

typedef struct
{
    char            *name;          ///< lcd's name used to configure its id, then use id to find its info, including driver function
    uint16_t        id;             ///< every lcd's id should be different, no matter lcd's type is the same or not
}lcdObj_t;

typedef enum
{
    stopPreview     = 0,     
    startPreview    = 1,
}camPreviewStartStop_e;

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

typedef struct _lcdDev_t lcdDev_t;

typedef struct 
{
    uint16_t id;

    int (*init)(lcdDev_t *lcd, void* cb);
    int (*drawPoint)(lcdDev_t* lcd, uint16_t x, uint16_t y, uint32_t dataWrite);
    int (*fill)(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey, uint8_t *buf, uint32_t dmaTrunkLength);
    int (*dump)(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint32_t *buf, uint32_t Length);
    int (*prepareDisplay)(lcdDev_t* lcd, uint16_t sx, uint16_t ex, uint16_t sy, uint16_t ey);
    int (*onoff)(lcdDev_t* lcd, uint8_t sta);
    void (*backLight)(lcdDev_t* lcd, uint8_t sta);   
    void (*startStop)(lcdDev_t* lcd, bool startOrStop);
    void (*clear)(lcdDev_t* lcd, uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength);
    void (*startStopPreview)(lcdDev_t* lcd, camPreviewStartStop_e previewStartStop);
    uint16_t (*check)(lcdDev_t *lcd);
    int (*direction)(lcdDev_t *lcd,DisDirection_e dir);
    int (*close)(lcdDev_t *lcd);
    //void (*startRamWrite)(lcdDev_t* lcd);
    //void (*addrSet)(lcdDev_t *lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey);
}lcdDrvFunc_t;

typedef struct _lcdDev_t
{
    int             handle;         ///< lcd descriptor, every opened lcd will return a handle. So need lcd init first, then open it

    lcdObj_t        *obj;           ///< lcd info    
    lcdDrvPra_t     *pra;           ///< lcd parameters
    lcdDrvFunc_t    *drv;           ///< lcd driver functions

    // the parameters that the driver needs
    uint8_t         dir;            ///< vertical: 0;     horizontal: 1;  
    uint16_t        width;          ///< lcd width
    uint16_t        height;         ///< lcd height

    void            *pri;           ///< private parameters that 1-bit screen or oled will use
}lcdDev_t;

int lcdInit(void* spi_cb);
lcdDev_t *lcdOpen(char* name);
int lcdClose(lcdDev_t *pdev);
uint16_t lcdCheck(lcdDev_t *pdev);
int lcdDirection(lcdDev_t *pdev,DisDirection_e dir);
void lcdClear(lcdDev_t* lcd, uint8_t* buf, uint16_t lcdHeight, uint16_t lcdWidth, uint32_t dmaTrunkLength);
int lcdFill(lcdDev_t* lcd, uint16_t sx, uint16_t sy, uint16_t ex, uint16_t ey, uint8_t* buf, uint32_t dmaTrunkLength);
int lcdDump(lcdDev_t* lcd, uint16_t sx, uint16_t sy, uint32_t* buf, uint32_t Length);
int lcdDrawPoint(lcdDev_t* lcd, uint16_t x, uint16_t y, uint32_t dataWrite);
void camPreview(lcdDev_t *pdev, camPreviewStartStop_e previewStartStop);
void lcdBackLight(lcdDev_t* lcd, uint8_t level);
#ifdef __cplusplus
}
#endif
#endif
