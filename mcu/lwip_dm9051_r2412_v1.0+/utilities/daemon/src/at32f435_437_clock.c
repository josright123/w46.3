/**
  **************************************************************************
  * @file     at32f435_437_clock.c
  * @version  v2.0.6
  * @date     2022-03-11
  * @brief    system clock config program
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
#include "at32f435_437_clock.h"

/**
  * @brief  system clock config program
  * @note   the system clock is configured as follow:
  *         - system clock        = (hext * pll_ns)/(pll_ms * pll_fr)
  *         - system clock source = pll (hext)
  *         - hext                = 8000000
  *         - sclk                = 250000000
  *         - ahbdiv              = 1
  *         - ahbclk              = 250000000
  *         - apb2div             = 2
  *         - apb2clk             = 125000000
  *         - apb1div             = 2
  *         - apb1clk             = 125000000
  *         - pll_ns              = 125
  *         - pll_ms              = 1
  *         - pll_fr              = 4
  * @param  none
  * @retval none
  */
void system_clock_config(void)
{
  /* enable pwc periph clock */
  crm_periph_clock_enable(CRM_PWC_PERIPH_CLOCK, TRUE);

  /* config ldo voltage */
  pwc_ldo_output_voltage_set(PWC_LDO_OUTPUT_1V3);
 
  /* set the flash clock divider */
  flash_clock_divider_set(FLASH_CLOCK_DIV_3);
 
  /* reset crm */
  crm_reset();

  crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);

  /* wait till hext is ready */
  while(crm_hext_stable_wait() == ERROR)
  {
  }

  /* config pll clock resource 
  common frequency config list: pll source selected  hick or hext(8mhz)
  _______________________________________________________________________________________
  |        |         |         |         |         |         |         |        |        |
  |pll(mhz)|   288   |   252   |   216   |   180   |   144   |   108   |   72   |   36   |
  |________|_________|_________|_________|_________|_________|_________|_________________| 
  |        |         |         |         |         |         |         |        |        |
  |pll_ns  |   72    |   63    |   108   |   90    |   72    |   108   |   72   |   72   |
  |        |         |         |         |         |         |         |        |        |
  |pll_ms  |   1     |   1     |   1     |   1     |   1     |   1     |   1    |   1    |
  |        |         |         |         |         |         |         |        |        |
  |pll_fr  |   FR_2  |   FR_2  |   FR_4  |   FR_4  |   FR_4  |   FR_8  |   FR_8 |   FR_16|
  |________|_________|_________|_________|_________|_________|_________|________|________|
 
  if pll clock source selects hext with other frequency values, or configure pll to other
  frequency values, please use the at32 new clock  configuration tool for configuration.  */
  crm_pll_config(CRM_PLL_SOURCE_HEXT, 125, 1, CRM_PLL_FR_4);

  /* enable pll */
  crm_clock_source_enable(CRM_CLOCK_SOURCE_PLL, TRUE);

  /* wait till pll is ready */
  while(crm_flag_get(CRM_PLL_STABLE_FLAG) != SET)
  {
  }

  /* config ahbclk */
  crm_ahb_div_set(CRM_AHB_DIV_1);

  /* config apb2clk */
  crm_apb2_div_set(CRM_APB2_DIV_2);

  /* config apb1clk */
  crm_apb1_div_set(CRM_APB1_DIV_2);

  /* enable auto step mode */
  crm_auto_step_mode_enable(TRUE);

  /* select pll as system clock source */
  crm_sysclk_switch(CRM_SCLK_PLL);

  /* wait till pll is used as system clock source */
  while(crm_sysclk_switch_status_get() != CRM_SCLK_PLL)
  {
  }

  /* disable auto step mode */
  crm_auto_step_mode_enable(FALSE);

  /* update system_core_clock global variable */
  system_core_clock_update();
}

/**
 * @brief  initialize systick timer for system timing with interrupts
 * @note   configures systick to generate interrupts every 10ms (100Hz)
 * @param  none
 * @retval 0 if successful, 1 if failed
 */
uint32_t system_timer_init(void)
{
//  extern unsigned int system_core_clock;
  
  /* SYSTEMTICK_PERIOD_MS is 10ms, so frequency is 100Hz */
  /* Calculate ticks: system_core_clock / (1000 / SYSTEMTICK_PERIOD_MS) */
  /* For 10ms: ticks = system_core_clock / 100 */
  uint32_t ticks = (uint32_t)(system_core_clock / 100);
  
  /* Configure SysTick to generate interrupts */
  /* SysTick_Config sets up: reload value, interrupt priority, and enables timer+interrupt */
  if (SysTick_Config(ticks) != 0)
  {
    return 1; /* Configuration failed */
  }
  
  return 0; /* Configuration successful */
}

/**
 * @brief  initialize tmr6 for emac
 * @note   wait usage
 * @retval none
 */
//#include <stdio.h>
//void emac_tmr_init(void)
//{
//    crm_clocks_freq_type crm_clocks_freq_struct = {0};

//    printf("Using TMR6 for system timing\r\n");

//    crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);

//    crm_clocks_freq_get(&crm_clocks_freq_struct);
//    tmr_base_init(TMR6, 99, (crm_clocks_freq_struct.ahb_freq / 10000) - 1);
//    tmr_cnt_dir_set(TMR6, TMR_COUNT_UP);

//    /* overflow interrupt enable */
//    tmr_interrupt_enable(TMR6, TMR_OVF_INT, TRUE);

//    /* tmr1 overflow interrupt nvic init */
//    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
//    nvic_irq_enable(TMR6_GLOBAL_IRQn, 0, 0);
//    tmr_counter_enable(TMR6, TRUE);

//	uint32_t ticks;
//    printf("\r\n");
//    printf("[AT32F403a]\r\n");
//	printf("tmr_base_init: tmr6 \r\n");
//	ticks = (crm_clocks_freq_struct.ahb_freq / 10000) - 1;
//    printf("tmr6: %d Hz, %d ticks\r\n", 
//           (crm_clocks_freq_struct.ahb_freq / (ticks + 1)) / 100, 
//           (ticks)/100);
//}
