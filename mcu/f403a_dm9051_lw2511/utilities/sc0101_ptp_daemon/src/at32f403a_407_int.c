/**
  **************************************************************************
  * @file     at32f403a_407_int.c
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
#include "at32f403a_407_clock.h"
#include "at32f403a_407_int.h"
//void time_update(void); //#include "netconf.h"
//#include "ethernetif.h"

/** @addtogroup AT32F407_periph_examples
  * @{
  */

/** @addtogroup 407_EMAC_telnet
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
  while (1)
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
  while (1)
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
  while (1)
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
  while (1)
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

extern confirm_state identify_of_systick;
extern confirm_state identify_of_tmr;
/**
  * @brief  this function handles systick handler.
  * @note   This handler is called every 10ms when SysTick is configured
  *         using systick_init(). It updates the system time for network
  *         stack and PTP daemon operations.
  * @param  none
  * @retval none
  */
void SysTick_Handler(void)
{
    /* Update the local_time by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
    system_time_update(identify_of_systick); //time_update();
}

//void EXINT9_5_IRQHandler(void) {
//	if (exint_flag_get(HAL_IRQLine()) != RESET)
//	{
//		dm9051_interrupt_set(HAL_IRQLine()); //hal_enum_enqueue_interrupt(EXINT_LINE_9/...);
//		exint_flag_clear(HAL_IRQLine());
//	}
//}

/**
 * @brief  Timer 6 overflow interrupt handler
 *
 * This interrupt handler is triggered when Timer 6 overflows. It checks if the
 * overflow flag is set, updates the system time by calling time_update(), and
 * then clears the overflow flag.
 *
 * The handler performs following operations:
 * 1. Checks TMR6 overflow flag
 * 2. Updates local time via time_update()
 * 3. Clears the overflow flag
 *
 * @param  None
 * @retval None
 * @note   This handler is called automatically by hardware when TMR6 overflows
 */
void TMR6_GLOBAL_IRQHandler(void)
{
  if(tmr_flag_get(TMR6, TMR_OVF_FLAG) != RESET)
  {
    /* Update the local_time by adding SYSTEMTICK_PERIOD_MS each SysTick interrupt */
    system_time_update(identify_of_tmr); //time_update();
    tmr_flag_clear(TMR6, TMR_OVF_FLAG);
  }
}

/**
 * @brief EMAC (Ethernet MAC) Interrupt Handler for PTP timestamp trigger events
 *
 * This interrupt handler processes PTP (Precision Time Protocol) timestamp trigger events.
 * When triggered, it performs the following operations:
 * 1. Checks if interrupt is caused by timestamp trigger
 * 2. Gets current PTP time as basis for next trigger
 * 3. Performs different operations based on whether current second is odd or even
 * 4. Sets up next trigger time exactly 1 second in future
 * 5. Re-enables PTP trigger interrupt for next cycle
 *
 * Note: The trigger time must not be set in the past as it will cause immediate triggering
 *
 * @details The handler uses EMAC's PTP functionality to generate precise 1-second interval
 * triggers, useful for time synchronization and periodic tasks.
 *
 * @warning Setting trigger time in the past will cause immediate interrupt triggering
 */
void EMAC_IRQHandler(void)
{
#if 0
  struct ptptime_t next_trigger;

  /* 檢查是否為時間戳記觸發中斷 */
  if(emac_interrupt_status_read(EMAC_TST_FLAG) == SET)
  {
    /* 獲取當前PTP時間作為下一次觸發的基準 */
    /* 注意：設定的觸發時間不應在過去，否則會立即觸發 */
//    emac_ptptime_gettime(&next_trigger);
    /* 此處可根據時間狀態切換LED指示燈 */

    /* 根據秒數的奇偶來執行不同操作 */
    if(next_trigger.tv_sec % 2 == 0) {
        /* 偶數秒時的操作 */
    } else {
        /* 奇數秒時的操作 */
    }

    /* 設定下一次觸發時間為當前時間加1秒 */
    next_trigger.tv_sec += 1;
    /* 設定PTP目標觸發時間（秒部分） */
//    emac_ptp_target_second_set(next_trigger.tv_sec);
    /* 設定PTP目標觸發時間（納秒部分）為0 */
//    emac_ptp_target_nanosecond_set(0);

    /* 啟用下一次PTP時間觸發中斷 */
//    emac_ptp_interrupt_trigger_enable(TRUE);
  }
#endif
}
/**
  * @}
  */

/**
  * @}
  */
