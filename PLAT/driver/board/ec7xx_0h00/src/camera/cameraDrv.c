#include "cameraDrv.h"
#include "hal_i2c.h"
#include "sctdef.h"

extern cspiDrvInterface_t   cspiDrvInterface0;
extern cspiDrvInterface_t   cspiDrvInterface1;

extern camI2cCfg_t          sp0A39Cfg[];
extern camI2cCfg_t          sp0821Cfg[];
extern camI2cCfg_t          gc6123Cfg[];
extern camI2cCfg_t          gc032ACfg[];
extern camI2cCfg_t          bf30a2Cfg[];
extern cspiCtrl_t           cspiCtrl;
extern cspiBinaryCtrl_t     cspiBinaryCtrl;
extern cspiIntCtrl_t        cspiIntCtrl;
extern cspiDataFmt_t        cspiDataFmt;
extern cspiFrameProcLspi_t  cspiFrameProcLspi;

#define EIGEN_CSPI(n)             ((CSPI_TypeDef *) (MP_USP0_BASE_ADDR + 0x1000*n))
AP_PLAT_COMMON_BSS static camErrCb             	camErrStatsFunc;
AP_PLAT_COMMON_BSS static cspiCbEvent_fn 		userCamUspCb   = NULL;
AP_PLAT_COMMON_BSS static cspiCbEvent_fn		userCamDmaCb   = NULL;
AP_PLAT_COMMON_BSS CameraBuf_t *camBufBak;
void camDmaCb();

#if (CAMERA_ENABLE_GC032A)
 #if (GC032A_2SDR)
    char* regName = "gc032a_2sdr";
 #elif (GC032A_1SDR)
    char* regName = "gc032a_1sdr";
 #elif (GC032A_2DDR)
    char* regName = "gc032a_2ddr";
 #endif
#elif (CAMERA_ENABLE_GC6153)
 #if (GC6153_1SDR)
	char* regName = "gc6153_1sdr";
 #endif  
#endif

AP_PLAT_COMMON_BSS static uint8_t      slaveAddr;
AP_PLAT_COMMON_BSS static uint16_t     regCnt;
AP_PLAT_COMMON_BSS static camI2cCfg_t* regInfo = NULL;

#if (RTE_CSPI1 == 1)
AP_PLAT_COMMON_DATA static cspiDrvInterface_t   *cspiDrv        = &CREATE_SYMBOL(cspiDrvInterface, 1);
#else
AP_PLAT_COMMON_DATA static cspiDrvInterface_t   *cspiDrv        = &CREATE_SYMBOL(cspiDrvInterface, 0);
#endif
extern void delay_us(uint32_t us);

void findRegInfo(char* regName, uint8_t* slaveAddr, uint16_t* regCnt, camI2cCfg_t** regInfo)
{
 	if (strcmp(regName, "gc032a_2sdr") == 0)
    {
        extern camI2cCfg_t gc032A_2sdrRegInfo[];
        *regInfo = gc032A_2sdrRegInfo;
        *slaveAddr = GC032A_I2C_ADDR;
        *regCnt = gc032aGetRegCnt(regName);
    }
    else if (strcmp(regName, "gc032a_1sdr") == 0)
    {
        extern camI2cCfg_t gc032A_1sdrRegInfo[];
        *regInfo = gc032A_1sdrRegInfo;
        *slaveAddr = GC032A_I2C_ADDR;
        *regCnt = gc032aGetRegCnt(regName);
    }
    else if (strcmp(regName, "gc032a_2ddr") == 0)
    {
        extern camI2cCfg_t gc032A_2ddrRegInfo[];
        *regInfo = gc032A_2ddrRegInfo;
        *slaveAddr = GC032A_I2C_ADDR;
        *regCnt = gc032aGetRegCnt(regName);
    }
	else if (strcmp(regName, "gc6153_1sdr") == 0)
    {
        extern camI2cCfg_t gc6153_1sdrRegInfo[];
        *regInfo = gc6153_1sdrRegInfo;
        *slaveAddr = GC6153_I2C_ADDR;
        *regCnt = gc6153GetRegCnt(regName);
    }
}

