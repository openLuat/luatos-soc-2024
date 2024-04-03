#ifndef __IMS_COMM_TYPE_H__
#define __IMS_COMM_TYPE_H__
/*******************************************************************************
 Copyright:      - 2023- Copyrights of AirM2M Ltd.
 File name:      - imscommtype.h
 Description:    - IMS common struct/marco header
 History:        - 2023/04/23, Original created
******************************************************************************/
#include "commontypedef.h"

/******************************************************************************
 *****************************************************************************
 * Marco/Enum
 *****************************************************************************
******************************************************************************/


/*
 * IMS string format, max len: xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx
 *  contain last: '\0'
*/
#define IMS_IPV6_STR_LEN            40
#define IMS_IP_MAX_STR_LEN          IMS_IPV6_STR_LEN
#define IMS_REALM_MAX_LEN           48
#define IMS_USER_NAME_MAX_LEN       64      /*impi/nai, eg: 46001000000009@ims.mnc001.mcc460.3gppnework.org*/
#define IMS_IMSI_MAX_LEN            16
#define IMS_IMEI_MAX_LEN            16
#define IMS_PRODUCT_NAME_MAX_LEN    32      /*user-agent header, eg: "SIP/2.0 (AirM2M Ltd)" */
#define IMS_APN_MAX_STR_LEN         100
#define IMS_CELL_ID_MAX_STR_LEN     22

#define IMS_USER_IDENTITY_MAX_LEN   128


/*
 * AKA auth related marco
*/
#define IMS_AKA_RAND_LEN    16
#define IMS_AKA_AUTN_LEN    16
#define IMS_AKA_IK_LEN      16
#define IMS_AKA_CK_LEN      16
#define IMS_AKA_RES_LEN     16
#define IMS_AKA_AUTS_LEN    14


/**
 * IMS IP type
*/
typedef enum _EPAT_ImsIpType_enum
{
    IMS_IPV4,
    IMS_IPV6,
    IMS_IPV4V6_V4_perfer,   /* used in some IP selection case */
    IMS_IPV4V6_V6_perfer,   /* used in some IP selection case */
}ImsIpType;


/**
 * IMS transport type
*/
typedef enum ImsTpType_enum
{
    IMS_UDP,
    IMS_TCP,
    IMS_UDP_perfer          /* if pkg > MTU size, using TCP, else using UDP */
}ImsTpType;



/**
 * IMS network acctype
*/
typedef enum ImsAccNetTpType_enum
{
    IMS_NET_TDD_TYPE,
    IMS_NET_FDD_TYPE,
}msAccNetTpType;


/**
 * PLMN oper marco
*/
#define IMS_GET_PURE_MNC(MNCWITHADDINFO)    ((MNCWITHADDINFO)&0xFFF)
#define IMS_IS_2_DIGIT_MNC(MNCWITHADDINFO)  (((MNCWITHADDINFO)>>12) == 0x0F)
#define IMS_SET_MNC_WITH_ADD_INFO(TWODIGType, pureMNC)  ((TWODIGType)?((pureMNC)|0xF000):((pureMNC)&0xFFF))
#define IMS_IS_VALID_PLMN(LPLMN)            (((LPLMN).mcc != 0) ? TRUE : FALSE)
#define IMS_IS_SAME_PLMN(PLMN1, PLMN2)      (((PLMN1).mcc == (PLMN2).mcc) && ((PLMN1).mncWithAddInfo == (PLMN2).mncWithAddInfo))


/******************************************************************************
 *****************************************************************************
 * Struct
 *****************************************************************************
******************************************************************************/

/**
 * Numberic PLMN format, 4 bytes,
 * example: CMCC: 46000; mcc = 0x0460, mnc = 0xf000
*/
typedef struct ImsPlmn_Tag
{
    UINT16 mcc;
    UINT16 mncWithAddInfo;  // if 2-digit MNC type, the 4 MSB bits should set to 'F',
}ImsPlmn;




/******************************************************************************
 *****************************************************************************
 * API
 *****************************************************************************
******************************************************************************/

#endif

