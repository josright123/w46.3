/**
 **************************************************************************
 * @file     main.c
 * @version  v2.0.0
 * @date     2020-11-02
 * @brief    main program
 **************************************************************************
 *                       Copyright notice & Disclaimer
 *
 * The software Board Support Package (BSP) that is made available to
 * download from Artery official website is the copyrighted work of Artery.
 * Artery authorizes customers to use, copy, and distribute the BSP
 * software and its related documentation for the purpose of design and
 * development in conjunction with Artery microcontrollers. Use of the
 * software is governed by this copyright notice and the following disclaimer.
 *
 * THIS SOFTWARE IS PROVIDED ON "AS IS" BASIS WITHOUT WARRANTIES,
 * GUARANTEES OR REPRESENTATIONS OF ANY KIND. ARTERY EXPRESSLY DISCLAIMS,
 * TO THE FULLEST EXTENT PERMITTED BY LAW, ALL EXPRESS, IMPLIED OR
 * STATUTORY OR OTHER WARRANTIES, GUARANTEES OR REPRESENTATIONS,
 * INCLUDING BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE, OR NON-INFRINGEMENT.
 *
 **************************************************************************
 */

#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"
#include "at32_emac_eli.h"
//#include "lwip_ethif/nosys/ethif.h"
#include "netconf.h"
#include "ptpd.h"
#include "../../dm9051_u2510_if/if.h" //"all, as main.c including"

/* lwIP iPerf 應用程式 */
#if LWIPERF_APP
#include "iperf1.h"
//#include "lwip/apps/lwiperf.h"
#endif

/* MQTT 應用程式 */
#if LWIP_MQTT
#include "developer_conf.h"
#endif

/** @addtogroup AT32F407_periph_examples
 * @{
 */

//#ifdef LWIP_PROVIDE_ERRNO
///* Define errno variable */
//int errno = 0;
//#endif

/** @addtogroup 407_EMAC_telnet EMAC_telnet
 * @{
 */
volatile uint32_t local_time = 0;
char app_heads[62];
int hn = 0;

/**
 * @brief ptpClock's .portDS.versionNumber, .defaultDS.domainNumber
 */

const char *app0_head[] = {
	"[/LWIP_PTP 0]",
	"[/LWIP_PTP 1]",
};

const char *app1_head[] = {
	"[/LWIP_MQTT 0]",
	"[/LWIP_MQTT 1]",
};

const char *app3_head[] = {
	"[/IPERF3 APP 0]",
	"[/IPERF3 APP 1]",
};

const char *APP_head0(void)
{
#if LWIP_PTP
	return app0_head[1];
#else
	return app0_head[0];
#endif
}

const char *APP_head1(void)
{
#if LWIP_MQTT
	return app1_head[1];
#else
	return app1_head[0];
#endif
}

const char *APP_head3(void)
{
#if LWIPERF_APP
	return app3_head[1];
#else
	return app3_head[0];
#endif
}

/**
 * @brief State machine states for PTPD combined periodic handler
 */
typedef enum
{
    DM_ENUM_COMBINED_WAIT_NET,   /**< Waiting for network to be ready */
    DM_ENUM_COMBINED_START_PTPD, /**< Start PTPD process */
    DM_ENUM_COMBINED_START_MQTT,
    DM_ENUM_COMBINED_PERIODIC,    /**< PTPD process started, run periodic */
    //DM_ENUM_COMBINED_WAIT2_NET,   /**< Waiting for network to be ready */
} DmPtpdCombinedState_t;

static DmPtpdCombinedState_t state = DM_ENUM_COMBINED_WAIT_NET;

int dm_ptpd_mqtt_periodic_stat(void)
{
	return state == DM_ENUM_COMBINED_PERIODIC ? 1 : 0;
}

void dm_wait2_periodic_stat(void)
{
	state = DM_ENUM_COMBINED_WAIT_NET; //DM_ENUM_COMBINED_WAIT2_NET;
}

/**
 * @brief Combined state machine and periodic handler for PTPD
 * @note Call this function periodically in the main loop
 * @param localtime Current system time
 * @retval none
 */
