#ifndef __AT32_EMAC_ELI_H
#define __AT32_EMAC_ELI_H

#ifdef __cplusplus
 extern "C" {
#endif

/* includes ------------------------------------------------------------------*/
//#include "at32f403a_407.h"
//#include "netif.h"
#include "netif.h"

/** @addtogroup AT32F407_periph_examples
  * @{
  */

/** @addtogroup 407_EMAC_telnet
  * @{
  */

//#define tmr_configuration()
//uint16_t link_update(struct netif *netif);
//void ethernetif_notify_conn_changed(struct netif *netif);
//void emac_tmr_init(void);
uint8_t network_ip_is_valid(struct netif *netif);

//#include "lwip/netif.h"
/* to be [platform_info_w.h]" */
void ethernetif_notify_conn_changed(struct netif *netif);
int ethernetif_set_link(void const *argument);

/**
  * @}
  */

/**
  * @}
  */

#ifdef __cplusplus
}
#endif

#endif //__AT32_EMAC_ELI_H
