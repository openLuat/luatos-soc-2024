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
AP_PLAT_COMMON_BSS static camErrCb             camErrStatsFunc;


#if (CAMERA_ENABLE_SP0A39)
 #if (SP0A39_2SDR)
    char* regName = "sp0a39_2sdr";
 #elif (SP0A39_1SDR)
    char* regName = "sp0a39_1sdr";
 #endif

#elif (CAMERA_ENABLE_SP0821)
 #if (SP0821_2SDR)
    char* regName = "sp0821_2sdr";
 #elif (SP0821_1SDR)
    char* regName = "sp0821_1sdr";
 #endif

#elif (CAMERA_ENABLE_GC6123)
 #if (GC6123_2SDR)
    char* regName = "gc6123_2sdr";
 #elif (GC6123_1SDR)
    char* regName = "gc6123_1sdr";
 #endif

#elif (CAMERA_ENABLE_GC032A)
 #if (GC032A_2SDR)
    char* regName = "gc032a_2sdr";
 #elif (GC032A_1SDR)
    char* regName = "gc032a_1sdr";
 #elif (GC032A_2DDR)
    char* regName = "gc032a_2ddr";
 #endif
#elif (CAMERA_ENABLE_BF30A2)
 #if (BF30A2_1SDR)
    char* regName = "bf30a2_1sdr";
 #endif 
#elif (CAMERA_ENABLE_GC6153)
 #if (GC6153_1SDR)
	char* regName = "gc6153_1sdr";
 #endif 
#elif (CAMERA_ENABLE_BF30A2)
 #if (BF30A2_1SDR)
    char* regName = "bf30a2_1sdr";
 #endif 
