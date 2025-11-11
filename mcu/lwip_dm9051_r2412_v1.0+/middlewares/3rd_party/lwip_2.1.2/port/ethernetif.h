#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
struct ethernetif
{
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int dm_hardware_ptp_ts;
  int unused;
};

struct ptptime_t {
  s32_t tv_sec;
  s32_t tv_nsec;
};

#if LWIP_PTP
void emac_ptptime_settime(struct ptptime_t * timestamp);
void emac_ptptime_gettime(struct ptptime_t * timestamp);
void emac_ptptime_updateoffset(struct ptptime_t * timeoffset);
void emac_ptptime_adjfreq(int32_t Adj);
#endif

err_t ethernetif_init_E(struct netif *netif);
err_t ethernetif_input_E(struct netif *netif);
struct netif *ethernetif_register(void);
int ethernetif_poll(void);
void lwip_set_mac_address(unsigned char* macadd);
u32_t emac_ptpsubsecond2nanosecond(u32_t subsecondvalue);

#ifdef SERVER

#define MAC_ADDR0 0x00
#define MAC_ADDR1 0x00
#define MAC_ADDR2 0x00
#define MAC_ADDR3 0x00
#define MAC_ADDR4 0x00
#define MAC_ADDR5 0x01

#else

#define MAC_ADDR0 0x00
#define MAC_ADDR1 0x00
#define MAC_ADDR2 0x00
#define MAC_ADDR3 0x00
#define MAC_ADDR4 0x00
//#define MAC_ADDR5 0x02
#define MAC_ADDR5 0x03
//#define MAC_ADDR5 0x04

#endif

#endif
