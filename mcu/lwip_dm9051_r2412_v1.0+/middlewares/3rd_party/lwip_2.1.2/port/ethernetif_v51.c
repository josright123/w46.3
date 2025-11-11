/**
 * @file
 * Ethernet Interface Skeleton
 *
 */
/*
 * Copyright (c) 2001-2004 Swedish Institute of Computer Science.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT
 * SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT
 * OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 * IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY
 * OF SUCH DAMAGE.
 *
 * This file is part of the lwIP TCP/IP stack.
 *
 * Author: Adam Dunkels <adam@sics.se>
 *
 */

/*
 * This file is a skeleton for developing Ethernet network interface
 * drivers for lwIP. Add code to the low_level functions and do a
 * search-and-replace for the word "ethernetif" to replace it with
 * something that better describes your network interface.
 */
#include <string.h>
#include <stdbool.h>
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"
#include "lwip/tcpip.h"
//#include "lwip_ethif/nosys/ethernetif_types.h"
#include "lwip/netif.h" //#include "lwip/err.h"
#include "netconf.h"
#include "ethernetif_v51.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"

void DM_Eth_Show_status_W(char *head, uint8_t *statdat, int force);
void DM_Eth_Read_Info(uint8_t *stat);
int DM_Eth_Info_Linkup(uint8_t *stat);
const uint8_t *DM_ETH_Init_W(const uint8_t *adr);
uint16_t rx_manager_dispatch_w(uint8_t *buf);
void tx_manager_dispatch_w(uint8_t *buffer, uint16_t len);

const uint8_t *lwip_set_MACaddr;

void low_level_init(struct netif *netif)
{
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags = NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;
	
	if (hal_active_interrupt_mode()) {
		//#ifdef ETHERNET_INTERRUPT_MODE
		//printf("[interrupt].s\r\n");
		//#else
	} else {
		printf("[polling].s\r\n");
		//#endif
	}

  //LOCK_TCPIP_CORE();
	const uint8_t *pd = DM_ETH_Init_W(NULL); //DM_ETH_Init_W(lwip_set_MACaddr); //_dm9051_boards_initialize(), pd = _dm9051_init()
	if (pd) {
		// memcpy(netif->hwaddr, pd, sizeof(netif->hwaddr));
		netif->hwaddr[0] = pd[0];
		netif->hwaddr[1] = pd[1];
		netif->hwaddr[2] = pd[2];
		netif->hwaddr[3] = pd[3];
		netif->hwaddr[4] = pd[4];
		netif->hwaddr[5] = pd[5];
	}
	printf("reg mac %02x %02x %02x %02x %02x %02x\r\n",
		netif->hwaddr[0],
		netif->hwaddr[1],
		netif->hwaddr[2],
		netif->hwaddr[3],
		netif->hwaddr[4],
		netif->hwaddr[5]);

	netif_v51_ready = pd ? 1 : 0;
  //UNLOCK_TCPIP_CORE();
}

err_t low_level_output(struct netif *netif, struct pbuf *p)
{
#if 0
  struct ethernetif *ethernetif = netif->state;
  DM_ETH_Output_NW(p, ethernetif);
  return ERR_OK;
#endif
//. struct ethernetif *ethernetif = netif->state;
	//uint8_t *buffer = get_TransmitBuffer();
	struct pbuf *q;
	int l = 0;
	uint8_t *eth_buf = get_TransmitBuffer();

	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t *)&eth_buf[l], q->payload, q->len);
		l = l + q->len;
	}

  //iperf3_low_level_pnt("low_level_output", 1, eth_buf, (uint16_t)l);
  //tx_manager_dispatch_ptpTrans(eth_buf, (uint16_t)l, p); //ptp_inst.tx_dispatch(buffer, (uint16_t)l, p);
  tx_manager_dispatch_w(eth_buf, (uint16_t)l);
  return ERR_OK;
}

struct pbuf *low_level_input(struct netif *netif)
{
  uint8_t *eth_buf = get_ReceiveBuffer();
  uint16_t len = rx_manager_dispatch_w(eth_buf);
  if (!len)
	return NULL;

