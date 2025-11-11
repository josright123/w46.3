//.#ifndef __DM_TYPES3_H
//.#define __DM_TYPES3_H

/* dbg_core_opts.h
 */
//#ifndef __IDENTIFY_CORE_DBG_OPTS_H__
//#define __IDENTIFY_CORE_DBG_OPTS_H__

//#endif //__IDENTIFY_CORE_DBG_OPTS_H__

/* Configuration Constants */
#define MAX_NODE_CANDIDATES    			6
#define RX_BUFFER_START       			0xC00
#define RX_BUFFER_END         			0x4000
#define MAX_HEX_LINE_BUF      			180
#define MAX_HEX_SEGMENT       			32
#define DEFAULT_MDRA_RD       			0x4000
#define TOTAL_DIFF_OFFSET     			0x3400

#define MONITOR_RXPOINTER_PACKET_NUM		8	// log times used in dbg mode (log from power-up)
//#define MONITOR_RXPOINTER_PACKET_NUM		0
#define MONITOR_RXPOINTER_CYCLE_NUM		3	// log times used in dbg mode (log for recycle happen)
//#define MONITOR_RXPOINTER_CYCLE_NUM		0

/* Debug Configuration */
#define MAX_RX_LOG_ENTRIES			2 //1
#define RX_LOG_DUMP_PACKET_NAME			1
#define RX_LOG_DUMP_PAYLOAD_DATA 		1 //0
#define TX_LOG_DUMP_PAYLOAD_DATA 		0
#define RX_LOG_DUMP_MDRA_OVERNIGHT_TEST 	0
#define MIN_HEADER_LENGTH			14
#define MIDL_HEADER_LENGTH			46
#define MIN(a, b)             			((a < b) ? a : b)
#define LIMIT_LEN(n, nTP)     			((n <= nTP) ? n : nTP)

/* example
 */
#define DEMO_LED_FLASH	1
#define net_led_demo(fdemo)
#define button_led_demo_init(fdemo) 
#define button_led_demo(fdemo)

	/*
	 * DEBUG-SEC-0, public
	 *  - essential extern sub declaration (or extern data)
	 */
	#if DM_DEBUG_TYPE == 0

