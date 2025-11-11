/**
  **************************************************************************
  * @file     main.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    main program
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

#include "at32f435_437_board.h"
#include "at32f435_437_clock.h"
#include "at32_emac.h"
#include "netconf.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/etharp.h"
#include "ethernetif_v51.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#define APP_HEADER_MCU_DESC		"f437"

int DM_ETHER_Receive_Route_W0(void);
int DM_ETHER_Receive_Route_W1(void);
int ethernetif_set_link(void const *argument);
struct netif *dm_eth_netif(void);
void dm_wait2_periodic_stat(void);

volatile uint32_t tcp_timer = 0;
volatile uint32_t arp_timer = 0;
volatile uint32_t link_timer = 0;

#if freeRTOS
#warning "freeRTOS is defined"
#else
//.#warning "freeRTOS is exactly pa project NOT defined"
#endif

/** @addtogroup AT32F437_periph_examples
  * @{
  */

/** @addtogroup 437_EMAC_telnet EMAC_telnet
  * @{
  */

//#define main_tick_handler SysTick_Handler //JJ0 (in "main.c")
//void main_tick_handler(void);

#define DELAY                            100
#define FAST                             1
#define SLOW                             4

uint8_t g_speed = FAST;
volatile uint32_t local_time = 0;

#if 1
void main_tick_handler(void)
{
	//dm9051_boards_heartbeat_tick();
	//xPortSysTickHandler(); //SysTick_Handler_from_main(); //xPortSysTickHandler(); 
}
#endif

int rxhdl_v51 = RXHDL_V51; //!RXHDL_OLD;

struct netif netif_emac;
struct netif netif_v51;
int netif_v51_ready;

//struct netif *tcpip_stack_netif_emac(void)
//{
//	return &netif_emac;
//}

struct netif *tcpip_stack_netif(void)
{
	return &netif_v51;
}

//static void stack_show_default_netif_ip(void) //(const Octet *ifaceName, Octet *uuid, NetPath *netPath)
//{
//    struct netif * iface;
//	u32_t ipaddr;
//	
//    iface = netif_default;
//	ipaddr = iface->ip_addr.addr;
//    //memcpy(uuid, iface->hwaddr, iface->hwaddr_len);
//    //return iface->ip_addr.addr;

//	printf("It's ip : %u.%u.%u.%u\r\n", 
//		ipaddr & 0xff, (ipaddr >> 8) & 0xff,
//		(ipaddr >> 16) & 0xff, (ipaddr >> 24) & 0xff);
//}

void stack_enum_show_netif_ip(void)
{
	//struct netif *netif;
	netif_set_default(&netif_v51);
	printf("Enum set default netif netif_v51:\r\n");
	printf("Enum netif name %c%c, It's ip %u.%u.%u.%u\r\n",
		netif_default->name[0], netif_default->name[1],
		netif_default->ip_addr.addr & 0xff, (netif_default->ip_addr.addr >> 8) & 0xff,
		(netif_default->ip_addr.addr >> 16) & 0xff, (netif_default->ip_addr.addr >> 24) & 0xff);
	/*  show Registers the default network interface ip.*/
	//stack_show_default_netif_ip();
}

void stack_switch_default_netif_ip(struct netif *netif)
{
	netif_set_default(netif);
	
  if (NETIF_IS_EMAC(netif_default))
	printf("on set default netif_emac:\r\n");
  else if (NETIF_IS_V51(netif_default))
	printf("on set default netif_v51:\r\n");
  else
	printf("on set default unknow netif:\r\n");

	printf("netif name %c%c, It's ip %u.%u.%u.%u\r\n",
		netif_default->name[0], netif_default->name[1],
		netif_default->ip_addr.addr & 0xff, (netif_default->ip_addr.addr >> 8) & 0xff,
		(netif_default->ip_addr.addr >> 16) & 0xff, (netif_default->ip_addr.addr >> 24) & 0xff);
}

void stack_show_netif_list(void)
{
  struct netif *netif;
  int n = 0;
  /* loop through netif's */
  NETIF_FOREACH(netif) {
	  printf("Foreach, netif %d: %c%c\r\n", ++n, netif->name[0], netif->name[1]);
  }
}

void board_init(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  system_clock_config();
  uart_print_init(115200);

  //=at32_board_init();
  delay_init();
  at32_led_init(LED2);
  at32_led_init(LED3);
  at32_led_init(LED4);
}

