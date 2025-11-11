/**
 **************************************************************************
 * @file     ethif.c
 * @version  v1.0.1
 * @date     2024-12-12
 * @brief    DM9051 Ethernet driver lwip interface support file (or referred to be as eth.c)
 **************************************************************************
 *
 * To restructure and improve the file to enhance readability, maintainability,
 * and potentially performance.
 * Last updated: 2024-09-05
 *
 */
#include "cmsis_compiler.h" /* CMSIS compiler specific defines */
#include "lwip/pbuf.h"
#include "lwip/netif.h"
#include "ethernetif_types.h"
#include "ethernetif.h"
#include "ethernetif_v51.h"
#include "netconf.h"

#include "platform_info/control/conf.h"

//#include "sys/sys_control/conf_rtos.h"
//#include "sys/sys_control/dm9051_plat_debug.h"
#include "platform_info/control/drv_control/conf_core.h"
#include "platform_info/control/drv_control/dm9051_drv_debug.h"
//#include "rtos_opts.h"

/* eth api */
#include "platform_info/nosys/uip_eth/types_define_all.h"

// Initialize the Ethernet driver
const uint8_t *DM_ETH_Init_W(struct netif *netif, const uint8_t *adr)
{
	const uint8_t *pd;

	//pd = DM_ETH_Init(adr);
	DM_ETH_IRQInit();
	dm9051_boards_initialize();
	pd = dm9051_init(adr);
	if (pd) {
		#if LWIP_PTP
		/* Enable PTP Timestamping */
		//dm9051_ptpstart(EMAC_PTP_FINEUPDATE); /* ETH_PTPStart(ETH_PTP_CoarseUpdate); */
		#endif

		// Set the hardware address for the network interface
		// memcpy(netif->hwaddr, pd, sizeof(netif->hwaddr));
		netif->hwaddr[0] = pd[0];
		netif->hwaddr[1] = pd[1];
		netif->hwaddr[2] = pd[2];
		netif->hwaddr[3] = pd[3];
		netif->hwaddr[4] = pd[4];
		netif->hwaddr[5] = pd[5];
		return pd;
	}
	return NULL;
}

// Receive data from the Ethernet driver
struct pbuf *DM_ETH_Input_W(void)
{
#if LWIP_PTP
#if 0
//.	struct ptptime_t timestamp; //...
	uint8_t ts_buffer[8];
#endif
#endif
	struct pbuf *p, *q;
	u16_t len = 0;
	int l = 0;
	uint8_t *buffer = get_ReceiveBuffer();

#if LWIP_PTP
	//len = DM_ETH_PTP_Input(buffer, ts_buffer); //...
	len = dm9051_rx(buffer);
	dm_eth_input_hexdump(buffer, len);
#else
	//len = DM_ETH_Input(buffer); // Get the length of received data
	len = dm9051_rx(buffer);
	dm_eth_input_hexdump(buffer, len);
#endif

	if (!len)
		return NULL;

	/* We allocate a pbuf chain of pbufs from the pool. */
	p = pbuf_alloc(PBUF_RAW, len, PBUF_POOL);
	if (p != NULL)
	{
		for (q = p; q != NULL; q = q->next)
		{
			memcpy((u8_t *)q->payload, (u8_t *)&buffer[l], q->len);
			l = l + q->len;
		}
	}
	return p;
}

// Transmit data through the Ethernet driver
void DM_ETH_Output_NW(struct pbuf *p, struct ethernetif *ethernetif)
{
#if LWIP_PTP
	int sync_message_flg = 1; //struct ptptime_t timestamp; //...
	#if 0
	uint8_t ts_buffer[8];
	#endif
#endif
	uint8_t *buffer;
	struct pbuf *q;
	int l = 0;

	buffer = get_TransmitBuffer();

	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t *)&buffer[l], q->payload, q->len);
		l = l + q->len;
	}

#if LWIP_PTP
	if (sync_message_flg && ethernetif->dm_hardware_ptp_ts)
		dm9051_tx(buffer, (uint16_t)l); //DM_ETH_PTP_HW_TIMESTAMP_Output(buffer, (uint16_t)l, ts_buffer); // Transmit the data - to change to dm9051_ptp_tx()
	else
		dm9051_tx(buffer, (uint16_t)l); //DM_ETH_PTP_Output(buffer, (uint16_t)l, ts_buffer);
#else
	dm9051_tx(buffer, (uint16_t)l); //DM_ETH_Output(buffer, (uint16_t)l); // Transmit the data
#endif
}

