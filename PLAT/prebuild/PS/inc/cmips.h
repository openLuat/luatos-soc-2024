#ifndef __CMI_PS_H__
#define __CMI_PS_H__
/******************************************************************************
Copyright:      - 2017- Copyrights of AirM2M Ltd.
File name:      - cmips.h
Description:    - CMI (CIoT modem interface) PS handling interface defination
History:        - 08/09/2020, Originated by Jason
******************************************************************************/
#include "cmicomm.h"

/******************************************************************************
 *****************************************************************************
 * Macro
 *****************************************************************************
******************************************************************************/

#define CMI_PS_CID_NUM              16
#define CMI_PS_INVALID_CID          0xFF
#define CMI_PS_MAX_VALID_CID        15  //CID MAX VALUE
#define CMI_PS_MIN_VALID_CID        0
#define CMI_PS_MAX_TFT_FILTERS      16
#define CMI_PS_MAX_APN_LEN          100
#define CMI_PS_MAX_AUTH_STR_LEN     64
#define CMI_PDN_MAX_NW_ADDR_NUM     4
#define CMI_PS_MAX_BEARER_NUM       11
#define CMI_PCSCF_MAX_NW_ADDR_NUM      6
#define CMI_PS_LAST_ESM_CAUSE_NUM      3    //For easy, should same with CCM_MAX_ESM_LAST_REJ_CAUSE_NUM

#define CMI_PS_CHECK_CID_VALID(cid) ((UINT32)(cid) <= CMI_PS_MAX_VALID_CID)

/******************************************************************************
 *****************************************************************************
 * Enums
 *****************************************************************************
******************************************************************************/
typedef enum _EPAT_CMI_PS_PRIM_ID_TAG
{
    CMI_PS_PRIM_BASE = 0,

    CMI_PS_DEFINE_BEARER_CTX_REQ = 0x01,    //AT+CGDCONT=<cid>,<PDP_type>, CmiPsDefineBearerCtxReq
    CMI_PS_DEFINE_BEARER_CTX_CNF,           //CmiPsDefineBearerCtxCnf
    CMI_PS_DEL_BEARER_CTX_REQ,              //AT+CGDCONT=<cid>, CmiPsDelBearerCtxReq
    CMI_PS_DEL_BEARER_CTX_CNF,              //CmiPsDelBearerCtxCnf

    CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ,   /*CmiPsGetAllBearersCidsInfoReq*/
    CMI_PS_GET_ALL_BEARERS_CIDS_INFO_CNF,   /*CmiPsGetAllBearersCidsInfoCnf*/

    /*
     * AT+CGDCONT? is to query all the defined PDP info, in order to reduce the struct size,
     * Each time return one bearer info;
    */
    CMI_PS_GET_DEFINED_BEARER_CTX_REQ,      //CmiPsGetDefinedBearerCtxReq
    CMI_PS_GET_DEFINED_BEARER_CTX_CNF,      //CmiPsGetDefinedBearerCtxCnf

    /*
     * AT+CGDCONT=?, return directly in AtCmdSrv side, don't need to involved a new CMI
     * CMI_PS_GET_BEARER_CTX_CAPA_REQ, //AT+CGDCONT=?
     * CMI_PS_GET_BEARER_CTX_CAPA_CNF = 10,
    */

    /*
     * useless, please use: CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ
     * CMI_PS_GET_VACANCY_CIDS_REQ, //AT+CGVCID?, get vacant (not used) CID list
     * CMI_PS_GET_VACANCY_CIDS_CNF = 10,
     */

    /***********************************************************************
     * Define the NB attached bearer context, by:
     *  AT+CGDCONT=0,<pdp_type>, or
     *  AT+ECATTBEARER=<PDP_type>, ....
     **********************************************************************/
    CMI_PS_SET_ATTACHED_BEARER_CTX_REQ = 0x10,      //CmiPsSetAttachedBearerCtxReq
    CMI_PS_SET_ATTACHED_BEARER_CTX_CNF,             //CmiPsSetAttachedBearerCtxCnf
    CMI_PS_GET_ATTACHED_BEARER_CTX_REQ,             //CmiPsGetAttachedBearerCtxReq
    CMI_PS_GET_ATTACHED_BEARER_CTX_CNF,             //CmiPsGetAttachedBearerCtxCnf

    CMI_PS_SET_ATTACHED_AUTH_CTX_REQ,               //AT+ECATTAUTH=xxx,xxx, - TBD
    CMI_PS_SET_ATTACHED_AUTH_CTX_CNF,
    CMI_PS_GET_ATTACHED_AUTH_CTX_REQ,               //AT+ECATTAUTH?, -TBD
    CMI_PS_GET_ATTACHED_AUTH_CTX_CNF,

    CMI_PS_DEFINE_DEDICATED_BEARER_CTX_REQ = 0x20,  //AT+CGDSCONT=<cid>,<p_cid>, ...,
    CMI_PS_DEFINE_DEDICATED_BEARER_CTX_CNF,
    CMI_PS_DEL_DEDICATED_BEARER_CTX_REQ,            //AT+CGDSCONT=<cid>,
    CMI_PS_DEL_DEDICATED_BEARER_CTX_CNF,
    CMI_PS_GET_DEFINED_DEDICATED_BEARER_CTX_REQ,    //AT+CGDSCONT?,
    CMI_PS_GET_DEFINED_DEDICATED_BEARER_CTX_CNF,

    CMI_PS_SET_ATTACH_STATE_REQ = 0x30,             //AT+CGATT, CmiPsSetAttachStateReq
    CMI_PS_SET_ATTACH_STATE_CNF,                    //CmiPsSetAttachStateCnf
    CMI_PS_GET_ATTACH_STATE_REQ,                    //AT+CGATT?, CmiPsGetAttachStateReq
    CMI_PS_GET_ATTACH_STATE_CNF,                    //CmiPsGetAttachStateCnf
    CMI_PS_GET_ATTACH_STATE_CAPA_REQ,               //AT+CGATT=?, -TBD
    CMI_PS_GET_ATTACH_STATE_CAPA_CNF,

    CMI_PS_SET_BEARER_ACT_STATE_REQ ,               //AT+CGACT, CmiPsSetBearerActStateReq
    CMI_PS_SET_BEARER_ACT_STATE_CNF,                //CmiPsSetBearerActStateCnf


    CMI_PS_GET_BEARER_ACT_CAPA_REQ,                 //AT+CGACT=?, -TBD
    CMI_PS_GET_BEARER_ACT_CAPA_CNF,

    CMI_PS_SET_EMC_ATTACH_STATE_REQ,                //AT+ECEMCATT, CmiPsSetEmcAttachStateReq
    CMI_PS_SET_EMC_ATTACH_STATE_CNF,                //CmiPsSetEmcAttachStateCnf
    CMI_PS_GET_EMC_ATTACH_STATE_REQ,                //AT+ECEMCATT?, CmiPsGetEmcAttachStateReq
    CMI_PS_GET_EMC_ATTACH_STATE_CNF,                //CmiPsGetEmcAttachStateCnf


    CMI_PS_MODIFY_BEARER_CTX_REQ = 0x40,            //AT+CGCMOD, CmiPsModifyBearerCtxReq
    CMI_PS_MODIFY_BEARER_CTX_CNF,                   //CmiPsModifyBearerCtxCnf

    /************************************************************************
     * Get CIDs associated with actived default & dedicated bearer
     * AT+CGCMOD=?
     * AT+CGTFTRDP=?
     * AT+CGEQOSRDP=?
     ************************************************************************/
    /*
     * Useless, please try: CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ
     * CMI_PS_GET_ACTED_BEARER_CIDS_REQ,
     * CMI_PS_GET_ACTED_BEARER_CIDS_CNF = 40,
    */
    CMI_PS_ENTER_DATA_STATE_REQ,                    //AT+CGDATA, CmiPsEnterDataStateReq
    CMI_PS_ENTER_DATA_STATE_CNF,                    //CmiPsEnterDataStateCnf

    CMI_PS_SET_PS_EVENT_REPORT_CFG_REQ,             //AT+CGEREP=[<mode>[,bfr]], -TBD
    CMI_PS_SET_PS_EVENT_REPORT_CFG_CNF,             //
    CMI_PS_GET_PS_EVENT_REPORT_CFG_REQ,             //AT+CGEREP? -TBD
    CMI_PS_GET_PS_EVENT_REPORT_CFG_CNF,
    CMI_PS_GET_PS_EVENT_REPORT_CFG_CAPA_REQ,        //AT+CGEREP=? -TBD
    CMI_PS_GET_PS_EVENT_REPORT_CFG_CAPA_CNF ,

    CMI_PS_DETACH_STATE_IND =0x50,                  //+CGEV: xx DETACH,

    //+CGEV: ME PDN ACT <cid>
    //+CGEV: NW/ME ACT <p_cid>, <cid>, <event_type>
    CMI_PS_BEARER_ACTED_IND,    //+CGEV: ME PDN ACT <cid>, CmiPsBearerActedInd
    //CMI_PS_DEDICATED_BEARER_ACTED_IND, //+CGEV: NW/ME ACT <p_cid>, <cid>, <event_type>

    //+CGEV: NW/MS PDN DEACT <cid>
    //+CGEV: NW/ME DEACT <p_cid>,<cid>
    CMI_PS_BEARER_DEACT_IND,                        //+CGEV: NW/MS PDN DEACT <cid>, CmiPsBearerDeActInd

    CMI_PS_BEARER_MODIFY_IND,                       //+CGEV: NW/ME MODIFY <cid>, CmiPsBearerModifyInd

    //CMI_PS_SET_CEREG_MODE_REQ,  //AT+CEREG=<n>, CmiPsSetCeregModeReq
    //CMI_PS_SET_CEREG_MODE_CNF = 0x30,   //CmiPsSetCeregModeCnf
    CMI_PS_GET_CEREG_REQ  = 0x60,                   //AT+CEREG? CmiPsGetCeregReq
    CMI_PS_GET_CEREG_CNF,                           //CmiPsGetCeregCnf
    CMI_PS_GET_CEREG_CAP_REQ,                       //AT+CEREG=?, CmiPsGetCeregCapReq
    CMI_PS_GET_CEREG_CAP_CNF,                       //CmiPsGetCeregCapCnf
    CMI_PS_CEREG_IND,                               //+CEREG xxx, CmiPsCeregInd

    CMI_PS_READ_BEARER_DYN_CTX_REQ,                 //AT+CGCONTRDP=<cid>; CmiPsReadBearerDynCtxParamReq
    CMI_PS_READ_BEARER_DYN_CTX_CNF,                 //CmiPsReadBearerDynCtxParamCnf

    /*
     * Useless, please try: CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ
     * CMI_PS_GET_ACTED_DEFAULT_BEARER_CIDS_REQ, //AT+CGCONTRDP=?
     * CMI_PS_GET_ACTED_DEFAULT_BEARER_CIDS_CNF,
     */

    CMI_PS_READ_DEDICATED_BEARER_DYN_CTX_REQ,       //AT+CGSCONTRDP,
    CMI_PS_READ_DEDICATED_BEARER_DYN_CTX_CNF,

    /*
     * Useless, please try: CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ
     * CMI_PS_GET_ACTED_DEDICATED_BEARER_CIDS_REQ, //AT+CGSCONTRDP=?
     * CMI_PS_GET_ACTED_DEDICATED_BEARER_CIDS_CNF,
     */

    CMI_PS_DEFINE_TFT_PARM_REQ = 0x70,              //AT+CGTFT=[<cid>, ...], CmiPsSetDefineTFTParamReq
    CMI_PS_DEFINE_TFT_PARM_CNF,                     //CmiPsSetDefineTFTParamCnf
    CMI_PS_GET_DEFINED_TFT_PARM_REQ ,               //AT+CGTFT?, CmiPsGetDefineTFTParamReq
    CMI_PS_GET_DEFINED_TFT_PARM_CNF,                //CmiPsGetDefineTFTParamCnf
    CMI_PS_DELETE_TFT_PARM_REQ,                     //CmiPsDelTFTParamReq
    CMI_PS_DELETE_TFT_PARM_CNF,                     //CmiPsDelTFTParamCnf

    /*
     * AT+CGTFT=?, AtCmdSrv return directly
     * CMI_PS_GET_TFT_SETTING_PARA_CAPA_REQ,
     * CMI_PS_GET_TFT_SETTING_PARA_CAPA_CNF,
    */

    CMI_PS_READ_DYN_TFT_CTX_REQ,                    //AT+CGTFTRDP[=<cid>], CmiPsReadTFTDynCtxParamReq
    CMI_PS_READ_DYN_TFT_CTX_CNF,                    //CmiPsReadTFTDynCtxParamCnf

    CMI_PS_DEFINE_EPS_QOS_REQ = 0x80,               //AT+CGEQOS=<cid>,<QCI>,..., CmiPsDefineEpsQoSReq
    CMI_PS_DEFINE_EPS_QOS_CNF,                      //CmiPsDefineEpsQoSCnf
    CMI_PS_DEL_DEFINE_EPS_QOS_REQ,                  //CmiPsDelEpsQosReq
    CMI_PS_DEL_DEFINE_EPS_QOS_CNF,                  //CmiPsDelEpsQosCnf
    CMI_PS_GET_DEFINED_EPS_QOS_REQ,                 //AT+CEQOS?, CmiPsGetDefinedEpsQoSReq, get current settings of each defined QOS, note here, not activated EPS QOS
    CMI_PS_GET_DEFINED_EPS_QOS_CNF,                 //CmiPsGetDefinedEpsQoSCnf
    CMI_PS_GET_EPS_QOS_SETTING_CAPA_REQ,            //AT+CEQOS=? -TBD
    CMI_PS_GET_EPS_QOS_SETTING_CAPA_CNF,

    CMI_PS_READ_DYN_BEARER_EPS_QOS_REQ,             //AT+CGEQOSRDP[=<cid>], CmiPsReadDynEpsQoSReq
    CMI_PS_READ_DYN_BEARER_EPS_QOS_CNF,             //CmiPsReadDynEpsQoSCnf

    CMI_PS_DEL_NON_ACT_BEARER_CTX_REQ,              //AT+CGDEL[=<cid>] //seems just the same as CGDCONT=<cid>, CGDSCONT=<cid>, -TBD
    CMI_PS_DEL_NON_ACT_BEARER_CTX_CNF,

    CMI_PS_DEFINE_AUTH_CTX_REQ = 0x90,              //AT+CGAUTH=<cid>,<auth_proto>,[<userid>,<password>]
    CMI_PS_DEFINE_AUTH_CTX_CNF,
    CMI_PS_GET_DEFINED_AUTH_CTX_REQ,                //AT+CGAUTH?
    CMI_PS_GET_DEFINED_AUTH_CTX_CNF,
    CMI_PS_GET_AUTH_CTX_CAPA_REQ,                   //AT+CGAUTH=?
    CMI_PS_GET_AUTH_CTX_CAPA_CNF,


    CMI_PS_SET_ATTACH_WITH_OR_WITHOUT_PDN_REQ,      //AT+CIPCA, CmiPsSetAttachWithOrWithoutPdnReq
    CMI_PS_SET_ATTACH_WITH_OR_WITHOUT_PDN_CNF,      //CmiPsSetAttachWithOrWithoutPdnCnf
    CMI_PS_GET_ATTACH_WITH_OR_WITHOUT_PDN_REQ,      //AT+CIPCA? CmiPsGetAttachWithOrWithoutPdnReq
    CMI_PS_GET_ATTACH_WITH_OR_WITHOUT_PDN_CNF,      //CmiPsSetAttachWithOrWithoutPdnCnf

    CMI_PS_NO_MORE_PS_DATA_REQ = 0xa0,              //CmiPsNoMorePsDataReq
    CMI_PS_NO_MORE_PS_DATA_CNF,                     //CmiPsNoMorePsDataCnf

    CMI_PS_SEND_CP_DATA_REQ,                        //AT+CSODCP, CmiPsSendOriDataViaCpReq
    CMI_PS_SEND_CP_DATA_CNF,                        //CmiPsSendOriDataViaCpCnf

    CMI_PS_SET_MT_CP_DATA_REPORT_CFG_REQ,           //AT+CRTDCP=<reporting>
    CMI_PS_SET_MT_CP_DATA_REPORT_CFG_CNF,
    CMI_PS_GET_MT_CP_DATA_REPORT_CFG_REQ,           //AT+CRTDCP?
    CMI_PS_GET_MT_CP_DATA_REPORT_CFG_CNF,
    CMI_PS_GET_MT_CP_DATA_REPORT_CAPA_REQ,          //AT+CRTDCP=?
    CMI_PS_GET_MT_CP_DATA_REPORT_CAPA_CNF,

    CMI_PS_MT_CP_DATA_IND,                          //+CRTDCP:<cid>,<cpdata_length>,<cpdata>

    CMI_PS_GET_APN_RATE_CTRL_PARM_REQ = 0xb0,       //AT+CGAPNRC, CmiPsGetAPNRateCtrlReq
    CMI_PS_GET_APN_RATE_CTRL_PARM_CNF,              //CmiPsGetAPNRateCtrlCnf

    CMI_PS_GET_BEARER_IPADDR_REQ,                   //AT+CGPADDR = <cid1>[,<cid2>], CmiPsGetBearerIpAddrReq
    CMI_PS_GET_BEARER_IPADDR_CNF,                   //CmiPsGetBearerIpAddrCnf

    /*
     * AT+ECSENDDATA=<cid>,<data_length>,<data>[,<RAI>[,<type_of_user_data>]]
     * Note:
     * 1> User could send non-ip/ip data via this AT CMD;
     * 2> Not the same as: +CSODCP,
     *    AT+ECSENDDATA: could send data via CP or UP
     *    AT+CSODCP:   only send the data via CP
    */
    CMI_PS_SEND_UL_DATA_REQ,                        //AT+ECSENDDATA=<cid>,<data_length>,<data>[,<RAI>[,<type_of_user_data>]], CmiPsSendUlDataReq
    CMI_PS_SEND_UL_DATA_CNF,                        //CmiPsSendUlDataCnf

    /*
     * for NON-IP data, just foward to AT CMD SRV UART
    */
    CMI_PS_RECV_DL_NON_IP_DATA_IND,                 //+RECVNONIP: <cid>,<data_length>,<data> , CmiPsRecvDlNonIpDataInd

    CMI_PS_TRANS_CIOT_PLANE_REQ,                    //AT+ECCIOTPLANE=plane, to select CP or UP, CmiPsTransCiotPlaneReq
    CMI_PS_TRANS_CIOT_PLANE_CNF,                    //CmiPsTransCiotPlaneCnf

    CMI_PS_GET_CONN_STATUS_REQ,                     //CmiPsGetConnStatusReq, AT+CSCON?
    CMI_PS_GET_CONN_STATUS_CNF,                     //CmiPsGetConnStatusCnf

    CMI_PS_CONN_STATUS_IND,                         //CmiPsConnStatusInd, +CSCON: <mode>
    CMI_PS_CNEC_ERROR_CODE_REPORT_IND,              //CmiPsCnecErrorCodeReportInd, +CNEC: <error_code>[,<cid>]

    CMI_PS_SET_UE_OPERATION_MODE_REQ  = 0xc0,       //AT+CEMODE, CmiPsSetUeOperationModeReq
    CMI_PS_SET_UE_OPERATION_MODE_CNF,               //CmiPsSetUeOperationModeCnf
    CMI_PS_GET_UE_OPERATION_MODE_REQ,               //CmiPsGetUeOperationModeReq
    CMI_PS_GET_UE_OPERATION_MODE_CNF,               //CmiPsGetUeOperationModeCnf

    CMI_PS_SET_DATA_OFF_REQ,                        //AT+CPSDO= Status, CmiPsPsDataoffReq
    CMI_PS_SET_DATA_OFF_CNF,                        //AT+CPSDO= Status, CmiPsPsDataoffCnf

    CMI_PS_GET_DATA_OFF_REQ,                        //CmiPsGetPsDataoffReq
    CMI_PS_GET_DATA_OFF_CNF,                        //CmiPsGetPsDataoffCnf


    //AT+ECNASTCFG
    CMI_PS_SET_TIMER_PARA_REQ = 0xd0,  //
    CMI_PS_SET_TIMER_PARA_CNF,         //

    CMI_PS_GET_TIMER_PARA_REQ,         //
    CMI_PS_GET_TIMER_PARA_CNF,         //


    #if 0
    CMI_PS_ACT_DED_HANDSHAKE_RESULT_REQ,
    CMI_PS_ACT_DED_HANDSHAKE_RESULT_CNF,

    CMI_PS_MOD_DED_HANDSHAKE_RESULT_REQ,
    CMI_PS_MOD_DED_HANDSHAKE_RESULT_CNF,
    #endif

    //AT+CABTSR / AT+CABTRDP, -TBD

    //CMI_PS_GET_CEER_REQ ,                           //CmiPsGetCeerReq, AT+CEER
    //CMI_PS_GET_CEER_CNF,                            //CmiPsGetCeerCnf

    CMI_PS_GET_DATA_COUNTER_REQ,                    //CmiPsGetDataCounterReq
    CMI_PS_GET_DATA_COUNTER_CNF,                    //CmiPsGetDataCounterCnf

    CMI_PS_SET_DATA_COUNTER_REQ,                    //CmiPsSetDataCounterReq
    CMI_PS_SET_DATA_COUNTER_CNF,                    //CmiPsSetDataCounterCnf

    CMI_PS_GET_SAVE_DATA_COUNTER_PERIOD_REQ,        //CmiPsGetSaveDataCounterPeriodReq
    CMI_PS_GET_SAVE_DATA_COUNTER_PERIOD_CNF,        //CmiPsGetSaveDataCounterPeriodCnf

    CMI_PS_SET_SAVE_DATA_COUNTER_PERIOD_REQ = 0xe0, //CmiPsSetSaveDataCounterPeriodReq
    CMI_PS_SET_SAVE_DATA_COUNTER_PERIOD_CNF,        //CmiPsSetSaveDataCounterPeriodCnf

    CMI_PS_PKG_DATA_TRANS_STATE_IND,                //CmiPsPkgDataTransStateInd

    CMI_PS_GET_TRAFFIC_IDLE_MONITOR_REQ,            //CmiPsGetTrafficIdleMonitorReq
    CMI_PS_GET_TRAFFIC_IDLE_MONITOR_CNF,            //CmiPsGetTrafficIdleMonitorCnf

    CMI_PS_SET_TRAFFIC_IDLE_MONITOR_REQ,            //CmiPsSetTrafficIdleMonitorReq
    CMI_PS_SET_TRAFFIC_IDLE_MONITOR_CNF,            //CmiPsSetTrafficIdleMonitorCnf
    CMI_PS_TRAFFIC_IDLE_MONITOR_IND,                //CmiPsTrafficIdleMonitorInd

    CMI_PS_SET_UE_VOICE_DOMAIN_PREFERENCE_REQ,      //AT+CEVDP, CmiPsSetUeVoiceDomainPreferenceReq
    CMI_PS_SET_UE_VOICE_DOMAIN_PREFERENCE_CNF,      //CmiPsSetUeVoiceDomainPreferenceCnf
    CMI_PS_GET_UE_VOICE_DOMAIN_PREFERENCE_REQ,      //CmiPsGetUeVoiceDomainPreferenceReq
    CMI_PS_GET_UE_VOICE_DOMAIN_PREFERENCE_CNF,      //CmiPsGetUeVoiceDomainPreferenceCnf

#if 0
    CMI_PS_SEND_RRC_CONN_EST_REQ,                   //CmiPsSendRrcConnEstReq
    CMI_PS_SEND_RRC_CONN_EST_CNF,                   //CmiPsSendRrcConnEstCnf

    CMI_PS_SET_L2_MONITOR_REPORT_REQ,               //CmiPsSetL2MonitorReportReq
    CMI_PS_SET_L2_MONITOR_REPORT_CNF,               //CmiPsSetL2MonitorReportCnf

    CMI_PS_L2_CONGEST_REPORT_IND    = 0xf0,         //CmiPsL2CongestReportInd
    CMI_PS_L2_ROHC_CONFIG_IND,                      //CmiPsL2RohcConfigInd
    CMI_PS_ROHC_ERROR_REPORT_IND,                   //CmiPsRohcErrorReportInd
#endif

    CMI_PS_GET_ECSMER_REQ ,                         //CmiPsGetEcsmerReq, AT+ECSMER
    CMI_PS_GET_ECSMER_CNF,                          //CmiPsGetEcsmerCnf
    CMI_PS_DEL_ECSMER_REQ,                          //CmiPsDelEcsmerReq
    CMI_PS_DEL_ECSMER_CNF,                          //CmiPsDelEcsmerCnf

    CMI_PS_PRIM_END = 0x0fff
}CMI_PS_PRIM_ID;

