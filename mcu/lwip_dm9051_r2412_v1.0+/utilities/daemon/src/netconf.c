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
#include "lwip/tcpip.h"
#include "lwip/init.h"
#include "ethernetif_v51.h"
#include "ethernetif.h"
#include "netconf.h"
#include "stdio.h"
#include "at32_emac.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "../../dm9051_u2510_if/ip_status.h"

//#include "platform_info/control/cdef.h" //of #include "control/conf.h"
//#include "platform_info/nosys/nosys_control/conf_ap.h"

//#include "platform_info/nosys/nosys_control/dm9051_ap_debug.h"

/* eth api */
//#include "platform_info/nosys/uip_eth/types_define_all.h"
//#include "platform_info/nosys/uip_eth/eth_api.h"

#include "core/dm9051_constants.h" //[temp here!]

static uint8_t mac_address1[1][MAC_ADDR_LENGTH] = {
	{0, 0x60, 0x6e, 0x00, 0x00, 0x33},
//	{0, 0x60, 0x6e, 0x00, 0x00, 0x35},
//	{0, 0x60, 0x6e, 0x00, 0x00, 0x37},
};
#if LWIP_DHCP == 0
//static uint8_t local_ip1[1][ADDR_LENGTH]   = {
//	//{192, 168, 1,  33},
//	{192, 168, 6,  33},
////	{192, 168, 6,  35},
////	{192, 168, 6,  37},
//};
//static uint8_t local_gw1[1][ADDR_LENGTH]   = {
////	{192, 168, 1,   254},
//	{192, 168, 6,   1},
////	{192, 168, 6,   1},
////	{192, 168, 6,   1},
//};
//static uint8_t local_mask1[1][ADDR_LENGTH] = {
//	{255, 255, 255, 0},
////	{255, 255, 255, 0},
////	{255, 255, 255, 0},
//};
#endif

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
/* Private variables ---------------------------------------------------------*/
//extern struct netif netif_emac;

/* Network Configuration */
//const struct ip_node_t ip_candidate[1] = {
//	{
//		//{0, 0x60, 0x6e, 0x00, 0x00, 0x17},
//		{192, 168, 6, 17},
//		{192, 168, 6, 1},
//		{255, 255, 255, 0},
//	},

/* Private function prototypes -----------------------------------------------*/
/* Private functions ---------------------------------------------------------*/

#if 1 //[temp here]
#define OVERSIZE_LEN			PBUF_POOL_BUFSIZE //#define _PBUF_POOL_BUFSIZE 1514 //defined in "lwipopts.h" (JJ20201006)
#define RXBUFF_OVERSIZE_LEN		(OVERSIZE_LEN+2)
union {
	uint8_t rx;
	uint8_t tx;
} EthBuff[RXBUFF_OVERSIZE_LEN]; //[Single Task project.] not occupied by concurrently used.

uint8_t *get_ReceiveBuffer(void)
{
	return &EthBuff[0].rx;
}

uint8_t *get_TransmitBuffer(void)
{
	return &EthBuff[0].tx;
}

/* ethernetif_v51.c */
//extern struct netif netif_v51;

extern const uint8_t *lwip_set_MACaddr; //MACaddr[6];

/* Use 'DM_ETH_Input' and then 'dm9051_rx' */
#if 0
//struct pbuf *input_intr(void) {
////struct netif *netif = tcpip_stack_netif();
//struct pbuf *p;
////LOCK_TCPIP_CORE();
//p = DM_ETH_Input_W(); //= low_level_input(netif);
////UNLOCK_TCPIP_CORE();
//return p;
//}
#endif

/* netconf1.c */
void ethnode_config(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw)
{
//	const uint8_t *ipn;
//	const uint8_t *gwn;
//	const uint8_t *maskn;
  int pin = 0;

#if LWIP_DHCP  //need DHCP server
  ipaddr->addr = 0;
  gw->addr = 0;
  netmask->addr = 0;
#else
	
//	ipn = DM_ETH_Ip_Configuration(local_ip1[pin]);
//	gwn = DM_ETH_Gw_Configuration(local_gw1[pin]);
//	maskn = DM_ETH_Mask_Configuration(local_mask1[pin]);

//  IP4_ADDR(ipaddr, ipn[0], ipn[1], ipn[2], ipn[3]);
//  IP4_ADDR(gw, gwn[0], gwn[1], gwn[2], gwn[3]);
//  IP4_ADDR(netmask, maskn[0], maskn[1], maskn[2], maskn[3]);

    IP4_ADDR(ipaddr, local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
    IP4_ADDR(netmask, local_mask[0], local_mask[1], local_mask[2], local_mask[3]);
    IP4_ADDR(gw, local_gw[0], local_gw[1], local_gw[2], local_gw[3]);
#endif

  lwip_set_MACaddr = mac_address1[pin]; //as= lwip_set_mac_address(mac_address1[pin]);
}

#if 0
	void tcpip_stack_init(void)
	{
	  struct netif *netif = tcpip_stack_netif();
	  ...
	}
#endif
#endif

void stack_tcpip_init_v51(void)
{
#if !NO_SYS
  tcpip_init(NULL,NULL);
#else
  lwip_init();
#endif
}

//void stack_set_default(void)
//{
//#if 1
//#if 1
//  /*  Registers the default network interface.*/
//  netif_set_default(&netif_v51);
//	//..# else
//  /*  Registers the default network interface.*/
//  netif_set_default(&netif_emac);
//#endif
//#endif
//}

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

#if 1
#if RXHDL_V51 == RXHDL_OLD
bool Receive_handle_v51(void);
int DM_ETHER_Receive_Route_W0(void) {
	int nRx = 0;
	//int sema_event;
	if (hal_active_interrupt_mode()) {
		//#ifdef ETHERNET_INTERRUPT_MODE
		if (dm9051_interrupt_get()) { //as DM_ETH_IRQEvent();
		//#endif
			while(Receive_handle_v51()) {
				nRx++;
			}
			display_input_mode(nRx);
			dm9051_interrupt_reset(); //cspi_disble_irq(); //dm9051_isr_enab(); //DM_ETH_ToRst_ISR();
			return 1;
		//#ifdef ETHERNET_INTERRUPT_MODE
		}
		//#endif
		return 0;
	}

	//else {
	//}=
	while(Receive_handle_v51()) {
		nRx++;
	}
	display_input_mode(nRx);
	return 1;
}
#endif
#endif
#if 1
#if RXHDL_V51 == !RXHDL_OLD
/**
  * @brief  called when a frame is received
  * @param  none
  * @retval none
  */
bool lwip_pkt_handle_v51(void)
{
	struct netif *netif = tcpip_stack_netif();

    #if (rt_print | drv_print)
    static uint16_t gmdra_rds;
    diff_rx_pointers_s(&gmdra_rds);
    #endif
    
    err_t err = ethernetif_input(netif);
    if (err == ERR_OK) {
        #if (rt_print | drv_print)
        diff_rx_pointers_e(&gmdra_rds, 1);
        #endif
        return true;
    }
    return false;
}

int DM_ETHER_Receive_Route_W1(void) {

	if (dm9051_interrupt_get()) {
		int nRx = 0;

		while(lwip_pkt_handle_v51()) {
#if LWIPERF_APP
			lwip_periodic_handle(0); //param: 0, is to use with special ,only sys_check_timeouts(); in lwip_periodic_handle()
#endif
			nRx++;
		}

		display_input_mode(nRx);
		dm9051_interrupt_reset();
		return 1;
	}
	return 0;
}
#endif
#endif

/**
  * @}
  */

/**
  * @}
  */
/**
  * @}
  */
