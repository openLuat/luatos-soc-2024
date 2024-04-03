/*
 * Copyright (c) 2023 OpenLuat & AirM2M
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy of
 * this software and associated documentation files (the "Software"), to deal in
 * the Software without restriction, including without limitation the rights to
 * use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of
 * the Software, and to permit persons to whom the Software is furnished to do so,
 * subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS
 * FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR
 * COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER
 * IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
 * CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 */

#include "luat_mobile.h"
#include "ps_lib_api.h"
#include "pscfgnvm.h"
#include "common_api.h"
#include "cmimm.h"
#include "cmidev.h"
#include "cmips.h"
#include "cms_api.h"
#include "soc_service.h"

extern void soc_mobile_event_deregister_handler(void);
extern void soc_mobile_get_imsi(uint8_t *buf);
extern void soc_mobile_get_iccid(uint8_t *buf);
extern void soc_mobile_get_sim_number(uint8_t *buf);
extern void soc_mobile_event_register_handler(void *handle);
extern void soc_mobile_set_period(uint32_t get_cell_period, uint32_t check_sim_period, uint8_t search_cell_time);
extern void soc_mobile_reset_stack(void);
extern void soc_mobile_get_signal(CmiMmCesqInd *info);
extern void soc_mobile_get_cell_info(CmiDevGetBasicCellListInfoInd *info);
extern void soc_mobile_get_lte_service_info(CmiPsCeregInd *info);
extern void soc_mobile_get_sim_id(uint8_t *sim_id, uint8_t *is_auto);
extern void soc_mobile_set_sim_id(uint8_t sim_id);
extern void soc_mobile_sim_detect_sim0_first(void);
extern void soc_mobile_sms_event_register_handler(void *handle);
extern uint8_t soc_mobile_get_csq(void);
extern void soc_mobile_search_cell_info_async(uint8_t param);
extern void soc_mobile_set_user_apn_auto_active(uint8_t cid,
		uint8_t ip_type,
		uint8_t protocol_type,
		uint8_t *apn_name, uint8_t apn_name_len,
		uint8_t *user, uint8_t user_len,
		uint8_t *password, uint8_t password_len);
extern void soc_mobile_get_ip_data_traffic(uint64_t *uplink, uint64_t *downlink);
extern void soc_mobile_clear_ip_data_traffic(uint8_t clear_uplink, uint8_t clear_downlink);
extern uint8_t soc_mobile_get_sim_state(void);
extern void soc_mobile_rf_test_mode(uint8_t uart_id, uint8_t on_off);
extern void soc_mobile_rf_test_input(uint8_t *data, uint32_t len);
extern void soc_mobile_get_last_cc_event_info(void *info, uint32_t *info_len);
extern void soc_mobile_get_last_nas_event(uint8_t *type, uint16_t *cause, uint16_t *phy_cellid, uint32_t *freq);
void soc_mobile_set_network_check_period(uint32_t period);

/**
 * @brief 获取默认PDP的一些信息
 *
 * @param ip_type[OUT] 激活的IP类型
 * @param apn[OUT] apn name
 * @param apn_len[OUT] apn name 长度
 * @param dns_num[OUT] DNS数量
 * @param dns_ip[OUT] DNS ip
 * @return int < 0错误 >0实际使用的CID序号
 */
int soc_mobile_get_default_pdp_part_info(uint8_t *ip_type, uint8_t *apn,uint8_t *apn_len, uint8_t *dns_num, ip_addr_t *dns_ip);

int luat_mobile_get_imei(int sim_id, char* buff, size_t buf_len)
{
	char temp[20] = {0};
	int result = appGetImeiNumSync(temp);
	if (!result)
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}


}

