/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    hal_i2s.c
 * Description:  EC7xx i2s hal driver source file
 * History:      Rev1.0   2021-9-18
 *
 ****************************************************************************/
#include "hal_i2s.h"
#ifdef FEATURE_OS_ENABLE
#include DEBUG_LOG_HEADER_FILE
#endif

extern I2sDrvInterface_t    i2sDrvInterface0;
extern I2sDrvInterface_t    i2sDrvInterface1;

AP_PLAT_COMMON_BSS static I2sDrvInterface_t   *i2sDrv = NULL;

extern I2sDataFmt_t         i2sDataFmt;
extern I2sSlotCtrl_t        i2sSlotCtrl;
extern I2sBclkFsCtrl_t      i2sBclkFsCtrl;
extern I2sCtrl_t            i2sCtrl;
extern I2sDmaCtrl_t         i2sDmaCtrl;
extern I2sIntCtrl_t         i2sIntCtrl;
AP_PLAT_COMMON_BSS static I2sMode_e            i2sMode;

// [cmd][curState]
static int32_t i2sCtrlState[6][4] = 
{
    // stop_i2s    only_send    only_recv     send_recv
    {STOP_I2S,  STOP_I2S,    -1,      ONLY_RECV},   // STOP_SEND
    {STOP_I2S,    -1,      STOP_I2S,  ONLY_SEND},   // STOP_RECV
    {STOP_I2S,  STOP_I2S,  STOP_I2S,  STOP_I2S},    // STOP_ALL
    {ONLY_SEND, ONLY_SEND, SEND_RECV, SEND_RECV},   // START_SEND
    {ONLY_RECV, SEND_RECV, ONLY_RECV, SEND_RECV},   // START_RECV
    {SEND_RECV, SEND_RECV, SEND_RECV, SEND_RECV},   // START_ALL
};

// Register 
void halI2sInit(i2sCbFunc_fn txCb, i2sCbFunc_fn rxCb)
{
#if (RTE_I2S0)
    i2sDrv = &CREATE_SYMBOL(i2sDrvInterface, 0); // Choose i2s0
#elif (RTE_I2S1)
    i2sDrv = &CREATE_SYMBOL(i2sDrvInterface, 1); // Choose i2s1
#endif

    i2sDrv->init(txCb, rxCb);
    i2sDrv->powerCtrl(I2S_POWER_FULL);
}

void halI2sSetDmaDescriptorNum(I2sDirectionSel_e direc, uint8_t num)
{
    switch (direc)
    {
        case I2S_TX:
            i2sDrv->ctrl(I2S_CTRL_TX_DESCRIPTOR , num);
            break;

        case I2S_RX:
            i2sDrv->ctrl(I2S_CTRL_RX_DESCRIPTOR , num);
            break;

         default:
            break;         
    }
}

void halI2sDeInit()
{
    i2sDrv->powerCtrl(I2S_POWER_OFF);
    i2sDrv->deInit();
}

void halI2sStopDma()
{
    i2sDrv->stopDmaChannel();
}


void halI2sSetChannel(I2sChannelSel_e channelSel)
{
    switch(channelSel)
    {
		case MONO:
			{
				i2sSlotCtrl.slotEn  = 1;
				i2sSlotCtrl.slotNum = 1;
			}
			break;

		case DUAL_CHANNEL:
			{
				i2sSlotCtrl.slotEn  = 3;
				i2sSlotCtrl.slotNum = 1;
			}
			break;

		default:
			break;
    }
    
    i2sDrv->ctrl(I2S_CTRL_SLOT_CTRL , 0);
}

void halI2sSetTotalNum(uint32_t totalNum)
{
    i2sDrv->ctrl(I2S_CTRL_SET_TOTAL_NUM , totalNum);
}

uint32_t halI2sGetTotalNum()
{
	return i2sDrv->getTotalCnt();
}

uint32_t halI2sGetTrunkNum()
{
	return i2sDrv->getTrunkCnt();
}

void halI2sSetSampleRate(I2sRole_e i2sRole, I2sSampleRate_e sampleRate)
{
    if (i2sRole == I2S_SLAVE_MODE) // Codec act as master
    {
        i2sDrv->ctrl(I2S_CTRL_SAMPLE_RATE_SLAVE, sampleRate); // I2S Set sample rate in slave role
    }
    else
    {
        i2sDrv->ctrl(I2S_CTRL_SAMPLE_RATE_MASTER, sampleRate); // I2S Set sample rate in master role
    }
}

