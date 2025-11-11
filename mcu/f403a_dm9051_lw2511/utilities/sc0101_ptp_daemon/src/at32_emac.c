/**
  **************************************************************************
  * @file     at32_emac.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    DM9051 ethernet controller driver for AT32F403A
  *
  * @details  [Refactored for improved call stack clarity and readability.]
  */

/* includes ------------------------------------------------------------------*/
#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"
#include "lwip/priv/tcp_priv.h"
#include "netif/etharp.h"
#include "netif.h"
#include "netconf.h"
#include "../../dm9051_u2510_if/ip_status.h"

//extern const uint8_t local_ip[ADDR_LENGTH]; //= {192, 168, 6, 117};
//extern const uint8_t local_gw[ADDR_LENGTH]; //= {192, 168, 6, 1};
//extern const uint8_t local_mask[ADDR_LENGTH]; //= {255, 255, 255, 0};

extern volatile uint32_t local_time;
extern u32_t lwip_sys_now;

/* MQTT 應用程式 */
//#if _LWIP_MQTT
//#include "developer_conf.h"
//#endif

/* defines -------------------------------------------------------------------*/
//#define ADDR_LENGTH                      (4)
#define LINK_TIMER_MAX                   (0xf0000000/500000)
#define SYSTEMTICK_PERIOD_MS             10

/* Function prototypes for internal functions --------------------------------*/
//uint16_t _link_update(struct netif *netif);
//static void lwip_periodic_display_handle(char *head, uint16_t regvalue, struct netif *netif);
/* static variables ---------------------------------------------------------*/
//static int dhcp_start_first_flag = 1;
volatile uint32_t tcp_timer = 0;
volatile uint32_t arp_timer = 0;
//#if LWIP_DHCP
//volatile uint32_t dhcp_fine_timer = 0;
//volatile uint32_t dhcp_coarse_timer = 0;
//#endif

/* ===============================
 * Section: Link Management
 * =============================== */
/**
 * @brief  Update link states by reading DM9051 register
 * @note   Reads the network status register to determine link state
 * @param  netif: pointer to network interface structure
 * @retval link state: 0 = disconnect, 1 = connection
 */
//uint16_t link_update(struct netif *netif)
//{
//    uint8_t nsr = HAL_read_reg(DM9051_NSR);
//    uint16_t regvalue = (nsr & NSR_LINKST) >> 6;
//    return link_display(regvalue, netif);
//}

/**
 * @brief = _dm_eth_polling_downup()
 * @brief  Set network interface link status
 * @note   Manages the netif link status based on physical link state
 * @param  argument: pointer to network interface structure
 * @retval 1 if link status changed to up, 0 otherwise
 */
//int ethernetif_set_link(void const *argument)
//{
//	int on_dhcp = LWIP_DHCP;
//    struct netif *netif = (struct netif *)argument;
//    //int linkchg_up = 0;
//	uint16_t regvalue = link_update(netif);
//	if (!netif_is_link_up(netif) && regvalue) {
//		netif_set_link_up(netif);
//		//printf("dm9051 link up\r\n");
//		printf("%s dm9051 link up\r\n", on_dhcp ? "DHCP" : "static IP");
//		//#if _LWIP_MQTT
//		//#endif
//		return 1; //linkchg_up = 1;
//	} else if (netif_is_link_up(netif) && !regvalue) {
//		netif_set_link_down(netif);
//		//printf("dm9051 link down\r\n");
//		printf("%s dm9051 link down\r\n", on_dhcp ? "DHCP" : "static IP");
//		//#if _LWIP_MQTT
//		//#endif
//	}
//    return 0; //return linkchg_up;
//}

//static
//void set_link(struct netif *netif)
//{
//    //static uint8_t periodic_set_link_tries = 0;
//    //if (++periodic_set_link_tries >= 5) {
//    //  periodic_set_link_tries = 0;
//		int linkchg_up = ethernetif_set_link(netif);
//		if (linkchg_up) {
//			if (LWIP_DHCP)
//				printf("to_alloc_ip, current ip %d.%d.%d.%d\r\n", 0, 0, 0, 0);
//			else
//				printf("local_ip %d.%d.%d.%d\r\n", 
//					ip4_addr1(&netif_IpAddr), ip4_addr2(&netif_IpAddr), 
//					ip4_addr3(&netif_IpAddr), ip4_addr4(&netif_IpAddr)
//					);
//		}
//	//}
//}

/**
 * @brief  檢查網路介面是否已取得有效的 IP 位址
 * @retval 1 如果 IP 有效，0 如果無效（0.0.0.0）
 */