//	#define DBG_SET_FIELD(field, val)		dbg_mset_##field(val)	// call-use
//	#define DBG_GET_FIELD(field)			dbg_mget_##field()	// call-use
//	#define identify_link(info)			DBG_SET_FIELD(flgLink_r, info)	//DR_SET_IRQMACRO(flgLink_r, info)
//	#define identified_link()			DBG_GET_FIELD(flgLink_r)	//DR_GET_IRQMACRO(flgLink_r)

	#undef DBG_DECL_MACRO
	#define DBG_DECL_MACRO(rtype, mtype, field) 			\
			rtype dbg_mget_##field(void); 			\
			rtype dbg_mset_##field(const mtype param)
	DBG_DECL_MACRO(int, uint8_t *, flgLink_r);

	/*static*/
	void diff_rx_s(void);
	void diff_rx_e(void);
	void diff_mdra(void);
	void dm_eth_input_hexdump_reset(void);
	void dm_eth_input_hexdump(const void *buf, size_t len);
	void dm_eth_output_hexdump(const void *buf, size_t len);
	void dm_eth_output_data_dump(char *head, const void *buf, uint16_t tot_len, size_t len);
	void eth_printf(char *str);
	void eth_printkey(char *str);
	void DM_ETH_Read_mdra(uint16_t *mdra_rd_now);
	uint16_t DM_ETH_ToCalc_rx_pointers(int state, const uint16_t mdra_rd_org, uint16_t mdra_rd_now);
	#endif //DM_DEBUG_TYPE 0

	/*
	 * DEBUG-SEC-10, static
	 *  - essential static sub (or data)
	 *  - non-debug static sub
	 */
	#if DM_DEBUG_TYPE == 10

	/* Global dr */
	#undef DBG_DATA_MACRO
	#define DBG_DATA_MACRO(mtype, field) \
		mtype field;

	static struct maindbg_data
	{
		DBG_DATA_MACRO(int, flgLink_r)
	} dbgdata =
	{0, };

	//with head-str
	void eth_printf(char *str) {
		printf("%s", str);
	}
	//no head-str
	void eth_printkey(char *str) {
		printkey("%s", str);
	}
	#endif //DM_DEBUG_TYPE 10

	/*
	 * DEBUG-SEC-20, public
	 *  - essential extern sub
	 *  - non-debug extern sub
	 */
	#if DM_DEBUG_TYPE == 20
	/* Check LINK model define */
	/* NSR register or BMSR register */
	const enum link_state_t {
		DM9051_CHECK_MAC = 0,
		DM9051_CHECK_PHY
	} stat_model = DM9051_CHECK_MAC;

	#undef DBG_FUNCMACRO
	#define DBG_FUNCMACRO(rtype, mtype, field)                              \
		rtype dbg_mget_##field(void)                                    \
		{                                               		\
			return dbgdata.field;                                   \
		}                                                                         \
		rtype dbg_mset_##field(const mtype parmeter)                         \
		{                                  \
			if (stat_model == DM9051_CHECK_MAC) {	\
				dbgdata.field = parmeter[1] & 0x40 ? 1 : 0;	\
			}	\
			else if (stat_model == DM9051_CHECK_PHY) {	\
				dbgdata.field = parmeter[5] & 0x04 ? 1 : 0;  	\
			}	\
			else	\
				dbgdata.field = 1;	\
			return dbgdata.field;	\
		}

	DBG_FUNCMACRO(int, uint8_t *, flgLink_r)
	#endif //DM_DEBUG_TYPE 22

	/*
	 * DEBUG-SEC-11, static
	 *  - debug static sub
	 */
	#if DM_DEBUG_TYPE == 11
	#if DM_ETH_DEBUG_MODE && RX_LOG_DUMP_PAYLOAD_DATA
	static int link_log_reset_allow_num = 0;
	void dm_eth_input_hexdump_reset(void)
	{
		// Jos like protect from always write-it.
		if (link_log_reset_allow_num) {
			link_log_reset_allow_num = 0;
		}
	}
	#else
	void dm_eth_input_hexdump_reset(void)
	{
	}
	#endif //DM_ETH_DEBUG_MODE && RX_LOG_DUMP_PAYLOAD_DATA

	#if DM_ETH_DEBUG_MODE
	#if RX_LOG_DUMP_PAYLOAD_DATA
	/* Hex Dump Implementation */
	static void room_printf_space(char *lineroom, int n)
	{
		int offset;
		//offset = 0;
	    //while (n--)
	    //    offset += sprintf(lineroom + offset, "%c", ' ');
	    //return offset;
		
		for (offset = 0; offset < n; offset++)
			lineroom[offset] = ' ';
	}

	static int room_printf_rxlen_head(char *lineroom, int nspc)
	{
	    //if (!nspc)
	    //    nspc = sprint_headspace_dump0(lineroom, tlen, 0);

	    room_printf_space(lineroom, nspc);
	    return nspc;
	}

	static int sprint_headspace_dump0(size_t tlen, int also_print)
	{
	    //int offset = 0;
		char lineroombuff[MAX_HEX_LINE_BUF];
	    char textspace[16];
	    int n = sprintf(textspace, "rxlen %4d", tlen);

	    room_printf_space(lineroombuff, n);
	    sprintf(lineroombuff + n, " %s", textspace);
		if (also_print)
			printf("%s\r\n", lineroombuff);

	    return n;
	}

	static void sprint_hex_dump0(int initspace, int titledn, char *prefix_str,
				   size_t tlen, int rowsize, const void *buf, 
				   int seg_start, size_t len, int cast_lf)
	{
	#if (drv_print) || (defined(__DM9051_AP_DEBUG_H) && ap_print)
	    char lineroombuff[MAX_HEX_LINE_BUF];
	    int print_linefeed_flag;
	    int si = seg_start;
	    int se = seg_start + len;
	    int titlec = 0;
	    int nspace = 0;
			int i;

		/*int head_space, 
	    (void)head_space;*/
		
		//initspace = sprint_headspace_dump0(lineroombuff, tlen, 0);

	    for (i = si; i < se; i += rowsize)
	    {
		char linebuf[(12 * 3) + (3 * 16) + 1 + 32];
		int remaining = len - (i - si);
		int linelen = MIN(remaining, rowsize);
		const uint8_t *ptr = buf;
		int nb = 0;
		int j;

		nspace = room_printf_rxlen_head(lineroombuff, initspace);
		
		/* Format hex values */
		for (j = 0; j < linelen && (size_t)nb < sizeof(linebuf); j++)
		{
		    if (j && !(j % 8))
			nb += snprintf(linebuf + nb, sizeof(linebuf) - nb, " ");
		    if (((rowsize >> 1) != 8) && !(j % (rowsize >> 1)))
			nb += snprintf(linebuf + nb, sizeof(linebuf) - nb, " ");

		    nb += snprintf(linebuf + nb, sizeof(linebuf) - nb, "%02x ", *(ptr + i + j));
		}

		/* Format output line */
		nspace += sprintf(lineroombuff + nspace, " ");
		if (prefix_str) {
		    nspace += sprintf(lineroombuff + nspace, "%s %.3x %s", prefix_str, i, linebuf);
		    while (titledn) {
			titledn--;
			prefix_str[titlec++] = ' ';
		    }
		} else {
		    nspace += sprintf(lineroombuff + nspace, "(dm9 xfer) %.3x %s", i, linebuf);
		}

		/* Handle line endings */
		print_linefeed_flag = ((i + rowsize) < se) || cast_lf;
		printf("%s%s", lineroombuff, print_linefeed_flag ? "\r\n" : "");
	    }
	#endif
	}
	#endif //RX_LOG_DUMP_PAYLOAD_DATA
	#endif //DM_ETH_DEBUG_MODE
	#endif //DM_DEBUG_TYPE 11

	#if DM_DEBUG_TYPE == 21 && DM_ETH_DEBUG_MODE
	static int fifoTurn_nRx = 0, fifoCyc_num = 0, fifoPkt_num = 0; //...
	uint16_t gkeep_mdra_rds;
	uint16_t wrpadiff(uint16_t rwpa_s, uint16_t rwpa_e)
	{
	    return (rwpa_e >= rwpa_s) ? 
		   rwpa_e - rwpa_s : 
		   (rwpa_e + RX_BUFFER_END - RX_BUFFER_START) - rwpa_s;
	}

	void one_packet_tocalc(uint16_t mdra_rd, uint16_t diff) {
		static uint16_t dfmdra_rd = 0xc00;
		#if 1 //[extra-condition-to-be-checked] checked.
//		if (identified_dhcpc_en()) {
//			const uint8_t *ip = (const uint8_t *)identified_tcpip_ip();
//			if (!ip[0] & !ip[1] & !ip[2] & !ip[3])
//				return;
//		}
		if (link_log_reset_allow_num < MAX_RX_LOG_ENTRIES)
			return;
		#endif
		if (fifoPkt_num < MONITOR_RXPOINTER_PACKET_NUM) {
			uint16_t df = wrpadiff(dfmdra_rd, mdra_rd);
			printf("[dbg_pkts %d.%d] mdra.s %02x%02x e %02x%02x dif %02x%02x (len %02x%02x nrx %2d)\r\n",
				MONITOR_RXPOINTER_PACKET_NUM, fifoPkt_num,
				gkeep_mdra_rds >> 8, gkeep_mdra_rds & 0xff,
				mdra_rd >> 8, mdra_rd & 0xff,
				diff >> 8, diff & 0xff,
				df >> 8, df & 0xff,
				fifoTurn_nRx);
			fifoPkt_num++;
		}
		dfmdra_rd = mdra_rd;
	}

	int cyc_packets_tocalc(uint16_t mdra_rd, uint16_t diff, int fcyc_num) {
		static uint16_t diffmdra_rd = 0x4000;
		#if 1 //[extra-condition-to-be-checked] checked.
//		if (identified_dhcpc_en()) {
//			const uint8_t *ip = (const uint8_t *)identified_tcpip_ip();
//			if (!ip[0] & !ip[1] & !ip[2] & !ip[3])
//				return fcyc_num;
//		}
		if (link_log_reset_allow_num < MAX_RX_LOG_ENTRIES)
			return fcyc_num;
		#endif
		if (mdra_rd < diffmdra_rd && (diffmdra_rd != 0x4000)) {
			diff += (mdra_rd >= gkeep_mdra_rds) ? 0x3400 : 0;
			
			/* cycle-time */
			printf("[dbg_cycs %d.%d] mdra.s %02x%02x e %02x%02x dif %02x%02x (nrx %d)\r\n",
				MONITOR_RXPOINTER_CYCLE_NUM, fcyc_num,
				gkeep_mdra_rds >> 8, gkeep_mdra_rds & 0xff,
				mdra_rd >> 8, mdra_rd & 0xff,
				diff >> 8, diff & 0xff,
				fifoTurn_nRx);
			fcyc_num++;
			fifoTurn_nRx = 0;
		}
		diffmdra_rd = mdra_rd;
		return fcyc_num;
	}

	/* debug */
	/*static*/ void DM_ETH_Read_mdra(uint16_t *mdra_rd_now)
	{
	  uint16_t dummy_rwpa;
	  dm9051_read_rx_pointers(&dummy_rwpa, mdra_rd_now);
	}

	static void debug_diff_rx_pointers(int state, uint16_t rd_now)
	{
	#if (defined(__DM9051_ETH_DEBUG_H) && drv_print)  || (defined(__DM9051_AP_DEBUG_H) && ap_print) //org 'drv_print'
	#endif
	}

	/**
	 * @brief  Debug function for RX pointer calculation
	 */
	/*static*/ uint16_t DM_ETH_ToCalc_rx_pointers(int state, const uint16_t mdra_rd_org, uint16_t mdra_rd_now)
	{
	  debug_diff_rx_pointers(state, mdra_rd_now);
	  return (state == 0) ? 0 : wrpadiff(mdra_rd_org, mdra_rd_now);
	}

	static void diff_rx_pointers_s(void) {
		DM_ETH_Read_mdra(&gkeep_mdra_rds);
		DM_ETH_ToCalc_rx_pointers(0, 0xc00, gkeep_mdra_rds); ////uint16_t dummy_rds= 0xc00;
	}

	static int diff_rx_pointers_e(int fcyc_num) {
		uint16_t mdra_rd_now, diff;
		DM_ETH_Read_mdra(&mdra_rd_now);
		diff = DM_ETH_ToCalc_rx_pointers(1, gkeep_mdra_rds, mdra_rd_now); //......................

		one_packet_tocalc(mdra_rd_now, diff); /*&gkeep_mdra_rds,*/ 
		return cyc_packets_tocalc(mdra_rd_now, diff, fcyc_num); /*&gkeep_mdra_rds,*/
		//return fcyc_num;
	}
	#endif //DM_DEBUG_TYPE 21 && DM_ETH_DEBUG_MODE

	#if DM_DEBUG_TYPE == 21
	#if DM_ETH_DEBUG_MODE
	void diff_rx_s(void)
	{
	#if DM_ETH_DEBUG_MODE
		if (!fifoTurn_nRx)
			diff_rx_pointers_s(); //&mdra_rds
	#endif
	}

	void diff_rx_e(void)
	{
	#if DM_ETH_DEBUG_MODE
		fifoTurn_nRx++;
		if (fifoCyc_num < MONITOR_RXPOINTER_CYCLE_NUM ||
			fifoPkt_num < MONITOR_RXPOINTER_PACKET_NUM) {
			fifoCyc_num = diff_rx_pointers_e(fifoCyc_num);
		} //do .. while(0);
	#endif
	}

	void diff_mdra(void)
	{
	#if DM_ETH_DEBUG_MODE
	#if RX_LOG_DUMP_MDRA_OVERNIGHT_TEST
		uint16_t mdra_rd_now;
		static int cyc_mdra = 0;
		if (++cyc_mdra >= 3) {
			DM_ETH_Read_mdra(&mdra_rd_now);
			printf("uip periodic_timer check for mdra ... %02x%02x\r\n", mdra_rd_now >> 8, mdra_rd_now & 0xff);
			cyc_mdra = 0;
		}
	#endif
	#endif
	}


	#if RX_LOG_DUMP_PACKET_NAME
	char *parse_buf_rx_packet(uint8_t *bff)
	{
	    if (bff[12] == 0x08 && bff[13] == 0x06)
		return "Arp";
	    else if (bff[23] == 0x11 && bff[34] == 0x00 && bff[35] == 0x43) //rx dhcp, src port 67
		return "Dhcp";
	    else if (bff[23] == 0x11 && bff[34] == 0x00 && bff[35] == 0x44) //tx dhcp, src port 68
		return "Dhcp";
	    else if (bff[23] == 0x01 && bff[34] == 0x08 /*&& bff[35] == 0x00*/) 
		return "Icmp";
	    else
		return "Unknown_Others";
	}
	#endif

	/*static*/
	typedef enum {
	  DM_FALSE = 0,
	  DM_TRUE = !DM_FALSE,
	} enable_t;

	static void hex_limit_dump0(char *prefix_str, const void *buf, size_t tlen, size_t dlen) {
		int initspace = sprint_headspace_dump0(dlen, 0);
		sprint_hex_dump0(initspace, 0, prefix_str, tlen, MAX_HEX_SEGMENT,
			    buf, 0, LIMIT_LEN(tlen, dlen), DM_TRUE);
	}


	/*static*/
	void dm_eth_input_hexdump(const void *buf, size_t len)
	{
	#if DM_ETH_DEBUG_MODE
	    if (!len)
		return;
	    if (link_log_reset_allow_num >= MAX_RX_LOG_ENTRIES) {
		return;
	    }
	#if RX_LOG_DUMP_PACKET_NAME
		do {
			char *pars = parse_buf_rx_packet((uint8_t *)buf);
			//. link_log_reset_allow_num++;
			if (!strcmp(pars, "Arp"))
				printf("recv %s\r\n", pars);
			else if (!strcmp(pars, "Dhcp") || !strcmp(pars, "Icmp"))
				printf("recv %s\r\n", pars);
			else
				printf("recv (dumpRPkt %d / allowMax %d) rxlen %4d\r\n",
					link_log_reset_allow_num, MAX_RX_LOG_ENTRIES, len);
			#if RX_LOG_DUMP_PAYLOAD_DATA
			//if (identified_dhcpc_en()) 
			//#include "lwip/opt.h"
			//if (LWIP_DHCP)
			//#ifdef __DHCPC_H__
			//#endif
			if (1)
			{
				const uint8_t *ip = (const uint8_t *)identified_tcpip_ip();
				if (!ip[0] & !ip[1] & !ip[2] & !ip[3])
					return;
			}
			hex_limit_dump0("(dm9 head   <<rx)", buf, len, MIDL_HEADER_LENGTH);
			link_log_reset_allow_num++; //.
			#endif
		} while(0);
	#endif
	#endif
	}

	void dm_eth_output_hexdump(const void *buf, size_t len)
	{
	#if DM_ETH_DEBUG_MODE
	#if TX_LOG_DUMP_PAYLOAD_DATA
		char *pars = parse_buf_rx_packet((uint8_t *)buf);
		if (!strcmp(pars, "Dhcp")) {
			hex_limit_dump0(" dm9 head   tx>> ", buf, len, len); //hex_dump0(" dm9 head   tx>> ", buf, LIMIT_LEN(len, 1));
			dm_eth_input_hexdump_reset();
		}
	#endif
	#endif
	}

	void dm_eth_output_data_dump(char *head, const void *buf, uint16_t tot_len, size_t len)
	{
		char head_buf[32];
		sprintf(head_buf, " %s", head);
		printf("%s (len: %d)\r\n", head_buf, tot_len);
		hex_limit_dump0("", buf, len, len); //hex_dump0(" dm9 head   tx>> ", buf, LIMIT_LEN(len, 1));
	}
	#else
	void diff_rx_s(void)
	{
	}
	void diff_rx_e(void)
	{
	}
	#endif // && DM_ETH_DEBUG_MODE
	#endif //DM_DEBUG_TYPE 21

#undef DM_DEBUG_TYPE
//.#endif //__DM_TYPES3_H
