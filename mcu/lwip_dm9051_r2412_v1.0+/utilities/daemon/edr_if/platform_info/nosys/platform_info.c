/**
 **************************************************************************
 * @file     platform_info.c
 * @version  v1.0.1
 * @date     2024-12-12
 * @brief    DM9051 Ethernet driver info file
 **************************************************************************
 *
 * To restructure and improve the file to enhance readability, maintainability,
 * and potentially performance.
 * Last updated: 2024-12-12
 *
 */
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

/*-----------------------------------------------------------------------------
 * Public dm_impl Functions
 *-----------------------------------------------------------------------------*/

/* Type Definitions [Public dm_types Functions, belong to dm9051.c] */
#define DM_TYPE 1
#include "platform_info/nosys/uip_eth/dm_types_define.h"
#define DM_TYPE 2
#include "platform_info/nosys/uip_eth/dm_types_define.h"

/* Interrupt handler mapping */
#define DM_ETH_IRQHandler EXINT9_5_UserFunction

/* Global state variables */
static volatile int flgSemaphore_r = 0;

/**
 * @brief  Interrupt service routine init
 * @note   Handles packet reception
 */
void DM_ETH_IRQInit(void)
{
  flgSemaphore_r = 0;
}

/**
 * @brief  Interrupt service routine for Ethernet events
 * @note   Handles packet reception and updates interrupt statistics
 */
void DM_ETH_IRQHandler(void)
{
  flgSemaphore_r = 1;
  inc_interrupt_count();
  identify_irq_stat(ISTAT_IRQ_NOW2);
  trace_irq_stat(ISTAT_IRQ_NOW2);
  deidentify_irq_stat(ISTAT_IRQ_NOW2);
}

/**
 * @brief  Retrieves pending interrupt events
 * @return 1 if event pending, 0 otherwise
 */
int DM_ETH_IRQEvent(void)
{
  if (flgSemaphore_r) {
    flgSemaphore_r = 0;
    return 1;
  }
  return 0;
}

/**
 * @brief  Network configuration functions
 */
const uint8_t *DM_ETH_Ip_Configuration(const uint8_t *ip)
{
//	static uint8_t ip_printag = 0x1;
//	if (ip_printag & 0x01) {
//		ip_printag &= ~0x01;
//		identify_tcpip_ip(ip);
//		return dm_eth_show_identified_ip(ip ? "config ip" : "candidate ip");
//	}
	return identify_tcpip_ip(ip);
}

const uint8_t *DM_ETH_Gw_Configuration(const uint8_t *ip)
{
//	static uint8_t gw_printag = 0x1;
//	if (gw_printag & 0x01) {
//		gw_printag &= ~0x01;
//		identify_tcpip_gw(ip);
//		return dm_eth_show_identified_gw(ip ? "config gw" : "candidate gw");
//	}
	return identify_tcpip_gw(ip);
}

const uint8_t *DM_ETH_Mask_Configuration(const uint8_t *ip)
{
  return identify_tcpip_mask(ip);
}

/**
 * @brief  Network configuration retrieval functions
 */
uint8_t *DM_ETH_Ip_Configured(void)
{
  return identified_tcpip_ip();
}

uint8_t *DM_ETH_Gw_Configured(void)
{
  return identified_tcpip_gw();
}

uint8_t *DM_ETH_Mask_Configured(void)
{
  return identified_tcpip_mask();
}

/**
 * @brief  Checks if link is up based on configured source
 * @param  stat: Status register values
 * @return 1 if link is up, 0 otherwise
 */
int DM_Eth_Info_Linkup(uint8_t *stat)
{
	enum link_state_t lst = LINK_STATE_SOURCE;
	if (lst == DM9051_CHECK_MAC) {
		return stat[1] & 0x40 ? 1 : 0;  /* NSR register */
	}
	else if (lst == DM9051_CHECK_PHY) {
		return stat[5] & 0x04 ? 1 : 0;  /* BMSR register */
	}
	return 1;
}

/**
 * @brief  Reads and processes register information (could periodic call)
 * @param  stat: Buffer for register values
 */
void DM_Eth_Read_Info(uint8_t *stat)
{
  dm9051_read_regs_info(stat);
  if (!DM_Eth_Info_Linkup(stat))
		/* Resets the hex dump state for input processing */
    dm_eth_input_hexdump_reset();
}

#define SD_100M 0x20
#define SD_10M 0x10
#define SD_FULL 0x02
#define SD_HALF 0x01
static uint8_t speed_duplex = 0;

