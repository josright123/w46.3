//#include "sys/rtos_opts.h"
#include "control/dm9051opts.h"

#if DM_ETH_DEBUG_MODE
//static void dm_eth_config_interrupt_polling_log(void) {
//	printf("(Control. config.)\r\n");
//	printf("%s\r\n", dm_int_string());
//#if defined(DRV_INTR_MODE) && INTERRUPT_HANDLER_ON
//	printf("Interrupt.Hdlr On\r\n");
//#endif
//#if defined(DRV_INTR_MODE) && INTERRUPT_HANDLER_SUPPLIMENT //when '_DRV_INTR_MODE', accept INT active (default active low) trigger.
//	printf("%s\r\n", RX_HANDLER_ACCEPTED_TAKE_PLAIN_SEM ? "PLAIN SEMAPHORE_BINARY" : "NO PLAIN SEMAPHORE");
//	printf("%s\r\n", RX_HANDLER_ACCEPTED_TAKE_GENNY_SEM ? "JENNY SEMAPHORE_BINARY" : "NO JENNY SEMAPHORE");
//#endif
//	printk("\r\n");
//}
#endif

#if INTERRUPT_HANDLER_SUPPLIMENT
int nExpire_checking = 0;
#endif

static int nExpireCount = 0;
//static int nExpireCount_print;

static void dm_eth_semaphore_renew(void) {
	//nExpireCount_print = nExpireCount;
	nExpireCount = 0;
#if defined(DRV_INTR_MODE) && INTERRUPT_HANDLER_SUPPLIMENT
	nExpire_checking = 0;
#endif //DRV_INTR_MODE && INTERRUPT_HANDLER_SUPPLIMENT
}

/**
 * This function should be called when a packet is ready to be read
 * from the interface. It uses the function low_level_input()= DM_ETH_Input_W() that
 * should handle the actual reception of bytes from the network
 * interface. Then the type of the received packet is determined and
 * the appropriate input function is called.
 *
 * @param netif the lwip network interface structure for this ethernetif
 */
#if INTERRUPT_HANDLER_SUPPLIMENT_RECV
int net_pkts_handle_poll(struct netif *netif) {
	static uint16_t skeep_mdra_rds;
	int n = 0;
	
	#if DM_ETH_DEBUG_MODE
	uint16_t /*rwpa_wts,*/ mdra_rds, /*rwpa_wt,*/ mdra_rd, diff;
	DM_ETH_Diff_rx_pointers_W(0, /*&rwpa_wts*/ /*'pMdra_rds'*/ &skeep_mdra_rds, &mdra_rds);
	#endif

	while (ethernetif_input_poll(netif) == ERR_OK)
		n++;

	#if DM_ETH_DEBUG_MODE
	if (n >= 3) {
		diff = DM_ETH_Diff_rx_pointers_W(1, /*&rwpa_wt*/ /*'pMdra_rds'*/ &skeep_mdra_rds, &mdra_rd);
		diff = wrpadiff(mdra_rds, mdra_rd);
		printf("POLL.mdra.s %02x%02x mdra.e %02x%02x diff %02x%02x (%d packets)\r\n",
			mdra_rds >> 8, mdra_rds & 0xff,
			mdra_rd >> 8, mdra_rd & 0xff,
			diff >> 8, diff & 0xff, n);
		debug_packets(n);
	}
	#endif

	return n;
}
#endif
