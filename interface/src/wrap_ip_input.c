/*
代理并拦截网络下发的IP包
*/
#include "lwip/opt.h"
#include "common_api.h"
#include "time.h"
#include "osasys.h"
#include "mw_aon_info.h"
#include "slpman.h"

#ifdef __LUATOS__
#include "luat_conf_bsp.h"
#endif

#if defined(LUAT_NET_IP_INTERCEPT) && (LUAT_NET_IP_INTERCEPT == 1)

#define ENABLE_PSIF_IPV6 1

#include "lwip/ip_addr.h"
#include "lwip/ip.h"

#ifdef __LUATOS__
#include "luat_conf_bsp.h"
#endif

err_t __wrap_ps_ip_input(struct pbuf *p, struct netif *inp) {
  u8_t ipVersion;
  if (p != NULL) {
    #ifdef LUAT_USE_ULWIP
    extern err_t ulwip_ip_input_cb(struct pbuf *p, struct netif *inp);
    if (ERR_OK == ulwip_ip_input_cb(p, inp)) {
      pbuf_free(p);   // free the pbuf
      return ERR_OK;
    }
    #endif
    ipVersion = IP_HDR_GET_VERSION(p->payload);
    if (ipVersion == 6) {
#if ENABLE_PSIF_IPV6
        ip6_input(p, inp);
#else
        ECOMM_TRACE(UNILOG_TCPIP_NETADPT, ps_ip_input_1, P_ERROR, 0,
            "ps_ip_input ip version %u not support", ipVersion);
        pbuf_free(p);   // free the pbuf
        return ERR_VAL;
#endif
    }
    else if(ipVersion == 4)
    {
        ip4_input(p, inp);
    }
    else
    {
        ECOMM_TRACE(UNILOG_TCPIP_NETADPT, ps_ip_input_2, P_ERROR, 1,
            "ps_ip_input invalid ip version %u", ipVersion);
        pbuf_free(p);   // free the pbuf
        return ERR_VAL;
    }
  }
  else
  {
    return ERR_VAL;
  }
  return ERR_OK;
}

#endif
