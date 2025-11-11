#ifndef __DM_ETH_H__
#define __DM_ETH_H__
#include "platform_info/control/dm9051opts.h"

//[uip support version]
//const uint8_t *DM_ETH_Init(const uint8_t *adr);
//uint16_t DM_ETH_Input(uint8_t *bff);
//void DM_ETH_Output(uint8_t *bff, uint16_t len);
//.#if LWIP_PTP
//uint16_t DM_ETH_PTP_Input(uint8_t *bff, uint8_t *ts_bff);
//void DM_ETH_PTP_Output(uint8_t *bff, uint16_t len, uint8_t *ts_bff);
//void DM_ETH_PTP_HW_TIMESTAMP_Output(uint8_t *bff, uint16_t len, uint8_t *ts_bff);
//.#endif
const uint8_t *DM_ETH_Ip_Configuration(const uint8_t *ip);
const uint8_t *DM_ETH_Gw_Configuration(const uint8_t *ip);
const uint8_t *DM_ETH_Mask_Configuration(const uint8_t *ip);
uint8_t *DM_ETH_Ip_Configured(void);
uint8_t *DM_ETH_Gw_Configured(void);
uint8_t *DM_ETH_Mask_Configured(void);
int DM_Eth_Info_Linkup(uint8_t *stat);
void DM_Eth_Read_Info(uint8_t *stat);
void DM_Eth_Show_status_W(char *head, uint8_t *statdat, int force);
int dm_eth_polling_downup(void);
//static void dm_eth_poll_event(void);
void dm_eth_show_app_help_info(char *drv_modeS, char *statusS);
void dm_eth_show_app_help_info_w(char *drv_modeS, char *root_dirS, char *prjS, char *locStr);
void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, char *clkTypeStr);
//const uint8_t *dm_eth_show_identified_ip(char *headtypestr);
//const uint8_t *dm_eth_show_identified_gw(char *headtypestr);

enum operate_tag {OPS_LED3 = 0, };
void dm_eth_polling_button_init(enum operate_tag tag);
void dm_eth_polling_button_ops(enum operate_tag tag);

#ifdef ETHERNET_INTERRUPT_MODE
void DM_ETH_IRQInit(void);
int DM_ETH_IRQEvent(void);
#endif
//void DM_ETH_ToRst_ISR(void);
#if DM_ETH_DEBUG_MODE
uint16_t DM_ETH_ToCalc_rx_pointers(int state, const uint16_t *mdra_rd_org, uint16_t *mdra_rdp);
#endif
#endif //__DM_ETH_H__
