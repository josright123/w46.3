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
			static_ip_welcome_page();
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

#if DM_ETH_DEBUG_MODE && 0
/* Buffer Management */
#define RX_BUFFER_START       0xC00
#define RX_BUFFER_END         0x4000
static int fifoTurn_n = 0; //...
uint16_t gkeep_mdra_rds;

uint16_t wrpadiff(uint16_t rwpa_s, uint16_t rwpa_e)
{
    return (rwpa_e >= rwpa_s) ? 
           rwpa_e - rwpa_s : 
           (rwpa_e + RX_BUFFER_END - RX_BUFFER_START) - rwpa_s;
}

void DM_ETH_Read_mdra(uint16_t *mdra_rd_now)
{
  uint16_t dummy_rwpa;
  dm9051_read_rx_pointers(&dummy_rwpa, mdra_rd_now);
}

/**
 * @brief  Debug function for RX pointer calculation
 */
#if DM_ETH_DEBUG_MODE
	static void debug_diff_rx_pointers(int state, uint16_t rd_now)
	{
	#if (defined(__DM9051_ETH_DEBUG_H) && drv_print)  || (defined(__DM9051_AP_DEBUG_H) && ap_print) //org 'drv_print'
		if (fifo_premdra_rd == DEFAULT_MDRA_RD)
			fifo_mdra_rd = rd_now;

		if (state)
			fifo_fifoTurn_n++;

		if (rd_now < fifo_premdra_rd && (fifo_premdra_rd != DEFAULT_MDRA_RD))
		{
			uint16_t compos_totaldiff = (rd_now >= fifo_mdra_rd) ? TOTAL_DIFF_OFFSET : 0;
			uint16_t diff = wrpadiff(fifo_mdra_rd, rd_now);

			printf("(INT %lu) mdra s %02x%02x e %02x%02x dif %x (nrx %d) .eth\r\n",
				   dm_eth_interrupt_count(),
				   fifo_mdra_rd >> 8, fifo_mdra_rd & 0xff,
				   rd_now >> 8, rd_now & 0xff,
				   diff + compos_totaldiff,
				   fifo_fifoTurn_n);

			fifo_fifoTurn_n = 0;
			fifo_mdra_rd = rd_now;
		}
		fifo_premdra_rd = rd_now;
	#endif
	}
uint16_t DM_ETH_ToCalc_rx_pointers(int state, const uint16_t mdra_rd_org, uint16_t mdra_rd_now)
{
  debug_diff_rx_pointers(state, mdra_rd_now);
  return (state == 0) ? 0 : wrpadiff(mdra_rd_org, mdra_rd_now);
}
#endif

/* debug */
static void diff_rx_pointers_s(uint16_t *pMdra_rds) {
	uint16_t dummy_rds= 0xc00;
  DM_ETH_Read_mdra(pMdra_rds);
	DM_ETH_ToCalc_rx_pointers(0, dummy_rds, *pMdra_rds);
}

void diff_rx_s(void)
{
	if (!fifoTurn_n)
		diff_rx_pointers_s(&gkeep_mdra_rds); //&mdra_rds
}

static void diff_rx_pointers_e(uint16_t *pMdra_rds) {
	uint16_t mdra_rd, diff;
  DM_ETH_Read_mdra(&mdra_rd);
	diff = DM_ETH_ToCalc_rx_pointers(1, *pMdra_rds, mdra_rd); //......................

	do {
		static uint16_t premdra_rd = 0x4000;
		if (mdra_rd < premdra_rd && (premdra_rd != 0x4000)) {
			//uint16_t compos_totaldiff = (mdra_rd >= *pMdra_rds) ? 0x3400 : 0;
			diff += (mdra_rd >= *pMdra_rds) ? 0x3400 : 0;
			printf("[vuIP_Tasks] mdra.s %02x%02x e %02x%02x dif %02x%02x (nrx %d)\r\n",
				*pMdra_rds >> 8, *pMdra_rds & 0xff,
				mdra_rd >> 8, mdra_rd & 0xff,
				diff >> 8, diff & 0xff,
				fifoTurn_n);
			fifoTurn_n = 0;
		}
		premdra_rd = mdra_rd;
	} while(0);
}

void diff_rx_e(void)
{
	fifoTurn_n++;
	diff_rx_pointers_e(&gkeep_mdra_rds); //&mdra_rds
}
#endif //DM_ETH_DEBUG_MODE

#if 0
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
#endif