// Show Ethernet status
void DM_Eth_Show_status_W(char *head, uint8_t *statdat, int force)
{
	uint8_t speeddupd = 0;

	if (DM_Eth_Info_Linkup(statdat))
	{
		if (statdat[0] & 0x08)
			speeddupd |= SD_FULL;
		else
			speeddupd |= SD_HALF;
		if (statdat[1] & 0x80)
			speeddupd |= SD_10M;
		else
			speeddupd |= SD_100M;

		if ((speeddupd != speed_duplex) || force)
		{
			speed_duplex = speeddupd;

			printf("%s Link up as %s %s\r\n",
				   head,
				   speeddupd & SD_100M ? "100M" : speeddupd & SD_10M ? "10M"
																	 : "UN_SPEED",
				   speeddupd & SD_FULL ? "Full" : speeddupd & SD_HALF ? "Half"
																	  : "UN_DUPLEX");
		}
		printk("\r\n");
	}
}

/**
 * @brief  Debug function for RX pointer calculation
 */
#if DM_ETH_DEBUG_MODE
uint16_t DM_ETH_ToCalc_rx_pointers(int state, const uint16_t *mdra_rd_org, uint16_t *mdra_rd_now)
{
  static uint16_t dummy_rwpa;
  dm9051_read_rx_pointers(&dummy_rwpa, mdra_rd_now);
  debug_diff_rx_pointers(state, *mdra_rd_now);
  return (state == 0) ? 0 : wrpadiff(*mdra_rd_org, *mdra_rd_now);
}
#endif

int dm_eth_polling_downup(void)
{
	static int link_stat = 0;
	uint8_t statdat[6];

	DM_Eth_Read_Info(statdat);
	#if 1
	operate_led3(VIA_NET, DM_Eth_Info_Linkup(statdat) ? LED_FLASH : LED_OFF);
	#endif
	if (DM_Eth_Info_Linkup(statdat) && !link_stat) {
		link_stat = 1;
		printf("(down to link up)\r\n");
		return 1;
	} else if (!DM_Eth_Info_Linkup(statdat) && link_stat) {
		link_stat = 0;
		printf("(up2down to link down)\r\n");
		return 0;
	}
	return 0;
}

void dm_eth_show_app_help_info(char *drv_modeS, char *statusS)
{
	printkey("\r\n\r\n\r\n[%s mode] /UIP_project /R2412 [uip_dm9051_r2412] %s %s\r\n", drv_modeS, statusS, __DATE__);
}

void dm_eth_show_app_help_info_w(char *drv_modeS, char *root_dirS, char *prjS, char *locStr)
{
	printkey("\r\n[%s mode] /%s /%s [%s] %s\r\n\r\n", drv_modeS, locStr, root_dirS, prjS, __DATE__);
}

void dm_eth_show_app_help_info_ptp(char *op_modeStr, char *statStr, char *clkTypeStr)
{
	printkey("\r\n\r\n\r\n[%s mode][%s][%s] /at32f437_ptp_daemon_client /R2412 [v51][emac] %s\r\n", 
		op_modeStr, statStr, clkTypeStr, __DATE__);
}

#if 1
#if defined(_DLW_AT32F437xx)

/*-----------------------------------------------------------------------------
 * GPIO usage example
 *-----------------------------------------------------------------------------*/

/*-----------------------------------------------------------------------------
 * GPIO Pin Configurations and Control Functions
 *
 * Example Implementations
 * The file includes two concrete GPIO implementations:
 * LED3 Pin
 *  .Output pin (PD14)
 *  .No pull-up/down
 *  .Functions: config_led3(), led3_lo(), led3_hi()
 * BUTTON Pin
 *  .Input pin (PA0)
 *  .Pull-down enabled
 *  .Functions: config_button(), button_is_pressed()
 * DIAG Pin
 *  .Output pin (PA15)
 *  .No pull-up/down
 *  .Functions: config_diag(), diag_lo(), diag_hi()
 * INPT Pin
 *  .Input pin (PC7)
 *  .Pull-up enabled
 *  .Functions: config_inpt(), inpt_get()
 *-----------------------------------------------------------------------------*/
/**
 * @brief GPIO configuration structures for commonly used pins
 */
struct gpio_config_t
	led3 = {
		GPIOD,		  // LED3_GPIO
		GPIO_PINS_14, // LED3_PIN
		CRM_GPIOD_PERIPH_CLOCK, // LED3_GPIO_CRM_CLK
		{
			GPIO_PULL_NONE,
			GPIO_MODE_OUTPUT,
		}
},
	button = {
		GPIOA,		 // USER_BUTTON_PORT
		GPIO_PINS_0, // USER_BUTTON_PIN
		CRM_GPIOA_PERIPH_CLOCK, // USER_BUTTON_CRM_CLK
		{
			GPIO_PULL_DOWN,
			GPIO_MODE_INPUT,
		}
},
	diag = {
		GPIOA,
		GPIO_PINS_15,
		CRM_GPIOA_PERIPH_CLOCK,
		{
			GPIO_PULL_NONE,
			GPIO_MODE_OUTPUT,
		}
},
	inpt = {
		GPIOC,
		GPIO_PINS_7,
		CRM_GPIOC_PERIPH_CLOCK,
		{
			GPIO_PULL_UP,
			GPIO_MODE_INPUT,
		}
};