	/* We allocate a pbuf chain of pbufs from the pool. */
	struct pbuf *p, *q;
	int l = 0;
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p != NULL)
	{
		for (q = p; q != NULL; q = q->next)
		{
			memcpy((u8_t *)q->payload, (u8_t *)&eth_buf[l], q->len);
			l = l + q->len;
		}
	}
//  p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
  return p;
}

#if RXHDL_V51 == !RXHDL_OLD
/* Define those to better describe your network interface. */
//#define DM_IFNAME0 'd'
//#define DM_IFNAME1 'm'

//#include "platform_info/nosys/nosys_control/conf_ap.h"
//#include "lwip_ethif/nosys/ethif.h"
//#include "platform_info/nosys/nosys_control/dm9051_ap_debug.h"
/* eth api */
//#include "platform_info/nosys/uip_eth/types_define_all.h"
//#include "platform_info/nosys/uip_eth/eth_api.h"

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function _low_level_input() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
err_t
ethernetif_input(struct netif *netif)
{
  err_t err;
  struct pbuf *p;

  p = low_level_input(netif);

  /* no packet could be read, silently ignore this */
  if (p == NULL) return ERR_MEM;

  err = netif->input(p, netif);
  if (err != ERR_OK) {
		LWIP_DEBUGF(NETIF_DEBUG, ("_ethernetif_input: IP input error\n"));
		pbuf_free(p);
  }

  return err;
}

//err_t
//ethernetif_init(struct netif *netif)
//{
//  struct ethernetif *ethernetif;
//  ...
//}
#endif

void polling_end_info(struct netif *netif, uint8_t *stat)
{
#if 0
  printf("ChipID %04x -> Link %s 'netif_set_link_down' nsr.d6 %02x %c\r\n",
		stat[2] << 8 | stat[3],
		(stat[1] & 0x40) ? "up" : "down",
		stat[1],
		(LINK_STATE_SOURCE == DM9051_CHECK_MAC) ? '*' : ' ');
  printf("ChipID %04x -> Link %s 'netif_set_link_down' bmsr.d2 %04x %c\r\n",
		stat[2] << 8 | stat[3],
		(stat[5] & 0x04) ? "up" : "down",
		stat[4] << 8 | stat[5],
		(LINK_STATE_SOURCE == DM9051_CHECK_PHY) ? '*' : ' '
		); 
#endif
  DM_Eth_Show_status_W("Eth Status", stat, 1); //but force
}

void low_level_status(uint8_t *stat)
{
	DM_Eth_Read_Info(stat); //DM_Eth_ReadRegsInfo_W(stat); //(netif_is_link_up(netif));
}

//int link_startup_update_flg = 0;
//int link_change_flg;

uint16_t ethernetif_link_polling(struct netif *netif, uint8_t *stat)
{
//#if LINK_STATE_SOURCE == DM9051_CHECK_MAC
//  uint16_t link = stat[1] & 0x40 ? 1 : 0;
//#endif
//#if LINK_STATE_SOURCE == DM9051_CHECK_PHY
//  uint16_t link = stat[5] & 0x04 ? 1 : 0; //'id_bmsr & 0x0004 '
//#endif
  uint16_t link = (uint16_t) DM_Eth_Info_Linkup(stat);

  #if 1
	//link_change_flg = 0;
	/* Check whether the netif link down and the PHY link is up */
	if(!netif_is_link_up(netif) && (link))
	{
	  /* network cable is connected */ 
//	  printf("Link 'netif_set_link_up'\r\n");
	  netif_set_link_up(netif);
	  //.check_link_up_can_fix(netif, id_bmsr);
	  //link_startup_update_flg = 1;
	  //link_change_flg = 1;
	  polling_end_info(netif, stat);
	}
	else if(netif_is_link_up(netif) && (!link))
	{
	  /* network cable is dis-connected */
	  netif_set_link_down(netif);
	  //link_startup_update_flg = 1;
	  //link_change_flg = 1;
	  polling_end_info(netif, stat);
	}
  #endif
  return link;
}

void ethernetif_polling_downup(struct netif *netif)
{
	uint8_t statdat[6];
	low_level_status(statdat);
	ethernetif_link_polling(netif, statdat);
}