#if 0
void emac_ptp(void)
{
  crm_clocks_freq_type crm_clocks_freq_struct = {0};	//modify 2023 11-14

  emac_ptp_target_second_set(10);
  emac_ptp_target_nanosecond_set(0);
  emac_interrupt_mask_set(EMAC_INTERRUPT_TST_MASK, FALSE);
  emac_ptp_interrupt_trigger_enable(TRUE);
  emac_pps_out();

#if 1		//modifyh 2023 11-08  test
		//emac_ptp_psv2_enable(TRUE);
		emac_ptp_clock_node_set(EMAC_PTP_END_TO_END_CLOCK);
		
		crm_clocks_freq_get(&crm_clocks_freq_struct);
		printf("\nCLK:%d %d %d %d\r\n",crm_clocks_freq_struct.sclk_freq,crm_clocks_freq_struct.ahb_freq,crm_clocks_freq_struct.apb2_freq,crm_clocks_freq_struct.apb1_freq);

	#if 0
    printf("EMAC->frmf 0x%08lx\r\n", (uint32_t)EMAC->frmf);
    printf("EMAC->frmf.pcf 0x%08lx\r\n", (uint32_t)(EMAC->frmf & 0x3));
    printf("EMAC_PTP->tsctrl 0x%08lx\r\n", (uint32_t)EMAC_PTP->tsctrl);
    printf("EMAC_PTP->ssinc 0x%08lx\r\n", (uint32_t)EMAC_PTP->ssinc);
    printf("EMAC_PTP->tsh 0x%08lx\r\n", (uint32_t)EMAC_PTP->tsh);
    printf("EMAC_PTP->tsl 0x%08lx\r\n", (uint32_t)EMAC_PTP->tsl);
    printf("EMAC_PTP->tshud 0x%08lx\r\n", (uint32_t)EMAC_PTP->tshud);
    printf("EMAC_PTP->tslud 0x%08lx\r\n", (uint32_t)EMAC_PTP->tslud);
    printf("EMAC_PTP->tsad 0x%08lx\r\n", (uint32_t)EMAC_PTP->tsad);
    printf("EMAC_PTP->tth 0x%08lx\r\n", (uint32_t)EMAC_PTP->tth);
    printf("EMAC_PTP->ttl 0x%08lx\r\n", (uint32_t)EMAC_PTP->ttl);
    printf("EMAC_PTP->tssr 0x%08lx\r\n", (uint32_t)EMAC_PTP->tssr);
    printf("EMAC_PTP->ppscr 0x%08lx\r\n", (uint32_t)EMAC_PTP->ppscr);
    printf("EMAC_PTP->tsctrl.tdbrc 0x%08lx\r\n", (uint32_t)(EMAC_PTP->tsctrl & 0x2));
	#endif
#endif
}
#endif

#if 1
void v51_ptp(void)
{
}
#endif

/**
  * @brief  lwip periodic tasks
  * @param  localtime the current localtime value
  * @retval none
  */
void lwip_periodic_handle(volatile uint32_t localtime)
{
  /* 'localtime' from global varible
  */
  localtime = local_time;

  /* TCP periodic process every 250 ms */
  if (localtime - tcp_timer >= TCP_TMR_INTERVAL || localtime < tcp_timer)
  {
    tcp_timer =  localtime;
    tcp_tmr();
  }
  /* ARP periodic process every 5s */
  if (localtime - arp_timer >= ARP_TMR_INTERVAL || localtime < arp_timer)
  {
    arp_timer =  localtime;
    etharp_tmr();
  }

#if LWIP_DHCP
  /* Fine DHCP periodic process every 500ms */
  if (localtime - dhcp_fine_timer >= DHCP_FINE_TIMER_MSECS || localtime < dhcp_fine_timer)
  {
    dhcp_fine_timer =  localtime;
    dhcp_fine_tmr();
  }
  /* DHCP Coarse periodic process every 60s */
  if (localtime - dhcp_coarse_timer >= DHCP_COARSE_TIMER_MSECS || localtime < dhcp_coarse_timer)
  {
    dhcp_coarse_timer =  localtime;
    dhcp_coarse_tmr();
  }
#endif
}

static void lwip_periodic_link(volatile uint32_t localtime, link_t t)
{
	struct netif *netif = tcpip_stack_netif(); //tcpip_stack_netif_emac();
	switch(t) {
		case NET_DETECT_UIP:
			/* link detection process every 500 ms */
			if (localtime - link_timer >= 500 || localtime < link_timer)
			{
				link_timer = localtime;
#if LWIP_DHCP
				if (dm_eth_polling_downup(1))
#else
				if (dm_eth_polling_downup(0)) //in "platform_info.c"
#endif
				{
					#if LWIP_DHCP
					//dhcpc_renew();... (done, by ethernetif_notify_conn_changed())
					#else
					printf("(up %d.%d.%d.%d) notify\r\n",
						ip4_addr1(&netif->ip_addr), ip4_addr2(&netif->ip_addr),
						ip4_addr3(&netif->ip_addr), ip4_addr4(&netif->ip_addr));
					#endif
				}
			}
			break;
		case NET_DETECT_LIKE_LWIP:
			if (localtime - link_timer >= 500 || localtime < link_timer)
			{
				link_timer =  localtime;

				ethernetif_polling_downup(tcpip_stack_netif());
			}
			break;
		case NET_DETECT_LWIP:
			/* link detection process every 500 ms */
			#if (LINK_DETECTION > 0)
			  if (localtime - link_timer >= 500 || localtime < link_timer)
			  {
				link_timer =  localtime;

//				if (ethernetif_set_link(netif))
//					; //if(LWIP_DHCP)...
				if (ethernetif_set_link(netif)) //set_link(&netif); //in "at32_emac.c"
					; //if(LWIP_DHCP)...
//				ethernetif_set_link(netif);
			  }
			#endif
			break;
	}
}

