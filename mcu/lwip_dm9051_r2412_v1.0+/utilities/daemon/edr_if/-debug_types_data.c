/**
 * @file     debug_types_data.c
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
///* Standard Library Includes */
//#include <stdio.h>
//#include <stdarg.h>
//#include <string.h>
//#include <stdlib.h>

///* Project Specific Includes */
//#include "platform_info/control/drv_control/conf_core.h"
//#include "platform_info/control/drv_control/dm9051_drv_debug.h"

#include "platform_info/control/drv_control/conf_core.h"
#include "platform_info/control/drv_control/dm9051_drv_debug.h"

/* Basic Type Definitions */
#include "platform_info/nosys/uip_eth/types_define_all.h"

/* Debug Type Definitions */
#define DM_DEBUG_TYPE 10
#include "platform_info/nosys/uip_eth/debug_types_define.h"
#define DM_DEBUG_TYPE 11
#include "platform_info/nosys/uip_eth/debug_types_define.h"
#define DM_DEBUG_TYPE 20
#include "platform_info/nosys/uip_eth/debug_types_define.h"
#define DM_DEBUG_TYPE 21
#include "platform_info/nosys/uip_eth/debug_types_define.h"
#endif
