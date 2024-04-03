
/******************************************************************************

*(C) Copyright 2018 AirM2M International Ltd.

* All Rights Reserved

******************************************************************************
*  Filename:
*
*  Description:
*
*  History: initiated by xxxx
*
*  Notes:
*
******************************************************************************/

/*----------------------------------------------------------------------------*
 *                    INCLUDES                                                *
 *----------------------------------------------------------------------------*/
#include "usbd_func_cconf.h"
#include "ccid_desc.h"
/*----------------------------------------------------------------------------*
 *                    MACROS                                                  *
 *----------------------------------------------------------------------------*/

#define LOBYTE(x)  ((uint8_t)(x & 0x00FF))
#define HIBYTE(x)  ((uint8_t)((x & 0xFF00) >>8))


/*----------------------------------------------------------------------------*
 *                    DATA TYPE DEFINITION                                    *
 *----------------------------------------------------------------------------*/






/*----------------------------------------------------------------------------*
 *                      PRIVATE FUNCTION DECLEARATION                         *
 *----------------------------------------------------------------------------*/





/*----------------------------------------------------------------------------*
 *                      GLOBAL VARIABLES                                      *
 *----------------------------------------------------------------------------*/

#ifdef __USER_CODE__
const usbd_ccid_desc_st usbd_ccid_config_desc;
const usbd_ccid_desc_st usbd_ccid_other_config_desc;

#else //__USER_CODE__

/* 2 seperate descriptor if support HS, seems waste some const memory
   host req----usbpcd_proc_get_desc----get_(other)cfg_desc_handler
   ----usbd_multi_get_cfg-----usbd_multi_fill_config_data*/

