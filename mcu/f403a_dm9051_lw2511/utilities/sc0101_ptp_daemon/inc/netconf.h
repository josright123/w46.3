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
#include <stdbool.h>
#include "lwip/ip_addr.h"
#include "lwip/dhcp.h"
#include "lwip/prot/dhcp.h"

/* Private define ------------------------------------------------------------*/
//#define MAC_ADDR_LENGTH                  (6)
//#define ADDR_LENGTH                      (4)
#define OVERSIZE_LEN                     1536
#define RXBUFF_OVERSIZE_LEN              (OVERSIZE_LEN+2)

typedef enum
{
  NET_DETECT_UIP = 0,
  NET_DETECT_LWIP = !NET_DETECT_UIP,
} link_t;

//extern const ip_addr_t netif_IpAddr;
extern const ip_addr_t netif_GwAddr;
extern const ip_addr_t netif_MaskAddr;
//extern const uint8_t local_ip[ADDR_LENGTH];
//extern const uint8_t local_mask[ADDR_LENGTH];
//extern const uint8_t local_gw[ADDR_LENGTH];

/* Includes ------------------------------------------------------------------*/
const char *APP_head0(void);
const char *DHCP_head(void);
const char *DEBUG_head(void);
void tcpip_stack_init(void);
void lwip_pkt_handle(void);
bool lwip_pkt_handle_v51(void);
void time_update(void);
//void lwip_rx_loop_handler(void);

//struct netif *tcpip_stack_netif_v51(void);
//void dm_eth_init(void);
void dm_eth_receive(void);
//void dm_ptpd_ip_run(void);
void lwip_periodic_handle(volatile uint32_t localtime);
//static void lwip_periodic_link(volatile uint32_t localtime, link_t t);
void lwip_periodic_link_hdlr(volatile uint32_t localtime);
void lwip_periodic_systick_time_display(void);
void lwip_periodic_tmr_time_display(void);
//void lwip_periodic_slow(volatile uint32_t localtime);
//void dm_ptpd_periodic(volatile uint32_t localtime);
void dm_ptpd_combined_periodic(uint32_t localtime);

//void set_link(struct netif *netif);
//int ethernetif_set_link(void const *argument);
uint8_t network_dhcp_is_bound_enhanced(void);
int net_ip_bound(void);
int net_ip_done(void); //void periodic_ip_ptpd_run(struct netif *netif);
void network_ptp_init(void);

#if LWIP_DHCP /* don't build if not configured for use in lwipopts.h */
int dhcp_monitor_state(struct dhcp *dhcp, struct netif *netif);
#endif
void dhcpc_configured(void);

int dm_ptpd_mqtt_periodic_stat(void);

#include "ptpd.h" //#include "ptpd_v51.h" //TEMP
void publish_sync_info(char *buf);
void piblish_sync_info1(char *buf, TimeInternal *pts_arrive, TimeInternal *pComeInPkt_t1, TimeInternal *pt2);
	
#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */



