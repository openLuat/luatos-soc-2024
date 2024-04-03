#ifndef __IMI_REG_H__
#define __IMI_REG_H__

/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imireg.h
 Description:    - IMS registation interface
 History:        - 2023/04/10,   Original created
******************************************************************************/

#include "imicomm.h"


/******************************************************************************
 *****************************************************************************
 * IMI enum
 *****************************************************************************
******************************************************************************/
typedef enum IMI_REG_PRIM_ID_TAG
{
    IMI_REG_PRIM_BASE = 0,

    IMI_REG_GET_CIREG_REQ,      /* ImiRegGetCiregReq, AT+CIREG? */
    IMI_REG_GET_CIREG_CNF,      /* ImiRegGetCiregCnf */

    IMI_REG_CIREGU_IND,         /*+CIREGU: <reg_info>[,<ext_info>]*/

    IMI_REG_SET_REG_ACT_REQ,    /* ImiRegSetRegActReq, AT+ECIMSREG=0/1 */
    IMI_REG_SET_REG_ACT_CNF,
    IMI_REG_GET_REG_ACT_REQ,    /* ImiRegGetRegActReq, AT+ECIMSREG?  */
    IMI_REG_GET_REG_ACT_CNF,    /* ImiRegGetRegActCnf */

    IMI_REG_PRIM_END    = 0x00ff
}IMI_REG_PRIM_ID;


/******************************************************************************
 *****************************************************************************
 * STRUCT
 *****************************************************************************
******************************************************************************/

/**
 * IMI_REG_GET_CIREG_REQ
*/
typedef  ImiEmptySig       ImiRegGetCiregReq;

typedef enum
{
    IMI_REG_IMS_NOT_REG = 0,
    IMI_REG_IMS_REGED   = 1
}ImiCiregRegInfo;

/**
 * IMI_REG_GET_CIREG_CNF
 * +CIREG: <n>,<reg_info>[,<ext_info>]; <n> is saved in CMS layer
*/
typedef struct ImiRegGetCiregCnf_Tag
{
    UINT8           regInfo;            /* ImiCiregRegInfo */
    BOOL            extInfoPresent;

    UINT16          rsvd1;

    /*
     * e.g: <ext_info>=5 (0101) means that both RTP-based transfer of voice according to
     *      MMTEL and SMS using IMS functionality can be used
    */
    union {
        struct {
            UINT32  rtpVoice    : 1;    /* 0 - voice over PS not avaiable, 1 - voice over PS avaiable */
            UINT32  rtpText     : 1;    /* not support */
            UINT32  smsIms      : 1;    /* 0 - SMS can't use/over IMS, 1 - SMS can use/over IMS */
            UINT32  rtpVideo    : 1;    /* not support */
            UINT32              :16;    /* rsvd for 3GPP */
            UINT32              :12;    /* rsvd for user */
        }u;

        UINT32  value32;
    }extInfo;
}ImiRegGetCiregCnf;


/**
 * IMI_REG_CIREGU_IND
 * +CIREGU: <reg_info>[,<ext_info>]
*/
typedef struct ImiRegCireguInd_Tag
{
    UINT8           regInfo;            /* ImiCiregRegInfo */
    BOOL            extInfoPresent;

    UINT16          rsvd1;

    /*
     * e.g: <ext_info>=5 (0101) means that both RTP-based transfer of voice according to
     *      MMTEL and SMS using IMS functionality can be used
    */
    union {
        struct {
            UINT32  rtpVoice    : 1;    /* 0 - voice over PS not avaiable, 1 - voice over PS avaiable */
            UINT32  rtpText     : 1;    /* not support */
            UINT32  smsIms      : 1;    /* 0 - SMS can't use/over IMS, 1 - SMS can use/over IMS */
            UINT32  rtpVideo    : 1;    /* not support */
            UINT32              :16;    /* rsvd for 3GPP */
            UINT32              :12;    /* rsvd for user */
        }u;

        UINT32  value32;
    }extInfo;
}ImiRegCireguInd;


typedef enum ImiRegImsRegAct_Tag
{
    IMI_REG_IMS_DEREG   = 0,    //IMS de-register
    IMI_REG_IMS_REG             //IMS register
}ImiRegImsRegAct;


/*
 * IMI_REG_SET_REGISTER_ACT_REQ
 * AT+ECIMSREG= 0/1
 *   a) 0 - deregister, when IMS deregister done return
 *   b) 1 - register, if IMS layer check could handle registration procedure, then could return OK
*/
typedef struct ImiRegSetRegActReq_Tag
{
    UINT8           regAct;     /* ImiRegImsRegAct */
    UINT8           resv0;
    UINT16          resv1;
}ImiRegSetRegActReq;

/**
 * IMI_REG_SET_REGISTER_ACT_CNF
*/
typedef struct ImiRegSetRegActCnf_Tag
{
    UINT8           result;
    UINT8           resv0;
    UINT16          resv1;
}ImiRegSetRegActCnf;

/**
 * IMI_REG_GET_REGISTER_ACT_REQ
*/
typedef  ImiEmptySig      ImiRegGetRegActReq;

/**
 * IMI_REG_GET_REGISTER_ACT_CNF
*/
typedef struct ImiRegGetRegActCnf_Tag
{
    UINT8           regAct;     /* ImiRegImsRegAct */
    UINT8           resv0;
    UINT16          resv1;
}ImiRegGetRegActCnf;




#endif

