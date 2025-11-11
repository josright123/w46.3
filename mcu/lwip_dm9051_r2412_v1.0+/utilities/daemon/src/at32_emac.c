/**
  **************************************************************************
  * @file     at32_emac.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    emac config program
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

/* includes ------------------------------------------------------------------*/
#include "at32f435_437_board.h"

#include "netif.h"
void ethernetif_notify_conn_changed(struct netif *netif);
//#include "lwip/dhcp.h"
//#include "at32_emac.h"

//#include "platform_info/nosys/uip_eth/eth_api.h"

/** @addtogroup AT32F437_periph_examples
  * @{
  */

/** @addtogroup 437_EMAC_telnet
  * @{
  */

#if 0

emac_control_config_type mac_control_para;

/**
  * @brief  enable emac clock and gpio clock
  * @param  none
  * @retval success or error
  */
error_status emac_system_init(void)
{
}

/**
  * @brief  configures emac irq channel.
  * @param  none
  * @retval none
  */
void emac_nvic_configuration(void)
{
}

/**
  * @brief  configures emac required pins.
  * @param  none
  * @retval none
  */
void emac_pins_configuration(void)
{
}

/**
  * @brief  configures emac layer2
  * @param  none
  * @retval error or success
  */
error_status emac_layer2_configuration(void)
{
}

/**
  * @brief  reset layer 1
  * @param  none
  * @retval none
  */
void static reset_phy(void)
{
}

/**
  * @brief  reset phy register
  * @param  none
  * @retval SUCCESS or ERROR
  */
error_status emac_phy_register_reset(void)
{
}

/**
  * @brief  set mac speed related parameters
  * @param  nego: auto negotiation on or off.
  *         this parameter can be one of the following values:
  *         - EMAC_AUTO_NEGOTIATION_OFF
  *         - EMAC_AUTO_NEGOTIATION_ON.
  * @param  mode: half-duplex or full-duplex.
  *         this parameter can be one of the following values:
  *         - EMAC_HALF_DUPLEX
  *         - EMAC_FULL_DUPLEX.
  * @param  speed: 10 mbps or 100 mbps
  *         this parameter can be one of the following values:
  *         - EMAC_SPEED_10MBPS
  *         - EMAC_SPEED_100MBPS.
  * @retval none
  */
error_status emac_speed_config(emac_auto_negotiation_type nego, emac_duplex_type mode, emac_speed_type speed)
{
}

/**
  * @brief  initialize emac phy
  * @param  none
  * @retval SUCCESS or ERROR
  */
error_status emac_phy_init(emac_control_config_type *control_para)
{
}

/**
  * @brief  updates the link states
  * @param  none
  * @retval link state 0: disconnect, 1: connection
  */
//uint16_t link_update(void)
//{
//}

/**
  * @brief  this function notify user about link status changement.
  * @param  netif: the network interface
  * @retval none
  */
void ethernetif_notify_conn_changed(struct netif *netif)
{
  /* note : this is function could be implemented in user file 
            when the callback is needed,
  */

  if (netif_is_link_up(netif)) {
    netif_set_up(netif);

#if LWIP_DHCP
    /*  creates a new dhcp client for this interface on the first call.
    note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
    the predefined regular intervals after starting the client.
    you can peek in the netif->dhcp struct for the actual dhcp status.*/
    dhcp_start(netif);
#endif
  }
  else
    netif_set_down(netif);
}

/**
  * @brief  link callback function, this function is called on change of link status
  *         to update low level driver configuration.
  * @param  netif: the network interface
  * @retval none
  */
void ethernetif_update_config(struct netif *netif)
{
  if(netif_is_link_up(netif))
  { 
    emac_speed_config(mac_control_para.auto_nego, mac_control_para.duplex_mode, mac_control_para.fast_ethernet_speed);
    delay_ms(300);
    /* enable mac and dma transmission and reception */
    emac_start();
  }
  else
  {
    /* disable mac and dma transmission and reception */
    emac_stop(); 
  }

  ethernetif_notify_conn_changed(netif);
}

/**
  * @brief  initialize tmr6 for emac
  * @param  none
  * @retval none
  */
void emac_tmr_init(void)
{
}

/**
  * @brief  configure timers
  * @param  none
  * @retval none
  */
void tmr_configuration(void)
{
}

// ---------------------- hw_impl -------------------------------------------------------------
void emac_pps_lo(int tcount)
{
}

void emac_pps_hi(int tcount)
{
}

void emac_led_periodic_put_bump(void)
{
}

void emac_led_periodic_put_handle(volatile uint32_t localtime)
{
}

void emac_pps_out(void)
{
}
#endif

/**
  * @brief  updates the link states
  * @param  none
  * @retval link state 0: disconnect, 1: connection
  */
//uint16_t link_update(void)
//{
//  uint8_t statdat[6];
//  DM_Eth_Read_Info(statdat);
//  return DM_Eth_Info_Linkup(statdat);
//}

/**
  * @brief  this function sets the netif link status.
  * @param  netif: the network interface
  * @retval none
  */  
//xxx ethernetif_set_link(void const *argument)
//{
//  uint16_t regvalue = 0;
//  struct netif *netif = (struct netif *)argument;
//  /* read phy_bsr*/
//  regvalue = link_update();
//  /* check whether the netif link down and the phy link is up */
//  if(!netif_is_link_up(netif) && (regvalue))
//  {
//    /* network cable is connected */ 
//    netif_set_link_up(netif);        
//  }
//  else if(netif_is_link_up(netif) && (!regvalue))
//  {
//    /* network cable is dis-connected */
//    netif_set_link_down(netif);
//  }
//}

/**
  * @brief  link callback function, this function is called on change of link status
  *         to update low level driver configuration.
  * @param  netif: the network interface
  * @retval none
  */
void ethernetif_update_config(struct netif *netif)
{
//  if(netif_is_link_up(netif))
//  { 
//    netif_set_up(netif);

//#if LWIP_DHCP
//    /*  creates a new dhcp client for this interface on the first call.
//    note: you must call dhcp_fine_tmr() and dhcp_coarse_tmr() at
//    the predefined regular intervals after starting the client.
//    you can peek in the netif->dhcp struct for the actual dhcp status.*/
//    dhcp_start(netif);
//#endif
//  }
//  else
//  {
//    netif_set_down(netif);
//    printf("Link down\r\n");
//  }
//=
  ethernetif_notify_conn_changed(netif);
}

/**
  * @}
  */

/**
  * @}
  */
