#ifndef  _LCD_COMM_H
#define  _LCD_COMM_H
#ifdef __cplusplus
extern "C" {
#endif
#include <stdio.h>
#include <string.h>
#include "ec7xx.h"
#include "bsp.h"
#include "lspi.h"

#ifdef LCD_RST_GPIO_PIN
#define LCD_RST_LOW    do {GPIO_pinWrite(LCD_RST_GPIO_INSTANCE, 1 << LCD_RST_GPIO_PIN, 0);}while(0)
#define LCD_RST_HIGH   do {GPIO_pinWrite(LCD_RST_GPIO_INSTANCE, 1 << LCD_RST_GPIO_PIN, 1 << LCD_RST_GPIO_PIN);}while(0)
#endif

#define LCD_BK_PWM_VAL_MAX                      800
#define LCD_BK_PWM_VAL_DEF                      100
#define LCD_BK_PWM_VAL_MIM                      4  //0.5%
// The sequences of pins in this enum should be fixed
typedef enum
{
    LCD_RST_PIN,            
    LCD_BACK_LIGHT_PIN,
    LCD_DS_PIN,
    LCD_CS_PIN,
    LCD_GPIO_PIN_NUM,
}lcdGpioPinType_e;

typedef enum
{
    SPI_3W_I,            
    SPI_3W_II,
    SPI_4W_I,
    SPI_4W_II,
}lcdInterfaceType_e;

typedef enum
{
    DIS_FMT_RGB444 = 0,            
    DIS_FMT_RGB565,
    DIS_FMT_RGB666,
    DIS_FMT_GRAY_2,
    DIS_FMT_GRAY_1,
}lcdcolorModeOut_e;

typedef struct
{
    uint16_t            id;             ///< every id points to a series of gpio pins
    lcdGpioPinType_e    lcdGpioPinType;
}lcdGpioPinInfo_t;

typedef struct _lcdDev_t lcdDev_t;

void mDelay(uint32_t mDelay);
void lcdWriteCmd(uint8_t cmd);
void lcdWriteData(uint8_t data);
void lcdGpioInit(lcdDev_t *lcd);
void lcdInterfaceType(lcdInterfaceType_e type);
void lcdColorMode(lcdcolorModeOut_e ModeOut);
void lspiCmdSet(uint8_t cmd, uint8_t reg);
void lspiCmdSend(uint8_t cmd, uint8_t *data, uint8_t num);
void lspiReadReg(uint8_t addr,uint32_t *data,uint16_t num);
void lspiReadRam(uint32_t *data,uint32_t num);
void lcdbkInit(void);
void lcdBackLightOn(bool lightOn);
int16_t lcdBackLightVal(void);
int16_t lcdBackLightLevel(int16_t level);
int16_t lcdBackLightUp(int16_t vol);
int16_t lcdBackLightDown(int16_t vol);

uint32_t millis(void);
typedef void (*lspiCbEvent_fn)(void);  
    
typedef void (*lcd_dma_cb)(uint32_t event);
typedef void (*lcd_usp_cb)(uint32_t event);

int dmaInit(lcd_dma_cb cb);
int dmaStartStop(bool start);
int lcdSpiInit(uint32_t Height,uint32_t Width,lcd_usp_cb cb);
int spidmaTrans(lcdDev_t* lcd,void *sourceAddress, uint32_t totalLength);
#ifdef FEATURE_MES_LCD_ENABLE
void measure_init(void);
uint32_t measure_time(uint32_t *tick);
uint32_t measure_execution(void (*func)(va_list), ...);
#endif
#ifdef __cplusplus
}
#endif
#endif /* LCDCOMM_H */

