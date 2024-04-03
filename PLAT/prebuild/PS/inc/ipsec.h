/******************************************************************************
 ******************************************************************************
 Copyright:      - 2022- Copyrights of AirM2M Ltd.
 File name:      - ipsec.h
 Description:    - ipsec process hdr file
 History:        - 12/01/2022 xwang
 ******************************************************************************
******************************************************************************/

#ifndef TCPIP_IPSEC_H
#define TCPIP_IPSEC_H

#include "commontypedef.h"
#include "lwip/opt.h"
#include "lwip/ip_addr.h"

err_t ipsec_ip_output_if_src(struct pbuf *p, const ip_addr_t *src, const ip_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

err_t ipsec_ip_output_if(struct pbuf *p, const ip_addr_t *src, const ip_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

err_t ipsec_ip4_output_if_src(struct pbuf *p, const ip4_addr_t *src, const ip4_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

err_t ipsec_ip6_output_if_src(struct pbuf *p, const ip6_addr_t *src, const ip6_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

err_t ipsec_ip4_output_if(struct pbuf *p, const ip4_addr_t *src, const ip4_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

err_t ipsec_ip6_output_if(struct pbuf *p, const ip6_addr_t *src, const ip6_addr_t *dest,
             u8_t ttl, u8_t tos,
             u8_t proto, struct netif *netif);

void esp_input(struct pbuf *p, struct netif *inp, ip_addr_t *srcAddr, ip_addr_t *dstAddr);


#endif

