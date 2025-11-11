#ifndef __DM_ETH_RT_API_H__
#define __DM_ETH_RT_API_H__
#include "lwip/netif.h" 
#include "ethernetif_types.h"

//[lwip]

const uint8_t *DM_ETH_Init_W(struct netif *netif, const uint8_t *adr);
struct pbuf *DM_ETH_Input_W(void);
struct pbuf *DM_ETH_Input_RT(void);
void DM_ETH_Output_W(struct pbuf *p);
void DM_ETH_Output_NW(struct pbuf *p, struct ethernetif *ethernetif);

void DM_ETH_ToRst_ISR_W(void);
void DM_Eth_ReadRegsInfo_W(uint8_t *stat);

//uint16_t DM_ETH_ToCalc_rx_pointers_W(int state, const uint16_t *mdra_rd_org, uint16_t *mdra_rdp);
uint16_t DM_ETH_Diff_rx_pointers_W(int state, const uint16_t *mdra_rd_org, uint16_t *mdra_rdp);

void DM_Eth_Show_status_W(char *head, uint8_t *statdat, int force);

void DM_Eth_CreateRecvTask_R(void);

//int _freertos_task_tryDisplay_R(char *head, unsigned long base_countf, unsigned long base_count_value, unsigned long *dispp);  //OF DM9051_RTOS.H
//void print_task_list_R(unsigned long disppc, unsigned long base_count_value, char *head, unsigned long int_rcv_task_cnt);

void DM_Eth_Poll_W(void);
void DM_ETH_SHOW_CONSECUTIVE_Packets(void);

#endif //__DM_ETH_RT_API_H__
