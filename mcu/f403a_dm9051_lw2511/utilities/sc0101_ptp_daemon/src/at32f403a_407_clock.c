/**
  **************************************************************************
  * @file     at32f403a_407_clock.c
  * @version  v2.0.0
  * @date     2020-11-02
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
#include "at32f403a_407_board.h"
#include "at32f403a_407_clock.h"
//void time_update(void); //#include "netconf.h"

/**
  * @brief  system clock config program
  * @note   the system clock is configured as follow:
  *         - system clock        = hext / 2 * pll_mult
  *         - system clock source = pll (hext)
  *         - hext                = 8000000
  *         - sclk                = 200000000
  *         - ahbdiv              = 1
  *         - ahbclk              = 200000000
  *         - apb2div             = 2
  *         - apb2clk             = 100000000
  *         - apb1div             = 2
  *         - apb1clk             = 100000000
  *         - pll_mult            = 50
  *         - pll_range           = GT72MHZ (greater than 72 mhz)
  * @param  none
  * @retval none
  */
void system_clock_config(void)
{
  /* reset crm */
  crm_reset();

  crm_clock_source_enable(CRM_CLOCK_SOURCE_HEXT, TRUE);

   /* wait till hext is ready */
  while(crm_hext_stable_wait() == ERROR)
  {
  }

  /* config pll clock resource */
  crm_pll_config(CRM_PLL_SOURCE_HEXT_DIV, /*CRM_PLL_MULT_50*/ /*CRM_PLL_MULT_36*/
			/*CRM_PLL_MULT_42*/ /*CRM_PLL_MULT_46*/ /*CRM_PLL_MULT_52*/ /*CRM_PLL_MULT_60*/ /*CRM_PLL_MULT_64*/
			//CRM_PLL_MULT_36,
			CRM_PLL_MULT_48,
			//CRM_PLL_MULT_52,
			CRM_PLL_OUTPUT_RANGE_GT72MHZ);

  /* config hext division */
  crm_hext_clock_div_set(CRM_HEXT_DIV_2);

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

void system_clock_print(void)
{
  /* print clock frequencies */
  crm_clocks_freq_type crm_clocks_freq_struct = {0};
  crm_clocks_freq_get(&crm_clocks_freq_struct);

  printf("\r\n[CLK]\r\n");
  printf("SCLK: %d MHz, AHBCLK: %d MHz\r\n", crm_clocks_freq_struct.sclk_freq / 1000000, crm_clocks_freq_struct.ahb_freq / 1000000);
  printf("APB2CLK: %d MHz, APB1CLK: %d MHz\r\n", crm_clocks_freq_struct.apb2_freq / 1000000, crm_clocks_freq_struct.apb1_freq / 1000000);
}

/**
 * @brief  initialize systick for system timing
 * @note   configures systick for 10ms intervals to match current TMR6 usage
 * @retval none
 */
void systick_init(void)
{
	uint32_t systick_ticks;
    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    printf("Using SysTick for system timing\r\n");
    
    /* Get current clock frequencies */
    crm_clocks_freq_get(&crm_clocks_freq_struct);
    
    /* Configure SysTick clock source to use AHB clock (no division) */
    systick_clock_source_config(SYSTICK_CLOCK_SOURCE_AHBCLK_NODIV);
    
    /* Calculate ticks for 10ms interval (SYSTEMTICK_PERIOD_MS = 10) */
    systick_ticks = (crm_clocks_freq_struct.ahb_freq / 100) - 1;
    
    /* Configure SysTick with calculated ticks */
    if (SysTick_Config(systick_ticks) != 0)
    {
        /* Error: SysTick configuration failed */
        printf("SysTick configuration failed!\r\n");
        while(1);
    }

    printf("\r\n");
    printf("SysTick init: core_cm4 %d Hz, %d ticks\r\n", 
           crm_clocks_freq_struct.ahb_freq / (systick_ticks + 1), 
           systick_ticks);
}

/**
 * @brief  initialize tmr6 for emac
 * @note   wait usage
 * @retval none
 */
#include <stdio.h>
void emac_tmr_init(void)
{
    crm_clocks_freq_type crm_clocks_freq_struct = {0};

    printf("Using TMR6 for system timing\r\n");

    crm_periph_clock_enable(CRM_TMR6_PERIPH_CLOCK, TRUE);

    crm_clocks_freq_get(&crm_clocks_freq_struct);
    tmr_base_init(TMR6, 99, (crm_clocks_freq_struct.ahb_freq / 10000) - 1);
    tmr_cnt_dir_set(TMR6, TMR_COUNT_UP);

    /* overflow interrupt enable */
    tmr_interrupt_enable(TMR6, TMR_OVF_INT, TRUE);

    /* tmr1 overflow interrupt nvic init */
    nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
    nvic_irq_enable(TMR6_GLOBAL_IRQn, 0, 0);
    tmr_counter_enable(TMR6, TRUE);

	uint32_t ticks;
    printf("\r\n");
    printf("[AT32F403a]\r\n");
	printf("tmr_base_init: tmr6 \r\n");
	ticks = (crm_clocks_freq_struct.ahb_freq / 10000) - 1;
    printf("tmr6: %d Hz, %d ticks\r\n", 
           (crm_clocks_freq_struct.ahb_freq / (ticks + 1)) / 100, 
           (ticks)/100);
//	printf("CLK:%d(sclk_freq) %d %d(apb2_freq) %d(apb1_freq) \r\n",
//		crm_clocks_freq_struct.sclk_freq,
//		crm_clocks_freq_struct.ahb_freq,
//		crm_clocks_freq_struct.apb2_freq,
//		crm_clocks_freq_struct.apb1_freq);
}
