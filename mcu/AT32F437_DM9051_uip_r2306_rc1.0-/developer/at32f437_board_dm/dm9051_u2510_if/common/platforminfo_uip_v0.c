/**
 **************************************************************************
 * @file     platform_info_v0.c
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
#include "stdint.h"
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "core/dm9051.h"
#include "uip.h"

#define DM9051_FLAG_LINK_UP					 0x01U
#define dm9051_set_flags(flg, set_flags)     do { flg = (uint8_t)(flg | (set_flags)); } while(0)
#define dm9051_clear_flags(flg, clr_flags)   do { flg = (uint8_t)(flg & (uint8_t)(~(clr_flags) & 0xff)); } while(0)
#define dm9051_is_flag_set(flg, flag)        ((flg & (flag)) != 0)
static uint8_t lw_flag = 0;
int tracing_init_downup = 1;

void print_up(char *up_head)
{
	uint8_t *ipaddr = (uint8_t *) uip_hostaddr; //identified_tcpip_ip(); //uip_gethostaddr(ipaddr);
	printf("[%s mode] (%s) %d.%d.%d.%d\r\n",
		hal_active_interrupt_desc(), up_head,
		ipaddr[0], ipaddr[1], ipaddr[2], ipaddr[3]);
}
void print_down(char *down_head)
{
	printf("[%s mode] (%s)\r\n",
		hal_active_interrupt_desc(), down_head);
}

void dm9051_debug_flags(char *headstr)
{
	//if (lw_flag & DM9051_FLAG_LINK_UP)
	//	link_reset = 1;

	if (headstr) {
		printf("%s: dm9051 driver lw_flag %s\r\n", headstr, 
				(lw_flag & DM9051_FLAG_LINK_UP) ? "linkup" : "linkdown");
		return;
	}
	printf("dm9051 driver lw_flag %s\n", (lw_flag & DM9051_FLAG_LINK_UP) ? "linkup" : "linkdown");
}

void platform_if_notify_conn_changed(void)
{
	if (tracing_init_downup) {
		if (dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP)) { //platform_is_link_up()
			print_up("init to link up");
//			identified_dhcp_start();
//			static_ip_welcome_page();
		} else {
			print_down("init to link down");
			//identified_dhcp_stop();
		}
		return;
	}

	if (dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP)) { //platform_is_link_up()
		print_up("down to link up");
//		identified_dhcp_start();
	} else {
		print_down("up2down to link down");
		printf("->lwip_periodic_link(localtime, NET_DETECT_UIP)->platform_set_link_down().s\r\n");
//		identified_dhcp_stop();
		printf("->lwip_periodic_link(localtime, NET_DETECT_UIP)->platform_set_link_down().e\r\n");
	}
	return;
}

static void dm9051_update_flags(char *querystr, int linkup)
{
	//.if (linkup)
	//.	printf("%s: dm9051_lw - DM9051 linkup\r\n", querystr);
	//.else
	//.	printf("%s: dm9051_lw - DM9051 linkdown\r\n", querystr);
	
	if (linkup)
		dm9051_set_flags(lw_flag, DM9051_FLAG_LINK_UP);
	else
		dm9051_clear_flags(lw_flag, DM9051_FLAG_LINK_UP);

	dm9051_debug_flags(querystr);
	platform_if_notify_conn_changed();
	tracing_init_downup = 0;
}

/**
  * @brief  updates the link states
  * @param  headstr
  * @retval bmsr 7649: disconnect, 764d: connection, usually
  */
void dm9051_update_phyread(char *queryStr)
{
	uint8_t stat[6];
	dm9051_read_regs_info(stat);
	//uint16_t bmsr = phy_read(PHY_STATUS_REG);
	//uint16_t bmsr = stat[4] << 8 | stat[5]; /* BMSR */
	int now_link_stat = stat[5] & 0x04 ? 1 : 0;  /* BMSR register */
		
	if ((!dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP)) && now_link_stat)
		dm9051_update_flags(queryStr, now_link_stat);
	else if (dm9051_is_flag_set(lw_flag, DM9051_FLAG_LINK_UP) && !now_link_stat)
		dm9051_update_flags(queryStr, now_link_stat);

	//return bmsr;
}
