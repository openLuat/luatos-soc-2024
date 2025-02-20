
#ifndef SPI4G_ETH_H
#define SPI4G_ETH_H

#include "cmsis_compiler.h"
#include "lwip/ip_addr.h"
#include "lwip/ip4.h"
#include "lwip/ip6_addr.h"
#include "lwip/ip6_addr.h"
#include "lwip/prot/etharp.h"
#include "lwip/prot/ethernet.h"
#include "lwip/prot/nd6.h"
#include "lwip/prot/icmp6.h"
#include "lwip/prot/ip.h"
#include "lwip/prot/ip6.h"
#include "lwip/prot/nd6.h"
#include "lwip/inet_chksum.h"
#include "lwip/dns.h"
#include <stdbool.h>
#include <stdint.h>
#include <machine/endian.h>

typedef __PACKED_STRUCT
{
    struct eth_hdr hdr;
    uint8_t data[0];
} ethPacket_t;

static inline bool isARPPackage(ethPacket_t *pkt, uint32_t size)
{
    if (__ntohs(pkt->hdr.type) == ETHTYPE_ARP)
    {
        return true;
    }
    return false;
}

static inline bool isNeighborSolicitationPackage(ethPacket_t *pkt, uint32_t size)
{
    bool isIPV6package = false;
    if (__ntohs(pkt->hdr.type) == ETHTYPE_IPV6)
    {
        isIPV6package = true;
    }
    if (!isIPV6package)
        return false;

    bool isV6 = false;
    bool isICMP6 = false;
    uint8_t *ipData = pkt->data;
    if (IP_HDR_GET_VERSION(ipData) == 6) //V6
    {
        isV6 = true;
    }
    if (ipData[6] == 0x3a)
    {
        isICMP6 = true;
    }

    if (!isV6)
        return false;
    if (!isICMP6)
        return false;

    struct icmp6_hdr *icmp_pkg = (struct icmp6_hdr *)(ipData + 40);
    if (icmp_pkg->type == ICMP6_TYPE_NS)
    {
        return true;
    }
    return false;
}

static struct etharp_hdr *ARP_reply(ethPacket_t *in, const uint8_t *dev_mac)
{
    static uint8_t idx = 0;
    static struct etharp_hdr rely;

    struct etharp_hdr *arp_recv = (struct etharp_hdr *)(in->data);

    struct netif *netinfo = netif_find_by_ip4_cid(1);
    u8_t *ip = (u8_t *)&netinfo->ip_addr;

    if (memcmp(&arp_recv->dipaddr, ip, 4) == 0)
    {
        return NULL;
    }

    struct etharp_hdr *arp_rely = &rely;

    arp_rely->hwtype = arp_recv->hwtype;
    arp_rely->proto = arp_recv->proto;
    arp_rely->hwlen = arp_recv->hwlen;
    arp_rely->protolen = arp_recv->protolen;
    arp_rely->opcode = __htons(2);
    memcpy(&arp_rely->dipaddr, &arp_recv->sipaddr, 4);
    memcpy(&arp_rely->dhwaddr, &arp_recv->shwaddr, ETH_HWADDR_LEN);
    memcpy(&arp_rely->sipaddr, &arp_recv->dipaddr, 4);

    if (memcmp(&arp_recv->dipaddr, ip, 3) == 0)
    {
        memcpy(&arp_rely->shwaddr, dev_mac, ETH_HWADDR_LEN);
    }
    else
    {
        memcpy(&arp_rely->shwaddr, dev_mac, ETH_HWADDR_LEN);
        ((u8_t *)&(arp_rely->shwaddr))[5] += ++idx;
    }

    return arp_rely;
}

#endif
