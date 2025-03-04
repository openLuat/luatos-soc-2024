
#ifndef SPI4G_ETH_H
#define SPI4G_ETH_H

#include "csdk.h"
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
#include "lwip/prot/dhcp.h"
#include "lwip/prot/udp.h"
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

typedef __PACKED_STRUCT
{
    struct ip_hdr iphd;
    struct udp_hdr udphd;
    struct dhcp_msg data;
} dhcpsPacket_t;

static uint8_t g_dhcpDiscover_received = 0;

static u8_t dhcp_offer[] = {
    0x45, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0x39, 0xa6, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, /* IP header */
    0x00, 0x43, 0x00, 0x44, 0x01, 0x34, 0x00, 0x00,                                                                         /* UDP header */

    0x02,                                                                                                 /* Type == Boot reply */
    0x01, 0x06,                                                                                           /* Hw Ethernet, 6 bytes addrlen */
    0x00,                                                                                                 /* 0 hops */
    0xAA, 0xAA, 0xAA, 0xAA,                                                                               /* Transaction id, will be overwritten */
    0x00, 0x00,                                                                                           /* 0 seconds elapsed */
    0x00, 0x00,                                                                                           /* Flags (unicast) */
    0x00, 0x00, 0x00, 0x00,                                                                               /* Client ip */
    0xc3, 0xaa, 0xbd, 0xc8,                                                                               /* Your IP */
    0xc3, 0xaa, 0xbd, 0xab,                                                                               /* DHCP server ip */
    0x00, 0x00, 0x00, 0x00,                                                                               /* relay agent */
    0x00, 0x23, 0xc1, 0xde, 0xd0, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* MAC addr + padding */

    /* Empty server name and boot file name */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x63, 0x82, 0x53, 0x63,             /* Magic cookie */
    0x35, 0x01, 0x02,                   /* Message type: Offer */
    0x36, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* Server identifier (IP) */
    0x33, 0x04, 0x00, 0x00, 0x78, 0x78, /* Lease time 8 days */
    0x03, 0x04, 0x0a, 0x00, 0x00, 0x01, /* Router IP */
    0x01, 0x04, 0xff, 0xff, 0xff, 0x00, /* Subnet mask */
    0x06, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* DNS IP */
    0xff,                               /* End option */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Padding */
};

static u8_t dhcp_ack[] = {
    0x45, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0x39, 0xa6, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, /* IP header */
    0x00, 0x43, 0x00, 0x44, 0x01, 0x34, 0x00, 0x00,                                                                         /* UDP header */
    0x02,                                                                                                                   /* Bootp reply */
    0x01, 0x06,                                                                                                             /* Hw type Eth, len 6 */
    0x00,                                                                                                                   /* 0 hops */
    0xAA, 0xAA, 0xAA, 0xAA,                                                                                                 /* Transaction id, will be overwritten */
    0x00, 0x00,                                                                                                             /* 0 seconds elapsed */
    0x00, 0x00,                                                                                                             /* Flags (unicast) */
    0x00, 0x00, 0x00, 0x00,                                                                                                 /* Client IP */
    0xc3, 0xaa, 0xbd, 0xc8,                                                                                                 /* Your IP */
    0xc3, 0xaa, 0xbd, 0xab,                                                                                                 /* DHCP server IP */
    0x00, 0x00, 0x00, 0x00,                                                                                                 /* Relay agent */
    0x00, 0x23, 0xc1, 0xde, 0xd0, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                   /* Macaddr + padding */

    /* Empty server name and boot file name */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x63, 0x82, 0x53, 0x63,             /* Magic cookie */
    0x35, 0x01, 0x05,                   /* Dhcp message type ack */
    0x36, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* DHCP server identifier */
    0x33, 0x04, 0x00, 0x00, 0x78, 0x78, /* Lease time 8 days */
    0x03, 0x04, 0x0a, 0x00, 0x00, 0x01, /* Router IP */
    0x01, 0x04, 0xff, 0xff, 0xff, 0x00, /* Netmask */
    0x06, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* DNS IP */
    0xff,                               /* End marker */

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Padding */
};

