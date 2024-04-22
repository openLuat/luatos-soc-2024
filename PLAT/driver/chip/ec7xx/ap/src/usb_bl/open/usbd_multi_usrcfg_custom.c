#include "cmsis_os2.h"
#include "plat_config.h"
#include "string.h"
#include "usbd_clscdc.h"
#include "usbd_func_cc.h"
#include "usbd_multi_usrcfg_common.h"
#include "sctdef.h"

#ifndef USB_DRV_SMALL_IMAGE
#include "plat_config.h"

#include DEBUG_LOG_HEADER_FILE
#endif
#include "RTE_Device.h"

#define USBD_MULTIDEV_LOAD_FAIL 1

#define EP_REMAP_CUSTOM_CNT 13

#define CUST_DEF_TEST_TYPE1
//#define CUST_DEF_TEST_TYPE_CCID


#if  (VCOM_CCINST_SUBTP0_INHERENT_CNT>0)
extern usb_func_ccinst_st t_vcom_func_custp0_inhrnt_arr[VCOM_CCINST_SUBTP0_INHERENT_CNT];
#endif

extern usb_func_ccinst_st t_vcom_func_custp1_arr[VCOM_CCINST_SUBTP1_2EP_CNT];
extern usb_func_ccinst_st t_vcom_func_custp2_arr[VCOM_CCINST_SUBTP2_3EP_CNT];

typedef struct {
    uint8_t vcom_ccinst_subtp0_inhrnt_cnt;
    uint8_t vcom_ccinst_subtp1_2ep_cnt;
    uint8_t vcom_ccinst_subtp2_3ep_cnt;
    usb_func_ccinst_st *p_vcom_ccinst_subtp0_entry;
    usb_func_ccinst_st *p_vcom_ccinst_subtp1_entry;
    usb_func_ccinst_st *p_vcom_ccinst_subtp2_entry;

}usbd_multidev_ccinst_res_st;

usbd_multidev_ccinst_res_st t_usbd_multidev_ccinst_res =
{
    .vcom_ccinst_subtp0_inhrnt_cnt = VCOM_CCINST_SUBTP0_INHERENT_CNT,
    .vcom_ccinst_subtp1_2ep_cnt = VCOM_CCINST_SUBTP1_2EP_CNT,
    .vcom_ccinst_subtp2_3ep_cnt = VCOM_CCINST_SUBTP2_3EP_CNT,
#if  (VCOM_CCINST_SUBTP0_INHERENT_CNT>0)
    .p_vcom_ccinst_subtp0_entry = &(t_vcom_func_custp0_inhrnt_arr[0]),
#endif
#if (VCOM_CCINST_SUBTP1_2EP_CNT >0)
    .p_vcom_ccinst_subtp1_entry = &(t_vcom_func_custp1_arr[0]),
#endif
#if (VCOM_CCINST_SUBTP2_3EP_CNT >0)
    .p_vcom_ccinst_subtp2_entry = &(t_vcom_func_custp2_arr[0]),
#endif
};
//#define CUST_DEF_TEST_TYPE2//change at/log order--ok
//#define CUST_DEF_TEST_TYPE3//add a vcom---ok
//#define CUST_DEF_TEST_TYPE4//remove a vcom---ok also need to modify dev create/destory
//#define CUST_DEF_TEST_TYPE5//remove rndis--ok also need to modify dev create/destory
//#define CUST_DEF_TEST_TYPE6//change ep--ok


/*
org: if0/1/2/3/4
     iep =2/4/6/8/10
     oep=1/2/3/4/5
     intep=1/2/3/4/5-also in-type

usr cfg:
     usb driver still use org epidx, map 2 usr defined ep in below table
     should make sure no same ep num in below table

     e.g-1. if0 use iep=2 oep=1,intep=1, but user define iep=10, oep=5,intep still 1
     epin_remap_custom_tbl[2] =10
     epout_remap_custom_tbl[1]=5
     epin_remap_custom_tbl[10]=2 //not used but change to another val
     epout_remap_custom_tbl[5]=1//not used but change to another val

     e.g-2. if0 use iep=2 oep=1,intep=1, if2 use iep=6 oep=3,intep=5. usr want switch if0/2 ep
     epin_remap_custom_tbl[2] =6
     epout_remap_custom_tbl[1]=3
     epin_remap_custom_tbl[6]=2
     epout_remap_custom_tbl[3]=1


     NOTE:
     1 currently, if enbale rndis, rndis should be at the first interface,
       if not,rndis enum will have some issue, mostly win host issue, need further debug

     2 after add or remove some if, e.g. remmove PPP vcom or add a raw VCOM, user should also modify
       ccio dev create logic: not create corresponding dev if removed and create new dev if added.
       if still not clear ask EC for help
*/



