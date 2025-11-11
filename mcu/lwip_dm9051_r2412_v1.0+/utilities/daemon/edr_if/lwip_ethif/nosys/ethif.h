#ifndef __DM_ETHIF_API_H__
#define __DM_ETHIF_API_H
#include "lwip/netif.h" 
#include "ethernetif_types.h"

//[netif api]
const uint8_t *DM_ETH_Init_W(struct netif *netif, const uint8_t *adr);
struct pbuf *DM_ETH_Input_W(void);
struct pbuf *DM_ETH_Input_RT(void);
void DM_ETH_Output_W(struct pbuf *p);
void DM_ETH_Output_NW(struct pbuf *p, struct ethernetif *ethernetif);

int DM_ETHER_Receive_Route_W(void);

//void print_task_list_R(unsigned long disppc, unsigned long base_count_value, char *head, unsigned long int_rcv_task_cnt);
//void DM_Eth_Poll_W(void);

#endif //__DM_ETHIF_API_H
