/* def.h
 */
#ifndef __DM_DEF_H__
#define	__DM_DEF_H__
#include <stdint.h>
#include <stdio.h>

/* domain */
#define MAC_ADDR_LENGTH                  		(6)
#define ADDR_LENGTH                      		(4)

/* struct define */
  struct eth_node_t
  {
    uint8_t mac_addresse[MAC_ADDR_LENGTH];
  };

  struct ip_node_t
  {
    //uint8_t mac_addresse[MAC_ADDR_LENGTH];
    uint8_t local_ipaddr[ADDR_LENGTH];
    uint8_t local_gwaddr[ADDR_LENGTH];
    uint8_t local_maskaddr[ADDR_LENGTH];
  };
  
  struct ip_data_t
  {
    uint8_t ipaddr[ADDR_LENGTH];
  };

	/* types define : [ identify.h/identify.c ] */
	typedef uint8_t mac_t[MAC_ADDR_LENGTH];
	typedef uint8_t ip_t[ADDR_LENGTH];

/* types define */
  typedef enum
  {
    CS_EACH = 0,
    CS_LONG,
  } csmode_t;

//[use define otherwise not validated retrival by #.if (xxx == SEMA_ON)]
//	enum sema_tag_t {
//		SEMA_OFF = 0,
//		SEMA_ON,
//	};
#define	SEMA_OFF	0
#define	SEMA_ON		1

//[1.1]
typedef enum
{
  DM_FALSE = 0,
  DM_TRUE = !DM_FALSE,
} enable_t;

#define DM_UNUSED_ARG(x) (void)x
#define DM_NONUSED_ARG(x) (void)x

/* domain */
#define	uipAPP_CONF	1
#define	lwipAPP_CONF	2
  
/* eth_main */
//hdlr
//#define	DM_ETH_IRQHandler	EXINT9_5_UserFunction	(EXINT9_5_IRQHandler)

/* dbg_def */
//nosys
#define	PRINT_SEMA	SEMA_OFF
#define PRINT_INFO_AP	"[iAP] "
#define PRINT_INFO_APIN	"[APIN] "

//rt
#define	PRINT_SEMA_RT	SEMA_OFF //SEMA_ON //ON to print with sema, OFF without sema.
#define PRINT_INFO_RT	"[iRT]  "

//drv
#define PRINT_INFO	"[DRV] "
#define	PRINT_INFO_IRQ	"[IRQ] "

// Debug levels
typedef enum {
    DM9051_ETH_DEBUG_LEVEL_ERROR = 0,
    DM9051_ETH_DEBUG_LEVEL_DEBUG,
    DM9051_ETH_DEBUG_LEVEL_WARN,
    DM9051_ETH_DEBUG_LEVEL_INFO,
} dm9051_eth_debug_level_t;

static const char* const LEVEL_STRINGS[] = {
	"ERROR", //[DM9051_ETH_DEBUG_LEVEL_ERROR] = 
	"DEBUG", //[DM9051_ETH_DEBUG_LEVEL_DEBUG] = 
	"WARN", //[DM9051_ETH_DEBUG_LEVEL_WARN]  = 
	"INFO", //[DM9051_ETH_DEBUG_LEVEL_INFO]  = 
};

/* Implementation of the debug handler
 */

#define LOG_LEVEL DM9051_ETH_DEBUG_LEVEL_DEBUG

#define dm9051_eth_debug_handler(level, message) do { \
	if (level >= LOG_LEVEL) { \
		printf("[%s] %s", level_str_impl(level), message);	 \
    } \
} while(0)

/* Implementation the essential-printf
 *  - printkey is essential print. while
 *  - printf, printk are optional print, 
 *    check to dm9051_ap_debug.h/dm9051_drv_debug.h (and dm9051_plat_debug.h)
 */

#define	printkey(fmt, ...) \
	do { \
		char debug_msg[256]; \
		char *p = debug_msg; \
		snprintf(debug_msg, sizeof(debug_msg), fmt, ##__VA_ARGS__); \
		while(*p) \
			putchar(*p++); /*fputc_dbg(*p++);*/ \
	} while(0)

//#define DM9051_HEXDUMP_RESET(expression, handler) do { if ((expression)) { \
//	(handler);}} while(0)

#define DM9051_RX_BREAK(expression, handler) \
  do                                         \
  {                                          \
    if ((expression))                        \
    {                                        \
      handler;                               \
    }                                        \
  } while (0)

#define DM9051_TX_DELAY(expression, handler) \
	do                                       \
	{                                        \
		if ((expression))                    \
		{                                    \
			handler;                         \
		}                                    \
	} while (0)
	
#endif //__DM_DEF_H__