static u8_t dhcp_nak[] = {
    0x45, 0x00, 0x01, 0x48, 0x00, 0x00, 0x00, 0x00, 0x80, 0x11, 0x39, 0xa6, 0x00, 0x00, 0x00, 0x00, 0xff, 0xff, 0xff, 0xff, /* IP header */
    0x00, 0x43, 0x00, 0x44, 0x01, 0x34, 0x00, 0x00,                                                                         /* UDP header */
    0x02,                                                                                                                   /* Bootp reply */
    0x01, 0x06,                                                                                                             /* Hw type Eth, len 6 */
    0x00,                                                                                                                   /* 0 hops */
    0xAA, 0xAA, 0xAA, 0xAA,                                                                                                 /* Transaction id, will be overwritten */
    0x00, 0x00,                                                                                                             /* 0 seconds elapsed */
    0x00, 0x00,                                                                                                             /* Flags (unicast) */
    0x00, 0x00, 0x00, 0x00,                                                                                                 /* Client IP */
    0xc3, 0xaa, 0xbd, 0xc8,                                                                                                 /* Your IP */
    0xc3, 0xaa, 0xbd, 0xab,                                                                                                 /* DHCP server IP */
    0x00, 0x00, 0x00, 0x00,                                                                                                 /* Relay agent */
    0x00, 0x23, 0xc1, 0xde, 0xd0, 0x0d, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,                   /* Macaddr + padding */

    /* Empty server name and boot file name */
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00,

    0x63, 0x82, 0x53, 0x63,             /* Magic cookie */
    0x35, 0x01, 0x06,                   /* Dhcp message type nak */
    0x36, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* DHCP server identifier */
    0x33, 0x04, 0x00, 0x00, 0x78, 0x78, /* Lease time 8 days */
    0x03, 0x04, 0x0a, 0x00, 0x00, 0x01, /* Router IP */
    0x01, 0x04, 0xff, 0xff, 0xff, 0x00, /* Netmask */
    0x06, 0x04, 0xc0, 0xa8, 0x01, 0x01, /* DNS IP */
    0xff,                               /* End marker */

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* Padding */
};

static inline bool isARPPackage(ethPacket_t *pkt, uint32_t size)
{
    if (__ntohs(pkt->hdr.type) == ETHTYPE_ARP)
    {
        return true;
    }
    return false;
}

static bool isDhcpPackage(ethPacket_t *pkt, uint32_t size)
{
    if (size < 266)
    {
        return false;
    }

    bool isIPpackage = false;
    if (__ntohs(pkt->hdr.type) == ETHTYPE_IP)
    {
        isIPpackage = true;
    }

    if (!isIPpackage)
    {
        return false;
    }

    bool isUDP = false;
    uint8_t *ipData = pkt->data;
    if (ipData[9] == 0x11) // UDP
    {
        isUDP = true;
    }

    if (!isUDP)
    {
        return false;
    }

    uint8_t *udpData = pkt->data + 20;
    bool isUDPPortOK = false;
    if (udpData[0] == 0x00 && udpData[1] == 0x44 && udpData[2] == 0x00 && udpData[3] == 0x43) // port 68->67
    {
        isUDPPortOK = true;
    }

    if (!isUDPPortOK)
    {
        return false;
    }

    struct dhcp_msg *dhcp_pkg = (struct dhcp_msg *)(pkt->data + 20 + 8);
    if (dhcp_pkg->cookie == 0x63538263)
    {
        return true;
    }

    return false;
}

static struct etharp_hdr *ARP_reply(ethPacket_t *in, const uint8_t *dev_mac, struct netif *netif)
{
    static uint8_t idx = 0;
    static struct etharp_hdr reply = {0};

    struct etharp_hdr *arp_recv = (struct etharp_hdr *)(in->data);

    if (netif == NULL)
    {
        return NULL;
    }

    u8_t *ip = (u8_t *)netif_ip4_addr(netif);

