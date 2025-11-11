/* conf.h
 */
#ifndef __DM_CONF_H__
#define	__DM_CONF_H__

/* domain */
#include "cdef.h"

/* hal, such as designed a MCU */
#include "hal/hal_api.h" //#include "hal/dm9051_hal.h"

/* drv util */
#define DRV_TYPE 0
#include "platform_info/control/drv_types_define.h"

/* common util */
//.#define DM_DEBUG_TYPE 0
//.#include "eth/debug_types_define.h"
//.#define DM_TYPE 0
//.#include "core/dm_types_define.h"

/* all common */
#include "dm9051opts.h" //"temp"

/* all common */
//#include "dbg_opts.h"

/* api */
#include "platform_info/nosys/uip_eth/eth_api.h"

#endif //__DM_CONF_H__
