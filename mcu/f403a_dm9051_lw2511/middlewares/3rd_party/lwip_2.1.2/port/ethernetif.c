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
#include "lwip/opt.h"
#include "lwip/def.h"
#include "lwip/ip4.h"
#include "lwip/ip6.h"
#include "lwip/udp.h"
//#include "lwip/prot/ethernet.h"
#include "lwip/mem.h"
#include "lwip/pbuf.h"
#include "lwip/sys.h"
#include <lwip/stats.h>
#include <lwip/snmp.h>
#include "netif/etharp.h"
#include "netif/ppp/pppoe.h"
#include "err.h"
#include "ethernetif.h"
#include "ethernetif_v51.h" //v.s. "ethernetif.h"
#include "netconf.h"
#include "../../dm9051_u2510_if/platform_info.h" //"all, as main.c including"

#if (EDRIVER_ADDING_PTP && LWIP_PTP)
 #include "ptpd.h"
 #include "dm9051_edriver_extend/dm9051a_ptp.h" /* eth api */
 const uint8_t *DM_ETH_Init_ptpTrans(uint8_t *adr);
 void tx_manager_dispatch_ptpTrans(uint8_t *buffer, uint16_t len, struct pbuf *p);
 uint16_t rx_manager_dispatch_ptpTrans(uint8_t *buf);
 int rx_manager_dispatch_pbuf_postTrans(uint8_t *buf, struct pbuf *p);
 err_t rx_igmp_mac_filter_ptpTrans(struct netif *netif, const ip4_addr_t *group, enum netif_mac_filter_action action);
 void buffer_ts_time(uint8_t *buffer, TimeInternal *pTimeTmp);
void PTPd_Init_manager_setup(void); //.add
#endif

/* Define those to better describe your network interface. */
#define IFNAME0 'a'
#define IFNAME1 't'

#define MAC_ADDR_LENGTH 6

uint8_t eth_buf[RXBUFF_OVERSIZE_LEN];

void network_ptp_init(void)
{
#if LWIP_PTP && EDRIVER_ADDING_PTP
	// Initialize PTPD (only once)
	/* void start_ptpd_run(void) */
	printf("\r\n");
	printf("\r\n");
	printf("[ptpd.init]\r\n");
	PTPd_Init();
	PTPd_Init_manager_setup(); //PTPd_Init_manager_update(); //_dm9051etc.tx_update();
	printf("[ptpd.init].all.done\r\n");
	printf("\r\n");
	printf("\r\n");
#endif
}

/**
 * In this function, the hardware should be initialized.
 * Called from ethernetif_init().
 *
 * @param netif the already initialized lwip network interface structure
 *        for this ethernetif
 */
static void
low_level_init(struct netif *netif)
{
  /* set MAC hardware address length */
  netif->hwaddr_len = ETHARP_HWADDR_LEN;

  /* maximum transfer unit */
  netif->mtu = 1500;

  /* device capabilities */
  /* don't set NETIF_FLAG_ETHARP if this device is not an ethernet one */
  netif->flags |= NETIF_FLAG_BROADCAST | NETIF_FLAG_ETHARP | NETIF_FLAG_LINK_UP;

//  netif->hwaddr[0] =  MACaddr[0];
//  netif->hwaddr[1] =  MACaddr[1];
//  netif->hwaddr[2] =  MACaddr[2];
//  netif->hwaddr[3] =  MACaddr[3];
//  netif->hwaddr[4] =  MACaddr[4];
//  netif->hwaddr[5] =  MACaddr[5];
  /* If NOT called (mask) here is for main.c fo dm9051_conf()/dm9051_init()... */
  memset(netif->hwaddr, 0, MAC_ADDR_LENGTH);
  printf("reg mac %02x %02x %02x %02x %02x %02x\r\n",
	netif->hwaddr[0],
	netif->hwaddr[1],
	netif->hwaddr[2],
	netif->hwaddr[3],
	netif->hwaddr[4],
	netif->hwaddr[5]);
  const uint8_t *pd = DM_ETH_Init_ptpTrans(NULL); //dm_eth_init(); //= DM_ETH_Init_W(/*netif*/ NULL, NULL) ? 1 : 0;
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
}

