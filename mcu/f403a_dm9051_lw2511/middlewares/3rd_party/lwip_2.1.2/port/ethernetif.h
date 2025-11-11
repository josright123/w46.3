#ifndef __ETHERNETIF_H__
#define __ETHERNETIF_H__


#include "lwip/err.h"
#include "lwip/netif.h"
//#include "lwip_ethif/nosys/ethernetif_types.h"

//struct ptptime_t {
//  s32_t tv_sec;
//  s32_t tv_nsec;
//};

err_t ethernetif_init(struct netif *netif);
err_t ethernetif_input(struct netif *netif);
struct netif *ethernetif_register(void);
int ethernetif_poll(void);
//void lwip_set_mac_address(unsigned char* macadd);
//u32_t emac_ptpsubsecond2nanosecond(u32_t subsecondvalue);

/* added */
//void periodic_handle(volatile uint32_t localtime, struct netif *netif);
//void start_ptpd_run(void);
	
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