void camI2cInit()
{
    halI2cInit(true);

    // Backup some info about this sensor
    findRegInfo(regName, &slaveAddr, &regCnt, &regInfo);
}

void camI2cWrite(uint8_t slaveAddr, uint8_t regAddr, uint8_t regData, uint32_t num)
{
    uint8_t tempBuffer[2];

    tempBuffer[0] = regAddr;
    tempBuffer[1] = regData;
   
    uint8_t rxNack = 0;
    halI2cWrite(slaveAddr, tempBuffer, num, &rxNack, true);

    if (rxNack == 1)
    {
        // if fail , write again
        halI2cWrite(slaveAddr, tempBuffer, num, &rxNack, true);
    }
}

uint8_t camI2cRead(uint8_t slaveAddr, uint8_t regAddr)
{
    uint8_t readData;
    halI2cRead(slaveAddr, regAddr, &readData, true);
    return readData;   
}

uint8_t camReadReg(uint8_t regAddr)
{
    uint8_t recvData;

    recvData = camI2cRead(slaveAddr, regAddr);
    return recvData;
}

void camWriteReg(camI2cCfg_t* regInfo)
{
    camI2cWrite(slaveAddr, regInfo->regAddr, regInfo->regVal, 2);
}

void camRegCfg()
{
    //uint8_t dataRead;
    
    camI2cInit();

    // Configure all the registers about this sensor
    for (int i=0; i < regCnt; i++)
    {
        camI2cWrite(slaveAddr, regInfo[i].regAddr, regInfo[i].regVal, 2);
        delay_us(10000); // delay 10ms

#if 0
        dataRead = camI2cRead(slaveAddr, regInfo[i].regAddr);
        printf("reg addr=0x%02x, reg val=0x%02x\n", regInfo[i].regAddr, dataRead);
        delay_us(15000);
#endif
    }
}

void camInterfaceCfg(camParamCfg_t* config)
{
	cspiDataFmt.endianMode 	= config->endianMode;
	cspiCtrl.rxWid 			= config->wireNum;
	cspiCtrl.rxdSeq 		= config->rxSeq;
    cspiCtrl.cpol    		= config->cpol;
    cspiCtrl.cpha    		= config->cpha;
    cspiCtrl.ddrMode    	= config->ddrMode;
    cspiBinaryCtrl.wordIdSeq = config->wordIdSeq;
    cspiBinaryCtrl.dummyAllowed = config->dummyAllowed;
    cspiCtrl.fillYonly    	= config->yOnly;
    cspiCtrl.rowScaleRatio  = config->rowScaleRatio;
    cspiCtrl.colScaleRatio  = config->colScaleRatio;
    cspiCtrl.scaleBytes    	= config->scaleBytes;
}

#if (ENABLE_CAMERA_LDO == 1)
void camPowerOn(uint8_t ioInitVal)
{
	PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);
    padConfig.pullUpEnable 		= PAD_PULL_UP_DISABLE;
	padConfig.pullDownEnable 	= PAD_PULL_DOWN_DISABLE;
    padConfig.mux 				= CAM_PD_PAD_ALT_FUNC;
    PAD_setPinConfig(CAM_PD_PAD_INDEX, &padConfig);

    GpioPinConfig_t config;
    config.pinDirection 		= GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput 		= ioInitVal;
    GPIO_pinConfig(CAM_PD_GPIO_INSTANCE, CAM_PD_GPIO_PIN, &config);
}
#endif

