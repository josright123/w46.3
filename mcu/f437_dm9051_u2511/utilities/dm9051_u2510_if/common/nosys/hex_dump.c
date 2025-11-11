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
#define DM_DEBUG_TYPE 10
#include "hex.h"
#define DM_DEBUG_TYPE 11
#include "hex.h"
#define DM_DEBUG_TYPE 20
#include "hex.h"
#define DM_DEBUG_TYPE 21
#include "hex.h"

//optional-1: #define DM_PLAT_TYPE	IF_PLAT_UIP  //(=1)
//optional-2: #define DM_PLAT_TYPE	IF_PLAT_LWIP //(=2)

//#define DM_PLAT_TYPE	IF_PLAT_LWIP //(=2)
//#include "plat.h"

//#define DM_PLAT_TYPE	3 //(=3) [=IF_PLAT_COMMON, app print info]
//#include "plat.h"

char mac_name_store[8] = {'a', 'r', 'm'};
void dm9051_config_app_mcu_name(char *name) {
	strcpy(mac_name_store, name);
}
char *dm_eth_app_mcu_name(char *name)
{
	if (name)
		strcpy(mac_name_store, name);
	return mac_name_store;
}