#ifdef CUST_DEF_TEST_TYPE_CCID

const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


static multidev_custom_info_st t_multidev_custom_info = {
    .elem_cnt = 3,
    #if 1
    .elem_arr[0]  =
    {
        (const uint8_t*)"ccid",
        multidev_tp_ccid,
        ccinst_setting_mainttp_none,
        ccinst_setting_subtp_none,
    },
    #endif
    .elem_arr[1]  =
    {
        (const uint8_t*)"vcom0",
        multidev_tp_vcom_at,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    .elem_arr[2]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },

};


#endif

#ifdef __USER_CODE__

const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


static multidev_custom_info_st t_multidev_custom_info = {
    .elem_cnt = 5,
    .elem_arr[0]  =
    {
        (const uint8_t*)"rndis",
        multidev_tp_rndis,
        ccinst_setting_mainttp_none,
        ccinst_setting_subtp_none,
    },
    .elem_arr[1]  =
    {
        (const uint8_t*)"ecm",
        multidev_tp_ecm,
        ccinst_setting_mainttp_none,
        ccinst_setting_subtp_none,
    },

    .elem_arr[2]  =
    {
        (const uint8_t*)"vcom0",
		multidev_tp_vcom_com,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    .elem_arr[3]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },

    .elem_arr[4]  =
    {
        (const uint8_t*)"vcom2",
		multidev_tp_vcom_com,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
};

#else // __USER_CODE__

#ifdef CUST_DEF_TEST_TYPE1

#ifdef OPEN_CPU_MODE

#ifdef SIMU_QUEC_CFG
const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 7,3,9,6,5,2,8,1,4,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 12,9,11,10,5,6,7,8,1,4,3,2
};

static multidev_custom_info_st t_multidev_custom_info = {
    .elem_cnt = 2,

    .elem_arr[0]  =
    {
        (const uint8_t*)"vcom0",
        multidev_tp_vcom_at,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp2_3ep,
    },
    .elem_arr[1]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp1_2ep,
    },
};

#else
const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


static multidev_custom_info_st t_multidev_custom_info = {
    .elem_cnt = 2,
    .elem_arr[0]  =
    {
        (const uint8_t*)"vcom0",
        multidev_tp_vcom_at,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    .elem_arr[1]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
};

//SIMU_QUEC_CFG end
#endif


#else /* non-opencpu branch */

#ifdef SIMU_QUEC_CFG

const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 7,3,9,6,5,2,8,1,4,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 12,9,11,10,5,6,7,8,1,4,3,2
};

#else

const uint8_t epin_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};


const uint8_t epout_remap_custom_tbl[EP_REMAP_CUSTOM_CNT] = {
    0, 1,2,3,4,5,6,7,8,9,10,11,12
};
#endif

static multidev_custom_info_st t_multidev_custom_info = {
    .elem_cnt = 3,
    #ifdef SIMU_QUEC_CFG
    .elem_arr[0]  =
    {
        (const uint8_t*)"vcom0",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp1_2ep,
    },

    .elem_arr[1]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_at,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp2_3ep,
    },
    .elem_arr[2]  =
    {
        (const uint8_t*)"vcom2",
        multidev_tp_vcom_ppp,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp2_3ep,
    },

    #else
    .elem_arr[0]  =
    {
        (const uint8_t*)"vcom0",
        multidev_tp_vcom_at,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    .elem_arr[1]  =
    {
        (const uint8_t*)"vcom1",
        multidev_tp_vcom_log,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    .elem_arr[2]  =
    {
        (const uint8_t*)"vcom2",
        multidev_tp_vcom_ppp,
        ccinst_setting_cdc_vcom_maintp,
        ccinst_setting_vcom_subtp0_inhrnt,
    },
    #endif

};

#endif
//end open cpu

#endif
//end CUST_DEF_TEST_TYPE1
#endif // __USER_CODE__

extern uint32_t usbc_dev_int_handler (void);
PLAT_BL_CIRAM_FLASH_TEXT void USB_IRQ_Handler(void)
{
#if (RTE_USB_EN == 1)
#if MEASURE_USB_INT_TIME
    extern void TMU_APTimeReadOpen(UINT32 *sysTime);
    uint32_t startTime, endTime;
    TMU_APTimeReadOpen(&startTime);
#endif
  usbc_dev_int_handler();
#if MEASURE_USB_INT_TIME
    TMU_APTimeReadOpen(&endTime);
    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, USB_IRQ_Handler_0, P_VALUE, "USB IRQ Time=%d", (endTime-startTime)&0x1FFFFFFF);
#endif
#endif
}


uint8_t usbcustom_epin_id_map(uint8_t ep_idx)
{
    if (ep_idx > EP_REMAP_CUSTOM_CNT)
    {
        return 0xff;
    }
    return epin_remap_custom_tbl[ep_idx];
}

uint8_t usbcustom_epout_id_map(uint8_t ep_idx)
{
    if (ep_idx > EP_REMAP_CUSTOM_CNT)
    {
        return 0xff;
    }
    return epout_remap_custom_tbl[ep_idx];
}

uint8_t usbcustom_multidev_ccinfo_maprst(void)
{
    uint8_t idx;
    usb_func_ccinst_st * p_ccinst;
    ccinst_cdc_setting_st *p_ccinst_cdc_set;

    for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp0_inhrnt_cnt; idx++)
    {
        p_ccinst = t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp0_entry;
        if (p_ccinst==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }

        p_ccinst = p_ccinst+idx;
        p_ccinst_cdc_set = p_ccinst->p_cc_setting;
        if (p_ccinst_cdc_set==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }
        p_ccinst_cdc_set->map_flag = 0;
    }

    for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp1_2ep_cnt;idx++)
    {
        p_ccinst =  t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp1_entry;
        if (p_ccinst==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }

        p_ccinst = p_ccinst+idx;
        p_ccinst_cdc_set = p_ccinst->p_cc_setting;
        if(p_ccinst_cdc_set==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }
        p_ccinst_cdc_set->map_flag = 0;
    }

    for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp2_3ep_cnt;idx++)
    {
        p_ccinst =  t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp2_entry;
        if (p_ccinst==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }
        p_ccinst = p_ccinst+idx;
        p_ccinst_cdc_set = p_ccinst->p_cc_setting;
        if(p_ccinst_cdc_set==NULL)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }

        p_ccinst_cdc_set->map_flag = 0;
    }

    return 0;
}

uint8_t usbcustom_multidev_ccinfo_map(usbcust_mdcd_ccinfo_st *p_mdcd_ccinfo)
{
    uint8_t idx;
    usb_func_ccinst_st * p_ccinst;
    ccinst_cdc_setting_st *p_ccinst_cdc_set;
    uint8_t map_valid = 0;
    if(p_mdcd_ccinfo->setting_maintp == ccinst_setting_cdc_vcom_maintp)
    {
        if (p_mdcd_ccinfo->setting_subtp== ccinst_setting_vcom_subtp0_inhrnt)
        {
            for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp0_inhrnt_cnt;idx++)
            {
                p_ccinst =  t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp0_entry;
                if (p_ccinst==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }
                p_ccinst = p_ccinst+idx;
                p_ccinst_cdc_set = p_ccinst->p_cc_setting;
                if (p_ccinst_cdc_set==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }
                if (p_ccinst_cdc_set->map_flag ==1)
                {
                    continue;
                }
                p_ccinst_cdc_set->map_flag = 1;
                map_valid = 1;
                p_mdcd_ccinfo->p_func_ccinst = p_ccinst;
                break;

            }
        }

       else if (p_mdcd_ccinfo->setting_subtp== ccinst_setting_vcom_subtp1_2ep)
        {
            for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp1_2ep_cnt;idx++)
            {
                p_ccinst =  t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp1_entry;
                if (p_ccinst==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }

                p_ccinst = p_ccinst+idx;
                p_ccinst_cdc_set = p_ccinst->p_cc_setting;
                if (p_ccinst_cdc_set==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }
                if (p_ccinst_cdc_set->map_flag ==1)
                {
                    continue;
                }
                p_ccinst_cdc_set->map_flag = 1;
                map_valid = 1;
                p_mdcd_ccinfo->p_func_ccinst = p_ccinst;
                break;
            }
        }
        else if (p_mdcd_ccinfo->setting_subtp== ccinst_setting_vcom_subtp2_3ep)
        {
            for(idx = 0; idx < t_usbd_multidev_ccinst_res.vcom_ccinst_subtp2_3ep_cnt;idx++)
            {
                p_ccinst =  t_usbd_multidev_ccinst_res.p_vcom_ccinst_subtp2_entry;
                if (p_ccinst==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }
                p_ccinst = p_ccinst+idx;
                p_ccinst_cdc_set = p_ccinst->p_cc_setting;
                if (p_ccinst_cdc_set==NULL)
                {
                    return USBD_MULTIDEV_LOAD_FAIL;
                }
                if (p_ccinst_cdc_set->map_flag ==1)
                {
                    continue;
                }
                p_ccinst_cdc_set->map_flag = 1;
                map_valid = 1;
                p_mdcd_ccinfo->p_func_ccinst = p_ccinst;
                break;
            }
        }

    }
    else
    {
        //other dev not support ccinst
        p_mdcd_ccinfo->p_func_ccinst = NULL;
        map_valid = 1;
    }

            //load cnt is limited to MULTI_LOAD_CNT_MAX
