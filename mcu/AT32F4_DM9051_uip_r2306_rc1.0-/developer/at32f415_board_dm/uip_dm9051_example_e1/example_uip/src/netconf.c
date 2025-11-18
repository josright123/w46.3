/**
  **************************************************************************
  * @file     netconf.c
  * @version  v1.0
  * @date     2023-04-28
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

//#include "lwip/memp.h"
//#include "lwip/tcp.h"
//#include "lwip/priv/tcp_priv.h"
//#include "lwip/udp.h"
//#include "netif/etharp.h"
//#include "lwip/dhcp.h"
//#include "lwip/init.h"
#include "at32f415_board.h" //"at32f403a_407_board.h"
#include "at32f415_clock.h" //"at32f403a_407_clock.h"
#include "uip_arp.h"
#include "uip.h"
#include "developer_conf.h"
#include "ethernetif.h"
#include "netconf.h"
#include "stdio.h"
#include "dm9051_env.h" //"dm9051f_netconf.h" //"at32_emac.h"
#include <string.h>

#ifndef DM9051_DIAG
#define DM9051_DIAG(x) do {printf x;} while(0)
#include <stdio.h>
#include <stdlib.h>
#endif

#define DM9051_RX_DBGMSG(expression, message) do { if ((expression)) { \
  DM9051_DIAG(message);}} while(0)

u32_t lwip_sys_now = 0;
u32_t g_RunTime = 0; /* __IO */

//.extern volatile uint32_t all_local_time;
volatile uint32_t all_local_time = 0;

int dm9051_read_eeprom_mac(void);
int cpu_read_flash_mac(void);

//static uint8_t mac_address[MAC_ADDR_LENGTH] = {0, 0x60, 0x6e, 0x00, 0x01, 0x25};
//=
static dm9051_mac_conf maccfg = {
	//.cfg = {
	//	.adopt_mode = ADOPTE_FLASH_MODE,
	//	.mac_f = cpu_read_flash_mac,
	//},
	//.cfg = {
	//	.adopt_mode = ADOPTE_EEPROM_MODE,
	//	.mac_f = dm9051_read_eeprom_mac,
	//},
	//.cfg = {
	//	.adopt_mode = ADOPTE_CONST_MODE,
	//	.mac_f = NULL,
	//	.macaddr = { 0x00, 0x60, 0x6e, 0x00, 0x01, 0x25},
	//},
	.cfg = {
		.adopt_mode = ADOPTE_EEPROM_MODE,
		.mac_f = dm9051_read_eeprom_mac,
	},
};

//struct netif netif;

//#if (!LWIP_DHCP)
#define	HOST_IP0	192
#define	HOST_IP1	168
#define	HOST_IP2	1
#define	HOST_IP3	37
//static const uint8_t cfg_local_ip[ADDR_LENGTH]   = {192, 168, 1, 37}; //{192, 168, 6, 37};
//static uint8_t cfg_local_gw[ADDR_LENGTH]   = {192, 168, 1, 254}; //{192, 168, 6, 1};
//static uint8_t cfg_local_mask[ADDR_LENGTH] = {255, 255, 255, 0};

#define	GW_IP0	192
#define	GW_IP1	168
#define	GW_IP2	1
#define	GW_IP3	254

#define	MASK_IP0	255
#define	MASK_IP1	255
#define	MASK_IP2	255
#define	MASK_IP3	0
//#endif
const uip_ipaddr_t HOST_IP = {
	HTONS(((HOST_IP0) << 8) | (HOST_IP1)), 
	HTONS(((HOST_IP2) << 8) | (HOST_IP3)),
};
//const uip_ipaddr_t HOST_IP_LW = {
//	HTONS(((cfg_local_ip[0]) << 8) | (cfg_local_ip[1])), 
//	HTONS(((cfg_local_ip[2]) << 8) | (cfg_local_ip[3])),
//};
const uip_ipaddr_t GW_IP = {
	HTONS(((GW_IP0) << 8) | (GW_IP1)), 
	HTONS(((GW_IP2) << 8) | (GW_IP3)),
};
const uip_ipaddr_t MASK_IP = {
	HTONS(((MASK_IP0) << 8) | (MASK_IP1)), 
	HTONS(((MASK_IP2) << 8) | (MASK_IP3)),
};

/* if "expression" is true, then execute "handler" expression */
#define NET_TIMER_TASK(expression, handler) do { if ((expression)) { \
  handler;}} while(0)
#define NET_INPUT_BREAK(expression, handler) do { if ((expression)) { \
  handler;}} while(0)
 
#if 0
/**
  * @brief  this function sets the netif link status.
  * @param  netif: the network interface
  * @retval none
  *
  * Called by _lwip_periodic_handle() / "netconf.c"
  *
  */ 
/* A function env_ethernetif_set_link_Timer(void const *argument) / call to "dm9051f.c",
 */
uint16_t env_ethernetif_set_link_Timer(void const *argument)
{
  /*static*/ uint16_t regvalue;
  
  /* Read PHY_BSR*/
  regvalue = _dm9051_update_phyread("___periodic_TMR5___");
  //as:
  //static uint16_t netif_flags = 0;
  //if(!netif_flags && (regvalue))
	//  printf("(netconf)  link 111111\r\n");
  //else if(netif_flags && (!regvalue))
	//  printf("(netconf)  link 000000\r\n");
  //netif_flags = regvalue;
  return regvalue;
}
#endif

int dm9051_read_eeprom_mac(void)
{
	//maccfg.cfg.macaddr.addr[0] = 0;
	//uip_ethaddr.addr[0] = 0;
	dm9051_ethaddr_eepromread(maccfg.cfg.macaddr.addr);
	return 0;
}
int cpu_read_flash_mac(void)
{
	/* CUSTOMIZATION REQUIRED 
	 */
	return 0;
}

/**
  * @brief  updates the system local time
  * @param  none
  * @retval none
  */
void time_update(void)
{
  all_local_time += SYSTEMTICK_PERIOD_MS10;
}

/* linkDetectFunc
 */
static void link_handle(uint32_t eachtime)
{
  static volatile uint32_t link_timer = 0;

  if (all_local_time - link_timer >= eachtime || all_local_time < link_timer)
  {
    link_timer =  all_local_time;
	  
	NET_TIMER_TASK(1, dm9051_update_phyread("periodic_TMR5")); //[JJ another Link-detect!!]
    //= env_ethernetif_set_link_Timer(NULL);
  }
}

void _periodic_handle(void)
{
#if (LINK_DETECTION > 0)
	link_handle(500);
#endif
}

static struct timer periodic_timer, arp_timer;

//..struct DM9051_eth DM9051_device;

void tapdev_init(void) //or ever 'InitNet_Config' 
{
	//struct uip_eth_addr ethaddr;
	uip_ipaddr_t ipaddr;
	
	timer_set(&periodic_timer, CLOCK_SECOND / 2);	//500ms
  timer_set(&arp_timer, CLOCK_SECOND * 10);	// 10sec
  
  uip_init();
	uip_arp_init();	// Clear arp table.

	#if 0	
	/* inline source code in "core_cm4.h"
	 */
	/*Initial and start system tick time = 1ms */
	SysTick_Config(SystemCoreClock / 1000); 
	#endif
	
	/* Mac address does from the DM9051 driver */
	if (maccfg.cfg.adopt_mode == ADOPTE_FLASH_MODE) {
		maccfg.cfg.mac_f();
	} else if (maccfg.cfg.adopt_mode == ADOPTE_EEPROM_MODE) {
		maccfg.cfg.mac_f();
	} else {
		/* already defined in the maccfg.cfg.macaddr.addr[] */
	}
	
#if DHCPC_EN
	/* setup the dhcp renew timer the make the first request */
	uip_setethaddr(maccfg.cfg.macaddr); //ethaddr
	timer_set(&dhcp_timer, CLOCK_SECOND * 600);
	dhcpc_init(&uip_ethaddr, 6);
#else
	uip_setethaddr(maccfg.cfg.macaddr); //ethaddr
	#if 0
	//uip_ipaddr(ipaddr, 192,168,6,25);=
	//uip_ipaddr(ipaddr, HOST_IP0, HOST_IP1, HOST_IP2, HOST_IP3);=
	//uip_ipaddr_copy(ipaddr, HOST_IP);=
	  uip_ipaddr(ipaddr, 
	  uip_ipaddr1(HOST_IP),uip_ipaddr2(HOST_IP),uip_ipaddr3(HOST_IP),uip_ipaddr4(HOST_IP)); //Host IP address
	  uip_sethostaddr(ipaddr);
	  uip_ipaddr(ipaddr, GW_IP0, GW_IP1, GW_IP2, GW_IP3);		//Default Gateway
	  uip_setdraddr(ipaddr);
	  uip_ipaddr(ipaddr, MASK_IP0, MASK_IP1, MASK_IP2, MASK_IP3);	//Network Mask
	  uip_setnetmask(ipaddr);
	#else
		uip_sethostaddr(HOST_IP); //Host IP address
		uip_setdraddr(GW_IP);		//Default Gateway
		uip_setnetmask(MASK_IP);	//Network Mask
	#endif
	//show_netwaork_configure();
	printf("---------------------------------------------\r\n");
	printf("Network chip: DAVICOM DM9051 \r\n");
	printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n", 
					uip_ethaddr.addr[0], uip_ethaddr.addr[1], uip_ethaddr.addr[2], uip_ethaddr.addr[3], uip_ethaddr.addr[4], uip_ethaddr.addr[5]);
	uip_gethostaddr(ipaddr);				
	printf("Host IP Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	uip_getnetmask(ipaddr);
	printf("Network Mask: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	uip_getdraddr(ipaddr);
	printf("Gateway IP Address: %d.%d.%d.%d \r\n", uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	printf("---------------------------------------------\r\n");
#endif //DHCPC_EN

	ethernetif_init((uint8_t *)&maccfg.cfg.macaddr); //ethaddr.addr
}

void tapdev_loop(void)
{
	_periodic_handle();
	uip_input_protocol();
	uip_timer_protocol();
}

#define BUF ((struct uip_eth_hdr *)&uip_buf[0])
	
/*
struct ethip_hdr { //[temp-debug]
  struct uip_eth_hdr ethhdr;
  u8_t vhl,// IP header
    tos,
    len[2],
    ipid[2],
    ipoffset[2],
    ttl,
    proto;
  u16_t ipchksum;
  u16_t srcipaddr[2],
    destipaddr[2];
};
#define IPBUF ((struct ethip_hdr *)&uip_buf[0])
*/

void uip_input_protocol(void) //= 'uip_protocol()'
{
	uip_len = ethernetif_input(); //spi_DM9051_rx(); //tapdev_read();
	NET_INPUT_BREAK(uip_len <= 0, return);

	if(uip_len > 0) 
	{
		if(BUF->type == htons(UIP_ETHTYPE_IP)) {
			
			//.NET_TIMER_TASK(IPBUF->proto == 1, printxxx()); //[temp-debug]

			uip_arp_ipin();   //Removed by Spenser
			uip_input();		// uip_process(UIP_DATA)
			
			/* If the above function invocation resulted in data that
				should be sent out on the network, the global variable
				uip_len is set to a value > 0. */
			if(uip_len > 0) {
				uip_arp_out();
				ethernetif_output(); //spi_DM9051_tx(); //tapdev_send();
			}
		} else if(BUF->type == htons(UIP_ETHTYPE_ARP)) {
			
			uip_arp_arpin();
			/* If the above function invocation resulted in data that
				 should be sent out on the network, the global variable
				 uip_len is set to a value > 0. */
			if(uip_len > 0) {
				ethernetif_output(); //spi_DM9051_tx(); //tapdev_send();
			}
		}
	}
}

/* if "expression" is true, then execute "handler" expression */
#define DBG_TIMER_PROTO(expression, handler) do { if ((expression)) { \
  handler;}} while(0)