void lwip_periodic_link_hdlr(volatile uint32_t localtime)
{
  /* 'localtime' from global varible
  */
  localtime = local_time;

	if (netif_v51_ready) {
		#if 0
		lwip_periodic_link(localtime, NET_DETECT_UIP);
		if (!platform_is_link_up())
			dm_wait2_periodic_stat(); //to main.c
		#elif 0
		lwip_periodic_link(localtime, NET_DETECT_LIKE_LWIP);
		if (!netif_is_link_up(dm_eth_netif()))
			dm_wait2_periodic_stat(); //to main.c
		#else
		lwip_periodic_link(localtime, NET_DETECT_LWIP);
		if (!netif_is_link_up(dm_eth_netif()))
			dm_wait2_periodic_stat(); //to main.c
		#endif
//		if (state == 0 || linkdown())
//			DmPtpdCombinedState_t state = DM_ENUM_COMBINED_WAIT_NET; //= dm_wait2_periodic_stat();
	}
}

typedef enum
{
    DM_ENUM_COMBINED_WAIT_NET = 1,   /**< Waiting for network to be ready */
    DM_ENUM_COMBINED_START_PTPD, /**< Start PTPD process */
    DM_ENUM_COMBINED_START_MQTT,
    DM_ENUM_COMBINED_PERIODIC,    /**< PTPD process started, run periodic */
    //DM_ENUM_COMBINED_WAIT2_NET,   /**< Waiting for network to be ready */
} DmPtpdCombinedState_t;

void dm_wait2_periodic_stat(void)
{
//	state = DM_ENUM_COMBINED_WAIT_NET; //DM_ENUM_COMBINED_WAIT2_NET;
}

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  dm9051_config_app_mcu_name(APP_HEADER_MCU_DESC);
  board_init();
  system_timer_init();  /* Initialize SysTick to generate interrupts every 10ms */

  printf("\r\n");
  printf("[at32%s]\r\n", dm_eth_app_mcu_name(NULL));
  dm_eth_show_app_help_info("LWIP_project", "webserver"); //printkey("\r\n\r\n\r\n/ZYK_project /R2410 [uip_dm9051_r2410] %s\r\n", __DATE__);
  //dm_eth_show_app_help_info_w(RX_MODE_STR, "ARTERY", "Lwip_project", "pa");

//  status = emac_system_init();
//  while(status == ERROR);

//  tmr_configuration();
	
  stack_tcpip_init_v51();
  
  //dm9051_v51
  netif_v51.name[0] = DM_IFNAME0;
  netif_v51.name[1] = DM_IFNAME1;
  stack_init(&netif_v51); //DM_Eth_LwipInitialize_W();

  stack_enum_show_netif_ip();

  if (netif_v51_ready) {
	  stack_switch_default_netif_ip(&netif_v51);
  }
   
  stack_show_netif_list();
//  emac_ptp();

//static DmPtpdCombinedState_t state = 0;
  for(;;)
  {
	/* v51 receive handle */
	if (netif_v51_ready) {
//		if (rxhdl_v51 == RXHDL_OLD)
//			DM_ETHER_Receive_Route_W0(); //=.net_inp_poll();
//		else
//			DM_ETHER_Receive_Route_W1();
#if RXHDL_V51 == RXHDL_OLD
			DM_ETHER_Receive_Route_W0(); //=.net_inp_poll();
#endif
#if RXHDL_V51 == !RXHDL_OLD
			DM_ETHER_Receive_Route_W1();
#endif
	}
	  
	/* one Instance 
	 */
//	if(state != DM_ENUM_COMBINED_WAIT_NET || net_ip_bound()) {
//		if (state == DM_ENUM_COMBINED_WAIT_NET)
//			state = DM_ENUM_COMBINED_START_PTPD;

//		switch (state)
//		case DM_ENUM_COMBINED_START_PTPD:
//		network_ptp_init();
//		case DM_ENUM_COMBINED_START_MQTT:
//		mqtt_connecting_process();
//		case DM_ENUM_COMBINED_PERIODIC:
//		ptpd_Periodic_Handle(localtime);
//	}

    lwip_periodic_handle(0);
	lwip_periodic_link_hdlr(0);

	if (netif_v51_ready) {
		/* v51 todo track everything for _v51_ptpd .*/
		netif_set_default(&netif_v51);
	}
  }
}

/**
  * @}
  */

/**
  * @}
  */