/**
 * This function should do the actual transmission of the packet. The packet is
 * contained in the pbuf that is passed to the function. This pbuf
 * might be chained.
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @param p the MAC packet to send (e.g. IP packet including MAC addresses and type)
 * @return ERR_OK if the packet could be sent
 *         an err_t value if the packet couldn't be sent
 *
 * @note Returning ERR_MEM here if a DMA queue of your MAC is full can lead to
 *       strange results. You might consider waiting for space in the DMA queue
 *       to become availale since the stack doesn't retry to send a packet
 *       dropped because of memory failure (except for the TCP timers).
 */
void iperf3_low_level_pnt(char *head, int enter_code, uint8_t *buffer, uint16_t l); //'iperf3'

static err_t
low_level_output(struct netif *netif, struct pbuf *p)
{
//. struct ethernetif *ethernetif = netif->state;
	//uint8_t *buffer = get_TransmitBuffer();
	struct pbuf *q;
	int l = 0;

	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t *)&eth_buf[l], q->payload, q->len);
		l = l + q->len;
	}

  iperf3_low_level_pnt("low_level_output", 1, eth_buf, (uint16_t)l);
  tx_manager_dispatch_ptpTrans(eth_buf, (uint16_t)l, p); //ptp_inst.tx_dispatch(buffer, (uint16_t)l, p);
  return ERR_OK;
}

//#include "ptpd.h"
#define MQTT_PUB_PAYLOAD_BUFFER_SIZE					256

static struct pbuf *
low_level_input(struct netif *netif)
{
	//struct ptptime_t arrive_timestamp;
	//uint8_t *buffer = get_ReceiveBuffer();
	uint16_t len = rx_manager_dispatch_ptpTrans(eth_buf);
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

#if (EDRIVER_ADDING_PTP && LWIP_PTP)
	if (rx_manager_dispatch_pbuf_postTrans(eth_buf, p)) {
		printf("ptpd message type: 0x%02x\r\n", p->messageType);
		#if LWIP_MQTT
		if (p->messageType == SYNC) {
			TimeInternal t1;
			TimeInternal t2;
			TimeInternal dif;
			char zpayload[MQTT_PUB_PAYLOAD_BUFFER_SIZE];

			buffer_ts_time(((uint8_t *)p->payload)+14+20+8, &t1);
			t2.seconds = p->time_sec;
			t2.nanoseconds = p->time_nsec;
			subTime(&dif, &t2, &t1);

			piblish_sync_info1(zpayload, &t2, &t1, &dif);
		}
		#endif
	}
#endif
	return p;
}

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

/**
 * Should be called at the beginning of the program to set up the
 * network interface. It calls the function low_level_init() to do the
 * actual setup of the hardware.
 *
 * This function should be passed as a parameter to netif_add().
 *
 * @param netif the lwip network interface structure for this ethernetif
 * @return ERR_OK if the loopif is initialized
 *         ERR_MEM if private data couldn't be allocated
 *         any other err_t on error
 */
err_t
ethernetif_init(struct netif *netif)
{
  struct ethernetif *ethernetif;

  LWIP_ASSERT("netif != NULL", (netif != NULL));

  ethernetif = mem_malloc(sizeof(struct ethernetif));
  if (ethernetif == NULL)
  {
    LWIP_DEBUGF(NETIF_DEBUG, ("ethernetif_init: out of memory\n"));
    return ERR_MEM;
  }

#if LWIP_NETIF_HOSTNAME
  /* Initialize interface hostname */
  netif->hostname = "lwip";
#endif /* LWIP_NETIF_HOSTNAME */

  /*
   * Initialize the snmp variables and counters inside the struct netif.
   * The last argument should be replaced with your link speed, in units
   * of bits per second.
   */
  NETIF_INIT_SNMP(netif, snmp_ifType_ethernet_csmacd, 100000000);

  netif->state = ethernetif;
  netif->name[0] = IFNAME0;
  netif->name[1] = IFNAME1;
  /* We directly use etharp_output() here to save a function call.
   * You can instead declare your own function an call etharp_output()
   * from it if you have to do some checks before sending (e.g. if link
   * is available...) */
  netif->output = etharp_output;
  netif->linkoutput = low_level_output;

  ethernetif->ethaddr = (struct eth_addr *)&(netif->hwaddr[0]);

  /* initialize the hardware */
  low_level_init(netif);
  
#ifdef LWIP_IGMP
printf("...foreach, INIT: netif->flags |= NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST\r\n");
  netif->flags |= NETIF_FLAG_IGMP | NETIF_FLAG_BROADCAST;

  // Set the IGMP MAC filter callback
  netif->igmp_mac_filter = rx_igmp_mac_filter_ptpTrans;
#endif

  return ERR_OK;
}