int luat_mobile_get_sn(char* buff, size_t buf_len)
{
	char temp[32] = {0};
//	int result = appGetSNNumSync(temp);
	uint32_t result = nvramRead(0,temp,32,32);
	if (result > 0)
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_set_sn(char* buff, uint8_t buf_len)
{
	int result = appSetSNNumSync(buff, buf_len);
	return result==1 ? 0 : -1;
}

int luat_mobile_get_muid(char* buff, size_t buf_len)
{
	char temp[64] = {0};
	int result = soc_get_sn(temp, sizeof(temp));
	if (!result)
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_get_iccid(int sim_id, char* buff, size_t buf_len)
{
	char temp[24] = {0};
	soc_mobile_get_iccid(temp);
	if (temp[0])
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}

}

int luat_mobile_get_imsi(int sim_id, char* buff, size_t buf_len)
{
	char temp[20] = {0};
	soc_mobile_get_imsi(temp);
	if (temp[0])
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_get_sim_number(int sim_id, char* buff, size_t buf_len)
{
	char temp[24] = {0};
	soc_mobile_get_sim_number(temp);
	if (temp[0])
	{
		memcpy(buff, temp, (buf_len > sizeof(temp))?sizeof(temp):buf_len);
		return (buf_len > sizeof(temp))?sizeof(temp):buf_len;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_get_sim_id(int *id)
{
	uint8_t sim_id, is_auto;
	soc_mobile_get_sim_id(&sim_id, &is_auto);
	if (sim_id != 0xff)
	{
		*id = sim_id;
		return 0;
	}
	else
	{
		return -1;
	}

}

int luat_mobile_set_sim_id(int id)
{
	if (id > 2)
	{
		return -1;
	}
	else
	{
		soc_mobile_set_sim_id(id);
		return 0;
	}
}

int luat_mobile_set_sim_pin(int sim_id, uint8_t operation, char pin1[9], char pin2[9])
{
	SetPinOperReqParams pPinOperReqParams = {0};
	memcpy(pPinOperReqParams.pinStr, pin1, 8);
	memcpy(pPinOperReqParams.newPinStr, pin2, 8);
	pPinOperReqParams.operMode = operation;
	return appSetPinOperationSync(&pPinOperReqParams);
}

int luat_mobile_soft_sim_switch(uint8_t enable)
{
	if(enable != 0 && enable != 1)
	{
		return -1;
	}
	EcSimCfgSetParams cfg = {0};
	cfg.softsimPresent = TRUE;
	cfg.bSoftSim = enable;
	return appSetECSIMCFGSync(&cfg);
}


int luat_mobile_get_soft_sim_cfg(uint8_t *is_soft_sim)
{
	if(is_soft_sim == NULL)
	{
		return -1;
	}
	EcSimCfgGetParams cfg = {0};
	if(appGetECSIMCFGSync(&cfg) != CMS_RET_SUCC)
	{
		return -1;
	}
	*is_soft_sim = cfg.bSoftSim;
	return 0;
}

uint8_t luat_mobile_get_sim_ready(int id)
{
	return soc_mobile_get_sim_state();
}

void luat_mobile_set_sim_detect_sim0_first(void)
{
	soc_mobile_sim_detect_sim0_first();
}

int luat_mobile_get_apn(int sim_id, int cid, char* buff, size_t buf_len)
{
	uint8_t type;
	int work_cid = soc_mobile_get_default_pdp_part_info(&type, NULL, NULL, NULL, NULL);
	if (cid > 0 && work_cid != cid)
	{
		return -1;
	}
	uint8_t apn_len = buf_len;
	soc_mobile_get_default_pdp_part_info(&type, buff, &apn_len, NULL, NULL);
	return apn_len;
}

int luat_mobile_get_default_apn(int sim_id, char* buff, size_t buf_len)
{
	return luat_mobile_get_apn(sim_id, -1, buff, buf_len);
}

static void luat_mobile_del_apn_ec618(UINT16 paramSize, void *pParam)
{
	uint8_t *temp = (uint8_t *)pParam;
	if (temp[1])
	{
		psDeleteCGDedDCONTContext(PS_DIAL_REQ_HANDLER, temp[0]);
	}
	else
	{
		psDeleteCGDCONTContext(PS_DIAL_REQ_HANDLER, temp[0]);
	}
}

int luat_mobile_del_apn(int sim_id, uint8_t cid, uint8_t is_dedicated)
{
	uint8_t temp[4];
	temp[0] = cid;
	temp[1] = is_dedicated;
	return cmsNonBlockApiCall(luat_mobile_del_apn_ec618, 4, temp);
}

static uint8_t s_disable_default_pdp;
static uint8_t s_default_pdn_ip_type = 1;
uint8_t soc_disable_tcpip_use_default_pdp(void)
{
	return s_disable_default_pdp;
}
uint8_t soc_mobile_default_pdn_ip_type(void)
{
	return s_default_pdn_ip_type;
}

void luat_mobile_user_ctrl_apn(void)
{
	s_disable_default_pdp = 1;
}

void luat_mobile_user_ctrl_apn_stop(void)
{
	s_disable_default_pdp = 0;
}

void luat_mobile_set_default_pdn_ipv6(uint8_t onoff)
{
	s_default_pdn_ip_type = onoff?3:1;
}

uint8_t luat_mobile_get_default_pdn_ipv6(void)
{
	return (s_default_pdn_ip_type == 2 || s_default_pdn_ip_type == 3)?1:0;
}

int luat_mobile_set_apn_base_info(int sim_id, int cid, uint8_t type, uint8_t* apn_name, uint8_t name_len)
{
	CmiPsDefPdpDefinition   pdpCtxInfo = {0};
	pdpCtxInfo.cid = cid;
	pdpCtxInfo.pdnType = type;
	pdpCtxInfo.apnPresentType = CMI_UPDATE_WITH_NEW;
	pdpCtxInfo.apnLength = name_len;
	memcpy( pdpCtxInfo.apnStr, apn_name, name_len);
	return psSetCdgcont(PS_DIAL_REQ_HANDLER, &pdpCtxInfo);
}


int luat_mobile_set_apn_auth_info(int sim_id, int cid, uint8_t protocol, uint8_t *user_name, uint8_t user_name_len, uint8_t *password, uint8_t password_len)
{
	CmiPsSetDefineAuthCtxReq req = {0};
	req.cmiAuthInfo.cid = cid;
	if (protocol != 0xff)
	{
		req.cmiAuthInfo.authProtPresent = 1;
		req.cmiAuthInfo.authProtocol = protocol;
	}

	if (user_name_len)
	{
		if (user_name)
		{
			req.cmiAuthInfo.authUserNameLength = user_name_len;
			memcpy(req.cmiAuthInfo.authUserName, user_name, user_name_len);
		}
		req.cmiAuthInfo.authUserPresent = 1;
	}

	if (password_len)
	{
		if (password)
		{
			req.cmiAuthInfo.authPasswordLength = password_len;
			memcpy(req.cmiAuthInfo.authPassword, password, password_len);
		}
		req.cmiAuthInfo.authPswdPresent = 1;
	}
	return psSetCGAUTH(PS_DIAL_REQ_HANDLER, &req);
}

static void luat_mobile_deactive_apn_ec618(UINT16 paramSize, void *pParam)
{
	int cid;
	memcpy(&cid, pParam, 4);
	psSetCGACT(PS_DIAL_REQ_HANDLER, cid, 0);
}

int luat_mobile_active_apn(int sim_id, int cid, uint8_t state)
{
	soc_mobile_active_cid(cid);
	if (state)
	{
		return psSetCGACT(PS_DIAL_REQ_HANDLER, cid, state);
	}
	else
	{
		return cmsNonBlockApiCall(luat_mobile_deactive_apn_ec618, 4, &cid);
	}
}

int luat_mobile_active_netif(int sim_id, int cid)
{
	return psGetCGCONTRDPParam(PS_DIAL_REQ_HANDLER, cid);
}

void luat_mobile_user_apn_auto_active(int sim_id, uint8_t cid,
		uint8_t ip_type,
		uint8_t protocol_type,
		uint8_t *apn_name, uint8_t apn_name_len,
		uint8_t *user, uint8_t user_len,
		uint8_t *password, uint8_t password_len)
{
	s_disable_default_pdp = 1;
	soc_mobile_set_user_apn_auto_active(cid, ip_type, protocol_type, apn_name, apn_name_len, user, user_len, password, password_len);
}
// 进出飞行模式
int luat_mobile_set_flymode(int index, int mode)
{
	return appSetCFUN(!mode);
}

int luat_mobile_get_flymode(int index)
{
	uint8_t state;
	int result = appGetCFUN(&state);
	if (!result)
	{
		return state;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_get_local_ip(int sim_id, int cid, ip_addr_t *ip_v4, ip_addr_t *ip_v6)
{
	int i;
	NmAtiNetifInfo NetifInfo;
	NetMgrGetNetInfo(cid, &NetifInfo);
	if (NetifInfo.ipv4Cid != 0xff)
	{
		ip_v4->u_addr.ip4 = NetifInfo.ipv4Info.ipv4Addr;
		ip_v4->type = IPADDR_TYPE_V4;
	}
	else
	{
		ip_v4->type = 0xff;
	}
	if (NetifInfo.ipv6Cid != 0xff)
	{
		ip_v6->u_addr.ip6 = NetifInfo.ipv6Info.ipv6Addr;
		ip_v6->type = IPADDR_TYPE_V6;
	}
	else
	{
		ip_v6->type = 0xff;
	}
	return 0;
}

/* -------------------------------------------------- cell info begin -------------------------------------------------- */
uint8_t luat_mobile_rssi_to_csq(int8_t rssi)
{
	if (rssi <= -113)
	{
		return 0;
	}
	else if (rssi < -52)
	{
		return (rssi + 113) >> 1;
	}
	else
	{
		return 31;
	}
}

static void ec618_cell_to_luat_cell(BasicCellListInfo *bcListInfo, luat_mobile_cell_info_t *info)
{
	memset(info, 0, sizeof(luat_mobile_cell_info_t));
	info->gsm_info_valid = 0;
	if (!bcListInfo->sCellPresent && !bcListInfo->nCellNum)
	{
		info->lte_info_valid = 0;
		return;
	}
	if (!bcListInfo->sCellPresent)
	{
		info->lte_service_info.cid = 0;
	}
	else
	{
		if (bcListInfo->sCellInfo.rsrp < 0)
		{
			info->lte_info_valid = 1;
			info->lte_service_info.cid = bcListInfo->sCellInfo.cellId;
			info->lte_service_info.band = bcListInfo->sCellInfo.band;
			info->lte_service_info.dlbandwidth = bcListInfo->sCellInfo.dlBandWidth;
			info->lte_service_info.ulbandwidth = bcListInfo->sCellInfo.ulBandWidth;
			info->lte_service_info.is_tdd = bcListInfo->sCellInfo.isTdd;
			info->lte_service_info.earfcn = bcListInfo->sCellInfo.earfcn;
			info->lte_service_info.pci = bcListInfo->sCellInfo.phyCellId;
			info->lte_service_info.tac = bcListInfo->sCellInfo.tac;
			info->lte_service_info.snr = bcListInfo->sCellInfo.snr;
			info->lte_service_info.rsrp = bcListInfo->sCellInfo.rsrp;
			info->lte_service_info.rsrq = bcListInfo->sCellInfo.rsrq;
			info->lte_service_info.rssi = bcListInfo->sCellInfo.rsrp - bcListInfo->sCellInfo.rsrq + (bcListInfo->sCellInfo.rssiCompensation/100);
			info->lte_service_info.mcc = bcListInfo->sCellInfo.plmn.mcc;
			if (0xf000 == (bcListInfo->sCellInfo.plmn.mncWithAddInfo & 0xf000))
			{
				info->lte_service_info.mnc = bcListInfo->sCellInfo.plmn.mncWithAddInfo & 0x0fff;
			}
			else
			{
				info->lte_service_info.mnc = bcListInfo->sCellInfo.plmn.mncWithAddInfo;
			}
		}
		else
		{
			info->lte_info_valid = 0;
			return;
		}

	}

	if (!bcListInfo->nCellNum)
	{
		info->lte_neighbor_info_num = 0;
	}
	else
	{
		uint8_t j = 0;
		for(uint8_t i = 0; i < bcListInfo->nCellNum; i++)
		{
			if (bcListInfo->nCellList[i].cellInfoValid)
			{
				info->lte_info[j].cid = bcListInfo->nCellList[i].cellId;
				info->lte_info[j].tac = bcListInfo->nCellList[i].tac;
				info->lte_info[j].mcc = bcListInfo->nCellList[i].plmn.mcc;

				if (0xf000 == (bcListInfo->nCellList[i].plmn.mncWithAddInfo & 0xf000))
				{
					info->lte_info[j].mnc = bcListInfo->nCellList[i].plmn.mncWithAddInfo & 0x0fff;
				}
				else
				{
					info->lte_info[j].mnc = bcListInfo->nCellList[i].plmn.mncWithAddInfo;
				}
			}
			else
			{
				info->lte_info[j].cid = 0;
				info->lte_info[j].tac = 0;
				info->lte_info[j].mcc = 0;
				info->lte_info[j].mnc = 0;
			}
			info->lte_info[j].earfcn = bcListInfo->nCellList[i].earfcn;
			info->lte_info[j].pci = bcListInfo->nCellList[i].phyCellId;
			info->lte_info[j].snr = bcListInfo->nCellList[i].snr;
			info->lte_info[j].rsrp = bcListInfo->nCellList[i].rsrp;
			info->lte_info[j].rsrq = bcListInfo->nCellList[i].rsrq;
//			if ((info->lte_info[j].mcc == 0x0460) && (info->lte_info[j].mnc != 0x0015))
			{
				j++;

			}
		}
		info->lte_neighbor_info_num = j;
	}
}

static void ec618_signal_to_luat_signal(CmiMmCesqInd *cesq_info, luat_mobile_signal_strength_info_t *info)
{
	uint8_t zero[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
	info->luat_mobile_gw_signal_strength_vaild = 0;
	if (memcmp(zero, cesq_info, sizeof(CmiMmCesqInd)))
	{
		info->luat_mobile_lte_signal_strength_vaild = 1;
	    if (cesq_info->rsrp != CMI_MM_NOT_DETECT_RSRP)
	    {
	        if (cesq_info->rsrp > 0)
	        {
	        	info->lte_signal_strength.rsrp = (cesq_info->rsrp - 141);
	        }
	        else
	        {
	        	info->lte_signal_strength.rsrp = (cesq_info->rsrp - 140);
	        }
	    }
	    else
	    {
	    	info->lte_signal_strength.rsrp = -999;
	    }
	    if (cesq_info->rsrq != CMI_MM_NOT_DETECT_RSRQ)
	    {
	        if (cesq_info->rsrq <= 0)
	        {
	        	info->lte_signal_strength.rsrq = (cesq_info->rsrq - 39) >> 1;
	        }
	        else if (cesq_info->rsrq <= 34)
	        {
	        	info->lte_signal_strength.rsrq = (cesq_info->rsrq - 40) >> 1;
	        }
	        else
	        {
	        	info->lte_signal_strength.rsrq = (cesq_info->rsrq - 41) >> 1;
	        }
	    }
	    else
	    {
	    	info->lte_signal_strength.rsrq = -999;
	    }
		if (cesq_info->rsrp != CMI_MM_NOT_DETECT_RSRP &&
				cesq_info->rsrq != CMI_MM_NOT_DETECT_RSRQ)
		{
			info->lte_signal_strength.rssi = info->lte_signal_strength.rsrp - info->lte_signal_strength.rsrq + (cesq_info->rssiCompensation/100);

		}
		else
		{
			info->lte_signal_strength.rssi = -999;
		}
		info->lte_signal_strength.snr = cesq_info->snr;

	}
	else
	{
		info->luat_mobile_lte_signal_strength_vaild = 0;
	}
}

int luat_mobile_get_cell_info(luat_mobile_cell_info_t  *info)
{
	BasicCellListInfo bcListInfo;
	int result = appGetECBCInfoSync(&bcListInfo);
	if (!result)
	{
		ec618_cell_to_luat_cell(&bcListInfo, info);
		return 0;
	}
	else
	{
		return -1;
	}
}

int luat_mobile_get_cell_info_async(uint8_t max_time)
{
	soc_mobile_search_cell_info_async(max_time);
	return 0;
}

int luat_mobile_get_last_notify_cell_info(luat_mobile_cell_info_t  *info)
{
	BasicCellListInfo bcListInfo = {0};
	soc_mobile_get_cell_info(&bcListInfo);
	ec618_cell_to_luat_cell(&bcListInfo, info);
	return 0;
}


int luat_mobile_get_signal_strength_info(luat_mobile_signal_strength_info_t *info)
{
	return luat_mobile_get_last_notify_signal_strength_info(info);
}

int luat_mobile_get_signal_strength(uint8_t *csq)
{
	return luat_mobile_get_last_notify_signal_strength(csq);
}

int luat_mobile_get_last_notify_signal_strength_info(luat_mobile_signal_strength_info_t *info)
{
	CmiMmCesqInd cesq_info;
	soc_mobile_get_signal(&cesq_info);
	ec618_signal_to_luat_signal(&cesq_info, info);
	return 0;
}

int luat_mobile_get_last_notify_signal_strength(uint8_t *csq)
{
	*csq = soc_mobile_get_csq();
	return 0;
}

int luat_mobile_get_service_cell_identifier(uint32_t *eci)
{
	CmiPsCeregInd cereg;
	soc_mobile_get_lte_service_info(&cereg);
	if (cereg.cellId)
	{
		*eci = cereg.cellId;
		return 0;
	}
	else
	{
		return -1;
	}
}
int luat_mobile_get_service_tac_or_lac(uint16_t *tac) {
	CmiPsCeregInd cereg;
	soc_mobile_get_lte_service_info(&cereg);
	if (cereg.cellId)
	{
		*tac = cereg.tac;
		return 0;
	}
	else
	{
		return -1;
	}
}
/* --------------------------------------------------- cell info end --------------------------------------------------- */


/* ------------------------------------------------ mobile status begin ----------------------------------------------- */
LUAT_MOBILE_SIM_STATUS_E luat_mobile_get_sim_status(void)
{
	uint8_t pin_state = CMI_SIM_PIN_STATE_UNKNOWN;
	CmiRcCode result = simGetPinStateSync(&pin_state);
	if (CME_SUCC == result)
	{
		switch (pin_state)
		{
			case CMI_SIM_PIN_STATE_READY:
				return LUAT_MOBILE_SIM_READY;
				break;
			default:
				break;
		}
	}
	else if(CME_SIM_NOT_INSERT == result)
	{
		return LUAT_MOBILE_NO_SIM;
	}
	
	return LUAT_MOBILE_SIM_NUMBER;
}

LUAT_MOBILE_REGISTER_STATUS_E luat_mobile_get_register_status(void)
{
	CeregGetStateParams param;
	int result = appGetCeregStateSync(&param);
	if (!result)
	{
		return param.state;
	}
	return LUAT_MOBILE_STATUS_UNKNOW;
}

int luat_mobile_event_register_handler(luat_mobile_event_callback_t callback_fun)
{
	soc_mobile_event_register_handler(callback_fun);
	return 0;
}
int luat_mobile_event_deregister_handler(void)
{
	soc_mobile_event_deregister_handler();
	return 0;
}

int luat_mobile_sms_event_register_handler(luat_mobile_sms_event_callback_t callback_fun)
{
	soc_mobile_sms_event_register_handler(callback_fun);
	return 0;
}

/* ------------------------------------------------- mobile status end ------------------------------------------------ */

extern soc_mobile_set_rrc_release_time(uint8_t s);
void luat_mobile_set_rrc_auto_release_time(uint8_t s)
{
	if (1 == s) s = 2;
	soc_mobile_set_rrc_release_time(s);
}

extern void soc_mobile_release_rrc_pause(uint8_t onoff);
void luat_mobile_rrc_auto_release_pause(uint8_t onoff)
{
	;
}

extern void soc_mobile_rrc_release_once(void);
void luat_mobile_rrc_release_once(void)
{
	;
}

int luat_mobile_reset_stack(void)
{
	soc_mobile_reset_stack();
	return 0;
}
extern void soc_mobile_fatal_error_auto_reset_stack(uint8_t onoff);
void luat_mobile_fatal_error_auto_reset_stack(uint8_t onoff)
{
	soc_mobile_fatal_error_auto_reset_stack(onoff);
}

int luat_mobile_set_period_work(uint32_t get_cell_period, uint32_t check_sim_period, uint8_t search_cell_time)
{
	soc_mobile_set_period(get_cell_period, check_sim_period, search_cell_time);
	return 0;
}

void luat_mobile_set_check_network_period(uint32_t period)
{
	soc_mobile_set_network_check_period(period);
}

void luat_mobile_get_ip_data_traffic(uint64_t *uplink, uint64_t *downlink)
{
	soc_mobile_get_ip_data_traffic(uplink, downlink);
}

void luat_mobile_clear_ip_data_traffic(uint8_t clear_uplink, uint8_t clear_downlink)
{
	soc_mobile_clear_ip_data_traffic(clear_uplink, clear_downlink);
}

int luat_mobile_set_cell_resel(uint8_t resel)
{
	EcCfgSetParamsReq req = {0};
	req.reselToWeakNcellOpt = resel;
	req.reselToWeakNcellOptPresent = 1;
	if (appSetEcCfgSettingSync(&req) != CMS_RET_SUCC)
	{
		return -1;
	}
	return 0;
}

int luat_mobile_get_support_band(uint8_t *band,  uint8_t *total_num)
{
	return (appGetSupportedBandModeSync(total_num, band) == 0)?0:-1;
}

int luat_mobile_get_band(uint8_t *band,  uint8_t *total_num)
{
	return (appGetBandModeSync(total_num, band) == 0)?0:-1;
}

int luat_mobile_set_band(uint8_t *band,  uint8_t total_num)
{
	return (appSetBandModeSync(total_num, band) == 0)?0:-1;
}


static CmsRetId luatSetPSMSettingTest(UINT8 psmMode)
{
    CmsRetId            cmsRet = CMS_RET_SUCC;
    AppPsCmiReqData     psCmiReqData = {0};   //20 bytes
    CmiMmSetPsmParmReq  cmiReq;
    CmiMmSetPsmParmCnf  cmiCnf;
    UINT8               bitmap = 0;

    memset(&cmiReq, 0, sizeof(CmiMmSetPsmParmReq));
    cmiReq.mode = psmMode;

    psCmiReqData.sgId       = CAM_MM;
    psCmiReqData.reqPrimId  = CMI_MM_SET_REQUESTED_PSM_PARM_REQ;
    psCmiReqData.cnfPrimId  = CMI_MM_SET_REQUESTED_PSM_PARM_CNF;
    psCmiReqData.reqParamLen = sizeof(cmiReq);
    psCmiReqData.pReqParam  = &cmiReq;

    /* output here */
    psCmiReqData.cnfBufLen = sizeof(cmiCnf);
    psCmiReqData.pCnfBuf   = &cmiCnf;

    cmsRet = appPsCmiReq(&psCmiReqData, CMS_MAX_DELAY_MS);

    if (cmsRet == CMS_RET_SUCC && psCmiReqData.cnfRc == CME_SUCC)
    {
        cmsRet = CMS_RET_SUCC;
    }
    else
    {
        OsaDebugBegin(FALSE, cmsRet, psCmiReqData.cnfRc, 0);
        OsaDebugEnd();

        if (cmsRet == CMS_RET_SUCC)
        {
            cmsRet = psSyncProcErrCode(psCmiReqData.cnfRc);
        }
    }

    return cmsRet;
}

static void luat_mobile_set_attach_type_ec618(UINT16 paramSize, void *pParam)
{
	uint32_t value;
	memcpy(value, pParam, 4);
	DBG("CE MODE %d", value);
	if (value <= CMI_PS_MODE_1_OF_OPERATION)
	{
		psSetCemode(PS_DIAL_REQ_HANDLER, value);
	}
}

static void luat_mobile_sim_write_mode(UINT16 paramSize, void *pParam) {
	CmiSimSetSimWriteCounterReq req2 = {0};
	uint32_t value = 0;
	memcpy(&value, pParam, 4);
	req2.mode = (uint8_t)value;
	//DBG("luat_mobile_sim_write_mode %d", value);
	psCamCmiReq(CMS_REQ_HANDLER, CAM_SIM, CMI_SIM_SET_SIM_WRITE_COUNTER_REQ, sizeof(CmiSimSetSimWriteCounterReq), &req2);
}

int luat_mobile_config(uint8_t item, uint32_t value)
{
	EcCfgSetParamsReq req = {0};
	switch(item)
	{

	case MOBILE_CONF_RESELTOWEAKNCELL:
		req.reselToWeakNcellOpt = value;
		req.reselToWeakNcellOptPresent = 1;
		break;
	case MOBILE_CONF_STATICCONFIG:
		req.staticConfig = value;
		req.staticConfigPresent = 1;
		break;
	case MOBILE_CONF_QUALITYFIRST:
		req.qualityFirst = value;
		req.qualityFirstPresent = 1;
		break;
	case MOBILE_CONF_USERDRXCYCLE:
		req.userDrxCycle = value;
		req.userDrxCyclePresent = 1;
		break;
	case MOBILE_CONF_T3324MAXVALUE:
		req.t3324MaxValueS = value;
		req.t3324MaxValuePresent = 1;
		break;
	case MOBILE_CONF_PSM_MODE:
		if (luatSetPSMSettingTest(value) != CMS_RET_SUCC)
		{
			return -1;
		}
		return 0;
	case MOBILE_CONF_CE_MODE:
		cmsNonBlockApiCall(luat_mobile_set_attach_type_ec618, 4, &value);
		return 0;
		break;
	case MOBILE_CONF_SIM_WC_MODE:
		// DBG("CALL psCamCmiReq");
		cmsNonBlockApiCall(luat_mobile_sim_write_mode, 4, &value);
		return 0;
	case MOBILE_CONF_FAKE_CELL_BARTIME:
		req.enableFakeCellOpt = value?1:0;
		req.fakeCellBarTimerS = value;
		req.enableFakeCellOptPresent = 1;
		break;
	case MOBILE_CONF_RESET_TO_FACTORY:
		PsCfgResetAllUeConfig();
		return 0;
	default:
		return -1;
	}
	if (appSetEcCfgSettingSync(&req) != CMS_RET_SUCC)
	{
		return -1;
	}
	return 0;
}

#include "luat_uart.h"
#include "luat_debug.h"
#include "ctype.h"

void luat_mobile_rf_test_input(char *data, uint32_t data_len)
{
	if (data && data_len)
	{
		uint32_t i;
		for(i = 0; i < data_len; i++)
		{
			data[i] = toupper(data[i]);
		}
		soc_mobile_rf_test_input(data, data_len);
	}
	else
	{
		soc_mobile_rf_test_input(NULL, 0);
	}
}

void luat_mobile_rf_test_mode(uint8_t uart_id, uint8_t on_off)
{
	soc_mobile_rf_test_mode(uart_id, on_off);
}

void luat_mobile_get_last_nas_info(uint8_t *type, uint16_t *cause, uint16_t *phy_cellid, uint32_t *freq)
{
	soc_mobile_get_last_nas_event(type, cause, phy_cellid, freq);
}
//VOLTE相关

#if defined FEATURE_IMS_ENABLE

void luat_mobile_get_last_call_num(char *buf, uint8_t buf_len)
{
	char number[80];
	uint32_t len;
	soc_mobile_get_last_cc_event_info(number, &len);
	if (len < buf_len)
	{
		memcpy(buf, number, len);
		buf[len] = 0;
	}
	else
	{
		memcpy(buf, number, buf_len - 1);
		buf[buf_len] = 0;
	}
}

int luat_mobile_make_call(uint8_t sim_id, char *number, uint8_t len)
{
	return soc_mobile_make_call(sim_id, number, len);
}

void luat_mobile_hangup_call(uint8_t sim_id)
{
	soc_mobile_hangup_call(sim_id);
}

int luat_mobile_answer_call(uint8_t sim_id)
{
	return soc_mobile_answer_call(sim_id);
}

int luat_mobile_speech_init(uint8_t multimedia_id,void *callback)
{
	soc_mobile_speech_init(callback);
	return 0;
}

int luat_mobile_speech_upload(uint8_t *data, uint32_t len)
{
	return soc_mobile_speech_upload(data, len);
}
#else
void luat_mobile_get_last_call_num(char *buf, uint8_t buf_len)
{

}

int luat_mobile_make_call(uint8_t sim_id, char *number, uint8_t len)
{
	return -1;
}

void luat_mobile_hangup_call(uint8_t sim_id)
{

}

int luat_mobile_answer_call(uint8_t sim_id)
{
	return -1;
}

int luat_mobile_speech_init(uint8_t multimedia_id,void *callback)
{
	return -1;
}

int luat_mobile_speech_upload(uint8_t *data, uint32_t len)
{
	return -1;
}
#endif

static uint8_t luat_mobile_sync_time_enable = 1;
uint8_t soc_mobile_is_sync_time_enable(void) {return luat_mobile_sync_time_enable;}

void luat_mobile_set_sync_time(uint8_t on_off) {luat_mobile_sync_time_enable = on_off;}
uint8_t luat_mobile_get_sync_time(void) {return luat_mobile_sync_time_enable;}
