/* define to prevent recursive inclusion -------------------------------------*/
#ifndef __DM9051A_PTP_H
#define __DM9051A_PTP_H

#ifdef __cplusplus
extern "C"
{
#endif
#include "lwip/opt.h" //[TEMP for LWIP_PTP]
#include "lwip/netif.h"
#include "lwip/pbuf.h"

#include "netconf.h"

// PTP FIELD
#define PTP_ETHERTYPE                         0x88F7 // Layer 2 PTP
#define PTP_EVENT_PORT                        319    // UDP PTP EVENT
#define PTP_GENERAL_PORT                      320    // UDP PTP GENERAL

// 02H TX Control Reg
#define TCR_TSEN_CAP                 			TCR_RSV_BIT7
#define TCR_TS1STEP_EMIT             			TCR_TJDIS //TCR_DIS_JABBER_TIMER

// 06H RX Status Reg
// BIT(5),PTP use the same bit, timestamp is available
// BIT(3),PTP use the same bit, this is odd parity rx TimeStamp
// BIT(2),PTP use the same bit: 1 => 8-bytes, 0 => 4-bytes, for timestamp length
#define RSR_RXTS_EN                           (1 << 5)
#define RSR_RXTS_PARITY                       (1 << 3)
#define RSR_RXTS_LEN                          (1 << 2)
#define RSR_PTP_BITS                          (RSR_RXTS_EN | RSR_RXTS_PARITY | RSR_RXTS_LEN)

//#if EDRIVER_ADDING_PTP && LWIP_PTP
struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

int is_issue_ptp_sync_one_step_emit(uint8_t *buf);
int is_issue_ptp_tstamp_tsen(uint8_t *buf);
uint8_t ptp_tx_tstamp_parse_packet(uint8_t *buffer, struct pbuf *p);
void ptp_tx_tstamp_pass_to(uint8_t *buffer, struct pbuf *p, uint8_t tcr_wr);

  //[API]uip
const uint8_t *dm9051a_init_ptp(const uint8_t *adr);
//#endif

//#if EDRIVER_ADDING_PTP && LWIP_PTP //(ptpd-2.0.0\src\dep\sys_time.c is using)
void v51_ptptime_add_offset(struct ptptime_t *timestamp);
void dm9051_ptptime_gettime(struct ptptime_t * timestamp);
void dm9051_ptptime_settime(struct ptptime_t * timestamp);
void dm9051_ptptime_updateoffset(struct ptptime_t * timeoffset);
void dm9051_ptptime_adjfreq(int32_t Adj);
//#endif

//void v51_ptptime_gettime(struct ptptime_t *timestamp);
//void v51_ptptime_settime(struct ptptime_t *timestamp);
uint16_t dm9051_rx_ptp(uint8_t *buffer, uint8_t *receivedata, uint8_t *ts_bff); //uint16_t dm9051_rx_ptp(uint8_t *buff, uint8_t *ts_bff);
void dm9051_tx_ptp(uint8_t *buff, uint16_t len, struct pbuf *p);
//void _ptp_rx_tstamp_process(struct pbuf *p, uint8_t *buffer, uint8_t *receivedata, uint8_t *ts_bff, uint16_t len);
int rx_ptptime_debug(uint8_t *buffer, struct pbuf *p);

uint16_t cspi_rx_head_ptp(uint8_t *receivedata, uint8_t *ts_bff);
uint16_t rx_head_takelen_ptp(uint8_t *receivedata, uint8_t *ts_bff);
void cspi_rx_tstamp_mem(uint8_t *receivedata, uint8_t *ts_bff);

void v51_ptptime_set_tx_timestamp_reg(uint8_t val);

void v51_getPtpClockRate1(uint32_t *rateValuePtr, uint8_t *sign);
void v51_adjust_ptp_frequency(uint32_t adjustment, int8_t direction);
//void v51_adjustPtpFrequency1(uint32_t adjustment, uint8_t direction);
//void v51_ptptime_zero_rate(void);

void v51_putPtpClockRate(int32_t rateValue/*, uint8_t sign*/);
void v51_pushPtpClockRate(int reverse);
void v51_updatePtpClockRate(int64_t signed_addend); //(int32_t adjust_value, uint8_t adjust_direction);

void internaltime_as_ymd_s(char *head, const s32_t seconds, s32_t nanoseconds);

//#if EDRIVER_ADDING_PTP && LWIP_PTP
void impl_dm9051_get_tx_timestamp(struct ptptime_t *pTimeStamp);
void print_as_ymd(char *head, const struct ptptime_t *time);
void print_as_ymd_s(int rnd, char *head, const struct ptptime_t *time);
//#endif

#ifdef __cplusplus
}
#endif

#endif //__DM9051A_PTP_H
