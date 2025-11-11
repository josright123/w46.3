/**
  **************************************************************************
  * @file     dm_eth.c
  * @file     dm_eth_main.c
  * @version  v1.0.0
  * @date     2024-06-24
  * @brief    as eth.c
  **************************************************************************
  */
#include "cmsis_compiler.h"               /* CMSIS compiler specific defines */
#include "ethernetif_v51.h"
#include "ethernetif.h"
#include "netconf.h" //#include "lwip/netif.h" //#include "lwip/err.h"

#include "control/conf.h"

#include "control/drv_control/conf_core.h"

#include "lwip_ethif/nosys/dm_rt_api.h"

#include "control/drv_control/dm9051_drv_debug.h"
//#include "sys/rtos_opts.h"
//#include "sys/sys_control/conf_rtos.h"
//#include "sys/sys_control/dm9051_plat_debug.h"
#include "rtos_opts.h"

//(#include "rtos/rtos_rtos/dm9051_rtos.h")

/* eth api */
#include "platform_info/nosys/uip_eth/eth_types.h"
#include "platform_info/nosys/uip_eth/eth_api.h"
#include "lwip_ethif/nosys/plat_eth_api.h"

#include "dm_rt_api.h"

#if freeRTOS
void DM_Eth_SemaphoreNewbinary_Log(void); //dm_eth_mutex.h
//void DM_ETHER_Receive_Task_supp_poll(void); //dm_eth_mutex.h
void DM_ETHER_Receive_Task(void* param); //dm_eth_task_recv.h

TaskHandle_t network_rx_handler;
#endif

uint16_t isrSemaphore_src;
int isrSemaphore_n = 0; //extern int flgSemaphore_n;
#if (rt_print | drv_print)
static int fifoTurn_n = 0; //.....
#endif

#include "rt/dm_rt_intr_supp.h" //#include "dm_eth_intr_supp.h"
#include "rt/dm_rt_recv_supp_intr.h"
#include "rt/dm_rt_recv_supp_poll.h"

//#undef printf
//#undef printk
//#define printf(fmt, ...) TASK_DM9051_DEBUGF(PRINT_INFO_RT, (fmt, ##__VA_ARGS__)) //PRINT_RT or "[RT] "
//#define	printk(fmt, ...) TASK_DM9051_DEBUGK((fmt, ##__VA_ARGS__))


static void diff_rx_pointers_s(uint16_t *pMdra_rds) {
#if DM_ETH_DEBUG_MODE
	//uint16_t rwpa_wts /*_mdra_rds*/;
	DM_ETH_Diff_rx_pointers_W(0, /*&rwpa_wts*/ pMdra_rds, pMdra_rds);
#endif
}

static void diff_rx_pointers_e(uint16_t *pMdra_rds, int n) {
#if DM_ETH_DEBUG_MODE
#if (rt_print | drv_print)
	static uint16_t premdra_rd = 0x4000;
	uint16_t mdra_rd;

	if (n >= 0) { /*3*/
		#if rt_print
		//uint16_t rwpa_wt;
		uint16_t diff =
		#endif
		DM_ETH_Diff_rx_pointers_W(1, /*&rwpa_wt*/pMdra_rds, &mdra_rd);
	//#if rt_print
	//	diff = DM_ETH_Diff_rx_pointers_W(1, /*&rwpa_wt*/pMdra_rds, &mdra_rd);
	//	diff = wrpadiff(*pMdra_rds, mdra_rd);
	//#elif drv_print
	//	DM_ETH_ToCalc_rx_pointers_W(1, pMdra_rds, &mdra_rd); //Extra-on-20241021
	//#endif

		fifoTurn_n += n;
		if (mdra_rd < premdra_rd && (premdra_rd != 0x4000))
		{
			#if rt_print
			uint16_t compos_totaldiff = (mdra_rd >= *pMdra_rds) ? 0x3400 : 0;
			printf("(INT %lu) mdra s %02x%02x e %02x%02x dif %x (nrx %d) .rt\r\n",
				get_interrupt_count(),
				*pMdra_rds >> 8, *pMdra_rds & 0xff,
				mdra_rd >> 8, mdra_rd & 0xff,
				diff + compos_totaldiff,
				fifoTurn_n);
			#endif
			fifoTurn_n = 0;
		}
		premdra_rd = mdra_rd;
	}
#endif
#endif
}

