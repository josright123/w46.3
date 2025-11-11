/*
 * Copyright (c) 2001, Adam Dunkels.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *      This product includes software developed by Adam Dunkels.
 * 4. The name of the author may not be used to endorse or promote
 *    products derived from this software without specific prior
 *    written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS
 * OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 * NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * This file is part of the uIP TCP/IP stack.
 *
 * $Id: main.c,v 1.16 2006/06/11 21:55:03 adam Exp $
 *
 */
/*------------------------------------------------------------*/
// main.c
// Modified by Spenser  2013/11
// Platform: M08G16 Cortex-M0
// Builder: Keil 4
/*------------------------------------------------------------*/
#include "stdio.h"

// FreeRTOS includes 
#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"

// uIP includes
#include "uip.h"
#include "uip_arp.h"
#include "tapdev.h"
#include "dhcpc.h"
#include "timer.h"

#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "../../dm9051_u2510_if/irq_status.h"
#include "../../dm9051_u2510_if/ip_status.h"

/* devif-api */
void DM_ETH_Init(const uint8_t *adr);
int DM_ETH_Init_mode(void);
void tx_manager_dispatch(uint8_t *buf, uint16_t len);
uint16_t rx_manager_dispatch(uint8_t *buf);
int input_mode;
//[version_1]

/*
 * should be called only in dm9051_beta.c
 */
uint16_t dm9051_isr_enab(void);

//[version_1.ok]
#define	tapdev_init									DM_ETH_Init
//#define tapdev_init(adr)								/*DM_ETH_IRQInit();*/ \
//														input_mode = dm9051_conf(); /*dm9051_boards_initialize();*/ \
//														dm9051_init(adr)

//#define	DM9051_tx									DM_ETH_Output
//#define tapdev_send(buf,len)							DM9051_tx(buf,len)=
//#define tapdev_send(buf,len)							dm9051_tx(buf,len)
#define tapdev_send										tx_manager_dispatch

//#define	DM9051_rx									DM_ETH_Input
//#define tapdev_read(buf)								DM9051_rx(buf)
//#define tapdev_read(buf)								dm9051_rx(buf)
#define tapdev_read										rx_manager_dispatch

//#define tapdev_get_ievent()					DM_ETH_IRQEvent()
#define tapdev_get_ievent()					dm9051_interrupt_get()

//#define tapdev_clr_ievent()					DM_ETH_ToRst_ISR()
//#define tapdev_clr_ievent()					dm9051_isr_enab()
#define tapdev_clr_ievent()					dm9051_interrupt_reset()

//#define tapdev_ip_configure(i,g,m)	DM_ETH_IpConfiguration(i,g,m)
//#define	tapdev_set_ip(ip)				DM_ETH_Ip_Configuration(ip)
//#define	tapdev_set_gw(ip)				DM_ETH_Gw_Configuration(ip)
//#define	tapdev_set_mask(ip)				DM_ETH_Mask_Configuration(ip)

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])

#ifndef NULL
    #define NULL (void *)0
#endif /* NULL */

#ifdef __DHCPC_H__
    struct timer dhcp_timer;
#endif

uint32_t downupcount = 0, dhcpccount = 0;

static struct timer periodic_timer, arp_timer;

/*---------------------------------------------------------------------------*/
//#ifdef __DHCPC_H__
//int identified_dhcpc_en1(void)
//{
//#ifdef __DHCPC_H__
//	return 1;
//#else
//	return 0;
//#endif
//}
static void uip_update_ip_config(const uint8_t *ip, const uint8_t *gw, const uint8_t *mask)
{
//	if (!ip && !gw && !mask) {
//		uint8_t fxip[4] = {192, 168, 1, 17};
//		uint8_t fxgw[4] = {192, 168, 1, 1};
//		uint8_t fxmk[4] = {255, 255, 255, 0};
//		uip_sethostaddr(fxip); //192, 168, 1, 17
//		uip_setdraddr(fxgw);
//		uip_setnetmask(fxmk);
//		
//		DM_ETH_Ip_Configuration(fxip);
//		DM_ETH_Gw_Configuration(fxgw);
//		DM_ETH_Mask_Configuration(fxmk);
//		return;
//	}
	
    // If any parameter is NULL, use default configuration
//	const uint8_t *ipn = ip ? ip : identified_dhcpc_en1() ? null_eth_ip() : candidate_eth_ip();
//	const uint8_t *gwn = gw ? gw : identified_dhcpc_en1() ? null_eth_ip() : candidate_eth_gw();
//	const uint8_t *maskn = mask ? mask : candidate_eth_mask();
	const uint8_t *ipn = ip ? ip : local_ip;
	const uint8_t *gwn = gw ? gw : local_gw;
	const uint8_t *maskn = mask ? mask : local_mask;

    uip_sethostaddr(ipn);
    uip_setdraddr(gwn);
    uip_setnetmask(maskn);
}
//#endif