uint8_t network_ip_is_valid(struct netif *netif)
{
  return !ip4_addr_isany(netif_ip4_addr(netif));
}

//int ptpd_init = 0;

//void start_ptpd_run(void)
//{
//	if (!ptpd_init) {
//		printf("\r\n");
//		printf("\r\n");
//		print_ptpd_ver();
//		printf("[ptpd.init]\r\n");
//		PTPd_Init();
//		PTPd_Init_manager_setup();
//		ptpd_init = 1;
//		printf("[ptpd.init].all.done\r\n");
//		print_ptpd_ver();
//		printf("\r\n");
//		printf("\r\n");
//	}
//}

//void dm_ptpd_periodic(volatile uint32_t localtime)
//{
//	if (ptpd_init)
//	{
//		ptpd_Periodic_Handle(localtime);
//	}
//}

//void periodic_ip_ptpd_run(struct netif *netif)
//{
//#if LWIP_DHCP
//	if (net_ip_done())
//		start_ptpd_run();
//#else
//	start_ptpd_run();
//#endif
//}

/**
 * @brief  Main periodic handler for network stack
 * @note   Entry point for all periodic network tasks (timers, DHCP, link)
 * @param  localtime: current system time
 * @param  netif: pointer to network interface structure
 * @retval none
 */
//void periodic_handle(volatile uint32_t localtime, struct netif *netif)
//{
//}

enum {
  MY_PHY_LINKDOWN,
  MY_PHY_LINKUP
};

/**
 * @brief  Display link status and control LEDs
 * @note   Updates LED indicators based on link status
 * @param  regvalue: link status register value
 * @param  netif: pointer to network interface structure
 * @retval link status value
 */
//uint16_t link_display(uint16_t regvalue, struct netif *netif)
//{
//    static uint32_t mc_timer = 0;
//    if (mc_timer < LINK_TIMER_MAX) {
//        mc_timer++;
//        if (mc_timer == 1) {
//            lwip_periodic_display_handle("startup", regvalue, netif);
//        } else {
//            lwip_periodic_display_handle("mainloop", regvalue, netif);
//        }
//    }
////    if (regvalue) {
////        at32_led_on(LED4);
////        at32_led_off(LED2);
////    } else {
////        at32_led_on(LED2);
////        at32_led_off(LED4);
////    }
//    return regvalue;
//}

/**
 * @brief  Handle periodic display for lwip tasks
 * @note   Manages DHCP startup and link status display
 * @param  head: display header string
 * @param  regvalue: link status register value
 * @param  netif: pointer to network interface structure
 * @retval none
 */
//static void lwip_periodic_display_handle(char *head, uint16_t regvalue, struct netif *netif)
//{
//    if (dhcp_start_first_flag) {
//#if !LWIP_DHCP
//        if (regvalue) {
//            printf("%s link %s\r\n", head, "up");
//            printf("local_ip %d.%d.%d.%d\r\n", 
//                   local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
//            dhcp_start_first_flag = 0;
//        } else {
//            printf("%s link %s\r\n", head, "down");
//        }
//        if (dhcp_start_first_flag == 1) {
//            printf("local_ip %d.%d.%d.%d\r\n", 
//                   local_ip[0], local_ip[1], local_ip[2], local_ip[3]);
//            dhcp_start_first_flag = 2;
//        }
//#endif
//#if LWIP_DHCP
//        if (regvalue) {
//            printf("%s link %s\r\n", head, "up");

//			if (netif_is_up(netif)) {
//				if (dhcp_start_first_flag) {
//					printf("%s link %s (netif is up, dhcp_start())\r\n", head, "up");
//printf("--------------------------- dhcp_start(netif)  ------------------ \r\n");
//printf(" LINK-UP, on lwip_periodic_display_handle, Call dhcp_start(netif)\r\n");
//					dhcp_start_first_flag = 0;
//					dhcp_start(netif);
//				}
//			}
//			else
//				printf("%s link %s (!netif is not change to up, NOT dhcp_start())\r\n", head, "up");
//        } else {
//            printf("%s link %s\r\n", head, "down");
//        }
//#endif
//    }
//}

/* ===============================
 * Section: Network Interface Status Change
 * =============================== */
/**
 * @brief  Notify user about link status changes
 * @note   Handles network interface state changes
 * @param  netif: pointer to network interface structure
 * @retval none
 */