static void camUspCb()
{
	uint32_t cspiStatus;
	
	cspiStatus = camGetCspiStats();
	
	if (cspiStatus & ICL_STATS_FRAME_END_Msk)
	{
		CSPI1->STAS |= 0x3<<3;
		CSPI1->STAS |= 0xf<<7;
		CSPI1->STAS |= 0x3<<11;
		CSPI1->DMACTL |= 1<<24;

		if (userCamUspCb)
		{
			userCamUspCb(cspiStatus);
		}
		
		CSPI1->CBCTRL |= 2<<25;
	}
}


void camInit(void* dataAddr, cspiCbEvent_fn uspCb, void* dmaCb, camErrCb errCb)
{
	camResolution_e camResolution;
	camParamCfg_t camParamCfg;
	IRQn_Type irqNum;

	#if (RTE_CSPI0 == 1)
	irqNum = PXIC0_USP0_IRQn;
	#elif (RTE_CSPI1 == 1)
	irqNum = PXIC0_USP1_IRQn;
	#endif

	camBufBak 					 = dataAddr;
	camBufBak[0].enableForCamera = 1;
	camBufBak[0].enableForUsr    = 0;
	camBufBak[0].workingForUsr   = 0;

	if(uspCb) 
    {
        userCamUspCb = uspCb;
    }

	if (errCb)
	{
		camErrStatsFunc = errCb;
	}

	if (dmaCb)
	{
		userCamDmaCb = dmaCb;
	}
	
    XIC_SetVector(irqNum, camUspCb);
    XIC_EnableIRQ(irqNum);	
	
#if (CAMERA_ENABLE_GC032A)
 #if (GC032A_2SDR)
 	camParamCfg.wireNum  	= WIRE_2;
    camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;
	camParamCfg.cpha		= 0;
	camParamCfg.cpol		= 0;
	camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
	camParamCfg.dummyAllowed   = 0;
	camResolution 			= CAM_CHAIN_COUNT;
 #elif (GC032A_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
    camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;
	camParamCfg.cpha		= 0;
	camParamCfg.cpol		= 0;
	camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
	camParamCfg.dummyAllowed   = 0;
	camResolution 			= CAM_CHAIN_COUNT;
 #elif (GC032A_2DDR)
    camParamCfg.wireNum     = WIRE_2;
	camParamCfg.endianMode  = CAM_MSB_MODE;
	camParamCfg.rxSeq		= SEQ_1;
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camParamCfg.ddrMode     = 1;
	camParamCfg.wordIdSeq   = 1;
	camParamCfg.dummyAllowed   = 1;
	camResolution 			= CAM_CHAIN_COUNT;
 #endif

    if (CAM_CHAIN_COUNT == CAM_8W_COLOR)
    {    
	    camParamCfg.yOnly               = 0;
	    camParamCfg.rowScaleRatio       = 1;
	    camParamCfg.colScaleRatio       = 1;
	    camParamCfg.scaleBytes          = 3;
	}
	else if (CAM_CHAIN_COUNT == CAM_8W_Y)
	{
		camParamCfg.yOnly				= 1;
		camParamCfg.rowScaleRatio		= 1;
		camParamCfg.colScaleRatio		= 1;
		camParamCfg.scaleBytes			= 1;
	}
	else if (CAM_CHAIN_COUNT == CAM_30W_Y)
	{
		camParamCfg.yOnly				= 1;
		camParamCfg.rowScaleRatio		= 0;
		camParamCfg.colScaleRatio		= 0;
		camParamCfg.scaleBytes			= 0;
	}
	else if (CAM_CHAIN_COUNT == CAM_30W_COLOR)
	{
		camParamCfg.yOnly				= 0;
		camParamCfg.rowScaleRatio		= 0;
		camParamCfg.colScaleRatio		= 0;
		camParamCfg.scaleBytes			= 0;
	}
#elif (CAMERA_ENABLE_GC6153)
 #if (GC6153_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_1;
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camParamCfg.yOnly       = 1;
	camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
    camParamCfg.rowScaleRatio       = 0;
    camParamCfg.colScaleRatio       = 0;
    camParamCfg.scaleBytes          = 0;
	camResolution 			= CAM_CHAIN_COUNT;	

    if (CAM_CHAIN_COUNT == CAM_8W_COLOR)
    {    
	    camParamCfg.yOnly               = 0;
	    camParamCfg.rowScaleRatio       = 0;
	    camParamCfg.colScaleRatio       = 0;
	    camParamCfg.scaleBytes          = 3;
	}	
#endif

    camInterfaceCfg(&camParamCfg);

	cspiDrv->ctrl(CSPI_CTRL_MEM_ADDR , (uint32_t)dataAddr); // register the recv memory
    cspiDrv->powerCtrl(CSPI_POWER_FULL);
    cspiDrv->init(camDmaCb);
    cspiDrv->ctrl(CSPI_CTRL_DATA_FORMAT , 0);
    cspiDrv->ctrl(CSPI_CTRL_RXTOR , 0);
    cspiDrv->ctrl(CSPI_CTRL_FRAME_INFO0 , 0);
	cspiDrv->ctrl(CSPI_CTRL_INT_CTRL , 0);
    cspiDrv->ctrl(CSPI_CTRL_CSPICTL , 0);
	cspiDrv->ctrl(CSPI_CTRL_DMA_CTRL , 0);
	cspiDrv->ctrl(CSPI_CTRL_RESOLUTION_SET , camResolution);
    cspiDrv->ctrl(CSPI_CTRL_BUS_SPEED, (camFrequence_e)CAM_25_5_M);
    cspiDrv->ctrl(CSPI_BINARY_CTRL, 0);
    cspiDrv->ctrl(CSPI_CTRL_AUTO_CG_CTRL, 0);
    cspiDrv->ctrl(CSPI_FRAME_PROC_LSPI, 0);
    cspiDrv->ctrl(CSPI_DELAY_CTRL, 0);
}

