/**
  **************************************************************************
  * @file     netconf.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    network connection configuration
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

/* Includes ------------------------------------------------------------------*/
#include "stdio.h"
#include <string.h>
#include "lwip/tcp.h"
#include "lwip/udp.h"
#include "lwip/init.h"
#include "lwip/timeouts.h"
#include "ethernetif.h"
#include "netconf.h"
#include "at32_emac_eli.h"
#include "ptpd.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "../../dm9051_u2510_if/ip_status.h"
//void ethernetif_notify_conn_changed(struct netif *netif);

//#define MAC_ADDR_LENGTH 6

#define LINK_DETECTION                   1
#define LOCAL_OVER_MAX_DHCP_TRIES        2  //extern uint32_t LOCAL_OVER_MAX_DHCP_TRIES;

void dm9051_config_if(struct netif *set_netif);
void dm_wait2_periodic_stat(void);

volatile uint32_t link_timer = 0;

struct netif netif;

/* Private functions ---------------------------------------------------------*/

uint8_t ethernetif_is_state(void)
{
	struct netif *netif = netif_default;
	return netif_is_link_up(netif);
}

void ethernetif_info(void)
{
	struct netif *netif = netif_default;
	if (netif != NULL) {
		printf("[MQTT] Network interface status:\r\n");
		printf("[MQTT] - Interface up: %s\r\n", netif_is_up(netif) ? "YES" : "NO");
		printf("[MQTT] - Link up: %s\r\n", netif_is_link_up(netif) ? "YES" : "NO");
		printf("[MQTT] - IP: %d.%d.%d.%d\r\n", 
			   ip4_addr1(netif_ip4_addr(netif)), ip4_addr2(netif_ip4_addr(netif)),
			   ip4_addr3(netif_ip4_addr(netif)), ip4_addr4(netif_ip4_addr(netif)));
		printf("[MQTT] - Gateway: %d.%d.%d.%d\r\n", 
			   ip4_addr1(netif_ip4_gw(netif)), ip4_addr2(netif_ip4_gw(netif)),
			   ip4_addr3(netif_ip4_gw(netif)), ip4_addr4(netif_ip4_gw(netif)));
	}
}

const char *dhcp_head[] = {
	"[LWIP_DHCP 0]",
	"[LWIP_DHCP 1]",
};

const char *debug_head[] = {
	"DM_ETH_DEBUG_MODE=0",
	"DM_ETH_DEBUG_MODE=1",
};

const char *DHCP_head(void)
{
#if LWIP_DHCP
	return dhcp_head[1];
#else
	return dhcp_head[0];
#endif
}

const char *DEBUG_head(void)
{
#if DM_ETH_DEBUG_MODE
	return debug_head[1];
#else
	return debug_head[0];
#endif
}

/**
  * @brief  initializes the lwip stack
  * @param  none
  * @retval none
  */
