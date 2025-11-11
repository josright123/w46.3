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
#include "lwip/memp.h"
#include "lwip/tcp.h"
#include "lwip/priv/tcp_priv.h"
#include "lwip/udp.h"
#include "netif/etharp.h"
#include "lwip/dhcp.h"
#include "lwip/init.h"
#include "ethernetif.h"
#include "netconf.h"
#include "stdio.h"
#include "at32_emac.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
void dm9051_config_if(struct netif *set_netif);

/** @addtogroup AT32F437_periph_examples
  * @{
  */

/** @addtogroup 437_EMAC_telnet
  * @{
  */

/** @addtogroup network_configuration
  * @{
  */

/* Private define ------------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
#define MAC_ADDR_LENGTH                  (6)
#define ADDR_LENGTH                      (4)
#define SYSTEMTICK_PERIOD_MS             10

//static uint8_t mac_address[MAC_ADDR_LENGTH] = {0, 0x60, 0x6e, 0x00, 0x00, 0x17};

#if LWIP_DHCP
volatile uint32_t dhcp_fine_timer = 0;
volatile uint32_t dhcp_coarse_timer = 0;
#else
//static uint8_t local_ip[ADDR_LENGTH]   = {192, 168, /*1*/6, 17};
//static uint8_t local_gw[ADDR_LENGTH]   = {192, 168, /*1*/6, 1 /*254*/}; //1
//static uint8_t local_mask[ADDR_LENGTH] = {255, 255, 255, 0};
#endif
// ...
//struct netif netif_emac;
extern volatile uint32_t local_time;

err_t httpd_init_with_netif(struct netif *preferred_netif);

#define DAVICOM_ASSERT(message, assertion) do { if (!(assertion)) { \
  printf(message); }} while(0)

void stack_init(struct netif *netif)
{
  tcpip_stack_init_E(netif); //(netif, &ipaddr, &netmask, &gw);
  dm9051_config_if(netif);
//#if 1
//  print_up("stack_init(test)");
//#endif

  if (NETIF_IS_V51(netif) && netif_v51_ready) {
	  err_t err = httpd_init_with_netif(netif); //or, httpd_init(); //part of _DM_Eth_LwipInitialize_W();
	  DAVICOM_ASSERT("httpd_init_with_netif: failed", err == ERR_OK);
  }
}

//void ethnode_config_E(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw)
//{
//#if LWIP_DHCP  //need DHCP server
//  ipaddr->addr = 0;
//  netmask->addr = 0;
//  gw->addr = 0;

//#else
//  IP4_ADDR(ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
//  IP4_ADDR(netmask, local_mask[0], local_mask[1], local_mask[2], local_mask[3]);
//  IP4_ADDR(gw, local_gw[0], local_gw[1], local_gw[2], local_gw[3]);
//#endif

//	printf("IP:%u %u %u %u\r\n",local_ip[0], local_ip[1], local_ip[2], local_ip[3]);		//modify 2023-11-23
//  lwip_set_mac_address(mac_address);
//}

/**
  * @brief  initializes the lwip stack
  * @param  struct netif *netif, tcpip_stack_netif_emac() or tcpip_stack_netif()
  * @retval none
  */