    if (memcmp(&arp_recv->dipaddr, ip, 4) == 0)
    {
        return NULL;
    }

    struct etharp_hdr *arp_reply = &reply;

    arp_reply->hwtype = arp_recv->hwtype;
    arp_reply->proto = arp_recv->proto;
    arp_reply->hwlen = arp_recv->hwlen;
    arp_reply->protolen = arp_recv->protolen;
    arp_reply->opcode = __htons(2);
    memcpy(&arp_reply->dipaddr, &arp_recv->sipaddr, 4);
    memcpy(&arp_reply->dhwaddr, &arp_recv->shwaddr, ETH_HWADDR_LEN);
    memcpy(&arp_reply->sipaddr, &arp_recv->dipaddr, 4);

    if (memcmp(&arp_recv->dipaddr, ip, 3) == 0)
    {
        memcpy(&arp_reply->shwaddr, dev_mac, ETH_HWADDR_LEN);
    }
    else
    {
        memcpy(&arp_reply->shwaddr, dev_mac, ETH_HWADDR_LEN);
        ((u8_t *)&(arp_reply->shwaddr))[5] += ++idx;
    }

    return arp_reply;
}

static dhcpsPacket_t *DHCPS_reply(ethPacket_t *pkt, uint32_t size, struct netif *netif)
{
    static dhcpsPacket_t dhcp_reply = {0};

    struct dhcp_msg *dhcp_pkg = (struct dhcp_msg *)(pkt->data + 20 + 8);
    u8_t *options = dhcp_pkg->options;
    uint16_t offset = 0;
    uint16_t offset_max = size - ((uint32_t)options - (uint32_t)dhcp_pkg) - (14 + 20 + 8);
    uint8_t dhcpType = 0;
    ip4_addr_t request_ip = {0};
    LUAT_DEBUG_PRINT("dhcps_reply offset_max 0x%x", offset_max);
    while ((offset < offset_max) && (options[offset] != DHCP_OPTION_END))
    {
        u8_t op = options[offset];
        u8_t len;
        len = options[offset + 1];
        LUAT_DEBUG_PRINT("dhcps_reply op %x len %d", op, len);
        switch (op)
        {
            case (DHCP_OPTION_MESSAGE_TYPE):
            {
                if (len != 1)
                {
                    LUAT_DEBUG_PRINT("dhcps_reply DHCP_OPTION_MESSAGE_TYPE len error");
                }
                dhcpType = options[offset + 2];
                offset += (len + 2);
                break;
            }

            case (DHCP_OPTION_REQUESTED_IP):
            {
                if (len != 4)
                {
                    LUAT_DEBUG_PRINT("dhcps_reply DHCP_OPTION_REQUESTED_IP len error");
                }

                IP4_ADDR(&request_ip, options[offset + 2], options[offset + 3], options[offset + 4], options[offset + 5]);
                LUAT_DEBUG_PRINT("dhcps_reply DHCP_OPTION_REQUESTED_IP %s", ip4addr_ntoa(&request_ip));
                offset += (len + 2);
                break;
            }

            default:
            {
                offset += (len + 2);
                break;
            }
        }
    }
    LUAT_DEBUG_PRINT("dhcps_reply dhcpType %x", dhcpType);
    if (dhcpType == DHCP_DISCOVER)
    {
        if (netif == NULL)
        {
            return NULL;
        }

        uint8_t *dhcp_offer_reply = (uint8_t *)&dhcp_reply.iphd;
        memcpy(dhcp_offer_reply, dhcp_offer, sizeof(dhcp_offer));
        LUAT_DEBUG_PRINT("DHCP reply OFFER");

        struct dhcp_msg *dhcp_pkg_reply = &dhcp_reply.data;
        dhcp_pkg_reply->xid = dhcp_pkg->xid;
        memcpy(dhcp_pkg_reply->chaddr, dhcp_pkg->chaddr, DHCP_CHADDR_LEN);

        ip4_addr_t GATE_ip = {0};
        IP4_ADDR(&GATE_ip, 192, 168, 1, 1);

        ip_addr_t DHCP_server_ip = {0};
        IP_ADDR4(&DHCP_server_ip, 192, 168, 1, 1);
        ip4_addr_t *pDHCP_server_ip4 = ip_2_ip4(&DHCP_server_ip);
        memcpy(&(dhcp_pkg_reply->siaddr), pDHCP_server_ip4, sizeof(ip4_addr_p_t));

        memcpy(dhcp_pkg_reply->options + 3 + 6 + 6 + 2, &GATE_ip, sizeof(ip4_addr_p_t));
        if (*((uint8_t *)(&GATE_ip) + 3) != 0x01)
            *(dhcp_pkg_reply->options + 3 + 6 + 6 + 2 + 3) = 0x01;
        else
            *(dhcp_pkg_reply->options + 3 + 6 + 6 + 2 + 3) = 0x02;

        ip_addr_t DHCP_dns_server_ip = {0};
        // const ip_addr_t *dns1 = NULL, *dns2 = NULL;
        // dns1 = dns_getserver(0);
        // dns2 = dns_getserver(1);
        // if (!ip_addr_isany(dns1))
        //     ip_addr_copy(DHCP_dns_server_ip, *dns1);
        // else if (!ip_addr_isany(dns2))
        //     ip_addr_copy(DHCP_dns_server_ip, *dns2);
        // else
            IP_ADDR4(&DHCP_dns_server_ip, 114, 114, 114, 114);

        ip4_addr_t *pDHCP_dns_server_ip4 = ip_2_ip4(&DHCP_dns_server_ip);
        memcpy(dhcp_pkg_reply->options + 3 + 6 + 6 + 6 + 6 + 2, pDHCP_dns_server_ip4, sizeof(ip4_addr_p_t));

        struct pbuf *p, *q;
        int totalLen = sizeof(dhcp_offer);
        uint8_t *pData = dhcp_offer_reply;
        int offset = 0;
        p = (struct pbuf *)pbuf_alloc(PBUF_RAW, totalLen, PBUF_POOL);
        if (p != NULL)
        {
            if (totalLen > p->len)
            {
                for (q = p; totalLen > q->len; q = q->next)
                {
                    memcpy(q->payload, &(pData[offset]), q->len);
                    totalLen -= q->len;
                    offset += q->len;
                }
                if (totalLen != 0)
                {
                    memcpy(q->payload, &(pData[offset]), totalLen);
                }
            }
            else
            {
                memcpy(p->payload, &(pData[offset]), totalLen);
            }
            struct udp_hdr *udp_hdr_reply = (struct udp_hdr *)(dhcp_offer_reply + 20);
            ip_addr_t src_ip = {0};
            IP_ADDR4(&src_ip, 0, 0, 0, 0);
            ip_addr_t dst_ip = {0};
            IP_ADDR4(&dst_ip, 255, 255, 255, 255);
            udp_hdr_reply->chksum = ip_chksum_pseudo(p, IP_PROTO_UDP, p->tot_len - 20, &src_ip, &dst_ip);
            LUAT_DEBUG_PRINT("UDP chksum 0x%x", udp_hdr_reply->chksum);
            pbuf_free(p);
        }

        g_dhcpDiscover_received = 1;

        return &dhcp_reply;
    }

    if (dhcpType == DHCP_REQUEST)
    {
        if (netif == NULL)
        {
            return NULL;
        }

        uint8_t *dhcp_offer_reply = (uint8_t *)&dhcp_reply.iphd;

        ip4_addr_t GATE_ip = {0};
        IP4_ADDR(&GATE_ip, 192, 168, 1, 1);

        ip4_addr_t your_ip = {0};
        IP4_ADDR(&your_ip, 192, 168, 1, 2);

        if (g_dhcpDiscover_received == 0 && (your_ip.addr != request_ip.addr))
        {
            memcpy(dhcp_offer_reply, dhcp_nak, sizeof(dhcp_nak));
            LUAT_DEBUG_PRINT("DHCP reply NAK");
        }
        else
        {
            memcpy(dhcp_offer_reply, dhcp_ack, sizeof(dhcp_ack));
            LUAT_DEBUG_PRINT("DHCP reply ACK");
            g_dhcpDiscover_received = 0;
        }
        struct dhcp_msg *dhcp_pkg_reply = &dhcp_reply.data;
        dhcp_pkg_reply->xid = dhcp_pkg->xid;
        memcpy(dhcp_pkg_reply->chaddr, dhcp_pkg->chaddr, DHCP_CHADDR_LEN);
        memcpy(&(dhcp_pkg_reply->yiaddr), &your_ip, sizeof(ip4_addr_p_t));

        ip_addr_t DHCP_server_ip = {0};
        IP_ADDR4(&DHCP_server_ip, 192, 168, 1, 1);
        ip4_addr_t *pDHCP_server_ip4 = ip_2_ip4(&DHCP_server_ip);
        memcpy(&(dhcp_pkg_reply->siaddr), pDHCP_server_ip4, sizeof(ip4_addr_p_t));
        memcpy(dhcp_pkg_reply->options + 3 + 6 + 6 + 2, &GATE_ip, sizeof(ip4_addr_p_t));
        if (*((uint8_t *)(&GATE_ip) + 3) != 0x01)
            *(dhcp_pkg_reply->options + 3 + 6 + 6 + 2 + 3) = 0x01;
        else
            *(dhcp_pkg_reply->options + 3 + 6 + 6 + 2 + 3) = 0x02;

        ip_addr_t DHCP_dns_server_ip = {0};
        // const ip_addr_t *dns1 = NULL, *dns2 = NULL;
        // dns1 = dns_getserver(0);
        // dns2 = dns_getserver(1);
        // if (!ip_addr_isany(dns1))
        //     ip_addr_copy(DHCP_dns_server_ip, *dns1);
        // else if (!ip_addr_isany(dns2))
        //     ip_addr_copy(DHCP_dns_server_ip, *dns2);
        // else
            IP_ADDR4(&DHCP_dns_server_ip, 114, 114, 114, 114);
        ip4_addr_t *pDHCP_dns_server_ip4 = ip_2_ip4(&DHCP_dns_server_ip);
        memcpy(dhcp_pkg_reply->options + 3 + 6 + 6 + 6 + 6 + 2, pDHCP_dns_server_ip4, sizeof(ip4_addr_p_t));

        struct pbuf *p, *q;
        int totalLen = sizeof(dhcp_ack);
        uint8_t *pData = dhcp_offer_reply;
        int offset = 0;
        p = (struct pbuf *)pbuf_alloc(PBUF_RAW, totalLen, PBUF_POOL);
        if (p != NULL)
        {
            if (totalLen > p->len)
            {
                for (q = p; totalLen > q->len; q = q->next)
                {
                    memcpy(q->payload, &(pData[offset]), q->len);
                    totalLen -= q->len;
                    offset += q->len;
                }
                if (totalLen != 0)
                {
                    memcpy(q->payload, &(pData[offset]), totalLen);
                }
            }
            else
            {
                memcpy(p->payload, &(pData[offset]), totalLen);
            }
            struct udp_hdr *udp_hdr_reply = (struct udp_hdr *)(dhcp_offer_reply + 20);
            ip_addr_t src_ip = {0};
            IP_ADDR4(&src_ip, 0, 0, 0, 0);
            ip_addr_t dst_ip = {0};
            IP_ADDR4(&dst_ip, 255, 255, 255, 255);
            udp_hdr_reply->chksum = ip_chksum_pseudo(p, IP_PROTO_UDP, p->tot_len - 20, &src_ip, &dst_ip);
            LUAT_DEBUG_PRINT("UDP chksum 0x%x", udp_hdr_reply->chksum);
            pbuf_free(p);
        }

        return &dhcp_reply;
    }

    return NULL;
}

#endif