#if freeRTOS
void DM_Eth_CreateRecvTask_R(void)
{
//	void DM_ETHER_Receive_Task_supp_poll(void);

//	DM_Eth_SemaphoreNewbinary();
	DM_Eth_SemaphoreNewbinary_Log();
	
	printf("Create.S DM_ETHER_Receive_Task\r\n");
	printf("Receive_Task.S delay %u\r\n", 0);
//	DM_ETHER_Receive_Task_supp_poll();

	if(xTaskCreate((TaskFunction_t )DM_ETHER_Receive_Task,     
				 (const char*    )"Rx_task",   
				 (uint16_t       )512+256, //is to stacksize larger.
				 (void*          )tcpip_stack_netif(), //netif, //NULL,
				 (UBaseType_t    )NET_TASK_PRIO, //2
				 (TaskHandle_t*  )&network_rx_handler) != pdPASS) {
		printf("Rx task not be created. Error!\r\n");
	}
	printf("Create.E DM_ETHER_Receive_Task\r\n");
	printk("\r\n");
}
#endif

int DM_ETHER_Receive_Task_Loop_W(void) {
		int sema_event = 0;
	
#ifdef ETHERNET_INTERRUPT_MODE
		sema_event = DM_ETH_GetInterruptEvent();
		if (sema_event) {
#else
		//if (flgSemaphore_r == 1) {flgSemaphore_r = 0;}
#endif			
			#if (rt_print | drv_print)
			static uint16_t gmdra_rds;
			#endif

			//uint16_t isrs = dm9051_isr_disab();

			isrSemaphore_src = 0x5555 >> 8;
		
			isrSemaphore_n = 0;	

			do {
				struct pbuf *p;
				while(1) {
					#if (rt_print | drv_print)
					if (!fifoTurn_n)
						diff_rx_pointers_s(&gmdra_rds);
					#endif
					//p = input_intr(); //= DM_ETH_Input_W(); //= low_level_input(netif);
					p = DM_ETH_Input_W();

					if (p && process_input_pbuf(p)) {
						isrSemaphore_n++;
					
						#if (rt_print | drv_print)
						diff_rx_pointers_e(&gmdra_rds, 1); //fifoTurn_n++; insided.
						#endif
						continue;
					}
					break;
				}
			} while(0);			
				
			dm9051_isr_enab(); //DM_ETH_ToRst_ISR();
			//=
			//cspi_isr_enab(); //DM_ETH_IRQEnable(); //dm9051_isr_enab();

#ifdef ETHERNET_INTERRUPT_MODE
		}
#endif

		//if (flgSemaphore_r == 2) flgSemaphore_r = 0; //for next to direct no-limited change-in
		#if 0
		if (sema_event == 2)
		{
			void net_inp_poll(void);
			
			net_inp_poll();
		}
		#endif
		
		return sema_event;
}

#if freeRTOS
void DM_ETHER_Receive_Task(void* param) {
	(void) param; //struct netif *netif = param;

#if 1
    dm_eth_polling_button_init(OPS_LED3);
	      //button_toggle_led3_init();
#endif
	while(1) {
		DM_ETHER_Receive_Task_Loop_W();
		dm_eth_polling_downup_w();
	#if 1
        dm_eth_polling_button_ops(OPS_LED3);
		      //button_toggle_led3();
	#endif
		vTaskDelay(0); //vTaskDelay(pdMS_TO_TICKS(1));
	}
}
#endif
