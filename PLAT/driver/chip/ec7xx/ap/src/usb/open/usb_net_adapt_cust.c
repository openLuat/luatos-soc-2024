#include "cmsis_os2.h"
#include "plat_config.h"
#include "string.h"
#include "usb_net_adapt_cust.h"

//DFEATURE_USBNET_ATA_FOR_AP: MACRO DEFINE CONFIGURED BY PROJECT MAKEFILE 
void UsbNetATAdaptInitWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
  UsbNetATAdaptInitCb();
#endif
}

uint8_t UsbNetATAdaptGetCfgEnWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    //volatile uint8_t ret = 1;

    return UsbNetATAdaptEnabledCb();
#else
    volatile uint8_t ret = 0;
        
    return ret;
#endif
}





void UsbNetATAdaptRstIsrWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    if (UsbNetATAdaptEnabledCb())
    {
        UsbNetATAdaptRstIsrCb();
    }
#endif
}

void UsbNetATAdaptAddrWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    if (UsbNetATAdaptEnabledCb())
    {
        UsbNetATAdaptAddrIsrCb();
    }
#endif
}

void UsbNetATAdaptDevDescIsrWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    if (UsbNetATAdaptEnabledCb())
    {
        UsbNetATAdaptDevDescIsrCb();
    }
#endif
}

void UsbNetATAdaptCfgDescIsrWrapCb(uint32_t ReqMsgLen)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    if (UsbNetATAdaptEnabledCb())
    {
        UsbNetATAdaptCfgDescIsrCb(ReqMsgLen);
    }
#endif
}

void UsbNetATAdaptSetCfgIsrWrapCb(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    if (UsbNetATAdaptEnabledCb())
    {
        UsbNetATAdaptSetCfgIsrCb();
    }
#endif
}



void UsbNetATAGetSwtExpectTpWrapCb(uint8_t *pSwtUsbNetTypePending, uint8_t *pExpectedSwtUsbNetType)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
     //if UsbNetATAdaptEnabledCb() false, *pSwtUsbNetTypePending will return 0
     UsbNetATAGetSwtExpectType(pSwtUsbNetTypePending, pExpectedSwtUsbNetType);
#else
    *pSwtUsbNetTypePending = 0;
    *pExpectedSwtUsbNetType = 0;
#endif
}

uint8_t UsbNetATAUpdGetRealUsedTpWrapCb(uint8_t NeedUpd, uint8_t *pRealUsedUsbNetTypeCur)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    return UsbNetATAUpdGetRealUsedType(NeedUpd, pRealUsedUsbNetTypeCur);
#else
    return BSP_GetPlatConfigItemValue(PLAT_CONFIG_ITEM_USB_NET)&1;
#endif
}



const usbnet_adapt_ops t_usbnet_adapt ={
.init = UsbNetATAdaptInitWrapCb,
.get_adapt_cfg_enabled = UsbNetATAdaptGetCfgEnWrapCb,
.rstisr_cb = UsbNetATAdaptRstIsrWrapCb,
.addrisr_cb = UsbNetATAdaptAddrWrapCb,
.devdesc_cb = UsbNetATAdaptDevDescIsrWrapCb,
.cfgdesc_cb = UsbNetATAdaptCfgDescIsrWrapCb,
.setcfgdesc_cb = UsbNetATAdaptSetCfgIsrWrapCb,


//.suspend_cb = UsbNetATAdaptSWTPendSuspCb,
.get_adapt_swt_type = UsbNetATAGetSwtExpectTpWrapCb,
.upd_get_real_used_type = UsbNetATAUpdGetRealUsedTpWrapCb
};



//////////////////////////////////////////////////////////
void USBDynamicInstWrapInit(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    USBDynamicInstInit();
#endif

}



//////////////////////////////////////////////////////////
//////// Dynamic vbus inst ///////////////////////////////
uint8_t UsbVBUSTrySwtEntInstWrapMutex(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    return UsbVBUSTrySwtEntInstMutex();
#else
    //always success
    return 1;
#endif
}

void UsbVBUSwtExitInstWrapUnMutex(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    UsbVBUSwtExitInstUnMutex();
#endif
}


//////////////////////////////////////////////////////////
//////// Dynamic net type swt inst ///////////////////////
uint8_t UsbNetATATrySwtEntInstWrapMutex(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    return UsbNetATATrySwtEntInstMutex();
#else
    //always success
    return 1;
#endif
}

void UsbNetATASwtExitInstWrapUnMutex(void)
{
#ifdef FEATURE_USBNET_ATA_FOR_AP
    UsbNetATASwtExitInstUnMutex();
#endif
}




const usb_dynamic_inst_mgr_ops t_usb_dynamic_inst_mgr_ops ={
.usb_dyn_inst_mgr_init = USBDynamicInstWrapInit,
.usb_net_ata_swt_try_enter_inst_mutex = UsbNetATATrySwtEntInstWrapMutex,
.usb_net_ataswt_exit_inst_unmutex = UsbNetATASwtExitInstWrapUnMutex,
.vbus_swt_try_enter_inst_mutex = UsbVBUSTrySwtEntInstWrapMutex,
.vbus_swt_exit_inst_unmutex = UsbVBUSwtExitInstWrapUnMutex,
};



