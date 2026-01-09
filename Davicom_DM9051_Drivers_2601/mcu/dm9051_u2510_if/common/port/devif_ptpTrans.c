//#include "lwip/opt.h"
#include "lwip/pbuf.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
#include "dm9051_edriver_extend/dm9051a_ptp.h"

void dm9051_tx_ptp_if_trans(uint8_t *buff, uint16_t len, struct pbuf *p);
uint16_t dm9051_rx_ptp_if_trans(uint8_t *buffer);
void PTPd_Init_manager_support_hashing(void);

/* Global variables ---------------------------------------------------------*/
//extern int input_mode; //return from "dm9051_conf()"

const uint8_t *DM_ETH_Init_ptpTrans(/*const*/ uint8_t *adr)
{
	/*input_mode =*/
	dm9051_conf();
	return dm9051a_init_ptp(adr);
}

int DM_ETH_Init_mode_ptpTrans(void)
{
	//return input_mode;
	return hal_active_interrupt_mode();
}

void tx_manager_dispatch_ptpTrans(uint8_t *buffer, uint16_t len, struct pbuf *p)
{
	dm9051_tx_ptp_if_trans(buffer, len, p); //dm9051_tx_ptp_if(buffer, len, p);
}

uint16_t rx_manager_dispatch_ptpTrans(uint8_t *buf)
{
	uint16_t len = dm9051_rx_ptp_if_trans(buf); //dm9051_rx_ptp_if(buf);
	if (len) {
		dump_data(buf, len); //dm_eth_input_hexdump(buf, len);
	}
	return len;
}

