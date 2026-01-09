/* plat.h (plat_info.h)
 */
//#ifndef __IDENTIFY_CORE_OPTS_H__
//#define __IDENTIFY_CORE_OPTS_H__

#if DM_PLAT_TYPE == 0 //(=0) =IF_PLAT_DECAL
/* ethif.h */
void static_ip_welcome_page(void);
uint8_t *identified_tcpip_mac(void);
uint8_t *identified_tcpip_ip(void);
uint8_t *identified_tcpip_gw(void);
uint8_t *identified_tcpip_mask(void);
void identified_dhcp_start(void);
void identified_dhcp_stop(void);

void dm_eth_show_app_help_info(char *projStyle, char *projApplication);
void dm_eth_show_app_modes_info(const char *dhcp_hd, const char *dbg_hd, const char *app0_hd);
void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, const char *clkTypeStr, char *mcu_name);
typedef void (*printkey_ptr)(char *); //hook function
void ap_print_ipconfig(char *head, const uint8_t *mac, printkey_ptr printky);
#endif

#if DM_PLAT_TYPE == IF_PLAT_UIP //(=1)
/**
 **************************************************************************
 * @file     ethif.c
 * @version  v1.0.1
 * @date     2025-10-15
 * @brief    DM9051 Ethernet driver uip interface support file (or referred to be as eth.c)
 **************************************************************************
 *
 * To restructure and improve the file to enhance readability, maintainability,
 * and potentially performance.
 * Last updated: 2025-10-15
 *
 */
//uip's
#include "uip.h"
void static_ip_welcome_page(void)
{
	u16_t ipaddr[2];
    printf("---------------------------------------------\r\n");
    printf("Network chip: DAVICOM DM9051 \r\n");
    printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
			uip_ethaddr.addr[0], uip_ethaddr.addr[1], uip_ethaddr.addr[2],
			uip_ethaddr.addr[3], uip_ethaddr.addr[4], uip_ethaddr.addr[5]);
	uip_gethostaddr(ipaddr);
    printf("Host IP Address: %d.%d.%d.%d \r\n",
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	uip_getnetmask(ipaddr);
    printf("Network Mask: %d.%d.%d.%d \r\n", 
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
	uip_getdraddr(ipaddr);
    printf("Gateway IP Address: %d.%d.%d.%d \r\n", 
			uip_ipaddr1(ipaddr), uip_ipaddr2(ipaddr), uip_ipaddr3(ipaddr), uip_ipaddr4(ipaddr));
    printf("---------------------------------------------\r\n");
}

uint8_t *identified_tcpip_mac(void)
{
	return (uint8_t *) &uip_ethaddr.addr[0];
}

uint8_t *identified_tcpip_ip(void)
{
	return (uint8_t *) uip_hostaddr;
}

uint8_t *identified_tcpip_gw(void)
{
	return (uint8_t *) uip_draddr;
}

uint8_t *identified_tcpip_mask(void)
{
	return (uint8_t *) uip_netmask;
}

void identified_dhcp_start(void)
{
	//ethernetif_dhcp_start(&netif); of uip
}
void identified_dhcp_stop(void)
{
	//ethernetif_dhcp_stop(&netif); of uip
}
#endif

#if DM_PLAT_TYPE == IF_PLAT_LWIP //(=2)
/**
 **************************************************************************
 * @file     ethif_w.c
 * @version  v1.0.1
 * @date     2025-10-15
 * @brief    DM9051 Ethernet driver uip interface support file (or referred to be as eth.c)
 **************************************************************************
 *
 * To restructure and improve the file to enhance readability, maintainability,
 * and potentially performance.
 * Last updated: 2025-10-15
 *
 */
#include "lwip/netif.h"
//extern struct netif netif;
void dm9051_config_if(struct netif *set_netif);
struct netif *dm_eth_netif(void);
void ethernetif_dhcp_start(struct netif *netif);
void ethernetif_dhcp_stop(struct netif *netif);
extern struct netif netif;

void static_ip_welcome_page(void)
{
    printf("---------------------------------------------\r\n");
    printf("Network chip: DAVICOM DM9051 \r\n");
    printf("MAC Address: %02X:%02X:%02X:%02X:%02X:%02X \r\n",
			(uint8_t *)(dm_eth_netif()->hwaddr)[0], (uint8_t *)(dm_eth_netif()->hwaddr)[1], (uint8_t *)(dm_eth_netif()->hwaddr)[2],
			(uint8_t *)(dm_eth_netif()->hwaddr)[3], (uint8_t *)(dm_eth_netif()->hwaddr)[4], (uint8_t *)(dm_eth_netif()->hwaddr)[5]);
    printf("Host IP Address: %d.%d.%d.%d \r\n",
			((uint8_t *)&dm_eth_netif()->ip_addr.addr)[0],
			((uint8_t *)&dm_eth_netif()->ip_addr.addr)[1],
			((uint8_t *)&dm_eth_netif()->ip_addr.addr)[2],
			((uint8_t *)&dm_eth_netif()->ip_addr.addr)[3]);
    printf("Network Mask: %d.%d.%d.%d \r\n",
			((uint8_t *)&dm_eth_netif()->netmask.addr)[0],
			((uint8_t *)&dm_eth_netif()->netmask.addr)[1],
			((uint8_t *)&dm_eth_netif()->netmask.addr)[2],
			((uint8_t *)&dm_eth_netif()->netmask.addr)[3]);
    printf("Gateway IP Address: %d.%d.%d.%d \r\n",
			((uint8_t *)&dm_eth_netif()->gw.addr)[0],
			((uint8_t *)&dm_eth_netif()->gw.addr)[1],
			((uint8_t *)&dm_eth_netif()->gw.addr)[2],
			((uint8_t *)&dm_eth_netif()->gw.addr)[3]);
    printf("---------------------------------------------\r\n");
}

uint8_t *identified_tcpip_mac(void)
{
	//return (uint8_t *)netif.hwaddr;
	return (uint8_t *)(dm_eth_netif()->hwaddr);
}

uint8_t *identified_tcpip_ip(void)
{
	//return (uint8_t *)&netif.ip_addr.addr;
	return (uint8_t *)&(dm_eth_netif()->ip_addr.addr);
}
uint8_t *identified_tcpip_gw(void)
{
	//return (uint8_t *)&netif.gw.addr;
	return (uint8_t *)&(dm_eth_netif()->gw.addr);
}
uint8_t *identified_tcpip_mask(void)
{
	//return (uint8_t *)&netif.netmask.addr;
	return (uint8_t *)&(dm_eth_netif()->netmask.addr);
}

void identified_dhcp_start(void)
{
	//ethernetif_dhcp_start(&netif);
	ethernetif_dhcp_start(dm_eth_netif());
}
void identified_dhcp_stop(void)
{
	//printf("netif_set_link_down(netif)->netif_set_link_callback(ethernetif_notify_conn_changed()).s\r\n");
	//ethernetif_dhcp_stop(&netif);
	ethernetif_dhcp_stop(dm_eth_netif());
	//printf("netif_set_link_down(netif)->netif_set_link_callback(ethernetif_notify_conn_changed()).e\r\n");
}
#endif

#if DM_PLAT_TYPE == 3 //(=3) =IF_PLAT_COMMON
void dm_eth_show_app_help_info(char *projStyle, char *projApplication)
{
	printkey("\r\n\r\n\r\n[%s mode] /%s [%s] %s /%s %s\r\n", 
		hal_active_interrupt_desc(),
		projStyle,
		hal_get_dm9051_date(),
		projApplication,
		hal_get_dm9051_release_num(),
		__DATE__);
}

void dm_eth_show_app_modes_info(const char *dhcp_hd, const char *dbg_hd, const char *app0_hd)
{
	printkey("%s /%s %s\r\n", dhcp_hd, dbg_hd, app0_hd);
}

//void dm_eth_show_app_help_info_ptp(char *statStr, char *clkTypeStr)
//void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, const char *clkTypeStr)
//void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, const char *clkTypeStr, char *mcu_name)
void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, const char *clkTypeStr, char *mcu_name)
{
	printkey("\r\n\r\n\r\n[%s mode][%s][%s][%s] /%s_ptp_daemon_client /R2510 %s\r\n", 
		hal_active_interrupt_desc(), op_modeStr, statStr, clkTypeStr, mcu_name, __DATE__);

//	printkey("\r\n\r\n\r\n[%s mode][%s][%s] /at32f437_ptp_daemon_client /R2412 [v51][emac] %s\r\n", 
//		hal_active_interrupt_desc(), statStr, clkTypeStr, __DATE__);

//	printkey("\r\n\r\n\r\n[%s mode][%s][%s] /at32f403a_ptp_daemon_client /R2507 %s\r\n", 
//		op_modeStr, statStr, clkTypeStr, __DATE__);
}

