/**
 **************************************************************************
 * @file     platform_info.c
 * @version  v1.0.1
 * @date     2024-12-12
 * @brief    DM9051 Ethernet driver info file
 **************************************************************************
 *
 * To restructure and improve the file to enhance readability, maintainability,
 * and potentially performance.
 * Last updated: 2024-12-12
 *
 */
#include "../../dm9051_u2510_if/if.h"
#include "../../dm9051_u2510_if/ip_status.h"

/* Check LINK state from ops in source 
 * Use [NSR/NSR] 0
 * Use [BMSR]    1
 */
enum link_state_t {DM9051_CHECK_MAC = 0, DM9051_CHECK_PHY};

#define LINK_STATE_SOURCE 	DM9051_CHECK_MAC

/**
 * @brief  Checks if link is up based on configured source
 * @param  stat: Status register values
 * @return 1 if link is up, 0 otherwise
 */
int DM_Eth_Info_Linkup(uint8_t *stat)
{
	enum link_state_t lst = LINK_STATE_SOURCE;
	if (lst == DM9051_CHECK_MAC) {
		return stat[1] & 0x40 ? 1 : 0;  /* NSR register */
	}
	else if (lst == DM9051_CHECK_PHY) {
		return stat[5] & 0x04 ? 1 : 0;  /* BMSR register */
	}
	return 1;
}

/**
 * @brief  Reads and processes register information (could periodic call)
 * @param  stat: Buffer for register values
 */
void DM_Eth_Read_Info(uint8_t *stat)
{
  dm9051_read_regs_info(stat);
  /* Resets the hex dump state for input processing */
  if (!DM_Eth_Info_Linkup(stat))
	dm_eth_input_hexdump_reset();
}

#define SD_100M 0x20
#define SD_10M 0x10
#define SD_FULL 0x02
#define SD_HALF 0x01
static uint8_t speed_duplex = 0;

// Show Ethernet status
void DM_Eth_Show_status_W(char *head, uint8_t *statdat, int force)
{
	uint8_t speeddupd = 0;

	if (DM_Eth_Info_Linkup(statdat))
	{
		if (statdat[0] & 0x08)
			speeddupd |= SD_FULL;
		else
			speeddupd |= SD_HALF;
		if (statdat[1] & 0x80)
			speeddupd |= SD_10M;
		else
			speeddupd |= SD_100M;

		if ((speeddupd != speed_duplex) || force)
		{
			speed_duplex = speeddupd;

			printf("%s Link up as %s %s\r\n",
				   head,
				   speeddupd & SD_100M ? "100M" : speeddupd & SD_10M ? "10M"
																	 : "UN_SPEED",
				   speeddupd & SD_FULL ? "Full" : speeddupd & SD_HALF ? "Half"
																	  : "UN_DUPLEX");
		}
		printk("\r\n");
	}
}

int tracing_init_downup = 1;
static int link_stat = 0;
int platform_is_link_up(void) {
	return link_stat;
}
void platform_set_link_up(void) {
	/*
	 * if (!platform_is_link_up())
	 *   netif_set_up(); ---> netif_set_flags()
	 */
	link_stat = 1; //netif_set_flags()
}
void platform_set_link_down(void) {
	/*
	 * if (platform_is_link_up())
	 *   netif_set_down(); ---> netif_clear_flags()
	 */
	link_stat = 0; //netif_clear_flags()
}

//#define head_init_up		"init to link up"
//#define head_init_down		"init to link down"
//#define periodic_link_up	"down to link up"
//#define periodic_link_down	"up2down to link down"

