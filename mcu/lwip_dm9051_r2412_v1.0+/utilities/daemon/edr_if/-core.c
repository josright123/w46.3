/**
 * @file     dm9051_data.c
 * @brief    DM9051 Ethernet Controller Driver for LWIP TCP/IP Stack
 * @version  V3.0
 * @date     2023-05-28
 * 
 * @author   Joseph CHANG <joseph_chang@davicom.com.tw>
 * @copyright (c) 2023-2025 Davicom Semiconductor, Inc.
 * 
 * @details
 *   This driver implements the hardware abstraction layer for the DM9051
 *   Ethernet controller, providing initialization, transmit, and receive
 *   functionality integrated with the LWIP TCP/IP stack.
 * 
 *   Key Features:
 *   - Full integration with uip and LWIP TCP/IP stack
 *   - Support for interrupt and polling modes
 *   - Configurable MAC address handling
 *   - Error detection and recovery mechanisms
 * 
 * @note    First verification: AT32F415
 */
#if RXHDL_V51 == RXHDL_OLD
/* Standard Library Includes */
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <stdlib.h>

/* Project Specific Includes */
#include "platform_info/control/drv_control/conf_core.h"
#include "platform_info/control/drv_control/dm9051_drv_debug.h"

/*-----------------------------------------------------------------------------
 * Public drv_impl Functions
 *-----------------------------------------------------------------------------*/
 
#define DRV_TYPE 1
#include "platform_info/control/drv_types_define.h"
#define DRV_TYPE 2
#include "platform_info/control/drv_types_define.h"

/*-----------------------------------------------------------------------------
 * Public dm_impl Functions
 *-----------------------------------------------------------------------------*/

/* Type Definitions [Public dm_types Functions, belong to dm9051.c] */
//#define DM_TYPE 1
//#include "dm_types_define.h"
//#define DM_TYPE 2
//#include "dm_types_define.h"
#endif