// Control I2S to start or stop. Note: when app wakeup from sleep1, need to call this api to start MCLK and i2s
int32_t halI2sStartStop(I2sStartStop_e startStop)
{
    int32_t         ret            = 0;
    I2sCtrlMode_e   ctrlRegMode    = (I2sCtrlMode_e)i2sDrv->getCtrlReg();
    
#ifdef FEATURE_OS_ENABLE
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, halI2sStartStop_0, P_DEBUG, "startStop=%e<I2sStartStop_e>, ctrlRegMode=%e<I2sCtrlMode_e>", startStop, ctrlRegMode);
#endif
    
    ret = i2sCtrlState[startStop][ctrlRegMode];
    
#ifdef FEATURE_OS_ENABLE
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, halI2sStartStop_1, P_DEBUG, "ret=%d", ret);
#endif
	
    if (ret < 0)
    {
        return -1;
    }
    else
    {
        i2sDrv->ctrl(I2S_CTRL_START_STOP , ret);
        return 0;
    }
}

void halI2sConfig(I2sParamCtrl_t paramCtrl)
{   
    // 1. Setting parameters per the I2S working mode    
    i2sMode = paramCtrl.mode;
    switch (i2sMode)
    {
        case MSB_MODE:
            i2sDataFmt.dataDly          = 0;
            i2sBclkFsCtrl.bclkPolarity  = 1; 
            i2sBclkFsCtrl.fsPolarity    = 0;
            
            break;

        case LSB_MODE:
            i2sDataFmt.dataDly          = 1;
            i2sBclkFsCtrl.bclkPolarity  = 1;
            i2sBclkFsCtrl.fsPolarity    = 0;

            break;

        case I2S_MODE:
            i2sDataFmt.dataDly          = 1;
            i2sBclkFsCtrl.bclkPolarity  = paramCtrl.polarity; // 8311 need 1, others need 0
            i2sBclkFsCtrl.fsPolarity    = 1;

            break;

        case PCM_MODE:    
            // Configure codec to PCM mode
            i2sBclkFsCtrl.bclkPolarity  = 1;
            break;

        default:
            break;
    }

    
    // 2. Init codec and I2S controller
    switch (paramCtrl.role)
    {
        case I2S_SLAVE_MODE:
        {
            i2sDrv->ctrl(I2S_CTRL_SAMPLE_RATE_SLAVE , paramCtrl.sampleRate); // I2S Set sample rate in slave role 
            break;
        }

        case I2S_MASTER_MODE:
        {

            i2sDataFmt.slaveModeEn = 0; // Master mode
            i2sDrv->ctrl(I2S_CTRL_DATA_FORMAT , 0); 
            i2sDrv->ctrl(I2S_CTRL_SAMPLE_RATE_MASTER , paramCtrl.sampleRate); // I2S Set sample rate in master role
                
            break;
        }

        default:
            break;
    }

    // 3. Set frame size
    switch (paramCtrl.frameSize)
    {
        case FRAME_SIZE_16_16:
            i2sDataFmt.slotSize     = 0xf;
            i2sBclkFsCtrl.fsWidth   = 0xf;
            i2sDataFmt.wordSize     = 0xf;

            break;

        case FRAME_SIZE_16_32:
            // I2S controller part
            i2sDataFmt.slotSize     = 0x1f;
            i2sDataFmt.wordSize     = 0xf;
            i2sBclkFsCtrl.fsWidth   = 0x1f;

            break;

        case FRAME_SIZE_24_32:
            // I2S controller part
            i2sDataFmt.slotSize     = 0x17;
            i2sDataFmt.wordSize     = 0x17;
            i2sBclkFsCtrl.fsWidth   = 0x17;
            i2sDataFmt.txPack       = 0;
            i2sDataFmt.rxPack       = 0;

            break;

        case FRAME_SIZE_32_32:
            // I2S controller part
            i2sDataFmt.slotSize     = 0x1f;
            i2sDataFmt.wordSize     = 0x1f;
            i2sBclkFsCtrl.fsWidth   = 0x1f;

            break;

        default:
            break;
    }

	// 4. Select mono or dual-channel
    switch(paramCtrl.channelSel)
    {
		case MONO:
			{
				i2sSlotCtrl.slotEn  = 1;
				i2sSlotCtrl.slotNum = 1;
			}
			break;

		case DUAL_CHANNEL:
			{
				i2sSlotCtrl.slotEn  = 3;
				i2sSlotCtrl.slotNum = 1;
			}
			break;

		default:
			break;
    }

    // Init part of I2S controller
    i2sDrv->ctrl(I2S_CTRL_DATA_FORMAT , 0);
    i2sDrv->ctrl(I2S_CTRL_BCLK_FS_CTRL , 0);
    i2sDrv->ctrl(I2S_CTRL_SLOT_CTRL , 0);
    i2sDrv->ctrl(I2S_CTRL_INT_CTRL , 0);
	i2sDrv->ctrl(I2S_CTRL_DMA_CTRL , 0);
}