void camStartStop(cspiStartStop_e startStop)
{
    cspiDrv->ctrl(CSPI_CTRL_START_STOP , (uint32_t)startStop);
}

void cspiStartIntEnable(cspiIntEnable_e intEnable)
{
    if (intEnable)
    {
        cspiIntCtrl.frameStartIntEn |= intEnable;
    }
    else
    {
        cspiIntCtrl.frameStartIntEn &= intEnable;
    }
    cspiDrv->ctrl(CSPI_CTRL_INT_CTRL , 0); // cspi interrupt enable or disable
}

void cspiEndIntEnable(cspiIntEnable_e endIntEnable)
{
    if (endIntEnable)
    {
        cspiIntCtrl.frameEndIntEn |= endIntEnable;
    }
    else
    {
        cspiIntCtrl.frameEndIntEn &= endIntEnable;
    }
    
    cspiDrv->ctrl(CSPI_CTRL_INT_CTRL , 0);
}

void cspi2LspiEnable(uint8_t enable)
{
    cspiFrameProcLspi.outEnLspi = enable;
    cspiDrv->ctrl(CSPI_FRAME_PROC_LSPI, 0);
}

void camFlush()
{
	cspiDrv->ctrl(CSPI_CTRL_FLUSH_RX_FIFO , 0);
}

void camRegisterIRQ(cspiInstance_e instance, camIrq_fn irqCb)
{
	IRQn_Type irqNum;

	if (instance == CSPI_0)
	{
		irqNum = PXIC0_USP0_IRQn;
	}
	else
	{
		irqNum = PXIC0_USP1_IRQn;
	}
	
    XIC_SetVector(irqNum, irqCb);
    XIC_EnableIRQ(irqNum);	
}

void camRecv(uint8_t * dataAddr)
{
    cspiDrv->ctrl(CSPI_CTRL_MEM_ADDR , (uint32_t)dataAddr);
    cspiDrv->recv();   
}