/*   USB smart card device Configuration Descriptor */
/*   Configuration, Interface, Endpoint, Class, Vendor */
usbd_ccid_desc_st usbd_ccid_config_desc =
{
  /********************configuration ********************/
    .cfg_desc = {
        0x09,                                                         /* bLength: Configuration Descriptor size */
        0x02,                                  /* bDescriptorType: Configuration */
        SMARTCARD_SIZ_CONFIG_DESC,                                    /* sTotalLength  :  size of Descriptor */
        0x00,
        0x01,                                                         /* bNumInterfaces: 1 interface */
        0x01,                                                         /* bConfigurationValue: */
        0x00,                                                         /* iConfiguration: */
        0x40,                                                         /* bmAttributes: self powered */
        0x32,                                                         /* MaxPower 100 mA */
    },

  /********************interface ********************/
    .intf_data_desc = {
        0x09,                                                         /* bLength: Interface Descriptor size */
        0x04,                                                         /* bDescriptorType: */
        INVALID_INTF_NUM,                                             /* bInterfaceNumber: Number of Interface */
        0x00,                                                         /* bAlternateSetting: Alternate setting */
        #ifdef CCID_SUPPORT_NOTIFY_MSG
        0x03,
        #else
        0x02,
        #endif                                                        /* bNumEndpoints: 2 or 3 endpoints used */
        0x0B,                                                         /* bInterfaceClass: user's interface for CCID */
        0x00,                                                         /* bInterfaceSubClass : */
        0x00,                                                         /* nInterfaceProtocol : None */
        0x05,                                                         /* iInterface: */
    },


  /*******************  class specific descriptor ********************/
    {
        0x36,                                                         /* bLength: CCID Descriptor size */
        0x21,                                                         /* bDescriptorType: Functional Descriptor type. */
        0x10,                                                         /* bcdCCID(LSB): CCID Class Spec release number (1.00) */
        0x01,                                                         /* bcdCCID(MSB) */
        0x00,                                                         /* bMaxSlotIndex :highest available slot on this device */
        0x02,                                                         /* bVoltageSupport: bit Wise OR for 01h-5.0V 02h-3.0V 04h 1.8V*/
        0x03,0x00,0x00,0x00,                                          /* dwProtocols: 0001h = Protocol T=0  0002h = Protocol T=1, 0003h both T=0/1 ??*/
        0xA0,0x0F,0x00,0x00,                                          /* dwDefaultClock: 4Mhz = 4000kHz = 0x0FA0,
                                                                       for 4 Mhz the value is (0x00000FA0) :
                                                                       This is used in ETU and waiting time calculations*/
        0xA0,0x0F,0x00,0x00,                                          /* dwMaximumClock: Maximum supported ICC clock frequency
                                                                        in KHz. So, 3.6Mhz = 3600kHz = 0x0E10,
                                                                        4 Mhz (0x00000FA0) : */
        0x00,                                                         /* bNumClockSupported : no setting from PC
                                                                        If the value is 00h, the
                                                                        supported clock frequencies are assumed to be the
                                                                        default clock frequency defined by dwDefaultClock
                                                                        and the maximum clock frequency defined by
                                                                        dwMaximumClock */

        0x00,0x2A,0x00,0x00,                                          /* dwDataRate: Default ICC I/O data rate in bps
                                                                        9677 bps = 0x25CD
                                                                        for example 10752 bps (0x00002A00) */

        0x16,0x40,0x05,0x00,                                          /* dwMaxDataRate: Maximum supported ICC I/O data
                                                                       rate in bps */
        0x00,                                                         /* bNumDataRatesSupported :
                                                                     The number of data rates that are supported by the CCID
                                                                     If the value is 00h, all data rates between the default
                                                                     data rate dwDataRate and the maximum data rate
                                                                     dwMaxDataRate are supported.
                                                                     Dont support GET_CLOCK_FREQUENCIES
                                                                    */

        0xF6,0x04,0x00,0x00,                                          /* dwMaxIFSD:    */
        0x00,0x00,0x00,0x00,                                          /* dwSynchProtocols  */
        0x00,0x00,0x00,0x00,                                          /* dwMechanical: no special characteristics */

        0xFE,0x00,EXCHANGE_LEVEL_FEATURE,0x00,
                                                                    /* dwFeatures: clk, baud rate, voltage : automatic
                                                                     00000008h Automatic ICC voltage selection
                                                                     00000010h Automatic ICC clock frequency change
                                                                     00000020h Automatic baud rate change according to
                                                                     active parameters provided by the Host or self determined

                                                                     00000100h CCID can set ICC in clock stop mode

                                                                     Only one of the following values may be present to
                                                                     select a level of exchange:
                                                                     00010000h TPDU level exchanges with CCID
                                                                     00020000h Short APDU level exchange with CCID
                                                                     00040000h Short and Extended APDU level exchange
                                                                     If none of those values : character level of exchange*/

        0xF6,0x04,0x00,0x00,                                           /* dwMaxCCIDMessageLength: MIN wMaxPacketSize*/

        0x00,                                                          /* bClassGetResponse*/
        0x00,                                                          /* bClassEnvelope */
        0x00,0x00,                                                     /* wLcdLayout : 0000h no LCD. */
        0x00,                                                          /* bPINSupport : no PIN verif and modif  */
        0x01,                                                          /* bMaxCCIDBusySlots	*/
    },
  /********************  Endpoints ********************/

    .ccid_doep_desc = {
        0x07,                                                         /*Endpoint descriptor length = 7 */
        0x05,                                 /*Endpoint descriptor type */
        INVALID_EP_NUM,                                               /*Endpoint address (OUT, address 1) */
        0x02,                                                         /*Bulk endpoint type */
        LOBYTE(CCID_DATA_OUT_SZ),
        HIBYTE(CCID_DATA_OUT_SZ),
        0x00,                                                         /*Polling interval in milliseconds*/
    },
    .ccid_diep_desc = {
        0x07,                                                         /*Endpoint descriptor length = 7*/
        0x05,                                 /*Endpoint descriptor type */
        INVALID_EP_NUM,                                               /*Endpoint address (IN, address 1) */
        0x02,                                                         /*Bulk endpoint type */
        LOBYTE(CCID_DATA_IN_SZ),
        HIBYTE(CCID_DATA_IN_SZ),
        0x00,                                                         /*Polling interval in milliseconds */
    },


#ifdef CCID_SUPPORT_NOTIFY_MSG
    .ccid_cmd_ep_desc = {
        0x07,                                                         /*bLength: Endpoint Descriptor size*/
        0x05,                                 /*bDescriptorType:*/
        INVALID_EP_NUM,                                               /*bEndpointAddress: Endpoint Address (IN)*/
        0x03,                                                         /* bmAttributes: Interrupt endpoint */
        LOBYTE(CCID_INTR_EP_MAX_PACKET),
        HIBYTE(CCID_INTR_EP_MAX_PACKET),
#ifdef USE_USBC_CTRL_HS
        0x9,                                                          /* bInterval: 2^9-1 *125us */
#else
        0x02,                                                         /* bInterval       = 2 ms polling from host */
#endif

        },
#endif

};


