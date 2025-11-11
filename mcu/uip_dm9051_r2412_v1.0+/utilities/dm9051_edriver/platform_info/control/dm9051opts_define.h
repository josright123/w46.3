/* 
 * dm9051_lw_driver_define.h
 */
#ifndef __DM9051_LW_DRIVER_DEF_H
#define __DM9051_LW_DRIVER_DEF_H
#include "platform_info/control/dm9051opts.h"

//[1]
#ifndef ETHERNET_INTERRUPT_MODE
#define	ETHERNET_POLLING_MODE
#endif

#ifdef ETHERNET_INTERRUPT_MODE
#define RX_MODE_STR "interrupt"
#endif

#ifdef ETHERNET_POLLING_MODE
#define RX_MODE_STR "polling"
#endif


/* Check LINK state from ops in source 
 * Use [NSR/NSR] 0
 * Use [BMSR]    1
 */
enum link_state_t {DM9051_CHECK_MAC = 0, DM9051_CHECK_PHY};

#define LINK_STATE_SOURCE 	DM9051_CHECK_MAC

#endif //__DM9051_LW_DRIVER_DEF_H