//void ethernetif_notify_conn_changed(struct netif *netif)
//{
//    if (netif_is_link_up(netif)) {
//        netif_set_up(netif);
//#if LWIP_DHCP
//		printf("(up) notify\r\n");
//printf("----------------------------- dhcp_start(netif)  ------------------ \r\n");
//printf(" LINK-UP, on link_changed, Call dhcp_start(netif)\r\n");
//        dhcp_start(netif);
//#else
//		printf("(up %d.%d.%d.%d) notify\r\n",
//			ip4_addr1(&netif->ip_addr), ip4_addr2(&netif->ip_addr),
//			ip4_addr3(&netif->ip_addr), ip4_addr4(&netif->ip_addr));
//#endif
//    } else {
//		printf("(down)\r\n");
//        netif_set_down(netif);
//    }
//}

/* ===============================
 * Section: Utility Functions
 * =============================== */
/**
 * @brief  updates the system local time
 * @param  none
 * @retval none
 */
void time_update(void)
{
    local_time += SYSTEMTICK_PERIOD_MS;
	lwip_sys_now += SYSTEMTICK_PERIOD_MS;
}

//#define ROUNDER_DEBUG_COMPARE

/**
 * @brief  Example function showing both TMR6 and SysTick configurations
 * @note   This function demonstrates how to configure either TMR6 or SysTick
 *         for system timing. Choose one approach based on your requirements.
 * @param  use_systick: TRUE to use SysTick, FALSE to use TMR6
 * @retval none
 */
confirm_state identified_use_systick = OPTIONAL_SYSTICK_CORE_CM4;

confirm_state identify_of_systick;
confirm_state identify_of_tmr;

void system_timer_init(void)
{
	//confirm_state use_systick = OPTIONAL_SYSTICK_CORE_CM4;
    if (OPTIONAL_SYSTICK_CORE_CM4)
    {
//        /* 
//         * SysTick Configuration (Recommended for most applications)
//         * 
//         * Advantages:
//         * - Built into ARM Cortex-M4 core
//         * - Highest interrupt priority
//         * - Simple configuration
//         * - Standard for RTOS and system timing
//         * - No peripheral clock setup required
//         * 
//         * Disadvantages:
//         * - Limited to one timer
//         * - Fixed priority level
//         * - May conflict with RTOS usage
//         */
        systick_init();
		#ifdef ROUNDER_DEBUG_COMPARE
        emac_tmr_init();
		#endif

		//_identified_use_systick = use_systick;
		identify_of_systick = OPTIONAL_SYSTICK_CORE_CM4;
		identify_of_tmr = identify_of_systick ? FALSE : TRUE; //= !OPTIONAL_SYSTICK_CORE_CM4;
    }
    else
    {
//        /* 
//         * TMR6 Configuration (Current implementation)
//         * 
//         * Advantages:
//         * - Configurable priority
//         * - Multiple timers available
//         * - More flexible for complex timing requirements
//         * - Can be used alongside SysTick
//         * 
//         * Disadvantages:
//         * - Requires peripheral clock setup
//         * - More complex configuration
//         * - Uses peripheral resources
//         */
		#ifdef ROUNDER_DEBUG_COMPARE
        systick_init();
		#endif
        emac_tmr_init();

		//_identified_use_systick = use_systick;
		identify_of_tmr = OPTIONAL_SYSTICK_CORE_CM4;
		identify_of_systick = identify_of_tmr ? FALSE : TRUE; //= !OPTIONAL_SYSTICK_CORE_CM4;
    }
}

#ifdef ROUNDER_DEBUG_COMPARE
volatile uint32_t local_systick_time = 0;
volatile uint32_t local_tmr_time = 0;
volatile uint32_t c_systick_time = 10000;
volatile uint32_t c_tmr_time = 10000;
#endif

void system_time_update(confirm_state use_systick)
{
    if (use_systick == identified_use_systick)
		time_update();
#ifdef ROUNDER_DEBUG_COMPARE
	if (use_systick == identify_of_systick)
		local_systick_time++;
	if (use_systick == identify_of_tmr)
		local_tmr_time++;
#endif
}

void lwip_periodic_systick_time_display(void)
{
#ifdef ROUNDER_DEBUG_COMPARE
	if (local_systick_time >= c_systick_time) {
		c_systick_time += 10000;
		printf("[               %u local_systick_time                ]\r\n", local_systick_time);
	}
#endif
}

void lwip_periodic_tmr_time_display(void)
{
#ifdef ROUNDER_DEBUG_COMPARE
	if (local_tmr_time >= c_tmr_time) {
		c_tmr_time += 10000;
		printf("[               %u local_tmr_time                ]\r\n", local_tmr_time);
	}
#endif
}

//void system_time_periodic_display(void)
//{
//	lwip_periodic_systick_time_display();
//	lwip_periodic_tmr_time_display();
//}