#if 0
typedef enum NmCmsPrimId_enum
{
    NM_ATI_PRIM_BASE    = 0x00,

    /*
     * ASYN PRIM ID, REQ/CNF
    */
    NM_ATI_ASYNC_BASE   = 0x00,

    NM_ATI_ASYNC_GET_DNS_REQ, //ASYNC reqeust
    NM_ATI_ASYNC_GET_DNS_CNF, //NmAtiGetDnsCnf

    NM_ATI_ASYNC_END    = 0X1F,
    /*
     * NM IND PRIM ID
    */
    NM_ATI_PING_RET_IND = 0x20,
    NM_ATI_IPERF_RET_IND,
    NM_ATI_NET_INFO_IND,
    NM_ATI_SNTP_RET_IND,
    NM_ATI_LAN_INFO_IND,
    NM_ATI_PRIM_END = 0XFF,
}NmCmsPrimId;
#endif


typedef enum CmiPsBearerTypeTag
{
    CMI_PS_BEARER_NULL,
    CMI_PS_BEARER_DEFAULT,
    CMI_PS_BEARER_DEDICATED,
    CMI_PS_BEARER_MAX_TYPE
}CmiPsBearerType;

/*
 * CGEV reason
 * <reason>: integer type; indicates the reason why the context activation request for PDP type IPv4v6 was not
 *   granted. This parameter is only included if the requested PDP type associated with <cid> is IPv4v6, and the
 *   PDP type assigned by the network for <cid> is either IPv4 or IPv6.
 * 0 IPv4 only allowed
 * 1 IPv6 only allowed
 * 2 single address bearers only allowed.
 * 3 single address bearers only allowed and MT initiated context activation for a second address type bearer was
 *   not successful.
*/
typedef enum CmiPsPdnTypeReasonTag
{
    CMI_PS_IPV4_ONLY_ALLOWED,
    CMI_PS_IPV6_ONLY_ALLOWED,
    CMI_PS_SINGLE_ADDR_BEARER_ONLY_ALLOWED,
    CMI_PS_SINGLE_ADDR_ONLY_ALLOWED_BUT_ACTIVE_SECOND_BEARER_FAILED,    // - NOT support now
    CMI_PS_PDN_TYPE_REASON_NULL  //indicate no reason
}CmiPsPdnTypeReason;

/*
 * TS27007 10.1.1, only IPv4, IPv6 and IPv4v6 values are supported for EPS service.
*/
typedef enum CmiPsPdnTypeTag
{
    //CMI_PS_PDN_TYPE_PPP = 0,  /* PPP */
    CMI_PS_PDN_TYPE_IP_V4 = 1,  /* IPv4 */
    CMI_PS_PDN_TYPE_IP_V6,      /* IPv6 */
    CMI_PS_PDN_TYPE_IP_V4V6,    /* IPv4v6 */
    //CMI_PS_PDN_TYPE_X25,      /* X.25 */
    //CMI_PS_PDN_TYPE_OSPIH,    /* OSPIH */
    CMI_PS_PDN_TYPE_NON_IP =5,     /* NON IP*/
    CMI_PS_PDN_TYPE_NUM
}CmiPsPdnType;

/*at+cgdcont/at+ecattbearer the apn parameters presenttype*/
typedef enum CmiApnPresentType_Enum
{
    CMI_UPDATE_WITH_DEFAULT = 0,      /*no apnString information,and still uses former apn name*/
    CMI_UPDATE_WITH_NEW,    /*use input apnString information,example : input a ""(len ==0) or "abcd"(len > 0) */
}CmiApnPresentType;


/*
 *  Authentication protocol used for this PDP context
*/
typedef enum CmiSecurityProtocolTag
{
    CMI_SECURITY_PROTOCOL_NULL  = 0,    /*null*/
    CMI_SECURITY_PROTOCOL_PAP   = 1,    /*PAP*/
    CMI_SECURITY_PROTOCOL_CHAP  = 2,         /*CHAP*/
    CMI_SECURITY_PROTOCOL_CHAP_PAP =3,  /*If CHAP failed, then PAP */


    CMI_SECURITY_PROTOCOL_PPP_LCP_PAP   = 13,   /* AUTH info extract from PPP LCP AUTH info, which type is PAP,
                                                 * difference with: CMI_SECURITY_PROTOCOL_PAP:
                                                 * a) "PPP_LCP_PAP" not need to save NVM, and when PDP activated/deactivated could remove.
                                                 * b) "PPP_LCP_PAP" not need to return in "CmiPsGetDefineAuthCtxCnf", when read the AUTH info
                                                */
    CMI_SECURITY_PROTOCOL_PPP_LCP_CHAP  = 14,   /* AUTH info extract from PPP LCP AUTH info, which type is CHAP,
                                                 * difference with: CMI_SECURITY_PROTOCOL_CHAP:
                                                 * a) "PPP_LCP_CHAP" not need to save NVM, and when PDP activated/deactivated could remove.
                                                 * b) "PPP_LCP_CHAP" not need to return in "CmiPsGetDefineAuthCtxCnf", when read the AUTH info.
                                                 * c) auth info is not username/paaword, but encoded CHAP challenge and CHAP response info.
                                                */

    CMI_SECURITY_PROTOCOL_NUM   = 0xF   /* 4 bits */
}CmiSecurityProtocol;


typedef enum CmiPsBearerStateTag
{
    CMI_PS_BEARER_INVALID           = 0,
    CMI_PS_BEARER_DEFINED           = 1,
    CMI_PS_BEARER_ACTIVATING        = 2,
    CMI_PS_BEARER_ACTIVATED         = 3,
    CMI_PS_BEARER_MODIFING          = 4,
    CMI_PS_BEARER_DEACTIVATING      = 5,
    CMI_PS_BEARER_MAX_STATE
}CmiPsBearerState;

typedef enum CmiPsIpv4AllocTypeTag
{
    CMI_IPV4_ADDR_ALLOC_THROUGH_NAS_SIGNALNG,
    CMI_IPV4_ADDR_ALLOC_THROUGH_DHCP
}CmiPsIpv4AllocType;

typedef enum CmiPsPcscfDiscoveryTypeTag
{
    CMI_PS_PCSCF_ADDR_DISCOVERY_NOT_INFLUENCED_BY_CGDCONT,
    CMI_PS_PCSCF_ADDR_DISCOVERY_THROUGH_NAS_SIGNALNG,
    CMI_PS_PCSCF_ADDR_DISCOVERY_THROUGH_DHCP
}CmiPsPcscfDiscoveryType;

typedef enum CmiPsIpv4MtuDiscoveryTypeTag
{
    CMI_IPV4_ADDR_MTU_SIZE_DISCOVERY_NOT_INFLUENCED_BY_CGDCONT,
    CMI_IPV4_ADDR_MTU_SIZE_DISCOVERY_THROUGH_NAS_SIGNALNG
}CmiPsIpv4MtuDiscoveryType;


typedef enum CmiPsNonIpMtuDiscoveryTypeTag
{
    CMI_PS_NONIP_MTU_SIZE_DISCOVERY_NOT_INFLUENCED_BY_CGDCONT,
    CMI_PS_NONIP_MTU_SIZE_DISCOVERY_THROUGH_NAS_SIGNALNG
}CmiPsNonIpMtuDiscoveryType;

/*
 * request_type, 27.007
 * 0 PDP context is for new PDP context establishment or for handover from a non-3GPP access network (how
 *   the MT decides whether the PDP context is for new PDP context establishment or for handover is
 *   implementation specific)
 * 1 PDP context is for emergency bearer services
 * 2 PDP context is for new PDP context establishment
 * 3 PDP context is for handover from a non-3GPP access network
 * 4 PDP context is for handover of emergency bearer services from a non-3GPP access networ
*/
typedef enum CmiPsBearerReqTypeTag
{
    CMI_PS_NEW_BEARER_NO_KNOWN_REQ = 0,
    CMI_PS_EMERGENCY_BEARER_REQ = 1,    //support, if enable MACO: PS_ENABLE_EMC_BR_FEATURE
    CMI_PS_NEW_BEARER_REQ = 2,
    CMI_PS_HO_BEARER_REQ  = 3,
    CMI_PS_HO_EMERGENCY_BEARER_REQ = 4  //not support
}CmiPsBearerReqType;

