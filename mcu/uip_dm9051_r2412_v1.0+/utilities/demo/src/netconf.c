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
#include "dm9051opts.h"
#include "dm9051_lw.h"
//#include "dm9051_cboard.h"
//#include "dm9051_lw_cint.h"
//#include "dm_eth.h"
#include "uip_d/freertos_tasks_debug.h"
#define printf(fmt, ...) TASK_DM9051_DEBUGF(TASK_SEMAPHORE_NETCONF_ON, /*SEMA_OFF*/ SEMA_ON, "[N]", (fmt, ##__VA_ARGS__))

#if 0
//struct netif netif;
//static uint8_t mac_address1[BOARD_SPI_COUNT][MAC_ADDR_LENGTH] = {
//	{0, 0x60, 0x6e, 0x00, 0x00, 0x33},
//	{0, 0x60, 0x6e, 0x00, 0x00, 0x35},
//	{0, 0x60, 0x6e, 0x00, 0x00, 0x37},
//};
//#if LWIP_DHCP == 0
//static uint8_t local_ip1[BOARD_SPI_COUNT][ADDR_LENGTH]   = {
//	{192, 168, 6,  33},
//	{192, 168, 6,  35},
//	{192, 168, 6,  37},
//};
//static uint8_t local_gw1[BOARD_SPI_COUNT][ADDR_LENGTH]   = {
//	{192, 168, 6,   1},
//	{192, 168, 6,   1},
//	{192, 168, 6,   1},
//};
//static uint8_t local_mask1[BOARD_SPI_COUNT][ADDR_LENGTH] = {
//	{255, 255, 255, 0},
//	{255, 255, 255, 0},
//	{255, 255, 255, 0},
//};
//#endif

//static void ethernetif_notify_conn_changed(struct netif *netif)
//{
//}

//void ethernetif_update_config(struct netif *netif)
//{
//  ethernetif_notify_conn_changed(netif);
//}

//void ethnode_config(ip4_addr_t *ipaddr, ip4_addr_t *netmask, ip4_addr_t *gw)
//{
//}

//void tcpip_stack_init(void)
//{
//}

//void net_task_freertos_init_led(uint16_t link, int link_change) {
//}

//static void net_task_freertos_led_flash(uint32_t ms, uint32_t totaldlyms) { //5,1, (2000)(500)(250);
//	static int j = 0;
//	static int k = 0;
//	static uint32_t ledtimes = 0;

//	vTaskDelay(pdMS_TO_TICKS(ms)); //[essential] yield the cpu, with this dly API.
//}

//void net_task_freertos_link_handler_and_init_led(uint32_t ms) {
//	uint16_t regvalue = ethernetif_link(&netif);
//	net_task_freertos_init_led(regvalue, link_change_flg);
//	if (netconf_is_link_up()) {
//		static int ssww = 0;
//		if (link_startup_update_flg == 0) {
//			uint16_t bmsr = _dm9051_bmsr_update();
//			printf("--- [wait a link up ! ] %04x ---\r\n", bmsr);
//			if (++ssww == 7) {
//				/* "--- [while(1) infinite not-work loop.] ---" */
//				while(1)
//					net_task_freertos_led_flash(/*NET_TASK_DLY, 1, 3*/ ms, NUM_COUNT_CYC);
//			}
//			return;
//		}
//	}
//}

//void net_task_freertos_recv_and_led(int rx_flg, uint32_t ms)
//{
//  if (_netconf_is_link_up()) {
//#if 1
//	if (!_intr_gpio_mptr()) { //pooling
//#endif
//		if (rx_flg) {
//		  err_t err = lwip_pkt_handle(); //net_tcpip_thread();
//		}
//	}
//  }
//}

///* x2web */
//err_t rx_direct0(void) {
//	return ethernetif_inp(&netif); //= err = ethernetif_input(&netif);
//}

///**
//  * @brief  called when a frame is received
//  * @param  none
//  * @retval none
//  */
//err_t lwip_pkt_handle(void)
//{
//  return ethernetif_input(&netif);
//}

//#define DM9051_RX_DBGMSG(expression, message) do { if ((expression)) { \
//  DM9051_DIAG(message);}} while(0)
//  
//err_t rx_direct(struct netif *netif) {
//	err_t err;
//	err = ethernetif_inp(netif); //= lwip_rx_hdlr(&xnetif[i]);
//	DM9051_RX_DBGMSG(err != ERR_OK && err != ERR_INPROGRESS, ("[netconf.c] ethernetif_input: IP input error\r\n"));
//	return err;
//}

//void rx_handler_direct(int i, confirm_state directFlag) {
//	if (netconf_is_link_up()) {
//		rx_direct(&netif);
//	}
//}
//#endif

//void lwip_periodic_handle(volatile uint32_t localtime)
//{
//}
#endif