#ifndef USB_DRV_SMALL_IMAGE
    ECOMM_TRACE(UNILOG_PLA_DRIVER, usbcustom_multidev_ccinfo_map_1, P_ERROR, 4, \
                                "clstype %d, inst_id %d, maintp %d, subtp %d", \
                                p_mdcd_ccinfo->cls_type,         \
                                p_mdcd_ccinfo->inst_id,        \
                                p_mdcd_ccinfo->setting_maintp,  \
                                p_mdcd_ccinfo->setting_subtp);
#endif

    if (map_valid==0)
    {
#ifndef USB_DRV_SMALL_IMAGE
        ECOMM_TRACE(UNILOG_PLA_DRIVER, usbcustom_multidev_ccinfo_map_2, P_ERROR, 4, \
                                "clstype %d, inst_id %d, maintp %d, subtp %d", \
                                p_mdcd_ccinfo->cls_type,         \
                                p_mdcd_ccinfo->inst_id,         \
                                p_mdcd_ccinfo->setting_maintp,  \
                                p_mdcd_ccinfo->setting_subtp);
        EC_ASSERT(0, p_mdcd_ccinfo->cls_type, p_mdcd_ccinfo->inst_id,   \
                                    (p_mdcd_ccinfo->setting_maintp<<8) |p_mdcd_ccinfo->setting_subtp);
#else
    return USBD_MULTIDEV_LOAD_FAIL;

#endif
    }
    return 0;
}




uint8_t usbcustom_multidev_cfg_reset(void)
{
    uint8_t loop_idx;
    uint32_t map_flag = 0;
    uint32_t cur_flag = 0;
    uint8_t ret;
    if (epout_remap_custom_tbl[0]!=0)
    {
        return USBD_MULTIDEV_LOAD_FAIL;
    }
    for (loop_idx = 1; loop_idx < EP_REMAP_CUSTOM_CNT; loop_idx++)
    {
        if (epout_remap_custom_tbl[loop_idx] >= EP_REMAP_CUSTOM_CNT)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }
        cur_flag = (1UL<<epout_remap_custom_tbl[loop_idx]);
        if (cur_flag&map_flag)
        {
            return USBD_MULTIDEV_LOAD_FAIL;
        }
        map_flag |= (1UL<<epout_remap_custom_tbl[loop_idx]);
    }
    ret = usbcustom_multidev_ccinfo_maprst();

    if(ret)
    {
        return USBD_MULTIDEV_LOAD_FAIL;
    }
#ifdef USBD_EMC_MAC_CONFIG
    update_ecm_mac();
#endif

    return 0;
}

multidev_custom_info_st* usbcustom_multidev_cfg_update(void)
{
    return &t_multidev_custom_info;
}

uint8_t usbcustom_multidev_cfg_end(multidev_load_info_st *p_multidev_load)
{
    if (p_multidev_load->load_cnt > MULTI_DEV_LOADCNT_MAX)
    {
        //load cnt is limited to MULTI_LOAD_CNT_MAX
#ifndef USB_DRV_SMALL_IMAGE
        ECOMM_TRACE(UNILOG_PLA_DRIVER, usbcustom_multidev_cfg_end_1, P_ERROR, 1,"p_multidev_load->load_cnt %x", p_multidev_load->load_cnt);
#endif
        return USBD_MULTIDEV_LOAD_FAIL;
    }
    return 0;
}