/*
 * IM_CN_Signalling_Flag, 27.007
 * 0 UE indicates that the PDP context is not for IM CN subsystem-related signalling only
 * 1 UE indicates that the PDP context is for IM CN subsystem-related signalling only
*/
typedef enum CmiPsImCnSigFlagTag
{
    CMI_PS_BEARER_NOT_FOR_IM_CN_SIG_ONLY = 0,
    CMI_PS_BEARER_FOR_IM_CN_SIG_ONLY
}CmiPsImCnSigFlag;

/*
 * NSLPI, 27.007
 * <NSLPI>: integer type; indicates the NAS signalling priority requested for this PDP context:
 * 0 indicates that this PDP context is to be activated with the value for the low priority indicator configured in the MT.
 * 1 indicates that this PDP context is is to be activated with the value for the low priority indicator set to "MS is not configured for NAS signalling low priority".
*/
typedef enum CmiPsNSLPICfgTag
{
    CMI_PS_CFG_FOR_NSLPI = 0,
    CMI_PS_NOT_CFG_FOR_NSLPI
}CmiPsNSLPICfg;


#define CMI_PS_IS_BEARER_ACTIVTED(cmiPsBrState)   (((cmiPsBrState) == CMI_PS_BEARER_ACTIVATED) || ((cmiPsBrState) == CMI_PS_BEARER_MODIFING))
#define CMI_PS_IS_BEARER_INVALID(cmiPsBrState)    (((cmiPsBrState) == CMI_PS_BEARER_INVALID) || ((cmiPsBrState) >= CMI_PS_BEARER_MAX_STATE))


/******************************************************************************
 *****************************************************************************
 * CMI STRUCT
 *****************************************************************************
******************************************************************************/

/*
 * <cid>,<QCI>,[<DL_GBR>,<UL_GBR>],[<DL_MBR>,<UL_MBR>][,<DL_AMBR>,<UL_AMBR>]
*/
typedef struct CmiPsEpsQosDynParamsTag
{
    UINT8       cid;
    UINT8       qci;
    UINT16      gbrPresent : 1; /* indicate the present of GBR */
    UINT16      mbrPresent : 1; /* indicate the present of MBR */
    UINT16      ambrPresent: 1; /* indicate the present of AMBR */
    UINT16      reserved0 : 13;

    UINT32      ulGBR;          /* UL_GBR, kbit/s */
    UINT32      dlGBR;          /* DL_GBR, kbit/s */
    UINT32      ulMBR;          /* UL_MBR, kbit/s */
    UINT32      dlMBR;          /* DL_MBR, kbit/s */
    UINT32      ulAMBR;
    UINT32      dlAMBR;
}CmiPsEpsQosDynParams; //28 bytes

/*
 * Local/remote port range
*/
typedef struct CmiPsPfPortInfoTag
{
    UINT16    min;
    UINT16    max;
}CmiPsPfPortInfo;

/*
 * <type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>
*/
typedef struct CmiPsTosMaskTag
{
    BOOL                present;
    UINT8               rsvd;
    UINT8               tos;        /* <type of service (tos) (ipv4)> / traffic class (ipv6)>*/
    UINT8               mask;       /* mask */
}CmiPsTosMask;

/*
 * CMI_PS_DEFINE_TFT_PARM_REQ, //AT+CGTFT=[<cid>, ...],
 * sizeof(CmiPsPacketFilter) = 56
*/
typedef struct CmiPsPacketFilter_struct
{
    UINT8               cid;
    UINT8               pktFilterId;    /* a) packet filter identifier, [0:15], and in AT layer this range is: [1:16],
                                         * b) so AT layer need to +1 when print out */
    UINT8               epIdx;          /* evaluation precedence index, [0:255] */
    BOOL                protIdPresent;

    BOOL                remotePortPresent;
    BOOL                localPortPresent;
    BOOL                ipSpiPresent;
    BOOL                ipv6FLPresent;

    UINT8               protId;     /* Protocol identifier/Next header type */
    UINT8               rsvd0;
    UINT16              rsvd1;

    CmiPsTosMask        tosMask;    /*<type of service (tos) (ipv4) and mask / traffic class (ipv6) and mask>*/

    CmiPsPfPortInfo     remotePort;     /*remote port range*/
    CmiPsPfPortInfo     localPort;      /*local port range*/
    UINT32              ipSpi;          /*ipsec security parameter index (spi)*/
    UINT32              ipv6FL;         /*flow label (ipv6)*/

    CmiIpAddr           remoteAddr;     /*<remote address and subnet mask>*/

    UINT8               dir;
    UINT8               nwPktFilterId;  /* ref 27.007: NW packet filter Identifier
                                         * The value range is from 1 to 16. In EPS the value is assigned by the network when established
                                         * a) Now same value as: "pktFilterId", and range is: [0:15]
                                         * b) And AT layer need to +1, and range in AT layer: [1:16] when print out
                                        */
    UINT16              reserved0;
}CmiPsPacketFilter;


/* Info in CGDCONT */
typedef struct CmiDefPdpDefinition_Tag
{
    UINT8   cid;
    UINT8   pdnType;        //CmiPsPdnType
    UINT8   apnPresentType; /* CmiApnPresentType, record this input apnstring type : NULL or ""*/
    UINT8   apnLength;      /* 0 - apn is not present */
    UINT8   apnStr[CMI_PS_MAX_APN_LEN]; //apn string, readable format, example: cmnbiot.mnc004.mcc460.gprs

    /* "PDP_addr" is ignored with the set command */
    /* "d_comp" & "h_comp" is for 2/3G, don't need */

    BOOL    ipv4AlloTypePresent;
    UINT8   ipv4AlloType;   /* CmiPsIpv4AllocType. IPv4 address allocation through NAS signalling, or by DHCP */

    BOOL    reqTypePresent;
    UINT8   reqType;        /*CmiPsBearerReqType */

    BOOL    NSLPIPresent;
    UINT8   NSLPI;          /*CmiPsNSLPICfg, NAS signalling priority*/

    BOOL    ipv4MtuDisTypePresent;
    BOOL    ipv4MtuDisByNas;    /* IPv4 MTU size discovery through NAS signalling */

    BOOL    RDSPresent;
    UINT8   RDS;                /* 0-Reliable Data Service is not being used for the PDN connection;
                                 * 1-Reliable Data Service is being used for the PDN connection
                                */


    BOOL    secPcoPresent;
    UINT8   secPco;             /* 0 Security protected transmission of PCO is not requested
                                 * 1 1 Security protected transmission of PCO is requested
                                */
    //UINT16  reserved1;

    /*
     * <securePCO>, not support now (PAP/CHAP)
    */

    /*
     * <Local_Addr_Ind>, not support now
     * "local IP address in TFT"
    */

    /*
     * IMS/pcscf not support now
    */
    BOOL    pcscfDisTypePresent;
    UINT8   pcscfDisType;       //CmiPsPcscfDiscoveryType

    BOOL    imCnSigFlagPresent;
    UINT8   imCnSigFlag;        //CmiPsImCnSigFlag

    BOOL    noSaveNvmFlag;      /* When cid equal to EPS attach cid, this flag indicate whether save in pdp nv context.
                                 * 0: pdp context save in nv context, default value.
                                 * 1: pdp context don`t save in nv context;
                                 */
    UINT8   reserved[3];
}CmiPsDefPdpDefinition;     /* 124 bytes */

typedef struct DefineDedBearerCtxInfoTag
{
    UINT8               cid; // context Id
    UINT8               pCid;// primary cid
    UINT8               dComp;//controls PDP data compression
    UINT8               hComp;//controls PDP header copmpression
    BOOL                imCnSigFlagPresent;
    UINT8               imCnSigFlag; //indication pdp whether is IMS related pdp
    UINT16              rsvd;
}DefineDedBearerCtxInfo;        /* 8 bytes */



typedef struct CmiPsDedicateBearerCtxReqTag
{
    UINT8   cid;
    UINT8   pCid;        //primary cid,  The list of permitted values is returned by the test form of the command.
    /*
     * <d_comp>,<h_comp> is used for 2/3G;
     * default value is 0 - off in AT CMD SRV
    */

    BOOL    imCnSigFlagPresent;
    UINT8   imCnSigFlag;    //CmiPsImCnSigFlag

}CmiPsDedicateBearerCtxReq;         /* 4 bytes */


typedef struct CmiPsBearerCtxDynParamTag
{
    UINT8               cid; // context Id
    UINT8               bid; // bearer Id
    UINT8               reserved0;
    UINT8               apnLength;          /* 0 - apn is not present */
    UINT8               apnStr[CMI_PS_MAX_APN_LEN]; //apn string, readable format, example: cmnbiot.mnc004.mcc460.gprs
    CmiIpAddr           ipv4Addr;   // if bearer is non-ip type, ipv4Addr/ipv6Addr both invalid
    CmiIpAddr           ipv6Addr;
    CmiIpAddr           gwIpv4Addr;

    UINT8               dnsAddrNum;
    UINT8               pCscfAddrNum; //max value is CMI_PCSCF_MAX_NW_ADDR_NUM
    UINT8               imCnSigFlag;  //0, 1
    UINT8               lipaInd;      //0, 1;

    CmiIpAddr           dnsAddr[CMI_PDN_MAX_NW_ADDR_NUM];
    CmiIpAddr           pCscfAddr[CMI_PCSCF_MAX_NW_ADDR_NUM];

    BOOL                rdsInd;
    BOOL                ipv4MtuPresent;
    UINT16              ipv4Mtu;
    BOOL                psDataOffSupport;
    //UINT16              servingPlmnRateCtrl;
    BOOL                needRetryAnotherIPType;//retry another IP type bearer needed;
    BOOL                bEmergency;         /* whether bearer is emergency bearer,
                                            *1> Emergency ATTACH, this flag will set TRUE and ignore CGDCONT set type;
                                            *2> Normal ATTACH, if CGDCONT set "emergency" type, this flag will set TRUE;
                                            *3> additional BR actived, if CGDCONT set "emergency" type, this flag will set TRUE;
                                            */
    UINT8               rsvd1;

}CmiPsBearerCtxDynParam;    //376 bytes

typedef struct CmiPsDedBearerCtxDynParamTag
{
    UINT8               cid; // context Id
    UINT8               pCid;//
    UINT8               bid; // bearer Id

    UINT8               imCnSigFlagPresent:1;
    UINT8               imCnSigFlag:1;  //0, 1
    UINT8               rsvd:6;
}CmiPsDedBearerCtxDynParam;



/*
 * CMI_PS_DEFINE_BEARER_CTX_REQ
 *  AT+CGDCONT=[<cid>[,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>
 *             [,<h_comp>[,<IPv4AddrAlloc>[,<request_type>[,<PCSCF_discovery>
 *             [,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>[,<IPv4_MTU_discovery>]
 *             [,<Local_Addr_Ind>][,<NonIP_MTU_discovery>]]]]]]]]]]]]]
*/
typedef struct CmiPsDefineBearerCtxReqTag
{
    CmiPsDefPdpDefinition          bearerCtxInfo;
}CmiPsDefineBearerCtxReq;


/*
 * CMI_PS_DEFINE_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig CmiPsDefineBearerCtxCnf;


/*
 * CMI_PS_GET_DEFINED_BEARER_CTX_REQ
*/
typedef struct CmiPsGetDefinedBearerCtxReqTag
{
    BOOL    bAllCid;    //whether get all defined bearer context info
    UINT8   cid;        //if "bAllCid" == FALSE, specified one CID
    UINT16  reserved1;
}CmiPsGetDefinedBearerCtxReq;



/*
 * CMI_PS_GET_SEC_BEARER_CTX_REQ
*/
typedef struct CmiPsGetDedBearerCtxReqTag
{
    BOOL    bAllCid;    //whether get all defined bearer context info
    UINT8   cid;        //if "bAllCid" == FALSE, specified one CID
    UINT8   reserved1;
}CmiPsGetDedBearerCtxReq;

/*
 * CMI_PS_GET_DEFINED_BEARER_CTX_CNF
 * +CGDCONT: <cid>,<PDP_type>,<APN>,<PDP_addr>,<d_comp>,<h_comp>
 *           [,<IPv4AddrAlloc>[,<request_type>[,<PCSCF_discovery>
 *           [,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>
 *           [,<IPv4_MTU_discovery>[,<Local_Addr_Ind>[,<NonIP_MTU_discovery>]]]]]]]]]
*/

typedef struct CmiPsGetDefinedBearerCtxCnfTag
{
    /*
     * 1> AT+CGDCONT? is to query all the defined PDP info, in order to reduce the struct size,
     *     each time return one bearer info;
     * 2> here, why also return the "CmiPsBearerCtxDynParam"? as some user want to return NW acivated info in
     *    CGDCONT query, such as the APN & IP type. here we also carried these info, let user decide how to use.
     * 3> if "bContinue" set to FALSE, means the last bearer info, and "definePresent"/"dynParamPresent" maybe FALSE,
     *    in case of last bearer is dedicated bearer
    */
    BOOL        bContinue;
    BOOL        definePresent;      /* whether 'pdpDefine' is valid */
    BOOL        dynParamPresent;
    UINT8       rsvd;

    CmiPsDefPdpDefinition       pdpDefine;      /* 120 bytes */
    CmiPsBearerCtxDynParam      pdpDynParam;    /* 376 bytes */
}CmiPsGetDefinedBearerCtxCnf;

/*
 * CMI_PS_DEL_BEARER_CTX_REQ
*/
typedef struct CmiPsDelBearerCtxReqTag
{
    UINT8           cid;
    UINT8           reserved;
    UINT16          reserved2;
}CmiPsDelBearerCtxReq;


/*
 * CMI_PS_DEL_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig  CmiPsDelBearerCtxCnf;


/*
 * CMI_PS_DEFINE_DEDICATED_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig CmiPsDefineDedicateBearerCtxCnf;

/*
* CMI_PS_DEL_DEDICATED_BEARER_CTX_REQ
*/
typedef struct CmiPsDelDedBearerCtxReqTag
{
    UINT8           cid;
    UINT8           reserved;
    UINT16          reserved2;
}CmiPsDelDedBearerCtxReq;

/*
* CMI_PS_DEL_DEDICATED_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig  CmiPsDelDedBearerCtxCnf;


/*
 * CMI_PS_GET_DEFINED_DEDICATED_BEARER_CTX_REQ
*/
typedef struct CmiPsGetDedicateBearerCtxReqTag
{
    BOOL    bAllCid; //whether get all defined bearer context info
    UINT8   cid;    //if "bAllCid" == FALSE, specified one CID
    UINT16  reserved1;
}CmiPsGetDedicateBearerCtxReq;

/*
 * CMI_PS_GET_DEFINED_DEDICATED_BEARER_CTX_CNF
 * [+CGDSCONT: <cid>,<p_cid>,<d_comp>,<h_comp>,<IM_CN_Signalling_Flag_Ind>]
*/
typedef struct CmiPsGetDefinedDedBearerCtxTag
{
    UINT8   cid;
    UINT8   pCid;
    BOOL    imCnSigFlagPresent;
    UINT8   imCnSigFlag;    //CmiPsImCnSigFlag
}CmiPsGetDefinedDedBearerCtx;


typedef struct CmiPsGetDefinedDedBearerCtxCnfTag
{
    BOOL    bContinue;
    BOOL    bCtxValid;  /*whether 'definedBrCtx' is valid*/
    CmiPsGetDefinedDedBearerCtx  dedCtxInfo;
}CmiPsGetDefinedDedBearerCtxCnf;


/*
 * CMI_PS_SET_BEARER_ACT_STATE_REQ
*/
typedef struct CmiPsSetBearerActStateReqTag
{
    UINT8           cid;
    UINT8           state; //indicates the state of bearer context activation. 1--activated; 0 - deactivated
    UINT16          reserved;
}CmiPsSetBearerActStateReq;

