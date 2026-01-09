//#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "lwip/netif.h"

/* Global variables ---------------------------------------------------------*/
//extern int input_mode;

const uint8_t *DM_ETH_Init_W(const uint8_t *adr)
{
	/*input_mode =*/
	dm9051_conf();
	return dm9051_init(adr);
}

int DM_ETH_Init_mode_W(void)
{
	//return input_mode;
	return hal_active_interrupt_mode();
}

void tx_manager_dispatch_w(uint8_t *buffer, uint16_t len)
{
	dm9051_tx(buffer, len);
}

uint16_t rx_manager_dispatch_w(uint8_t *buf)
{
	uint16_t len = dm9051_rx(buf);
	if (len) {
		dump_data(buf, len); //dm_eth_input_hexdump(buf, len);
	}
	return len;
}

/* IGMP callback for joining/leaving multicast groups */
err_t rx_igmp_mac_filter_w(struct netif *netif, const ip4_addr_t *group, enum netif_mac_filter_action action)
{
    switch (action) {
        case NETIF_ADD_MAC_FILTER:
			dm9051_igmp_ctrl(group->addr, 1); //dm9051_rx_mode_add_hash(group->addr);
            break;
            
        case NETIF_DEL_MAC_FILTER:
			dm9051_igmp_ctrl(group->addr, 0); //dm9051_rx_mode_del_hash(group->addr);
            break;
        default:
            return ERR_ARG; //return 1;
    }
    return ERR_OK; //return 0;
}