#define DBG_TIMER_PROTO_VAR_DECLA \
struct clock_arch_test_t { \
	int clock_tome_expirechecking; \
	int clock_time_expire; \
} cat = { \
	.clock_tome_expirechecking = 9, \
	.clock_time_expire = 8, \
};

#define DBG_TIMER_PROTO_FUNC_DECLA \
static void cat_checking(void) \
{ \
  cat.clock_tome_expirechecking--; \
  printf("periodic_timer expire-checking %d (lwip_sys_now %u g_RunTime %u)\r\n", \
	cat.clock_tome_expirechecking, lwip_sys_now, g_RunTime); \
}

#define DBG_TIMER_PROTO_FUNC2_DECLA \
static void cat_checking2(void) \
{ \
	cat.clock_time_expire--; \
	printf("periodic_timer expire %d (lwip_ %u g_ %u)\r\n", \
	cat.clock_time_expire, lwip_sys_now, g_RunTime); \
}

//DBG_TIMER_PROTO_VAR_DECLA
//DBG_TIMER_PROTO_FUNC_DECLA
//DBG_TIMER_PROTO_FUNC2_DECLA

void uip_timer_protocol(void)  //= 'poll_protocol()'/='eth_poll()'
{
#if DHCPC_EN
	uint16_t status, linkch; 
#endif //DHCPC_EN
	
	//DBG_TIMER_PROTO(cat.clock_tome_expirechecking, cat_checking());
  
	if(timer_expired(&periodic_timer)) 
	{
	  //.DBG_TIMER_PROTO(cat.clock_time_expire, cat_checking2());
		
      timer_reset(&periodic_timer);
	  do {
		  int i;
		  for(i = 0; i < UIP_CONNS; i++) {
				uip_periodic(i);
				/* If the above function invocation resulted in data that
				should be sent out on the network, the global variable
				uip_len is set to a value > 0. */
				if(uip_len > 0) {
					uip_arp_out();   
					ethernetif_output(); //spi_DM9051_tx(); //tapdev_send();
				}
		  }
	#if UIP_UDP
		  for(i = 0; i < UIP_UDP_CONNS; i++) {
				uip_udp_periodic(i);
				/* If the above function invocation resulted in data that
				should be sent out on the network, the global variable
				uip_len is set to a value > 0. */
				if(uip_len > 0) {
					uip_arp_out();
					ethernetif_output(); //spi_DM9051_tx(); //tapdev_send();
					printf("i---%d\r\n",i);
					//uip_udp_conns[i].rport = 0;  //¶Ë¿ÚÉèÖÃÎª0
					//.break;
				}
		  }
	#endif /* UIP_UDP */

		  NET_TIMER_TASK(1, dm9051_update_phyread("timer_expired")); //[JJ new-add Link-detect!!]
		  
		  /* Call the ARP timer function every 10 seconds. */
		  if(timer_expired(&arp_timer)) {
					timer_reset(&arp_timer);
					uip_arp_timer();
		  }
	  } while(0);
	}
	
#if DHCPC_EN
	else if (timer_expired(&dhcp_timer)) 
	{
		/* for now turn off the led when we start the dhcp process */
		status = DM9051_Read_Reg(DM9051_NSR);
		linkch = DM9051_Read_Reg(DM9051_ISR);
		
		if((status == 0x0C) || (linkch == 0xA3)){
			//dhcpc_renew();
		}
		timer_reset(&dhcp_timer);
    }
#endif //DHCPC_EN
}

#if 0
void _tcpip_stack_init(void) {
  ...
  netif_set_link_callback(&netif, env_ethernetif_update_config_cb);
  ...
}
	
void _lwip_periodic_handle(void) {
	static volatile uint32_t arp_timer = 0;
	arp_timer = arp_handle(arp_timer, ARP_TMR_INTERVAL);
	#if (LINK_DETECTION > 0)
	_link_handle(500);
	#endif
	tcp_handle(TCP_TMR_INTERVAL);
	publish_handle(2500);
}
#endif