/*
 * CMI_PS_SET_BEARER_ACT_STATE_CNF
*/
typedef CamCmiEmptySig  CmiPsSetBearerActStateCnf;

/*
 * CMI_PS_DETACH_STATE_IND
 * +CGEV:NW DETACH
 * +CGEV:ME DETACH
*/
typedef struct CmiPsDetachStateIndTag
{
    BOOL           isMeInitial; /* indicate the detach is intialed by ME(TRUE) or NW(FALSE) */
    UINT8          rsvd;
    UINT16         rsvd1;
}CmiPsDetachStateInd;


/*
 * CMI_PS_BEARER_ACTED_IND ,
 * default pdn //pCid=0, bearerType=CMI_PS_BEARER_DEFAULT
 * +CGEV: NW PDN ACT <cid>
 * +CGEV: ME PDN ACT <cid>[,<reason>,...

 * dedicate pdn //<pCid> should exist, <bearerType> should be CMI_PS_BEARER_DEDICATED, <event_type>: only support: 0 Informational event
 * +CGEV: NW ACT <p_cid>, <cid>, <event_type>
 * +CGEV: ME ACT <p_cid>, <cid>, <event_type>
*/

typedef struct CmiPsBearerActedIndTag
{
    UINT8       cid;
    UINT8       pCid;           // the associated primary cid,if not equal to 0, bearerType should be CMI_PS_BEARER_DEDICATED;
    UINT8       bearerType;     //CmiPsBearerType
    BOOL        isMeInitial;    /* indicate the bearer activation is intialed by ME(TRUE) or NW(FALSE) */

    UINT8       pdnReason;      //CmiPsPdnTypeReason
    //UINT8     reserved1;
    BOOL        isRemap;        /* Whether current bearer is a remaped bearer
                                 * TRUE - PDP/bearer is remaped from other acted bearer, not acted by NW this time
                                 * FALSE - acted by PDN connection request procedure
                                */
    UINT16      reserved2;
}CmiPsBearerActedInd;


/*
 * CMI_PS_BEARER_MODIFY_IND ,
 * +CGEV: NW MODIFY <cid>, <change_reason>, <event_type>[,<WLAN_Offload>]
 * +CGEV: ME MODIFY <cid>, <change_reason>, <event_type>[,<WLAN_Offload>]
 * Note:
 *  <change_reason>: TFT changed/Qos changed
 *  <event_type>: only support: 0 Informational event
*/

/* bitmap that indicates what kind of change occurred, CGEV print value */
typedef enum CmiPsBearerModifyReason_enum
{
    /*
     * Bit 1 TFT changed    => 'changeReasonBitmap' = 1
     * Bit 2 Qos changed,   => 'changeReasonBitmap' = 2
     * Bit 1&2, TFT and Qos changed => 'changeReasonBitmap' = 3
    */
    //CMI_PS_NONE_CHANGED = 0,
    CMI_PS_TFT_CHANGED_BITMAP   = 0,    /* 3GPP */
    CMI_PS_QOS_CHANGED_BITMAP   = 1,    /* 3GPP */

    CMI_PS_BR_REACT_BITMAP      = 7     /* internal: Bearer re-activated */
}CmiPsBearerModifyReason;

typedef struct CmiPsBearerModifyIndTag
{
    UINT8           cid;
    BOOL            bUeInited;          /*whether UE init this modification procedure*/
    UINT8           changeReasonBitmap; /*bit map of "CmiPsBearerModifyReason" */
    UINT8           rsvd;

    //CmiPsEpsQosDynParams       qos;
    //CmiPsPacketFilter             Tft;
}CmiPsBearerModifyInd;


/*
 * CMI_PS_BEARER_DEACT_IND ,
 * default pdn //pCid=0, bearerType=CMI_PS_BEARER_DEFAULT
 * +CGEV: NW PDN DEACT <cid>  //network init
 * +CGEV: ME PDN DEACT <cid>  //UE init

 * dedicate pdn //<pCid> should exist, <bearerType> should be CMI_PS_BEARER_DEDICATED,
 *                <event_type>: only support: 0 Informational event
 * +CGEV: NW DEACT <p_cid>, <cid>, <event_type>
 * +CGEV: ME DEACT <p_cid>, <cid>, <event_type>
*/
typedef struct CmiPsBearerDeActIndTag
{
    UINT8           cid;
    UINT8           pCid;           // the associated primary cid,if not equal to 0, bearerType should be CMI_PS_BEARER_DEDICATED;
    UINT8           bearerType;     //CmiPsBearerType
    BOOL            isMeInitial;    /* indicate the bearer activation is intialed by ME(TRUE) or NW(FALSE) */

    UINT16          esmCause;       /* ESM cause: EsmCause
                                     * 0 - no cause
                                     * [1:0xDF], 3GPP defined ESM cause: 24.301, 9.9.4.4
                                     * [0xE0: 0xFF], internal defined ESM cause
                                    */
    BOOL            isRemap;        /* Whether current bearer remap to other CID
                                     * TRUE - this bearer remap to other CID, not real deactivated
                                     * FALSE - bearer deactivated
                                    */
    UINT8           reserved2;
}CmiPsBearerDeActInd;


/*
 * CMI_PS_DEFINE_EPS_QOS_REQ
 * AT+CGEQOS=[<cid>[,<QCI>[,<DL_GBR>,<UL_GBR>[,<DL_MBR>,<UL_MBR]]]]
*/
typedef struct CmiPsEpsQosParamsTag
{
    UINT8               cid;
    UINT8               qci;
    BOOL                gbrMbrPresent;  /* indicate the present of GBR/MBR, need two BOOL ? */
    UINT8               reserved0;
    UINT32              ulMBR;          /* UL_MBR, kbit/s */
    UINT32              dlMBR;          /* DL_MBR, kbit/s */
    UINT32              ulGBR;          /* UL_GBR, kbit/s */
    UINT32              dlGBR;          /* DL_GBR, kbit/s */
}CmiPsEpsQosParams; //20 bytes

typedef struct CmiPsDefineEpsQoSReqTag
{
    CmiPsEpsQosParams   epsQosParam;
}CmiPsDefineEpsQoSReq;

/*
 * CMI_PS_DEFINE_EPS_QOS_CNF
*/
typedef CamCmiEmptySig CmiPsDefineEpsQoSCnf;

/*
 * CMI_PS_DEL_DEFINE_EPS_QOS_REQ
*/
typedef struct CmiPsDelEpsQosReq_Tag
{
    UINT8    cid;
}CmiPsDelEpsQosReq;


/*
 * CMI_PS_DEL_DEFINE_EPS_QOS_CNF
*/
typedef CamCmiEmptySig CmiPsDelEpsQosCnf;

/*
 * CMI_PS_GET_DEFINED_EPS_QOS_REQ
*/
typedef struct CmiPsGetDefinedEpsQoSReqTag
{
    BOOL        bAllCid;    /* return all BERRER EQOS info*/
    UINT8       cid;
    UINT16      reserved2;
}CmiPsGetDefinedEpsQoSReq;

/*
 * CMI_PS_GET_DEFINED_EPS_QOS_CNF
*/
typedef struct CmiPsGetDefinedEpsQoSCnfTag
{
    //UINT8   epsQosNum;
    //UINT8   reserved0;
    //UINT16  reserved1;
    /*
     * AT+CGEQOS? need to return all defined QoS.
     * As PS each time could only return one EQOS info, here use "bContinue" to decide any EQOS followed
    */
    BOOL        bContinue;
    BOOL        epsQosValid;
    UINT16      reserved0;
    CmiPsEpsQosParams   epsQosParam;
}CmiPsGetDefinedEpsQoSCnf;

/*
 * CMI_PS_READ_DYN_BEARER_EPS_QOS_REQ
 * AT+CGEQOSRDP[=<cid>]
*/
typedef struct CmiPsReadDynEpsQoSReqTag
{
    BOOL        bAllCid;
    UINT8       cid;    /* if "bAllCid" == FALSE */
    UINT16      reserved1;
}CmiPsReadDynEpsQoSReq;



/*
 * CMI_PS_READ_DYN_BEARER_EPS_QOS_CNF
*/
typedef struct CmiPsReadDynEpsQoSCnfTag
{
    /*
     * AT+CGEQOSRDP need to return all bearer configed/activated EQOS.
     * As PS each time could only return one EQOS info, here use "bContinue" to decide any EQOS followed
    */
    BOOL        bContinue;
    BOOL        epsQosValid;
    UINT16      reserved0;
    CmiPsEpsQosDynParams    epsQosDynParams;
}CmiPsReadDynEpsQoSCnf;


/*
 * CMI_PS_READ_BEARER_DYN_CTX_REQ
 * AT+CGCONTRDP? / AT+CGCONTRDP=<cid>
*/
typedef struct CmiPsReadBearerDynCtxParamReqTag
{
    BOOL        bAllCid;    //whether request all actived default bearer info
    UINT8       cid;        //if "bAllCid" == FALSE, get specified default bearer info;
    UINT16      reserved1;
}CmiPsReadBearerDynCtxParamReq;

/*
 * CMI_PS_READ_BEARER_DYN_CTX_CNF
*/
typedef struct CmiPsReadBearerDynCtxParamCnfTag
{
    /*
     * AT+CGCONTRDP? is to query all the actived default bearer info, in order to reduce the struct size,
     * Each time return one bearer info.
     * Note:
     * 1> If not request all ('bAllCid' == FALSE, 'cid' in CmiPsReadTFTDynCtxParamReq specified),
     *    a) if specified bearer is dedicated bearer,
     *      return: 'rc' = "CME_PDN_INVALID_TYPE", 'bearerCtxPresent' = FALSE, but 'ctxDynPara.cid' = request cid
          b) else if specified bearer not activate, ,
     *      return: 'rc' = "CME_PDN_NOT_ACTIVED", 'bearerCtxPresent' = FALSE, but 'ctxDynPara.cid' = request cid
     *
     * 2> If request all, and last bearer is not activated default bearer
     *    return: 'rc' = "CME_SUCC", 'bearerCtxPresent' = FALSE, 'ctxDynPara.cid' = 0xFF
    */
    BOOL                    bContBearer;
    BOOL                    bearerCtxPresent;
    UINT16                  reserved0;

    CmiPsBearerCtxDynParam  ctxDynPara;     //376 bytes
}CmiPsReadBearerDynCtxParamCnf;     //380 bytes

/*
 * CMI_PS_READ_DYN_TFT_CTX_REQ
 * AT+CGTFTRDP? / AT+CGTFTRDP=<cid>
*/
typedef struct CmiPsReadTFTDynCtxParamReqTag
{
    BOOL        bAllCid;    //whether request all actived default bearer info
    UINT8       cid;        //if "bAllCid" == FALSE, get specified default bearer info;
    UINT16      reserved1;
}CmiPsReadTFTDynCtxParamReq;

/*
 * CMI_PS_READ_DYN_TFT_CTX_CNF,
*/
typedef struct  CmiPsReadTFTDynCtxParamCnfTag
{
    /*
     * 1> If not request all ('bAllCid' == FALSE, 'cid' in CmiPsReadTFTDynCtxParamReq specified),
     *     and no TFT activated,
     *    return: 'rc' = "CME_PDN_NO_PARAM", 'cid' = request cid, 'numFilters' = 0
     * 2> If request all, and last bearer no TFT configed
     *    return: 'rc' = "CME_SUCC", 'cid' = 0xFF, 'numFilters' = 0
    */
    BOOL            bContinue;      //whether next bearer TFT info followed
    UINT8           cid;            //TFT Dyn config for CID
    UINT8           numFilters;
    UINT8           reserved0;
    CmiPsPacketFilter  filters[CMI_PS_MAX_TFT_FILTERS];    //52*16 = 832 bytes
}CmiPsReadTFTDynCtxParamCnf;    //836 bytes



/*
 * CMI_PS_READ_DEDICATED_BEARER_DYN_CTX_REQ
 * AT+CGSCONTRDP?
*/
typedef struct CmiPsReadDedBearerDynCtxParamReqTag
{
    BOOL        bAllCid;    //whether request all actived default bearer info
    UINT8       cid;        //if "bAllCid" == FALSE, get specified default bearer info;
    UINT16      reserved1;
}CmiPsReadDedBearerDynCtxParamReq;


/*
 * CMI_PS_READ_DEDICATED_BEARER_DYN_CTX_CNF
 * AT+CGSCONTRDP?
*/
typedef struct CmiPsReadDedBearerDynCtxParamCnfTag
{
    /*
     * AT+CGSCONTRDP? is to query all the actived Dedicate bearer info, in order to reduce the struct size,
     * Each time return one bearer info
     * Note:
     * 1> If not request all ('bAllCid' == FALSE, 'cid' in CmiPsReadDedBearerDynCtxParamReq specified),
     *    a) if specified bearer is default (not dedicated) bearer,
     *      return: 'rc' = "CME_PDN_INVALID_TYPE", 'bearerCtxPresent' = FALSE, but 'ctxDynPara.cid' = request cid
          b) else if specified bearer not activate/undefined,
     *      return: 'rc' = "CME_PDN_NOT_ACTIVED"/"CME_PDN_NOT_VALID", 'bearerCtxPresent' = FALSE, but 'ctxDynPara.cid' = request cid
     *
     * 2> If request all, and last bearer is not activated default bearer
     *    return: 'rc' = "CME_SUCC", 'bearerCtxPresent' = FALSE, 'ctxDynPara.cid' = 0xFF
    */
    BOOL                    bContBearer;
    BOOL                    bearerCtxPresent;
    UINT16                  reserved0;

    CmiPsDedBearerCtxDynParam  ctxDynPara;
}CmiPsReadDedBearerDynCtxParamCnf;  // 8 bytes




