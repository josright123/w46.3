#ifndef __DM9051_ETH_DEBUG_H
#define __DM9051_ETH_DEBUG_H
#include "platform_info/control/dbg_opts.h"

//drv
#if drv_print
#define printf(fmt, ...) do { \
            char debug_msg[256]; \
            snprintf(debug_msg, sizeof(debug_msg), "%s" fmt, PRINT_INFO, ##__VA_ARGS__); \
			dm9051_eth_debug_handler(LOG_LEVEL, debug_msg); \
		} while(0)
#define	printk	printkey
#else //others
#define	printf(fmt, ...)
#define	printk(fmt, ...)
#endif

#endif //__DM9051_ETH_DEBUG_H