void print_up(char *up_head)
{
	uint8_t *ipaddr = identified_tcpip_ip(); //uip_gethostaddr(ipaddr);
	printf("[%s mode] (%s) %d.%d.%d.%d\r\n",
		hal_active_interrupt_desc(), up_head,
		ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
}
void print_down(char *down_head)
{
	printf("[%s mode] (%s)\r\n",
		hal_active_interrupt_desc(), down_head);
}

void platform_if_notify_conn_changed(void)
{
	if (tracing_init_downup) {
		if (platform_is_link_up()) {
			print_up("init to link up");
			identified_dhcp_start();
		} else {
			print_down("init to link down");
			//identified_dhcp_stop();
		}
		return;
	}

	if (platform_is_link_up()) {
		print_up("down to link up");
		identified_dhcp_start();
	} else {
		print_down("up2down to link down");
		printf("->lwip_periodic_link(localtime, NET_DETECT_UIP)->platform_set_link_down().s\r\n");
		identified_dhcp_stop();
		printf("->lwip_periodic_link(localtime, NET_DETECT_UIP)->platform_set_link_down().e\r\n");
	}
	return;
}

/**
 * @brief  = dm_eth_polling_downup()
 * @return = 1, only if down2up
 */
int dm_eth_polling_downup(int dhcp_en)
{
	uint8_t statdat[6];
	int now_link_stat;

	DM_Eth_Read_Info(statdat); //.....
	now_link_stat = DM_Eth_Info_Linkup(statdat) ? 1 : 0;

	if (tracing_init_downup) {
		if (now_link_stat) {
			platform_set_link_up();
			platform_if_notify_conn_changed();
			if (!dhcp_en)
				static_ip_welcome_page();
		} else {
			platform_set_link_down();
			platform_if_notify_conn_changed();
		}
		tracing_init_downup = 0;
		return 0; //now_link_stat;
	}

	if (!platform_is_link_up() && now_link_stat) {
		platform_set_link_up();
		platform_if_notify_conn_changed();
		return 1;
	} else if (platform_is_link_up() && !now_link_stat) {
		platform_set_link_down();
		platform_if_notify_conn_changed();
	}
	return 0; //now_link_stat;
}

//-----------------------------------------------------------------------------------------------------
//#include "netconf.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/dhcp.h"
uint16_t link_display(uint16_t regvalue, struct netif *netif);
//void dm_wait2_periodic_stat(void);

void ethernetif_dhcp_start(struct netif *netif) //or netif_dhcp_start(), or ethernetif_dhcp_start(), or platform_dhcp_start()
{
#if LWIP_DHCP
	printf("(up) notify\r\n");
	printf("----------------------------- dhcp_start(netif)  ------------------ \r\n");
	printf(" LINK-UP, on link_changed, Call dhcp_start(netif)\r\n");
	dhcp_start(netif);
	#if 1 //state = DM_ENUM_COMBINED_WAIT_NET;
//	dm_wait2_periodic_stat();
	#endif
#else
	printf("[%s mode] (up %d.%d.%d.%d) notify\r\n",
		hal_active_interrupt_desc(),
		ip4_addr1(&netif->ip_addr), ip4_addr2(&netif->ip_addr),
		ip4_addr3(&netif->ip_addr), ip4_addr4(&netif->ip_addr));
#endif
}

void ethernetif_dhcp_stop(struct netif *netif)
{
#if LWIP_DHCP
	dhcp_stop(netif);
	printf("/* dhcp STOP all\r\n");
	printf(" * LINK DOWN Found!!\r\n");
	printf(" */\r\n");
#endif
}

/**
 * @brief  Notify user about link status changes
 * @note   Handles network interface state changes
 * @param  netif: pointer to network interface structure
 * @retval none
 */
void ethernetif_notify_conn_changed(struct netif *netif)
{
    if (netif_is_link_up(netif)) {
        netif_set_up(netif);
		ethernetif_dhcp_start(netif);
    } else {
		printf("(down)\r\n");
        netif_set_down(netif);
		printf("netif_set_link_down(netif)->netif_set_link_callback(ethernetif_notify_conn_changed()).s\r\n");
		ethernetif_dhcp_stop(netif);
		printf("netif_set_link_down(netif)->netif_set_link_callback(ethernetif_notify_conn_changed()).e\r\n");
    }
}

/**
 * @brief  Handle periodic display for lwip tasks
 * @note   Manages DHCP startup and link status display
 * @param  head: display header string
 * @param  regvalue: link status register value
 * @param  netif: pointer to network interface structure
 * @retval none
 */
static int dhcp_start_first_flag = 1;
static void lwip_periodic_display_handle(char *head, uint16_t regvalue, struct netif *netif)
{
	int on_dhcp = LWIP_DHCP;
	
    if (dhcp_start_first_flag) {
#if !LWIP_DHCP
        if (regvalue) {
            printf("%s link %s\r\n", head, "up");
            printf("local_ip %d.%d.%d.%d\r\n", 
                   local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
			if (!on_dhcp)
				static_ip_welcome_page();
            dhcp_start_first_flag = 0;
        } else {
            printf("%s link %s\r\n", head, "down");
        }
        if (dhcp_start_first_flag == 1) {
            printf("local_ip %d.%d.%d.%d\r\n", 
                   local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
            dhcp_start_first_flag = 2;
        }
#endif
#if LWIP_DHCP
        if (regvalue) {
            printf("%s link %s\r\n", head, "up");

			if (netif_is_up(netif)) {
				if (dhcp_start_first_flag) {
					printf("%s link %s (netif is up, dhcp_start())\r\n", head, "up");
printf("--------------------------- dhcp_start(netif)  ------------------ \r\n");
printf(" LINK-UP, on lwip_periodic_display_handle, Call dhcp_start(netif)\r\n");
					dhcp_start_first_flag = 0;
					dhcp_start(netif);
				}
			}
			else
				printf("%s link %s (!netif is not change to up, NOT dhcp_start())\r\n", head, "up");
        } else {
            printf("%s link %s\r\n", head, "down");
        }
#endif
    }
}

/**
 * @brief  Display link status and control LEDs
 * @note   Updates LED indicators based on link status
 * @param  regvalue: link status register value
 * @param  netif: pointer to network interface structure
 * @retval link status value
 */
#define LINK_TIMER_MAX                   (0xf0000000/500000)
uint16_t link_display(uint16_t regvalue, struct netif *netif)
{
    static uint32_t mc_timer = 0;
    if (mc_timer < LINK_TIMER_MAX) {
        mc_timer++;
        if (mc_timer == 1) {
            lwip_periodic_display_handle("startup", regvalue, netif);
        } else {
            lwip_periodic_display_handle("mainloop", regvalue, netif);
        }
    }
    return regvalue;
}

/**
 * @brief = _dm_eth_polling_downup()
 * @brief  Set network interface link status
 * @note   Manages the netif link status based on physical link state
 * @param  argument: pointer to network interface structure
 * @retval 1 if link status changed to up, 0 otherwise
 */
int ethernetif_set_link(void const *argument)
{
	int on_dhcp = LWIP_DHCP;
    struct netif *netif = (struct netif *)argument;
    //int linkchg_up = 0;
	uint16_t regvalue = dm9051_link_update();
	link_display(regvalue, netif);
	if (!netif_is_link_up(netif) && regvalue) {
		netif_set_link_up(netif);
		//printf("dm9051 link up\r\n");
		printf("%s dm9051 link up\r\n", on_dhcp ? "DHCP" : "static IP");
		//#if _LWIP_MQTT
		//#endif
		return 1; //linkchg_up = 1;
	} else if (netif_is_link_up(netif) && !regvalue) {
		netif_set_link_down(netif);
		//printf("dm9051 link down\r\n");
		printf("%s dm9051 link down\r\n", on_dhcp ? "DHCP" : "static IP");
		//#if _LWIP_MQTT
		//#endif
	}
    return 0; //return linkchg_up;
}
