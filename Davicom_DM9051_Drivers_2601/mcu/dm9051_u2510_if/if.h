/* "if.h"
 */#ifndef __INTERFACE_H
#define __INTERFACE_H

#include <stdint.h>
#include "core/dm9051.h"
#include "../../dm9051_u2510_if/platform_info.h" //"all, as main.c including"

#define DM_DEBUG_TYPE 0
#include "../../dm9051_u2510_if/common/nosys/hex.h"

/* plat sections domain
 */
#define IF_PLAT_DECAL			0
#define IF_PLAT_UIP				1
#define IF_PLAT_LWIP			2
#define IF_PLAT_COMMON			3

#define DM_PLAT_TYPE	0  //(=0) [=IF_PLAT_DECAL, extern]
#include "../../dm9051_u2510_if/common/port/plat.h"

#define DM_NOUSED_ARG(x) (void)x
#define DM_UNUSED_ARG(x) (void)x
#define DM_NONUSED_ARG(x) (void)x

#define NULL_VALUE		0

#define	printk	printkey
#define	printkey(fmt, ...) \
	do { \
		char debug_msg[256]; \
		char *p = debug_msg; \
		snprintf(debug_msg, sizeof(debug_msg), fmt, ##__VA_ARGS__); \
		while(*p) \
			putchar(*p++); /*fputc_dbg(*p++);*/ \
	} while(0)

void dm9051_config_app_mcu_name(char *name);
char *dm_eth_app_mcu_name(char *name);
int dm_eth_polling_downup(int dhcp_en);
void print_up(char *up_head);
//void diff_rx_s(void);
//void diff_rx_e(void);

#endif //__INTERFACE_H