#elif (CAMERA_ENABLE_GC6133)
 #if (GC6133_1SDR)
	char* regName = "gc6133_1sdr";
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
    if (strcmp(regName, "sp0a39_2sdr") == 0)
    {
        extern camI2cCfg_t sp0A39_2sdrRegInfo[];
        *regInfo = sp0A39_2sdrRegInfo;
        *slaveAddr = SP0A39_I2C_ADDR;
        *regCnt = sp0a39GetRegCnt(regName);
    }
    else if (strcmp(regName, "sp0a39_1sdr") == 0)
    {
        extern camI2cCfg_t sp0A39_1sdrRegInfo[];
        *regInfo = sp0A39_1sdrRegInfo;
        *slaveAddr = SP0A39_I2C_ADDR;
        *regCnt = sp0a39GetRegCnt(regName);
    }
    else if (strcmp(regName, "sp0821_2sdr") == 0)
    {
        extern camI2cCfg_t sp0821_2sdrRegInfo[];
        *regInfo = sp0821_2sdrRegInfo;
        *slaveAddr = SP0821_I2C_ADDR;
        *regCnt = sp0821GetRegCnt(regName);
    }
    else if (strcmp(regName, "sp0821_1sdr") == 0)
    {
        extern camI2cCfg_t sp0821_1sdrRegInfo[];
        *regInfo = sp0821_1sdrRegInfo;
        *slaveAddr = SP0821_I2C_ADDR;
        *regCnt = sp0821GetRegCnt(regName);
    }
    else if (strcmp(regName, "gc6123_2sdr") == 0)
    {
        extern camI2cCfg_t gc6123_2sdrRegInfo[];
        *regInfo = gc6123_2sdrRegInfo;
        *slaveAddr = GC6123_I2C_ADDR;
        *regCnt = gc6123GetRegCnt(regName);
    }
    else if (strcmp(regName, "gc6123_1sdr") == 0)
    {
        extern camI2cCfg_t gc6123_1sdrRegInfo[];
        *regInfo = gc6123_1sdrRegInfo;
        *slaveAddr = GC6123_I2C_ADDR;
        *regCnt = gc6123GetRegCnt(regName);
    }
    else if (strcmp(regName, "gc032a_2sdr") == 0)
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
    else if (strcmp(regName, "bf30a2_1sdr") == 0)
    {
        extern camI2cCfg_t bf30a2_1sdrRegInfo[];
        *regInfo = bf30a2_1sdrRegInfo;
        *slaveAddr = BF30A2_I2C_ADDR;
        *regCnt = bf30a2GetRegCnt(regName);
    }
	else if (strcmp(regName, "gc6153_1sdr") == 0)
    {
        extern camI2cCfg_t gc6153_1sdrRegInfo[];
        *regInfo = gc6153_1sdrRegInfo;
        *slaveAddr = GC6153_I2C_ADDR;
        *regCnt = gc6153GetRegCnt(regName);
    }
	else if (strcmp(regName, "gc6133_1sdr") == 0)
    {
        extern camI2cCfg_t gc6133_1sdrRegInfo[];
        *regInfo = gc6133_1sdrRegInfo;
        *slaveAddr = GC6133_I2C_ADDR;
        *regCnt = gc6133GetRegCnt(regName);
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

void camSetMemAddr(uint32_t dataAddr)
{
    cspiDrv->ctrl(CSPI_CTRL_MEM_ADDR , dataAddr); // register the recv memory
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

void camInit(void* dataAddr, cspiCbEvent_fn cb)
{
	camResolution_e camResolution;

    // Need to enable cspi first to make camera clock working
	camParamCfg_t camParamCfg;
	
#if (CAMERA_ENABLE_SP0A39)
 #if (SP0A39_2SDR)
 	camParamCfg.wireNum  	= WIRE_2;
 #elif (SP0A39_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camResolution 			= CAM_CHAIN_COUNT;

#elif (CAMERA_ENABLE_SP0821)
 #if (SP0821_2SDR)
 	camParamCfg.wireNum  	= WIRE_2;
 #elif (SP0821_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;	
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camResolution 			= CAM_CHAIN_COUNT;

#elif (CAMERA_ENABLE_GC6123)
 #if (GC6123_2SDR)
 	camParamCfg.wireNum  	= WIRE_2;
 #elif (GC6123_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_1;
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
	camParamCfg.yOnly       = 1;
	camParamCfg.rowScaleRatio		= 0;
	camParamCfg.colScaleRatio		= 0;
	camParamCfg.scaleBytes		    = 0;
	camResolution 			= CAM_CHAIN_COUNT;
	// recv 8w pic into memory
    camParamCfg.yOnly               = 1;

#elif (CAMERA_ENABLE_GC032A)
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

    // recv 8w pic into memory
    if (CAM_CHAIN_COUNT == 20)
    {    
	    camParamCfg.yOnly               = 0;
	    camParamCfg.rowScaleRatio       = 1;
	    camParamCfg.colScaleRatio       = 1;
	    camParamCfg.scaleBytes          = 3;
	}
	else if (CAM_CHAIN_COUNT == 10)
	{
		camParamCfg.yOnly				= 1;
		camParamCfg.rowScaleRatio		= 1;
		camParamCfg.colScaleRatio		= 1;
		camParamCfg.scaleBytes			= 1;
	}

#elif (CAMERA_ENABLE_BF30A2)
 #if (BF30A2_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;
	camParamCfg.cpha		= 0;
	camParamCfg.cpol		= 0;
	camParamCfg.yOnly       = 1;
    camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
    camParamCfg.rowScaleRatio		= 0;
	camParamCfg.colScaleRatio		= 0;
	camParamCfg.scaleBytes		    = 0;
	camResolution 			= CAM_CHAIN_COUNT;
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
#elif (CAMERA_ENABLE_GC6133)
 #if (GC6133_1SDR)
 	camParamCfg.wireNum  	= WIRE_1;
 #endif
	camParamCfg.endianMode  = CAM_LSB_MODE;
	camParamCfg.rxSeq		= SEQ_0;
	camParamCfg.cpha		= 1;
	camParamCfg.cpol		= 0;
	camParamCfg.yOnly       = 1;
	camParamCfg.ddrMode     = 0;
	camParamCfg.wordIdSeq   = 0;
    camParamCfg.rowScaleRatio       = 0;
    camParamCfg.colScaleRatio       = 0;
    camParamCfg.scaleBytes          = 0;
	camResolution 			= CAM_CHAIN_COUNT;		
#endif

    camInterfaceCfg(&camParamCfg);

	cspiDrv->ctrl(CSPI_CTRL_MEM_ADDR , (uint32_t)dataAddr); // register the recv memory
    cspiDrv->powerCtrl(CSPI_POWER_FULL);
    cspiDrv->init(cb);
    cspiDrv->ctrl(CSPI_CTRL_DATA_FORMAT , 0); // control cspi
    cspiDrv->ctrl(CSPI_CTRL_RXTOR , 0);
    cspiDrv->ctrl(CSPI_CTRL_FRAME_INFO0 , 0);
	cspiDrv->ctrl(CSPI_CTRL_INT_CTRL , 0);
    cspiDrv->ctrl(CSPI_CTRL_CSPICTL , 0);
	cspiDrv->ctrl(CSPI_CTRL_DMA_CTRL , 0);
	cspiDrv->ctrl(CSPI_CTRL_RESOLUTION_SET , camResolution);
    cspiDrv->ctrl(CSPI_CTRL_BUS_SPEED, (camFrequence_e)CAM_25_5_M);  // cspi working frequency
    cspiDrv->ctrl(CSPI_BINARY_CTRL, 0);  // binary control
    cspiDrv->ctrl(CSPI_CTRL_AUTO_CG_CTRL, 0);  // autocg control
    cspiDrv->ctrl(CSPI_FRAME_PROC_LSPI, 0);  // out for lspi
    cspiDrv->ctrl(CSPI_DELAY_CTRL, 0);  // delay ctrl
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
        //printf("now enable end irq.\n");
        cspiIntCtrl.frameEndIntEn |= endIntEnable;
    }
    else
    {
        //printf("now disable end irq.\n");
        cspiIntCtrl.frameEndIntEn &= endIntEnable;
    }
    
    cspiDrv->ctrl(CSPI_CTRL_INT_CTRL , 0); // cspi interrupt enable or disable
}

void cspi2LspiEnable(uint8_t enable)
{
    cspiFrameProcLspi.outEnLspi = enable;
    cspiDrv->ctrl(CSPI_FRAME_PROC_LSPI, 0);
}

void camFlush()
{
	cspiDrv->ctrl(CSPI_CTRL_FLUSH_RX_FIFO , 0); // flush rx fifo
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

PLAT_FM_RAMCODE void camRecv(uint8_t * dataAddr)
{
    cspiDrv->ctrl(CSPI_CTRL_MEM_ADDR , (uint32_t)dataAddr); // register the recv memory
    cspiDrv->recv();   
}

uint32_t camGetCspiStats(cspiInstance_e instance)
{
    return EIGEN_CSPI(instance)->STAS;
}

uint32_t camGetCspiInt(cspiInstance_e instance)
{
    return EIGEN_CSPI(instance)->INTCTL;
}


void camClearIntStats(cspiInstance_e instance, uint32_t mask)
{
	EIGEN_CSPI(instance)->STAS = mask;
}

void camRegisterErrStatsCb(camErrCb errCb)
{
    camErrStatsFunc = errCb;
}

void camCheckErrStats()
{
    if (!camErrStatsFunc)
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
        camErrStatsFunc(status);
    }        

    return;    
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


