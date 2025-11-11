/**
  **************************************************************************
  * @file     netconf.h
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    This file contains all the functions prototypes for the netconf.c
  *           file.
  **************************************************************************
  *                       Copyright notice & Disclaimer
  *
  * The software Board Support Package (BSP) that is made available to
  * download from Artery official website is the copyrighted work of Artery.
  * Artery authorizes customers to use, copy, and distribute the BSP
  * software and its related documentation for the purpose of design and
  * development in conjunction with Artery microcontrollers. Use of the
  * software is governed by this copyright notice and the following disclaimer.
  *
  * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
  * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
  * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
  * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
  * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
  * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
  *
  **************************************************************************
  */

/* Define to prevent recursive inclusion -------------------------------------*/
#ifndef __NETCONF_H
#define __NETCONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/** @addtogroup AT32F437_periph_examples
  * @{
  */

/** @addtogroup 437_EMAC_telnet
  * @{
  */

/** @addtogroup network_configuration_prototype
  * @{
  */
#include "lwip/ip4_addr.h"
#include "lwip/apps/httpd.h"

typedef enum
{
  NET_DETECT_UIP = 0,
  NET_DETECT_LIKE_LWIP = 1,
  NET_DETECT_LWIP = 2,
} link_t;

void ethnode_config_E(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw);
void ethnode_config(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw);
void tcpip_stack_init_E(struct netif *netif); //, ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw);

/* Includes ------------------------------------------------------------------*/
void stack_init(struct netif *netif);
void lwip_pkt_handle(void);
void time_update(void);
void lwip_periodic_handle(volatile uint32_t localtime);
void lwip_rx_loop_handler(void);

/* Includes ------------------------------------------------------------------*/
struct netif *tcpip_stack_netif_emac(void);
struct netif *tcpip_stack_netif(void);

//void tcpip_stack_init(void);
void stack_tcpip_init_v51(void);
//void stack_set_default_v51(void);
//void stack_set_default_emac(void);

/* Define those to better describe your network interface. */
#define AT_IFNAME0 'a'
#define AT_IFNAME1 't'

/* Define those to better describe your network interface. */
#define DM_IFNAME0 'd'
#define DM_IFNAME1 'm'

#define	NETIF_IS_EMAC(netif) \
	(netif->name[0] == AT_IFNAME0 && netif->name[1] == AT_IFNAME1)

#define	NETIF_IS_V51(netif) \
	(netif->name[0] == DM_IFNAME0 && netif->name[1] == DM_IFNAME1)

extern int netif_v51_ready;

#ifdef __cplusplus
}
#endif

/**
  * @}
  */

/**
  * @}
  */
#endif /* __NETCONF_H */

/**
  * @}
  */

