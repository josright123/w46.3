#ifndef __ETHERNETIF_V51_H__
#define __ETHERNETIF_V51_H__
#include <stdbool.h>
#include "lwip/err.h"
#include "lwip/netif.h"

//#include "FreeRTOS.h"
//#include "task.h"
//#include "queue.h"
//#include "semphr.h"

/* 定义一个信号量用于PHY接受数据同步 */
//extern SemaphoreHandle_t PHY_RX_xSemaphore; //JJ.COMMENTED
/* 定义一个二值信号量用于PHY发送数据同步 */
//extern SemaphoreHandle_t PHY_TX_xSemaphore;

uint8_t *get_ReceiveBuffer(void);
uint8_t *get_TransmitBuffer(void);

//[To be re-arranged.]
//err_t ethernetif_init(struct netif *netif);
//err_t ethernetif_input_poll(struct netif *netif); //may be need: err_t xxx{}
err_t ethernetif_input(struct netif *netif);
//struct netif *ethernetif_register(void);
//int ethernetif_poll(void);

err_t tcpip_msging_pkt(struct pbuf *p, struct netif *inp);
//struct pbuf *low_level_input(struct netif *netif);= DM_ETH_Input_W();

void low_level_status(uint8_t *stat); //void low_level_status(struct netif *netif, uint8_t *stat);
uint16_t ethernetif_link_polling(struct netif *netif, uint8_t *stat);

//extern int link_startup_update_flg;
//extern int link_change_flg;
void ethernetif_polling_downup(struct netif *netif);

void link_quick_check(struct netif *netif);

struct pbuf *input_intr(void);
//bool process_input_pbuf(struct pbuf *p);
//err_t ethernetif_input_intr(struct netif *netif);

//#ifdef SERVER

//#define MAC_ADDR0 0x00
//#define MAC_ADDR1 0x00
//#define MAC_ADDR2 0x00
//#define MAC_ADDR3 0x00
//#define MAC_ADDR4 0x00
//#define MAC_ADDR5 0x01

//#else

//#define MAC_ADDR0 0x00
//#define MAC_ADDR1 0x00
//#define MAC_ADDR2 0x00
//#define MAC_ADDR3 0x00
//#define MAC_ADDR4 0x00
////#define MAC_ADDR5 0x02
//#define MAC_ADDR5 0x03
////#define MAC_ADDR5 0x04

//#endif

#endif 