#ifdef USE_USBC_CTRL_HS

/*   USB smart card device Configuration Descriptor */
/*   Configuration, Interface, Endpoint, Class, Vendor */
usbd_ccid_desc_st usbd_ccid_other_config_desc =
{
   /********************configuration ********************/
    .cfg_desc = {
        0x09,                                                         /* bLength: Configuration Descriptor size */
        0x02,                                  /* bDescriptorType: Configuration */
        SMARTCARD_SIZ_CONFIG_DESC,                                    /* sTotalLength  :  size of Descriptor */
        0x00,
        0x01,                                                         /* bNumInterfaces: 1 interface */
        0x01,                                                         /* bConfigurationValue: */
        0x00,                                                         /* iConfiguration: */
        0x40,                                                         /* bmAttributes: self powered */
        0x32,                                                         /* MaxPower 100 mA */
    },

  /********************interface ********************/
    .intf_data_desc = {
        0x09,                                                         /* bLength: Interface Descriptor size */
        0x04,                                                         /* bDescriptorType: */
        0x00,                                                         /* bInterfaceNumber: Number of Interface */
        0x00,                                                         /* bAlternateSetting: Alternate setting */
        #ifdef CCID_SUPPORT_NOTIFY_MSG
        0x03,
        #else
        0x02,
        #endif                                                        /* bNumEndpoints: 3 endpoints used */
        0x0B,                                                         /* bInterfaceClass: user's interface for CCID */
        0x00,                                                         /* bInterfaceSubClass : */
        0x00,                                                         /* nInterfaceProtocol : None */
        0x05,                                                         /* iInterface: */
    },


  /*******************  class specific descriptor ********************/
    {
        0x36,                                                         /* bLength: CCID Descriptor size */
        0x21,                                                         /* bDescriptorType: Functional Descriptor type. */
        0x10,                                                         /* bcdCCID(LSB): CCID Class Spec release number (1.00) */
        0x01,                                                         /* bcdCCID(MSB) */
        0x00,                                                         /* bMaxSlotIndex :highest available slot on this device */
        0x02,                                                         /* bVoltageSupport: bit Wise OR for 01h-5.0V 02h-3.0V 04h 1.8V*/
        0x03,0x00,0x00,0x00,                                          /* dwProtocols: 0001h = Protocol T=0  0002h = Protocol T=1, 0003h both T=0/1 ??*/
        0xA0,0x0F,0x00,0x00,                                          /* dwDefaultClock: 4Mhz = 4000kHz = 0x0FA0,
                                                                       for 4 Mhz the value is (0x00000FA0) :
                                                                       This is used in ETU and waiting time calculations*/
        0xA0,0x0F,0x00,0x00,                                          /* dwMaximumClock: Maximum supported ICC clock frequency
                                                                        in KHz. So, 3.6Mhz = 3600kHz = 0x0E10,
                                                                        4 Mhz (0x00000FA0) : */
        0x00,                                                         /* bNumClockSupported : no setting from PC
                                                                        If the value is 00h, the
                                                                        supported clock frequencies are assumed to be the
                                                                        default clock frequency defined by dwDefaultClock
                                                                        and the maximum clock frequency defined by
                                                                        dwMaximumClock */

        0x00,0x2A,0x00,0x00,                                          /* dwDataRate: Default ICC I/O data rate in bps
                                                                        9677 bps = 0x25CD
                                                                        for example 10752 bps (0x00002A00) */

        0x16,0x40,0x05,0x00,                                          /* dwMaxDataRate: Maximum supported ICC I/O data
                                                                       rate in bps */
        0x00,                                                         /* bNumDataRatesSupported :
                                                                     The number of data rates that are supported by the CCID
                                                                     If the value is 00h, all data rates between the default
                                                                     data rate dwDataRate and the maximum data rate
                                                                     dwMaxDataRate are supported.
                                                                     Dont support GET_CLOCK_FREQUENCIES
                                                                    */

        0xF6,0x04,0x00,0x00,                                          /* dwMaxIFSD:    */
        0x00,0x00,0x00,0x00,                                          /* dwSynchProtocols  */
        0x00,0x00,0x00,0x00,                                          /* dwMechanical: no special characteristics */

        0x38,0x00,EXCHANGE_LEVEL_FEATURE,0x00,
                                                                    /* dwFeatures: clk, baud rate, voltage : automatic
                                                                     00000008h Automatic ICC voltage selection
                                                                     00000010h Automatic ICC clock frequency change
                                                                     00000020h Automatic baud rate change according to
                                                                     active parameters provided by the Host or self determined

                                                                     00000100h CCID can set ICC in clock stop mode

                                                                     Only one of the following values may be present to
                                                                     select a level of exchange:
                                                                     00010000h TPDU level exchanges with CCID
                                                                     00020000h Short APDU level exchange with CCID
                                                                     00040000h Short and Extended APDU level exchange
                                                                     If none of those values : character level of exchange*/

        0xF6,0x04,0x00,0x00,                                           /* dwMaxCCIDMessageLength: MIN wMaxPacketSize*/

        0x00,                                                          /* bClassGetResponse*/
        0x00,                                                          /* bClassEnvelope */
        0x00,0x00,                                                     /* wLcdLayout : 0000h no LCD. */
        0x00,                                                          /* bPINSupport : no PIN verif and modif  */
        0x01,                                                          /* bMaxCCIDBusySlots	*/
    },

  /********************  Endpoints ********************/


    .ccid_doep_desc = {
        0x07,                                                         /*Endpoint descriptor length = 7 */
        0x05,                                 /*Endpoint descriptor type */
        INVALID_EP_NUM,                                               /*Endpoint address (OUT, address 1) */
        0x02,                                                         /*Bulk endpoint type */
        LOBYTE(CCID_DATA_OUT_SZ_FS),
        HIBYTE(CCID_DATA_OUT_SZ_FS),
        0x00,                                                         /*Polling interval in milliseconds*/
    },

    .ccid_diep_desc = {
        0x07,                                                         /*Endpoint descriptor length = 7*/
        0x05,                                 /*Endpoint descriptor type */
        INVALID_EP_NUM,                                               /*Endpoint address (IN, address 1) */
        0x02,                                                         /*Bulk endpoint type */
        LOBYTE(CCID_DATA_IN_SZ_FS),
        HIBYTE(CCID_DATA_IN_SZ_FS),
        0x00,                                                         /*Polling interval in milliseconds */
    },

#ifdef CCID_SUPPORT_NOTIFY_MSG

    .ccid_cmd_ep_desc = {
        0x07,                                                         /*bLength: Endpoint Descriptor size*/
        0x05,                                 /*bDescriptorType:*/
        INVALID_EP_NUM,                                               /*bEndpointAddress: Endpoint Address (IN)*/
        0x03,                                                         /* bmAttributes: Interrupt endpoint */
        LOBYTE(CCID_INTR_EP_MAX_PACKET),
        HIBYTE(CCID_INTR_EP_MAX_PACKET),
#ifdef USE_USBC_CTRL_HS
        0x9,                                                          /* bInterval: 2^9-1 *125us */
#else
        0x02,                                                         /* bInterval       = 2 ms polling from host */
#endif

    },
#endif
};

#endif

#endif //__USER_CODE__
