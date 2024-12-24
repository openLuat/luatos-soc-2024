/****************************************************************************
 *
 * Copy right:   2019-, Copyrigths of AirM2M Ltd.
 * File name:    lspi.c
 * Description:  Dedicated spi for LCD use in EC718. The interface is similar with CMSIS Driver API V2.0.
 * History:      Rev1.0   2021-11-25
 *
 ****************************************************************************/
#include "lspi.h"
#include "slpman.h"
#include "sctdef.h"

#if (RTE_LSPI2)
#if (LCD_INTERFACE_SPI == 1)
AP_PLAT_COMMON_DATA static PIN lspi2Ds      = {RTE_USP2_DS_PAD_ADDR,       RTE_USP2_DS_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Clk     = {RTE_USP2_CLK_PAD_ADDR,      RTE_USP2_CLK_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Cs      = {RTE_USP2_CS_PAD_ADDR,       RTE_USP2_CS_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Miso    = {RTE_USP2_DIN_PAD_ADDR,      RTE_USP2_DIN_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Mosi0   = {RTE_USP2_DOUT0_PAD_ADDR,   RTE_USP2_DOUT0_FUNC};
#if (SPI_2_DATA_LANE == 1)
AP_PLAT_COMMON_DATA static PIN lspi2Mosi1   = {RTE_USP2_DOUT1_PAD_ADDR,    RTE_USP2_DOUT1_FUNC};
#endif

#elif (LCD_INTERFACE_MSPI == 1)
AP_PLAT_COMMON_DATA static PIN lspi2Scl     = {RTE_USP2_SCL_PAD_ADDR,     RTE_USP2_SCL_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Cs      = {RTE_USP2_CS_PAD_ADDR,      RTE_USP2_CS_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Sdi     = {RTE_USP2_SDI_PAD_ADDR,     RTE_USP2_SDI_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D0      = {RTE_USP2_D0_PAD_ADDR,      RTE_USP2_D0_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D1      = {RTE_USP2_D1_PAD_ADDR,      RTE_USP2_D1_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D2      = {RTE_USP2_D2_PAD_ADDR,      RTE_USP2_D2_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D3      = {RTE_USP2_D3_PAD_ADDR,      RTE_USP2_D3_FUNC};

#elif (LCD_INTERFACE_8080 == 1)
AP_PLAT_COMMON_DATA static PIN lspi2Scl     = {RTE_USP2_SCL_PAD_ADDR,     RTE_USP2_SCL_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Cs      = {RTE_USP2_CS_PAD_ADDR,      RTE_USP2_CS_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Dcx     = {RTE_USP2_DCX_PAD_ADDR,     RTE_USP2_DCX_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2Rclk    = {RTE_USP2_RCLK_PAD_ADDR,    RTE_USP2_RCLK_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D0      = {RTE_USP2_D0_PAD_ADDR,      RTE_USP2_D0_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D1      = {RTE_USP2_D1_PAD_ADDR,      RTE_USP2_D1_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D2      = {RTE_USP2_D2_PAD_ADDR,      RTE_USP2_D2_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D3      = {RTE_USP2_D3_PAD_ADDR,      RTE_USP2_D3_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D4      = {RTE_USP2_D4_PAD_ADDR,      RTE_USP2_D4_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D5      = {RTE_USP2_D5_PAD_ADDR,      RTE_USP2_D5_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D6      = {RTE_USP2_D6_PAD_ADDR,      RTE_USP2_D6_FUNC};
AP_PLAT_COMMON_DATA static PIN lspi2D7      = {RTE_USP2_D7_PAD_ADDR,      RTE_USP2_D7_FUNC};
#endif
#endif

AP_PLAT_COMMON_BSS uint8_t 			lspiDiv;
AP_PLAT_COMMON_BSS lcdSlp1Cb_fn     lcdSlp1CbFn = 0;
AP_PLAT_COMMON_BSS static uint32_t 	bpsBak;
static int32_t lspiSetBusSpeed(uint32_t bps, lspiRes_t *lspi);


#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)

// 718 Data Format 
AP_PLAT_COMMON_DATA lspiDataFmt_t lspiDataFmt = 
{
    .slaveModeEn            = 0,
    .slotSize               = 0,
    .wordSize               = 7,
    .alignMode              = 0,
    .endianMode             = 0, // 0:LSB  1: MSB
    .dataDly                = 0,
    .txPad                  = 0,
    .rxSignExt              = 0,
    .txPack                 = 0,
    .rxPack                 = 0,
    .txFifoEndianMode       = 0,
    .rxFifoEndianMode       = 0,
    .eorMode                = 0,
};

// 718 DMA Control
AP_PLAT_COMMON_DATA lspiDmaCtrl_t lspiDmaCtrl =
{
    .rxDmaReqEn             = 0,
    .txDmaReqEn             = 0,
    .rxDmaTimeOutEn         = 0,
    .dmaWorkWaitCycle       = 15,
    .rxDmaBurstSizeSub1     = 7,
    .txDmaBurstSizeSub1     = 7,
    .rxDmaThreadHold        = 7,
    .txDmaThreadHold        = 7,
    .rxFifoFlush            = 0,
    .txFifoFlush            = 0
};

// 718 INT Control
AP_PLAT_COMMON_DATA lspiIntCtrl_t lspiIntCtrl =
{
    .txUnderRunIntEn        = 0,
    .txDmaErrIntEn          = 0,
    .txDatIntEn             = 0,
    .rxOverFlowIntEn        = 0,
    .rxDmaErrIntEn          = 0,
    .rxDatIntEn             = 0,
    .rxTimeOutIntEn         = 0,
    .fsErrIntEn             = 0,
    .frameStartIntEn        = 0,
    .frameEndIntEn          = 0,
    .cspiBusTimeOutIntEn    = 0,
    .lspiRamWrBreakIntEn    = 0,
    .lspiRamWrFrameStartEn  = 0,
    .lspiRamWrFrameEndEn    = 0,
    .lspiCmdEndEn           = 0,
    .cspiOtsuEndEn          = 0,
    .lspiRamWrEndEn         = 0,
    .txIntThreshHold        = 8,
    .rxIntThreshHold        = 8,
};

// 718 lspi control
AP_PLAT_COMMON_DATA lspiCtrl_t lspiCtrl =
{
    .enable                 = 1,           ///< lspi Enable
    .data2Lane              = 0,           ///< 2 data lane enable
    .line4                  = 0,           ///< 0= not use port as DCX, 1= use port as DCX
    .datSrc                 = 0,           ///< data from camera or memory
    .colorModeIn            = 0,           ///< Input data color mode
    .colorModeOut           = 0,           ///< Output data color mode
    .yAdjEn                 = 0,           ///< Y adjust enable
    .yAdjSel                = 0,           ///< Y adjustment from cspi or sw
    .yAdjBound              = 0,           ///< Y adjustment bound
    .dcDummy                = 0,           ///< Send DCX or dummy
    .busType                = 0,

};

// 718 cmd ctrl
AP_PLAT_COMMON_BSS lspiCmdCtrl_t lspiCmdCtrl =
{
    .wrRdn                  = 0,           ///< 0=rd, 1=wr
    .ramWr                  = 0,           ///< start to fill frame memory
    .rdatDummyCycle         = 0,           ///< Dummy cycle before data read
    .dataLen                = 0,           ///< data len for wr/rd
    .init                   = 0,           ///< always be 0
};

// 718 cmd addr
AP_PLAT_COMMON_BSS lspiCmdAddr_t lspiCmdAddr =
{
    .addr                   = 0,
};

// 718 lspi info
AP_PLAT_COMMON_BSS lspiInfo_t lspiInfo =
{
    .frameHeight            = 0,           ///< frame height
    .frameWidth             = 0,           ///< frame width
};

// 718 tailor info0
AP_PLAT_COMMON_BSS lspitailorInfo0_t lspiTailorInfo0 =
{
    .tailorBottom           = 0,           ///< cut bottom lines
    .tailorTop              = 0,           ///< cut top lines
};

// 718 tailor info
AP_PLAT_COMMON_BSS lspitailorInfo_t lspiTailorInfo =
{
    .tailorLeft             = 0,           ///< cut upper lines
    .tailorRight            = 0,
};

// 718 scale info
AP_PLAT_COMMON_BSS lspiScaleInfo_t lspiScaleInfo =
{
    .rowScaleFrac           = 0,           ///< cut upper lines
    .colScaleFrac           = 0,
};

// 718 quartile ctrl
AP_PLAT_COMMON_BSS lspiQuartileCtrl_t lspiQuartileCtrl =
{
    .grayCtrl               = 0,            ///< gray ctrl
    .quartileSel            = 0,            ///< quartile from cspi or sw
    .quartile1              = 0,            ///< quartile 1
    .quartile2              = 0,            ///< quartile 2
    .quartile3              = 0,            ///< quartile 3
};

// 718 yadj
AP_PLAT_COMMON_BSS lspiYAdj_t lspiYAdj =
{
    .yadjYmin               = 0,            ///< y adj min
    .yadjYmax               = 0,            ///< y adj max
    .yadjStrech             = 0,            ///< y adj stretch
    .yadjStrechFwl          = 0,            ///< y adj stretch fwl
};

// 718 gray page cmd0
AP_PLAT_COMMON_BSS lspiGrayPageCmd0_t lspiGrayPageCmd0 =
{
    .pageCmd                = 0,            ///< page cmd
    .pageCmd0               = 0,            ///< page cmd 0
    .pageCmd01ByteNum       = 0,            ///< page cmd0 + page cmd1 byte num
};

// 718 gray page cmd1
AP_PLAT_COMMON_BSS lspiGrayPageCmd1_t lspiGrayPageCmd1 =
{
    .pageCmd1               = 0,            ///< page cmd1
};

// 718 frame info out
AP_PLAT_COMMON_BSS lspiFrameInfoOut_t lspiFrameInfoOut =
{
    .frameHeightOut         = 0,            ///< frame height out
    .frameWidthOut          = 0,            ///< frame width out
};

// 718 bus sel
AP_PLAT_COMMON_DATA lspiBusSel_t lspiBusSel =
{
    .i2sBusEn               = 0,            ///< I2S bus enable
    .cspiBusEn              = 0,            ///< Cspi bus enable
    .lspiBusEn              = 1,            ///< Lspi bus enable
};


#else // CHIP 719

// 719 Data Format 
AP_PLAT_COMMON_DATA lspiDataFmt_t lspiDataFmt = 
{
    .slaveModeEn            = 0,
    .slotSize               = 0,
    .wordSize               = 7,
    .alignMode              = 0,
    .endianMode             = 0, // 0:LSB  1: MSB
    .dataDly                = 0,
    .txPad                  = 0,
    .rxSignExt              = 0,
    .txPack                 = 0,
    .rxPack                 = 0,
    .txFifoEndianMode       = 0,
    .rxFifoEndianMode       = 0,
    .eorMode                = 0,
};

// 719 DMA Control
AP_PLAT_COMMON_DATA lspiDmaCtrl_t lspiDmaCtrl =
{
    .rxDmaReqEn             = 0,
    .txDmaReqEn             = 0,
    .rxDmaTimeOutEn         = 0,
    .dmaWorkWaitCycle       = 31,
    .rxDmaBurstSizeSub1     = 7,
    .txDmaBurstSizeSub1     = 7,
    .rxDmaThreadHold        = 7,
    .txDmaThreadHold        = 7,
    .rxFifoFlush            = 0,
    .txFifoFlush            = 0
};

// 719 INT Control
AP_PLAT_COMMON_DATA lspiIntCtrl_t lspiIntCtrl =
{
    .txUnderRunIntEn        = 0,
    .txDmaErrIntEn          = 0,
    .txDatIntEn             = 0,
    .rxOverFlowIntEn        = 0,
    .rxDmaErrIntEn          = 0,
    .rxDatIntEn             = 0,
    .rxTimeOutIntEn         = 0,
    .fsErrIntEn             = 0,
    .frameStartIntEn        = 0,
    .frameEndIntEn          = 0,
    .cspiBusTimeOutIntEn    = 0,
    .lspiRamWrBreakIntEn    = 0,
    .lspiRamWrFrameStartEn  = 0,
    .lspiRamWrFrameEndEn    = 0,
    .lspiCmdEndEn           = 0,
    .cspiOtsuEndEn          = 0,
    .lspiRamWrEndEn         = 0,
    .txIntThreshHold        = 8,
    .rxIntThreshHold        = 8,
    .ramWrIntCtrl           = 0,
    .ramWrIntCtrlEn         = 0,
};

// 719 lspi ctrl
AP_PLAT_COMMON_DATA lspiCtrl_t lspiCtrl =
{
    .enable                 = 1,           // lspi Enable
    .dspiEn                 = 0,           // dual spi mode enable
    .line4                  = 0,           // 0= not use port as DCX, 1= use port as DCX
    .datSrc                 = 0,           // data from camera or memory
    .colorModeIn            = 0,           // Input data color mode
    .colorModeOut           = 0,           // Output data color mode
    .yAdjEn                 = 0,           // Y adjust enable
    .yAdjSel                = 0,           // Y adjustment from cspi or sw
    .yAdjBound              = 0,           // Y adjustment bound
    .dcDummy                = 0,           // Send DCX or dummy
    .dspiCfg                = 0,

};

// 719 cmd ctrl
AP_PLAT_COMMON_BSS lspiCmdCtrl_t lspiCmdCtrl =
{
    .wrRdn                  = 0,           // 0=rd, 1=wr
    .ramWr                  = 0,           // start to fill frame memory
    .rdatDummyCycle         = 0,           // Dummy cycle before data read
    .dataLen                = 0,           // data len for wr/rd
    .init                   = 0,           // always be 0
};

// 719 cmd addr
AP_PLAT_COMMON_BSS lspiCmdAddr_t lspiCmdAddr =
{
    .addr                   = 0,
    .csnHighCycleMin        = 0,
    .busType                = 0,
};

// 719 lspi info
AP_PLAT_COMMON_BSS lspiInfo_t lspiInfo =
{
    .frameHeight            = 0,           // frame height
    .frameWidth             = 0,           // frame width
};

// 719 tailor info0
AP_PLAT_COMMON_BSS lspitailorInfo0_t lspiTailorInfo0 =
{
    .tailorBottom           = 0,           // cut bottom lines
    .tailorTop              = 0,           // cut top lines
};

// 719 tailor info
AP_PLAT_COMMON_BSS lspitailorInfo_t lspiTailorInfo =
{
    .tailorLeft             = 0,           // cut upper lines
    .tailorRight            = 0,
};

// 719 scale info
AP_PLAT_COMMON_BSS lspiScaleInfo_t lspiScaleInfo =
{
    .rowScaleFrac           = 0,           // cut upper lines
    .colScaleFrac           = 0,
};

// 719 quartile ctrl
AP_PLAT_COMMON_BSS lspiQuartileCtrl_t lspiQuartileCtrl =
{
    .grayCtrl               = 0,            // gray ctrl
    .quartileSel            = 0,            // quartile from cspi or sw
    .quartile1              = 0,            // quartile 1
    .quartile2              = 0,            // quartile 2
    .quartile3              = 0,            // quartile 3
};

// 719 yadj
AP_PLAT_COMMON_BSS lspiYAdj_t lspiYAdj =
{
    .yadjYmin               = 0,            // y adj min
    .yadjYmax               = 0,            // y adj max
    .yadjStrech             = 0,            // y adj stretch
    .yadjStrechFwl          = 0,            // y adj stretch fwl
};

// 719 gray page cmd0
AP_PLAT_COMMON_BSS lspiGrayPageCmd0_t lspiGrayPageCmd0 =
{
    .pageCmd                = 0,            // page cmd
    .pageCmd0               = 0,            // page cmd 0
    .pageCmd01ByteNum       = 0,            // page cmd0 + page cmd1 byte num
};

// 719 gray page cmd1
AP_PLAT_COMMON_BSS lspiGrayPageCmd1_t lspiGrayPageCmd1 =
{
    .pageCmd1               = 0,            // page cmd1
};

// 719 frame info out
AP_PLAT_COMMON_BSS lspiFrameInfoOut_t lspiFrameInfoOut =
{
    .frameHeightOut         = 0,            // frame height out
    .frameWidthOut          = 0,            // frame width out
};

// 719 mspi ctrl
AP_PLAT_COMMON_BSS lspiMspiCtrl_t lspiMspiCtrl = 
{
    .mspiEn                 = 0,            // I2S bus enable
    .mspiAddrLane           = 0,            // Cspi bus enable
    .mspiDataLane           = 0,            // Lspi bus enable
    .rsv                    = 0,
    .mspiInst               = 0,            // Lspi bus enable
    .mspiVsyncEn            = 0,            // Lspi bus enable
    .vsyncLineCycle         = 0,            // Lspi bus enable
};

// 719 vsynctrl
AP_PLAT_COMMON_BSS lspiVsyncCtrl_t lspiVsyncCtrl = 
{
    .hsyncInst               = 0,
    .hsyncAddr               = 0,
    .vbp                     = 0,
    //.vfp                     = 0,
};

// 719 8080 ctrl
AP_PLAT_COMMON_BSS lspi8080Ctrl_t lspi8080Ctrl = 
{
    .lspi8080En              = 0,
    .vfp                     = 0,
};

// 719 pre param0
AP_PLAT_COMMON_BSS lspiCmdPreParam0_t lspiPreParam0 = 
{
    .lspiCmd0PreEn          = 0,            // Lspi cmd0 pre en
    .lspiCmd1PreEn          = 0,            // Lspi cmd1 pre en
    .lspiCmd0PreParaLen     = 0,            // Lspi cmd0 pre param len
    .lspiCmd1PreParaLen     = 0,            // Lspi cmd1 pre param len
    .lspiCmdPreMspiAddrLane = 0,            // Lspi cmd pre mspi addr lane
    .lspiCmdPreMspiDataLane = 0,            // Lspi cmd pre mspi data lane
};

AP_PLAT_COMMON_BSS lspiCmdPreParam2_t lspiPreParam2 = 
{
    .lspiCmd0PrePara        = 0,            // Lspi pre cmd param0
};

AP_PLAT_COMMON_BSS lspiCmdPreParam3_t lspiPreParam3 = 
{
    .lspiCmd1PrePara        = 0,            // Lspi pre cmd param1
};


// 719 post param0
AP_PLAT_COMMON_BSS lspiCmdPostParam0_t lspiPostParam0 = 
{
    .lspiCmd0PostEn          = 0,            // Lspi cmd0 Post en
    .lspiCmd1PostEn          = 0,            // Lspi cmd1 Post en
    .lspiCmd0PostParaLen     = 0,            // Lspi cmd0 Post param len
    .lspiCmd1PostParaLen     = 0,            // Lspi cmd1 Post param len
    .lspiCmdPostMspiAddrLane = 0,            // Lspi cmd Post mspi addr lane
    .lspiCmdPostMspiDataLane = 0,            // Lspi cmd Post mspi data lane
};

// 719 te param0
AP_PLAT_COMMON_BSS lspiTeParam0_t lspiTeParam0 = 
{
    .lspiTeEn          		 = 0,            // Lspi te en
    .lspiTeEdgeSel           = 0,            // Lspi te edge sel
    .lspiTePos0			     = 0,            // Lspi te pos0
};

// 719 te param1
AP_PLAT_COMMON_BSS lspiTeParam1_t lspiTeParam1 = 
{
    .lspiTePos1			     = 0,            // Lspi te pos1
};

// 719 bus sel
AP_PLAT_COMMON_DATA lspiBusSel_t lspiBusSel =
{
    .i2sBusEn               = 0,            // I2S bus enable
    .cspiBusEn              = 0,            // Cspi bus enable
    .lspiBusEn              = 1,            // Lspi bus enable
};
#endif

//////////////////////////////////////////////////////////////////////////////////////////////
// LSPI Setting field End
//////////////////////////////////////////////////////////////////////////////////////////////

static LSPI_TypeDef* const lspiInstance[LSPI_INSTANCE_NUM] = {LSPI1, LSPI2};

AP_PLAT_COMMON_DATA static ClockId_e lspiClk[LSPI_INSTANCE_NUM * 2] =
{
    PCLK_USP1,
    FCLK_USP1,
    PCLK_USP2,
    FCLK_USP2
};

#ifdef PM_FEATURE_ENABLE
/**
  \brief spi initialization counter, for lower power callback register/de-register
 */
AP_PLAT_COMMON_BSS static uint32_t lspiInitCnt = 0;

/**
  \brief Bitmap of LSPI working status, each instance is assigned 2 bits representing tx and rx status,
         when all LSPI instances are not working, we can vote to enter to low power state.
 */
AP_PLAT_COMMON_BSS static uint32_t lspiWorkingStats = 0;

/** \brief Internal used data structure */
typedef struct
{
    bool              isInited;                       /**< Whether spi has been initialized */

    struct
    {
#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
		__IO uint32_t DFMT;
		__IO uint32_t RSVD1[2];
		__IO uint32_t DMACTL;
		__IO uint32_t INTCTL;						  /**< Interrupt Control Register,			offset: 0x10 */
		__IO uint32_t RSVD2;
		__IO uint32_t STAS; 						  /**< Status Register, 					offset: 0x18 */
		__IO uint32_t RFIFO;						  /**< RFIFO,								offset: 0x1C */
		__IO uint32_t TFIFO;						  /**< TFIFO,								offset: 0x20 */
		__IO uint32_t RSVD7;
		__IO uint32_t CSPICTL;
		__IO uint32_t RSVD4[13];
		__IO uint32_t LSPI_CTRL;					  /**< LSPI control 						offset: 0x60 */
		__IO uint32_t LSPI_CCTRL;					  /**< LSPI command control 				offset: 0x64 */
		__IO uint32_t LSPI_CADDR;					  /**< LSPI command addr					offset: 0x68 */
		__IO uint32_t LSPI_STAT;					  /**< LSPI status							offset: 0x6c */
		__IO uint32_t LSPI_RAMWLEN; 				  /**< LSPI ram write len					offset: 0x70 */
		__IO uint32_t LSPFINFO; 					  /**< LSPI frame info						offset: 0x74 */
		__IO uint32_t LSPTINFO0;					  /**< LSPI tailor info0					offset: 0x78 */
		__IO uint32_t LSPTINFO; 					  /**< LSPI tailor info 					offset: 0x7c */
		__IO uint32_t LSPSINFO; 					  /**< LSPI scale info						offset: 0x80 */
		__IO uint32_t LSPIQUARTCTRL;				  /**< LSPI quartile ctrl					offset: 0x84 */
		__IO uint32_t LSPIQUARTUSE; 				  /**< LSPI quartile inuse					offset: 0x88 */
		__IO uint32_t LSPIYADJ; 					  /**< LSPI Y adj							offset: 0x8c */
		__IO uint32_t LSPIYADJUSE;					  /**< LSPI Y adj inuse 					offset: 0x90 */
		__IO uint32_t LSPIGPCMD0;					  /**< LSPI gray page cmd0					offset: 0x94 */
		__IO uint32_t LSPIGPCMD1;					  /**< LSPI gray page cmd1					offset: 0x98 */
		__IO uint32_t LSPFINFO0;					  /**< LSPI frame info out					offset: 0x9c */
		__IO uint32_t YUV2RGBINFO0; 				  /**< YUV to RGB info0 					offset: 0xa0 */
		__IO uint32_t YUV2RGBINFO1; 				  /**< YUV to RGB info1 					offset: 0xa4 */
		__IO uint32_t RSVD5[14];					  /**< Reserved 										 */
		__IO uint32_t I2SBUSSEL;					  /**< LSPI bus select						offset: 0xe0 */
		__IO uint32_t RSVD6[4]; 					  /**< Reserved 										 */
#else
		__IO uint32_t DFMT;
		__IO uint32_t RSVD1[2];
		__IO uint32_t DMACTL;
		__IO uint32_t INTCTL;						  /**< Interrupt Control Register,			offset: 0x10 */
		__IO uint32_t RSVD2;
		__IO uint32_t STAS; 						  /**< Status Register, 					offset: 0x18 */
		__IO uint32_t RFIFO;						  /**< RFIFO,								offset: 0x1C */
		__IO uint32_t TFIFO;						  /**< TFIFO,								offset: 0x20 */
		__IO uint32_t RSVD7;
		__IO uint32_t CSPICTL;
		__IO uint32_t RSVD4[13];
		__IO uint32_t LSPI_CTRL;					  /**< LSPI control 						offset: 0x60 */
		__IO uint32_t LSPI_CCTRL;					  /**< LSPI command control 				offset: 0x64 */
		__IO uint32_t LSPI_CADDR;					  /**< LSPI command addr					offset: 0x68 */
		__IO uint32_t LSPI_STAT;					  /**< LSPI status							offset: 0x6c */
		__IO uint32_t LSPI_RAMWLEN; 				  /**< LSPI ram write len					offset: 0x70 */
		__IO uint32_t LSPFINFO; 					  /**< LSPI frame info						offset: 0x74 */
		__IO uint32_t LSPTINFO0;					  /**< LSPI tailor info0					offset: 0x78 */
		__IO uint32_t LSPTINFO; 					  /**< LSPI tailor info 					offset: 0x7c */
		__IO uint32_t LSPSINFO; 					  /**< LSPI scale info						offset: 0x80 */
		__IO uint32_t LSPIQUARTCTRL;				  /**< LSPI quartile ctrl					offset: 0x84 */
		__IO uint32_t LSPIQUARTUSE; 				  /**< LSPI quartile inuse					offset: 0x88 */
		__IO uint32_t LSPIYADJ; 					  /**< LSPI Y adj							offset: 0x8c */
		__IO uint32_t LSPIYADJUSE;					  /**< LSPI Y adj inuse 					offset: 0x90 */
		__IO uint32_t LSPIGPCMD0;					  /**< LSPI gray page cmd0					offset: 0x94 */
		__IO uint32_t LSPIGPCMD1;					  /**< LSPI gray page cmd1					offset: 0x98 */
		__IO uint32_t LSPFINFO0;					  /**< LSPI frame info out					offset: 0x9c */
		__IO uint32_t YUV2RGBINFO0; 				  /**< YUV to RGB info0 					offset: 0xa0 */
		__IO uint32_t YUV2RGBINFO1; 				  /**< YUV to RGB info1 					offset: 0xa4 */
		__IO uint32_t DEBUG;						  /**< lspi debug							offset: 0xa8 */
		__IO uint32_t MSPI_CTRL;					  /**< lspi mspi ctrl						offset: 0xac */
		__IO uint32_t VSYNC_CTRL;					  /**< lspi vsync ctrl						offset: 0xb0 */
		__IO uint32_t LSPI8080CTRL; 				  /**< lspi 8080 ctrl						offset: 0xb4 */
		__IO uint32_t LSPICMDPREPARA0;				  /**< lspi pre cmd param0				    offset: 0xb8 */
		__IO uint32_t LSPICMDPREPARA1;				  /**< lspi pre cmd param1					offset: 0xbc */
		__IO uint32_t LSPICMDPREPARA2;				  /**< lspi pre cmd param2					offset: 0xc0 */
		__IO uint32_t LSPICMDPREPARA3;				  /**< lspi pre cmd param3					offset: 0xc4 */
		__IO uint32_t LSPICMDPOSTPARA0; 			  /**< lspi post cmd param0 				offset: 0xc8 */
		__IO uint32_t LSPICMDPOSTPARA1; 			  /**< lspi post cmd param1 				offset: 0xcc */
		__IO uint32_t LSPICMDPOSTPARA2; 			  /**< lspi post cmd param2 				offset: 0xd0 */
		__IO uint32_t LSPICMDPOSTPARA3; 			  /**< lspi post cmd param3 				offset: 0xd4 */
		__IO uint32_t RSVD5;						  /**< Reserved 										 */
		__IO uint32_t LSPITEPARA0;					  /**< lspi te param0						offset: 0xdc */
		__IO uint32_t LSPITEPARA1;					  /**< lspi te param1						offset: 0xe0 */
		__IO uint32_t I2SBUSSEL;					  /**< LSPI bus select						offset: 0xe4 */
		__IO uint32_t RSVD6[3]; 					  /**< Reserved 										 */
		__IO uint32_t USPVERSION;					  /**< USP version						    offset: 0xf4 */
#endif
    }regsBackup;
} lspiDataBase_t;

AP_PLAT_COMMON_BSS static lspiDataBase_t lspiDataBase[LSPI_INSTANCE_NUM] = {0};

/**
  \fn        static void lspiEnterLowPowerStatePrepare(void* pdata, slpManLpState state)
  \brief     Perform necessary preparations before sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LSPI hareware is repowered, we backup
             some registers here first so that we can restore user's configurations after exit.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void lspiEnterLpStatePrepare(void* pdata, slpManLpState state)
{
    uint32_t i;
    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < LSPI_INSTANCE_NUM; i++)
            {
                if(lspiDataBase[i].isInited == true)
                {
#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    lspiDataBase[i].regsBackup.DFMT         	= lspiInstance[i]->DFMT;
                    lspiDataBase[i].regsBackup.DMACTL      		= lspiInstance[i]->DMACTL;
                    lspiDataBase[i].regsBackup.INTCTL       	= lspiInstance[i]->INTCTL;
                    lspiDataBase[i].regsBackup.STAS       		= lspiInstance[i]->STAS;
                    lspiDataBase[i].regsBackup.INTCTL       	= lspiInstance[i]->INTCTL;
                    lspiDataBase[i].regsBackup.CSPICTL   		= lspiInstance[i]->CSPICTL;
                    lspiDataBase[i].regsBackup.LSPI_CTRL      	= lspiInstance[i]->LSPI_CTRL;
                    lspiDataBase[i].regsBackup.LSPI_CCTRL   	= lspiInstance[i]->LSPI_CCTRL;
                    lspiDataBase[i].regsBackup.LSPI_CADDR   	= lspiInstance[i]->LSPI_CADDR;
                    lspiDataBase[i].regsBackup.LSPI_STAT    	= lspiInstance[i]->LSPI_STAT;
                    lspiDataBase[i].regsBackup.LSPI_RAMWLEN 	= lspiInstance[i]->LSPI_RAMWLEN;
                    lspiDataBase[i].regsBackup.LSPFINFO     	= lspiInstance[i]->LSPFINFO;
                    lspiDataBase[i].regsBackup.LSPTINFO0    	= lspiInstance[i]->LSPTINFO0;
                    lspiDataBase[i].regsBackup.LSPTINFO     	= lspiInstance[i]->LSPTINFO;
                    lspiDataBase[i].regsBackup.LSPSINFO      	= lspiInstance[i]->LSPSINFO;                    
					lspiDataBase[i].regsBackup.LSPIQUARTCTRL	= lspiInstance[i]->LSPIQUARTCTRL;
                    lspiDataBase[i].regsBackup.LSPIQUARTUSE     = lspiInstance[i]->LSPIQUARTUSE;
                    lspiDataBase[i].regsBackup.LSPIYADJ       	= lspiInstance[i]->LSPIYADJ;
                    lspiDataBase[i].regsBackup.LSPIYADJUSE      = lspiInstance[i]->LSPIYADJUSE;
                    lspiDataBase[i].regsBackup.LSPIGPCMD0       = lspiInstance[i]->LSPIGPCMD0;
                    lspiDataBase[i].regsBackup.LSPIGPCMD1   	= lspiInstance[i]->LSPIGPCMD1;
                    lspiDataBase[i].regsBackup.LSPFINFO0        = lspiInstance[i]->LSPFINFO0;
                    lspiDataBase[i].regsBackup.YUV2RGBINFO0     = lspiInstance[i]->YUV2RGBINFO0;
                    lspiDataBase[i].regsBackup.YUV2RGBINFO1     = lspiInstance[i]->YUV2RGBINFO1;
                    lspiDataBase[i].regsBackup.I2SBUSSEL    	= lspiInstance[i]->I2SBUSSEL;
#else // chip 719
					lspiDataBase[i].regsBackup.DFMT         	= lspiInstance[i]->DFMT;
                    lspiDataBase[i].regsBackup.DMACTL      		= lspiInstance[i]->DMACTL;
                    lspiDataBase[i].regsBackup.INTCTL       	= lspiInstance[i]->INTCTL;
                    lspiDataBase[i].regsBackup.STAS       		= lspiInstance[i]->STAS;
                    lspiDataBase[i].regsBackup.INTCTL       	= lspiInstance[i]->INTCTL;
                    lspiDataBase[i].regsBackup.CSPICTL   		= lspiInstance[i]->CSPICTL;
                    lspiDataBase[i].regsBackup.LSPI_CTRL    	= lspiInstance[i]->LSPI_CTRL;
                    lspiDataBase[i].regsBackup.LSPI_CCTRL   	= lspiInstance[i]->LSPI_CCTRL;
                    lspiDataBase[i].regsBackup.LSPI_CADDR   	= lspiInstance[i]->LSPI_CADDR;
                    lspiDataBase[i].regsBackup.LSPI_STAT    	= lspiInstance[i]->LSPI_STAT;
                    lspiDataBase[i].regsBackup.LSPI_RAMWLEN 	= lspiInstance[i]->LSPI_RAMWLEN;
                    lspiDataBase[i].regsBackup.LSPFINFO     	= lspiInstance[i]->LSPFINFO;
                    lspiDataBase[i].regsBackup.LSPTINFO0    	= lspiInstance[i]->LSPTINFO0;
                    lspiDataBase[i].regsBackup.LSPTINFO     	= lspiInstance[i]->LSPTINFO;
                    lspiDataBase[i].regsBackup.LSPSINFO      	= lspiInstance[i]->LSPSINFO;                    
					lspiDataBase[i].regsBackup.LSPIQUARTCTRL	= lspiInstance[i]->LSPIQUARTCTRL;
                    lspiDataBase[i].regsBackup.LSPIQUARTUSE     = lspiInstance[i]->LSPIQUARTUSE;
                    lspiDataBase[i].regsBackup.LSPIYADJ       	= lspiInstance[i]->LSPIYADJ;
                    lspiDataBase[i].regsBackup.LSPIYADJUSE      = lspiInstance[i]->LSPIYADJUSE;
                    lspiDataBase[i].regsBackup.LSPIGPCMD0       = lspiInstance[i]->LSPIGPCMD0;
                    lspiDataBase[i].regsBackup.LSPIGPCMD1   	= lspiInstance[i]->LSPIGPCMD1;
                    lspiDataBase[i].regsBackup.LSPFINFO0        = lspiInstance[i]->LSPFINFO0;
                    lspiDataBase[i].regsBackup.YUV2RGBINFO0     = lspiInstance[i]->YUV2RGBINFO0;
                    lspiDataBase[i].regsBackup.YUV2RGBINFO1     = lspiInstance[i]->YUV2RGBINFO1;
                    lspiDataBase[i].regsBackup.DEBUG     		= lspiInstance[i]->DEBUG;
                    lspiDataBase[i].regsBackup.MSPI_CTRL     	= lspiInstance[i]->MSPI_CTRL;
                    lspiDataBase[i].regsBackup.VSYNC_CTRL     	= lspiInstance[i]->VSYNC_CTRL;
                    lspiDataBase[i].regsBackup.LSPI8080CTRL     = lspiInstance[i]->LSPI8080CTRL;
                    lspiDataBase[i].regsBackup.LSPICMDPREPARA0  = lspiInstance[i]->LSPICMDPREPARA0;
                    lspiDataBase[i].regsBackup.LSPICMDPREPARA1  = lspiInstance[i]->LSPICMDPREPARA1;
                    lspiDataBase[i].regsBackup.LSPICMDPREPARA2  = lspiInstance[i]->LSPICMDPREPARA2;
                    lspiDataBase[i].regsBackup.LSPICMDPREPARA3  = lspiInstance[i]->LSPICMDPREPARA3;
                    lspiDataBase[i].regsBackup.LSPICMDPOSTPARA0 = lspiInstance[i]->LSPICMDPOSTPARA0;
                    lspiDataBase[i].regsBackup.LSPICMDPOSTPARA1 = lspiInstance[i]->LSPICMDPOSTPARA1;
                    lspiDataBase[i].regsBackup.LSPICMDPOSTPARA2 = lspiInstance[i]->LSPICMDPOSTPARA2;
                    lspiDataBase[i].regsBackup.LSPICMDPOSTPARA3 = lspiInstance[i]->LSPICMDPOSTPARA3;
                    lspiDataBase[i].regsBackup.LSPITEPARA0     	= lspiInstance[i]->LSPITEPARA0;
                    lspiDataBase[i].regsBackup.LSPITEPARA1     	= lspiInstance[i]->LSPITEPARA1;
                    lspiDataBase[i].regsBackup.I2SBUSSEL     	= lspiInstance[i]->I2SBUSSEL;
                    lspiDataBase[i].regsBackup.USPVERSION     	= lspiInstance[i]->USPVERSION;
#endif
                }
            }

            break;
        default:
            break;
    }
}

/**
  \fn        static void lspiExitLowPowerStateRestore(void* pdata, slpManLpState state)
  \brief     Restore after exit from sleep.
             After recovering from SLPMAN_SLEEP1_STATE, LSPI hareware is repowered, we restore user's configurations
             by aidding of the stored registers.
  \param[in] pdata pointer to user data, not used now
  \param[in] state low power state
 */
static void lspiExitLpStateRestore(void* pdata, slpManLpState state)
{
    uint32_t i;
    switch (state)
    {
        case SLPMAN_SLEEP1_STATE:

            for(i = 0; i < LSPI_INSTANCE_NUM; i++)
            {
                if(lspiDataBase[i].isInited == true)
                {
                    GPR_clockEnable(lspiClk[2*i]);
                    GPR_clockEnable(lspiClk[2*i+1]);
					lspiSetBusSpeed(bpsBak, NULL);

#if ((defined CHIP_EC718) && !(defined TYPE_EC718M)) || (defined CHIP_EC716)
                    lspiInstance[i]->DFMT 						=lspiDataBase[i].regsBackup.DFMT;
                    lspiInstance[i]->DMACTL 					= lspiDataBase[i].regsBackup.DMACTL;
                    lspiInstance[i]->INTCTL 					= lspiDataBase[i].regsBackup.INTCTL;
                    lspiInstance[i]->STAS 						= lspiDataBase[i].regsBackup.STAS;
                    lspiInstance[i]->INTCTL 					= lspiDataBase[i].regsBackup.INTCTL;
                    lspiInstance[i]->CSPICTL 					= lspiDataBase[i].regsBackup.CSPICTL;
                    lspiInstance[i]->LSPI_CTRL 					= lspiDataBase[i].regsBackup.LSPI_CTRL;
                    lspiInstance[i]->LSPI_CCTRL					= lspiDataBase[i].regsBackup.LSPI_CCTRL;
                    lspiInstance[i]->LSPI_CADDR 				= lspiDataBase[i].regsBackup.LSPI_CADDR;
                    lspiInstance[i]->LSPI_STAT 					= lspiDataBase[i].regsBackup.LSPI_STAT;
                    lspiInstance[i]->LSPI_RAMWLEN 				= lspiDataBase[i].regsBackup.LSPI_RAMWLEN;
                    lspiInstance[i]->LSPFINFO 					= lspiDataBase[i].regsBackup.LSPFINFO;
                    lspiInstance[i]->LSPTINFO0 					= lspiDataBase[i].regsBackup.LSPTINFO0;
                    lspiInstance[i]->LSPTINFO 					= lspiDataBase[i].regsBackup.LSPTINFO;
                    lspiInstance[i]->LSPSINFO 					= lspiDataBase[i].regsBackup.LSPSINFO;                    
					lspiInstance[i]->LSPIQUARTCTRL 				= lspiDataBase[i].regsBackup.LSPIQUARTCTRL;
                    lspiInstance[i]->LSPIQUARTUSE 				= lspiDataBase[i].regsBackup.LSPIQUARTUSE;
                    lspiInstance[i]->LSPIYADJ 					= lspiDataBase[i].regsBackup.LSPIYADJ;
                    lspiInstance[i]->LSPIYADJUSE 				= lspiDataBase[i].regsBackup.LSPIYADJUSE;
                    lspiInstance[i]->LSPIGPCMD0 				= lspiDataBase[i].regsBackup.LSPIGPCMD0;
                    lspiInstance[i]->LSPIGPCMD1 				= lspiDataBase[i].regsBackup.LSPIGPCMD1;
                    lspiInstance[i]->LSPFINFO0 					= lspiDataBase[i].regsBackup.LSPFINFO0;
                    lspiInstance[i]->YUV2RGBINFO0 				= lspiDataBase[i].regsBackup.YUV2RGBINFO0;
                    lspiInstance[i]->YUV2RGBINFO1 				= lspiDataBase[i].regsBackup.YUV2RGBINFO1;
                    lspiInstance[i]->I2SBUSSEL 					= lspiDataBase[i].regsBackup.I2SBUSSEL;
#else // chip 719
                    lspiInstance[i]->DFMT 						=lspiDataBase[i].regsBackup.DFMT;
                    lspiInstance[i]->DMACTL 					= lspiDataBase[i].regsBackup.DMACTL;
                    lspiInstance[i]->INTCTL 					= lspiDataBase[i].regsBackup.INTCTL;
                    lspiInstance[i]->STAS 						= lspiDataBase[i].regsBackup.STAS;
                    lspiInstance[i]->INTCTL 					= lspiDataBase[i].regsBackup.INTCTL;
                    lspiInstance[i]->CSPICTL 					= lspiDataBase[i].regsBackup.CSPICTL;
                    lspiInstance[i]->LSPI_CTRL 					= lspiDataBase[i].regsBackup.LSPI_CTRL;
                    lspiInstance[i]->LSPI_CCTRL					= lspiDataBase[i].regsBackup.LSPI_CCTRL;
                    lspiInstance[i]->LSPI_CADDR 				= lspiDataBase[i].regsBackup.LSPI_CADDR;
                    lspiInstance[i]->LSPI_STAT 					= lspiDataBase[i].regsBackup.LSPI_STAT;
                    lspiInstance[i]->LSPI_RAMWLEN 				= lspiDataBase[i].regsBackup.LSPI_RAMWLEN;
                    lspiInstance[i]->LSPFINFO 					= lspiDataBase[i].regsBackup.LSPFINFO;
                    lspiInstance[i]->LSPTINFO0 					= lspiDataBase[i].regsBackup.LSPTINFO0;
                    lspiInstance[i]->LSPTINFO 					= lspiDataBase[i].regsBackup.LSPTINFO;
                    lspiInstance[i]->LSPSINFO 					= lspiDataBase[i].regsBackup.LSPSINFO;                    
					lspiInstance[i]->LSPIQUARTCTRL 				= lspiDataBase[i].regsBackup.LSPIQUARTCTRL;
                    lspiInstance[i]->LSPIQUARTUSE 				= lspiDataBase[i].regsBackup.LSPIQUARTUSE;
                    lspiInstance[i]->LSPIYADJ 					= lspiDataBase[i].regsBackup.LSPIYADJ;
                    lspiInstance[i]->LSPIYADJUSE 				= lspiDataBase[i].regsBackup.LSPIYADJUSE;
                    lspiInstance[i]->LSPIGPCMD0 				= lspiDataBase[i].regsBackup.LSPIGPCMD0;
                    lspiInstance[i]->LSPIGPCMD1 				= lspiDataBase[i].regsBackup.LSPIGPCMD1;
                    lspiInstance[i]->LSPFINFO0 					= lspiDataBase[i].regsBackup.LSPFINFO0;
                    lspiInstance[i]->YUV2RGBINFO0 				= lspiDataBase[i].regsBackup.YUV2RGBINFO0;
                    lspiInstance[i]->YUV2RGBINFO1 				= lspiDataBase[i].regsBackup.YUV2RGBINFO1;
                    lspiInstance[i]->DEBUG 						= lspiDataBase[i].regsBackup.DEBUG;
                    lspiInstance[i]->MSPI_CTRL 					= lspiDataBase[i].regsBackup.MSPI_CTRL;
                    lspiInstance[i]->VSYNC_CTRL 				= lspiDataBase[i].regsBackup.VSYNC_CTRL;
                    lspiInstance[i]->LSPI8080CTRL 				= lspiDataBase[i].regsBackup.LSPI8080CTRL;
                    lspiInstance[i]->LSPICMDPREPARA0 			= lspiDataBase[i].regsBackup.LSPICMDPREPARA0;
                    lspiInstance[i]->LSPICMDPREPARA1 			= lspiDataBase[i].regsBackup.LSPICMDPREPARA1;
                    lspiInstance[i]->LSPICMDPREPARA2 			= lspiDataBase[i].regsBackup.LSPICMDPREPARA2;
                    lspiInstance[i]->LSPICMDPREPARA3 			= lspiDataBase[i].regsBackup.LSPICMDPREPARA3;
                    lspiInstance[i]->LSPICMDPOSTPARA0 			= lspiDataBase[i].regsBackup.LSPICMDPOSTPARA0;
                    lspiInstance[i]->LSPICMDPOSTPARA1 			= lspiDataBase[i].regsBackup.LSPICMDPOSTPARA1;
                    lspiInstance[i]->LSPICMDPOSTPARA2 			= lspiDataBase[i].regsBackup.LSPICMDPOSTPARA2;
                    lspiInstance[i]->LSPICMDPOSTPARA3 			= lspiDataBase[i].regsBackup.LSPICMDPOSTPARA3;
                    lspiInstance[i]->LSPITEPARA0 				= lspiDataBase[i].regsBackup.LSPITEPARA0;
                    lspiInstance[i]->LSPITEPARA1 				= lspiDataBase[i].regsBackup.LSPITEPARA1;
                    lspiInstance[i]->I2SBUSSEL 					= lspiDataBase[i].regsBackup.I2SBUSSEL;
                    lspiInstance[i]->USPVERSION 				= lspiDataBase[i].regsBackup.USPVERSION;
#endif
                }
            }

			if (lcdSlp1CbFn)
            {
                lcdSlp1CbFn();
            }
            break;

        default:
            break;
    }
}

#define  LOCK_SLEEP(instance)                                                                   \
                                    do                                                          \
                                    {                                                           \
                                    }                                                           \
                                    while(0)

#define  CHECK_TO_UNLOCK_SLEEP(instance)                                                        \
                                    do                                                          \
                                    {                                                           \
                                    }                                                           \
                                    while(0)
#endif


#if (RTE_LSPI2)
static lspiRteInfo_t lspi2Info = {0};
static lspiRes_t lspi2Res = {
    LSPI2,
    {    
#if (LCD_INTERFACE_SPI == 1)
        &lspi2Ds,
        &lspi2Clk,
        &lspi2Cs,
        &lspi2Miso,
        &lspi2Mosi0,
#if (SPI_2_DATA_LANE == 1)
        &lspi2Mosi1,
#endif

#elif (LCD_INTERFACE_MSPI == 1)
        &lspi2Scl,
        &lspi2Cs,
        &lspi2Sdi,
        &lspi2D0,
        &lspi2D1,
        &lspi2D2,
        &lspi2D3,
        
#elif (LCD_INTERFACE_8080 == 1)
        &lspi2Scl,
        &lspi2Cs,
        &lspi2Dcx,
        &lspi2Rclk,
        &lspi2D0,
        &lspi2D1,
        &lspi2D2,
        &lspi2D3,        
        &lspi2D4,
        &lspi2D5,
        &lspi2D6,
        &lspi2D7,
#endif
    },
    NULL,
    &lspi2Info
};
#endif

/**
  \fn          static uint32_t lspiGetInstanceNum(lspiRes_t *lspi)
  \brief       Get instance number
  \param[in]   spi       Pointer to LSPI resources. 0: LSPI1;  1: LSPI2
  \returns     instance number
*/
static uint32_t lspiGetInstanceNum(lspiRes_t *lspi)
{
    return ((uint32_t)lspi->reg - (uint32_t)(LSPI_TypeDef *)MP_USP1_BASE_ADDR) >> 12U;
}

/**
  \fn          static int32_t lspiSetBusSpeed(uint32_t bps, lspiRes_t *lspi)
  \brief       Set bus speed
  \param[in]   bps       bus speed to set
  \param[in]   spi       Pointer to SPI resources
  \return      \ref execution_status
*/
static int32_t lspiSetBusSpeed(uint32_t bps, lspiRes_t *lspi)
{       
    lspiDiv = 1;
	bpsBak = bps;

    if(bps < 102*1024*1024) 
    {
        lspiDiv = (uint8_t)((102*1024*1024)/bps);
    }
    
    if(bps < 1024*1024)
    {
        lspiDiv = 12;
    }

  
#if (defined CHIP_EC718)  && !(defined TYPE_EC718M) || (defined CHIP_EC716)
    CLOCK_setClockSrc(CLK_APB_MP,CLK_APB_MP_SEL_102M);
    CLOCK_clockEnable(CLK_HF102M);
    CLOCK_setClockSrc(FCLK_USP2,FCLK_USP2_SEL_102M);
    CLOCK_setClockDiv(FCLK_USP2, lspiDiv);
    CLOCK_clockEnable(FCLK_USP2);
#else
    CLOCK_setClockSrc(CLK_APB_MP,CLK_APB_MP_SEL_102M);
    CLOCK_clockEnable(CLK_HF102M);
    CLOCK_setClockSrc(FCLK_USP2,FCLK_USP2_SEL_612M);
    CLOCK_setClockDiv(FCLK_USP2, lspiDiv*6);
    CLOCK_clockEnable(FCLK_USP2);
#endif

    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t lspiInit(lspiRes_t *lspi)
  \brief       Initialize SPI Interface.
  \param[in]   spi       Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiInit(lspiRes_t *lspi)
{
    int32_t returnCode;
    PadConfig_t config;

#ifdef PM_FEATURE_ENABLE
    uint32_t instance = lspiGetInstanceNum(lspi);
    lspiDataBase[instance].isInited = true;
    
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, false);
#endif

    // Initialize LSPI PINS
    PAD_getDefaultConfig(&config);
    
#if (LCD_INTERFACE_SPI == 1)
    config.mux = lspi->pins.pinDs->funcNum;
    PAD_setPinConfig(lspi->pins.pinDs->pinNum, &config);
    
    config.mux = lspi->pins.pinClk->funcNum;
    PAD_setPinConfig(lspi->pins.pinClk->pinNum, &config);
    
    config.mux = lspi->pins.pinCs->funcNum;
    PAD_setPinConfig(lspi->pins.pinCs->pinNum, &config);

#if ((LCD_INTERFACE == SPI_3W_II) || ((LCD_INTERFACE == SPI_4W_II)))
    config.mux = lspi->pins.pinMiso->funcNum;
    PAD_setPinConfig(lspi->pins.pinMiso->pinNum, &config);
#endif
    
    config.mux = lspi->pins.pinMosi0->funcNum;
    PAD_setPinConfig(lspi->pins.pinMosi0->pinNum, &config);

#if (SPI_2_DATA_LANE == 1)
    config.mux = lspi->pins.pinMosi1->funcNum;
    PAD_setPinConfig(lspi->pins.pinMosi1->pinNum, &config);
#endif
    
#elif (LCD_INTERFACE_MSPI == 1)
    config.mux = lspi->pins.pinScl->funcNum;
    PAD_setPinConfig(lspi->pins.pinScl->pinNum, &config);
    
    config.mux = lspi->pins.pinCs->funcNum;
    PAD_setPinConfig(lspi->pins.pinCs->pinNum, &config);
    
    config.mux = lspi->pins.pinDin->funcNum;
    PAD_setPinConfig(lspi->pins.pinDin->pinNum, &config);
    
    config.mux = lspi->pins.pinD0->funcNum;
    PAD_setPinConfig(lspi->pins.pinD0->pinNum, &config);
    
    config.mux = lspi->pins.pinD1->funcNum;
    PAD_setPinConfig(lspi->pins.pinD1->pinNum, &config);
    
    config.mux = lspi->pins.pinD2->funcNum;
    PAD_setPinConfig(lspi->pins.pinD2->pinNum, &config);
    
    config.mux = lspi->pins.pinD3->funcNum;
    PAD_setPinConfig(lspi->pins.pinD3->pinNum, &config);
            
#elif (LCD_INTERFACE_8080 == 1)
    config.mux = lspi->pins.pinScl->funcNum;
    PAD_setPinConfig(lspi->pins.pinScl->pinNum, &config);
    
    config.mux = lspi->pins.pinCs->funcNum;
    PAD_setPinConfig(lspi->pins.pinCs->pinNum, &config);
    
    config.mux = lspi->pins.pinDcx->funcNum;
    PAD_setPinConfig(lspi->pins.pinDcx->pinNum, &config);
    
    config.mux = lspi->pins.pinRclk->funcNum;
    PAD_setPinConfig(lspi->pins.pinRclk->pinNum, &config);
    
    config.mux = lspi->pins.pinD0->funcNum;
    PAD_setPinConfig(lspi->pins.pinD0->pinNum, &config);
    
    config.mux = lspi->pins.pinD1->funcNum;
    PAD_setPinConfig(lspi->pins.pinD1->pinNum, &config);
    
    config.mux = lspi->pins.pinD2->funcNum;
    PAD_setPinConfig(lspi->pins.pinD2->pinNum, &config);
    
    config.mux = lspi->pins.pinD3->funcNum;
    PAD_setPinConfig(lspi->pins.pinD3->pinNum, &config);
    
    config.mux = lspi->pins.pinD4->funcNum;
    PAD_setPinConfig(lspi->pins.pinD4->pinNum, &config);
    
    config.mux = lspi->pins.pinD5->funcNum;
    PAD_setPinConfig(lspi->pins.pinD5->pinNum, &config);
    
    config.mux = lspi->pins.pinD6->funcNum;
    PAD_setPinConfig(lspi->pins.pinD6->pinNum, &config);
    
    config.mux = lspi->pins.pinD7->funcNum;
    PAD_setPinConfig(lspi->pins.pinD7->pinNum, &config);
#endif

    // Configure DMA if necessary
    if (lspi->dma)
    {
        DMA_init(lspi->dma->txInstance);
        returnCode = DMA_openChannel(lspi->dma->txInstance);

        if (returnCode == ARM_DMA_ERROR_CHANNEL_ALLOC)
        {
            EC_ASSERT(0,0,0,0);
        }
        
        lspi->dma->txCh = returnCode;

        DMA_setChannelRequestSource(lspi->dma->txInstance, lspi->dma->txCh, (DmaRequestSource_e)lspi->dma->txReq);
        DMA_rigisterChannelCallback(lspi->dma->txInstance, lspi->dma->txCh, lspi->dma->txCb);
    }

#ifdef PM_FEATURE_ENABLE
    lspiInitCnt++;

    if(lspiInitCnt == 1U)
    {
        lspiWorkingStats = 0;
        slpManRegisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE, lspiEnterLpStatePrepare, NULL);
        slpManRegisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE, lspiExitLpStateRestore, NULL);
    }
#endif

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t lspiDeInit(lspiRes_t *lspi)
  \brief       De-initialize LSPI Interface.
  \param[in]   spi  Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiDeInit(lspiRes_t *lspi)
{
#ifdef PM_FEATURE_ENABLE
    uint32_t instance;

    instance = lspiGetInstanceNum(lspi);

    lspiDataBase[instance].isInited = false;

    lspiInitCnt--;

    if(lspiInitCnt == 0)
    {
        lspiWorkingStats = 0;
        //slpManUnregisterPredefinedBackupCb(SLP_CALLBACK_I2S_MODULE);
        //slpManUnregisterPredefinedRestoreCb(SLP_CALLBACK_I2S_MODULE);
    }
    
    apmuVoteToDozeState(PMU_DOZE_USP_MOD, true);
#endif

    return ARM_DRIVER_OK;
}


/**
  \fn          int32_t lspiPowerControl(lspiPowerState_e state, lspiRes_t *lspi)
  \brief       Control LSPI Interface Power.
  \param[in]   state  Power state
  \param[in]   lspi    Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiPowerCtrl(lspiPowerState_e state, lspiRes_t *lspi)
{
    uint32_t instance = lspiGetInstanceNum(lspi);

    switch (state)
    {
        case LSPI_POWER_OFF:
            if(lspi->dma)
            {
                DMA_stopChannel(lspi->dma->txInstance, lspi->dma->txCh, true);
            }

            GPR_clockDisable(lspiClk[instance*2]);
            GPR_clockDisable(lspiClk[instance*2+1]);
            break;

        case LSPI_POWER_FULL:
            GPR_clockEnable(lspiClk[instance*2]);
            GPR_clockEnable(lspiClk[instance*2+1]);
            break;

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

/**
  \fn          int32_t lspiReceive(void *data, uint32_t num, lspiRes_t *lspi)
  \brief       Start receiving data from SPI receiver.
  \param[out]  dataIn  Pointer to buffer for data to receive from LSPI receiver
  \param[in]   num   Number of data items to receive
  \param[in]   lspi   Pointer to LSPI resources
  \return      \ref execution_status
*/
int32_t lspiSend(void *dataOut, uint32_t num, lspiRes_t *lspi)
{
    uint8_t     *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t     *dataLen        = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp            = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList       = lspi->info->prePareSendInfo.dataList;

    // Collect the remainder bytes which is less then 4byte
    if (*dataLen % 4 != 0)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex          += 1;
    }
    
    for (int i = 0; i < *dataListIndex; i++)
    {
        lspi->reg->TFIFO = dataList[i];
    }

    lspi->reg->LSPI_CCTRL = 1 | *dataLen<<8;

    // wait until finish
    while (!(lspi->reg->LSPI_STAT&0x1));

    memset(dataList, 0, *dataListIndex * 4);
    lspi->info->prePareSendInfo.dataListIndex   = 0;
    lspi->info->prePareSendInfo.tmp             = 0;
    lspi->info->prePareSendInfo.trans           = 0;
    lspi->info->prePareSendInfo.dataLen         = 0;
    
    return ARM_DRIVER_OK;
}

void lspiPrepareSend(uint8_t data, lspiRes_t *lspi)
{
    uint8_t     *dataListIndex  = &(lspi->info->prePareSendInfo.dataListIndex);
    uint8_t     *trans          = &(lspi->info->prePareSendInfo.trans);
    uint8_t     *dataLen        = &(lspi->info->prePareSendInfo.dataLen);
    uint32_t    *tmp            = &(lspi->info->prePareSendInfo.tmp);
    uint32_t    *dataList       = lspi->info->prePareSendInfo.dataList;

    *tmp        |= data << *trans;
    *dataLen    += 1;
    *trans      += 8;

    // Round up to 4bytes, then store it into the array of dataList
    if (*trans == 32)
    {
        dataList[*dataListIndex] = *tmp;
        *dataListIndex          += 1;
        *trans                   = 0;
        *tmp                     = 0;
    }
}

/**
  \fn          int32_t lspiControl(uint32_t control, uint32_t arg, lspiRes_t *lspi)
  \brief       Control LSPI Interface.
  \param[in]   control  Operation
  \param[in]   arg      Argument of operation (optional)
  \param[in]   lspi      Pointer to LSPI resources
  \return      common \ref execution_status and driver specific \ref spi_execution_status
*/
int32_t lspiControl(uint32_t control, uint32_t arg, lspiRes_t *lspi)
{
    uint32_t instance = lspiGetInstanceNum(lspi);
    uint8_t *tmp = NULL;

    switch(control & 0xFFFFFFFF)
    {

        // Set Data Format
        case LSPI_CTRL_DATA_FORMAT:
        {
            tmp = (uint8_t*)&lspiDataFmt;
            lspiInstance[instance]->DFMT = *(uint32_t*)tmp;
            break;
        }

        // Set Bus Speed in bps; arg = value
        case LSPI_CTRL_BUS_SPEED:
        {
            if(lspiSetBusSpeed(arg, lspi) != ARM_DRIVER_OK)
            {
                return ARM_DRIVER_ERROR;
            }
            break;
        }

        // Set DMA control
        case LSPI_CTRL_DMA_CTRL:
        {
            tmp = (uint8_t*)&lspiDmaCtrl;
            lspiInstance[instance]->DMACTL = *(uint32_t*)tmp;
            break;
        }

        // Set INT control
        case LSPI_CTRL_INT_CTRL:
        {
            tmp = (uint8_t*)&lspiIntCtrl;
            lspiInstance[instance]->INTCTL = *(uint32_t*)tmp;
            break;
        }

        // Lspi control
        case LSPI_CTRL_CTRL:
        {
            tmp = (uint8_t*)&lspiCtrl;
            lspiInstance[instance]->LSPI_CTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi command control
        case LSPI_CTRL_CMD_CTRL:
        {
            tmp = (uint8_t*)&lspiCmdCtrl;
            (lspiInstance[instance])->LSPI_CCTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi YUV
        case LSPI_CTRL_YUV2RGB_INFO0:
        {
            LSPI2->YUV2RGBINFO0 = 0x199 << 18 | 0x12a << 8 | 0x10 << 0;
            break;
        }

        // Lspi YUV
        case LSPI_CTRL_YUV2RGB_INFO1:
        {
            LSPI2->YUV2RGBINFO1 = 0x204 << 20 | 0x64 << 10 | 0xd0 << 0;
            break;
        }

        // Lspi command address
        case LSPI_CTRL_CMD_ADDR:
        {
            tmp = (uint8_t*)&lspiCmdAddr;
            lspiInstance[instance]->LSPI_CADDR = *(uint32_t*)tmp;
            break;
        }

        // Lspi frame info
        case LSPI_CTRL_FRAME_INFO:
        {
            tmp = (uint8_t*)&lspiInfo;
            lspiInstance[instance]->LSPFINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi tailor info0
        case LSPI_CTRL_TAILOR_INFO0:
        {
            tmp = (uint8_t*)&lspiTailorInfo0;
            lspiInstance[instance]->LSPTINFO0 = *(uint32_t*)tmp;
            break;
        }

        // Lspi tailor info
        case LSPI_CTRL_TAILOR_INFO:
        {
            tmp = (uint8_t*)&lspiTailorInfo;
            lspiInstance[instance]->LSPTINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi scale info
        case LSPI_CTRL_SCALE_INFO:
        {
            tmp = (uint8_t*)&lspiScaleInfo;
            lspiInstance[instance]->LSPSINFO = *(uint32_t*)tmp;
            break;
        }

        // Lspi quartile control
        case LSPI_CTRL_QUARTILE_CTRL:
        {
            tmp = (uint8_t*)&lspiQuartileCtrl;
            lspiInstance[instance]->LSPIQUARTCTRL = *(uint32_t*)tmp;
            break;
        }

        // Lspi Y adjustment
        case LSPI_CTRL_YADJ:
        {
            tmp = (uint8_t*)&lspiYAdj;
            lspiInstance[instance]->LSPIYADJ = *(uint32_t*)tmp;
            break;
        }

        // Lspi gray page cmd0
        case LSPI_CTRL_GRAY_PAGE_CMD0:
        {
            tmp = (uint8_t*)&lspiGrayPageCmd0;
            lspiInstance[instance]->LSPIGPCMD0 = *(uint32_t*)tmp;
            break;
        }

        // Lspi gray page cmd1
        case LSPI_CTRL_GRAY_PAGE_CMD1:
        {
            tmp = (uint8_t*)&lspiGrayPageCmd1;
            lspiInstance[instance]->LSPIGPCMD1 = *(uint32_t*)tmp;
            break;
        }

        // Lspi frame info out
        case LSPI_CTRL_FRAME_INFO_OUT:
        {
            tmp = (uint8_t*)&lspiFrameInfoOut;
            lspiInstance[instance]->LSPFINFO0 = *(uint32_t*)tmp;
            break;
        }

        // USP bus select
        case LSPI_CTRL_BUS_SEL:
        {
            tmp = (uint8_t*)&lspiBusSel;
            lspiInstance[instance]->I2SBUSSEL = *(uint32_t*)tmp;
            break;
        }
#if (defined TYPE_EC718M)

        // Mspi control
        case LSPI_MSPI_CTRL:
        {
            tmp = (uint8_t*)&lspiMspiCtrl;
            lspiInstance[instance]->MSPI_CTRL = *(uint32_t*)tmp;
            break;
        }

        // Vsync control
        case LSPI_VSYNC_CTRL:
        {
            tmp = (uint8_t*)&lspiVsyncCtrl;
            lspiInstance[instance]->VSYNC_CTRL = *(uint32_t*)tmp;
            break;
        }

        // 8080 control
        case LSPI_8080_CTRL:
        {
            tmp = (uint8_t*)&lspi8080Ctrl;
            lspiInstance[instance]->LSPI8080CTRL = *(uint32_t*)tmp;
            break;
        }

        // pre param0
        case LSPI_PRE_PARA0_CTRL:
        {
            tmp = (uint8_t*)&lspiPreParam0;
            lspiInstance[instance]->LSPICMDPREPARA0 = *(uint32_t*)tmp;
			tmp = (uint8_t*)&lspiPreParam2;
            lspiInstance[instance]->LSPICMDPREPARA2 = *(uint32_t*)tmp;
			tmp = (uint8_t*)&lspiPreParam3;
            lspiInstance[instance]->LSPICMDPREPARA3 = *(uint32_t*)tmp;
            break;
        }

        // post param0
        case LSPI_POST_PARA0_CTRL:
        {
            tmp = (uint8_t*)&lspiPostParam0;
            lspiInstance[instance]->LSPICMDPOSTPARA0 = *(uint32_t*)tmp;
            break;
        }

		// te ctrl
        case LSPI_TE_CTRL:
        {
            tmp = (uint8_t*)&lspiTeParam0;
            lspiInstance[instance]->LSPITEPARA0 = *(uint32_t*)tmp;
            tmp = (uint8_t*)&lspiTeParam1;
            lspiInstance[instance]->LSPITEPARA1 = *(uint32_t*)tmp;
            break;
        }

#endif

        default:
            return ARM_DRIVER_ERROR_UNSUPPORTED;
    }
    return ARM_DRIVER_OK;
}

#if (RTE_LSPI2)
static int32_t lspi2Init()
{
    return lspiInit(&lspi2Res);
}
static int32_t lspi2Deinit(void)
{
    return lspiDeInit(&lspi2Res);
}
static int32_t lspi2PowerCtrl(lspiPowerState_e state)
{
    return lspiPowerCtrl(state, &lspi2Res);
}

static int32_t lspi2Send(void *data, uint32_t num)
{
    return lspiSend(data, num, &lspi2Res);
}

static void lspi2PrepareSend(uint8_t data)
{
    return lspiPrepareSend(data, &lspi2Res);
}

static int32_t lspi2Ctrl(uint32_t control, uint32_t arg)
{
    return lspiControl(control, arg, &lspi2Res);
}

// LSPI1 Driver Control Block
lspiDrvInterface_t lspiDrvInterface2 = {
    lspi2Init,
    lspi2Deinit,
    lspi2PowerCtrl,
    lspi2PrepareSend,
    lspi2Send,
    lspi2Ctrl,
};

#endif