/**
 * @brief  LED3 control functions
 */
void config_led3(void)
{
	dm9051if_stdpin_config(&led3);
}

void led3_toggle(void)
{
	led3.port->odt ^= led3.pin; //gpio_stdpin_toggle
}

void led3_on(void)
{
	dm9051if_stdpin_lo(&led3); //gpio_stdpin_lo
}

void led3_off(void)
{
	dm9051if_stdpin_hi(&led3); //gpio_stdpin_hi
}

/**
 * @brief  User button control functions
 */
void config_button(void)
{
	dm9051if_stdpin_config(&button);
}

button_type button_is_pressed(void)
{
	return (gpio_stdpin_get(&button) == SET) ? USER_BUTTON : NO_BUTTON;
}

/**
 * @brief  Periodically to Demo toggle led3
 */
#define NMS 250

struct led_control_t {
    uint32_t start_time;
    uint32_t interval;
    confirm_state is_active; //bool is_active;
    //sled_ops_state state;
};

static struct led_control_t led_control[2] = {0};

void led_start_alloc(trigger_type trigger, uint32_t now)
{
	struct led_control_t *ctrl = &led_control[trigger];

	ctrl->start_time = now;

	trigger++;
	if (trigger > VIA_NET)
		trigger = VIA_BUTTON;
	ctrl = &led_control[trigger];
	ctrl->start_time = now + (NMS >> 1);
}

void operate_led3(trigger_type trigger, led_ops_state ops)
{
	//static uint32_t statime[2] = {0, 0};
	//static uint32_t intvltime[2];
	struct led_control_t *ctrl = &led_control[trigger];
	uint32_t elapsed;

	if (ops == LED_FLASH)
	{
		uint32_t now = dm9051_boards_heartbeat_now();
		if (!ctrl->is_active) //(!statime[trigger])
		{
			led_start_alloc(trigger, now); //ctrl->start_time = now; //statime[trigger] = now;
			ctrl->interval = NMS; //intvltime[trigger] = NMS;
			ctrl->is_active = TRUE;
			led3_toggle(); //led3_on
			return;
		}

		elapsed = now - ctrl->start_time; //statime[trigger];
		if (elapsed >= ctrl->interval) //intvltime[trigger]
		{
			if (ctrl->interval == NMS) //intvltime[trigger]
				led3_toggle(); //led3_off
			if (ctrl->interval == (NMS * 2))
				ctrl->is_active = FALSE; //statime[trigger] = 0;
			ctrl->interval += NMS;
		}
	}
	else
	{
		/* for only turn led off once by per operated trigger source!
		 */
		if (ctrl->is_active) { //statime[trigger]
			led3_off();
			ctrl->is_active = FALSE; //statime[trigger] = 0;
		}
	}
}

/**
 * @brief  Diagnostic pin control functions
 */
void config_diag(void)
{
	dm9051if_stdpin_config(&diag);
}

void diag_lo(void)
{
	dm9051if_stdpin_lo(&diag); //gpio_stdpin_lo
}

void diag_hi(void)
{
	dm9051if_stdpin_hi(&diag); //gpio_stdpin_hi
}

/**
 * @brief  Input pin control functions
 */
void config_inpt(void)
{
	dm9051if_stdpin_config(&inpt);
}

flag_status inpt_get(void)
{
	return gpio_stdpin_get(&inpt);
}

#endif /* _DLW_AT32F437xx */

/**
 * @brief  Init to Demo button control led3
 */
void dm_eth_polling_button_init(enum operate_tag tag)
{
	config_button();
	if (tag == OPS_LED3)
		config_led3();
}

/**
 * @brief  Periodically to Demo button control led3
 */
void dm_eth_polling_button_ops(enum operate_tag tag)
{
	static int button_stat = 0;

	if (button_is_pressed() == USER_BUTTON) {
		if (!button_stat) {
			dm_eth_show_app_help_info(RX_MODE_STR, "polling_button_pressed");
			button_stat = 1;
		}
		if (tag == OPS_LED3)
			operate_led3(VIA_BUTTON, LED_FLASH);
	} else {
		if (button_stat) { //release button
			dm_eth_show_identified_ip("display config ip");
			dm_eth_show_identified_gw("display config gw");
			button_stat = 0;
		}
		if (tag == OPS_LED3)
			operate_led3(VIA_BUTTON, LED_OFF);
	}
}
#endif
