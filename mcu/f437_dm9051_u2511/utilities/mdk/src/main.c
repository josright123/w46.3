/**
  **************************************************************************
  * @file     main.c
  * @version  v2.0.6
  * @date     2022-03-11
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
#include "at32f435_437_board.h"
#include "at32f435_437_clock.h"
#include "stdio.h"
#include "FreeRTOS.h"
#include "task.h"

#include "../../dm9051_u2510_if/if.h" //"all"
#define AT_hal_tick_count dm9051_boards_heartbeat_now //dm_sys_now
uint32_t AT_hal_tick_count(void);
#define AT_hal_tick dm9051_boards_heartbeat_tick
void AT_hal_tick(void);

#if freeRTOS
#error "freeRTOS check defined, WRONG CONDITION!"
#else
//#warning "freeRTOS is exactly UIP not need defined"
#endif

#define NET_TASK_PRIO           		2 //FOR 'net_task'
#include "uIP_Task.h" //.void _vuIP_Task(void *pvParameters);

#include "main.h"

void main_tick_handler(void);

int Web_LED_FLASH = 1; // Default set 1 use freertos task control led, if set 0 web control

void create_network_task(void);

/** @addtogroup UTILITIES_examples
  * @{
  */
  
/** @addtogroup FreeRTOS_demo
  * @{
  */

TaskHandle_t network_handler;

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  nvic_priority_group_config(NVIC_PRIORITY_GROUP_4);
  system_clock_config();
  uart_print_init(115200);
  
  dm_eth_show_app_help_info("UIP_project", "web_server"); //printkey("\r\n\r\n\r\n/ZYK_project /R2410 [uip_dm9051_r2410] %s\r\n", __DATE__);

  /* enter critical */
  taskENTER_CRITICAL(); 

  create_network_task();
  
  /* exit critical */            
  taskEXIT_CRITICAL();      
              
  /* start scheduler */            
  vTaskStartScheduler(); 
}

//uint32_t mainTicks = 0;

//uint32_t main_tick_count(void)
//{
//	return mainTicks;
//}

//#define AT_hal_tick_count dm9051_boards_heartbeat_now //already defined in "hal_api.h"
//#define AT_hal_tick dm9051_boards_heartbeat_tick

/*******************************************************************************
 * Utility Functions
 ******************************************************************************/
// Tick Management
static uint32_t AT_HalTicks = 0;

uint32_t AT_hal_tick_count(void) {
	static uint32_t hal_tick_count = 0; 
	#if 1 //[Not only by SysTick counting][if take this, experiment to get 1000000 correction with your system.]
	hal_tick_count++;
	if (hal_tick_count >= 1000000) {
		hal_tick_count = 0;
		AT_HalTicks++;
	}
	#endif
    return AT_HalTicks;
}

// SysTick counting (NOT must essential)
void AT_hal_tick(void) {
    AT_HalTicks++;
}

void main_tick_handler(void)
{
	//mainTicks++;
	dm9051_boards_heartbeat_tick();
	xPortSysTickHandler(); //SysTick_Handler_from_main(); //xPortSysTickHandler(); 
}

void create_network_task(void)
{
  if(xTaskCreate((TaskFunction_t )vuIP_Task,     
                 (const char*    )"net_task",   
                 (uint16_t       )512+128, 
                 (void*          )NULL,
                 (UBaseType_t    )NET_TASK_PRIO, //2
                 (TaskHandle_t*  )&network_handler) != pdPASS)
  {
    printf("Net_task created Error!\r\n");
  }
}

/**
  * @}
  */ 

/**
  * @}
  */ 
