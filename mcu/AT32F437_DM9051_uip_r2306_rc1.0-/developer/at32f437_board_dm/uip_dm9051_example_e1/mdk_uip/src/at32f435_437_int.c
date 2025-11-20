/**
  **************************************************************************
  * @file     at32f435_437_int.c
  * @version  v2.0.0
  * @date     2020-11-02
  * @brief    main interrupt service routines.
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

/* includes ------------------------------------------------------------------*/
#include "at32f435_437_int.h"
#include "uip.h"
#include "netconf.h"
//#include "ethernetif.h"
#if 0
#include "platform_ethif/nosys/platform_api.h" //#include "core/dm9051.h"
#endif
//#include "core/hal_int_export.h"
//void EXINT9_5_UserFunction(void);
//uint32_t HAL_IRQLine(void);
//void EXINT9_5_RT_UserFunction(void);
//uint32_t HAL_IRQLine(void);

/** @addtogroup AT32F437_periph_examples
  * @{
  */

/** @addtogroup 437_EMAC_telnet
  * @{
  */

/**
  * @brief  this function handles nmi exception.
  * @param  none
  * @retval none
  */
void NMI_Handler(void)
{
}

/**
  * @brief  this function handles hard fault exception.
  * @param  none
  * @retval none
  */
void HardFault_Handler(void)
{
  /* go to infinite loop when hard fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles memory manage exception.
  * @param  none
  * @retval none
  */
void MemManage_Handler(void)
{
  /* go to infinite loop when memory manage exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles bus fault exception.
  * @param  none
  * @retval none
  */
void BusFault_Handler(void)
{
  /* go to infinite loop when bus fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles usage fault exception.
  * @param  none
  * @retval none
  */
void UsageFault_Handler(void)
{
  /* go to infinite loop when usage fault exception occurs */
  while(1)
  {
  }
}

/**
  * @brief  this function handles svcall exception.
  * @param  none
  * @retval none
  */
void SVC_Handler(void)
{
}

/**
  * @brief  this function handles debug monitor exception.
  * @param  none
  * @retval none
  */
void DebugMon_Handler(void)
{
}

/**
  * @brief  this function handles pendsv_handler exception.
  * @param  none
  * @retval none
  */
void PendSV_Handler(void)
{
}

/**
  * @brief  this function handles systick handler.
  * @param  none
  * @retval none
  */
//void main_tick_handler(void);

void SysTick_Handler(void)
{
	//main_tick_handler();
}

#if 1
extern u32 lwip_sys_now; //jj-temp-extern

//:TMR5_GLOBAL_IRQHandler
//~TMR6_GLOBAL_IRQHandler
//
// Did it we know this timer is 10 ms !
//
//or
// static int co = 0;
// co++;
// if (!(co % 100)) printf("count (co = %d) - TMR5 control once line\r\n", co);
  
// if (co >= 1000) {
//	printf("10 seconds (co = %d) - TMR5 control once line\r\n", co);
//	co = 0;
// }
void TMR5_GLOBAL_IRQHandler(void) //Accept TMR5, NOT startup_at32f415.s ; TMR6 (Porting by JJ, as 'TMR6')
{
  if(tmr_flag_get(TMR5, TMR_OVF_FLAG) != RESET) //TMR6
  {
	
	/* used by clock_time(), uip */
	lwip_sys_now++; //jj-temp-extern (to compare to '_all_local_time' in "netconf.c")
	/* used by link_handle(), main_loop's tapdev_loop() --> _periodic_handle() --> link_handle(500) */
    time_update(); /* Update the _all_local_time by adding 'SYSTEMTICK_PERIOD_MS10' each SysTick interrupt */
    tmr_flag_clear(TMR5, TMR_OVF_FLAG); //TMR6
  }
}
#endif

#if 0
	#if 0
	void emac_tmr_init(void)
	{
	  crm_clocks_freq_type crm_clocks_freq_struct = {0};
	  crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);

	  crm_clocks_freq_get(&crm_clocks_freq_struct);
	  /* tmr1 configuration */
	  /* time base configuration */
	  /* systemclock/24000/100 = 100hz */
	  tmr_base_init(TMR6, 99, (crm_clocks_freq_struct.ahb_freq / 10000) - 1);
	  tmr_cnt_dir_set(TMR6, TMR_COUNT_UP);

	  /* overflow interrupt enable */
	  tmr_interrupt_enable(TMR6, TMR_OVF_INT, TRUE);

	  /* tmr1 overflow interrupt nvic init */
	  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
	  nvic_irq_enable(TMR6_DAC_GLOBAL_IRQn, 0, 0);
	  tmr_counter_enable(TMR6, TRUE);
		//printf("TMR6 init ..\r\n");
		
	//    printf("\r\n");
	//    printf("[AT32F437]\r\n");
		printf("[CLK]\r\n");
		printf(" %d(sclk_freq) %d(ahb)\r\n %d(apb2_freq) %d(apb1_freq)\r\n",
			crm_clocks_freq_struct.sclk_freq,
			crm_clocks_freq_struct.ahb_freq,
			crm_clocks_freq_struct.apb2_freq,
			crm_clocks_freq_struct.apb1_freq);
	}
	#endif
/**
  * @brief  this function handles timer6 overflow handler.
  * @param  none
  * @retval none
  */
void TMR6_DAC_GLOBAL_IRQHandler(void)
{
  if(tmr_flag_get(TMR6, TMR_OVF_FLAG) != RESET)
  {
    /* Update the local_time by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
    _time_update();
    tmr_flag_clear(TMR6, TMR_OVF_FLAG);
  }
}
#endif

//extern char emac_pps_ready;
//void emac_pps_lo(int tcount);
//void emac_pps_hi(int tcount);

//void EMAC_IRQHandler(void)
//{
//  struct ptptime_t next_trigger;
//  __IO uint32_t ts_status = 0;
//  if(emac_interrupt_status_read(EMAC_TST_FLAG) == SET) 
//  {
//    ts_status = EMAC_PTP->tssr;
//    /* set next trigger time */
//    /* this should not be in the past, because it will trigger imidietly */
//    emac_ptptime_gettime(&next_trigger);    
//  	/* Toggle LED */
//  	
//	if (emac_pps_ready) {
//		static char tten = 2; //5; //12;
//		static char tdir = 1;
//		static int tcount = 2; //0;
//		//tcount++;
//		if (tdir) tcount++;
//		else tcount--;
//		if ((tcount < -5) || (tcount > 5)) 
//			tdir = 1 - tdir;
//		if(next_trigger.tv_sec % 2 == 0) {
//			emac_pps_lo(tcount);
////			if (tten) { printf("%2d%3u pptsec %lu, emac trigger Lo\r\n", tcount, tten, next_trigger.tv_sec % 1000); }
//		} else {
//			emac_pps_hi(tcount);
////			if (tten) { printf("%2d%3u pptsec %lu, emac trigger Hi\r\n", tcount, tten, next_trigger.tv_sec % 1000); }
//			if (tten) tten--;
//		}
//	}

//    next_trigger.tv_sec += 1;   	
//    emac_ptp_target_second_set(next_trigger.tv_sec);
//    emac_ptp_target_nanosecond_set(0);
//    
//    /* enable next trigger */
//    emac_ptp_interrupt_trigger_enable(TRUE);
//  }  
//}

//void v51_ptptime_gettime(struct ptptime_t *timestamp);
//struct ptptime_t g_timestamp;

//void EXINT9_5_IRQHandler(void) {
//	if (exint_flag_get(HAL_IRQLine()) != RESET)
//	{
//		cdata_enqueue_ievent(); //EXINT9_5_HAL_IRQHdlr();
//		exint_flag_clear(HAL_IRQLine());
//	}
//}

//void EXINT9_5_IRQHandler(void) {
//	if (exint_flag_get(HAL_IRQLine()) != RESET)
//	{
//			EXINT9_5_UserFunction();
//			//EXINT9_5_RT_UserFunction();
//			exint_flag_clear(HAL_IRQLine());
//	}
//}
/**
  * @}
  */

/**
  * @}
  */