void ap_print_ipconfig(char *head, const uint8_t *mac, printkey_ptr printky)
{
#if ap_print //defined(__DM9051_AP_DEBUG_H) && ap_print
	uint8_t *addr;
	char buf[100];
	
	sprintf(buf, "%s\r\n", head);
	printky(buf);

	sprintf(buf, "  Network chip: DAVICOM DM9051 \r\n");
	printky(buf);
	sprintf(buf, "  MAC Address: %X:%X:%X:%X:%X:%X \r\n", mac[0], mac[1],
				 mac[2], mac[3], mac[4], mac[5]);
	printky(buf);
	addr = identified_tcpip_ip(); //DM_ETH_Ip_Configured();
	sprintf(buf, "  Host IP Address: %d.%d.%d.%d \r\n", addr[0], addr[1], addr[2], addr[3]);
	printky(buf);
	addr = identified_tcpip_mask(); //DM_ETH_Mask_Configured();
	sprintf(buf, "  Network Mask: %d.%d.%d.%d \r\n", addr[0], addr[1], addr[2], addr[3]);
	printky(buf);
	addr = identified_tcpip_gw(); //DM_ETH_Gw_Configured();
	sprintf(buf, "  Gateway IP Address: %d.%d.%d.%d \r\n", addr[0], addr[1], addr[2], addr[3]);
	printky(buf);
	printky("-----------------------------------------\r\n");
#endif
}
#endif

#undef DM_PLAT_TYPE
//#endif //__IDENTIFY_CORE_OPTS_H__
