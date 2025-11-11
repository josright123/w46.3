/**
 **************************************************************************
 * @file     hex_dump.c
 * @brief    DM9051 Ethernet driver debug ops file
 * 
 * @details To restructure this file to enhance maintainability,
 *          and potentially performance.
 * 
 * @version  v1.0.1
 * @author   Joseph CHANG <joseph_chang@davicom.com.tw>
 * @copyright (c) 2025-2027 Davicom Semiconductor, Inc.
 * @date     2025-10-22
 **************************************************************************
 */
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"
 
/* Debug Type Definitions */
//#define DM_DEBUG_TYPE 10
//#include "dump_info.h"
//#define DM_DEBUG_TYPE 11
//#include "dump_info.h"
//#define DM_DEBUG_TYPE 20
//#include "dump_info.h"
//#define DM_DEBUG_TYPE 21
//#include "dump_info.h"

//optional-1: #define DM_PLAT_TYPE	IF_PLAT_UIP  //(=1)
//optional-2: #define DM_PLAT_TYPE	IF_PLAT_LWIP //(=2)

#define DM_PLAT_TYPE	IF_PLAT_LWIP //(=2)
#include "plat.h"

#define DM_PLAT_TYPE	3 //(=3) [=IF_PLAT_COMMON, app print info]
#include "plat.h"

struct netif *plat_netif = NULL;
void dm9051_config_if(struct netif *set_netif)
{
	plat_netif = set_netif;
//#if 1
//	print_up("tcpip_stack_init.dm9051_config_if(test)");
//#endif
}
struct netif *dm_eth_netif(void)
{
	//if (netif) {
	//	return plat_netif = netif;
	//}

	if (plat_netif) {
		return plat_netif;
	}

	printf("plat_lwip need the app set *netif by dm9051_config_if()\r\n");
	while(1);
}