uint32_t camGetCspiStats()
{
	uint32_t status = 0;
	
#if (RTE_CSPI1 == 1)
	{
		status = EIGEN_CSPI(CSPI_1)->STAS;
	}
#else
	{
		status = EIGEN_CSPI(CSPI_0)->STAS;
	}
#endif

	return status;
}

void camClearErrStats()
{
#if (RTE_CSPI1 == 1)
	CSPI1->STAS 	|= 0x3<<3;
	CSPI1->STAS 	|= 0xf<<7;
	CSPI1->DMACTL 	|= 1<<24;
	CSPI1->CBCTRL 	|= 2<<25;
#else
	CSPI0->STAS 	|= 0x3<<3;
	CSPI0->STAS 	|= 0xf<<7;
	CSPI0->DMACTL 	|= 1<<24;
	CSPI0->CBCTRL 	|= 2<<25;
#endif
}

int camCheckErrStats()
{
	uint32_t status = camGetCspiStats();

    if ( (status >> ICL_STATS_RX_OVERFLOW_Pos & 1) || 
		(status >> ICL_STATS_RX_DMA_ERR_Pos & 1) || 
		(status >> ICL_STATS_FS_ERR_Pos & 1)
        ) 
    {   
		if (camErrStatsFunc)
		{
			camErrStatsFunc(status);
		}

        return -1; // has cspi err
    }
	else
	{
		return 0; 
	}
}

void camGpioPulseCfg(uint8_t padAddr, uint8_t pinInstance, uint8_t pinNum)
{
    PadConfig_t padConfig;
    PAD_getDefaultConfig(&padConfig);

    padConfig.mux = PAD_MUX_ALT0;
    PAD_setPinConfig(padAddr, &padConfig);

    GpioPinConfig_t config;
    config.pinDirection = GPIO_DIRECTION_OUTPUT;
    config.misc.initOutput = 0;

    GPIO_pinConfig(pinInstance, pinNum, &config);
}

void camGpioPulse(uint8_t pinInstance, uint8_t pinNum, uint32_t pulseDurationUs, uint8_t initialState, bool needLoop)
{
    GPIO_TypeDef *gpio = (GPIO_TypeDef*)(RMI_GPIO_BASE_ADDR + 0x1000*pinInstance);

	if (needLoop)
	{	
		// for test gpio is good or not
		while (1)
		{
			gpio->DATAOUT = ((~(1<<pinNum)) << 16) | (1<<pinNum);
			delay_us(pulseDurationUs);
			gpio->DATAOUT = ((~(1<<pinNum)) << 16);
			delay_us(pulseDurationUs);
		}
	}
	else
	{	
		// inititalState: 0=low level; 1=high level
		if (initialState == 0)
		{
			// pulse 0->1
			gpio->DATAOUT = ((~(1<<pinNum)) << 16) | (1<<pinNum);
			delay_us(pulseDurationUs);
			gpio->DATAOUT = ((~(1<<pinNum)) << 16);
		}
		else
		{
			// pulse 1->0
			gpio->DATAOUT = ((~(1<<pinNum)) << 16);
			delay_us(pulseDurationUs);
			gpio->DATAOUT = ((~(1<<pinNum)) << 16) | (1<<pinNum);
		}
	}
}

/************************camera rbuf*************************/
PLAT_PM_RAMCODE void timeRead(UINT32 *sysTime)
{
    //T_TMU_BC_RD* bcRd = HW_TmuBcRd; 4f0700c4
    UINT32 mask;
    UINT32 hfnsfnsbn;

    mask = SaveAndSetIRQMask();
//    CLOCK_clockEnable(PCLK_TMU);

    hfnsfnsbn = *(uint32_t*)0x4f0700c4;//bcRd->dbg_bc_h;

    *sysTime = (hfnsfnsbn >> 4) * 10 + (hfnsfnsbn & 0xf);

//    CLOCK_clockDisable(PCLK_TMU);
    RestoreIRQMask(mask);
}