void stack_init(struct netif *netif)
{
    ip_addr_t ipaddr;
    ip_addr_t netmask;
    ip_addr_t gw;

    /* Initialize the LwIP stack */
    lwip_init();

#if LWIP_DHCP
    ipaddr.addr = 0;
    netmask.addr = 0;
    gw.addr = 0;
#else
    IP4_ADDR(&ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
    IP4_ADDR(&netmask, local_mask[0], local_mask[1], local_mask[2], local_mask[3]);
    IP4_ADDR(&gw, local_gw[0], local_gw[1], local_gw[2], local_gw[3]);
#endif

    if(netif_add(netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init, &netif_input) == NULL)
    {
        while(1);
    }

	#if 1
	//do {
	//    uint8_t c_adr[6];
	//    memcpy(c_adr, identified_eth_mac(), MAC_ADDR_LENGTH); //cspi_get_par(c_adr);
	//    printf("reg mac %02x %02x %02x %02x %02x %02x\r\n", c_adr[0], c_adr[1], c_adr[2], c_adr[3], c_adr[4], c_adr[5]);
	//  
	//    /* re-place here is for main.c fo dm9051_conf()/dm9051_init()... */
	//    netif.hwaddr[0] = c_adr[0];
	//    netif.hwaddr[1] = c_adr[1];
	//    netif.hwaddr[2] = c_adr[2];
	//    netif.hwaddr[3] = c_adr[3];
	//    netif.hwaddr[4] = c_adr[4];
	//    netif.hwaddr[5] = c_adr[5];
	//} while(0);
	#endif

    /* Registers the default network interface */
    netif_set_default(netif);

    /* When the netif is fully configured this function must be called */
    netif_set_up(netif);
  
    /* Set the link callback function, this function is called on change of link status */
    netif_set_link_callback(netif, ethernetif_notify_conn_changed); /* ethernetif_update_config */
}

void tcpip_stack_init(void)
{
    printf("\r\n");
    printf("[AT32F403a]\r\n");
    printf("tcpip_stack_init\r\n");

	stack_init(&netif);
	dm9051_config_if(&netif);
}

#if (rt_print | drv_print)
static int fifoTurn_n = 0;

static void diff_rx_pointers_s(uint16_t *pMdra_rds) {
    if (!fifoTurn_n) {
        DM_ETH_Read_mdra(pMdra_rds);
        #if DM_ETH_DEBUG_MODE
        DM_ETH_ToCalc_rx_pointers(0, *pMdra_rds, *pMdra_rds);
        #endif
    }
}

static void diff_rx_pointers_e(uint16_t *pMdra_rds, int n) {
#if DM_ETH_DEBUG_MODE
    static uint16_t premdra_rd = 0x4000;
    uint16_t mdra_rd;

    if (n >= 0) {
        DM_ETH_Read_mdra(&mdra_rd);
        DM_ETH_ToCalc_rx_pointers(1, *pMdra_rds, mdra_rd);

        fifoTurn_n += n;
        if (mdra_rd < premdra_rd && (premdra_rd != 0x4000))
        {
            fifoTurn_n = 0;
        }
        premdra_rd = mdra_rd;
    }
#endif
}
#endif

/**
  * @brief  called when a frame is received
  * @param  none
  * @retval none
  */
bool lwip_pkt_handle_v51(void)
{
    #if (rt_print | drv_print)
    static uint16_t gmdra_rds;
    diff_rx_pointers_s(&gmdra_rds);
    #endif
    
    err_t err = ethernetif_input(&netif);
    if (err == ERR_OK) {
        #if (rt_print | drv_print)
        diff_rx_pointers_e(&gmdra_rds, 1);
        #endif
        return true;
    }
    return false;
}

//extern volatile uint32_t local_time;

static void display_input_mode(int nRx)
{
	int input_mode = hal_active_interrupt_mode(); //=DM_ETH_Init_mode_ptpTrans()
	static int pass6 = 3;

	if (pass6) {
		if (nRx) {
			pass6--;
			printf("Note: %s, %d This tapdev_loop() GET interrupt, only rc %d packet(s).\r\n",
				input_mode == INPUT_MODE_POLL ? "POL" : "INT", pass6, nRx);
		}
	}
}

int DM_ETHER_Receive_Route_W(void) {

	if (dm9051_interrupt_get()) {
		int nRx = 0;

		while(lwip_pkt_handle_v51()) {
#if LWIPERF_APP
			lwip_periodic_handle(0);
#endif
			nRx++;
		}

		display_input_mode(nRx);
		dm9051_interrupt_reset();
		return 1;
	}
	return 0;
}

//void dm_eth_init(void)
//{
//    DM_ETH_Init_W(NULL, NULL);
//}

void dm_eth_receive(void)
{
    /* lwip receive handle */
    DM_ETHER_Receive_Route_W();
}

void lwip_periodic_handle(volatile uint32_t localtime)
{
	DM_NONUSED_ARG(localtime);

	/* lwip timeout */
	sys_check_timeouts();

	//periodic_handle(localtime, &netif);=
#if 0
//    /* TCP periodic process every 250 ms */
//    if (localtime - tcp_timer >= TCP_TMR_INTERVAL || localtime < tcp_timer)
//    {
//        tcp_timer = localtime;
//        tcp_tmr();
//    }
//    /* ARP periodic process every 5s */
//    if (localtime - arp_timer >= ARP_TMR_INTERVAL || localtime < arp_timer)
//    {
//        arp_timer = localtime;
//        etharp_tmr();
//    }
//#if LWIP_DHCP
//    /* Fine DHCP periodic process every 500ms */
//    if (localtime - dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS || localtime < dhcp_fine_timer)
//    {
//        dhcp_fine_timer = localtime;
//        dhcp_fine_tmr();
//    }
//    /* DHCP Coarse periodic process every 60s */
//    if (localtime - dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS || localtime < dhcp_coarse_timer)
//    {
//        dhcp_coarse_timer = localtime;
//        dhcp_coarse_tmr();
//    }
//#endif
#endif
}

static void lwip_periodic_link(volatile uint32_t localtime, link_t t)
{
	switch(t) {
		case NET_DETECT_UIP:
			/* link detection process every 500 ms */
			if (localtime - link_timer >= 500 || localtime < link_timer)
			{
				link_timer = localtime;
#if LWIP_DHCP
				if (dm_eth_polling_downup(1))
#else
				if (dm_eth_polling_downup(0))
#endif
				{ //in "platform_info.c"
					#if LWIP_DHCP
					//dhcpc_renew();... (done, by ethernetif_notify_conn_changed())
					#else
					printf("(up %d.%d.%d.%d) notify\r\n",
						ip4_addr1(&netif.ip_addr), ip4_addr2(&netif.ip_addr),
						ip4_addr3(&netif.ip_addr), ip4_addr4(&netif.ip_addr));
					#endif
				}
			}
			break;
		case NET_DETECT_LWIP:
			/* link detection process every 500 ms */
			#if (LINK_DETECTION > 0)
			//link_lwip_netconf(localtime);
			if (localtime - link_timer >= 500 || localtime < link_timer)
			{
				link_timer = localtime;

				if (ethernetif_set_link(&netif)) //set_link(&netif); //in "at32_emac.c"
					; //if(LWIP_DHCP)...
			}
			#endif
			break;
	}
}

void lwip_periodic_link_hdlr(volatile uint32_t localtime)
{
	#if LWIPERF_APP
	#if 0
	/* pass correct local_time variable and use lwIP detector */
	lwip_periodic_link(localtime, NET_DETECT_LWIP);
	if (!platform_is_link_up())
		dm_wait2_periodic_stat(); //to main.c
	#else
	lwip_periodic_link(localtime, NET_DETECT_UIP);
	if (!netif_is_link_up(&netif))
		dm_wait2_periodic_stat(); //to main.c
	#endif
	#else
	/* lwip_periodic_slow(local_time); */
    /* Combined periodic handling with reduced overhead */
	do {
		static uint32_t last_periodic_time = 0;

		/* Only process periodic tasks if sufficient time has elapsed (reduce CPU load) */
		if ((localtime - last_periodic_time) >= 10 || localtime < last_periodic_time)
		{
			last_periodic_time = localtime;

			/* pass correct local_time variable and use lwIP detector */
			lwip_periodic_link(localtime, NET_DETECT_LWIP);
			if (!platform_is_link_up())
				dm_wait2_periodic_stat(); //to main.c
			lwip_periodic_systick_time_display();
			lwip_periodic_tmr_time_display();
		}
	} while(0);
	#endif
}

//void lwip_periodic_slow(volatile uint32_t localtime)
//{
//    /* Combined periodic handling with reduced overhead */
//    static uint32_t last_periodic_time = 0;

//    /* Only process periodic tasks if sufficient time has elapsed (reduce CPU load) */
//    if ((localtime - last_periodic_time) >= 10 || localtime < last_periodic_time)
//    {
//		last_periodic_time = localtime;

//		_lwip_periodic_link(localtime);
//		_lwip_periodic_systick_time_display();
//		_lwip_periodic_tmr_time_display();
//    }
//}

/**
  * @brief  Check if DHCP is fully bound (enhanced version)
  * @note   Only returns true when DHCP state is BOUND
  *         Used by PTP daemon to determine when network is ready
  * @param  none
  * @retval 1 if DHCP is bound, 0 otherwise
  */
uint8_t network_dhcp_is_bound_enhanced(void)
{
#if LWIP_DHCP
    struct dhcp *dhcp = netif_dhcp_data(&netif);
    if (dhcp && dhcp->state == DHCP_STATE_BOUND)
    {
        return 1; /* DHCP bound - network fully ready */
    }
    return 0; /* DHCP not bound or no DHCP client */
#else
    /* Non-DHCP mode - check if IP is valid */
    return network_ip_is_valid(&netif);
#endif
}

/* by DHCP bound
 */
int net_ip_bound(void)
{
	if (network_dhcp_is_bound_enhanced())
	{
		dhcpc_configured();
		return 1;
	}
	return 0;
}

int net_ip_done(void)
{
#if LWIP_DHCP
    struct dhcp *dhcp = netif_dhcp_data(&netif);
    if (dhcp) {
        //Monitor dhcp state 
		return dhcp_monitor_state(dhcp, &netif);
	}
	return 0;
#else
	return network_ip_is_valid(&netif);
#endif
}

/* ===============================
 * Section: Periodic Network Tasks
 * =============================== */

//void pack_sync_info(char *buf, struct ptptime_t *pts_arrive, TimeInternal *pComeInPkt_t1, TimeInternal *pt2)
//{
//	sprintf(buf, "{ \"TimeStamp\" : \"D (Sync_arrive %d.%09d T2)(Unpack %d.%09d T1) diff %u s %u ns\" }",
//		pts_arrive->tv_sec, pts_arrive->tv_nsec,
//		pComeInPkt_t1->seconds, pComeInPkt_t1->nanoseconds,
//		pt2->seconds, pt2->nanoseconds); // " .%d", ++ssc
//}
void piblish_sync_info1(char *buf, TimeInternal *pts_arrive, TimeInternal *pComeInPkt_t1, TimeInternal *pt2)
{
	sprintf(buf, "{ \"TimeStamp\" : \"D (Sync_arrive %d.%09d T2)(Unpack %d.%09d T1) diff %u s %u ns\" }",
		pts_arrive->seconds, pts_arrive->nanoseconds,
		pComeInPkt_t1->seconds, pComeInPkt_t1->nanoseconds,
		pt2->seconds, pt2->nanoseconds); // " .%d", ++ssc
	publish_sync_info(buf);
}

//static void dhcp_stop_monitor(struct dhcp *dhcp, struct netif *netif)
//{
//}

#if LWIP_DHCP
int dhcp_monitor_state(struct dhcp *dhcp, struct netif *netif)
{
	static uint8_t lastest_state = 0xFF;
	int res = 0;

//    static uint8_t periodic_tries = 0;
//	if (dhcp->tries > periodic_tries) {
//		periodic_tries = dhcp->tries;
//		//printf("DHCP %u tries\r\n", periodic_tries);
//	}
    static uint8_t periodic_tries = 0;
	if (dhcp->tries > periodic_tries) {
		periodic_tries = dhcp->tries;
		printf("DHCP %u tries\r\n", periodic_tries);
	}

	//Monitor dhcp stop: dhcp_stop_monitor(dhcp, netif);
	if ((dhcp->state != DHCP_STATE_BOUND) && (dhcp->tries > LOCAL_OVER_MAX_DHCP_TRIES)) {
		dhcp_stop(netif);
		printf("/* dhcp STOP all\r\n");
		printf(" * DHCP NOT BOUND Found!!\r\n");
		printf(" */\r\n");
		if (1) {
			ip_addr_t ipaddr, netmask, gw;
			IP4_ADDR(&ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
			IP4_ADDR(&netmask, local_mask[0], local_mask[1], local_mask[2], local_mask[3]);
			IP4_ADDR(&gw, local_gw[0], local_gw[1], local_gw[2], local_gw[3]);
			netif_set_addr(netif, &ipaddr, &netmask, &gw);
			printf("/* dhcp STATE all done\r\n");
			printf(" * DHCP tries timeout\r\n");
			printf(" * lwip_periodic_handle(): static ip %d.%d.%d.%d\r\n",
					local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
			printf(" * lwip_periodic_handle(): static gw %d.%d.%d.%d\r\n",
					local_gw[0], local_gw[1], local_gw[2], local_gw[3]);
			printf(" */\r\n");
			res = 1;
		}
	}

    if (dhcp->state != lastest_state)
    {
		lastest_state = dhcp->state;

		switch (dhcp->state)
		{
//		  case DHCP_STATE_OFF:
//			printf("DHCP: State = OFF\r\n");
//			break;
//		  case DHCP_STATE_REQUESTING:
//			printf("DHCP: State = REQUESTING\r\n");
//			break;
//		  case DHCP_STATE_INIT:
//			printf("DHCP: State = INIT\r\n");
//			break;
//		  case DHCP_STATE_REBOOTING:
//			printf("DHCP: State = REBOOTING\r\n");
//			break;
//		  case DHCP_STATE_REBINDING:
//			printf("DHCP: State = REBINDING\r\n");
//			break;
//		  case DHCP_STATE_RENEWING:
//			printf("DHCP: State = RENEWING\r\n");
//			break;
//		  case DHCP_STATE_SELECTING:
//			printf("DHCP: State = SELECTING\r\n");
//			break;
//		  case DHCP_STATE_INFORMING:
//			printf("DHCP: State = INFORMING\r\n");
//			break;
//		  case DHCP_STATE_CHECKING:
//			printf("DHCP: State = CHECKING\r\n");
//			break;
//		  case DHCP_STATE_PERMANENT:
//			printf("DHCP: State = PERMANENT\r\n");
//			break;
		  case DHCP_STATE_BOUND:
//			printf("DHCP: State = BOUND - IP assigned!\r\n");
			printf("DHCP: Assigned IP: %d.%d.%d.%d\r\n", ip4_addr1(&netif->ip_addr), ip4_addr2(&netif->ip_addr),
				ip4_addr3(&netif->ip_addr), ip4_addr4(&netif->ip_addr));
			printf("DHCP: Ethernet GW: %d.%d.%d.%d\r\n",
				ip4_addr1_val(dhcp->offered_gw_addr),ip4_addr2_val(dhcp->offered_gw_addr),
				ip4_addr3_val(dhcp->offered_gw_addr),ip4_addr4_val(dhcp->offered_gw_addr));
			res = 1;
			break;
//		  default:
//			printf("DHCP: State = UNKNOWN (%d)\r\n", dhcp->state);
//			break;
		}
	}
	return res;
}
#endif

static void ap_printf(char *str) {
	printf("%s", str);
}
static void ap_printkey(char *str) {
	printkey("%s", str);
}

#if LWIP_DHCP
void dhcpc_print(uint8_t ap_printag) {
	ap_print_ipconfig("--IP address setting from DHCP-----------",
		identified_tcpip_mac(), //netif.hwaddr, //uip_ethaddr.addr,
		ap_printag & 0x02 ? 
			ap_printf : 
			ap_printkey
		);
}
#endif

void fixed_ip_print(uint8_t ap_printag) {
	ap_print_ipconfig("--Fixed IP address ----------------------",
		identified_tcpip_mac(), //netif.hwaddr, //uip_ethaddr.addr,
		ap_printag & 0x01 ? 
			ap_printf : 
			ap_printkey
		);
}

void dhcpc_configured(void)
{
	static uint8_t ap_printag = 0x3;
#if LWIP_DHCP
	struct dhcp *dhcp = netif_dhcp_data(&netif);
	if (dhcp->state == DHCP_STATE_BOUND) {
		dhcpc_print(ap_printag);
		ap_printag &= ~0x02;
	} else {
		fixed_ip_print(ap_printag);
		ap_printag &= ~0x01;
	}
#else
	fixed_ip_print(ap_printag);
	ap_printag &= ~0x01;
#endif
}