extern void usbd_multi_lib_get_load_info(multidev_load_info_st **p_pload_info);
uint8_t* usbcustom_multidev_get_devname(uint8_t func_idx)
{
    multidev_load_info_st  *load_info = NULL;

    usbd_multi_lib_get_load_info(&load_info);
    if(load_info == NULL) return NULL;

    if(func_idx >= load_info->load_cnt) return NULL;

    return (uint8_t*)load_info->elem_arr[func_idx].p_dev_name;
}

uint8_t *usbcustom_multidev_strdesc(usbcust_mdcd_strdesc_st *p_mdcd_strdesc)
{
    uint8_t* p_intf_str_desc = (uint8_t*)"default";
    switch(p_mdcd_strdesc->cls_type)
    {
        #ifdef SIMU_QUEC_CFG
        case multidev_tp_vcom_at:
            p_intf_str_desc = (uint8_t*)"Mobile AT Interface";
            break;
        case multidev_tp_vcom_log:
            p_intf_str_desc = (uint8_t*)"Mobile Diag Interface";
            break;
        case multidev_tp_vcom_ppp:
            p_intf_str_desc = (uint8_t*)"Mobile AT Interface";
            break;
        case multidev_tp_vcom_com:
            p_intf_str_desc = (uint8_t*)"Mobile Serial number";
            break;
        default:
            break;
        #else
        case multidev_tp_vcom_at:
            p_intf_str_desc = (uint8_t*)"at";
            break;
        case multidev_tp_vcom_log:
            p_intf_str_desc = (uint8_t*)"log";
            break;
        case multidev_tp_vcom_ppp:
            p_intf_str_desc = (uint8_t*)"ppp";
            break;
        case multidev_tp_vcom_com:
            p_intf_str_desc = (uint8_t*)"com";
            break;
        default:
            break;
        #endif
    }

#ifndef USB_DRV_SMALL_IMAGE

    ECPLAT_PRINTF(UNILOG_PLA_DRIVER, usbcustom_multidevstr_desc_1, P_DEBUG,             \
                            "cls_type %d, inst_id 0x%x, name %s, intf string %s", \
                            p_mdcd_strdesc->cls_type,  \
                            p_mdcd_strdesc->inst_id,     \
                            p_mdcd_strdesc->p_func_name, \
                            p_intf_str_desc);

#endif
    return p_intf_str_desc;
}

uint8_t usbcustom_multidev_cmndesc(usbcust_mdcd_cmndesc_st *p_mdcd_cmndesc)
{

    usbcust_cmndesc_data_st *p_cmndesc_data=  &(p_mdcd_cmndesc->t_cmndesc_data);

    switch(p_mdcd_cmndesc->cls_type)
    {

        case multidev_tp_vcom_at:
        case multidev_tp_vcom_log:
        case multidev_tp_vcom_ppp:
        case multidev_tp_vcom_com:
                #ifdef SIMU_QUEC_CFG


                p_cmndesc_data->intf_ctrl_desc.bInterfaceClass = 0xFF;
                p_cmndesc_data->intf_ctrl_desc.bInterfaceSubClass = 0x00;
                p_cmndesc_data->intf_ctrl_desc.bInterfaceProtocol = 0x00;
                p_cmndesc_data->b_intf_ctrl_desc_upd  = 1;


                #else
                p_cmndesc_data->intf_ctrl_desc.bInterfaceClass = 0x02;
                p_cmndesc_data->intf_ctrl_desc.bInterfaceSubClass = 0x02;
                p_cmndesc_data->intf_ctrl_desc.bInterfaceProtocol = 0x01;
                p_cmndesc_data->b_intf_ctrl_desc_upd  = 1;
                #endif

#ifndef USB_DRV_SMALL_IMAGE

                ECPLAT_PRINTF(UNILOG_PLA_DRIVER, usbcustom_multidev_cmndesc_1, P_DEBUG,             \
                                        "name %s, cls = 0x%x, subcls 0x%x, protocol 0x%x",   \
                                        p_mdcd_cmndesc->p_func_name,                         \
                                        p_cmndesc_data->intf_ctrl_desc.bInterfaceClass,  \
                                        p_cmndesc_data->intf_ctrl_desc.bInterfaceSubClass,     \
                                        p_cmndesc_data->intf_ctrl_desc.bInterfaceProtocol);
#endif

            break;


        default:
            break;
    }

    return 0;
}


