#include "common_api.h"
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/netif.h"
#include "lwip/ip.h"
#include "lwip/ip6.h"
#include "lwip/ip6_addr.h"
#include "lwip/ip6_frag.h"
#include "lwip/icmp6.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/dhcp6.h"
#include "lwip/nd6.h"
#include "lwip/mld6.h"
#include "lwip/debug.h"
#include "lwip/stats.h"
#include "lwip/sys.h"
#include "lwip/ip4_frag.h"
#include "lwip/inet_chksum.h"
#include "lwip/netif.h"
#include "lwip/icmp.h"
#include "lwip/igmp.h"
#include "lwip/raw.h"
#include "lwip/udp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/autoip.h"
#include "lwip/stats.h"
#include "lwip/prot/dhcp.h"
#include "psifapi.h"
#include "netifadptapi.h"
#include "mw_nvm_config.h"
#include "networkmgr.h"
#define IP4_INPUT_ADPT_PROCESS_INPUT 0
#define IP4_INPUT_ADPT_PROCESS_FWD 1
#define IP4_INPUT_ADPT_PROCESS_UN_REACHABLE 2
#define CMI_PS_INVALID_CID          0xFF
extern struct netif *netif_find_by_ip4_clat_cid(u8_t cid);
//#define LUAT_USE_SDK_XLAT
#ifdef LUAT_USE_SDK_XLAT
__attribute__((weak)) void psDialClatInit(void)
{
	NetMgrClatCfg clatCfg = {0};
    clatCfg.bEnable = true;
    clatCfg.bindIpv6Cid = 1;
    clatCfg.ipv6PrefixLen = 96;
    ip6addr_aton("64:ff9b::", &clatCfg.ipv6Preix);
    ip4addr_aton("192.168.10.5", &clatCfg.ipv4Local);
    ip4addr_aton("8.8.8.8", &clatCfg.ipv4Dns1);
    ip4addr_aton("8.8.4.4", &clatCfg.ipv4Dns2);
    NetMgrClatConfig(&clatCfg);
}
void ps_f1(uint8_t *newIp4Cid, uint8_t newIp6Cid)
{
    MWNvmCfgNetXlatParam clatParamCfg = {0};
    //get clat config
    mwNvmCfgGetNetClatConfig(&clatParamCfg);

    if(clatParamCfg.bEnable)
    {
        if(newIp6Cid != CMI_PS_INVALID_CID && newIp6Cid == clatParamCfg.bindIpv6Cid)
        {
        	*newIp4Cid  = clatParamCfg.bindIpv6Cid;
        }
    }

}
void ps_f2(uint8_t *ip4Cid, uint8_t *ip6Cid, MWNvmCfgNetXlatParam *clatParamCfg)
{
	*ip4Cid = *ip6Cid;
}
__CORE_FUNC_IN_RAM__ uint8_t ipv4_f1(struct pbuf *p, struct netif *inp, struct netif *fwdNetif)
{
    fwdNetif = NetGetBindIp4ClatWanNetifByLanNetif(inp);
    if(fwdNetif)
    {
        ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_input_adpt_process_clat_1, P_VALUE, 2, "ip4_input_adpt_process fwd CLAT WAN name %c%u", fwdNetif->name[0], fwdNetif->name[1]);

        if(NetifIp4PkgFwdClatWanFromLanProcess(p, fwdNetif) == ERR_OK)
        {
            ip6_forwardto(p, fwdNetif);
            return IP4_INPUT_ADPT_PROCESS_FWD;
        }
        else
        {
        	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
        }
    }
    else
    {
    	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
    }
}

__CORE_FUNC_IN_RAM__ struct netif *ipv4_f2(struct netif *inp)
{
	return NetGetBindLanNetifByIp4ClatWanNetif(inp);
}

__CORE_FUNC_IN_RAM__ struct netif *ipv4_f3(struct netif *inp, UINT8 lanNetifType)
{
	return NetGetBindLanNetifByIp4ClatWanNetifAndLanType(inp, lanNetifType);
}
__CORE_FUNC_IN_RAM__ uint8_t ipv4_f4(struct pbuf *p, struct netif *inp, struct netif *fwdNetif, ip4_addr_t *destAddr)
{
    fwdNetif = NetGetBindIp4ClatWanNetifByLanNetif(inp);
    if(fwdNetif)
    {
        ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_input_adpt_process_clat_2, P_VALUE, 2, "ip4_input_adpt_process fwd CLAT WAN name %c%u", fwdNetif->name[0], fwdNetif->name[1]);

        if(NetifDnsRelayLanProcess(inp, fwdNetif, p, destAddr) == TRUE)
        {
            if(NetifIp4PkgFwdClatWanFromLanProcess(p, fwdNetif) == ERR_OK)
            {
                ip6_forwardto(p, fwdNetif);
                return IP4_INPUT_ADPT_PROCESS_FWD;
            }
            else
            {
            	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
            }
        }
        else
        {
        	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
        }
    }
    else
    {
    	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
    }
}

