#ifndef __ETHERNETIF_TYPES_H__
#define __ETHERNETIF_TYPES_H__
//#include <stdbool.h>
//#include "lwip/err.h"
//#include "lwip/netif.h"

/**
 * Helper struct to hold private data used to operate your ethernet interface.
 * Keeping the ethernet address of the MAC in this struct is not necessary
 * as it is already kept in the struct netif.
 * But this is only an example, anyway...
 */
#if 0
struct ethernetif
{
  struct eth_addr *ethaddr;
  /* Add whatever per-interface state that is needed here. */
  int dm_hardware_ptp_ts;
  int unused;
};
#endif

#endif //__ETHERNETIF_TYPES_H__
