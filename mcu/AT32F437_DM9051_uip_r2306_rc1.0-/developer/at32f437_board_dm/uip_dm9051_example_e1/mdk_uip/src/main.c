/**
  **************************************************************************
  * @file     main.c
  * @version  v1.0
  * @date     2023-04-28
  * @brief    main program
  * @mode	  polling mode
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

//#include "at32f415_board.h" //"at32f403a_407_board.h"
//#include "at32f415_clock.h" //"at32f403a_407_clock.h"
#include "at32f435_437_board.h"
#include "at32f435_437_clock.h"
#include "uip_arp.h"
#include "developer_conf.h"
#include "ethernetif.h"
#include "dm9051_env.h" //"dm9051f_netconf.h" //"at32_emac.h"
#include "netconf.h"
#include "core/dm9051.h"

/** @addtogroup AT32F407_periph_examples
  * @{
  */

/** @addtogroup 415_dm9051_example
  * @{
  */

/**
  * @brief  main function.
  * @param  none
  * @retval none
  */
int main(void)
{
  system_clock_config();
  uart_print_init(115200);
  at32_board_init();

  printf("\r\n");
  printf("@example  example_uip\r\n");
  printf("@version  AT32%s_DM9051_uip_r2306_rc1.0\r\n", UIP_HEADER_MCU_DESC);
  printf("@date     2023-06-15\r\n");
  env_main_system_init();
  delay_ms(300);
	
//  tcpip_stack_init();
	dm9051_conf(); /*dm9051_boards_initialize();*/ //dm9051_pins_configuration();
	tapdev_init();

#if WEB_EN
	httpd_init();
#endif
	
  for(;;)
  {
	  tapdev_loop();
  }
}

/**
  * @}
  */

/**
  * @}
  */