/*
 * CMI_PS_MODIFY_BEARER_CTX_REQ,
 *  +CGCMOD[=<cid>
*/
typedef struct CmiPsModifyBearerCtxReqTag
{
    UINT8           cid;
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsModifyBearerCtxReq;

/*
 * CMI_PS_MODIFY_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig CmiPsModifyBearerCtxCnf;


/*
 * CMI_PS_GET_ALL_BEARERS_CIDS_INFO_REQ
 * 1> This CMI will return all current basic bearers (cids) info;
 * 2> And several AT CMDs could be mapped to this CMI interface, in order to distinguish which AT CMD,
 *    involved a member: "getCidsCmd";
 * 3> PS will return all bearers (CIDs) basic info, no matter the request CMD ("getCidsCmd"),
 *    and AT CMD (psproxy) need to decide which info need to return (print) according to the member: "getCidsCmd"
*/
typedef enum CmiPsGetAllCidsInfoCmd_Enum
{
    CMI_PS_DEFAULT_GET_CIDS_CMD = 0,

    /*
     * AT+CGACT?
     *  returns the current activation states for all the defined PDP contexts
    */
    CMI_PS_CGACT_GET_CIDS_CMD,
    CMI_PS_GET_ACTED_BEARERS_CMD = CMI_PS_CGACT_GET_CIDS_CMD,

    /*
     * AT+CGCONTRDP=?
     *   returns a list of <cid>s associated with active non secondary contexts
    */
    CMI_PS_CGCONTRDP_GET_CIDS_CMD,
    CMI_PS_GET_ACTED_DEFAULT_BEARERS_CMD = CMI_PS_CGCONTRDP_GET_CIDS_CMD,

    /*
     * AT+CGSCONTRDP=?
     *  returns a list of <cid>s associated with active secondary PDP contexts
    */
    CMI_PS_CGSCONTRDP_GET_CIDS_CMD,
    CMI_PS_GET_ACTED_DEDICATED_BEARERS_CMD = CMI_PS_CGSCONTRDP_GET_CIDS_CMD,

    /*
     * AT+CGTFTRDP=?
     *  returns a list of <cid>s associated with active secondary and non secondary contexts
    */
    CMI_PS_CGTFTRDP_GET_CIDS_CMD,

    /*
     * AT+CGEQOSRDP=?
     *  returns a list of <cid>s associated with secondary or non secondary active PDP contexts
    */
    CMI_PS_CGEQOSRDP_GET_CIDS_CMD,

    /*
     * AT+CGCMOD=?
     *  returns a list of <cid>s associated with active contexts
    */
    CMI_PS_CGCMOD_GET_CIDS_CMD,

    /*
     * AT+CGPADDR=?
     *  returns a list of defined <cid>s
    */
    CMI_PS_CGPADDR_GET_CIDS_REQ,
    CMI_PS_GET_DEFINED_BEARERS_CMD = CMI_PS_CGPADDR_GET_CIDS_REQ,

    /*
     * AT+CGAPNRC=?
     *  returns a list of <cid>s associated with secondary and non secondary active PDP contexts
    */
    CMI_PS_CGAPNRC_GET_CIDS_CMD,          /* CGAPNRC */

    /*
     * AT+CGVCID?
     *  return vacant (not used) CID list
    */
    CMI_PS_CGVCID_GET_CIDS_CMD,

    CMI_PS_GET_CIDS_NUM_OPERS
}CmiPsGetAllCidsInfoCmd;

typedef struct CmiPsGetAllBearersCidsInfoReqTag
{
    UINT8       getCidsCmd; //CmiPsGetAllCidsInfoCmd
    UINT8       reserved;
    UINT16      reserved2;
}CmiPsGetAllBearersCidsInfoReq;

typedef struct CmiPsBearerCidBasicInfoTag
{
    UINT8       cid;        //0-15
    UINT8       epsId;      //0, 5-15, if not activated, set to 0
    UINT8       bearerState;    //CmiPsBearerState
    UINT8       bearerType;     //CmiPsBearerType
}CmiPsBearerCidBasicInfo;

/*
 * CMI_PS_GET_ALL_BEARERS_CIDS_INFO_CNF
*/
typedef struct CmiPsGetAllBearersCidsInfoCnfTag
{
     UINT8          getCidsCmd; //CmiPsGetAllCidsInfoCmd
     UINT8          validNum;
     UINT16         reserved0;
     CmiPsBearerCidBasicInfo    basicInfoList[CMI_PS_CID_NUM];  //4*16 = 64
}CmiPsGetAllBearersCidsInfoCnf; //68 bytes


/*
 * CMI_PS_GET_BEARER_IPADDR_REQ,//AT+CGPADDR=<cid>
 * AT+CGPADDR=[=<cid>[,<cid>[,...]]]
 *  The execution command returns a list of PDP addresses for the specified context identifiers. If no <cid> is specified,
 *  the addresses for all defined contexts are returned.
*/
typedef struct CmiPsGetBearerIpAddrReqTag
{
    /*
     * if no <cid> is specified, should return all bearers IP address
    */
    BOOL            bAllCid;
    UINT8           cid;
    UINT16          reserved0;
}CmiPsGetBearerIpAddrReq;

/*
 * CMI_PS_GET_BEARER_IPADDR_CNF,
*/
typedef struct CmiPsGetBearerIpAddrCnfTag
{
    BOOL            bContinue;  //whether another bearer IP address return followed
    UINT8           cid;
    UINT16          reserved0;
    CmiIpAddr       ipv4Addr;
    CmiIpAddr       ipv6Addr;
}CmiPsGetBearerIpAddrCnf;


/*
 * CMI_PS_GET_ATTACH_STATE_REQ,
*/
typedef CamCmiEmptySig  CmiPsGetAttachStateReq;

typedef enum CmiPsAttachStateEnum_Tag
{
    CMI_PS_DETACHED = 0,    //Not attached
    CMI_PS_ATTACHED        //PS normal attached
}CmiPsAttachStateEnum;

/*
 * CMI_PS_GET_ATTACH_STATE_CNF,
*/
typedef struct CmiPsGetAttachStateCnfTag
{
    UINT8           state;      //CmiPsAttachStateEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsGetAttachStateCnf;

/*
 * CMI_PS_SET_ATTACH_STATE_REQ,
*/
typedef struct CmiPsSetAttachStateReqTag
{
    UINT8           state;      //CmiPsAttachStateEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsSetAttachStateReq;

/*
 * CMI_PS_SET_ATTACH_STATE_CNF,
*/
typedef CamCmiEmptySig CmiPsSetAttachStateCnf;

/*For Emergency Attach Info*/
typedef enum CmiPsEmergencyAttachStateEnum_Tag
{
    CMI_PS_EMC_DETACHED = 0,        //Emergency detached
    CMI_PS_EMC_ATTACHED,            //Emergency attached
    CMI_PS_NORMAL_ATTACHED          //PS normal attached
}CmiPsEmcAttachStateEnum;

/*
 * CMI_PS_GET_EMC_ATTACH_STATE_REQ,
*/
typedef CamCmiEmptySig  CmiPsGetEmcAttachStateReq;

/*
 * CMI_PS_GET_EMC_ATTACH_STATE_CNF,
*/
typedef struct CmiPsGetEmcAttachStateCnfTag
{
    UINT8           state;      //CmiPsEmcAttachStateEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsGetEmcAttachStateCnf;

/*
 * CMI_PS_SET_EMC_ATTACH_STATE_REQ,
*/
typedef struct CmiPsSetEmcAttachStateReqTag
{
    UINT8           state;      //CmiPsEmcAttachStateEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsSetEmcAttachStateReq;


/*
 * CMI_PS_SET_EMC_ATTACH_STATE_CNF,
*/
typedef CamCmiEmptySig CmiPsSetEmcAttachStateCnf;

/*
 * CMI_PS_ENTER_DATA_STATE_REQ,//AT+CGDATA
*/
typedef struct CmiPsEnterDataStateReqTag
{
    UINT8           cid;//cid to be actived;
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsEnterDataStateReq;

/*
 *CMI_PS_ENTER_DATA_STATE_CNF,
*/
typedef CamCmiEmptySig CmiPsEnterDataStateCnf;


/*
 * CMI_PS_NO_MORE_PS_DATA_REQ, //AT+CNMPSD
*/
typedef CamCmiEmptySig CmiPsNoMorePsDataReq;

/*
 * CMI_PS_NO_MORE_PS_DATA_CNF, //AT+CNMPSD
*/
typedef CamCmiEmptySig CmiPsNoMorePsDataCnf;


/* ===== struct of CEREG ===== */
typedef enum CmiMmCeregModeEnum_TAG
{
    CMI_PS_DISABLE_CEREG = 0,
    CMI_PS_ENABLE_CEREG = 1,
    CMI_PS_CEREG_LOC_INFO = 2,
    CMI_PS_CEREG_LOC_EMM_CAUSE = 3,
    CMI_PS_CEREG_LOC_PSM_INFO = 4,
    CMI_PS_CEREG_LOC_PSM_INFO_EMM_CAUSE = 5
}CmiPsCeregModeEnum;

typedef enum _EPAT_CmiCeregStateEnum_Tag
{
    CMI_PS_NOT_REG = 0,
    CMI_PS_REG_HOME = 1,
    CMI_PS_NOT_REG_SEARCHING = 2,
    CMI_PS_REG_DENIED = 3,
    CMI_PS_REG_UNKNOWN = 4,
    CMI_PS_REG_ROAMING = 5,
    CMI_PS_REG_SMS_ONLY_HOME = 6,       // not applicable
    CMI_PS_REG_SMS_ONLY_ROAMING = 7,    // not applicable
    CMI_PS_REG_EMERGENCY = 8,           /* attached for emergency bearer services only  */
    CMI_PS_REG_CSFB_NOT_PREFER_HOME = 9,        // not applicable
    CMI_PS_REG_CSFB_NOT_PREFER_ROAMING = 10,    //not applicable

    CMI_PS_REG_EMERGENCY_CAMP   = 128   /* Emergency camp on a cell, but not emergency attached
                                         * 1> this is a internal CEREG state, if user/app not expected it, act it to 0 (CMI_PS_NOT_REG)
                                        */

}CmiCeregStateEnum;

typedef enum CmiCeregActEnum_Tag
{
    CMI_PS_GSM = 0,
    CMI_PS_GSM_COMPACT = 1,
    CMI_PS_UMTS = 2,
    CMI_PS_GSM_EGPRS = 3,
    CMI_PS_HSDPA = 4,
    CMI_PS_HSUPA = 5,
    CMI_PS_HSDPA_HSUPA = 6,
    CMI_PS_LTE = 7,
    CMI_PS_EC_GSM = 8,
    CMI_PS_NB_IOT = 9 //NB only
}CmiCeregActEnum;


/*
 * CMI_PS_GET_CEREG_REQ, //AT+CEREG?
*/
typedef CamCmiEmptySig CmiPsGetCeregReq;

/*
 * CMI_PS_GET_CEREG_CNF,
*/
typedef struct CmiPsGetCeregCnf_Tag
{
    //UINT8   ceregMode;    //CmiPsCeregModeEnum, this "ceregMode" saved in ATCMDSRC side
    UINT8   state;          //CmiCeregStateEnum
    BOOL    bRegOngoing;    //if not registered, whether low layer is trying to search/register to a PLMN
    BOOL    isTdd;
    UINT8   rsvd0;

    BOOL    locPresent;
    UINT8   act;            //CmiCeregActEnum
    UINT16  tac;
    UINT32  celId;

    CmiNumericPlmn  plmn;

    BOOL    rejCausePresent;
    UINT8   causeType;      // only support 0 by now
    UINT16  rejCause;

    BOOL    activeTimePresent;
    UINT8   activeTime;     //T3324, valid when "activeTimePresent" == TRUE

    BOOL    extTauTimePresent;
    UINT8   extPeriodicTau; //extT3412, valid when "extTauTimePresent" == TRUE

    /*
     * Active Time value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  activeTimeS;
    /*
     * extended periodic TAU value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  extPeriodicTauS;

    /*
     * following NW capability also report in: CmiMmNwImsVopsEmcSuppInd, but some user also want it in CEREG
    */
    UINT8   imsVopsSupp;    /* EPS network feature: IMS voice over PS indicator, 0 - not supported, 1 - supported */
    UINT8   embS1Supp;      /* EPS network feature: Emergency bearer indicator, 0 - not supported, 1 - supported */
    UINT8   epsSibImsEmcSupp;   /* EPS SIB1->ims-EmergencySupport-r9, 0 - not support, 1 - support */

    /*
     * 1> if NW not config the extT3412(extPeriodicTau), should return this T3412 (periodicTauS) value in seconds;
     * 2> else (if NW config extT3412), don't need to return this T3412 (periodicTauS)
    */
    BOOL    tauTimerPresent;

    UINT32  periodicTauS;
}CmiPsGetCeregCnf;

/*
 * CMI_PS_GET_CEREG_CAP_REQ, //AT+CEREG=?
*/
typedef CamCmiEmptySig CmiPsGetCeregCapReq;

/*
 * CMI_PS_GET_CEREG_CAP_CNF,
*/
typedef struct CmiPsGetCeregCapCnf_Tag
{
    UINT8     bBitMap; // bit 0 set to 1, means suppurt n = 0;
    UINT8     reserved1;
    UINT16    reserved2;
}CmiPsGetCeregCapCnf;


/******************************************************************************
 * Note:
 * Change level should: CMI_CEREG_STATE_CHANGED > CMI_CEREG_LOC_INFO_CHANGED_WITH_TAU > CMI_CEREG_LOC_INFO_CHANGED >
 *                       CMI_CEREG_REJECT_INFO_CHANGED > CMI_CEREG_PSM_INFO_CHANGED
 * Just means:
 * 1> If "CMI_CEREG_STATE_CHANGED" is set, just means registation state changed,
 *     can't set to low level: "CMI_CEREG_LOC_INFO_CHANGED/CMI_CEREG_REJECT_INFO_CHANGED/CMI_CEREG_PSM_INFO_CHANGED"
 * 2> If "CMI_CEREG_LOC_INFO_CHANGED" is set, just means registation state not changed, but location info changed,
 *     can't set to low level: "CMI_CEREG_REJECT_INFO_CHANGED/CMI_CEREG_PSM_INFO_CHANGED"
******************************************************************************/
typedef enum CmiCeregChangedTypeEnum_Tag
{
    CMI_CEREG_NONE_CHANGED = 0,
    CMI_CEREG_STATE_CHANGED,
    CMI_CEREG_LOC_INFO_CHANGED_WITH_TAU,    //location info changes, and TAU triggered
    CMI_CEREG_LOC_INFO_CHANGED,             //location info changes, but no TAU triggered
    CMI_CEREG_REJECT_INFO_CHANGED,
    CMI_CEREG_PSM_INFO_CHANGED
}CmiCeregChangedTypeEnum;

/*
 * CMI_PS_CEREG_IND, //+CEREG xxx
*/
typedef struct CmiPsCeregInd_Tag
{
    UINT8   state;          //CmiCeregStateEnum
    UINT8   changedType;    //CmiCeregChangedTypeEnum, which type of info are changed
    BOOL    bRegOngoing;    //if not registered, whether UE is trying to register to a PLMN later, maybe after OOS timer expiried
    BOOL    isTdd;

    BOOL    locPresent;
    UINT8   act;            //CmiCeregActEnum
    UINT16  tac;

    UINT32  cellId;
    CmiNumericPlmn  plmn;

    BOOL    rejCausePresent;
    UINT8   causeType;      // only support 0 by now
    UINT16  rejCause;

    BOOL    activeTimePresent;
    UINT8   activeTime;     //T3324, valid when "activeTimePresent" == TRUE

    BOOL    extTauTimePresent;
    UINT8   extPeriodicTau; //extT3412, valid when "extTauTimePresent" == TRUE

    /*
     * Active Time value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  activeTimeS;

    /*
     * extended periodic TAU value in seconds, valid when "extTauTimePresent" == TRUE
    */
    UINT32  extPeriodicTauS;

    /*
     * following NW capability also report in: CmiMmNwImsVopsEmcSuppInd, but some user also want it in CEREG
    */
    UINT8   imsVopsSupp;    /* EPS network feature: IMS voice over PS indicator, 0 - not supported, 1 - supported */
    UINT8   embS1Supp;      /* EPS network feature: Emergency bearer indicator, 0 - not supported, 1 - supported */
    UINT8   epsSibImsEmcSupp;   /* EPS SIB1->ims-EmergencySupport-r9, 0 - not support, 1 - support */
    /*
     * 1> if NW not config the extT3412(extPeriodicTau), should return this T3412 (periodicTauS) value in seconds;
     * 2> else (if NW config extT3412), don't need to return this T3412 (periodicTauS)
    */
    BOOL    tauTimerPresent;

    UINT32  periodicTauS;

    BOOL    bHandover;
    BOOL    bReselection;
    UINT16  rsvd;
}CmiPsCeregInd;     //44 bytes


/*
 * CMI_PS_SET_ATTACH_WITH_OR_WITHOUT_PDN_REQ, //AT+CIPCA
*/
typedef struct CmiPsSetAttachWithOrWithoutPdnReq_Tag
{
    UINT8       nflag;
    UINT8       attachedWithoutPdn; // value 0-1;
    UINT16      reserved1;
}CmiPsSetAttachWithOrWithoutPdnReq;

/*
 * CMI_PS_SET_ATTACH_WITH_OR_WITHOUT_PDN_CNF,
*/
typedef CamCmiEmptySig CmiPsSetAttachWithOrWithoutPdnCnf;

/*
 * CMI_PS_GET_ATTACH_WITH_OR_WITHOUT_PDN_REQ, //AT+CIPCA?
*/
typedef CamCmiEmptySig CmiPsGetAttachWithOrWithoutPdnReq;

/*
 * CMI_PS_GET_ATTACH_WITH_OR_WITHOUT_PDN_CNF,
*/
typedef struct CmiPsGetAttachWithOrWithoutPdnCnf_Tag
{
    UINT8       nflag;
    UINT8       attachedWithoutPdn; // value 0-1;
    UINT16      reserved1;
}CmiPsGetAttachWithOrWithoutPdnCnf;

/*
 * CMI_PS_GET_APN_RATE_CTRL_PARM_REQ, //
 * AT+CGAPNRC=<cid>
 *  +CGAPNRC: <cid>[,<Additional_exception_reports>[,<Uplink_time_unit>[,<Maximum_uplink_rate]]
 * If the parameter <cid> is omitted, the APN rate control parameters for all active PDP contexts are returned
*/
typedef struct CmiPsGetAPNRateCtrlReq_Tag
{
    BOOL        bAllCid;    //whether request all actived default bearer info
    UINT8       cid;        //if "bAllCid" == FALSE, get specified default bearer info;
    UINT16      reserved0;
}CmiPsGetAPNRateCtrlReq;

typedef enum CmiPsApnRateCtrlUlTimeUnit_Enum
{
    CMI_PS_APN_RATE_UNRESTRICTED = 0,
    CMI_PS_APN_RATE_MINUTE = 1,
    CMI_PS_APN_RATE_HOUR   = 2,
    CMI_PS_APN_RATE_DAY    = 3,
    CMI_PS_APN_RATE_WEEK   = 4
}CmiPsApnRateCtrlUlTimeUnit;

typedef struct CmiPsApnRateCtrlParam_Tag
{
    UINT8       aer;        //Additional exception reports
    UINT8       ulTimeUnit; //CmiPsApnRateCtrlUlTimeUnit
    UINT16      reserved0;
    UINT32      maxUlRate;
}CmiPsApnRateCtrlParam;

/*
 * CMI_PS_GET_APN_RATE_CTRL_PARM_CNF,
*/
typedef struct CmiPsGetAPNRateCtrlCnf_Tag
{
    /*
     * Note:
     * if get all bearer APNRC info, and if a bearer is activated, but no APNRC info configed, then:
     *  "cid" should be set to right value, and "apnRateValid" set to FALSE
    */
    BOOL        bContinue;
    BOOL        apnRateValid;
    UINT8       cid;        /*if not valid, set to 0*/
    UINT8       reserved0;
    CmiPsApnRateCtrlParam   apnRateParam;
}CmiPsGetAPNRateCtrlCnf;


typedef struct CmiPsSetDefineTFTParamReq_Tag
{
    CmiPsPacketFilter filter;
}CmiPsSetDefineTFTParamReq;

/*
 * CMI_PS_DEFINE_TFT_PARM_CNF,
*/
typedef CamCmiEmptySig CmiPsSetDefineTFTParamCnf;

/*
 * CMI_PS_DELETE_TFT_PARM_REQ,
*/
typedef struct CmiPsDelTFTParamReq_Tag
{
    UINT8       cid;
    UINT8       rsvd0;
    UINT16      rsvd1;
}CmiPsDelTFTParamReq;

/*
 * CMI_PS_DELETE_TFT_PARM_CNF,
*/
typedef CamCmiEmptySig CmiPsdelTFTParamCnf;

/*
 * CMI_PS_GET_DEFINED_TFT_PARM_REQ, //AT+CGTFT?
*/
typedef struct CmiPsGetDefineTFTParamReq_Tag
{
    BOOL    bAllCid;
    UINT8   cid;
    UINT16  reserved0;
}CmiPsGetDefineTFTParamReq;

/*
 * CMI_PS_GET_DEFINED_TFT_PARM_CNF,
*/
typedef struct  CmiPsGetDefineTFTParamCnfTag
{
    BOOL            epsTftValid;
    BOOL            bContinue;   //whether next bearer TFT info followed
    UINT8           numFilters;
    UINT8           reserved0;
    CmiPsPacketFilter  filters[CMI_PS_MAX_TFT_FILTERS];    //52*16 = 832 bytes
}CmiPsGetDefineTFTParamCnf;

/*
 * CMI_PS_SET_ATTACHED_BEARER_CTX_REQ
 * AT+ECATTBEARER=<PDP_type>[,<eitf>[,<apn>[,<IPv4AddrAlloc>[,<NSLPI>[,<IPv4_MTU_discovery>[,<NonIP_MTU_discovery>
 *            [,<PCSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>]]]]]]]]
 * +CGDCONT=0,<PDP_type>[,<APN>[,<PDP_addr>[,<d_comp>[,<h_comp>[,<IPv4AddrAlloc>
 *           [,<request_type>[,<PCSCF_discovery>[,<IM_CN_Signalling_Flag_Ind>[,<NSLPI>[,<securePCO>
 *           [,<IPv4_MTU_discovery>][,<Local_Addr_Ind>][,<NonIP_MTU_discovery>]]]]]]]]]]]
*/
typedef struct AttachedBearCtxPreSetParam_Tag
{
    UINT8   pdnType;        //CmiPsPdnType
    BOOL    eitfPresent;    //ESM information transfer flag
    UINT8   eitf;           //0 - 1; 0 - security protected ESM information transfer not required, 1 - security protected ESM information transfer required

    CmiApnPresentType   apnPresentType; /*record this input apnstring type : NULL or ""  or a valid apn string */
    UINT8   apnLength;      /* 0 - apn is not present */
    UINT8   apnStr[CMI_PS_MAX_APN_LEN];//apn string

    BOOL    ipv4AlloTypePresent;
    UINT8   ipv4AlloType;   /* CmiPsIpv4AllocType. IPv4 address allocation through NAS signalling, or by DHCP */

    BOOL    NSLPIPresent;
    UINT8   NSLPI;          /*CmiPsNSLPICfg, NAS signalling priority*/

    BOOL    ipv4MtuDisTypePresent;
    BOOL    ipv4MtuDisByNas;    /* IPv4 MTU size discovery through NAS signalling */

    BOOL    authProPresent;
    UINT8   authProtocol;    /*CmiSecurityProtocol */
    UINT8   authUserNameLength;
    UINT8   authUserName[CMI_PS_MAX_AUTH_STR_LEN];//auth username string
    UINT8   authPasswordLength;
    UINT8   authPassword[CMI_PS_MAX_AUTH_STR_LEN];//auth password string

    BOOL    rdsPresent;
    BOOL    rds;    //indicates whether the UE is using Reliable Data Service for a PDN connection or not,

    BOOL    apnCtrlPresent;
    BOOL    apnCtrl;    //indicates whether the UE is using Reliable Data Service for a PDN connection or not,

    BOOL    addApnCtrlPresent;
    BOOL    addApnCtrl;
   /*
     * IMS/pcscf not support now
    */
    BOOL    pcscfDisTypePresent;
    UINT8   pcscfDisType;   //CmiPsPcscfDiscoveryType

    BOOL    imCnSigFlagPresent;
    UINT8   imCnSigFlag;    //CmiPsImCnSigFlag
    UINT8   reserved1;
    UINT16  reserved2;
}AttachedBearCtxPreSetParam;

typedef struct CmiPsSetAttachedBearerCtxReq_Tag
{
    AttachedBearCtxPreSetParam     attBearCtxPreset;
}CmiPsSetAttachedBearerCtxReq;

/*
 * CMI_PS_SET_ATTACHED_BEARER_CTX_CNF
*/
typedef CamCmiEmptySig CmiPsSetAttachedBearerCtxCnf;

/*
 * CMI_PS_GET_ATTACHED_BEARER_CTX_REQ
 * AT+ECATTBEARER?
*/
typedef CamCmiEmptySig CmiPsGetAttachedBearerCtxReq;

/*
 * CMI_PS_GET_ATTACHED_BEARER_CTX_CNF
 *  +ECATTBEARER=<PDP_type>,<eitf>,<apn>,<IPv4AddrAlloc>,<NSLPI>,<IPv4_MTU_discovery>,<NonIP_MTU_discovery>
 *               ,<PCSCF_discovery>,<IM_CN_Signalling_Flag_Ind>
*/
typedef struct AttachedBearCtxParam_Tag
{
    UINT8   pdnType;        //CmiPsPdnType
    UINT8   eitf;           //0 - 1; 0 - security protected ESM information transfer not required, 1 - security protected ESM information transfer required
    UINT8   apnLength;      /* 0 - apn is not present */
    UINT8   apnStr[CMI_PS_MAX_APN_LEN];//apn string

    UINT8   ipv4AlloType;   /* CmiPsIpv4AllocType. IPv4 address allocation through NAS signalling, or by DHCP */
    UINT8   NSLPI;          /* CmiPsNSLPICfg, NAS signalling priority*/

    BOOL    ipv4MtuDisByNas;    /* IPv4 MTU size discovery through NAS signalling */

    UINT8   authProtocol;    /*CmiSecurityProtocol */
    UINT8   authUserNameLength;
    UINT8   authUserName[CMI_PS_MAX_AUTH_STR_LEN];//auth username string
    UINT8   authPasswordLength;
    UINT8   authPassword[CMI_PS_MAX_AUTH_STR_LEN];//auth password string

    BOOL    rds; /*rds support*/
    /*
     * IMS/pcscf not support now
    */
    UINT8   pcscfDisType;   //CmiPsPcscfDiscoveryType
    UINT8   imCnSigFlag;    //CmiPsImCnSigFlag
    UINT16  reserved1;

}AttachedBearCtxParam;

typedef struct CmiPsGetAttachedBearerCtxCnf_Tag
{
    AttachedBearCtxParam  attBearCtxParam;
}CmiPsGetAttachedBearerCtxCnf;

/*
 * RAI enum
*/
typedef enum CmiPsRelAssistIndEnum_Tag
{
    CMI_PS_RAI_NO_INFO = 0,
    CMI_PS_RAI_NO_UL_DL_FOLLOWED = 1,
    CMI_PS_RAI_ONLY_DL_FOLLOWED = 2,
    CMI_PS_RAI_RESERVED = 3
}CmiPsRelAssistIndEnum;

/*
 * type_of_user_data
*/
typedef enum CmiPsUserDataType_enum
{
    CMI_PS_REGULAR_DATA = 0,
    CMI_PS_EXCEPT_DATA = 1
}CmiPsUserDataType;

/*
 * CMI_PS_SEND_CP_DATA_REQ
 * +CSODCP=<cid>,<cpdata_length>,<cpdata>[,<RAI>[,<type_of_user_data>]
*/

typedef struct CmiPsSendOriDataViaCpReqTag
{
    UINT8   cid;
    UINT8   reserved0;
    UINT16  dataLen; //max len is 1600;
    UINT8   *cpData;  //need allocate firstly when called by AT

    UINT8   rai;     //CmiPsRelAssistIndEnum, indicates the value of the release assistance indication, if not set, default: CMI_PS_RAI_NO_INFO
    UINT8   typeOfUserData; //CmiPsUserDataType, indicates whether the user data that is transmitted is regular or exceptional.
    UINT16  reserved1;
}CmiPsSendOriDataViaCpReq;

/*
 * CMI_PS_SEND_CP_DATA_CNF
*/
typedef CamCmiEmptySig CmiPsSendOriDataViaCpCnf;

/*
 * CMI_PS_SET_MT_CP_DATA_REPORT_CFG_REQ
 * +CRTDCP=[<reporting>]
*/
typedef struct CmiPsSetCpDataReportCfgReq_Tag
{
    UINT8     reporting;    /* 0 - Disable reporting of MT control plane data, 1 - enable */
    UINT8     reserved1;
    UINT16    reserved2;
}CmiPsSetCpDataReportCfgReq;

/*
 * CMI_PS_SET_MT_CP_DATA_REPORT_CFG_CNF
*/
typedef CamCmiEmptySig CmiPsSetCpDataReportCfgCnf;

/*
 * CMI_PS_GET_MT_CP_DATA_REPORT_CFG_REQ
 * +CRTDCP?
*/
typedef CamCmiEmptySig CmiPsGetCpDataReportCfgReq;
/*
 * CMI_PS_GET_MT_CP_DATA_REPORT_CFG_CNF
*/
typedef struct CmiPsGetCpDataReportCfgCnf_Tag
{
    UINT8   reporting;      /* 0 - disable, 1 - enable */
    UINT8   reserved1;
    UINT16  srcHandler;     /* which SRC handler config the "CRTDCP" if not 0  */
}CmiPsGetCpDataReportCfgCnf;

/*
 * CMI_PS_MT_CP_DATA_IND, //+CRTDCP:<cid>,<cpdata_length>,<cpdata>
*/
typedef struct CmiPsMtCpDataInd_Tag
{
    UINT8   cid;
    UINT8   rsvd0;
    UINT16  rsvd1;

    UINT16  srcHandler;     /* which SRC handler need MT CP data */
    UINT16  length;

    UINT8   *cpdata;        /* memory alloc in heap */
}CmiPsMtCpDataInd;


/*
 * CMI_PS_SEND_UL_DATA_REQ
 *  AT+ECSENDDATA=<cid>,<data_length>,<data>[,<RAI>[,<type_of_user_data>]]
 * Note:
 * 1> User could send non-ip/ip data via this AT CMD;
 * 2> Not the same as: +CSODCP,
 *    AT+ECSENDDATA: could send data via CP or UP
 *    AT+CSODCP:   only send the data via CP
*/
typedef struct CmiPsSendUlDataReq_Tag
{
    UINT8   cid;
    UINT8   reserved0;
    UINT16  dataLen; //max len is 1600;
    UINT8   *pData;  //should use dynamic memory

    UINT8   rai;     //CmiPsRelAssistIndEnum, indicates the value of the release assistance indication, if not set, default: CMI_PS_RAI_NO_INFO
    UINT8   typeOfUserData; //CmiPsUserDataType, indicates whether the user data that is transmitted is regular or exceptional.
    UINT16  reserved1;
}CmiPsSendUlDataReq;

/*
 * CMI_PS_SEND_UL_DATA_CNF
 * AT+ECSENDDATA
*/
typedef CamCmiEmptySig CmiPsSendUlDataCnf;


/*
 * CMI_PS_RECV_DL_NON_IP_DATA_IND
 *  +RECVNONIP: <cid>,<data_length>,<data>
 *  For NON-IP data, just foward to AT CMD SRV UART
*/
typedef struct CmiPsRecvDlNonIpDataInd_Tag
{
    UINT8   cid;
    UINT8   reserved0;
    UINT16  length;
    UINT8   *pData;    // using dynamic memory
}CmiPsRecvDlNonIpDataInd;


/*
 * CMI_PS_TRANS_CIOT_PLANE_REQ,
 *  AT+ECCIOTPLANE=plane, to select CP or UP
*/
typedef enum CmiPsDataPlane_Enum
{
    CMI_PS_CTRL_PLANE,
    CMI_PS_USER_PLANE,
    CMI_PS_UNKNOWN_PLANE
}CmiPsDataPlane;

typedef struct CmiPsTransCiotPlaneReq_Tag
{
    UINT8   plane;      //CmiPsDataPlane
    UINT8   reserved0;
    UINT16  reserved1;
}CmiPsTransCiotPlaneReq;

/*
 * CMI_PS_TRANS_CIOT_PLANE_CNF
*/
typedef CamCmiEmptySig CmiPsTransCiotPlaneCnf;


/******************************************************************************
 * CMI_PS_GET_CONN_STATUS_REQ
 * CMI_PS_GET_CONN_STATUS_CNF
 * AT+CSCON?
 *  +CSCON: <n>,<mode>[,<state>]    //<state> not support now
******************************************************************************/
typedef CamCmiEmptySig  CmiPsGetConnStatusReq;

typedef enum CmiPsConnStatusModeEnum_Tag
{
    CMI_PS_IDLE_STATUS = 0,
    CMI_PS_CONNECTED_STATUS
}CmiPsConnStatusModeEnum;

typedef struct CmiPsGetConnStatusCnf_Tag
{
    UINT8   connMode;   //CmiPsConnStatusModeEnum
    UINT8   reserved0;
    UINT16  reserved1;
}CmiPsGetConnStatusCnf;

/*
 * CMI_PS_CONN_STATUS_IND
 *  +CSCON: <mode>[,<state>[,<access>]] // we only support: <mode>
*/
typedef struct CmiPsConnStatusInd_Tag
{
    UINT8   connMode;   //CmiPsConnStatusModeEnum
    UINT8   reserved0;
    UINT16  reserved1;
}CmiPsConnStatusInd;



/******************************************************************************
 * CMI_PS_DEFINE_AUTH_CTX_REQ
 * CMI_PS_DEFINE_AUTH_CTX_CNF
 * AT+CGAUTH=cid,auth_prot,userid,password
 *  OK

 * CMI_PS_GET_DEFINED_AUTH_CTX_REQ
 * CMI_PS_GET_DEFINED_AUTH_CTX_CNF
 * AT+CGAUTH?
 *  +CGAUTH: cid,auth_prot,<userid,password>
******************************************************************************/
typedef struct CmiPsSetAuthInfo_Tag
{
    UINT8   cid;

    UINT8   delAuthInfo     : 1;    /* whether delete AUTH info,
                                     * AT: AT+CGAUTH=<cid>, causes the authentication parameters for context number
                                     *     <cid> to become undefined */
    UINT8   authProtPresent : 1;    /*whether set the authProtocol by configure parameter */
    UINT8   authUserPresent : 1;    /*whether set the authUserPresent by configure parameter */
    UINT8   authPswdPresent : 1;    /*whether set the authPswdPresent by configure parameter */
    UINT8   authProtocol    : 4;    /*CmiSecurityProtocol*/

    UINT8   authUserNameLength;
    UINT8   authPasswordLength;

    /*
     * 1> If CMI_SECURITY_PROTOCOL_PPP_LCP_CHAP type,
     *    a) "authUserName" is the CHAP challenge which sent from PPP server (UE) to PPP client (host),
     *    b) "authPassword" is the CHAP response which sent from PPP client (host) to PPP server (UE),
     *  Format (RFC 1994, 4.1):
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  |     Code      |  Identifier   |            Length             |
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  |  Value-Size   |  Value ...
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  |  Name ...
     *  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
     *  Code:  1 - Challenge
     *         2 - Response
     *  Length: length of:  4 bytes (code + ID + length) + length of "Value-size + Value + name"
    */
    UINT8   authUserName[CMI_PS_MAX_AUTH_STR_LEN];  //auth username string
    UINT8   authPassword[CMI_PS_MAX_AUTH_STR_LEN];  //auth password string
}CmiPsSetAuthInfo;  //132 bytes

typedef struct CmiPsSetDefineAuthCtxReq_Tag
{
    CmiPsSetAuthInfo    cmiAuthInfo;    //132 bytes
}CmiPsSetDefineAuthCtxReq;

/*
 * CMI_PS_DEFINE_AUTH_CTX_CNF
*/
typedef CamCmiEmptySig CmiPsSetDefineAuthCtxCnf;

/*
 * CMI_PS_GET_DEFINED_AUTH_CTX_REQ
*/
//typedef CamCmiEmptySig CmiPsGetDefineAuthCtxReq;
typedef struct CmiPsGetDefineAuthCtxReq_Tag
{
    BOOL        bAllCid;    //whether request all PDP AUTH defination
    UINT8       cid;        //if "bAllCid" == FALSE, get specified PDP
    UINT16      reserved0;
}CmiPsGetDefineAuthCtxReq;

/*
 * CMI_PS_GET_DEFINED_AUTH_CTX_CNF
 * if request all BR AUTH info, and no BR found, then: CMICNF set to: CME_PDN_NOT_VALID, and CID set to 0xFF
*/
typedef struct CmiPsGetDefineAuthCtxCnf_Tag
{
    BOOL    bContinue;          //whether another bearer IP address return followed
    UINT8   cid;
    UINT8   authProtocol;       /* CmiSecurityProtocol, CMI_SECURITY_PROTOCOL_PPP_LCP_PAP/CMI_SECURITY_PROTOCOL_PPP_LCP_CHAP not return */
    UINT8   rsvd0;

    UINT8   authUserName[CMI_PS_MAX_AUTH_STR_LEN +1];//auth username string
    UINT8   authPassword[CMI_PS_MAX_AUTH_STR_LEN +1];//auth password string
}CmiPsGetDefineAuthCtxCnf;

#if 0
typedef enum CmiEmmCauseTag
{
    CMI_EMM_CAUSE_IMSI_UNKNOWN_IN_HSS                                  = 0x02,
    CMI_EMM_CAUSE_ILLEGAL_UE                                           = 0x03,
    CMI_EMM_CAUSE_IMEI_NOT_ACCEPTED                                    = 0x05,
    CMI_EMM_CAUSE_ILLEGAL_ME                                           = 0x06,
    CMI_EMM_CAUSE_EPS_SERVICES_NOT_ALLOWED                             = 0x07,
    CMI_EMM_CAUSE_EPS_AND_NON_EPS_SERVICES_NOT_ALLOWED                 = 0x08,
    CMI_EMM_CAUSE_UE_ID_CAN_NOT_BE_DERIVED_IN_NETWORK                  = 0x09,
    CMI_EMM_CAUSE_IMPLICITLY_DETACHED                                  = 0x0A,
    CMI_EMM_CAUSE_PLMN_NOT_ALLOWED                                     = 0X0B,
    CMI_EMM_CAUSE_TRACKING_AREA_NOT_ALLOWED                            = 0x0C,
    CMI_EMM_CAUSE_ROAMING_NOT_ALLOWED_IN_THIS_TRACKING_AREA            = 0X0D,
    CMI_EMM_CAUSE_EPS_SERVICE_NOT_ALLOWED_IN_THIS_PLMN                 = 0x0E,
    CMI_EMM_CAUSE_NO_SUITABLE_CELLS_IN_TRACKING_AREA                   = 0x0F,
    CMI_EMM_CAUSE_MSC_TEMPORARILY_NOT_REACHABLE                        = 0x10,
    CMI_EMM_CAUSE_NETWORK_FAILURE                                      = 0x11,
    CMI_EMM_CAUSE_CS_DOMAIN_NOT_AVAILABLE                              = 0x12,
    CMI_EMM_CAUSE_ESM_FAILURE                                          = 0x13,
    CMI_EMM_CAUSE_MAC_FAILURE                                          = 0X14,
    CMI_EMM_CAUSE_SYNCH_FAILURE                                        = 0X15,
    CMI_EMM_CAUSE_CONGESTION                                           = 0X16,
    CMI_EMM_CAUSE_UE_SECURITY_CAPAILITIES_MISMATCH                     = 0x17,
    CMI_EMM_CAUSE_SECURITY_MODE_REJECTED_UNSPECIFIED                   = 0x18,
    CMI_EMM_CAUSE_NOT_AUTHORIZED_FOR_THIS_CSG                          = 0x19,
    CMI_EMM_CAUSE_NON_EPS_AUTHENTICATION_UNACCEPTABLE                  = 0x1A,
    CMI_EMM_CAUSE_REQUESTED_SERVICE_OPTION_NOT_AUTHORIZED_IN_THIS_PLMN = 0X23,
    CMI_EMM_CAUSE_CS_SERVICE_TEMPORARILY_NOT_AVAILABLE                 = 0x27,
    CMI_EMM_CAUSE_NO_EPS_BEARER_CONTEXT_ACTIVATED                      = 0x28,
    CMI_EMM_CAUSE_SERVERE_NETWORK_FAILURE                              = 0x2A,
    CMI_EMM_CAUSE_SYMANTICALLY_INCORRECT_MESSAGE                       = 0X5F,
    CMI_EMM_CAUSE_INVALID_MANDATORY_INFORMATION                        = 0X60,
    CMI_EMM_CAUSE_MESSAGE_TYPE_NON_EXISTENT_OR_NOT_IMPLEMENTED         = 0X61,
    CMI_EMM_CAUSE_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE  = 0X62,
    CMI_EMM_CAUSE_INFORMATION_ELEMENT_NON_EXISTENT_OR_NOT_IMPLEMENTED  = 0X63,
    CMI_EMM_CAUSE_CONDITIONAL_IE_ERROR                                 = 0X64,
    CMI_EMM_CAUSE_MESSAGE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE       = 0X65,
    CMI_EMM_CAUSE_PROTOCOL_ERROR_UNSPECIFIED                           = 0X6F,
}CmiEmmCause;

typedef enum CmiEsmCauseTag
{
    CMI_ESM_CAUSE_SUCC                                                          =  0x00,

    /*
     * ref: 24.301, 9.9.4.4
    */
    CMI_ESM_OPERATOR_DETERMINED_BARRING                                         =  0x08,
    CMI_ESM_INSUFFICIENT_RESOURCES                                              =  0x1a,
    CMI_ESM_UNKNOWN_OR_MISSING_APN                                              =  0x1b,
    CMI_ESM_UNKNOWN_PDN_TYPE                                                    =  0x1c,
    CMI_ESM_USER_AUTHENTICATION_FAILED                                          =  0x1d,
    CMI_ESM_REQUEST_REJECTED_BY_SERVING_GW_OR_PDN_GW                            =  0x1e,
    CMI_ESM_REQUEST_REJECTED_UNSPECIFIED                                        =  0x1f,
    CMI_ESM_SERVICE_OPTION_NOT_SUPPORTED                                        =  0x20,
    CMI_ESM_REQUESTED_SERVICE_OPTION_NOT_SUBSCRIBED                             =  0x21,
    CMI_ESM_SERVICE_OPTION_TEMPORARILY_OUT_OF_ORDER                             =  0x22,
    CMI_ESM_PTI_ALREADY_IN_USE                                                  =  0x23,
    CMI_ESM_REGULAR_DEACTIVATION                                                =  0x24,
    CMI_ESM_EPS_QOS_NOT_ACCEPTED                                                =  0x25,
    CMI_ESM_NETWORK_FAILURE                                                     =  0x26,
    CMI_ESM_REACTIVATION_REQUESTED                                              =  0x27,
    CMI_ESM_SEMANTIC_ERROR_IN_THE_TFT_OPERATION                                 =  0x29,
    CMI_ESM_SYNTACTICAL_ERROR_IN_THE_TFT_OPERATION                              =  0x2a,
    CMI_ESM_INVALID_EPS_BEARER_IDENTITY                                         =  0x2b,
    CMI_ESM_SEMANTIC_ERRORS_IN_PACKET_FILTER                                    =  0x2c,
    CMI_ESM_SYNTACTICAL_ERRORS_IN_PACKET_FILTER                                 =  0x2d,
    CMI_ESM_EPS_BEARER_CONTEXT_WITHOUT_TFT_ALREADY_ACTIVATED                    =  0x2e,
    CMI_ESM_PTI_MISMATCH                                                        =  0x2f,
    CMI_ESM_LAST_PDN_DISCONNECTION_NOT_ALLOWED                                  =  0x31,
    CMI_ESM_PDN_TYPE_IPV4_ONLY_ALLOWED                                          =  0x32,
    CMI_ESM_PDN_TYPE_IPV6_ONLY_ALLOWED                                          =  0x33,
    CMI_ESM_SINGLE_ADDRESS_BEARERS_ONLY_ALLOWED                                 =  0x34,
    CMI_ESM_ESM_INFORMATION_NOT_RECEIVED                                        =  0x35,
    CMI_ESM_PDN_CONNECTION_DOES_NOT_EXIST                                       =  0x36,
    CMI_ESM_MULTIPLE_PDN_CONNECTIONS_FOR_A_GIVEN_APN_NOT_ALLOWED                =  0x37,
    CMI_ESM_COLLISION_WITH_NETWORK_INITIATED_REQUEST                            =  0x38,
    CMI_ESM_PDN_TYPE_IPV4V6_ONLY_ALLOW                                          =  0x39,
    CMI_ESM_PDN_TYPE_NON_IP_ONLY_ALLOW                                          =  0x3a,
    CMI_ESM_UNSUPPORTED_QCI_VALUE                                               =  0x3b,
    CMI_ESM_INVALID_PTI_VALUE                                                   =  0x51,
    CMI_ESM_SEMANTICALLY_INCORRECT_MESSAGE                                      =  0x5f,
    CMI_ESM_INVALID_MANDATORY_INFORMATION                                       =  0x60,
    CMI_ESM_MESSAGE_TYPE_NONEXISTENT_OR_NOT_IMPLEMENTED                         =  0x61,
    CMI_ESM_MESSAGE_TYPE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE                 =  0x62,
    CMI_ESM_INFORMATION_ELEMENT_NONEXISTENT_OR_NOT_IMPLEMENTED                  =  0x63,
    CMI_ESM_CONDITIONAL_IE_ERROR                                                =  0x64,
    CMI_ESM_MESSAGE_NOT_COMPATIBLE_WITH_THE_PROTOCOL_STATE                      =  0x65,
    CMI_ESM_PROTOCOL_ERROR_OR_UNSPECIFIED                                       =  0x6f,
    CMI_ESM_APN_RESTRICTION_VALUE_INCOMPATIBLE_WITH_ACTIVE_EPS_BEARER_CONTEXT   =  0x70,
    CMI_ESM_MUTIPLE_ACCESSES_TO_A_PDN_CONNECTION_NOT_ALLOWED                    =  0x71,

    /*
     * ESM internal cause, out of 3GPP defined values
    */
    CMI_ESM_INTERNAL_CAUSE_LOCAL_USED_BASE                                      =  0xE0,
    CMI_ESM_TIMER_EXPIRIED_FIVE_TIMES                                           =  0xE1,
    CMI_ESM_EPS_SERVICE_NOT_AVAILABLE                                           =  0xE2,
    CMI_ESM_UNKNOWN_BEARER_CONTEXT                                              =  0xE3,
    CMI_ESM_BEARER_CONTEXT_OPERATION_NOT_ALLOWED                                =  0xE4,
    CMI_ESM_APN_CONGESTION_CONTROL_BARRED                                       =  0xE5,
    CMI_ESM_ESTABLISH_REQ_TIMEOUT                                               =  0xE6,
    CMI_ESM_APN_AND_PDN_TYPE_DUPLICATE_USED                                     =  0xE7,
    CMI_ESM_LINKED_EPS_NOT_ACT                                                  =  0xE8,
    CMI_ESM_CAUSE_BEARER_REMAP                                                  =  0xE9,    /* ESM bearer is remapped to other CID */
    CMI_ESM_EMC_BR_ESTABLISH_FAILURE                                            =  0xEA,    /* ESM emergency bearer establish failure*/

    /* internal add here */
    CMI_ESM_CAUSE_UNKNOWN                                                       =  0xFF     /* Save memory, cause limited in one byte */

}CmiEsmCause;

/*
 * CMI_PS_GET_CEER_REQ,
*/
typedef CamCmiEmptySig CmiPsGetCeerReq;

/*
 * CMI_PS_GET_CEER_CNF,
*/
typedef struct CmiPsGetCeerCnf_Tag
{
    BOOL       bEmmCausePresent;
    BOOL       bEsmCausePresent;
    UINT16     emmCause;        /* CmiEmmCause */
    UINT16     esmCause;
 }
CmiPsGetCeerCnf;

#endif

/*
 * CMI_PS_SET_UE_OPERATION_MODE_REQ,
*/
typedef enum CmiPsUeOperationModeEnum_Tag
{
    CMI_PS_MODE_2_OF_OPERATION   = 0,
    CMI_CSPS_MODE_1_OF_OPERATION = 1,
    CMI_CSPS_MODE_2_OF_OPERATION = 2,
    CMI_PS_MODE_1_OF_OPERATION   = 3
}CmiPsUeOperationModeEnum;

typedef struct CmiPsSetUeOperationModeReqTag
{
    UINT8           ceMode;      //CmiPsUeOperationModeEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsSetUeOperationModeReq;

/*
 * CMI_PS_SET_UE_OPERATION_MODE_CNF,
*/
typedef CamCmiEmptySig  CmiPsSetUeOperationModeCnf;

/*
 * CMI_PS_GET_UE_OPERATION_MODE_REQ,
*/
typedef CamCmiEmptySig  CmiPsGetUeOperationModeReq;

/*
 * CMI_PS_GET_UE_OPERATION_MODE_CNF,
*/
typedef struct CmiPsGetUeOperationModeCnfTag
{
    UINT8           ceMode;      //CmiPsUeOperationModeEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsGetUeOperationModeCnf;

/*
 * CMI_PS_SET_UE_VOICE_DOMAIN_PREFERENCE_REQ,
*/
typedef enum CmiPsVoiceDomainPreferenceEnum_Tag
{
    CMI_CS_VOICE_ONLY                 = 1,
    CMI_CS_PREFERRED_IMS_PS_SECONDARY = 2,
    CMI_IMS_PS_PREFERRED_CS_SECONDARY = 3,
    CMI_IMS_PS_VOICE_ONLY             = 4
}CmiPsVoiceDomainPreferenceEnum;

typedef struct CmiPsSetUeVoiceDomainPreferenceReqTag
{
    UINT8           setting;      //CmiPsVoiceDomainPreferenceEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsSetUeVoiceDomainPreferenceReq;

/*
 * CMI_PS_SET_UE_VOICE_DOMAIN_PREFERENCE_CNF,
*/
typedef CamCmiEmptySig  CmiPsSetUeVoiceDomainPreferenceCnf;

/*
 * CMI_PS_GET_UE_VOICE_DOMAIN_PREFERENCE_REQ,
*/
typedef CamCmiEmptySig  CmiPsGetUeVoiceDomainPreferenceReq;

/*
 * CMI_PS_GET_UE_VOICE_DOMAIN_PREFERENCE_CNF,
*/
typedef struct CmiPsGetUeVoiceDomainPreferenceCnfTag
{
    UINT8           setting;      //CmiPsVoiceDomainPreferenceEnum
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsGetUeVoiceDomainPreferenceCnf;

/*CMI_PS_SET_DATA_OFF_REQ */
typedef struct CmiPsSetUePsDataOffReqTag
{
    UINT8           psDataoff;
    UINT8           reserved1;
    UINT16          reserved2;
}CmiPsSetUePsDataOffReq;

/*only used for ref QICSGP configure*/
#if  0
typedef struct CmirefPsDefineAuthReq_Tag
{
    BOOL bdefineAuth;
    CmiPsSetDefineAuthCtxReq  reqAuth;
}CmirefPsDefineAuthReq;
#endif

#if 0
/*CMI_PS_ACT_DED_HANDSHAKE_RESULT_REQ */
typedef struct CmiPsActDedHandSkResultReqTag
{
    UINT8           cid;
    UINT8           isAgreeAct;
    UINT16          reserved2;
}CmiPsActDedHandSkResultReq;

/*CMI_PS_MOD_DED_HANDSHAKE_RESULT_REQ */
typedef struct CmiPsModDedHandSkResultReqTag
{
    UINT8           cid;
    UINT8           isAgreeAct;
    UINT16          reserved2;
}CmiPsModDedHandSkResultReq;
#endif

/*
 * CMI_PS_GET_DATA_OFF_REQ,
*/
typedef CamCmiEmptySig  CmiPsGetUePsDataOffReq;

/*
 * CMI_PS_SET_DATA_OFF_REQ
*/
typedef struct CmiPsPsDataOffReq_Tag
{
    UINT8   psDO;
    UINT8   reserved0;
    UINT16  reserved1;
}CmiPsPsDataOffReq;

/*
 * CMI_PS_SET_DATA_OFF_CNF
*/
typedef CamCmiEmptySig CmiPsPsDataOffCnf;


/*
 * CMI_PS_GET_DATA_OFF_REQ
*/
typedef CamCmiEmptySig CmiPsGetPsDataOffReq;

/*
 * CMI_PS_GET_DATA_OFF_CNF
*/
typedef struct CmiPsGetPsDataOffCnf_Tag
{
    UINT8   psDO;
    UINT8   reserved1;
    UINT16  reserved2;
}CmiPsGetPsDataOffCnf;

#if 0
/*
 * CMI_PS_ACT_DED_HANDSHAKE_RESULT_CNF
*/
typedef CamCmiEmptySig CmiActDedHandSkRestCnf;

/*
 * CMI_PS_MOD_DED_HANDSHAKE_RESULT_CNF
*/
typedef CamCmiEmptySig CmiModDedHandSkRestCnf;
#endif


/*
 * CMI_PS_GET_DATA_COUNTER_REQ
*/
typedef CamCmiEmptySig CmiPsGetDataCounterReq;

/*
 * CMI_PS_GET_DATA_COUNTER_CNF
*/
typedef struct CmiPsGetDataCounterCnf_Tag
{
    UINT64  pkgSentBytes;
    UINT64  pkgRecvBytes;
}CmiPsGetDataCounterCnf;


/*
 * CmiPsDataCounterOptionEnum
*/
typedef enum CmiPsDataCounterOptionEnum_Tag
{
    CMI_PS_RESET_DATA_COUNTER = 0,
    CMI_PS_MANUAL_SAVE_DATA_COUNTER  = 1
}CmiPsDataCounterOptionEnum;

/*
 * CMI_PS_SET_DATA_COUNTER_REQ
*/
typedef struct CmiPsSetDataCounterReq_Tag
{
    UINT8   dataCounterOption;      //CmiPsDataCounterOptionEnum
}CmiPsSetDataCounterReq;

/*
 * CMI_PS_SET_DATA_COUNTER_CNF
*/
typedef CamCmiEmptySig CmiPsSetDataCounterCnf;


/*
 * CMI_PS_GET_SAVE_DATA_COUNTER_PERIOD_REQ
*/
typedef CamCmiEmptySig CmiPsGetSaveDataCounterPeriodReq;


/*
 * CMI_PS_GET_SAVE_DATA_COUNTER_PERIOD_CNF
*/
typedef struct CmiPsGetSaveDataCounterPeriodCnf_Tag
{
    UINT16  autoSavePeriodS;
    UINT16  rsvd;
}CmiPsGetSaveDataCounterPeriodCnf;


/*
 * CMI_PS_SET_SAVE_DATA_COUNTER_PERIOD_REQ
*/
typedef struct CmiPsSetSaveDataCounterPeriodReq_Tag
{
    UINT16   autoSavePeriodS;
    UINT16   rsvd;
}CmiPsSetSaveDataCounterPeriodReq;


/*
 * CMI_PS_SET_SAVE_DATA_COUNTER_PERIOD_CNF
*/
typedef CamCmiEmptySig CmiPsSetSaveDataCounterPeriodCnf;


typedef struct CmiPsPkgDataTransStateInd_Tag
{
    BOOL    bPkgDataTrans;    /* bwhether packet data start to transfer
                               * TRUE: L2 DRB setup, and UL or DL start Tx/Rx
                               * FALSE: UE go idle, all DRB released
                              */
    UINT8   resv[3];
}CmiPsPkgDataTransStateInd;


/*
 * CMI_PS_GET_TRAFFIC_IDLE_MONITOR_REQ
*/
typedef CamCmiEmptySig CmiPsGetTrafficIdleMonitorReq;


/*
 * CMI_PS_GET_TRAFFIC_IDLE_MONITOR_CNF
*/
typedef struct CmiPsGetTrafficIdleMonitorCnf_Tag
{
    BOOL       bEnableTrafficIdleMonitor;
    UINT8      trafficIdleMonitorTimeS;
    UINT16     retryTrafficIdleMonitorTimeS;
}CmiPsGetTrafficIdleMonitorCnf;


/*
 * CMI_PS_SET_TRAFFIC_IDLE_MONITOR_REQ
*/
typedef struct CmiPsSetTrafficIdleMonitorReq_Tag
{
    BOOL       bEnableTrafficIdleMonitor;
    BOOL       bTrafficIdleMonitorTimeSPresent;
    UINT8      trafficIdleMonitorTimeS;
    BOOL       bRetryTrafficIdleTimeSPresent;
    UINT16     retryTrafficIdleMonitorTimeS;    /*useless now*/
    UINT16     resv;
}CmiPsSetTrafficIdleMonitorReq;

/*
 * CMI_PS_SET_TRAFFIC_IDLE_MONITOR_CNF
*/
typedef CamCmiEmptySig CmiPsSetTrafficIdleMonitorCnf;

/*
 * CMI_PS_TRAFFIC_IDLE_MONITOR_IND
*/
typedef CamCmiEmptySig CmiPsTrafficIdleMonitorInd;


typedef enum CmiPsEstConnSrvType_enum
{
    CMI_PS_EST_CONN_COMMON_SERVICE,
    CMI_PS_EST_CONN_MMTEL_VOICE,
    CMI_PS_EST_CONN_MMTEL_VIDEO,   /*define here, not support now*/
    CMI_PS_EST_CONN_SMSOIP
}CmiPsEstConnSrvType;

/*
 * CMI_PS_SEND_RRC_CONN_EST_REQ
 * Request to estalbliesh RRC Connection
*/
typedef struct CmiPsSendRrcConnEstReq_Tag
{
    UINT8       srvType;        /* CmiPsEstConnSrvType */
    UINT8       rsvd0;
    UINT16      rsvd1;
}CmiPsSendRrcConnEstReq;

/**
 * CMI_PS_SEND_RRC_CONN_EST_CNF
 * estalbliesh RRC Connection confirm
*/
typedef struct CmiPsSendRrcConnEstCnf_Tag
{
    BOOL        bSucc;
    UINT8       rsvd0;
    UINT16      rsvd1;
}CmiPsSendRrcConnEstCnf;


/**
 * CMI_PS_SET_L2_MONITOR_REPORT_REQ
 * Note: following setting invalid when hardware power off, and default value is FALSE
*/
typedef struct CmiPsSetL2MonitorReportReq_Tag
{
    BOOL        rohcCfgRptPresent;  /* whether following: "rohcCfgRptEnable" present */
    BOOL        cgstParamPresent;   /* whether following: "cgstBufRptEnable"/"bufNormalLevl"/"bufWarnLevl"/"bufWarnLevl"/"bufErrorLevl" present */
    BOOL        rohcErrRptPresent;  /* whether following: "rohcErrRtpEnable"/"rohcErrLevl"/"rohcRptBaseCount" present */

    BOOL        rohcCfgRptEnable;   /* whether L2 ROHC configuration need to report in "CmiPsL2RohcConfigInd" */

    BOOL        cgstBufRptEnable;   /* Whether L2 buffer congestion monitor enabled, if enable, "CmiPsL2CongestReportInd" will report if event tiggered  */
    UINT8       bufNormalLevl;      /* UL buffer usage normal level in precentage, range: [10-100]  */
    UINT8       bufWarnLevl;        /* UL buffer usage wanring level in precentage, range: [30-100], and must > 'bufNormalLevl' */
    UINT8       bufErrorLevl;       /* UL buffer usage error level in precentage, range: [50-100], and must > 'bufWarnLevl' */


    BOOL        rohcErrRptEnable;
    UINT8       rohcErrLevl;        /* ROHC error report level, range: [10-100] */
    UINT16      rohcRptBaseCount;   /* ROHC error report base pkg count, range:[10-0xFFFF], to redure report frequency, report "CmiPsRohcErrorReportInd" in case of:
                                     * a) for all CID which configed ROHC: (ulRohcCount + dlRohcCount) >= rohcRptBaseCount, and
                                     * b) (ulRohcErrRate >= rohcErrLevl || dlRohcErrRate > rohcErrLevl)
                                    */
}CmiPsSetL2MonitorReportReq;    //12 bytes

/*
 * CMI_PS_SET_L2_MONITOR_REPORT_CNF
*/
typedef CamCmiEmptySig CmiPsSetL2MonitorReportCnf;


/**
 * CMI_PS_L2_CONGEST_REPORT_IND     //CmiPsL2CongestReportInd
*/
/*
 * L2 congest report event enum
*/
typedef enum
{
    CMI_PS_UL_BUF_INCREASE_WARNING  = 0,    /* UL buffer increase to warning level */
    CMI_PS_UL_BUF_INCREASE_ERROR,           /* UL buffer increase to error level */
    CMI_PS_UL_BUF_DECREASE_WARNING,         /* UL buffer decrease from error level to warning level */
    CMI_PS_UL_BUF_DECREASE_NORMAL           /* UL buffer decrease from warning level to normal level */
}CmiPsL2CongestEvent;

typedef struct
{
    UINT8   epsId;              /* EPS bearer ID, range: [5:15], is set to 0 means last EPS bearer info */
    UINT8   rsvd0;
    UINT16  rsvd1;
    UINT16  discardTimerMs;     /* configed PDCP discard timer in MS */
    UINT16  discardNum;         /* L2 reset to 0, after report */
}CmiPsL2CongestDiscardInfo; // 8 bytes

typedef struct CmiPsL2CongestReportInd_Tag
{
    UINT8       cgstEvent;              /* CmiPsL2CongestEvent */
    UINT8       rsvd0;
    UINT16      rsvd1;

    UINT32      ulBufTotalLen;          /* in unit of bytes */
    UINT32      ulBufUsedLen;

    INT16       sRsrp;                  /*current serving cell RSRP in dBm, range: [-150, -44]*/
    INT8        sRsrq;                  /*current serving cell RSRQ in dB, range: [-68, -5]*/
    INT8        snr;                    /*current serving cell SNR, range: [-20, 40]*/

    CmiPsL2CongestDiscardInfo   cgstDiscardInfo[CMI_PS_MAX_BEARER_NUM]; //88 bytes
}CmiPsL2CongestReportInd;   // 104 bytes

/**
 * CMI_PS_L2_ROHC_CONFIG_IND    //CmiPsL2RohcConfigInd
*/
typedef struct CmiPsL2RohcConfigInd_Tag
{
    struct {
        UINT8   epsId;      /* EPS bearer ID, range: [5:15], if set to 0 means last EPS bearer info */
        UINT8   drbId;
        UINT8   bRohcCfg;   /* whether L2/DRB ROHC configed */
        UINT8   rsvd0;

        UINT16  rohcMaxCID;

        /* ROHC profile bitmap:
         * profiles SEQUENCE {
         *  profile0x0001 BOOLEAN,      //RTP/UDP/IP    RFC 3095, RFC 4815      BIT 0
         *  profile0x0002 BOOLEAN,      //UDP/IP        RFC 3095, RFC 4815      BIT 1
         *  profile0x0003 BOOLEAN,      //ESP/IP        RFC 3095, RFC 4815      BIT 2
         *  profile0x0004 BOOLEAN,      //IP            RFC 3843, RFC 4815      BIT 3
         *  profile0x0006 BOOLEAN,      //TCP/IP        RFC 6846                BIT 4
         *  profile0x0101 BOOLEAN,      //RTP/UDP/IP    RFC 5225                BIT 5
         *  profile0x0102 BOOLEAN,      //UDP/IP        RFC 5225                BIT 6
         *  profile0x0103 BOOLEAN,      //ESP/IP        RFC 5225                BIT 7
         *  profile0x0104 BOOLEAN       //IP            RFC 5225                BIT 8
         *  },
        */
        UINT16  rohcProfiles;
    } rohcCfg[CMI_PS_MAX_BEARER_NUM];   //8*11 = 88 bytes
}CmiPsL2RohcConfigInd;  // 88 bytes


/**
 * CMI_PS_ROHC_ERROR_REPORT_IND     //CmiPsRohcErrorReportInd
*/
typedef struct CmiPsRohcErrorReportInd_Tag
{
    UINT8   cidNum;         /*valid CID number in "cidList" & "rohcErrInfo" */
    UINT8   rsvd0;
    UINT16  rsvd1;

    UINT8   cidList[CMI_PS_CID_NUM];
    struct {
        UINT32  ulErrCount; /* all these count reset to 0, when reported */
        UINT32  ulAllCount;

        UINT32  dlErrCount;
        UINT32  dlAllCount;
    }rohcErrInfo[CMI_PS_CID_NUM];   //16*16 = 256 bytes
}CmiPsRohcErrorReportInd;   //276 bytes

/**
 * CMI_PS_CNEC_ERROR_CODE_REPORT_IND     //CmiPsCnecErrorCodeReportInd
*/
/*
 * EMM/ESM/5GSM/5GMM report error code
*/
typedef enum
{
    CMI_PS_EMM  = 0,
    CMI_PS_ESM,
    CMI_PS_5GMM,
    CMI_PS_5GSM
}CmiPsCnecSrcModule;

/*
 * CMI_PS_CNEC_ERROR_CODE_REPORT_IND
 *  +CNEC: <error_code>[,<cid>]
*/
typedef struct CmiPsCnecErrorCodeReportInd_Tag
{
    UINT8   srcModule; //CmiPsCnecSrcModule
    UINT8   errorCode; //ESM/EMM/5GSM/5GMM unsolicited reporting of error codes sent by the network
    BOOL    cidPst;
    UINT8   cid;
}CmiPsCnecErrorCodeReportInd;

/*
 * CMI_PS_GET_ECSMER_REQ
 * Request to get last esm cause
*/
typedef struct CmiPsGetEcsmerReq_Tag
{
    BOOL        getAll;
    UINT8       cid;
    UINT16      rsvd;
}CmiPsGetEcsmerReq;

/*
 * CMI_PS_GET_ECSMER_CNF,
*/
typedef struct CmiPsGetEcsmerCnf_Tag
{
    struct {
        BOOL        valid;
        UINT8       cid;
        UINT16      esmCause; //EsmCause
    } lastCause[CMI_PS_LAST_ESM_CAUSE_NUM];

 }CmiPsGetEcsmerCnf;// 12 bytes

 /*
 * CMI_PS_DEL_ECSMER_REQ
 * Request to get last esm cause
*/
typedef struct CmiPsDelEcsmerReq_Tag
{
    UINT8       cid;
    UINT8       rsvd0;
    UINT16      rsvd1;
}CmiPsDelEcsmerReq;

typedef CamCmiEmptySig CmiPsDelEcsmerCnf;

#endif