void DM_ETH_Output_W(struct pbuf *p)
{
#if LWIP_PTP
#if 0
//.	struct ptptime_t timestamp; //...
	uint8_t ts_buffer[8];
#endif
#endif
	uint8_t *buffer;
	struct pbuf *q;
	int l = 0;

#if LWIP_PTP
	//.....
	buffer = get_TransmitBuffer(); //- to change to get_ptp_TransmitBuffer()
#else
	buffer = get_TransmitBuffer();
#endif

	for (q = p; q != NULL; q = q->next)
	{
		memcpy((u8_t *)&buffer[l], q->payload, q->len);
		l = l + q->len;
	}
#if LWIP_PTP
	dm9051_tx(buffer, (uint16_t)l); //DM_ETH_PTP_Output(buffer, (uint16_t)l, ts_buffer); // Transmit the data - to change to dm9051_ptp_tx()
#else
	dm9051_tx(buffer, (uint16_t)l); //DM_ETH_Output(buffer, (uint16_t)l); // Transmit the data
#endif
}


#if (rt_print | drv_print)
static int fifoTurn_n = 0; //.....
#endif

static void diff_rx_pointers_s(uint16_t *pMdra_rds) {
#if DM_ETH_DEBUG_MODE
	//uint16_t rwpa_wts /*_mdra_rds*/;
	DM_ETH_ToCalc_rx_pointers(0, /*&rwpa_wts*/ pMdra_rds, pMdra_rds);
#endif
}

static void diff_rx_pointers_e(uint16_t *pMdra_rds, int n) {
#if DM_ETH_DEBUG_MODE
#if (rt_print | drv_print)
	static uint16_t premdra_rd = 0x4000;
	uint16_t mdra_rd;

	if (n >= 0) { /*3*/
		#if rt_print
		//uint16_t rwpa_wt;
		uint16_t diff =
		#endif
		DM_ETH_ToCalc_rx_pointers(1, /*&rwpa_wt*/pMdra_rds, &mdra_rd);
	//#if rt_print
	//	diff = DM_ETH_ToCalc_rx_pointers(1, /*&rwpa_wt*/pMdra_rds, &mdra_rd);
	//	diff = wrpadiff(*pMdra_rds, mdra_rd);
	//#elif drv_print
	//	DM_ETH_ToCalc_rx_pointers_W(1, pMdra_rds, &mdra_rd); //Extra-on-20241021
	//#endif

		fifoTurn_n += n;
		if (mdra_rd < premdra_rd && (premdra_rd != 0x4000))
		{
			#if rt_print
			uint16_t compos_totaldiff = (mdra_rd >= *pMdra_rds) ? 0x3400 : 0;
			printf("(INT %lu) mdra s %02x%02x e %02x%02x dif %x (nrx %d) .rt\r\n",
				dm_eth_interrupt_count(),
				*pMdra_rds >> 8, *pMdra_rds & 0xff,
				mdra_rd >> 8, mdra_rd & 0xff,
				diff + compos_totaldiff,
				fifoTurn_n);
			#endif
			fifoTurn_n = 0;
		}
		premdra_rd = mdra_rd;
	}
#endif
#endif
}

/* Use ethernetif_v51.c, 'netif_v51' */
bool process_input_pbuf(struct pbuf *p) {
  struct netif *netif = tcpip_stack_netif();
  err_t err = netif->input(p, netif);

  if (err != ERR_OK) {
    LWIP_DEBUGF(NETIF_DEBUG, ("_ethernetif_input: IP input error\n"));
    pbuf_free(p);
    return false;
  }
  return true;
}

int DM_ETHER_Receive_Route_W(void) {
		int sema_event = 0;
	
#ifdef ETHERNET_INTERRUPT_MODE
		sema_event = DM_ETH_IRQEvent();
		if (sema_event) {
#endif			
			#if (rt_print | drv_print)
			static uint16_t gmdra_rds;
			#endif

			//uint16_t isrs = dm9051_isr_disab();
			//isrSemaphore_src = 0x5555 >> 8;
			//isrSemaphore_n = 0;	

			do {
				struct pbuf *p;
				while(1) {
					#if (rt_print | drv_print)
					if (!fifoTurn_n)
						diff_rx_pointers_s(&gmdra_rds);
					#endif
					
					p = DM_ETH_Input_W(); //p = input_intr(); //= low_level_input(netif);

					if (p && process_input_pbuf(p)) {
						//isrSemaphore_n++;
					
						#if (rt_print | drv_print)
						diff_rx_pointers_e(&gmdra_rds, 1); //fifoTurn_n++; insided.
						#endif
						continue;
					}
					break;
				}
			} while(0);			
				
			dm9051_isr_enab(); //DM_ETH_ToRst_ISR();

#ifdef ETHERNET_INTERRUPT_MODE
		}
#endif
		return sema_event;
}