/* IGMP callback for joining/leaving multicast groups */
err_t rx_igmp_mac_filter_ptpTrans(struct netif *netif, const ip4_addr_t *group, enum netif_mac_filter_action action)
{
	PTPd_Init_manager_support_hashing(); //dm9051etc.rx_setup = IGMP_HAS_HASHING;
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


uint8_t ts_receivedata[4];
uint8_t ts_buf[8];

//[ethif.info header file]
#define MANAGER_RAW	0
#define MANAGER_PTP	1

#define PTP_WAIT_SETUP 0
#define PTP_HAS_SETUP  1
#define PTP_DONE_HASHING 2

#define IGMP_NULL_HASHING	0
#define IGMP_HAS_HASHING	(1 << 0)
#define IGMP_HAS_HASHING1	(1 << 1) //not used

struct proc_info_t {
	/*
	 * .eth_tx_dynamic
	 */
	int init_manager;
	int init_setup;
	int rx_manager;
	int rx_setup;
	/*void (*tx_dispatch)(uint8_t *buffer, uint16_t l, struct pbuf *p);*/
};

struct proc_info_t dm9051etc = {
	/*
	 * .eth_tx_dynamic
	 */
	MANAGER_RAW, // --> init_manager
	PTP_WAIT_SETUP,
	MANAGER_RAW, // --> rx_manager
	IGMP_NULL_HASHING,
	/*
	tx_manager_dispatch_w,
	*/
};

char *get_ptp_header(uint8_t *p, uint16_t len, int rxts_en_packet);
void buffer_ts_time(uint8_t *buffer, TimeInternal *pTimeTmp);
char *parse_pbuf_ptp_packet1(uint8_t *buf, int in_offset);

extern PtpClock ptpClock; //for debug log
extern struct ptptime_t delayREQ_T3, master_recv_extra_T3;

void PTPd_Init_manager_support_hashing(void)
{
	dm9051etc.rx_setup = IGMP_HAS_HASHING;
}

void PTPd_Init_manager_setup(void)
{
	dm9051etc.init_setup = PTP_HAS_SETUP; //ptp_inst.eth_tx_dynamic = ptp_inst.eth_tx_operate_ptp;
	if (dm9051etc.rx_setup == IGMP_HAS_HASHING) {
		printf("After ptpd_Init, Support 'netif->igmp_mac_filter' OK\r\n");
		dm9051etc.init_setup = PTP_DONE_HASHING;
	}
	else {
		printf("Warning: After PTPd_Init(), It's better support 'netif->igmp_mac_filter' for dm9051_rx_ptp() to operate\r\n");
		
		//while(1);
		dm9051_rx_rcr_all(); //cspi_set_rcr(FORCE_RCR_ALL_ON);
	}
}

void dm9051_tx_ptp_if_trans(uint8_t *buff, uint16_t len, struct pbuf *p)
{
	if (dm9051etc.init_setup == PTP_WAIT_SETUP) {
		dm9051_tx(buff, len);
		return;
	}
	dm9051_tx_ptp(buff, len, p);
}

uint16_t dm9051_rx_ptp_if_trans(uint8_t *buffer)
{
	#if 0
	//if (dm9051etc.init_setup == PTP_HAS_SETUP) {
	//	if (dm9051etc.rx_setup == IGMP_HAS_HASHING) {
	//		printf("After ptpd_Init, Support 'netif->igmp_mac_filter' OK\r\n");
	//		dm9051etc.init_setup = PTP_DONE_HASHING;
	//	}
	//	else {
	//		printf("Warning: After PTPd_Init(), It's better support 'netif->igmp_mac_filter' for dm9051_rx_ptp() to operate\r\n");
	//		while(1);
	//	}
	//}
	#endif
	return dm9051_rx_ptp(buffer, ts_receivedata, ts_buf);
}

int rx_manager_dispatch_pbuf_postTrans(uint8_t *buf, struct pbuf *p)
{
	switch(dm9051etc.rx_manager) {
		case MANAGER_RAW:
			break;
		case MANAGER_PTP:
			if (p != NULL) {
				if (ts_receivedata[1] & RSR_RXTS_EN) {
					p->time_sec = (uint32_t)ts_buf[3] | (uint32_t)ts_buf[2] << 8 | (uint32_t)ts_buf[1] << 16 | (uint32_t)ts_buf[0] << 24;
					p->time_nsec = (uint32_t)ts_buf[7] | (uint32_t)ts_buf[6] << 8 | (uint32_t)ts_buf[5] << 16 | (uint32_t)ts_buf[4] << 24;
				}
				return rx_ptptime_debug(buf, p); //rx_tstamp_2_pbuf(buf, p);
			}
			break;
	}
	return 0;
}

int rx_ptptime_debug(uint8_t *buffer, struct pbuf *p)
{
	struct ptptime_t trapped_delayRESP_T4;
	char *ptp_hdr = get_ptp_header(buffer, p->tot_len, ts_receivedata[1] & RSR_RXTS_EN);
	if (ptp_hdr)
	{
			MsgHeader    header;
			msgUnpackHeader((const Octet *)(buffer+14+20+8), &header);
			p->messageType = header.messageType;
			switch (p->messageType)
			{
				case ANNOUNCE:
//					printf("in ANNOUNCE\r\n"); //handleAnnounce(ptpClock, isFromSelf);
				break;

				case SYNC:
//					printf("in SYNC\r\n"); //handleSync(ptpClock, &time, isFromSelf);
					#if 1
					if (1) {
						TimeInternal comeInPkt_t1;
						TimeInternal t2;

						//[show diff to master.]
						buffer_ts_time(buffer+14+20+8, &comeInPkt_t1); //UnpackTimestamp((const Octet *)(buffer+14+20+8), &unpk_timestamp);
						//arrive_timestamp.tv_sec, arrive_timestamp.tv_nsec
						t2.seconds = p->time_sec, //ts_arrive.tv_sec;
						t2.nanoseconds = p->time_nsec, //ts_arrive.tv_nsec;
						subTime(&t2, &t2, &comeInPkt_t1);

						printf("%s rx SYNC: slave OnRcv.msgSync(%u.%09u T2) unPack(%u.%09u T1) diff to master %10d s %11d ns\r\n",
							dm_eth_app_mcu_name(NULL), //=PTPD_HEADER_MCU_DESC
							p->time_sec, //ts_arrive.tv_sec,
							p->time_nsec, //ts_arrive.tv_nsec,
							comeInPkt_t1.seconds, 
							comeInPkt_t1.nanoseconds,
							t2.seconds, t2.nanoseconds);
					}
					#endif
				break;

				case FOLLOW_UP:
					printf("in follow_up\r\n"); //handleFollowUp(ptpClock, isFromSelf);
				break;

				case DELAY_REQ:
					/* master action
					 * master rx delayREQ_T4;
					 */
					if (ptpClock.portDS.portState == PTP_MASTER) {
						trapped_delayRESP_T4.tv_sec = p->time_sec; //ts_arrive.tv_sec;
						trapped_delayRESP_T4.tv_nsec = p->time_nsec; //ts_arrive.tv_nsec;

						/* master's check (peek) (if slave do a favor packet has ts.) */
						TimeInternal master_rcvDlyReq_extra_timeTmp;
						TimeInternal internalTime;
						#if SLAVE_MAKE_DELAY_REQ_FAVOR_TS
						/* master's check (peek) (when slave do a favor make the packet has ts.) */
						#endif
						buffer_ts_time(buffer+14+20+8, &master_rcvDlyReq_extra_timeTmp);
						master_recv_extra_T3.tv_sec = master_rcvDlyReq_extra_timeTmp.seconds;
						master_recv_extra_T3.tv_nsec = master_rcvDlyReq_extra_timeTmp.nanoseconds;

						internalTime.seconds = trapped_delayRESP_T4.tv_sec;
						internalTime.nanoseconds = trapped_delayRESP_T4.tv_nsec;

						subTime(&internalTime, &internalTime, &master_rcvDlyReq_extra_timeTmp);
						printf("recv DELAY_REQ at tstamp(%u.%09u) diff to slave %u sec %u ns\r\n",
							trapped_delayRESP_T4.tv_sec, trapped_delayRESP_T4.tv_nsec,
							internalTime.seconds, internalTime.nanoseconds);
					}
				break;

				case PDELAY_REQ:
					printf("in pdelay_req\r\n"); //handlePDelayReq(ptpClock, &time, isFromSelf);
				break;

				case DELAY_RESP:
				if (1) {
					Boolean isFromCurrentParent = FALSE;
					Boolean isCurrentRequest = FALSE;

					isFromCurrentParent = isSamePortIdentity( //DIS
						&ptpClock.parentDS.parentPortIdentity,
						&header.sourcePortIdentity); //&ptpClock.msgTmpHeader.sourcePortIdentity

					isCurrentRequest = isSamePortIdentity( //DIS
						&ptpClock.portDS.portIdentity,
						&ptpClock.msgTmp.resp.requestingPortIdentity);

					isFromCurrentParent = TRUE; //CAST
					isCurrentRequest = TRUE; //CAST
					if (((ptpClock.sentDelayReqSequenceId - 1) == header.sequenceId) //ptpClock.msgTmpHeader.sequenceId
						&& isCurrentRequest && isFromCurrentParent)
					{
						TimeInternal timeTmp;
						TimeInternal resp_receiveTimestamp;
						TimeInternal internalTime;

						buffer_ts_time(buffer+14+20+8, &resp_receiveTimestamp);
						timeTmp.seconds = resp_receiveTimestamp.seconds;
						timeTmp.nanoseconds = resp_receiveTimestamp.nanoseconds;
						internalTime.seconds = delayREQ_T3.tv_sec;
						internalTime.nanoseconds = delayREQ_T3.tv_nsec;
						//abs
						subTime(&timeTmp, &timeTmp, &internalTime);
						//abs(),abs()
						printf("in DELAY_RESP from-master-pk_ts(%u.%09u) diff to master %u sec %u ns\r\n",
							resp_receiveTimestamp.seconds, resp_receiveTimestamp.nanoseconds,
							timeTmp.seconds, timeTmp.nanoseconds);
					}
				}
				break;

				case PDELAY_RESP:
				printf("in pdelay_reap\r\n"); //handlePDelayResp(ptpClock, &time, isFromSelf);
				break;

				case PDELAY_RESP_FOLLOW_UP:
				printf("in pdelay_resp_follow_up\r\n"); //handlePDelayRespFollowUp(ptpClock, isFromSelf);
				break;

				case MANAGEMENT:
				printf("in management\r\n"); //handleManagement(ptpClock, isFromSelf);
				break;

				case SIGNALING:
				printf("in signaling\r\n"); //handleSignaling(ptpClock, isFromSelf);
				break;

				default:
				printf("in Unknow\r\n");
				break;
			}

			/* dump */
			do {
				char *ab1 = parse_pbuf_ptp_packet1(/*p*/ buffer, 14+20+8);
				if (!strcmp(ab1, "Delay_Req"))
					dump_data(buffer, p->tot_len); //dm_eth_input_hexdump(/*p->payload*/ buffer, p->tot_len); //dm_eth_davicom_hexdump(p->payload, l);
			} while(0);
			return 1;
	}
	return 0;
}
