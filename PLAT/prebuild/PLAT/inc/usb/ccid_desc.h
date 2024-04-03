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

#ifndef CCID_DESC_H
#define CCID_DESC_H



/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "ec7xx.h"

#ifdef __cplusplus
extern "C" {
#endif
#include "usbd_desc_def.h"




/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#ifdef USE_USBC_CTRL_HS
#define CCID_DATA_IN_SZ 512
#define CCID_DATA_OUT_SZ 512

#define CCID_DATA_IN_SZ_FS 0x40
#define CCID_DATA_OUT_SZ_FS 0x40
#else
#define CCID_DATA_IN_SZ         0x40
#define CCID_DATA_OUT_SZ        0x40
#endif

#define CCID_INTR_EP_MAX_PACKET               64//for both FS/HS


#define TPDU_EXCHANGE                  0x01
#define SHORT_APDU_EXCHANGE            0x02
#define EXTENDED_APDU_EXCHANGE         0x04
#define CHARACTER_EXCHANGE             0x00

#define EXCHANGE_LEVEL_FEATURE         EXTENDED_APDU_EXCHANGE

#define REQUEST_ABORT                  0x01
#define REQUEST_GET_CLOCK_FREQUENCIES  0x02
#define REQUEST_GET_DATA_RATES         0x03

#define SMARTCARD_SIZ_CONFIG_DESC      93

#define INVALID_EP_NUM 0xff
#define INVALID_INTF_NUM 0xff
/*----------------------------------------------------------------------------*
 *                   DATA TYPE DEFINITION                                     *
 *----------------------------------------------------------------------------*/

struct tag_usbd_ccid_desc
{
    usb_config_desc_st               cfg_desc;
    usb_interface_descriptor_st      intf_data_desc;
    uint8_t                          ccid_class_desc[54];
    usb_endpoint_descriptor_st       ccid_doep_desc;
    usb_endpoint_descriptor_st       ccid_diep_desc;
    #ifdef CCID_SUPPORT_NOTIFY_MSG
    usb_endpoint_descriptor_st       ccid_cmd_ep_desc;
    #endif


}__attribute__ ((packed));

typedef struct tag_usbd_ccid_desc usbd_ccid_desc_st;



/*----------------------------------------------------------------------------*
 *                    GLOBAL FUNCTIONS DECLEARATION                           *
 *----------------------------------------------------------------------------*/





#endif  // __USBD_FUNC_CCID_H_