void ctrlErrStatsBit(uint32_t errStats, bool enableErrInt)
{
    uint8_t bitIndex = 0;

    while (errStats >> bitIndex != 0)
    {
        if ((errStats >> bitIndex & 1) == 1)
        {
            switch (bitIndex)
            {
                case 0:
                {
                    enableErrInt ? (i2sIntCtrl.txUnderRunIntEn = 1) : (i2sIntCtrl.txUnderRunIntEn = 0);
                    break;
                }

                case 1:
                {
                    enableErrInt ? (i2sIntCtrl.txDmaErrIntEn = 1) : (i2sIntCtrl.txDmaErrIntEn = 0);
                    break;
                }

                case 3:
                {
                    enableErrInt ? (i2sIntCtrl.rxOverFlowIntEn = 1) : (i2sIntCtrl.rxOverFlowIntEn = 0);
                    break;
                }

                case 4:
                {
                    enableErrInt ? (i2sIntCtrl.rxDmaErrIntEn = 1) : (i2sIntCtrl.rxDmaErrIntEn = 0);
                    break;
                }

                case 5:
                {
                    enableErrInt ? (i2sIntCtrl.rxDatIntEn = 1) : (i2sIntCtrl.rxDatIntEn = 0);
                    break;
                }

                case 6:
                {
                    enableErrInt ? (i2sIntCtrl.rxTimeOutIntEn = 1) : (i2sIntCtrl.rxTimeOutIntEn = 0);
                    break;
                }

                case 7:
                {
                    enableErrInt ? (i2sIntCtrl.fsErrIntEn = 1) : (i2sIntCtrl.fsErrIntEn = 0);
                    break;
                }

                default:
                break;
            }

            bitIndex++;
        }
    }

    i2sDrv->ctrl(I2S_CTRL_INT_CTRL , 0);
}

void halI2sRegisterUspCb(i2sUspFunc_fn cb)
{
    if (cb != NULL)
    {
        #if (RTE_I2S0 == 1)
        XIC_SetVector(PXIC0_USP0_IRQn, cb);
        XIC_EnableIRQ(PXIC0_USP0_IRQn);
        #else 
        XIC_SetVector(PXIC0_USP1_IRQn, cb);
        XIC_EnableIRQ(PXIC0_USP1_IRQn);
        #endif            
    }
}


void halI2sTransfer(I2sPlayRecord_e playRecord, uint8_t* memAddr, uint32_t trunkNum)
{
    // 5. After other parameters are ready, start the I2S controller
    if (playRecord == PLAY) // Play audio
    {
        i2sDrv->send(true, true, false, memAddr, trunkNum);
    }
    else if (playRecord == RECORD) // Record audio
    {
        i2sDrv->recv(true, true, false, memAddr, trunkNum);
    }
    else if (playRecord == PLAY_LOOP)
    {
        i2sDrv->send(false, false, true, memAddr, trunkNum);
    }
    else if (playRecord == PLAY_LOOP_IRQ)
    {
        i2sDrv->send(false, true, true, memAddr, trunkNum);
    }
    else if (playRecord == RECORD_LOOP_IRQ)
    {
        i2sDrv->recv(false, true, true, memAddr, trunkNum);
    }
}

void halI2sSrcAdjustVolumn(int16_t* srcBuf, uint32_t srcTotalNum, uint16_t volScale)
{
	int integer = volScale / 10;
	int decimal = volScale % 10;
	int scale = 0;
	int32_t tmp = 0;
	uint32_t totalNum = srcTotalNum;
	uint32_t step = 0;
	
	while (totalNum)
	{
		if (volScale < 10)
		{
			tmp = ((*(srcBuf + step)) * (256 * integer + 26 * decimal)) >> 8;
		}
		else
		{
			scale = (256 * integer + 26 * decimal) >> 8;
			tmp = (*(srcBuf + step)) * scale;
		}
		
		if (tmp > 32767)
		{
			tmp = 32767;
		}
		else if (tmp < -32768)
		{
			tmp = -32768;
		}
			
		*(srcBuf + step) = (int16_t)tmp;
		step += 1;
		totalNum -= 2;
	}
}

void halI2sSetInt()
{
	//i2sDrv->ctrl(I2S_CTRL_INT_CTRL , startStop);
}

