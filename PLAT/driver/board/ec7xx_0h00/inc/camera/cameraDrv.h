#ifndef __CAMERA_DRV_H__
#define __CAMERA_DRV_H__

#include "cspi.h"
#include "gc032A.h"
#include "gc6153.h"

/**
  \addtogroup cam_interface_gr
  \{
 */

typedef struct
{
    uint8_t regAddr;                            ///< Sensor I2C register address
    uint8_t regVal;                             ///< Sensor I2C register value
}camI2cCfg_t;

typedef enum
{
    CAM_LSB_MODE    = 0,                            ///< Little endian
    CAM_MSB_MODE    = 1,                            ///< Big endian
}endianMode_e;

typedef enum
{
    WIRE_1      = 0,                            ///< 1 wire
    WIRE_2      = 1,                            ///< 2 wire
}wireNum_e;


typedef enum
{
    SEQ_0       = 0,                            ///< rxd[0] 6 4 2 0
                                                ///< rxd[1] 7 5 3 1    
    SEQ_1       = 1,                            ///< rxd[1] 6 4 2 0
                                                ///< rxd[0] 7 5 3 1
}rxSeq_e;

typedef enum
{
	CSPI_0		= 0,
	CSPI_1		= 1,
}cspiInstance_e;

typedef enum
{
	CSPI_START	 = 1,			///< cspi enable
	CSPI_STOP    = 0,			///< Cspi disable
}cspiStartStop_e;

typedef enum
{
	CSPI_INT_ENABLE	    = 1,		///< cspi interrupt enable
	CSPI_INT_DISABLE    = 0,		///< Cspi interrupt disable
}cspiIntEnable_e;


typedef struct
{
    endianMode_e    endianMode;                 ///< Endian mode
    wireNum_e       wireNum;                    ///< Wire numbers
    rxSeq_e         rxSeq;                      ///< Bit sequence in 2 wire mode
    uint8_t 		cpol;
    uint8_t			cpha;
    uint8_t         ddrMode;
    uint8_t         wordIdSeq;
	uint8_t         yOnly;
    uint8_t         rowScaleRatio;
    uint8_t         colScaleRatio;
    uint8_t         scaleBytes;
    uint8_t         dummyAllowed;
}camParamCfg_t;

typedef struct
{
	uint32_t enableForCamera	: 1; // 0: isn't work for camera now; 1: is working for camera now
	uint32_t enableForUsr		: 1; // 0: isn't ready for usr;       1: is ready for usr
	uint32_t workingForUsr		: 1; // 0: usr has used this buf;     1: usr is using this buf
	uint32_t camErrCnt			: 3; // record camera err count
	uint32_t rsvd				: 26;
	uint32_t timeStamp;
	uint8_t  data[320*240*2];   // can be configed
}CameraBuf_t;



typedef void (*camCbEvent_fn) (uint32_t event); ///< Camera callback event.
typedef void (*camIrq_fn)(void); 					///< Camera irq
typedef void (*camErrCb)(uint32_t stats);


void camInit(void* dataAddr, cspiCbEvent_fn uspCb, void* dmaCb, camErrCb errCb);

/**
  \brief Receive the picture has been taken.
  \param[out] dataIn     The buffer which is used to store the picture.
  \return              
*/
void camRecv(uint8_t * dataIn);

/**
  \brief Init sensor's registers.
  \return              
*/
void camRegCfg(void);

/**
  \brief Write some parameters into the sensor.
  \param[in] regInfo     Sensor I2C addr and value.
  \return              
*/
void camWriteReg(camI2cCfg_t* regInfo);

/**
  \brief Read from the sensor's I2C address.
  \param[in] regAddr     Sensor's I2C register address.
  \return              
*/
uint8_t camReadReg(uint8_t regAddr);

/**
  \brief Start or stop Camera controller.
  \param[in] startStop     If true, start camera controller. If false, stop camera controller.
  \return
*/
void camStartStop(cspiStartStop_e startStop);

/**
  \brief Register irq for cspi.
  \param[in] instance     cspi0 or cspi1.
  \param[in] irqCb        irq cb.
  \return              
*/
void camRegisterIRQ(cspiInstance_e instance, camIrq_fn irqCb);

uint32_t camGetCspiStats();

/**
  \brief Enable or disable interrupt of cspi.
  \param[in] intEnable     interrupt enable or not.
  \return              
*/
void cspiStartIntEnable(cspiIntEnable_e intEnable);
void cspiEndIntEnable(cspiIntEnable_e endIntEnable);
void cspi2LspiEnable(uint8_t enable);
int camCheckErrStats();
#if (ENABLE_CAMERA_LDO == 1)
void camPowerOn(uint8_t ioInitVal);
#endif
void camGpioPulseCfg(uint8_t padAddr, uint8_t pinInstance, uint8_t pinNum);
void camGpioPulse(uint8_t pinInstance, uint8_t pinNum, uint32_t pulseDurationUs, uint8_t initialState, bool needLoop);
int camPicTake();
void camPicGive(int index);

/** \} */

#endif
