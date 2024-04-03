/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History:
*
*  Notes:
*
******************************************************************************/


#ifndef __AUDIO_CFG_H__
#define __AUDIO_CFG_H__


/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/

#include <stdint.h>
#include "cmsis_compiler.h"


#ifdef __cplusplus
extern "C" {
#endif


/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

//#pragma pack(1)

#define TX_AEC
#define TX_ANS
#define TX_DRC
#define TX_AGC
#define TX_EQ

#define RX_ANS
#define RX_DRC
#define RX_AGC
#define RX_EQ

#define DRC_FIXED
#define EQ_FIXED

#define MAX_VQE_EQ_BAND 10

#define MAX_ANS_EQ_BAND 32


/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/    
typedef enum _EPAT_IIR_BIQUARD_TYPE
{
    IIR_BIQUARD_PASS = 0,
    IIR_BIQUARD_RAW,
    IIR_BIQUARD_LPF,
    IIR_BIQUARD_HPF,
    // band pass filter, constant skirt gain, peak gain = Q
    IIR_BIQUARD_BPF0,
    // band pass filter, const 0 dB peak gain
    IIR_BIQUARD_BPF1,
    IIR_BIQUARD_NOTCH,
    IIR_BIQUARD_APF,
    IIR_BIQUARD_PEAKINGEQ,
    IIR_BIQUARD_LOWSHELF,
    IIR_BIQUARD_HIGHSHELF,
    IIR_BIQUARD_QTY
}IIR_BIQUARD_TYPE;

typedef struct _EPAT_AgcConfig_t{
    int8_t     bypass;
    int8_t     targetLevel;
    int8_t     compressionGain;
    int8_t     limiterEnable;
} AgcConfig_t;

typedef struct _EPAT_DrcConfig_t
{
    int8_t        bypass;
    int16_t       compThreshold;
    int16_t       compRatio;
    int16_t       expandThreshold;
    int16_t       expandRatio;
    int16_t       attackTime;
    int16_t       releaseTime;
    int16_t       makeupGain;

} DrcConfig_t;

struct IirBiquardState
{
    int16_t a1, a2, b0, b1, b2;
    int16_t s0, s1, s2;
};

typedef __PACKED_STRUCT _EPAT_BiquardParam_raw
{
    int16_t a1; int16_t a2; int16_t b0; int16_t b1; int16_t b2;
}biquardParam_raw;

typedef __PACKED_STRUCT _EPAT_BiquardParam_design
{
    int16_t f0; int16_t gain; int16_t q;
}biquardParam_design;

typedef __PACKED_UNION _EPAT_BiquardParam_filt
{
    biquardParam_raw raw;
    biquardParam_design design;
}biquardParam_filt;

typedef __PACKED_STRUCT _EPAT_BiquardParam_t
{
    //enum IIR_BIQUARD_TYPE type;
    IIR_BIQUARD_TYPE type;
    biquardParam_filt filt;
}BiquardParam;

typedef __PACKED_STRUCT _EPAT_EqConfig_t
{
    int32_t     bypass;
    int         gain;
    int32_t     num;
    BiquardParam params[MAX_VQE_EQ_BAND];
}EqConfig_t;

typedef struct _EPAT_AnsConfig_t{
    int8_t    bypass;
    int8_t    mode;
    uint16_t  eqBand[MAX_ANS_EQ_BAND];
} AnsConfig_t;

typedef struct _EPAT_AecConfig_t{
    int8_t      bypass;
    int16_t     delay;
    int8_t      cngMode;
    int8_t      echoMode;
    int8_t      nlpFlag;
} AecConfig_t;

typedef struct _EPAT_SpeechConfig_t{
#if defined(TX_AEC)
    AecConfig_t      CVT_AEC;
#endif
#if defined(TX_ANS)
    AnsConfig_t      CVT_ANS;
#endif
#if defined(TX_DRC)
    DrcConfig_t         CVT_DRC;
#endif
#if defined(TX_AGC)
    AgcConfig_t    CVT_AGC;
#endif
#if defined(TX_EQ)
    EqConfig_t      CVT_EQ;
#endif

#if defined(RX_ANS)
    AnsConfig_t      CVT_ANS_RX;
#endif
#if defined(RX_DRC)
    DrcConfig_t         CVT_DRC_RX;
#endif
#if defined(RX_AGC)
    AgcConfig_t    CVT_AGC_RX;
#endif
#if defined(RX_EQ)
    EqConfig_t      CVT_EQ_RX;
#endif
} SpeechConfig_t;

typedef struct _EPAT_AudioConfig_t{
    int8_t          amrEncodeBypass;// whether need to bypass AMR encode, not related to index
    int8_t          amrDecodeBypass;// whether need to bypass AMR decode, not related to index
    int8_t          resv[2];
    SpeechConfig_t  speechCfgTx;     // fill by AP, read from NV according to index, AP NV store device*dierction*format =3*2*2=12 copies
    SpeechConfig_t  speechCfgRx;     // select one copy by index, index is passed by uplayer according to current user senario 
} AudioConfig_t;


//#pragma pack()


/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/





#ifdef __cplusplus
}
#endif

#endif


