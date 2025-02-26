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

#define DMA_BULK_NUM                (1023*8)
#define DMA_DESC_MAX                (160) // RGB888 man need big desc chain


void lcdWriteCmd(uint8_t cmd);
void lcdWriteData(uint8_t data);
void lcdInterfaceType(uint8_t type);
void lspiCmdSend(uint8_t cmd, uint8_t *data, uint8_t num);
void lspiReadReg(uint8_t addr,uint8_t *data,uint16_t num, uint8_t dummyCycleLen);
void lspiReadRam(uint32_t *data,uint32_t num);
void lcdDrvDelay(uint32_t ms);
void lspiFifoWrite(uint32_t data);

#if (BK_USE_PWM == 1)
uint32_t millis(void);
uint8_t  lcdPwmBkLevel(uint8_t level);
#endif

#if (BK_USE_GPIO == 1)
void     lcdGpioBkLevel(uint8_t level);
#endif

typedef void (*lcdDmaCb)(uint32_t event);
typedef void (*lcdUspCb)();

int     dmaInit(lcdDmaCb cb);
void    lcdRst(uint32_t highUs, uint32_t lowUs);
void    dmaStartStop(bool start);
int     lspiDefaultCfg(lcdDrvFunc_t *lcd, lcdUspCb cb, uint32_t freq, uint8_t bpp);
int     lcdDmaTrans(lcdDrvFunc_t *lcd, void *sourceAddress, uint32_t totalLength);
void    lcdMspiSet(uint8_t enable, uint8_t addrLane, uint8_t dataLane, uint8_t instruction);
void    lcdMspiHsyncSet(uint8_t hsyncAddr, uint8_t hsyncInst, uint16_t vbpNum, uint16_t vfpNum);
void    lcdMspiVsyncSet(uint8_t vsyncEnable, uint8_t vsyncInst, uint8_t lspiDiv);
void    lcdCsnHighCycleMin(uint8_t lspiDiv);
void    lcdDmaLoop(lcdDrvFunc_t *lcd, void *sourceAddress, uint32_t totalLength, uint32_t dmaTrunkLen);


#ifdef __cplusplus
}
#endif
#endif /* LCDCOMM_H */