err_t ipv4_f5(struct pbuf *p, const ip4_addr_t *dest, struct netif *netif)
{
  if(netif_check_netif_type_is_wan(netif) && netif->primary_ipv4_cid == LWIP_PS_INVALID_CID && netif->clat_ipv4_cid != LWIP_PS_INVALID_CID )
  {
    struct pbuf *clat_pbuf = NULL;

    ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_clat_1, P_VALUE, 0, "ip4_output_if: call clat netif->output(), do clat output");

    clat_pbuf = pbuf_alloc(PBUF_CLAT, p->tot_len, PBUF_RAM);

    if(clat_pbuf)
    {

        pbuf_copy(clat_pbuf, p);

        if(NetifIp4PkgFwdClatWanFromLanProcess(clat_pbuf, netif) == ERR_OK)
        {
            err_t clat_forward;

            clat_forward = ip6_forwardto(clat_pbuf, netif);

            pbuf_free(clat_pbuf);

            return clat_forward;
        }
        else
        {
            pbuf_free(clat_pbuf);
            ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_clat_2, P_WARNING, 0, "ip4_output_if: call clat netif->output(), do clat output fail");
            return ERR_RTE;
        }
    }
    else
    {
            ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_clat_3, P_WARNING, 0, "ip4_output_if: call clat netif->output(),allocate clat pbuf fail");
            return ERR_RTE;
    }
  }
  else
  {
    #if IP_FRAG
    /* don't fragment if interface has mtu set to 0 [loopif] */
    if (netif->mtu && (p->tot_len > netif->mtu)) {
        return ip4_frag(p, netif, dest);
    }
    #endif /* IP_FRAG */

    #if LWIP_ENABLE_UNILOG
    ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_6, P_VALUE, 0, "ip4_output_if: call netif->output()");
    #else
    LWIP_DEBUGF(IP_DEBUG, ("ip4_output_if: call netif->output()\n"));
    #endif

    #if ENABLE_PSIF
    if(p->tickType == UL_PDU_START_TICK) {
        p->sysTick = sys_now();
        ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_7, P_VALUE, 2, "ip4_output_if: call netif->output(),ticktype %u, systick %u", UL_PDU_START_TICK, p->sysTick);
    }
    #endif

    return netif->output(netif, p, dest);
  }
}

err_t ipv6_f1(struct pbuf *p, struct netif *inp, struct ip6_hdr *ip6hdr, uint8_t *is_end)
{
    if(netif_check_netif_type_is_wan(inp) && inp->primary_ipv6_cid != LWIP_PS_INVALID_CID && inp->clat_ipv4_cid == inp->primary_ipv6_cid)
    {
        ECOMM_TRACE(UNILOG_LWIP_CORE_IP, ip6_input_clat_1, P_VALUE, 0, "ip6_input: clat process");
        *is_end = 1;
        return NetifIp6PkgClatWanProcess(p, inp);
    }
    else
    {
    	*is_end = 0;
        ip6_forward(p, ip6hdr, inp);
        return 0;
    }
}
#else
void psDialClatInit(void) {;}
void ps_f1(uint8_t *newIp4Cid, uint8_t newIp6Cid) {;}
void ps_f2(uint8_t *ip4Cid, uint8_t *ip6Cid, MWNvmCfgNetXlatParam *clatParamCfg) {;}
__CORE_FUNC_IN_RAM__ uint8_t ipv4_f1(struct pbuf *p, struct netif *inp, struct netif *fwdNetif)
{
	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
}

__CORE_FUNC_IN_RAM__ struct netif *ipv4_f2(struct netif *inp) {return NULL;}
__CORE_FUNC_IN_RAM__ struct netif *ipv4_f3(struct netif *inp, UINT8 lanNetifType) {return NULL;}
__CORE_FUNC_IN_RAM__ uint8_t ipv4_f4(struct pbuf *p, struct netif *inp, struct netif *fwdNetif, ip4_addr_t *destAddr)
{
	return IP4_INPUT_ADPT_PROCESS_UN_REACHABLE;
}
err_t ipv4_f5(struct pbuf *p, const ip4_addr_t *dest, struct netif *netif)
{

#if IP_FRAG
  /* don't fragment if interface has mtu set to 0 [loopif] */
  if (netif->mtu && (p->tot_len > netif->mtu)) {
    return ip4_frag(p, netif, dest);
  }
#endif /* IP_FRAG */

#if LWIP_ENABLE_UNILOG
  ECOMM_TRACE_OPT(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_8, P_VALUE, 0, "ip4_output_if: call netif->output()");
#else
  LWIP_DEBUGF(IP_DEBUG, ("ip4_output_if: call netif->output()\n"));
#endif

#if ENABLE_PSIF
  if(p->tickType == UL_PDU_START_TICK) {
    p->sysTick = sys_now();
    ECOMM_TRACE_OPT(UNILOG_LWIP_CORE_IP, ip4_output_if_opt_src_9, P_VALUE, 2, "ip4_output_if: call netif->output(),ticktype %u, systick %u", UL_PDU_START_TICK, p->sysTick);
  }
#endif

  return netif->output(netif, p, dest);

}
err_t ipv6_f1(struct pbuf *p, struct netif *inp, struct ip6_hdr *ip6hdr, uint8_t *is_end)
{
	*is_end = 0;
	ip6_forward(p, ip6hdr, inp);
	return 0;
}
#endif