static uint8_t findNextCamBuf()
{
	int bypassIndex = -1;
	uint8_t minIndex = 0;
	uint32_t minVal = 0;

	for (int i = 0; i < 3; i++)
	{
		if (camBufBak[i].workingForUsr == 1)
		{
			bypassIndex = i;
			break;
		}
	}

	minIndex = (bypassIndex == 0)? 1 : 0;
	minVal = camBufBak[minIndex].timeStamp;
	
	for (int i = (minIndex+1); i < 3; i++)
	{
		if ((camBufBak[i].timeStamp < minVal) && (camBufBak[i].workingForUsr == 0))
		{
			minVal = camBufBak[i].timeStamp;
			minIndex = i;
		}
	}

	return minIndex;
}

void camDmaCb(uint32_t dmaStatus)
{
#if (ENABLE_CAMERA_RING_BUF == 1)	
	int ret = 0;

	for (int i = 0; i < 3; i++)
	{
		if ((camBufBak[i].enableForCamera == 1) && (camBufBak[i].workingForUsr == 0))
		{
			// find out the current working buffer
			if (camCheckErrStats() == 0) // cspi right
			{
				timeRead(&camBufBak[i].timeStamp);
				camBufBak[i].enableForCamera	= 0;
				camBufBak[i].enableForUsr		= 1;
				camBufBak[i].workingForUsr 	= 0;
				camBufBak[i].camErrCnt 		= 0;

								
				ret = findNextCamBuf();
				//printf("next cam buf: %d\n", ret);
				
				#if (RTE_CSPI1 == 1)
				CSPI1->CBCTRL	|= 2<<25;
				#else
				CSPI0->CBCTRL	|= 2<<25;
				#endif
				
				camRecv(camBufBak[ret].data);
				camBufBak[ret].enableForCamera = 1;
				camBufBak[ret].enableForUsr    = 0;
				camBufBak[ret].workingForUsr   = 0;
				camBufBak[ret].camErrCnt       = 0;
			}
			else
			{
				// 1. clear cspi status
				camClearErrStats();
				
				// 2. recv again
				camRecv(camBufBak[i].data);
				camBufBak[i].enableForCamera = 1;
				camBufBak[i].camErrCnt 	 += 1;
				camBufBak[i].enableForUsr    = 0;
				camBufBak[i].workingForUsr   = 0;
			
				if (camBufBak[i].camErrCnt == 5)
				{
					// send a msg
				}
			}
			
			break;
		}
	}
#endif

	if (userCamDmaCb)
	{
		userCamDmaCb(dmaStatus);
	}
}

static int8_t findUsrEnableBuf()
{
	for (int i = 0; i < 3; i++)
	{
		if (camBufBak[i].enableForUsr == 1)
		{
			return i;
		}
	}

	return -1;
}

int camPicTake()
{
	int minIndex    = findUsrEnableBuf();
	if (minIndex < 0)
	{
		return minIndex;
	}
	else
	{
		uint32_t minVal = camBufBak[minIndex].timeStamp;

		for (int i = (minIndex+1); i < 3; i++)
		{
			if ((camBufBak[i].timeStamp > 0) && (camBufBak[i].timeStamp < minVal) && (camBufBak[i].enableForUsr == 1))
			{
				minVal = camBufBak[i].timeStamp;
				minIndex = i;
			}
		}
		
		uint32_t mask = SaveAndSetIRQMask();
		camBufBak[minIndex].workingForUsr = 1;
		RestoreIRQMask(mask);
	}

	return minIndex;
}

void camPicGive(int index)
{
	if (index < 0)
	{
		return;
	}

	if (index >= 3)
	{
		EC_ASSERT(0,0,0,0);
	}
	
	uint32_t mask = SaveAndSetIRQMask();
	camBufBak[index].workingForUsr = 0;
	camBufBak[index].enableForUsr  = 1;
	RestoreIRQMask(mask);
}


/************************camera rbuf*************************/