void dm_ptpd_combined_periodic(uint32_t localtime)
{
	static uint8_t done_ptpd_connected = 0;
	static uint8_t done_mqtt_connected = 0;

    switch (state)
    {
        case DM_ENUM_COMBINED_WAIT_NET:
			if(net_ip_bound()) {
                state = DM_ENUM_COMBINED_START_PTPD;
				if (done_ptpd_connected)
					state = DM_ENUM_COMBINED_START_MQTT;
				if (done_mqtt_connected)
					state = DM_ENUM_COMBINED_PERIODIC;
			}
            break;
        case DM_ENUM_COMBINED_START_PTPD:
			network_ptp_init();
			done_ptpd_connected = 1;
			state = DM_ENUM_COMBINED_START_MQTT;
            break;
        case DM_ENUM_COMBINED_START_MQTT:
#if LWIP_MQTT
			mqtt_connecting_process();
#endif
			done_mqtt_connected = 1;
            state = DM_ENUM_COMBINED_PERIODIC;
            break;
        case DM_ENUM_COMBINED_PERIODIC:
#if LWIP_PTP
            ptpd_Periodic_Handle(localtime);
#endif
            break;
//		case DM_ENUM_COMBINED_WAIT2_NET:
//            if (net_ip_done())
//            {
//				dhcpc_configured();
//                state = DM_ENUM_COMBINED_PERIODIC;
//            }
//            break;
        default:
            // Never
            state = DM_ENUM_COMBINED_WAIT_NET;
            break;
    }
}

/**
 * @brief  main function.
 * @param  none
 * @retval none
 */
int main(void)
{
  dm9051_config_app_mcu_name(PTPD_HEADER_MCU_DESC);
  system_clock_config();

  //  at32_board_init();
  ctick_delay_ms(25);
  ctick_delay_ms(125);
  uart_print_init(115200);

  printf("\r\n");
  printf("[at32%s]\r\n", dm_eth_app_mcu_name(NULL));
  dm_eth_show_app_help_info("LWIP_project", "ptpd+mqtt_client+iperf3"); //printkey("\r\n\r\n\r\n/ZYK_project /R2410 [uip_dm9051_r2410] %s\r\n", __DATE__);

  hn += sprintf(app_heads+hn, "%s", APP_head0());
  hn += sprintf(app_heads+hn, "%s", APP_head1());
  hn += sprintf(app_heads+hn, "%s", APP_head3());
  dm_eth_show_app_modes_info(DHCP_head(), DEBUG_head(), app_heads);
	
#if 0 //_LWIP_PTP [rtOpts NOT init yet!]
  extern RunTimeOpts rtOpts;
  dm_eth_show_app_help_info_ptp(
		rtOpts.slaveOnly ? "SLAVE" : "MASTER",
		DEFAULT_TWO_STEP_FLAG ? "Two-step" : "One-step",
		BOUNDARY_CLOCK ?
			"BOUNDARY" : //"BOUNDARY_CLOCK"
			SyncMechMode[ptpClock.portDS.delayMechanism], //"~BOUNDARY_CLOCK",
		PTPD_HEADER_MCU_DESC
	); //modify 2024-11-20
#endif

  printf("[DM9051_ptp_daemon]\r\n");
  printf("[delay_ms] 25\r\n");
  system_clock_print();

  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);

  //  delay_init();
  // tmr_configuration();
	
  system_timer_init();
  // emac_tmr_init();  /* Comment out TMR6 initialization */
  // systick_init();     /* Use SysTick instead of TMR6 */

//  ethif_register_init(); //dm905ini.eth_register_init();
  //  dm_eth_init();= DM_ETH_Init_W(NULL, NULL);

  tcpip_stack_init();

  // iperf_init();
  //  lwiperf_start_tcp_client_default(const ip_addr_t* remote_addr,
  //                               lwiperf_report_fn report_fn, void* report_arg);
  // lwiperf_start_tcp_server_default(lwiperf_report_fn report_fn, void* report_arg);
  //lwiperf_start_tcp_server_default(iperf_report_callback, NULL);
#if LWIPERF_APP
  //iperf_init();
  
  //iperf_tcp_init();
	iperf3_tcp_init();
	iperf2_tcp_init();
	
  //iperf_udp_init();
	iperf2_udp_init();
#endif

  //_PTPd_Init();

#if LWIP_MQTT
  mqtt_client_process_init();
#endif

  for (;;)
  {
    dm_eth_receive();
    dm_ptpd_combined_periodic(local_time);

	net_ip_done();
	lwip_periodic_handle(local_time);
	lwip_periodic_link_hdlr(local_time);

#if LWIP_MQTT
	/* Network services management - manage mqtt client */
	ethernetif_services_manager();
	dm_mqtt_periodic(local_time);
#endif
	iperf2_udp_last_time_manager();
  }
}

/**
 * @}
 */

/**
 * @}
 */