int getMclkFromPin(uint8_t padAddr, PadMux_e padMux, uint32_t freq, FracDivRootClk_e clkSrc)
{
#include "math.h"
    /*
         718 supports:
                        padAddr:18/GPIO3 /USP1/Func1, 
                        padAddr:37/GPIO31/USP1/Func4, 
                        padAddr:39/GPIO33/USP0/Func1, 
                        padAddr:44/GPIO38/USP2/Func1, 
                        padAddr:30/GPIO15/USP2/Func4

         716 supports:   
                        padAddr:11/SWDIOC /USP0/Func6, 
                        padAddr:13/GPIO1  /USP0/Func6, 
                        padAddr:10/SWCLKC /USP1/Func6,
    */

    PadConfig_t config;
    FracDivConfig_t fracdivCfg;
    PAD_getDefaultConfig(&config);
    config.mux = padMux;
    PAD_setPinConfig(padAddr, &config);

    switch (padAddr)
    {
        #if defined CHIP_EC718
        case 18:
        case 37:
        #elif  defined CHIP_EC716
        case 10:
        #endif
        {
            // usp1
            *(uint32_t*)0x4d041028 = 1;
            GPR_clockEnable(FCLK_USP1);
            GPR_clockEnable(CLK_HF306M_G); // open cspi fclk src
            GPR_fracDivOutCLkEnable(FRACDIV1_OUT0); 
            GPR_setFracDivOutClkDiv(FRACDIV1_OUT0, 1); 

            GPR_setMclkSrc(MCLK1, MCLK_SRC_FRACDIV1_OUT0); 
            GPR_mclkEnable(MCLK1);
            fracdivCfg.fracdivSel = FRACDIV_1;
            
            if (clkSrc == FRACDIC_ROOT_CLK_612M)
            {
                fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
                fracdivCfg.fracDiv1DivRatioInteger  = 614400000/freq;
                fracdivCfg.fracDiv1DivRatioFrac     = (614400000%freq)*(pow(2,24))/freq;
            }
            else
            {
                fracdivCfg.source = FRACDIV_ROOT_CLK_26M;
                fracdivCfg.fracDiv1DivRatioInteger  = 26100000/freq;
                fracdivCfg.fracDiv1DivRatioFrac     = (26100000%freq)*(pow(2,24))/freq;
            }
    
            break;
        }

        #if defined CHIP_EC718
        case 39:
        #elif  defined CHIP_EC716
        case 11:
        case 13:
        #endif        
        {
            // usp0
            *(uint32_t*)0x4d040028 = 1;
            GPR_clockEnable(FCLK_USP0);
            GPR_clockEnable(CLK_HF306M_G); // open cspi fclk src
            GPR_fracDivOutCLkEnable(FRACDIV0_OUT0); 
            GPR_setFracDivOutClkDiv(FRACDIV0_OUT0, 1); 

            GPR_setMclkSrc(MCLK0, MCLK_SRC_FRACDIV0_OUT0); 
            GPR_mclkEnable(MCLK0);
            fracdivCfg.fracdivSel = FRACDIV_0;

            if (clkSrc == FRACDIC_ROOT_CLK_612M)
            {
                fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
                fracdivCfg.fracDiv0DivRatioInteger  = 612000000/freq;
                fracdivCfg.fracDiv0DivRatioFrac     = (612000000%freq)*(pow(2,24))/freq;
            }
            else
            {
                fracdivCfg.source = FRACDIV_ROOT_CLK_26M;
                fracdivCfg.fracDiv0DivRatioInteger  = 26000000/freq;
                fracdivCfg.fracDiv0DivRatioFrac     = (26000000%freq)*(pow(2,24))/freq;
            }
            
            break;
        }

        #if defined CHIP_EC718
        case 44:
        case 30:
        {
            // usp2
            *(uint32_t*)0x4d042028 = 1;
            GPR_clockEnable(FCLK_USP0);
            GPR_clockEnable(CLK_HF306M_G); // open cspi fclk src
            GPR_fracDivOutCLkEnable(FRACDIV0_OUT0); 
            GPR_setFracDivOutClkDiv(FRACDIV0_OUT0, 1); 

            GPR_setMclkSrc(MCLK2, MCLK_SRC_FRACDIV0_OUT0); 
            GPR_mclkEnable(MCLK2);
            fracdivCfg.fracdivSel = FRACDIV_0;

            if (clkSrc == FRACDIC_ROOT_CLK_612M)
            {
                fracdivCfg.source = FRACDIC_ROOT_CLK_612M;
                fracdivCfg.fracDiv0DivRatioInteger  = 612000000/freq;
                fracdivCfg.fracDiv0DivRatioFrac     = (612000000%freq)*(pow(2,24))/freq;
            }
            else
            {
                fracdivCfg.source = FRACDIV_ROOT_CLK_26M;
                fracdivCfg.fracDiv0DivRatioInteger  = 26000000/freq;
                fracdivCfg.fracDiv0DivRatioFrac     = (26000000%freq)*(pow(2,24))/freq;
            }
            break;
        }
        #endif

        default:
        return -2;
    }
    
    GPR_setFracDivConfig(&fracdivCfg);
    
    return 0;
}