void tcpip_stack_init_E(struct netif *netif) //(, ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw)
{	
  ip_addr_t ipaddr;
  ip_addr_t netmask;
  ip_addr_t gw;

//  /* Initialize the LwIP stack */
//  #if 0 //(!already)
//  lwip_init();
//  #endif

  //if (NETIF_IS_EMAC(netif))
	//ethnode_config_E(&ipaddr, &netmask, &gw);
  //else 
  if (NETIF_IS_V51(netif))
	ethnode_config(&ipaddr, &netmask, &gw);
  else {
	printf("stack_init wrong, check init.\r\n");
	while(1);
  }

  /* - netif_add(struct netif *netif, struct ip_addr *ipaddr,
            struct ip_addr *netmask, struct ip_addr *gw,
            void *state, err_t (* init)(struct netif *netif),
            err_t (* input)(struct pbuf *p, struct netif *netif))

   Adds your network interface to the netif_list. Allocate a struct
  netif and pass a pointer to this structure as the first argument.
  Give pointers to cleared ip_addr structures when using DHCP,
  or fill them with sane numbers otherwise. The state pointer may be NULL.

  The init function pointer must point to a initialization function for
  your ethernet netif interface. The following code illustrates it's use.*/

//  netif->name[0] = AT_IFNAME0;
//  netif->name[1] = AT_IFNAME1;

  //print_up("(test)\r\n");
  printf("\r\n");
//  printf("[%s mode] (%s) %d.%d.%d.%d\r\n",
//		hal_active_interrupt_desc(), "netif_add.s",
//		((uint8_t *)(netif->ip_addr.addr))[0],
//		((uint8_t *)(netif->ip_addr.addr))[1],
//		((uint8_t *)(netif->ip_addr.addr))[2],
//		((uint8_t *)(netif->ip_addr.addr))[3]);
  printf("[%s mode] (%s) param= %d.%d.%d.%d\r\n",
		hal_active_interrupt_desc(), "netif_add.s",
		((uint8_t *)(&ipaddr))[0],
		((uint8_t *)(&ipaddr))[1],
		((uint8_t *)(&ipaddr))[2],
		((uint8_t *)(&ipaddr))[3]);

  if(netif_add(netif, &ipaddr, &netmask, &gw, NULL, &ethernetif_init_E, &netif_input) == NULL)
  {
    while(1);
  }

  printf("[%s mode] (%s) %d.%d.%d.%d\r\n",
		hal_active_interrupt_desc(), "netif_add.e",
		ip4_addr1_16(&(netif->ip_addr)), //((uint8_t *)netif->ip_addr.addr)[0],
		ip4_addr2_16(&(netif->ip_addr)), //((uint8_t *)netif->ip_addr.addr)[1],
		ip4_addr3_16(&(netif->ip_addr)), //((uint8_t *)netif->ip_addr.addr)[2],
		ip4_addr4_16(&(netif->ip_addr))); //((uint8_t *)netif->ip_addr.addr)[3]);
  //ip4_addr_debug_print(1, &(netif->ip_addr));

  #if 0
  /*  Registers the default network interface.*/
  netif_set_default(netif);
  #endif

#if LWIP_DHCP
/*  Creates a new DHCP client for this interface on the first call.
Note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
the predefined regular intervals after starting the client.
You can peek in the netif->dhcp struct for the actual DHCP status.*/
dhcp_start(netif);
#endif

  /*  When the netif is fully configured this function must be called.*/
  if (netif_v51_ready)
	netif_set_up(netif);
  
  /* Set the link callback function, this function is called on change of link status*/
  netif_set_link_callback(netif, ethernetif_update_config);
}

/**
  * @brief  called when a frame is received
  * @param  none
  * @retval none
  */
//void lwip_pkt_handle(void)
//{
//  struct netif *netif = tcpip_stack_netif_emac();
//  /* Read a received packet from the Ethernet buffers and send it to the lwIP for handling */
//  if(ethernetif_input_E(netif) != ERR_OK)
//  {
//    while(1);
//  }
//}

/**
  * @brief  this function is receive handler.
  * @param  none
  * @retval none
  */
//void lwip_rx_loop_handler(void)
//{
//  /* handles all the received frames */
//  while(emac_received_packet_size_get() != 0)
//  {
//    lwip_pkt_handle();
//  }
//}

/**
  * @brief  updates the system local time
  * @param  none
  * @retval none
  */
void time_update(void)
{
  local_time += SYSTEMTICK_PERIOD_MS;
}

//void system_timer_init(void)
//{
//  /* 
//   * TMR6 Configuration (Current implementation)
//   */
//  emac_tmr_init();
//}

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */
