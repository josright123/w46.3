#ifndef __DM9051_OPTS_H
#define __DM9051_OPTS_H
#include "stdint.h"
#include "stdio.h"
#include <string.h>

/* __DRV_H
 */

#define ETHERNET_INTERRUPT_MODE // interrupt mode
//#undef ETHERNET_INTERRUPT_MODE

#define FORCE_MODE_10M // 10M mode
//#undef FORCE_MODE_10M

//#define FORCE_CHKSUM_OFFLOAD // checksum offload mode
//#undef FORCE_CHKSUM_OFFLOAD

#define FORCE_RCR_ALL // Pass all multi-cast
//#undef FORCE_RCR_ALL

//#define FORCE_STOP_IF_DM9051_NOT_FOUND // not pass through mode
//#undef FORCE_STOP_IF_DM9051_NOT_FOUND

/* __USR_H
 */

#include "dm9051opts_define.h"

#endif //__DM9051_OPTS_H
