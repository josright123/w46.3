/**
  **************************************************************************
  * @file     netconf.h
  * @version  v2.1.2
  * @date     2022-08-16
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

#define LINK_DETECTION                   (1)            /*!< link status detection, 0: no detection, 1: detect with polling */

/* Includes ------------------------------------------------------------------*/
//void tcpip_stack_init(void);
	 
//static void lwip_pkt_handle(void);
void time_update(void);
	 
void _periodic_handle(void);	 
//void lwip_periodic_handle(void);
//void lwip_rx_loop_handler(void);

void tapdev_init(void);
void tapdev_loop(void);
void uip_input_protocol(void);
void uip_timer_protocol(void);

#define MAC_ADDR_LENGTH                  (6)
#define ADDR_LENGTH                      (4)
	 
typedef enum {
	ADOPTE_CONST_MODE = 0,
	ADOPTE_EEPROM_MODE ,
	ADOPTE_FLASH_MODE 
} adopt_mode;

typedef struct dm9051_regist_mac_t {
	adopt_mode adopt_mode;
	int(* mac_f)(void);

	struct uip_eth_addr macaddr;
} dm9051_regist_mac;

typedef struct dm9051_mac_conf_t {
	dm9051_regist_mac cfg;
} dm9051_mac_conf;
	 
#ifdef __cplusplus
}
#endif

#endif /* __NETCONF_H */



