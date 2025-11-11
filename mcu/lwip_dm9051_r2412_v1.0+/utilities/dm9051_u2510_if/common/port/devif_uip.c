#include "stdio.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"

/** MAC Filter Actions, these are passed to a netif's igmp_mac_filter or
 * mld_mac_filter callback function. */
#if !defined(LWIP_HDR_NETIF_H)
enum netif_mac_filter_action {
  /** Delete a filter entry */
  NETIF_DEL_MAC_FILTER = 0,
  /** Add a filter entry */
  NETIF_ADD_MAC_FILTER = 1
};
#endif //LWIP_HDR_NETIF_H

/* Global variables ---------------------------------------------------------*/
//int eth_input_mode; //return from "dm9051_conf()"

void DM_ETH_Init(const uint8_t *adr)
{
	/*eth_input_mode =*/ 
	dm9051_conf(); /*dm9051_boards_initialize();*/ \
	dm9051_init(adr);
}

int DM_ETH_Init_mode(void)
{
	//return eth_input_mode;
	return hal_active_interrupt_mode();
}

void tx_manager_dispatch(uint8_t *buf, uint16_t len)
{
	dm9051_tx(buf, len);
}

uint16_t rx_manager_dispatch(uint8_t *buf)
{
	uint16_t len = dm9051_rx(buf); //dm9051dev.rx(buffer);
	if (len) {
		dump_data(buf, len);
	}
	return len;
}

/* IGMP callback for joining/leaving multicast groups */
int rx_igmp_mac_filter(void *arg, uint32_t group, enum netif_mac_filter_action action)
{
    switch (action) {
        case NETIF_ADD_MAC_FILTER:
			dm9051_igmp_ctrl(group, 1); //dm9051_rx_mode_add_hash(group);
            break;
            
        case NETIF_DEL_MAC_FILTER:
			dm9051_igmp_ctrl(group, 0); //dm9051_rx_mode_del_hash(group);
            break;
        default:
            return 1;
    }
    return 0;
}