#ifdef __DHCPC_H__
static int dbg_expire(void)
{
	clock_time_t now = clock_time();
	if (dbg_timer_expired(&dhcp_timer, now))
	{
		printkey("\r\n");
		uip_update_ip_config(NULL, NULL, NULL); //[dhcpc expire] //uip_update_ip_config(NULL, NULL, NULL); //[TESTING.]
		printf("dhcpc expire (CLOCK_SECOND * 600) - now %lu tm_sta %lu, diff %lu, intval %lu\r\n", 
			now, dhcp_timer.start, now - dhcp_timer.start, dhcp_timer.interval);
		return 1;
	}
	return 0;
}

void printf_dhcp_dbg(char *head, uint32_t op_count, uint32_t now)
{
	//DM_NONUSED_ARG(head);
	DM_NONUSED_ARG(now);
	printf("--. %s, times %lu\r\n", head, op_count);

	#if 0
	printf("--. dhcp_time: start heartbeat %lu, now %lu, elapsed %lu elaps-expire %lu\r\n",
				dhcp_timer.start, now, 
				now - dhcp_timer.start,
				dhcp_timer.interval);
	printf("--.\r\n");
	#endif
}
#endif

#if defined(DMPLUG_INT) || 1
//uint16_t isrSemaphore_src;
#endif

static int input_packet(void)
{
	uip_len = tapdev_read(uip_buf);
#if 0 //'DM_ETH_DEBUG_MODE'
	dump_data(uip_buf, uip_len)); //dm_eth_input_hexdump(uip_buf, uip_len);
#endif
	return (uip_len > 0) ? 1 : 0;
}

#if !defined(DMPLUG_INT) || 1
static void handle_packet(void) {			
	if (BUF->type == htons(UIP_ETHTYPE_IP))
	{
		uip_input();        // uip_process(UIP_DATA)

		/* If the above function invocation resulted in data that
		should be sent out on the network, the global variable
		uip_len is set to a value > 0. */
		if (uip_len > 0)
		{
			uip_arp_out();
			tapdev_send(uip_buf, uip_len);
		}
	}
	else if (BUF->type == htons(UIP_ETHTYPE_ARP))
	{
		uip_arp_arpin();

		/* If the above function invocation resulted in data that
			 should be sent out on the network, the global variable
			 uip_len is set to a value > 0. */
		if (uip_len > 0)
		{
			tapdev_send(uip_buf, uip_len);
		}
	}
}
#endif

#if !defined(DMPLUG_INT) || 1
uint16_t DM_ETH_RXHandler_Poll(void)
{
	if (input_packet()) /* Polling, per 1 packet */
		handle_packet();
	return uip_len;
}
/*---------------------------------------------------------------------------*/
#endif

static void display_input_mode(int nRx)
{
	static int pass6 = 3;

	if (pass6) {
		if (nRx) {
			pass6--;
			printf("Note: %s, %d This tapdev_loop() GET interrupt, only rc %d packet(s).\r\n",
				input_mode == INPUT_MODE_POLL ? "POL" : "INT", pass6, nRx);
		}
	}
}

int DM_ETHER_Receive_Route_W(void)
{
	if (tapdev_get_ievent()) { //= dm9051_interrupt_get()
		//isrSemaphore_src = 0x5555 >> 8;

		inc_interrupt_count();
		do { //[nRx = net_pkts_handle_intr(tcpip_stack_netif());]
			int nRx = 0;

			while (1) {
				diff_rx_s(); //diff_rx_pointers_s(&mdra_rds);
				if (input_packet()) { /* Interrupt, exhaust every exist packet */
				
					if (BUF->type == htons(UIP_ETHTYPE_IP))
					{
						uip_input();        // uip_process(UIP_DATA)

						/* If the above function invocation resulted in data that
						should be sent out on the network, the global variable
						uip_len is set to a value > 0. */
						if (uip_len > 0)
						{
							uip_arp_out();
							tapdev_send(uip_buf, uip_len);
						}
					}
					else if (BUF->type == htons(UIP_ETHTYPE_ARP))
					{
						uip_arp_arpin();

						/* If the above function invocation resulted in data that
							 should be sent out on the network, the global variable
							 uip_len is set to a value > 0. */
						if (uip_len > 0)
						{
							tapdev_send(uip_buf, uip_len);
						}
					}
					
					nRx++;
					
					#if 1
					diff_rx_e(); //diff_rx_pointers_e(1, &mdra_rds);
					#endif
				} else
					break;
			}

			display_input_mode(nRx);
			tapdev_clr_ievent(); //dm9051_interrupt_reset();
		} while(0);
		return 1;
	}
	return 0;
}

void vuIP_Init(void)
{
#ifndef __DHCPC_H__
//    uip_ipaddr_t ip, gw, mask; //ipaddr={0,0};
	u16_t ipaddr[2];
#endif

    /* FreeRTOS  task delay */
    timer_set(&periodic_timer, CLOCK_SECOND / 2); 		//500ms
    timer_set(&arp_timer, CLOCK_SECOND * 10);         // 10sec
	
    tapdev_init(&uip_ethaddr.addr[0]); //DM_ETH_Init(&uip_ethaddr.addr[0]); //DM_Eth_Open();
	input_mode = DM_ETH_Init_mode();
#if 1
//    dm_eth_polling_button_init(OPS_LED3);
#endif
	
    uip_init();
    uip_arp_init(); // Clear arp table.

#ifdef __DHCPC_H__ //if use fixed ip, #ifdef modify #ifndef
    // setup the dhcp renew timer the make the first request
	//printf("config: DHCPC\r\n");
	#if 1
//	identify_dhcpc_en(1);
		//.identify_tcpip_ip(NULL);
		//.identify_tcpip_gw(NULL);
		//.identify_tcpip_mask(NULL);
		uip_update_ip_config(NULL, NULL, NULL); //[init.dhcpc]
	#endif
    timer_set(&dhcp_timer, CLOCK_SECOND * 600);
    dhcpc_init(&uip_ethaddr, 6);

	downupcount = 0;
	dhcpccount = 0;
	//printf_dhcp_dbg("Init", dhcpccount, clock_time());
    //dhcpc_request();
#else //Fixed IP set

//	identify_dhcpc_en(0);

//.tapdev_set_ip(local_d_ip);
//.tapdev_set_gw(local_d_gw);
//.tapdev_set_mask(local_d_mask);

//.uip_sethostaddr(local_d_ip);
//.uip_setdraddr(local_d_gw);
//.uip_setnetmask(local_d_mask);
	uip_update_ip_config(local_ip, local_gw, local_mask); //[INIT] [init.static]

    /* Display system information */
    printf("---------------------------------------------\r\n");
    printf("Network chip: DAVICOM DM9051 \r\n");
    printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", uip_ethaddr.addr[0], uip_ethaddr.addr[1],
           uip_ethaddr.addr[2], uip_ethaddr.addr[3], uip_ethaddr.addr[4], uip_ethaddr.addr[5]);
    uip_gethostaddr(ipaddr);
    printf("Host IP Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
    uip_getnetmask(ipaddr);
    printf("Network Mask: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
    uip_getdraddr(ipaddr);
    printf("Gateway IP Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
    printf("---------------------------------------------\r\n");
#endif
    httpd_init();
}

int vuIP_Process(void)
{
	int i; //n = 0;

	if (hal_active_interrupt_mode())
	{
		//[version_1]
		/* Interrupt */
		DM_ETHER_Receive_Route_W();
	}
	else
	{
		//[version_1, to be continued.]
		/* Polling */
		if (DM_ETH_RXHandler_Poll())
			return 1; //continue;
	}

	if (timer_expired(&periodic_timer))
	{
		timer_reset(&periodic_timer);
		for (i = 0; i < UIP_CONNS; i++)
		{
			uip_periodic(i);
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
			if (uip_len > 0)
			{
				uip_arp_out();
				tapdev_send(uip_buf, uip_len);
			}
		}

#if UIP_UDP

		for (i = 0; i < UIP_UDP_CONNS; i++)
		{
			uip_udp_periodic(i);
			/* If the above function invocation resulted in data that
			should be sent out on the network, the global variable
			uip_len is set to a value > 0. */
			if (uip_len > 0)
			{
				uip_arp_out();
				tapdev_send(uip_buf, uip_len);
			}
		}

#endif /* UIP_UDP */

		/* Call the ARP timer function every 10 seconds. */
		if (timer_expired(&arp_timer))
		{
			timer_reset(&arp_timer);
			uip_arp_timer();
		}
	}
#ifdef __DHCPC_H__
	else if (dbg_expire()) //if (dbg_timer_expired(&dhcp_timer, clock_time())) //of timer_expired(&dhcp_timer)
	{
		// for now turn off the led when we start the dhcp process
		dhcpccount++;
		dhcpc_renew(); //timer hit...
		timer_reset(&dhcp_timer);
		printf_dhcp_dbg("Expire", dhcpccount, clock_time());
	}
#endif // __DHCPC_H__
#ifdef __DHCPC_H__
	else if (dm_eth_polling_downup(1))
	{
		downupcount++;
		dhcpc_renew(); //net hit...
		#if 1
		uip_update_ip_config(NULL, NULL, NULL); //[dhcpc.restart TESTING.]
		#endif
		timer_restart(&dhcp_timer); //instead, fixed the bug if using "timer_reset(&dhcp_timer)"; //as well
		printf_dhcp_dbg("Linkup", downupcount, clock_time());
	}
#else
	else if (dm_eth_polling_downup(0))
	{
	}
#endif // __DHCPC_H__
	else
	{
		return 0;
	}
#if 1
//	dm_eth_polling_button_ops(OPS_LED3);
#endif
	return 1;
}

void vuIP_Task(void *pvParameters)
{
    const TickType_t xFrequency = 10;
    TickType_t xLastWakeTime = clock_time(); //xTaskGetTickCount();
    (void) pvParameters;

	vuIP_Init();
	
    while (1)
    {
		if (!vuIP_Process())
			/* task delay */
			vTaskDelayUntil(&xLastWakeTime, xFrequency);
    }
}

/*---------------------------------------------------------------------------*/
void    uip_log(char *m)
{
    printf("uIP log message: %s\r\n", m);
}

void    resolv_found(char *name, u16_t *ipaddr)
{
    //u16_t *ipaddr2;

    if (ipaddr == NULL)
    {
        printf("Host '%s' not found.\n", name);
    }
    else
    {
        printf("Found name '%s' = %d.%d.%d.%d\n", name,
               htons(ipaddr[0]) >> 8,
               htons(ipaddr[0]) & 0xff,
               htons(ipaddr[1]) >> 8,
               htons(ipaddr[1]) & 0xff);
        /*    webclient_get("www.sics.se", 80, "/~adam/uip");*/
    }
}

#ifdef __DHCPC_H__
static void ap_printf(char *str) {
	printf("%s", str);
}
static void ap_printkey(char *str) {
	printkey("%s", str);
}
#endif

#ifdef __DHCPC_H__
void    dhcpc_configured(const struct dhcpc_state *s)
{
	static uint8_t ap_printag = 0x3;
    if (s->state == STATE_FAIL)
    {
		uip_update_ip_config(local_ip, local_gw, local_mask); //[NOT ESSENTIAL] [dhcpc fail] //uip_update_ip_config(NULL, NULL, NULL);
		ap_print_ipconfig("--Fixed IP address ----------------------",
			identified_tcpip_mac(), //uip_ethaddr.addr,
			ap_printag & 0x01 ? 
				ap_printf : 
				ap_printkey
			);
		ap_printag &= ~0x01;
    }
    else
    {
//      resolv_conf(s->dnsaddr);            // Now don't need DNS
		uip_update_ip_config((const uint8_t *) s->ipaddr, (const uint8_t *) s->default_router, (const uint8_t *) s->netmask); //[dhcpc succeed!]
		ap_print_ipconfig("--IP address setting from DHCP-----------", /* Display system information */
			uip_ethaddr.addr,
			ap_printag & 0x02 ? 
				ap_printf : 
				ap_printkey
			);
		ap_printag &= ~0x02;
    }
}
#endif /* __DHCPC_H__ */

void    smtp_done(unsigned char code)
{
    printf("SMTP done with code %d\n", code);
}
void    webclient_closed(void)
{
    printf("Webclient: connection closed\n");
}
void    webclient_aborted(void)
{
    printf("Webclient: connection aborted\n");
}
void    webclient_timedout(void)
{
    printf("Webclient: connection timed out\n");
}
void    webclient_connected(void)
{
    printf("Webclient: connected, waiting for data...\n");
}
void    webclient_datahandler(char *data, u16_t len)
{
    printf("Webclient: got %d bytes of data.\n", len);
}
